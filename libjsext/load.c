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

static JSBool JSX_ReportException(JSContext *cx, char *format, ...) {
  int len;
  char *msg;
  JSString *Str;
  va_list va;
  jsval str;

  msg=JS_malloc(cx, 81);
  va_start(va, format);
  len=vsnprintf(msg,80,format,va);
  msg[80]=0;
  va_end(va);
  Str=JS_NewString(cx, msg, len);
  str=STRING_TO_JSVAL(Str);
  JS_SetPendingException(cx, str);

  return JS_FALSE;
}

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
  //  JS_AddRoot(cx, &scrobj);
  
  free(buf);
  buf=0;
  
  //  *rval=OBJECT_TO_JSVAL(scrobj);
  *rval=JSVAL_ZERO;

  close(fd);
  fd=0;
  
  //  return JS_TRUE;

  if (!JS_ExecuteScript(cx, obj, script, rval))
    goto failure;

  JS_RemoveRoot(cx, &scrobj);
  //  JS_DestroyScript(cx, script);
  return JS_TRUE;

 failure:
  if (buf) free(buf);
  if (fd) close(fd);
  if (scrobj)
    JS_RemoveRoot(cx, &scrobj);
  //  if (script)
  //    JS_DestroyScript(cx, script);
  return JS_FALSE;
}


jsval JSX_make_load(JSContext *cx, JSObject *glo) {
  JSFunction *jsfun=JS_NewFunction(cx, load, 0, 0, 0, 0);
  if(!jsfun) return JSVAL_VOID;
  return OBJECT_TO_JSVAL(JS_GetFunctionObject(jsfun));
}

