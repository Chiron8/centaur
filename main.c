// probably MIT license. Maybe GNU GPL v3 but probably not

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Aim
// Program that can:
// - Read install script DONE!
// - Run commands in the install script DONE!
// - Create file to show what was installed
// 
// I will just do an example with echo commands instead of actually installing something

int concat_realloc(size_t current_len, size_t line_len, size_t *capacity, char **command) {
    // check if command needs more mem
    if (current_len + line_len + 2 > *capacity) {
        size_t new_capacity = *capacity * 2;
        char *temp = realloc(*command, new_capacity);
        if (temp == NULL) { // out of memory
            free(*command);
            printf("Out of memory!");
            return -1;
        }

        *command = temp;
        *capacity = new_capacity;
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
    command[0] = '\0';

    while(fgets(line, sizeof(line), fptr)) { // read lines one at a time
        line[strcspn(line, "\n")] = '\0'; // get rid of new line char
        size_t line_len = strlen(line);

        // blank line
        if (strlen(line) == 0) {
            system(command);
            printf("%s\n", command);
            command[0] = '\0';
            current_len = 0;
            continue;
        }

        // ignore comments
        if (line[0] == '#') {
            continue;
        }

        // realloc memory if needed
        if (concat_realloc(current_len, line_len, &capacity, &command) == -1) {
            return -1;
        }

        // concatinate command + line
        strcat(command, " ");
        strcat(command, line);
        current_len += line_len+1;
    }
    // execute last command
    system(command);

    fclose(fptr);
    return 0;
}
