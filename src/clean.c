//#include "uninstall.h"
#include "uninstall.h"

#include <stdbool.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

struct dirent *ent;

int check_file(const char name[]) {
    char worldPath[512];
    snprintf(worldPath, sizeof(worldPath), "%s%s", "/etc/centaur/packages/world", name);

    if (access(worldPath, F_OK) == 0) {
        return 1;
    }

    char installPath[512];
    snprintf(installPath, sizeof(installPath), "%s%s", "/etc/centaur/packages/installed/", name);

    FILE *install_fptr = fopen(installPath, "r");

    if (install_fptr == NULL) {
        printf(installPath);
        perror("Could not open install file");
        return 1;
    }

    char line[256];

    if (fgets(line, 256, install_fptr) && line[0] == '=') {
        fclose(install_fptr);
        return 0; // safe to remove
    }
    fclose(install_fptr);
    return 1;
}

int clean() {
    DIR *dir;
    if ((dir = opendir("/etc/centaur/packages/installed")) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            if (strcmp(ent->d_name, "..") != 0 && strcmp(ent->d_name, ".") != 0) {
                if (check_file(ent->d_name) == 0) {
                    int len = strlen(ent->d_name);
                    ent->d_name[len-8] = '\0';
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
