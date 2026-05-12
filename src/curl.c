#include <stdio.h>
#include <curl/curl.h> // might write own implementation later
#include <curl/types.h>
#include <curl/easy.h>
#include <string.h>

size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    size_t written = fwrite(ptr, size, nmemb, stream);
    return written;
}

int download_file(const char url[], const char package[], const char noversion[]) {
    CURL *curl;
    FILE *fp;
    CURLcode res;
    char outputFile[256];
    snprintf(outputFile, sizeof(outputFile), "%s%s%s", "/etc/centaur/tmp/", noversion, package);
    curl = curl_easy_init();
    if (curl) {
        fp = fopen(outputFile, "wb");
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        res = curl_easy_perform(curl);
        /* always cleanup */
        curl_easy_cleanup(curl);
        fclose(fp);
    }
    return 0;
}
