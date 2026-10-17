#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>

#include "sha256.h"
#include "parse.h"
#include "uninstall.h"
#include "curl_hash.h"
#include "dependencies.h"
#include "colours.h"
#include "sync.h"

#define BASE_DIR "/var/db/centaur/packages"

int prepare_install(char installer_path[], char script_file[], char build_dir[], bool force) {
    FILE *installer_fptr = fopen(installer_path, "w");
    if (!installer_fptr) {
        perror("Could not create installer");
        exit(1);
    }
    FILE *script_fptr = fopen(script_file, "r");
    if (!script_fptr) {
        perror("Could not open package script file");
        exit(1);
    }
    char line[512];
    while (fgets(line, sizeof(line), script_fptr) && strcmp(line, "[install]\n") != 0) {
        // skip until [install]
    }
    while (fgets(line, sizeof(line), script_fptr) && strcmp(line, "[/install]\n") != 0) {
        fprintf(installer_fptr, "%s", line);
    }
    fclose(installer_fptr);
    fclose(script_fptr);
    return 0;
}

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

int run_bwrap(char staging_dir[], char installer_script[]) {
    char work_dir[512];
    snprintf(work_dir, sizeof(work_dir), "%s/work", staging_dir); // tmp dir for overlayfs
    char *args[] = {
        BWRAP_PATH, // bwrap executable defined by CMAKE ignore this error
        "--unshare-all", // keep installer away from host system except for networking (line below)
        "--share-net",
        "--ro-bind", "/", "/", // keep root fs as ro so host toolchain is still visible
        "--overlay", staging_dir, work_dir, "/usr", // create overlayFS to make toolchain useable
        "--", // separate args from actual command
        "/bin/sh", installer_script,
        NULL // i think this means the end of the command????
    };

    pid_t pid = fork(); // create new process
    if (pid < 0) {
        perror("Failed to fork");
        exit(1);
    }
    if (pid == 0) {
        execvp("bwrap", args);
        perror("Failed to execute installer"); // if execvp() returns, there was an error
        exit(1);
    }
    else {
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) { // i didn't realise there a term called wife too awww
            return WEXITSTATUS(status);
        }
        return -1;
    }
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

    char staging_directory[256];
    snprintf(staging_directory, sizeof(staging_directory), "%s%s%s", "/tmp/centaur/", package_name, "STAGING");

    char installer_path[512];
    snprintf(installer_path, sizeof(installer_path), "%s/centaur_installer.sh", source_directory);

    run_bwrap(staging_directory, installer_path);

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
