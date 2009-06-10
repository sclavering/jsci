#include <string.h>
#include "jsci.h"


// Setting to undefined does nothing, only returns sizeof.
// Setting to null zeroes memory.

int JSX_Set(JSContext *cx, char *p, int will_clean, JsciType *type, jsval v) {
  if(!type) return JSX_ReportException(cx, "Cannot convert JS value to C value, because the C type is not known");

  int size=-1;
  int tmpint;

  int typepair = TYPEPAIR(JSX_JSType(cx, v), type->type);

  // Determine the appropriate conversion

  switch(typepair) {
  case TYPEPAIR(JSVAL_INT,INTTYPE):
  case TYPEPAIR(JSVAL_BOOLEAN,INTTYPE):
  case TYPEPAIR(JSVAL_DOUBLE,INTTYPE):
  case TYPEPAIR(JSVAL_INT,BITFIELDTYPE):
  case TYPEPAIR(JSVAL_BOOLEAN,BITFIELDTYPE):
  case TYPEPAIR(JSNULL,BITFIELDTYPE):
  case TYPEPAIR(JSVAL_DOUBLE,BITFIELDTYPE):
  case TYPEPAIR(JSVOID,BITFIELDTYPE):
  case TYPEPAIR(JSVAL_DOUBLE,FLOATTYPE):
  case TYPEPAIR(JSVAL_BOOLEAN,FLOATTYPE):
  case TYPEPAIR(JSVAL_INT,FLOATTYPE):
  case TYPEPAIR(JSVAL_STRING,ARRAYTYPE):
  case TYPEPAIR(JSARRAY,ARRAYTYPE):
  case TYPEPAIR(JSPOINTER,ARRAYTYPE):
  case TYPEPAIR(JSVAL_OBJECT,SUTYPE):
  case TYPEPAIR(JSFUNC,POINTERTYPE):
  case TYPEPAIR(JSPOINTER,POINTERTYPE):
  case TYPEPAIR(JSNULL,POINTERTYPE):
  case TYPEPAIR(JSVAL_STRING,POINTERTYPE):
    return type->JStoC(cx, p, v, will_clean);

  case TYPEPAIR(JSNULL,SUTYPE):
  case TYPEPAIR(JSNULL,ARRAYTYPE):
  case TYPEPAIR(JSNULL,INTTYPE):
  case TYPEPAIR(JSNULL,FLOATTYPE):

    // Initialize with zero

    size = type->SizeInBytes();
    memset(p, 0, size);
    return size;

  // Do-nothing cases
  case TYPEPAIR(JSVOID,POINTERTYPE):
  case TYPEPAIR(JSVOID,FUNCTIONTYPE):
  case TYPEPAIR(JSVOID,SUTYPE):
  case TYPEPAIR(JSVOID,ARRAYTYPE):
  case TYPEPAIR(JSVOID,INTTYPE):
  case TYPEPAIR(JSVOID,FLOATTYPE):
    return type->SizeInBytes();

  default:
    // Could not find appropriate conversion

    goto failure;

  }

 failure:
  JSX_ReportException(cx, "Set: Could not convert value from JS %s to C %s",JSX_jstypenames[typepair/TYPECOUNT2],JSX_typenames[typepair%TYPECOUNT2]);
  return 0;
}


JSBool JSX_Set_multi(JSContext *cx, char *ptr, JsciTypeFunction *funct, jsval *vp, void **argptr) {
  int cursiz;
  for(int i = 0; i < funct->nParam; ++i) {
    JsciType *t = funct->param[i];

    if(t->type == ARRAYTYPE) {
      // In function calls, arrays are passed by pointer
      ptr = new char[t->SizeInBytes()];
      cursiz = JSX_Set(cx, (char*) *(void **)ptr, 1, t, *vp);
      if(cursiz) {
        cursiz = sizeof(void *);
      } else {
        delete ptr;
        return JS_FALSE;
      }
    } else {
      cursiz = JSX_Set(cx, (char*) ptr, 1, t, *vp);
    }
    if(!cursiz) return JS_FALSE;
    *(argptr++) = ptr;
    ptr += cursiz;
    vp++;
  }

  return JS_TRUE;
}
