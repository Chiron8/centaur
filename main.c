// probably GNU GPL v3

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Aim
// Program that can:
// - Read install script DONE!
// - Run commands in the install script DONE!
// - Create file to show what was installed
// 
// I will just do an example with echo commands instead of actually installing something

int concat_realloc(size_t current_len, size_t line_len, size_t *capacity, char **command) {
    // check if command needs more mem
    if (current_len + line_len + 8 > *capacity) {
        size_t new_capacity = *capacity * 2;
        char *temp = realloc(*command, new_capacity);
        if (temp == NULL) { // out of memory
            return -1;
        }

        *command = temp;
        *capacity = new_capacity;
    }
    return 0;
}

int cat_command(bool new_command, size_t *current_len, size_t line_len, size_t *capacity, char **command, char line[]) {
    if (concat_realloc(*current_len, line_len, capacity, command) == -1) {
        return -1;
    }

    if (new_command == true) {
        strcat(*command, " && ");
        strcat(*command, line);
        *current_len += line_len+4;
    } else {
        // concatinate command + line
        strcat(*command, " ");
        strcat(*command, line);
        *current_len += line_len+1;
    } 
    return 0;
}

int main(int argc, char *argv[]) {
    // check if arg was passed
    if (argc == 1) {
        printf("Please enter package to install :)\n");
        return -1;
    }

    // package var
    char package[100];
    strcpy(package, argv[1]);
    strcat(package, ".centaur");

    // load package file
    FILE *fptr = fopen(package, "r");

    if (fptr == NULL) {
        printf("%s%s%s\n", "Unable to locate install script ", package, ". Perhaps you spelt it wrong?");
        return -1;
    }

    // vars to make sure we have enough memory (I'm so cool)
    char line[101];
    size_t capacity = 128;
    size_t current_len = 0;
    char *command = malloc(capacity);
    bool first = true;
    command[0] = '\0';

    while (fgets(line, sizeof(line), fptr)) { // read lines one at a time
        line[strcspn(line, "\n")] = '\0'; // get rid of new line char
        size_t line_len = strlen(line);

        // blank line
        if (strlen(line) == 0 || first == true) {
            cat_command(false, &current_len, line_len, &capacity, &command, line);
            first = false;
            continue;
        } else if (line[0] == '#') { // comment
            printf("%s", "hello");
            continue;
        }

        cat_command(true, &current_len, line_len, &capacity, &command, line);
    }
    // execute last command
    system(command);

    fclose(fptr);
    return 0;
}
