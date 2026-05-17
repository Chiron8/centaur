#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "sha256.h"
#include "parse.h"
#include "uninstall.h"
#include "curl_hash.h"

#define BASE_DIR "/etc/centaur/packages"

char* get_cloud_hash(char latest[], char noversion[]) {
    download_hash(latest, noversion);
    char tmp_hash[256];
    snprintf(tmp_hash, sizeof(tmp_hash), "%s/%s%s", "/etc/centaur/tmp", noversion, latest);

    FILE *fptr = fopen(tmp_hash, "r");
    if (fptr == NULL) {
        perror("could not open tmp hash file");
        exit(1);
    }
    char line[512];

    if (fgets(line, 512, fptr) == NULL) {
        fclose(fptr);
        return NULL;
    }
    fclose(fptr);

    line[strcspn(line, "\r\n")] = 0;

    char *result = malloc(strlen(line)+1);
    if (result == NULL) {
        perror("malloc failed");
        exit(1);
    }

    strcpy(result, line);
    return result;
}

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
    char scriptDirectory[200]; // package directory (not specific version)
    snprintf(scriptDirectory, sizeof(scriptDirectory), "%s/scripts/%s", BASE_DIR, package);

    char scriptPath[300]; // package script file (includes version)
    char latest[100];
    int code = get_latest(scriptDirectory, latest, sizeof(latest)); // latest holds the output not code

    if (code == -1) {
        // could not get latest version
        printf("%s\n", "Something went wrong :O");
        exit(1);
    }
    snprintf(scriptPath, sizeof(scriptPath), "%s/%s", scriptDirectory, latest);

    // hash stuff here
    char localHash[512];
    snprintf(localHash, sizeof(localHash), "%s", calculate_hash(scriptPath));
    char cloudHash[512];
    snprintf(cloudHash, sizeof(cloudHash), "%s", get_cloud_hash(latest, package));
    if (strcmp(localHash, cloudHash) != 0) {
        perror("Hashes for your install file DO NOT MATCH with the known file!!!");
        exit(1);
    }

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
