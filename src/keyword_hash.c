/* keyword_hash.c - Fast keyword lookup using hash table */
#include "keyword_hash.h"
#include "util.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define HASH_SIZE 256

/* Hash table entry (linked list for collisions) */
typedef struct HashEntry {
    char *keyword;               /* Uppercase keyword */
    unsigned char token;         /* Token value */
    struct HashEntry *next;      /* Next in collision chain */
} HashEntry;

/* Hash table */
static HashEntry *hash_table[HASH_SIZE];
static int initialized = 0;

/* djb2 hash function - fast and good distribution */
static unsigned int hash_string(const char *str) {
    unsigned int hash = 5381;
    int c;
    
    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    
    return hash % HASH_SIZE;
}

/* Initialize keyword hash table from keyword_table */
void keyword_hash_init(void) {
    int i;
    
    if (initialized) return;
    
    /* Clear hash table */
    for (i = 0; i < HASH_SIZE; i++) {
        hash_table[i] = NULL;
    }
    
    /* Insert all keywords from keyword_table */
    for (i = 0; i < keyword_table_size; i++) {
        unsigned int h = hash_string(keyword_table[i].keyword);
        HashEntry *entry = malloc(sizeof(HashEntry));
        
        entry->keyword = basset_strdup(keyword_table[i].keyword);
        entry->token = keyword_table[i].token;
        entry->next = hash_table[h];
        hash_table[h] = entry;
    }
    
    initialized = 1;
}

/* Lookup keyword in hash table
 * text: input text (may be lowercase)
 * len: length of text
 * token: output parameter for token value
 * Returns: 1 if found, 0 if not found
 */
int keyword_hash_lookup(const char *text, int len, unsigned char *token) {
    char upper[64];
    unsigned int h;
    HashEntry *entry;
    int i;
    
    /* Convert to uppercase for comparison */
    if (len >= 64) return 0;
    
    for (i = 0; i < len; i++) {
        upper[i] = toupper(text[i]);
    }
    upper[len] = '\0';
    
    /* Hash and search */
    h = hash_string(upper);
    entry = hash_table[h];
    
    while (entry) {
        if (strcmp(upper, entry->keyword) == 0) {
            *token = entry->token;
            return 1;
        }
        entry = entry->next;
    }
    
    return 0;
}

/* Free hash table resources */
void keyword_hash_cleanup(void) {
    int i;
    
    if (!initialized) return;
    
    for (i = 0; i < HASH_SIZE; i++) {
        HashEntry *entry = hash_table[i];
        while (entry) {
            HashEntry *next = entry->next;
            free(entry->keyword);
            free(entry);
            entry = next;
        }
        hash_table[i] = NULL;
    }
    
    initialized = 0;
}
