#include "ctoxml.h"
#include <stdlib.h>
#include "c_scan.h"
#include "strbuf.h"

struct strbuf *ctoxml_STDOUT;
extern int ctoxml_cfilepos;

#ifdef _WIN32
__declspec(dllexport)
#endif
void ctoxml_free(char *C) {
	free(C);
}

#ifdef _WIN32
__declspec(dllexport)
#endif
char *ctoxml(char *C, int *errorpos) {
  int res;
  char *ret;
  YY_BUFFER_STATE I=ctoxml_c_scan_string(C); // Input buffer
  
  ctoxml_STDOUT=strbuf_new();
  ctoxml_init();

  PUTS("<C>\n");
  res=ctoxml_cparse();
  if (errorpos) {
    if (res) *errorpos=ctoxml_cfilepos;
    else {
		*errorpos=-1;
	    PUTS("</C>");
	}
  } else {
    PUTS("</C>");
  }

  ctoxml_c_delete_buffer(I);
  ctoxml_end();

  ret=realloc(ctoxml_STDOUT->buf,ctoxml_STDOUT->len+1);
  free(ctoxml_STDOUT);

  return ret;
}

