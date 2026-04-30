#include "parse.h"
#include "uninstall.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define BASE_DIR "/etc/centaur/packages"


int install(char package[], char parent[]) {
    // package = something-1.2.3.centaur
    char scriptDirectory[200]; // package directory (not specific version)
    snprintf(scriptDirectory, sizeof(scriptDirectory), "%s/scripts/%s", BASE_DIR, package);

    char scriptPath[300]; // package script file (includes version)
    char latest[100];
    int code = get_latest(scriptDirectory, latest, sizeof(latest));

    if (code == -1) {
        // could not get latest version
        printf("%s\n", scriptDirectory);
        printf("%s\n", "Something went wrong :O");
        exit(1);
    }
    snprintf(scriptPath, sizeof(scriptPath), "%s/%s", scriptDirectory, latest);

    char installPath[200];
    snprintf(installPath, sizeof(installPath), "%s/installed/%s", BASE_DIR, latest);

    if (access(installPath, F_OK) == 0) {
        // NEED TO CHANGE LOGIC
        printf("%s\n", "Package already installed");
        return 1;
    } 

    if (read_execute(scriptPath, false, false) == 1) {
        printf("%s %s%s\n", "ERROR: could not install package ", package, ", trying clean-up...");
        uninstall(package, true);
    }

    FILE *fptr = fopen(installPath, "w");
    // is a dep of blah
    if (fptr && parent != NULL) {
        fprintf(fptr, "%s\n", parent);
        fclose(fptr);
    }
     
    return 0;
}
