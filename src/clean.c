#include "uninstall.h"

#include <stdbool.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

struct dirent *ent;

int check_file(const char package_name[]) {
    char install_path[512];
    snprintf(install_path, sizeof(install_path), "%s%s", "/var/db/centaur/packages/installed/", package_name); // install file path of packge_name

    FILE *install_fptr = fopen(install_path, "r");

    if (install_fptr == NULL) {
        perror("Could not open install file");
        return 1;
    }

    char line[256];
    if (fgets(line, 256, install_fptr) && line[0] == '=') { // if first line starts with "=" there are no dependencies
        fclose(install_fptr);
        return 0; // safe to remove
    }
    fclose(install_fptr);
    return 1;
}

int clean() {
    DIR *dir;
    if ((dir = opendir("/var/db/centaur/packages/installed")) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            if (strcmp(ent->d_name, "..") != 0 && strcmp(ent->d_name, ".") != 0) { // prevent trying to access files that aren't packages
                if (check_file(ent->d_name) == 0) {
                    uninstall(ent->d_name, false);
                }
            }
        }
        closedir(dir);
    } else {
        closedir(dir);
        perror("Could not open directory");
        exit(1);
    }
    return 0;
}
