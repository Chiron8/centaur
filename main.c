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

int concat_realloc(size_t currentLen, size_t lineLen, size_t *capacity, char **command) {
    // check if command needs more mem
    if (currentLen + lineLen + 8 > *capacity) {
        size_t newCapacity = *capacity * 2;
        char *temp = realloc(*command, newCapacity);
        if (temp == NULL) { // out of memory
            return -1;
        }

        *command = temp;
        *capacity = newCapacity;
    }
    return 0;
}

int cat_command(bool newCommand, size_t *currentLen, size_t lineLen, size_t *capacity, char **command, char line[]) {
    if (concat_realloc(*currentLen, lineLen, capacity, command) == -1) {
        return -1;
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

int write_file(char path[], char package[]) {
    FILE *fptr;
    fptr = fopen(path, "w");

    if (fptr == NULL) {
        perror(path);
        return 1;
    }

    fprintf(fptr, "%s", package);
    fclose(fptr);

    return 0;
}

int main(int argc, char *argv[]) {
    //check for root permissions
    if (geteuid() != 0) {
        printf("%s\n", "Please run as root.");
        return -1;
    }

    // check if arg was passed
    if (argc == 1) {
        printf("%s\n", "Please enter package to install :)");
        return -1;
    }

    // package var
    char package[100];
    strcpy(package, argv[1]);
    strcat(package, ".centaur");

    char path[150] = "/etc/centaur/packages/installed/";
    strcat(path, package);

    if (access(path, F_OK) != 0) {
        printf("%s\n", "Package already intalled, installing again.");
    } 

    // load package file
    FILE *fptr = fopen(package, "r");

    if (fptr == NULL) {
        printf("%s%s%s\n", "Unable to locate install script ", package, ". Perhaps you spelt it wrong?");
        return -1;
    }

    // vars to make sure we have enough memory (I'm so cool)
    char line[101];
    size_t capacity = 128;
    size_t currentLen = 0;
    char *command = malloc(capacity);
    bool first = true;
    command[0] = '\0';

    while (fgets(line, sizeof(line), fptr)) { // read lines one at a time
        line[strcspn(line, "\n")] = '\0'; // get rid of new line char
        size_t lineLen = strlen(line);

        // blank line
        if (strlen(line) == 0 || first == true) {
            cat_command(false, &currentLen, lineLen, &capacity, &command, line);
            first = false;
        } 
        else if (line[0] != '#') { // anything but comment
            cat_command(true, &currentLen, lineLen, &capacity, &command, line);
        }
    }
    // execute last command
    system(command);

    fclose(fptr);

    // write db file if file does not already exist
    if (access(path, F_OK) != 0) {
        write_file(path, package);
    } 

    return 0;
}
