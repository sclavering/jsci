#include "stringhash.h"
#include "hashtable.h"
#include "hashtable_itr.h"
#include <string.h>

static unsigned int stringhash_func(void *a) {
  char *cp=(char *)a;
  char c;
  int h;
  for (h=0; (c=*cp); ++cp) h = (h<<5)^(h>>27)^c;
  return h;
}

static int stringhash_eq(void *a, void *b) {
  return !strcmp((char *)a,(char *)b);
}

stringhash *stringhash_new() {
  return create_hashtable(256,stringhash_func,stringhash_eq);
}

int stringhash_insert(stringhash *h, char *key, void *value) { // non-null on success
  return hashtable_insert(h,key,value);
}

void *stringhash_search(stringhash *h, char *key) { // null if not found
  return hashtable_search(h, key);
}

void *stringhash_remove(stringhash *h, char *key) { // null if not found
  return hashtable_remove(h, key);
}

void stringhash_destroy(stringhash *h) {
  hashtable_destroy(h, 1);
}

stringhash_itr *stringhash_iterator(stringhash *h) {
  return hashtable_iterator(h);
}

char *stringhash_iterator_key(stringhash_itr *itr) {
  return (char *)hashtable_iterator_key(itr);
}

void *stringhash_iterator_value(stringhash_itr *itr) {
  return hashtable_iterator_value(itr);
}

int stringhash_iterator_advance(stringhash_itr *itr) {
  return hashtable_iterator_advance(itr);
}

// Return zero if advanced to end of table
int stringhash_iterator_remove(stringhash_itr *itr) {
  free(hashtable_iterator_value(itr));
  return hashtable_iterator_remove(itr);
}

// Return zero if not found
int stringhash_iterator_search(stringhash_itr *itr,
			      stringhash *h, char *key) {
  return hashtable_iterator_search(itr,h,key);
}

int stringhash_size(stringhash *h) {
  return h->entrycount;
}
