#include <stdio.h>

int license() {
    // output gpl license
    FILE *fptr = fopen("/var/db/centaur/doc/small_license.txt", "r");
    char line[256];
    while (fgets(line, sizeof(line), fptr)) {
        printf(line);
    }
    fclose(fptr);
    return 0;
}
