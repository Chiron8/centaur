#include <stdio.h>
#include <string.h>
#include <dirent.h>

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
        /* weird thing to exclude last 7 chars (removes .centaur)
        also check if file is not .. or .*/
        if (list_iscentaur(en->d_name)) {
            size_t len = strlen(en->d_name);
            printf("%.*s\n", (int)(len-8), en->d_name);
        } 
    }
    closedir(dr);
    return -1;
}
