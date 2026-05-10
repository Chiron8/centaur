#include "parse.h"
#include "uninstall.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define BASE_DIR "/etc/centaur/packages"

int add_deps_to_file(char scriptFile[], char installFile[]) {
    char line[150];
    FILE *script_ptr = fopen(scriptFile, "r");
    FILE *install_ptr = fopen(installFile, "a");

    if (script_ptr == NULL || install_ptr == NULL) {
        printf("%s\n", "Could not open file to add dependencies to parent install file.");
        exit(1);
    }

    fprintf(install_ptr, "%s\n", "=== STUFF ABOVE = PARENT PACKAGES, STUFF BELOW = DEPENDENCIES ===");

    while (fgets(line, sizeof(line), script_ptr)) {
        if (line[0] == '=') {
            break;
        }
        fprintf(install_ptr, "%s\n", line);
    }
    fclose(script_ptr);
    fclose(install_ptr);
    return 0;
}

int install(char package[], char parent[]) {
    // package = something-1.2.3.centaur
// awesome!!!
    char scriptDirectory[200]; // package directory (not specific version)
    snprintf(scriptDirectory, sizeof(scriptDirectory), "%s/scripts/%s", BASE_DIR, package);

    char scriptPath[300]; // package script file (includes version)
    char latest[100];
    int code = get_latest(scriptDirectory, latest, sizeof(latest));

    if (code == -1) {
        // could not get latest version
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

    if (parent == NULL) {
        char world_file[512];
        snprintf(world_file, sizeof(world_file), "%s%s", "/etc/centaur/packages/world/", latest);
        FILE *fptr = fopen(world_file, "w");
        fclose(fptr);
    }

    // is a dep of blah
    if (fptr && parent != NULL) {
        char parentLatest[100];
        char parentDirectory[300];

        snprintf(parentDirectory, sizeof(parentDirectory), "%s/scripts/%s", BASE_DIR, parent); 
        int parentCode = get_latest(parentDirectory, parentLatest, sizeof(parentLatest));

        if (parentCode == -1) {
            perror("could not get latest parent");
            exit(1);
        }

        fprintf(fptr, "%s\n", parentLatest);
        fclose(fptr);
    }
     
    add_deps_to_file(scriptPath, installPath);
    return 0;
}
