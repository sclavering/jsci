#ifndef LIBCTOXML_H
#define LIBCTOXML_H


/*
 * For public use
 */

/*
  C is a 0-terminated string
  errorpos, if not null, will be used to store string offset of 1st syntax error or -1 if parsing was ok
  Returns 0-terminated string allocated with malloc.
 */

char *ctoxml(char *C, int *errorpos);

void ctoxml_free(char *C);

/*
 * For internal use
 */

extern struct strbuf *ctoxml_STDOUT;

#endif
