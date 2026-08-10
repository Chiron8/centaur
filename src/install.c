#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "sha256.h"
#include "parse.h"
#include "uninstall.h"
#include "curl_hash.h"
#include "dependencies.h"

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

    line[strcspn(line, "\r\n")] = 0;

    char *result = malloc(strlen(line)+1);
    if (result == NULL) {
        perror("malloc failed");
        exit(1);
    }

    strcpy(result, line);
    fclose(fptr);
    return result;
}

int add_deps_to_file(char package[], char installFile[]) {
    Dependency *deps = NULL;
    size_t deps_size = 0;

    deps = getDependencies(package, NULL, deps, &deps_size);

    FILE *install_ptr = fopen(installFile, "a");
    if (install_ptr == NULL) {
        printf("\033[31mCould not open file to add dependencies to parent install file.\033[0m\n");
        for (size_t i = 0; i < deps_size; i++) {
            free(deps[i].dep);
            free(deps[i].parent);
        }
        free(deps);
        exit(1);
    }

    fprintf(install_ptr, "%s\n", "=== STUFF ABOVE = PARENT PACKAGES, STUFF BELOW = DEPENDENCIES");
    for (size_t i = 0; i < deps_size; i++) {
        if (deps[i].parent != NULL && strcmp(deps[i].parent, package) == 0) {
            fprintf(install_ptr, "%s\n", deps[i].dep);
        }
    }

    fclose(install_ptr);

    for (size_t i = 0; i < deps_size; i++) {
        free(deps[i].dep);
        free(deps[i].parent);
    }
    free(deps);
    return 0;
}

int install(char package[], char parent[], int check_hashes) {
    // package = something-1.2.3.centaur
    char scriptDirectory[256]; // package directory (not specific version)
    snprintf(scriptDirectory, sizeof(scriptDirectory), "%s/scripts/%s", BASE_DIR, package);

    char scriptPath[512]; // package script file (includes version)
    char latest[128];

    int code = get_latest(scriptDirectory, latest, sizeof(latest)); // latest holds the output not code
    if (code == -1) {
        // could not get latest version
        printf("%s\n", "\033[31mSomething went wrong getting latest (install.c)");
        printf("%s\033[0m\n", scriptDirectory);
        exit(1);
    }
    snprintf(scriptPath, sizeof(scriptPath), "%s/%s", scriptDirectory, latest);

    char uninstallFile[256];
    snprintf(uninstallFile, sizeof(uninstallFile), "%s/uninstall/%s", BASE_DIR, latest);

    FILE *fptr = fopen(scriptPath, "r");

    if (fptr == NULL) {
        perror("Could not open scriptPath");
        fclose(fptr);
        exit(1);
    }

    char line[256];
    bool inMeta = false;
    char createBlankUninstallFile[6] = "no";

    while (fgets(line, sizeof(line), fptr)) {
        if (strncmp(line, "[meta]", 6) == 0) {
            inMeta = true;
            continue;
        }

        if (strncmp(line, "[/meta]", 7) == 0) {
            break;
        }

        if (!inMeta) {
            continue;
        }

        if (strncmp(line, "create_blank_uninstall_file", 27) == 0) {
            char *eq = strchr(line, '=');
            if (!eq) {
                continue;
            }

            eq++;
            while (*eq == ' ') {
                eq++;
            }

            char *start = eq;
            if (*start == '"') {
                start++;
            }

            char *end = start + strlen(start) - 1;
            if (end >= start && *end == '"') {
                *end = '\0';
            }

            strncpy(createBlankUninstallFile, start, sizeof(createBlankUninstallFile) -1);
        }
    }

    if (strcmp(createBlankUninstallFile, "yes") == 0) {
        FILE *fptr = fopen(uninstallFile, "w");
        fclose(fptr);
    }

    fclose(fptr);

    if (check_hashes == 1) {
        // hash stuff here
        char localHash[512];
        snprintf(localHash, sizeof(localHash), "%s", calculate_hash(scriptPath));
        char cloudHash[512];
        snprintf(cloudHash, sizeof(cloudHash), "%s", get_cloud_hash(latest, package));
        if (strcmp(localHash, cloudHash) != 0) {
            printf("%s\n%s\n", localHash, cloudHash);
            perror("Hashes for your install file DO NOT MATCH with the known file!!!");
            exit(1);
        }
    }

    char installPath[200];
    snprintf(installPath, sizeof(installPath), "%s/installed/%s", BASE_DIR, latest);

    if (access(installPath, F_OK) == 0) {
        // NEED TO CHANGE LOGIC
        printf("%s\n", "\033[31mPackage already installed\033[0m");
        return 1;
    } 

    if (read_execute(scriptPath, false, false) == 1) {
        printf("\033[31m%s %s%s\n", "ERROR: could not install package ", package, ", trying clean-up...\033[0m");
        uninstall(package, true);
    }

    FILE *install_fptr = fopen(installPath, "w");

    /*if (parent == NULL) {
        char world_file[512];
        snprintf(world_file, sizeof(world_file), "%s%s", "/etc/centaur/packages/world/", latest);
        FILE *world_fptr = fopen(world_file, "w");
        fclose(world_fptr);
    }*/

    // is a dep of blah
    if (install_fptr && parent != NULL) {
        char parentLatest[100];
        char parentDirectory[300];

        // this is pretty cool!!!

        snprintf(parentDirectory, sizeof(parentDirectory), "%s/scripts/%s", BASE_DIR, parent); 
        int parentCode = get_latest(parentDirectory, parentLatest, sizeof(parentLatest));

        if (parentCode == -1) {
            perror("could not get latest parent");
            exit(1);
        }

        fprintf(install_fptr, "%s\n", parentLatest);
        fclose(install_fptr);
    }

    if (strcmp(createBlankUninstallFile, "yes") == 0) {
        FILE *fptr = fopen(uninstallFile, "w");
        fclose(fptr);
    }
    else {
        FILE *fptr = fopen(scriptPath, "r");
        while (fgets(line, sizeof(line), fptr) && strcmp(line, "[uninstall]\n") != 0) {
            // skip until [uninstall]
        }
        FILE *uninstall_fptr = fopen(uninstallFile, "w");
        while (fgets(line, sizeof(line), fptr) && strcmp(line, "[/uninstall]\n") != 0) {
            fprintf(uninstall_fptr, line);
        }
        fclose(fptr);
        fclose(uninstall_fptr);
    }
     
    add_deps_to_file(package, installPath);
    return 0;
}
