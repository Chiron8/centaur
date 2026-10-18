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
