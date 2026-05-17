int remove_directory(const char *path);
int copy_data(struct archive *ar, struct archive *aw);
int extract_tar_gz(const char *filename, const char *dest);
int replace_folders(char original_dir[], char replacer[]);
int download_package_dir();
int centaur_sync();
