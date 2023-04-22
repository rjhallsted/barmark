#include <stdlib.h>
#include <stdio.h>
#include <string.h>

char* addNullTerm(char* str, size_t len) {
    char *fixed = malloc(len + 1);
    stpncpy(fixed, str, len);
    fixed[len] = '\0';
    return fixed;
}

char* barmarkGetLine(FILE *fd) {
    size_t *len = malloc(sizeof(size_t));

    char *line = fgetln(fd, len);
    line = addNullTerm(line, *len);
    free(len);
    return line;
}

FILE* openFile(char *path) {
    FILE* fd = fopen(path, "r");
    if (fd == NULL) {
        printf("Error opening file: '%s'\n", path);
        exit(EXIT_FAILURE);
    }
    return fd;
}