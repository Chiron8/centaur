#include "parse.h"
#include "uninstall.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define BASE_DIR "/etc/centaur/packages/"

int install(char package[], char parent[]) {
    char installPath[200];
    snprintf(installPath, sizeof(installPath), "%s/installed/%s", BASE_DIR, package);

    if (access(installPath, F_OK) == 0) {
        printf("%s\n", "Package already installed");
        return 1;
    } 

    char scriptPath[200];
    snprintf(scriptPath, sizeof(scriptPath), "%s/scripts/%s", BASE_DIR, package);

    if (read_execute(scriptPath, false) == 1) {
        printf("%s %s%s\n", "ERROR: could not install package ", package, ", trying clean-up...");
        uninstall(package, true);
    }

    // write db file if file does not already exist
    if (access(installPath, F_OK) != 0) {
        FILE *fptr = fopen(installPath, "w");
        if (fptr) {
            fprintf(fptr, "%s\n", parent);
            fclose(fptr);
        }
    } 
    return 0;
}
