#include <dirent.h>
#include <stdio.h>
#include <curl/curl.h>
#include <string.h>
#include <archive.h>
#include <archive_entry.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>

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
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
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

    system("cd /etc/centaur/tmp/ && tar -xf centaur.tar.gz"); // i really cba fighting with libarchive
    replace_folders("/etc/centaur/packages", "/etc/centaur/tmp/centaur-master/packages");
    remove_directory("/etc/centaur/tmp/git_centaur");
    remove("/etc/centaur/tmp/centaur.tar.gz");
    return 0;
}
