#include <dirent.h>

typedef struct {
    char *dep;
    char *parent;
} Dependency;

void reverse_dependencies(Dependency *deps, size_t size);
bool already_added(char *target, Dependency *deps, size_t deps_size);
void free_dependencies(Dependency *deps, size_t size);
Dependency *get_dependencies(const char *file, const char *parent, Dependency *deps, size_t *deps_size);
