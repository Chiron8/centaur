// probably GNU GPL v3

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h> // to check for root permissions and checks if file exists
#include <dirent.h>

// Aim
// Program that can:
// - Read install script DONE!
// - Run commands in the install script DONE!
// - Create file to show what was installed DONE!
// 
// I will just do an example with echo commands instead of actually installing something

#define BASE_DIR "/etc/centaur/packages"

int concat_realloc(size_t currentLen, size_t lineLen, size_t *capacity, char **command) {
    // check if command needs more mem
    while (currentLen + lineLen + 8 > *capacity) {
        size_t newCapacity = *capacity * 2;
        char *temp = realloc(*command, newCapacity);
        if (temp == NULL) { // out of memory
            return 1;
        }

        *command = temp;
        *capacity = newCapacity;
    }
    return 0;
}

int cat_command(bool newCommand, size_t *currentLen, size_t lineLen, size_t *capacity, char **command, char line[]) {
    if (concat_realloc(*currentLen, lineLen, capacity, command) == 1) {
        return 1;
    }

    if (newCommand == true) {
        strcat(*command, " && ");
        strcat(*command, line);
        *currentLen += lineLen+4;
    } 
    else {
        // concatinate command + line
        strcat(*command, " ");
        strcat(*command, line);
        *currentLen += lineLen+1;
    } 
    return 0;
}

int check_root() {
    if (geteuid() != 0) {
        printf("%s\n", "Please run as root.");
        exit(1);
    }
    return 0;
}

int read_execute(char file[], bool force) {

    // load file
    FILE *fptr = fopen(file, "r");

    if (fptr == NULL && force != true) {
        printf("%s%s%s\n", "Unable to locate install file ", file, ". Perhaps you spelt it wrong?");
        exit(1);
    }
    char line[256];
    size_t capacity = 512;
    size_t currentLen = 0;
    char *command = malloc(capacity);
    command[0] = '\0';

    bool prevBlank = false;

    while (fgets(line, sizeof(line), fptr)) { // read lines one at a time
        line[strcspn(line, "\n")] = '\0'; // get rid of new line char
        size_t lineLen = strlen(line);

        if (lineLen == 0) {
            prevBlank = true;
            continue;
        }

        if (line[0] == '#') {
            continue;
        }

        // blank line
        if (prevBlank) {
            cat_command(true, &currentLen, lineLen, &capacity, &command, line);
        } 
        else { 
            cat_command(false, &currentLen, lineLen, &capacity, &command, line);
        }
        prevBlank = false;
    }
    if (system(command) > 0) {
        return 1;
    }
    system(command);
    free(command);
    fclose(fptr);
    return 0;
}

int uninstall(char package[], bool force) {
    char installPath[200];
    snprintf(installPath, sizeof(installPath), "%s/installed/%s", BASE_DIR, package);

    if (access(installPath, F_OK) != 0) {
        printf("%s %s %s\n", "Package", package, "not installed.");
        exit(1);
    } 

    char uninstallPath[200];
    snprintf(uninstallPath, sizeof(uninstallPath), "%s/uninstall/%s", BASE_DIR, package);

    read_execute(uninstallPath, force);

    if (remove(installPath) | remove(uninstallPath)) { // ensure both run
        if (force != true) {
            printf("%s\n", "Error removing files!");
            exit(1);
        }
    }

    return 0;
}

int install(char package[]) {
    char installPath[200];
    snprintf(installPath, sizeof(installPath), "%s/installed/%s", BASE_DIR, package);

    if (access(installPath, F_OK) == 0) {
        printf("%s\n", "Package already installed. Please uninstall first.");
        exit(1);
    } 

    char scriptPath[200];
    snprintf(scriptPath, sizeof(scriptPath), "%s/scripts/%s", BASE_DIR, package);

    if (read_execute(scriptPath, false) == 1) {
        printf("%s %s%s\n", "ERROR: could not install package ", package, ", trying clean-up...");
        uninstall(package, true);
    }

    // write db file if file does not already exist
    if (access(installPath, F_OK) != 0) {
        FILE *fptr = fopen(installPath, "w");
        if (fptr) {
            fprintf(fptr, "Installed: %s\n", package);
            fclose(fptr);
        }
    } 
    return 0;
}

int list_iscentaur(char file[]) { // list() helper, checks if centaur file
    size_t len = strlen(file);
    if (len < 8) {
        return 0;
    }
    return strcmp(file + len - 8, ".centaur") == 0; // checks if file ends with .centaur
}

int list() {
    printf("%s\n\n", "All packages currently installed are:");
    // gracefully stolen from a tutorialspoint article :)
    DIR *dr;
    struct dirent *en;
    dr = opendir("/etc/centaur/packages/installed/");
    while ((en = readdir(dr)) != NULL) {
        /* weird thing to exclude last 8 chars (removes .centaur)
        also check if file is not .. or .*/

        if (list_iscentaur(en->d_name)) {
            size_t len = strlen(en->d_name);
            printf("%.*s\n", (int)(len-8), en->d_name);
        } 
    }
    closedir(dr);
    return 0;
}

int main(int argc, char *argv[]) {
    // instruct var
    char instruction[50];
    strcpy(instruction, argv[1]);

    // list doesn't need package name so call before other checks
    if (strcmp(instruction, "list") == 0) {
        list();
        return 0;
    }

    if (argc < 3) {
        printf("%s\n", "ERROR: enter both command and package name");
        printf("%s\n", "USAGE: centaur [COMMAND] [PACKAGE]");
        return 1;
    }

    check_root();

    char package[150];
    snprintf(package, sizeof(package), "%s.centaur", argv[2]);

    // why can't c do switch case for strings >:(
    if (strcmp(instruction, "install") == 0) {
        install(package);
    }
    else if (strcmp(instruction, "uninstall") == 0) {
        uninstall(package, (argc == 4 && strcmp(argv[3], "force") == 0)); // check if force
    } 
    else {
        printf("%s %s %s\n", "ERROR: instruction", instruction, "does not exist!");
        return 1;
    }
    return 0;
}
