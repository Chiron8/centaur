#include "parse.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define BASE_DIR "/etc/centaur/packages"

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

void dep_check(char file[], char package[]) {
    FILE *fptr = fopen(file, "r");
    char line[256];

    if (strcmp(fgets(line, 256, fptr), "(null)")) {
        return;
    }
    else {
        fclose(fptr);
        FILE *fptr = fopen(file, "r");
        char line[256];
        printf("%s %s%s\n", "The following dependencies still rely on", package, ":");
        while (fgets(line, 256, fptr)) {
            printf(" - %s\n", line);
        }
        printf("%s\n", "Please uninstall these dependencies first.");
        exit(1);
    }
    fclose(fptr);
}

int uninstall(char package[], bool force) {
    char installPath[300];
    char noversion[150];
    snprintf(noversion, sizeof(noversion), "%s", remove_version(package));
    snprintf(installPath, sizeof(installPath), "%s/installed/%s%s", BASE_DIR, package, ".centaur");

    if (access(installPath, F_OK) != 0) {
        printf("%s\n", installPath);
        printf("%s %s %s\n", "Package", package, "not installed.");
        exit(1);
    } 

    if (force != true) {
        dep_check(installPath, package);
    }

    char uninstallPath[200];
    snprintf(uninstallPath, sizeof(uninstallPath), "%s/uninstall/%s%s", BASE_DIR, package, ".centaur");

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
