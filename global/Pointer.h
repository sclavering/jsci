#ifndef __Pointer_h
#define __Pointer_h

#include <jsapi.h>
#include "Type.h"

# define POINTER_API

struct JSX_Pointer {
  void *ptr; // 0 means unresolved. NULL pointer is repr by null value.
  struct JSX_Type *type;
  void (*finalize) (void *);
};

struct JSX_Callback {
  void *ptr; // Points to executable code
  struct JSX_Type *type;
  void (*finalize) (void *);
  JSContext *cx;
  JSFunction *fun;
  void *writeable; // Points to writeable code
};

POINTER_API JSClass * JSX_GetPointerClass(void);
POINTER_API int JSX_Get(JSContext *cx, char *p, char *oldptr, int do_clean, struct JSX_Type *type, jsval *rval);
POINTER_API JSBool JSX_InitPointer(JSContext *cx, JSObject *retobj, JSObject *typeobj);

#endif
