#include "parse.h"
#include "uninstall.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define BASE_DIR "/etc/centaur/packages/"


int install(char package[], char parent[]) {
    char scriptDirectory[200];
    snprintf(scriptDirectory, sizeof(scriptDirectory), "%s/scripts/%s", BASE_DIR, package);

    char scriptPath[300];
    char latest[100];
    get_latest(scriptPath, latest, sizeof(latest));

    snprintf(scriptPath, sizeof(scriptPath), "%s/%s", scriptDirectory, latest);

    printf("%s", scriptPath);

    char installPath[200];
    snprintf(installPath, sizeof(installPath), "%s/installed/%s", BASE_DIR, package);

    if (access(installPath, F_OK) == 0) {
        printf("%s\n", "Package already installed");
        return 1;
    } 

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
