typedef struct {
    char *dep;
    char *parent;
} Dependency;

void reverseDependencies(Dependency *deps, size_t size);
bool alreadyAdded(char *target, Dependency *deps, size_t deps_size);
void freeDependencies(Dependency *deps, size_t size);
Dependency *getDependencies(const char *file, const char *parent, Dependency *deps, size_t *deps_size);
