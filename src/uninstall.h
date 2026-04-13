#include <stdbool.h>

char* remove_version(char package[]);
void dep_check(char file[], char package[]);
int uninstall(char package[], int force);
