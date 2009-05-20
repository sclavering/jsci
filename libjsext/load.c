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
  char *filename;
  int fd;
  struct stat S;
  char *buf;
  int shebanglen=0;
  JSScript *script=0;
  JSObject *scrobj=0;

  if (argc<1) {
    JSX_ReportException(cx, "Too few arguments");
    return JS_FALSE;
  }

  if (!JS_ConvertArguments(cx, argc, argv, "s", &filename)) {
    return JS_FALSE;
  }

  fd=open(filename,O_RDONLY);
  if (fd==-1) {
    // File does not exist, throw exception
    JSX_ReportException(cx, "load: Could not open %s",filename);
    return JS_FALSE;
  }
  fstat(fd, &S);
  buf=malloc(S.st_size);
  if (!buf) {
    JSX_ReportException(cx, "load: Out of memory loading %s", filename);
    goto failure;
  }
  if(read(fd, buf, S.st_size) != S.st_size) {
    JSX_ReportException(cx, "load: Error loading %s",filename);
    goto failure;
  }

  if(S.st_size > 1 && buf[0] == '#' && buf[1] == '!') {
    // shebang
    while(buf[shebanglen] != '\n' && buf[shebanglen] != '\r') shebanglen++;
    shebanglen++;
    if(shebanglen < S.st_size && (buf[shebanglen] == '\n' || buf[shebanglen] == '\r') && buf[shebanglen] != buf[shebanglen - 1]) {
      shebanglen++;
    }
  }

  script=JS_CompileScript(cx, obj, buf + shebanglen, S.st_size - shebanglen, filename, shebanglen ? 2 : 1);
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


jsval JSX_make_load(JSContext *cx, JSObject *glo) {
  JSFunction *jsfun=JS_NewFunction(cx, load, 0, 0, 0, 0);
  if(!jsfun) return JSVAL_VOID;
  return OBJECT_TO_JSVAL(JS_GetFunctionObject(jsfun));
}

