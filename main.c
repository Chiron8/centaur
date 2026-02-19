// probably GNU GPL v3

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h> // to check for root permissions and checks if file exists

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
    if (currentLen + lineLen + 8 > *capacity) {
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

int read_execute(char file[]) {

    // load file
    FILE *fptr = fopen(file, "r");

    if (fptr == NULL) {
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
    system(command);
    free(command);
    fclose(fptr);
    return 0;
}

int install(char package[]) {
    char installPath[150];
    snprintf(installPath, sizeof(installPath), "%s/installed/%s", BASE_DIR, package);

    if (access(installPath, F_OK) == 0) {
        printf("%s\n", "Package already installed. Please uninstall first.");
        exit(1);
    } 

    char scriptPath[200];
    snprintf(scriptPath, sizeof(scriptPath), "%s/scripts/%s", BASE_DIR, package);

    read_execute(scriptPath);

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

int uninstall(char package[]) {
    char installPath[150];
    snprintf(installPath, sizeof(installPath), "%s/installed/%s.centaur", BASE_DIR, package);

    if (access(installPath, F_OK) != 0) {
        printf("%s %s %s\n", "Package", package, "not installed.");
        exit(1);
    } 

    char uninstallPath[150];
    snprintf(uninstallPath, sizeof(uninstallPath), "%s/uninstall/%s.centaur", BASE_DIR, package);

    read_execute(uninstallPath);

    if (remove(installPath) | remove(uninstallPath)) { // ensure both run
        printf("%s\n", "Error removing files!");
    }

    return 0;
}

int main(int argc, char *argv[]) {
    check_root();
    // check if arg was passed
    if (argc != 3) {
        printf("%s\n", "ERROR: enter both command and package name");
        printf("%s\n", "USAGE: centaur [COMMAND] [PACKAGE]");
        return 1;
    }

    // instruct var
    char instruction[50];
    strcpy(instruction, argv[1]);

    char package[150];
    snprintf(package, sizeof(package), "%s.centaur", argv[2]);

    // why can't c do switch case for strings >:(
    if (strcmp(instruction, "install") == 0) {
        install(package);
    }
    else if (strcmp(instruction, "uninstall") == 0) {
        uninstall(package);
    }
    else {
        printf("%s %s %s\n", "ERROR: instruction", instruction, "does not exist!");
        return 1;
    }
    return 0;
}
