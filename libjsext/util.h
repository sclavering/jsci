#ifndef _jsx_util_h
#define _jsx_util_h

#include "jsapi.h"

// This used to be a function, and it's still handy to write "return JSX_ReportException(...)" rather than two statements
#define JSX_ReportException(args...) (JS_ReportError(args), JS_FALSE);

#endif
