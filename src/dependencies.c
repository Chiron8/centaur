#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void reverseDependencies(char **deps, size_t size) {
    if (deps == NULL || size == 0) {
        return;
    }

    for (size_t i = 0; i < size / 2; i++) {
        char *temp = deps[i];
        deps[i] = deps[size-1-i];
        deps[size-1-i] = temp;
    }
}

bool alreadyAdded(char *target, char **deps, size_t deps_size) {
    for (size_t i = 0; i < deps_size; i++) {
        if (strcmp(deps[i], target) == 0) {
            return true;
        }
    }
    return false;
}

char **getDependencies(const char *file, char **deps, size_t *deps_size) {
    // load file
    char path[200];
    snprintf(path, sizeof(path), "/etc/centaur/packages/scripts/%s", file);
    FILE *fptr = fopen(path, "r");

    if (fptr == NULL) {
        printf("%s %s\n", "Something went wrong with dependency resolution :( Could not open", path);
        exit(1);
    }

    char line[150];
    char file_dep[200];

    snprintf(file_dep, sizeof(file_dep), "%s", file);

    (*deps_size)++;
    char **tmp_deps = realloc(deps, (*deps_size) * sizeof(char *));
    if (tmp_deps == NULL) {
        perror("Failed to realloc memory");
        exit(1);
    }
    deps = tmp_deps;
    deps[*deps_size - 1] = strdup(file_dep);


    while(fgets(line, sizeof(line), fptr) && line[0] != '=') {
        if (line[0] == '#' || line[0] == '\n') {
            continue;
        }
        line[strcspn(line, "\n")] = '\0';
        if (!alreadyAdded(line, deps, *deps_size)) {
            deps = getDependencies(line, deps, deps_size);
        }
        else {
            printf("Skipping circular dep");
        }
    }
    fclose(fptr);
    return deps;
}
