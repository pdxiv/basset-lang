/* util.c - Common utility functions */
#include "util.h"
#include <stdlib.h>
#include <string.h>

/* C89 compatible strdup (POSIX strdup not in C89 standard) */
char* basset_strdup(const char *s) {
    char *d;
    if (!s) return NULL;
    d = malloc(strlen(s) + 1);
    if (d) strcpy(d, s);
    return d;
}
