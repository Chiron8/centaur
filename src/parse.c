#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

    free(command);
    fclose(fptr);
    return 0;
}

