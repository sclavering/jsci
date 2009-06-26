/*
Defines the javascript function:

    script = load(filename)

Loads and compiles an ISO-8859-1 JavaScript file.  Returns the value of the last expression in the script.

The file may start with a #! line, which is ignored.
*/

#include <jsapi.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <fcntl.h>
#include <string.h>
#include "util.h"


static JSBool load(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
  if(!JSVAL_IS_STRING(argv[0])) return JSX_ReportException(cx, "load(): first argument must be a string");
  char *filename = JS_GetStringBytes(JSVAL_TO_STRING(argv[0]));
  int fd = open(filename, O_RDONLY);
  if(fd == -1) return JSX_ReportException(cx, "load(): could not open %s",filename);

  JSBool ok = JS_FALSE;
  struct stat S;
  fstat(fd, &S);
  char *buf = malloc(S.st_size);
  if (!buf) {
    JSX_ReportException(cx, "load: Out of memory loading %s", filename);
    goto exit;
  }
  if(read(fd, buf, S.st_size) != S.st_size) {
    JSX_ReportException(cx, "load: Error loading %s",filename);
    goto exit;
  }
  *rval=JSVAL_ZERO;
  if(!JS_EvaluateScript(cx, obj, buf, S.st_size, filename, 1, rval)) goto exit;

  ok = JS_TRUE;
 exit:
  if(buf) free(buf);
  if(fd) close(fd);
  return ok;
}


jsval JSX_make_load(JSContext *cx) {
  JSFunction *jsfun=JS_NewFunction(cx, load, 1, 0, 0, 0);
  if(!jsfun) return JSVAL_VOID;
  return OBJECT_TO_JSVAL(JS_GetFunctionObject(jsfun));
}

