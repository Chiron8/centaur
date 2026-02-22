#include "parse.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define BASE_DIR "/etc/centaur/packages/"

int uninstall(char package[], bool force) {
    char installPath[200];
    snprintf(installPath, sizeof(installPath), "%s/installed/%s", BASE_DIR, package);

    if (access(installPath, F_OK) != 0) {
        printf("%s %s %s\n", "Package", package, "not installed.");
        exit(1);
    } 

    char uninstallPath[200];
    snprintf(uninstallPath, sizeof(uninstallPath), "%s/uninstall/%s", BASE_DIR, package);

    read_execute(uninstallPath, force);

    if (remove(installPath) | remove(uninstallPath)) { // ensure both run
        if (force != true) {
            printf("%s\n", "Error removing files!");
            exit(1);
        }
    }

    printf("%s %s\n", package, "uninstalled!");
    return 0;
}
