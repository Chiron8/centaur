#include "parse.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#define BASE_DIR "/etc/centaur/packages"

int remove_line(const char *file, const char *target) {
    // weird way because theres no inbuilt func :(
    FILE *in = fopen(file, "r");
    if (!in) {
        perror("Failed to open input file");
        exit(1);
    }

    FILE *out = fopen("/tmp/centaur/temp.txt", "w");
    if (!out) {
        perror("Failed to open temp file");
        fclose(in);
        exit(1);
    }

    char line[256];
    int removed = 0;

    while (fgets(line, sizeof(line), in)) {
        char temp[256];
        strcpy(temp, line);

        // get rid of new line
        temp[strcspn(temp, "\n")] = 0;
        if (strcmp(temp, target) == 0) {
            removed = 1;
            continue;
        }
        fputs(line, out);
    }

    fclose(in);
    fclose(out);

    if (remove(file) != 0 || rename("/tmp/centaur/temp.txt", file) != 0) {
        perror("Failed to replace original file");
        exit(1);
    }

    return removed;
}

// I've now come to the conclusion that I have no idea what I'm doin
int remove_parent_from_installed_versions_of_dep(const char directory[], const char installPath[], const char package[]) {
    FILE *fptr = fopen(installPath, "r");
    char line[256];
    char depFile[512];
    char result[512];

    snprintf(result, sizeof(result), "%s%s", package, ".centaur");

    DIR *dir = opendir(directory);
    struct dirent *entry;

    if (!dir) {
        perror("opendir");
        return 1;
    }

    // only bother reading dependencies
    while (fgets(line, 256, fptr) && line[0] != '=') {
        continue;
    }

    while (fgets(line, 256, fptr)) {
        line[strcspn(line, "\n")] = 0;
        while ((entry = readdir(dir)) != NULL) {
            if (strstr(entry->d_name, line)) {
                snprintf(depFile, sizeof(depFile), "%s%s", "/etc/centaur/packages/installed/", entry->d_name);
                remove_line(depFile, result);
            }
        }
    }
    fclose(fptr);
    return 0;
}

char* remove_version(char package[]) {
    // example-1.2.3 -> example
    static char new[100];
    int index = 0;
    // removes everything past and including -
    while (package[index] != '-' && package[index] != '\0') {
        new[index] = package[index];
        index++;
    }
    new[index] = '\0';
    return new;
}

void dep_check(char file[], char package[]) {
    // install file contains parents
    FILE *fptr = fopen(file, "r");
    char line[256];

    if (fgets(line, 256, fptr) && line[0] == '=') {
        return;
    } else {
        printf("%s %s%s\n", "The following dependencies still rely on", package, ":");
        while (fgets(line, 256, fptr) && line[0] != '=') {
            // print every dep
            printf(" - %s\n", line);
        }
        printf("%s\n", "Please uninstall these dependencies first.");
        exit(1);
    }
    fclose(fptr);
}

int uninstall(char package[], int force) {
    char installPath[300];
    char noversion[150];
    snprintf(noversion, sizeof(noversion), "%s", remove_version(package));
    snprintf(installPath, sizeof(installPath), "%s/installed/%s%s", BASE_DIR, package, ".centaur");

    if (access(installPath, F_OK) != 0) {
        printf("%s\n", installPath);
        printf("%s %s %s\n", "Package", package, "not installed.");
        exit(1);
    } 

    if (force != 1) {
        dep_check(installPath, package);
    }

    char uninstallPath[200];
    snprintf(uninstallPath, sizeof(uninstallPath), "%s/uninstall/%s%s", BASE_DIR, package, ".centaur");

    read_execute(uninstallPath, force, true);
    
    remove_parent_from_installed_versions_of_dep("/etc/centaur/packages/installed", installPath, package);


    printf("%s %s\n", package, "uninstalled!");

    if (remove(installPath) | remove(uninstallPath)) { // ensure both run
        if (force != 1) {
            printf("%s\n", "Error removing files!");
            exit(1);
        }
    }
    return 0;
}
