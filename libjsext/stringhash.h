// Uses Christopher Clark's hash table and a simple hash function to provide hashes
// where keys are C strings and values are void *.
// Keys and values are freed by stringhash on destroy and remove.

#ifndef STRINGHASH_H
#define STRINGHASH_H

typedef struct hashtable stringhash;

stringhash *stringhash_new();
int stringhash_insert(stringhash *h, char *key, void *value); // non-null on success
void *stringhash_search(stringhash *h, char *key); // null if not found
void *stringhash_remove(stringhash *h, char *key); // null if not found
void stringhash_destroy(stringhash *h);

#endif

