#include <jsapi.h>
#include <string.h>

static JSBool isCompilableUnit(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
  char *expr;

  if (argc!=1) {
    JSX_ReportException(cx, "Wrong number of arguments");
    return JS_FALSE;
  }

  if (!JS_ConvertArguments(cx, argc, argv, "s", &expr)) {
    return JS_FALSE;
  }

  if (JS_BufferIsCompilableUnit(cx, obj, expr, strlen(expr)))
    *rval=JSVAL_TRUE;
  else
    *rval=JSVAL_FALSE;

  return JS_TRUE;
}

JSBool
JSX_init(JSContext *cx,  JSObject *obj, int argc, jsval *argv, jsval *rval) {
  JSFunction *jsfun=JS_NewFunction(cx, isCompilableUnit, 0, 0, 0, 0);
  if (!jsfun)
    return JS_FALSE;
  *rval=OBJECT_TO_JSVAL(JS_GetFunctionObject(jsfun));
  return JS_TRUE;
}

