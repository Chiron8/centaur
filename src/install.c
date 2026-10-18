#include <bits/types/idtype_t.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>

#include "sha256.h"
#include "parse.h"
#include "uninstall.h"
#include "curl_hash.h"
#include "dependencies.h"
#include "colours.h"
#include "sync.h"

#define BASE_DIR "/var/db/centaur/packages"

void manifest_helper(char dr[], FILE *fptr, char root_prefix[]) {
    DIR *dir = opendir(dr);
    if (!dir) return;

    struct dirent *de;
    while ((de = readdir(dir)) != NULL) {
        if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0) {
            continue;
        }
        char sub_path[1024];
        char new_root_dir[1024];
        snprintf(sub_path, sizeof(sub_path), "%s/%s", dr, de->d_name);
        snprintf(new_root_dir, sizeof(new_root_dir), "%s/%s", root_prefix, de->d_name);

        struct stat st;
        if (lstat(sub_path, &st) < 0) continue;
        fprintf(fptr, "%s\n", new_root_dir);
        if (S_ISDIR(st.st_mode) && !S_ISLNK(st.st_mode)) {
            manifest_helper(sub_path, fptr, new_root_dir);
        }
    }
    closedir(dir);
}

int generate_manifest(char staging_directory[], char full_package_name[]) {
    char manifest_file[512];
    snprintf(manifest_file, sizeof(manifest_file), "%s%s%s", "/var/db/centaur/packages/installed/", full_package_name, ".manifest");
    FILE *fptr = fopen(manifest_file, "w");
    if (!fptr) {
        perror("Could not create manifest file");
        exit(1);
    }

    char etc_dir[256];
    char usr_dir[256];
    char opt_dir[256];

    snprintf(etc_dir, sizeof(etc_dir), "%s%s", staging_directory, "/etc");
    snprintf(usr_dir, sizeof(usr_dir), "%s%s", staging_directory, "/usr");
    snprintf(opt_dir, sizeof(opt_dir), "%s%s", staging_directory, "/opt");

    manifest_helper(etc_dir, fptr, "/etc");
    manifest_helper(usr_dir, fptr, "/usr");
    manifest_helper(opt_dir, fptr, "/opt");

    return 0;
}

void copy_file(char src[], char dst[]) {
    int source_fd = open(src, O_RDONLY);
    if (source_fd < 0) return;

    struct stat st;
    fstat(source_fd, &st);

    int dest_fd = open(dst, O_WRONLY | O_CREAT | O_TRUNC, st.st_mode & 0777);
    if (dest_fd < 0) {
        close(source_fd);
        return;
    }
    char buffer[8192];
    ssize_t n;
    while ((n = read(source_fd, buffer, sizeof(buffer))) > 0) {
        write(dest_fd, buffer, n);
    }
    close(source_fd);
    close(dest_fd);
}

void install_tree(char src_dir[], char dst_dir[]) {
    DIR *dir = opendir(src_dir);
    if (!dir) return;
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        char src_path[1024];
        char dst_path[1024];
        snprintf(src_path, sizeof(src_path), "%s/%s", src_dir, entry->d_name);
        snprintf(dst_path, sizeof(dst_path), "%s/%s", dst_dir, entry->d_name);

        struct stat st;
        if (stat(src_path, &st) < 0) continue;
        if (S_ISDIR(st.st_mode)) {
            mkdir(dst_path, 0755);
            install_tree(src_path, dst_path);
        } else if (S_ISREG(st.st_mode)) {
            copy_file(src_path, dst_path);
        }
    }
    closedir(dir);
}

void move_staging_to_root(char staging_directory[]) {
    char staging_etc[256];
    char staging_usr[256];
    char staging_opt[256];

    snprintf(staging_etc, sizeof(staging_etc), "%s%s", staging_directory, "/etc");
    snprintf(staging_usr, sizeof(staging_usr), "%s%s", staging_directory, "/usr");
    snprintf(staging_opt, sizeof(staging_opt), "%s%s", staging_directory, "/opt");

    install_tree(staging_etc, "/etc");
    install_tree(staging_usr, "/usr");
    install_tree(staging_opt, "/opt");
}

int mkdir_helper(char directory[]) {
    int mkdir_status = mkdir(directory, 0755);
    if (mkdir_status != 0) {
        perror("Error creating staging directory");
        exit(1);
    }
    return 0;
}

int prepare_install(char installer_path[], char script_file[]) {
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

int run_bwrap(char staging_dir[], char installer_script[], char source_directory[]) {
    // I hope you can forgive me using AI to generate these blocks
    char usr_stage[512], usr_work[512];
    char etc_stage[512], etc_work[512];
    char var_stage[512], var_work[512];
    char opt_stage[512], opt_work[512];

    snprintf(usr_stage, sizeof(usr_stage), "%s/usr", staging_dir);
    snprintf(usr_work, sizeof(usr_work), "%s/usr_work", staging_dir);
    snprintf(etc_stage, sizeof(etc_stage), "%s/etc", staging_dir);
    snprintf(etc_work, sizeof(etc_work), "%s/etc_work", staging_dir);
    snprintf(var_stage, sizeof(var_stage), "%s/var", staging_dir);
    snprintf(var_work, sizeof(var_work), "%s/var_work", staging_dir);
    snprintf(opt_stage, sizeof(opt_stage), "%s/opt", staging_dir);
    snprintf(opt_work, sizeof(opt_work), "%s/opt_work", staging_dir);

    mkdir_helper(usr_stage);   mkdir_helper(usr_work);
    mkdir_helper(etc_stage);   mkdir_helper(etc_work);
    mkdir_helper(var_stage);   mkdir_helper(var_work);
    mkdir_helper(opt_stage);   mkdir_helper(opt_work);

    char *args[] = {
        BWRAP_PATH,          // bwrap executable defined by CMAKE
        "--unshare-all",     // Isolate the installer from the host system
        "--share-net",       // Allow network access for downloading/fetching if needed
        "--ro-bind", "/", "/", // Keep root fs read-only so host toolchain & /bin/sh remain visible
        "--proc", "/proc",   // Essential kernel virtual filesystem
        "--dev", "/dev",     // Essential device pseudo-files

        // --- /usr Overlay ---
        "--overlay-src", "/usr",
        "--overlay", usr_stage, usr_work, "/usr",

        // --- /etc Overlay ---
        "--overlay-src", "/etc",
        "--overlay", etc_stage, etc_work, "/etc",

        // --- /var Overlay ---
        //"--overlay-src", "/var",
        //"--overlay", var_stage, var_work, "/var",

        // --- /opt Overlay ---
        "--overlay-src", "/opt",
        "--overlay", opt_stage, opt_work, "/opt",

        "--chdir", source_directory,
        "--",                // Separate bwrap flags from the actual command
        "/bin/sh", installer_script,
        NULL                 // Marks the end of the argument array
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

    mkdir_helper(staging_directory);

    char installer_path[512];
    snprintf(installer_path, sizeof(installer_path), "%s/centaur_installer.sh", source_directory);

    char latest_sub_centaur[256];
    snprintf(latest_sub_centaur, sizeof(latest_sub_centaur), "%s", latest);
    latest_sub_centaur[strlen(latest_sub_centaur)-8] = '\0';
    prepare_install(installer_path, script_path);
    run_bwrap(staging_directory, installer_path, source_directory);
    generate_manifest(staging_directory, latest_sub_centaur);
    move_staging_to_root(staging_directory);

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

    add_deps_to_file(package_name, install_path);
    return 0;
}
