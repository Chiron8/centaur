#define _GNU_SOURCE
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

// returns value of variable in script file
// caller must free the string
char *get_meta_variable_value(FILE *script_file, const char *target_variable) {
    if (!script_file || !target_variable) {
        return NULL;
    }

    char line[512];
    bool in_meta = false;
    size_t target_len = strlen(target_variable);

    while (fgets(line, sizeof(line), script_file)) {
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

            p[strlen(p)-1] = '\0';
            return strdup(p); // woah this was easy peasy
        }
    }
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

int read_execute(char file[], bool force, bool uninstall) {
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

    int result = system(command);

    free(command);
    return (result > 0);
}
