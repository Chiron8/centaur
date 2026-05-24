#include <stdio.h>
#include <string.h>
#include <dirent.h>

#include "uninstall.h"
#include "parse.h"
#include "install.h"

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

int add_to_array(char **arr, int *arrCap, int *arrSize, char* element) {
    if (arrSize >= arrCap) {
        arrCap *= 2;
        char **temp = realloc(arr, arrCap * sizeof(char*));
        if (temp == NULL) {
            perror("Coulnd not create tmp array, realloc failed");
            return 1;
        }
        array_of_strings = temp;
    }

    array_of_strings[size] = malloc(strlen(element));
    size++;
    return 0;
}

int update() {
    printf("%s\n", "Updating packages:");
    int arrCap = 100;
    int arrSize = 0;
    char **arr = malloc(capacity * sizeof(char*));
    if (arr == NULL) {
        perror("could not create array, malloc failed");
        exit(1);
    }

    struct dirent *de;
    DIR *dr = opendir(INSTALLED_PATH);

    if (dr == NULL) {
        perror("Could not open installed script directory");
        exit(1);
    }

    while ((de = readdir(dr)) != NULL) {
        if (needs_update(remove_version(de->d_name), de->d_name)) {
            add_to_array(arr, *arrCap, *arrSize, de->d_name);
            printf("- %s\n", de->d_name);
        }
    }

    for (int i = 0; i < sizeof(arr)/sizeof(char*); i++) {
        install()
    }
}
