#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>

#include "uninstall.h"
#include "parse.h"
#include "install.h"
#include "dependencies.h"

#define SCRIPT_PATH "/etc/centaur/packages/scripts/"
#define INSTALLED_PATH "/etc/centaur/packages/installed/"

int needs_update(const char noversion[], const char full_package[]) {
    // full_package = example-0.1.centaur
    char packageDir[512];
    snprintf(packageDir, sizeof(packageDir), "/etc/centaur/packages/scripts/%s", noversion);

    char packageLatest[128];
    if (get_latest(packageDir, packageLatest, sizeof(packageLatest)) != 0) {
        perror("Could not get latest package");
        exit(1);
    }

    if (strcmp(packageLatest, full_package) == 0) return 0;
    return 1;
}

int add_to_array(char ***arr, int *arrCap, int *arrSize, char* element) {
    if (element == NULL) {
        return 1;
    }

    if (*arrSize >= *arrCap) {
        (*arrCap) *= 2;
        char **temp = realloc(*arr, (*arrCap) * sizeof(char*));
        if (temp == NULL) {
            perror("Coulnd not create tmp array, realloc failed");
            return 1;
        }
        *arr = temp;
    }

    (*arr)[*arrSize] = malloc(strlen(element)+1);
    strcpy((*arr)[*arrSize], element);
    (*arrSize)++;
    return 0;
}

int update() {
    // Source - https://stackoverflow.com/a/7876756
// Posted by Frerich Raabe, modified by community. See post 'Timeline' for change history
// Retrieved 2026-05-24, License - CC BY-SA 4.0

    int depCap = 100;
    int depSize = 0;
    char **dep = malloc(depCap * sizeof(char*));

    int parentCap = 100;
    int parentSize = 0;
    char **parent = malloc(parentCap * sizeof(char*));

    if (dep == NULL) {
        perror("could not create array, malloc failed");
        exit(1);
    }

    struct dirent *de;
    DIR *dr = opendir(INSTALLED_PATH);

    if (dr == NULL) {
        perror("Could not open installed script directory");
        exit(1);
    }

    printf("%s\n", "\033[34mThinking...\033[0m");

    while ((de = readdir(dr)) != NULL) {
        if (strcmp(de->d_name, "..") != 0 && strcmp(de->d_name, ".") != 0 && needs_update(remove_version(de->d_name), de->d_name)) {
            Dependency *deps = NULL;
            size_t total_deps = 0;
            deps = getDependencies(remove_version(de->d_name), NULL, deps, &total_deps);
            reverseDependencies(deps, total_deps);

            for (size_t i = 0; i < total_deps; i++) {
                add_to_array(&dep, &depCap, &depSize, deps[i].dep);
                add_to_array(&parent, &parentCap, &parentSize, deps[i].parent);
            }
        }
    }
    if (depSize == 0) {
        printf("%s\n", "\033[32mNo packages to update!\033[0m");
        return 0;
    }

    printf("%s\n", "\033[34mUpdating packages:\033[0m");
    for (int i = 0; i < depSize; i++) {
        printf("\033[32m- %s\033[0m\n", dep[i]);
    }

    for (int i = 0; i < depSize; i++) {
        install(dep[i], parent[i]);
    }

    for (int i = 0; i < depSize; i++) {
        free(dep[i]);
        free(parent[i]);
    }

    free(dep);
    free(parent);
    return 0;
}
