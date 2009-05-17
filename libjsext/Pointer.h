#ifndef __Pointer_h
#define __Pointer_h

#include <jsapi.h>
#include "Type.h"

JSClass * JSX_GetPointerClass(void);
int JSX_Get(JSContext *cx, char *p, char *oldptr, int do_clean, struct JSX_Type *type, jsval *rval);
JSBool JSX_InitPointer(JSContext *cx, JSObject *retobj, JSObject *typeobj);

#endif
