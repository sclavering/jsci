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
  JSScript *script=0;
  JSObject *scrobj=0;

  if(!JSVAL_IS_STRING(argv[0])) return JSX_ReportException(cx, "load(): first argument must be a string");

  char *filename = JS_GetStringBytes(JSVAL_TO_STRING(argv[0]));

  int fd = open(filename, O_RDONLY);
  if(fd == -1) return JSX_ReportException(cx, "load(): could not open %s",filename);

  struct stat S;
  fstat(fd, &S);
  char *buf = malloc(S.st_size);
  if (!buf) {
    JSX_ReportException(cx, "load: Out of memory loading %s", filename);
    goto failure;
  }
  if(read(fd, buf, S.st_size) != S.st_size) {
    JSX_ReportException(cx, "load: Error loading %s",filename);
    goto failure;
  }

  script=JS_CompileScript(cx, obj, buf, S.st_size, filename, 1);
  if (!script) goto failure;
  scrobj=JS_NewScriptObject(cx, script);
  
  free(buf);
  buf=0;
  
  *rval=JSVAL_ZERO;

  close(fd);
  fd=0;
  
  if (!JS_ExecuteScript(cx, obj, script, rval))
    goto failure;

  JS_RemoveRoot(cx, &scrobj);

  return JS_TRUE;

 failure:
  if (buf) free(buf);
  if (fd) close(fd);
  if (scrobj)
    JS_RemoveRoot(cx, &scrobj);
  return JS_FALSE;
}


jsval JSX_make_load(JSContext *cx) {
  JSFunction *jsfun=JS_NewFunction(cx, load, 1, 0, 0, 0);
  if(!jsfun) return JSVAL_VOID;
  return OBJECT_TO_JSVAL(JS_GetFunctionObject(jsfun));
}

