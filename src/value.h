/* value.h - Tagged value type for VM stack */
#ifndef VALUE_H
#define VALUE_H

/* Value type tags */
typedef enum {
    VAL_NUMBER,     /* Numeric value (double) */
    VAL_STRING      /* String value (char*) */
} ValueType;

/* Tagged value - holds either a number or string */
typedef struct {
    ValueType type;
    union {
        double number;
        char *string;
    } data;
} Value;

/* Helper functions for creating values */
#ifdef __GNUC__
__attribute__((unused))
#endif
static Value value_number(double n) {
    Value v;
    v.type = VAL_NUMBER;
    v.data.number = n;
    return v;
}

#ifdef __GNUC__
__attribute__((unused))
#endif
static Value value_string(char *s) {
    Value v;
    v.type = VAL_STRING;
    v.data.string = s;
    return v;
}

#endif
