#include "ctoxml.h"
#include <stdlib.h>
#include "c_scan.h"
#include "strbuf.h"
#include "util.h"

struct strbuf *ctoxml_STDOUT;
extern int ctoxml_cfilepos;

stringhash *ctoxml_typedefs;

// C is a 0-terminated string
// errorpos will be used to store string offset of 1st syntax error or -1 if parsing was ok
// Returns 0-terminated string allocated with malloc.
char *ctoxml(char *C, int *errorpos) {
  YY_BUFFER_STATE I=ctoxml_c_scan_string(C); // Input buffer
  
  ctoxml_STDOUT=strbuf_new();
  ctoxml_typedefs = stringhash_new();
  ctoxml_filename = 0;

  PUTS("<C>\n");
  int res = ctoxml_cparse();
  if (res) {
    *errorpos = ctoxml_cfilepos;
  } else {
    *errorpos = -1;
    PUTS("</C>");
  }

  ctoxml_c_delete_buffer(I);
  stringhash_destroy(ctoxml_typedefs);

  char *ret = realloc(ctoxml_STDOUT->buf, ctoxml_STDOUT->len + 1);
  free(ctoxml_STDOUT);
  return ret;
}


static JSBool jsx_cToXML(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
  jsval arg = argv[0];
  if(!JSVAL_IS_STRING(arg)) {
    JSX_ReportException(cx, "cToXML: argument must be a string");
    return JS_FALSE;
  }
  char *c_code = JS_GetStringBytes(JSVAL_TO_STRING(arg));
  int errpos = 0;
  char *xmlstr = ctoxml(c_code, &errpos);
  if(errpos != -1) {
    JSX_ReportException(cx, "cToXML: C syntax error at %d", errpos);
    return JS_FALSE;
  }
  *rval = STRING_TO_JSVAL(JS_NewStringCopyZ(cx, xmlstr));
  free(xmlstr);
  return JS_TRUE;
}


jsval make_cToXML(JSContext *cx) {
  JSFunction *jsfun = JS_NewFunction(cx, jsx_cToXML, 1, 0, 0, 0);
  if(!jsfun) return JSVAL_VOID;
  return OBJECT_TO_JSVAL(JS_GetFunctionObject(jsfun));
}
