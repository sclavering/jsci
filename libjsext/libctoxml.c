#include "ctoxml.h"
#include <stdlib.h>
#include "c_scan.h"
#include "strbuf.h"
#include "util.h"

struct strbuf *ctoxml_STDOUT;
extern int ctoxml_cfilepos;

JSContext *cparser_jscx;
jsval cparser_typedefs, cparser_preprocessor_directives;


// C is a 0-terminated string
// errorpos will be used to store string offset of 1st syntax error or -1 if parsing was ok
// Returns 0-terminated string allocated with malloc.
char *ctoxml(char *C, int *errorpos) {
  YY_BUFFER_STATE I=ctoxml_c_scan_string(C); // Input buffer
  
  ctoxml_STDOUT=strbuf_new();
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

  char *ret = realloc(ctoxml_STDOUT->buf, ctoxml_STDOUT->len + 1);
  free(ctoxml_STDOUT);
  return ret;
}


static JSBool jsx_cToXML(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
  jsval tmp, c_code_str = argv[0];
  if(!JSVAL_IS_STRING(c_code_str)) {
    JSX_ReportException(cx, "cToXML: first argument must be a string containing pre-processed C code");
    return JS_FALSE;
  }
  *rval = OBJECT_TO_JSVAL(JS_NewObject(cx, 0, 0, 0));
  if(argv[1] == JSVAL_NULL) return JS_FALSE;
  tmp = cparser_typedefs = OBJECT_TO_JSVAL(JS_NewObject(cx, 0, 0, 0));
  if(tmp == JSVAL_NULL) return JS_FALSE;
  if(!JS_SetProperty(cx, JSVAL_TO_OBJECT(*rval), "typedef_set", &tmp)) return JS_FALSE;
  tmp = cparser_preprocessor_directives = OBJECT_TO_JSVAL(JS_NewArrayObject(cx, 0, 0));
  if(tmp == JSVAL_NULL) return JS_FALSE;
  if(!JS_SetProperty(cx, JSVAL_TO_OBJECT(*rval), "preprocessor_directives", &tmp)) return JS_FALSE;

  cparser_jscx = cx;
  char *c_code = JS_GetStringBytes(JSVAL_TO_STRING(c_code_str));
  int errpos = 0;
  char *xmlstr = ctoxml(c_code, &errpos);
  if(errpos != -1) {
    JSX_ReportException(cx, "cToXML: C syntax error at %d", errpos);
    return JS_FALSE;
  }
  tmp = STRING_TO_JSVAL(JS_NewStringCopyZ(cx, xmlstr));
  JSBool res = JS_SetProperty(cx, JSVAL_TO_OBJECT(*rval), "xml", &tmp);
  free(xmlstr);
  cparser_jscx = 0;
  cparser_typedefs = JSVAL_VOID;
  return res;
}


jsval make_cToXML(JSContext *cx) {
  JSFunction *jsfun = JS_NewFunction(cx, jsx_cToXML, 1, 0, 0, 0);
  if(!jsfun) return JSVAL_VOID;
  return OBJECT_TO_JSVAL(JS_GetFunctionObject(jsfun));
}
