#include <jsapi.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <fcntl.h>

static JSBool GC(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
#ifdef JS_THREADSAFE
  jsrefcount count=JS_SuspendRequest(cx);
#endif
  JS_GC(cx);
#ifdef JS_THREADSAFE
  JS_ResumeRequest(cx, count);
#endif
  return JS_TRUE;
}

JSBool
JSX_init(JSContext *cx,  JSObject *obj, int argc, jsval *argv, jsval *rval) {
  JSFunction *jsfun=JS_NewFunction(cx, GC, 0, 0, 0, 0);
  if (!jsfun)
    return JS_FALSE;
  *rval=OBJECT_TO_JSVAL(JS_GetFunctionObject(jsfun));
  return JS_TRUE;
}

