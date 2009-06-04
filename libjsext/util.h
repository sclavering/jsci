#ifndef _jsx_util_h
#define _jsx_util_h

#include "jsapi.h"

JS_BEGIN_EXTERN_C

extern JSBool JSX_ReportException(JSContext *cx, const char *format, ...);

JS_END_EXTERN_C

#endif
