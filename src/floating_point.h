/* floating_point.h - BASIC Floating Point Support */
#ifndef FLOATING_POINT_H
#define FLOATING_POINT_H

/* Classic BASIC implementations used various floating point formats.
 * This implementation uses standard C double type for simplicity. */

/* Convert double to string in BASIC format */
void fp_to_string(double value, char *buffer, int bufsize);

/* Convert string to double */
double fp_from_string(const char *str);

#endif /* FLOATING_POINT_H */
