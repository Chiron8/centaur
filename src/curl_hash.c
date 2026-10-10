#include <curl/curl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) { // freaky curl stuff
    size_t written = fwrite(ptr, size, nmemb, stream);
    return written;
}

char* download_hash(const char package_name[], const char package_name_no_version[]) {
    char url[512];
    CURL *curl;
    FILE *fp;
    snprintf(url, sizeof(url), "https://raw.githubusercontent.com/Chiron8/centaur/refs/heads/master/packages/HASHES/%s/%s", package_name_no_version, package_name); // remote hash address

    char output_file[256];
    snprintf(output_file, sizeof(output_file), "/tmp/centaur/%sHASH", package_name); // no / on purpose, don't want to make dir

    curl = curl_easy_init(); // more curl stuff I don't understand
    if (curl) {
        fp = fopen(output_file, "wb");
        if (!fp) {
            perror("Failed to open file to download hash");
            return NULL;
        }
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);

        CURLcode res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            fprintf(stderr, "curl error: %s\n", curl_easy_strerror(res));
        }

        curl_easy_cleanup(curl);
        fclose(fp);
    }
    curl_global_cleanup();
    
    FILE *fptr = fopen(output_file, "r"); // open hash file to read
    if (fptr == NULL) {
        perror("Count not open package hash file");
        exit(1);
    }

    char line[512];
    if (fgets(line, 512, fptr) == NULL) {
        perror("package hash file empty!");
        fclose(fptr);
        return NULL;
    }
    fclose(fptr);
    line[strcspn(line, "\r\n")] = 0;

    char *result = malloc(strlen(line)+1);
    if (result == NULL) {
        perror("malloc failed");
        exit(1);
    }

    strcpy(result, line);
    return result; // return hash
}
