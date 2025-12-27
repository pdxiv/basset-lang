/* keyword_hash.h - Fast keyword lookup using hash table */
#ifndef KEYWORD_HASH_H
#define KEYWORD_HASH_H

#include "syntax_tables.h"

/* Initialize keyword hash table (call once at startup) */
void keyword_hash_init(void);

/* Lookup a keyword in the hash table
 * Returns 1 if found (token set), 0 if not found
 */
int keyword_hash_lookup(const char *text, int len, unsigned char *token);

/* Free hash table resources (call at shutdown) */
void keyword_hash_cleanup(void);

#endif
