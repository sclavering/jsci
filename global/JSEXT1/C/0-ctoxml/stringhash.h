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

typedef struct hashtable_itr stringhash_itr;

// Use free to destroy
stringhash_itr *stringhash_iterator(stringhash *h);
char *stringhash_iterator_key(stringhash_itr *itr);
void *stringhash_iterator_value(stringhash_itr *itr);

// Return zero if advanced to end of table
int stringhash_iterator_advance(stringhash_itr *itr);

// Return zero if advanced to end of table
int stringhash_iterator_remove(stringhash_itr *itr);

// Return zero if not found
int stringhash_iterator_search(stringhash_itr *itr,
			       stringhash *h, char *key);

int stringhash_size(stringhash *h);

#endif

