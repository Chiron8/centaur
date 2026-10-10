#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>

char *get_meta_variable_value(FILE *script_file, const char *target_variable);

int get_latest(const char *path, char *latest, size_t size);

int concat_realloc(size_t currentLen, size_t lineLen, size_t *capacity, char **command);

int cat_command(bool newCommand, size_t *currentLen, size_t lineLen, size_t *capacity, char **command, char line[]);

int read_execute(char file[], bool force, bool uninstall);


