#include <dirent.h>
#include <stdio.h>
#include <curl/curl.h>
#include <string.h>
#include <archive.h>
#include <archive_entry.h>
#include <stdlib.h>

#include "curl_hash.h"

// Source - https://stackoverflow.com/a/2256974
// Posted by asveikau, modified by community. See post 'Timeline' for change history
// Retrieved 2026-05-17, License - CC BY-SA 4.0

int remove_directory(const char *path) {
   DIR *d = opendir(path);
   size_t path_len = strlen(path);
   int r = -1;

   if (d) {
      struct dirent *p;

      r = 0;
      while (!r && (p=readdir(d))) {
          int r2 = -1;
          char *buf;
          size_t len;

          /* Skip the names "." and ".." as we don't want to recurse on them. */
          if (!strcmp(p->d_name, ".") || !strcmp(p->d_name, ".."))
             continue;

          len = path_len + strlen(p->d_name) + 2; 
          buf = malloc(len);

          if (buf) {
             struct stat statbuf;

             snprintf(buf, len, "%s/%s", path, p->d_name);
             if (!stat(buf, &statbuf)) {
                if (S_ISDIR(statbuf.st_mode))
                   r2 = remove_directory(buf);
                else
                   r2 = unlink(buf);
             }
             free(buf);
          }
          r = r2;
      }
      closedir(d);
   }

   if (!r)
      r = rmdir(path);

   return r;
}

int copy_data(struct archive *ar, struct archive *aw) {
    int r;
    const void *buff;
    size_t size;
    la_int64_t offset;

    while (1) {
        r = archive_read_data_block(ar, &buff, &size, &offset);

        if (r == ARCHIVE_EOF) {
            return ARCHIVE_OK;
        }

        if (r != ARCHIVE_OK) {
            return r;
        }

        r = archive_write_data_block(aw, buff, size, offset);

        if (r != ARCHIVE_OK) {
            fprintf(stderr, "%s\n", archive_error_string(aw));
            return r;
        }
    }
}

int extract_tar_gz(const char *filename, const char *dest) {
    struct archive *a;
    struct archive *ext;
    struct archive_entry *entry;
    int flags, r;

    flags = ARCHIVE_EXTRACT_TIME;

    a = archive_read_new();
    archive_read_support_format_tar(a);
    archive_read_support_filter_gzip(a);

    ext = archive_write_disk_new();
    archive_write_disk_set_options(ext, flags);
    archive_write_disk_set_standard_lookup(ext);

    if ((r = archive_read_open_filename(a, filename, 10240))) {
        fprintf(stderr, "Could not open archive\n");
        return 1;
    }

    while (1) {
        r = archive_read_next_header(a, &entry);

        if (r == ARCHIVE_EOF) {
            break;
        }

        if (r < ARCHIVE_OK) {
            fprintf(stderr, "%s\n", archive_error_string(a));
        }

        if (r < ARCHIVE_WARN) {
            return 1;
        }

        const char *current = archive_entry_pathname(entry);

        char fullpath[512];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", dest, current);

        archive_entry_set_pathname(entry, fullpath);
        r = archive_write_header(ext, entry);

        if (r < ARCHIVE_OK) {
            fprintf(stderr, "%s\n", archive_error_string(ext));
        }
        else if (archive_entry_size(entry) > 0) {
            copy_data(a, ext);
        }
        
        r = archive_write_finish_entry(ext);

        if (r < ARCHIVE_OK) {
            fprintf(stderr, "%s\n", archive_error_string(ext));
            return 1;
        }
    }

    archive_write_close(ext);
    archive_write_free(ext);
    archive_read_close(a);
    archive_read_free(a);

    return 0;
}

int replace_folders(char original_dir[], char replacer[]) {
    char tmp[512];
    snprintf(tmp, sizeof(tmp), "%s.old", original_dir);

    if (rename(original_dir, tmp) != 0) {
        perror("renaming dir failed");
        exit(1);
    } 

    if (rename(replacer, original_dir) != 0) {
        rename(tmp, original_dir);
        perror("replacing package dir with new package dir failed");
        exit(1);
    }

    remove_directory(tmp);

    return 0;
}

int download_package_dir() {
    CURL *curl;
    FILE *fp;

    char url[512];
    snprintf(url, sizeof(url), "https://github.com/Chiron8/centaur/archive/refs/heads/master.tar.gz");

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl) {
        fp = fopen("/etc/centaur/tmp/centaur.tar.gz", "wb");

        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        CURLcode res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            perror("Curl download failed");
            exit(1);
        }

        fclose(fp);
        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();

    return 0;
}

int centaur_sync() {
    download_package_dir();
    extract_tar_gz("/etc/centaur/tmp/centaur.tar.gz", "/etc/centaur/tmp/git_centaur");
    replace_folders("/etc/centaur/packages", "/etc/centaur/tmp/git_centaur/packages");
    remove_directory("/etc/centaur/tmp/git_centaur");
    remove("/etc/centaur/tmp/centaur.tar.gz");

    return 0;
}
