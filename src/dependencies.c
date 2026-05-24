#include "parse.h"
#include "uninstall.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char *dep;
    char *parent;
} Dependency;

void reverseDependencies(Dependency *deps, size_t size) {
    // dep graph needs to be reversed so deps are installed before parent
    if (deps == NULL || size == 0) {
        return;
    }

    for (size_t i = 0; i < size / 2; i++) {
        char *temp_dep = deps[i].dep;
        char *temp_parent = deps[i].parent;
        deps[i].dep = deps[size-1-i].dep;
        deps[i].parent = deps[size-1-i].parent;
        deps[size-1-i].dep = temp_dep;
        deps[size-1-i].parent = temp_parent;
    }
}

bool alreadyAdded(char *target, Dependency *deps, size_t deps_size) {
    // avoid circular dep
    for (size_t i = 0; i < deps_size; i++) {
        if (strcmp(deps[i].dep, target) == 0) {
            return true;
        }
    }
    return false;
}

void freeDependencies(Dependency *deps, size_t size) {
    // i <3 memory
    for (size_t i = 0; i < size; i++) {
        free(deps[i].dep);
        free(deps[i].parent);
    }
    free(deps);
}

Dependency *getDependencies(const char *file, const char *parent, Dependency *deps, size_t *deps_size) {
    // load file
    char packagePath[200];
    char latest[150];

    snprintf(packagePath, sizeof(packagePath), "/etc/centaur/packages/scripts/%s", file);

    // get latest version of dep
    // this is going to break when we need specific version :)
    int code = get_latest(packagePath, latest, sizeof(latest));

    if (code == -1) {
        printf("%s\n", "Something went wrong with getting latest (deps.c)");
        exit(1);
    }

    char path[350];
    snprintf(path, sizeof(path), "%s/%s", packagePath, latest);

    FILE *fptr = fopen(path, "r");

    if (fptr == NULL) {
        printf("%s %s\n", "Something went wrong with dependency resolution :( Could not open", path);
        exit(1);
    }

    char line[150];
    char file_dep[200];

    snprintf(file_dep, sizeof(file_dep), "%s", file);

    (*deps_size)++;
    Dependency *tmp_deps = realloc(deps, (*deps_size) * sizeof(Dependency));
    if (tmp_deps == NULL) {
        perror("Failed to realloc memory");
        exit(1);
    }
    deps = tmp_deps;
    deps[*deps_size - 1].dep = strdup(file);
    deps[*deps_size -1].parent = parent ? strdup(parent) : NULL;

    while(fgets(line, sizeof(line), fptr) && line[0] != '=') {
        if (line[0] == '#' || line[0] == '\n') {
            continue;
        }
        line[strcspn(line, "\n")] = '\0';
        if (!alreadyAdded(line, deps, *deps_size)) {
            // recursion RAAHHH
            deps = getDependencies(line, file, deps, deps_size);
        }
        else {
            printf("Skipping circular dep");
        }
    }
    fclose(fptr);
    return deps;
}
