#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "sha256.h"
#include "parse.h"
#include "uninstall.h"
#include "curl_hash.h"
#include "dependencies.h"
#include "colours.h"
#include "sync.h"

#define BASE_DIR "/var/db/centaur/packages"

int download_tarball(char install_file[], char package_name[]) {
    char *source = get_meta_variable_value(install_file, "source");
    char *checksum = get_meta_variable_value(install_file, "checksum");
    char download_file[256];
    snprintf(download_file, sizeof(download_file), "%s%s%s", "/tmp/centaur/", package_name, "TARBALL");
    curl_tarball(source, download_file);
    if (strcmp(calculate_hash(download_file), checksum) != 0) {
        printf("%s%s%s\n", ANSI_RED, "TARBALL CHECKSUMS DO NOT MATCH!", ANSI_CLEAR);
        free(source);
        free(checksum);
        exit(1);
    }
    char source_directory[256];
    snprintf(source_directory, sizeof(source_directory), "%s%s%s", "/tmp/centaur/", package_name, "SOURCE");

    extract_tar(download_file, source_directory);

    free(source);
    free(checksum);
    return 0;
}

int add_deps_to_file(char package_name[], char install_file[]) {
    Dependency *deps = NULL;
    size_t deps_size = 0;

    deps = get_dependencies(package_name, NULL, deps, &deps_size);

    FILE *install_ptr = fopen(install_file, "a");
    if (install_ptr == NULL) {
        printf("%s%s%s\n", ANSI_RED, "Could not open file to add dependencies to parent install file.", ANSI_CLEAR);
        free_dependencies(deps, deps_size);
        exit(1);
    }

    fprintf(install_ptr, "%s\n", "=== STUFF ABOVE = PARENT PACKAGES, STUFF BELOW = DEPENDENCIES ===");
    for (size_t i = 0; i < deps_size; i++) {
        if (deps[i].parent != NULL && strcmp(deps[i].parent, package_name) == 0) {
            fprintf(install_ptr, "%s\n", deps[i].dep);
        }
    }

    fclose(install_ptr);
    free_dependencies(deps, deps_size);
    return 0;
}

int install(char package_name[], char parent[], int check_hashes) {
    // package_name = something-1.2.3
    char script_directory[256]; // package directory (not specific version)
    snprintf(script_directory, sizeof(script_directory), "%s/scripts/%s", BASE_DIR, package_name);

    char script_path[512]; // package script file (includes version)
    char latest[128];

    int code = get_latest(script_directory, latest, sizeof(latest)); // latest holds the output not code
    if (code == -1) {
        // could not get latest version
        printf("%s\n", "\033[31mSomething went wrong getting latest (install.c)");
        printf("%s\033[0m\n", script_directory);
        exit(1);
    }
    snprintf(script_path, sizeof(script_path), "%s/%s", script_directory, latest);

    char install_path[200];
    snprintf(install_path, sizeof(install_path), "%s/installed/%s", BASE_DIR, latest);

    install_path[strlen(install_path)-8] = '\0';

    if (access(install_path, F_OK) == 0) {
        // NEED TO CHANGE LOGIC
        printf("%s\n", "\033[31mPackage already installed\033[0m");
        return 1;
    }

    if (check_hashes == 1) {
        // hash stuff here
        char localHash[512];
        snprintf(localHash, sizeof(localHash), "%s", calculate_hash(script_path));
        if (strcmp(download_hash(latest, package_name), localHash) != 0) {
            perror("Hashes for your install file DO NOT MATCH with the known file!!!");
            exit(1);
        }
    }

    download_tarball(script_path, package_name);

    char *extracted_folder_name = get_meta_variable_value(script_path, "source_directory");

    char source_directory[256];
    snprintf(source_directory, sizeof(source_directory), "%s%s%s/%s", "/tmp/centaur/", package_name, "SOURCE", extracted_folder_name);

    if (read_execute(script_path, source_directory, false, false) == 1) {
        printf("\033[31m%s %s%s\n", "ERROR: could not install package ", package_name, ", trying clean-up...\033[0m");
        uninstall(package_name, true);
    }

    FILE *install_fptr = fopen(install_path, "w");

    // is a dep of blah
    if (install_fptr && parent != NULL) {
        char parent_latest[100];
        char parent_directory[300];

        snprintf(parent_directory, sizeof(parent_directory), "%s/scripts/%s", BASE_DIR, parent); 
        int parentCode = get_latest(parent_directory, parent_latest, sizeof(parent_latest));

        if (parentCode == -1) {
            perror("could not get latest parent");
            exit(1);
        }

        fprintf(install_fptr, "%s\n", parent_latest);
        fclose(install_fptr);
    }

    FILE *fptr = fopen(script_path, "r");

    if (fptr == NULL) {
        perror("Could not open script_path");
        fclose(fptr);
        exit(1);
    }

    char *createBlankUninstallFile = get_meta_variable_value(script_path, "create_blank_uninstall_file");

    char uninstall_file[256];
    snprintf(uninstall_file, sizeof(uninstall_file), "%s/uninstall/%s", BASE_DIR, latest);

    uninstall_file[strlen(uninstall_file)-8] = '\0';

    if (createBlankUninstallFile) {
        FILE *fptr = fopen(uninstall_file, "w");
        fclose(fptr);
    } else {
        FILE *fptr = fopen(script_path, "r");
        char line[512];
        while (fgets(line, sizeof(line), fptr) && strcmp(line, "[uninstall]\n") != 0) {
            // skip until [uninstall]
        }
        FILE *uninstall_fptr = fopen(uninstall_file, "w");
        while (fgets(line, sizeof(line), fptr) && strcmp(line, "[/uninstall]\n") != 0) {
            fprintf(uninstall_fptr, "%s", line);
        }
        fclose(fptr);
        fclose(uninstall_fptr);
    }

    free(createBlankUninstallFile);

    add_deps_to_file(package_name, install_path);
    return 0;
}
