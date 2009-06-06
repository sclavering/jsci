#include <jsapi.h>
#include <stdarg.h>
#include "util.h"

JSBool JSX_ReportException(JSContext *cx, const char *format, ...) {
  char *msg = JS_malloc(cx, 201);
  va_list va;
  va_start(va, format);
  int len = vsnprintf(msg, 200, format, va);
  va_end(va);
  if(len > 200) len = 200;
  msg[len] = 0;

  jsval ex = JSVAL_VOID;
  JS_AddRoot(cx, &ex);
  jsval str = STRING_TO_JSVAL(JS_NewString(cx, msg, len));
  JS_AddRoot(cx, &str);
  if(JS_CallFunctionName(cx, JS_GetGlobalObject(cx), "Error", 1, &str, &ex)) JS_SetPendingException(cx, ex);
  else JS_SetPendingException(cx, str);
  JS_RemoveRoot(cx, &str);
  JS_RemoveRoot(cx, &ex);

  return JS_FALSE;
}
