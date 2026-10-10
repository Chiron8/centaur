#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include "colours.h"

int list() {
    printf("%s%s%s\n\n", ANSI_BLUE, "All packages currently installed are:", ANSI_CLEAR);

    struct dirent *en;
    DIR *dr = opendir("/var/db/centaur/packages/installed/");
    if (dr == NULL) {
        perror("Failed to open installed package directory");
        exit(1);
    }
    while ((en = readdir(dr)) != NULL) {
        if (en->d_name[0] == '.') continue; // skip . and ..
        printf("%s\n", en->d_name);
    }
    closedir(dr);
    return 0;
}
