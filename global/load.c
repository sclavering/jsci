#include <jsapi.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef _WIN32
# include <io.h>
#else
# define __declspec(x)
# include <unistd.h>
#endif
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
  char *before=0;
  char *after=0;
  int beforelen=0;
  int afterlen=0;

  if (argc<1) {
    JSX_ReportException(cx, "Too few arguments");
    return JS_FALSE;
  }

  if (argc>1 && JSVAL_IS_STRING(argv[1])) {
    before = JS_GetStringBytes(JSVAL_TO_STRING(argv[1]));
    beforelen = JS_GetStringLength(JSVAL_TO_STRING(argv[1]));
  }

  if (argc>2 && JSVAL_IS_STRING(argv[2])) {
    after = JS_GetStringBytes(JSVAL_TO_STRING(argv[2]));
    afterlen = JS_GetStringLength(JSVAL_TO_STRING(argv[2]));
  }

  if (!JS_ConvertArguments(cx, argc, argv, "s", &filename)) {
    return JS_FALSE;
  }

#ifdef _WIN32
  fd=_open(filename,O_RDONLY | O_BINARY);
#else
  fd=open(filename,O_RDONLY);
#endif
  if (fd==-1) {
    // File does not exist, throw exception
    JSX_ReportException(cx, "load: Could not open %s",filename);
    return JS_FALSE;
  }
  fstat(fd, &S);
  buf=malloc(S.st_size+beforelen+afterlen);
  if (!buf) {
    JSX_ReportException(cx, "load: Out of memory loading %s", filename);
    goto failure;
  }
#ifdef _WIN32
  if (_read(fd, buf+beforelen, S.st_size)!=S.st_size) {
#else
  if (read(fd, buf+beforelen, S.st_size)!=S.st_size) {
#endif
    JSX_ReportException(cx, "load: Error loading %s",filename);
    goto failure;
  }

  if (S.st_size>1 && buf[beforelen]=='#' && buf[beforelen+1]=='!') {
    // shebang
    while (buf[shebanglen+beforelen]!='\n' &&
	   buf[shebanglen+beforelen]!='\r') shebanglen++;
    shebanglen++;
    if (shebanglen<S.st_size &&
	(buf[shebanglen+beforelen]=='\n' ||
	 buf[shebanglen+beforelen]=='\r') &&
	buf[shebanglen+beforelen]!=buf[shebanglen+beforelen-1])
      shebanglen++;
  }
  if (before) {
    memcpy(buf+shebanglen, before, beforelen);
  }
  if (after) {
    memcpy(buf+beforelen+S.st_size, after, afterlen);
  }

  script=JS_CompileScript(cx, obj, buf+shebanglen, S.st_size-shebanglen+beforelen+afterlen, filename, shebanglen?2:1);
  if (!script) goto failure;
  scrobj=JS_NewScriptObject(cx, script);
  //  JS_AddRoot(cx, &scrobj);
  
  free(buf);
  buf=0;
  
  //  *rval=OBJECT_TO_JSVAL(scrobj);
  *rval=JSVAL_ZERO;

#ifdef _WIN32
  _close(fd);
#else
  close(fd);
#endif
  fd=0;
  
  //  return JS_TRUE;

  if (!JS_ExecuteScript(cx, obj, script, rval))
    goto failure;

  JS_RemoveRoot(cx, &scrobj);
  //  JS_DestroyScript(cx, script);
  return JS_TRUE;

 failure:
  if (buf) free(buf);
#ifdef _WIN32
  if (fd) _close(fd);
#else
  if (fd) close(fd);
#endif
  if (scrobj)
    JS_RemoveRoot(cx, &scrobj);
  //  if (script)
  //    JS_DestroyScript(cx, script);
  return JS_FALSE;
}

__declspec(dllexport) JSBool
JSX_init(JSContext *cx,  JSObject *obj, int argc, jsval *argv, jsval *rval) {
  JSFunction *jsfun=JS_NewFunction(cx, load, 0, 0, 0, 0);
  if (!jsfun)
    return JS_FALSE;
  *rval=OBJECT_TO_JSVAL(JS_GetFunctionObject(jsfun));
  return JS_TRUE;
}

