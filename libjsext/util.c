#include <jsapi.h>
#include <stdarg.h>
#include "util.h"

JSBool JSX_ReportException(JSContext *cx, const char *format, ...) {
  char *msg;
  JSString *Str;
  va_list va;
  va_start(va, format);
  msg = JS_malloc(cx, 201);
  va_start(va, format);
  int len = vsnprintf(msg, 200, format, va);
  if(len > 200) len = 200;
  msg[len] = 0;
  va_end(va);
  Str = JS_NewString(cx, msg, len);
  jsval str = STRING_TO_JSVAL(Str);
  JS_SetPendingException(cx, str);
  return JS_FALSE;
}
