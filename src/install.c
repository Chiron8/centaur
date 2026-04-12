#include "parse.h"
#include "uninstall.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define BASE_DIR "/etc/centaur/packages"


int install(char package[], char parent[]) {
    char scriptDirectory[200];
    snprintf(scriptDirectory, sizeof(scriptDirectory), "%s/scripts/%s", BASE_DIR, package);

    char scriptPath[300];
    char latest[100];
    int code = get_latest(scriptDirectory, latest, sizeof(latest));

    if (code == -1) {
        printf("%s\n", scriptDirectory);
        printf("%s\n", "Something went wrong :O");
        exit(1);
    }

    snprintf(scriptPath, sizeof(scriptPath), "%s/%s", scriptDirectory, latest);


    char installPath[200];
    snprintf(installPath, sizeof(installPath), "%s/installed/%s", BASE_DIR, latest);

    if (access(installPath, F_OK) == 0) {
        printf("%s\n", "Package already installed");
        exit(1);
    } 

    if (read_execute(scriptPath, false) == 1) {
        printf("%s %s%s\n", "ERROR: could not install package ", package, ", trying clean-up...");
        uninstall(package, true);
    }

    FILE *fptr = fopen(installPath, "w");
    if (fptr) {
        fprintf(fptr, "%s\n", parent);
        fclose(fptr);
    }
     
    return 0;
}
