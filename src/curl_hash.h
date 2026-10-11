size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream);
char* download_hash(const char package[], const char noversion[]);
int curl_tarball(char source[], char download_file[]);
