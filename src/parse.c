#define _GNU_SOURCE
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

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
               "Unable to locate install file",
               file,
               ". Perhaps you spelt it wrong?\033[0m");
        exit(1);
    }

    char *line = NULL;
    size_t len = 0;

    size_t capacity = 512;
    size_t currentLen = 0;
    char *command = malloc(capacity);
    if (!command) {
        fclose(fptr);
        return 1;
    }

    command[0] = '\0';

    // skip dependency block if needed
    if (!uninstall) {
        while (getline(&line, &len, fptr) != -1 && strcmp(line, "[install]") != 0) {
            // skip until [install] line
        }
    }

    bool first = true;

    while (getline(&line, &len, fptr) != -1) {
        // strip newline
        line[strcspn(line, "\n")] = '\0';
        size_t lineLen = strlen(line);

        if (lineLen == 0 || line[0] == '#') {
            continue;
        }

        if (!uninstall && strcmp(line, "[/install]") == 0) {
            break;
        }

        // resize if needed
        while (currentLen + lineLen + 5 >= capacity) {
            capacity *= 2;
            char *tmp = realloc(command, capacity);
            if (!tmp) {
                free(command);
                free(line);
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

    free(line);
    fclose(fptr);

    int result = system(command);

    free(command);
    return (result > 0);
}
