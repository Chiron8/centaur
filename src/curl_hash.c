#include <curl/curl.h> // might write own implementation later
#include <string.h>
#include <stdio.h>

size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    size_t written = fwrite(ptr, size, nmemb, stream);
    return written;
}

int download_hash(const char package[], const char noversion[]) {
    char url[512];
    CURL *curl;
    FILE *fp;
    snprintf(url, sizeof(url), "https://raw.githubusercontent.com/Chiron8/centaur/refs/heads/master/packages/HASHES/%s/%s", noversion, package);

    char outputFile[256];
    snprintf(outputFile, sizeof(outputFile), "%s/%s", "/etc/centaur/tmp", package); // no / on purpose, don't want to make dir

    curl = curl_easy_init();
    if (curl) {
        fp = fopen(outputFile, "wb");
        if (!fp) {
            perror("failed to open file to download hash");
            return 1;
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
    return 0;
}
