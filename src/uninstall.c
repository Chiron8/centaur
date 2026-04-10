#include "parse.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define BASE_DIR "/etc/centaur/packages/"

char* remove_version(char package[]) {
    static char new[100];
    int index = 0;
    while (package[index] != '-' && package[index] != '\0') {
        new[index] = package[index];
        index++;
    }
    new[index] = '\0';
    return new;
}

int dep_check(char file[], char package[]) {
    FILE *fptr = fopen(file, "r");
    char line[256];

    if (fgets(line, 256, fptr) == NULL) {
        return 0;
    }
    else {
        printf("%s %s%s\n", "The following dependencies still rely on", package, ":");
        while (fgets(line, 256, fptr)) {
            printf("%s\n", line);
        }
        printf("\n%s\n", "Please uninstall these dependencies first.");
        return 1;
    }
    fclose(fptr);
}

int uninstall(char package[], bool force) {
    char installPath[300];
    char noversion[150];
    snprintf(noversion, sizeof(noversion), "%s", remove_version(package));
    snprintf(installPath, sizeof(installPath), "%s/installed/%s/%s", BASE_DIR, noversion, package);

    if (access(installPath, F_OK) != 0) {
        printf("%s %s %s\n", "Package", package, "not installed.");
        exit(1);
    } 

    if (force != true && dep_check(installPath, package) == 1) {
        exit(1);
    }

    char uninstallPath[200];
    snprintf(uninstallPath, sizeof(uninstallPath), "%s/uninstall/%s", BASE_DIR, package);

    read_execute(uninstallPath, force);

    if (remove(installPath) | remove(uninstallPath)) { // ensure both run
        if (force != true) {
            printf("%s\n", "Error removing files!");
            exit(1);
        }
    }

    printf("%s %s\n", package, "uninstalled!");
    return 0;
}
