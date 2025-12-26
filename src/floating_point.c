/* floating_point.c - Floating Point Implementation */
#include "floating_point.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* Convert double to string */
void fp_to_string(double value, char *buffer, int bufsize) {
    /* Use standard C formatting for now */
    /* Note: Using sprintf instead of snprintf for C89 compatibility (snprintf is C99) */
    /* Caller must ensure buffer is large enough */
    (void)bufsize; /* Unused in sprintf version */
    sprintf(buffer, "%g", value);
}

/* Convert string to double */
double fp_from_string(const char *str) {
    return strtod(str, NULL);
}
