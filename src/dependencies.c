#include "parse.h"
#include "uninstall.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// dep is short for dependency
// deps is short for dependencies

typedef struct {
    char *dep;
    char *parent;
} Dependency;

void reverse_dependencies(Dependency *deps, const size_t size) {
    // dep graph needs to be reversed so deps are installed before parent
    if (deps == NULL || size == 0) {
        return;
    }

    // reverse
    for (size_t i = 0; i < size / 2; i++) {
        char *temp_dep = deps[i].dep;
        char *temp_parent = deps[i].parent;
        deps[i].dep = deps[size-1-i].dep;
        deps[i].parent = deps[size-1-i].parent;
        deps[size-1-i].dep = temp_dep;
        deps[size-1-i].parent = temp_parent;
    }
}

bool already_added(const char *target, const Dependency *deps, const size_t deps_size) {
    // avoid circular dep
    for (size_t i = 0; i < deps_size; i++) {
        if (strcmp(deps[i].dep, target) == 0) {
            return true;
        }
    }
    return false;
}

void free_dependencies(Dependency *deps, const size_t size) {
    // i <3 memory
    for (size_t i = 0; i < size; i++) {
        free(deps[i].dep);
        free(deps[i].parent);
    }
    free(deps);
}

Dependency *get_dependencies(const char *file, const char *parent, Dependency *deps, size_t *deps_size) {
    // load file
    char parent_script_path[200];
    char latest[150];

    snprintf(parent_script_path, sizeof(parent_script_path), "/var/db/centaur/packages/scripts/%s", file);

    // get latest version of dep
    // this is going to break when we need specific version :)
    int code = get_latest(parent_script_path, latest, sizeof(latest));

    if (code == -1) {
        printf("%s\n", "\033[31mSomething went wrong with getting latest (deps.c)\033[0m");
        exit(1);
    }

    char package_script_path[350];
    snprintf(package_script_path, sizeof(package_script_path), "%s/%s", parent_script_path, latest);

    FILE *fptr = fopen(package_script_path, "r");

    if (fptr == NULL) {
        printf("\033[31m%s %s\033[0m\n", "Something went wrong with dependency resolution :( Could not open", package_script_path);
        exit(1);
    }

    (*deps_size)++;
    Dependency *tmp_deps = realloc(deps, (*deps_size) * sizeof(Dependency));
    if (tmp_deps == NULL) {
        perror("\033[31mFailed to realloc memory");
        exit(1);
    }
    deps = tmp_deps;
    deps[*deps_size - 1].dep = strdup(file);
    deps[*deps_size -1].parent = parent ? strdup(parent) : NULL;

    char *value = get_meta_variable_value(package_script_path, "dependencies");
    if (value) {
        char *save_ptr;
        char *token = strtok_r(value, " ", &save_ptr);
        while (token) {
            if (!already_added(token, deps, *deps_size)) {
                // recursion RAAHHH
                deps = get_dependencies(token, file, deps, deps_size);
            }
            else {
                printf("\033[34mSkipping circular dep\033[0m");
            }    
            token = strtok_r(NULL, " ", &save_ptr);
        }
    }
    free(value);
    fclose(fptr);
    return deps;
}
