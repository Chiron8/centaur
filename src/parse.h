#include <stddef.h>
#include <stdbool.h>

int concat_realloc(size_t currentLen, size_t lineLen, size_t *capacity, char **command);

int cat_command(bool newCommand, size_t *currentLen, size_t lineLen, size_t *capacity, char **command, char line[]);

int read_execute(char file[], bool force);
