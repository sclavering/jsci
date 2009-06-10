#include <string.h>
#include "jsci.h"


// Setting to undefined does nothing, only returns sizeof.
// Setting to null zeroes memory.

int JSX_Set(JSContext *cx, char *p, int will_clean, JsciType *type, jsval v) {
  if(!type) return JSX_ReportException(cx, "Cannot convert JS value to C value, because the C type is not known");

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
  case TYPEPAIR(JSNULL,SUTYPE):
  case TYPEPAIR(JSNULL,ARRAYTYPE):
  case TYPEPAIR(JSNULL,INTTYPE):
  case TYPEPAIR(JSNULL,FLOATTYPE):
    return type->JStoC(cx, p, v, will_clean);

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
