#ifndef _jsx_util_h
#define _jsx_util_h

#include "jsapi.h"

// This used to be a function, and it's still handy to write "return JSX_ReportException(...)" rather than two statements
#define JSX_ReportException(args...) (JS_ReportError(args), JS_FALSE);

// Our natives used to be old slow JSNative's, and relied on being able to pass around "jsval* rval".  When porting to JSFastNative, we still needed that (for rooting in js1.8), and jsapi doesn't provide it itself, so we just cheat.  In js1.8.5+ we can just use a local var of type jsval, thanks to the conservative stack scanner.
#define JSX_RVAL_ADDR(vp) vp;

#endif
