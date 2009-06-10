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

    tmpint=JSVAL_TO_INT(v);
    goto intcommon;

  case TYPEPAIR(JSVAL_INT,BITFIELDTYPE):
  case TYPEPAIR(JSVAL_BOOLEAN,BITFIELDTYPE):

    tmpint=JSVAL_TO_INT(v);
    goto bitfieldcommon;

  case TYPEPAIR(JSNULL,BITFIELDTYPE):

    tmpint=0;
    goto bitfieldcommon;

  case TYPEPAIR(JSVAL_DOUBLE,BITFIELDTYPE):

    tmpint=(int)*JSVAL_TO_DOUBLE(v);
    
    // fall through

  case TYPEPAIR(JSVOID,BITFIELDTYPE):

  bitfieldcommon:
    size = ((JsciTypeNumeric *) ((JsciTypeBitfield *) type)->member)->size;
    goto intcommon;

  case TYPEPAIR(JSVAL_BOOLEAN,INTTYPE):

    tmpint=v==JSVAL_TRUE?1:0;
    goto intcommon;

  case TYPEPAIR(JSVAL_DOUBLE,INTTYPE):

    tmpint=(int)*JSVAL_TO_DOUBLE(v);

  intcommon:

    // Return a number from an int (of various sizes)
    switch(size != -1 ? size : ((JsciTypeNumeric *) type)->size) {

    case 0:
      *(char *)p=tmpint;
      size=sizeof(char);
      break;

    case 1:
      *(short *)p=tmpint;
      size=sizeof(short);
      break;

    case 2:
      *(int *)p=tmpint;
      size=sizeof(int);
      break;

    case 3:
      *(long *)p=tmpint;
      size=sizeof(long);
      break;

    case 4:
      *(long long *)p=tmpint;
      size=sizeof(long long);
      break;

    case 5:
      *(int64 *)p=tmpint;
      size=sizeof(int64);
      break;

    }

    return size;

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
    return sizeof(void *);
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
