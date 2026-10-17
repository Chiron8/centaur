#define _GNU_SOURCE
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>

// returns value of variable in script file
// caller must free the string
char *get_meta_variable_value(char *script_path, const char *target_variable) {
    if (!script_path || !target_variable) {
        return NULL;
    }

    char line[512];
    bool in_meta = false;
    size_t target_len = strlen(target_variable);
    FILE *script_fptr = fopen(script_path, "r");
    if (script_fptr == NULL) {
        perror("Could not open script file");
        fclose(script_fptr);
        exit(1);
    }

    while (fgets(line, sizeof(line), script_fptr)) {
        if (strstr(line, "[meta]") != NULL) {
            in_meta = true;
            continue;
        }
        if (strstr(line, "[/meta]") != NULL) {
            break;
        }
        if (!in_meta) {
            continue;
        }

        if (strstr(line, target_variable) != NULL) {
            char *p = line + target_len; // skip past variable name in memory
            while (*p == ' ' || *p == '\t') p++;
            if (*p != '=') {
                continue; // if line doesn't have = next, it isn't the (variable = value) line
            }
            p++; // skip =
            while (*p == ' ' || *p == '\t') p++;
            if (*p == '"') {
                p++;
            }
            // remove ending " or SPACE in p
            size_t val_len = strlen(p);
            if (val_len > 0 && p[val_len - 1] == '\n') {
                p[val_len - 1] = '\0';
            }

            if (p[strlen(p)-1] == '"') {
                p[strlen(p)-1] = '\0';
            }
            fclose(script_fptr);
            return strdup(p); // woah this was easy peasy
        }
    }
    fclose(script_fptr);
    return NULL;
}

int get_latest(const char *path, char *latest, size_t size) {
    // use GNU version sort to find latest version
    struct dirent **namelist;
    int n;

    n = scandir(path, &namelist, NULL, versionsort);
    if (n < 0) {
        perror("scandir");
        return -1;
    }

    if (n == 0) {
        return 0;
    }

    // get last entry
    const char *name = namelist[n - 1] -> d_name;

    snprintf(latest, size, "%s", name);
    latest[size-1] = '\0';

    for (int i = 0; i < n; i++) {
        free(namelist[i]);
    }
    free(namelist);

    return 0;
}

int prepare_install(char script_file[], char build_dir[], bool force) {
    char installer_path[512];
    snprintf(installer_path, sizeof(installer_path), "%s/centaur_installer.sh", build_dir);
    FILE *installer_fptr = fopen(installer_path, "w");
    if (!installer_fptr) {
        perror("Could not create installer");
        exit(1);
    }
    FILE *script_fptr = fopen(script_file, "r");
    if (!script_fptr) {
        perror("Could not open package script file");
        exit(1);
    }
    char line[512];
    while (fgets(line, sizeof(line), script_fptr) && strcmp(line, "[install]\n") != 0) {
        // skip until [install]
    }
    while (fgets(line, sizeof(line), script_fptr) && strcmp(line, "[/install]\n") != 0) {
        fprintf(installer_fptr, "%s", line);
    }
    fclose(installer_fptr);
    fclose(script_fptr);
    return 0;
}



int read_execute(char file[], char build_dir[], bool force, bool uninstall) {
    FILE *fptr = fopen(file, "r");

    if (fptr == NULL && force != true) {
        printf("\033[31m%s %s %s\n",
               "Unable to locate file",
               file,
               ". Perhaps you spelt it wrong?\033[0m");
        exit(1);
    }

    char line[256];

    size_t capacity = 512;
    size_t currentLen = 0;
    char *command = malloc(capacity);
    if (!command) {
        fclose(fptr);
        perror("Out of memory :(");
        return 1;
    }

    command[0] = '\0';

    if (!uninstall) {
        while (fgets(line, sizeof(line), fptr) && strcmp(line, "[install]\n") != 0) {
            // skip until [install]
        }
    }

    bool first = true;

    while (fgets(line, sizeof(line), fptr) && strcmp(line, "[/install]\n") != 0) { // checking for [/install] doesn't m
        // strip newline
        line[strcspn(line, "\n")] = '\0';
        size_t lineLen = strlen(line);

        if (lineLen == 0 || line[0] == '#') {
            continue;
        }

        // resize if needed
        while (currentLen + lineLen + 5 >= capacity) {
            capacity *= 2;
            char *tmp = realloc(command, capacity);
            if (!tmp) {
                free(command);
                fclose(fptr);
                return 1;
            }
            command = tmp;
        }

        if (!first) {
            strcat(command, " && ");
            currentLen += 4;
        }

        strcat(command, line);
        currentLen += lineLen;
        first = false;
    }

    fclose(fptr);

    if (build_dir) {
        if (chdir(build_dir) != 0) {
            perror("Failed to change directory");
            exit(1);
        }
    }

    int result = system(command); // crazy how this is the thing that just does everything

    free(command);
    return (result > 0);
}
