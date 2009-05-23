#include "stringhash.h"
#include "hashtable.h"
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
