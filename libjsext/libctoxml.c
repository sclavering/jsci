#include "ctoxml.h"
#include <stdlib.h>
#include "c_scan.h"
#include "strbuf.h"

struct strbuf *ctoxml_STDOUT;
extern int ctoxml_cfilepos;

void ctoxml_free(char *C) {
    free(C);
}

char *ctoxml(char *C, int *errorpos) {
  int res;
  char *ret;
  YY_BUFFER_STATE I=ctoxml_c_scan_string(C); // Input buffer
  
  ctoxml_STDOUT=strbuf_new();
  ctoxml_typedefs = stringhash_new();
  ctoxml_filename = 0;

  PUTS("<C>\n");
  res=ctoxml_cparse();
  if (errorpos) {
    if (res) {
      *errorpos = ctoxml_cfilepos;
    } else {
      *errorpos = -1;
      PUTS("</C>");
    }
  } else {
    PUTS("</C>");
  }

  ctoxml_c_delete_buffer(I);
  stringhash_destroy(ctoxml_typedefs);

  ret=realloc(ctoxml_STDOUT->buf,ctoxml_STDOUT->len+1);
  free(ctoxml_STDOUT);

  return ret;
}

