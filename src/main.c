#include <stdio.h>
#include <string.h>
#include <unistd.h> // to check for root permissions and checks if file exists
#include <stdlib.h>
#include <stdbool.h>
#include <sys/stat.h>

#include "install.h"
#include "uninstall.h"
#include "parse.h"
#include "license.h"
#include "list.h"
#include "dependencies.h"
#include "clean.h"
#include "sync.h"
#include "update.h"

void print_usage() {
    // error message
    printf("%s\n%s\n\n%s\n%s\n",
           "\033[31mERROR: Please enter command and related arguments.\033[0m",
           "\033[34mUSAGE: centaur [COMMAND] [ARGUMENTS]",
           "see the wiki for more information:",
           "https://github.com/Chiron8/centaur/wiki\033[0m");
}

int check_root() {
    // checks if program has root permissions
    if (geteuid() != 0) {
        printf("%s\n", "\033[31mPlease run as root.\033[0m");
        exit(1);
    }
    return 0;
}

int main(int argc, char *argv[]) {
    // ANSI ESCAPE CODES: 31 - RED, 32 - GREEN, 33 - YELLOW, 34 - BLUE
    // printf("\033[XXmthis is some text\033[0m")
    
    const char *dir_name = "/tmp/centaur";
    int status = mkdir(dir_name, 0755);

    if (status != 0) {
        perror("Error creating /tmp/centaur");
    }

    if (argc < 2) {
        print_usage();
        return 1;
    }

    int check_hash;

    // instruct var
    char instruction[50];
    strcpy(instruction, argv[1]);

    // list or license doesn't need package name or root so call before other checks
    if (strcmp(instruction, "list") == 0) {
        list();
        return 0;
    }
    else if (strcmp(instruction, "license") == 0) {
        license();
        return 0;
    }

    check_root();

    char package[150];

    // clean needs root but doesn't have any extra args
    if (strcmp(instruction, "clean") == 0) {
        clean();
        return 0;
    }

    if (strcmp(instruction, "sync") == 0) {
        centaur_sync();
        return 0;
    }

    if (strcmp(instruction, "update") == 0) {
        if (argc == 3 && strcmp(argv[2], "no_hash_check") == 0) {
            check_hash = 0;
        }
        else {
            check_hash = 1;
        }
        update(check_hash);
        return 0;
    }

    if (argc < 3) {
        print_usage();
        return 1;
    }

    snprintf(package, sizeof(package), "%s", argv[2]);

    // why can't c do switch case for strings >:(
    if (strcmp(instruction, "install") == 0) {
        if (strcmp(package, "HASHES") == 0) {
            perror("son you can't install the HASHES directory 🥀✌️");
            exit(1);
        }

        if (argc == 4 && strcmp(argv[3], "no_hash_check") == 0) {
            check_hash = 0;
        }
        else {
            check_hash = 1;
        }
        // get all dependencies...
        Dependency *deps = NULL;
        size_t total_deps = 0;

        //call main func in dependencies.c
        deps = getDependencies(package, NULL, deps, &total_deps);
        //array needs to be reversed so parent directory is installed first
        reverseDependencies(deps, total_deps);


        printf("%s\n", "\033[34mInstalling packages:\033[0m");
        for (size_t i = 0; i < total_deps; i++) {
            printf("\033[32m  - %s\033[0m\n", deps[i].dep);
        }

        if (argc > 3 && strcmp(argv[3], "pretend") == 0) {
            return 1;
        }

        for (size_t i = 0; i < total_deps; i++) {
            // check if package already exists
            install(deps[i].dep, deps[i].parent, check_hash);
        }
        freeDependencies(deps, total_deps);
        return 0;
    }
    if (strcmp(instruction, "uninstall") == 0) {
        // force uninstall if force keyword is passed
        int force = (argc == 4 && strcmp(argv[3], "force") == 0);
        uninstall(package, force);
        return 0;
    } 
    printf("\033[31m%s %s %s\033[0m\n", "ERROR: instruction", instruction, "does not exist!");
    return 1;
}
