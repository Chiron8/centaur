// probably GNU GPL v3

#include <stdio.h>
#include <string.h>
#include <unistd.h> // to check for root permissions and checks if file exists
#include <stdlib.h>
#include <stdbool.h>

#include "install.h"
#include "uninstall.h"
#include "parse.h"
#include "license.h"
#include "list.h"
#include "dependencies.h"

void print_usage() {
    printf("%s\n%s\n\n%s\n%s\n",
           "ERROR: Please enter command and related arguments.",
           "USAGE: centaur [COMMAND] [ARGUMENTS]",
           "see the wiki for more information:",
           "https://github.com/Chiron8/centaur/wiki");
}

int check_root() {
    if (geteuid() != 0) {
        printf("%s\n", "Please run as root.");
        exit(1);
    }
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc == 1) {
        print_usage();
        return 1;
    }

    // instruct var
    char instruction[50];
    strcpy(instruction, argv[1]);

    // list doesn't need package name so call before other checks
    if (strcmp(instruction, "list") == 0) {
        list();
        return 0;
    }
    else if (strcmp(instruction, "license") == 0) {
        license();
        return 0;
    }

    if (argc < 3) {
        print_usage();
        return 1;
    }

    check_root();

    char package[150];
    //snprintf(package, sizeof(package), "%s.centaur", argv[2]);
    snprintf(package, sizeof(package), "%s", argv[2]);
    // why can't c do switch case for strings >:(
    if (strcmp(instruction, "install") == 0) {
        // get all dependencies...
        Dependency *deps = NULL;
        size_t total_deps = 0;

        //call main func in dependencies.c
        deps = getDependencies(package, NULL, deps, &total_deps);
        //array needs to be reversed so parent directory is installed first
        reverseDependencies(deps, total_deps);

        printf("%s\n", "Installing packages:");
        for (size_t i = 0; i < total_deps; i++) {
            printf("  - %s\n", deps[i].dep);
        }

        for (size_t i = 0; i < total_deps; i++) {
            // check if package already exists
            install(deps[i].dep, deps[i].parent);
        }
        freeDependencies(deps, total_deps);
    }
    else if (strcmp(instruction, "uninstall") == 0) {
        int force = (argc == 4 && strcmp(argv[3], "force") == 0);
        uninstall(package, force);
    } 
    else {
        printf("%s %s %s\n", "ERROR: instruction", instruction, "does not exist!");
        return 1;
    }
    return 0;
}
