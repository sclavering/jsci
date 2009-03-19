#include <jsapi.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <fcntl.h>

static JSBool GC(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
  JS_GC(cx);
  return JS_TRUE;
}

JSBool JSX_init(JSContext *cx, JSObject *obj, int argc, jsval *argv, jsval *rval) {
  JSFunction *jsfun=JS_NewFunction(cx, GC, 0, 0, 0, 0);
  if(!jsfun) return JS_FALSE;
  *rval=OBJECT_TO_JSVAL(JS_GetFunctionObject(jsfun));
  return JS_TRUE;
}

