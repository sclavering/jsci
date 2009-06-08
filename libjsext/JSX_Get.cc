#include <string.h>
#include "jsci.h"


// rval should be rooted
// if p is NULL, type must also be NULL.
// Returns 1 for success, 0 for failure, and occasionally -1 for success where a JS function was created
int JSX_Get(JSContext *cx, char *p, JSX_Type *type, jsval *rval) {
  if(!type) return JSX_ReportException(cx, "Cannot convert C value to JS value, because the C type is not known");

  // Determine the appropriate conversion

  switch(type->type) {

  case POINTERTYPE:
  {
    if(*(void **)p == NULL) {
      *rval = JSVAL_NULL;
      return 1;
    }

    JSObject *obj = JS_NewObject(cx, JSX_GetPointerClass(), 0, 0);
    *rval = OBJECT_TO_JSVAL(obj);
    JsciPointer *ptr = new JsciPointer;
    ptr->ptr = *(void **)p;
    ptr->type = ((JsciTypePointer *) type)->direct;
    ptr->finalize = 0;
    JS_SetPrivate(cx, obj, ptr);
    return 1;
  }

  case INTTYPE:
  {
    int tmpint;
    // Return a number from an int (of various sizes)
    switch(((JSX_TypeNumeric *) type)->size) {

    case 0:
      tmpint=*(char *)p;
      break;

    case 1:
      tmpint=*(short *)p;
      break;

    case 2:
      tmpint=*(int *)p;
      break;

    case 3:
      tmpint=*(long *)p;
      break;

    case 4:
      tmpint=*(long long *)p;
      break;

    case 5:
      tmpint=*(int64 *)p;
      break;

    }

    if (INT_FITS_IN_JSVAL(tmpint))
      *rval=INT_TO_JSVAL(tmpint);
    else {
      jsdouble d=(jsdouble)tmpint;
      JS_NewDoubleValue(cx, d, rval);
    }

    return 1;
  }

  case BITFIELDTYPE:

    return JSX_Get(cx, p, ((JsciTypeBitfield *) type)->member, rval);

  case UINTTYPE:
  {
    int tmpuint;
    // Return a number from an unsigned int (of various sizes)
    switch(((JSX_TypeNumeric *) type)->size) {

    case 0:
      tmpuint=*(unsigned char *)p;
      break;

    case 1:
      tmpuint=*(unsigned short *)p;
      break;

    case 2:
      tmpuint=*(unsigned int *)p;
      break;

    case 3:
      tmpuint=*(unsigned long *)p;
      break;

    case 4:
      tmpuint=*(unsigned long long *)p;
      break;

    case 5:
      tmpuint=*(int64 *)p;
      break;

    default:
      goto failure;

    }

    if (INT_FITS_IN_JSVAL(tmpuint))
      *rval=INT_TO_JSVAL(tmpuint);
    else {
      jsdouble d=(jsdouble)tmpuint;
      JS_NewDoubleValue(cx, d, rval);
    }

    return 1;
  }

  case FLOATTYPE:
  {
    jsdouble tmpdouble;

    // Return a number from a float (of various sizes)

    switch(((JSX_TypeNumeric *) type)->size) {

    case 0:
      tmpdouble=*(float *)p;
      break;

    case 1:
      tmpdouble=*(double *)p;
      break;

      /*
    case 2:
      tmpdouble=*(long double *)p;
      break;
      */

    }

    JS_NewDoubleValue(cx, tmpdouble, rval);

    return 1;
  }

  case FUNCTIONTYPE:
  {
    // Create a new JS function which calls a C function
    JSFunction *fun = JS_NewFunction(cx, JSX_NativeFunction, ((JsciTypeFunction *) type)->nParam, 0, 0, "JSEXT_NATIVE");
    JSObject *funobj = JS_GetFunctionObject(fun);
    *rval=OBJECT_TO_JSVAL(funobj);
    return -1;
  }

  case ARRAYTYPE:
  {
    JsciTypeArray *ta = (JsciTypeArray *) type;

    if(type_is_char(ta->member)) {
      // Return a string from a char array
      *rval = STRING_TO_JSVAL(JS_NewStringCopyN(cx, (char *) p, ta->length));
      return 1;
    }

    *rval=OBJECT_TO_JSVAL(JS_NewArrayObject(cx, 0, 0));
    int fieldsize = ta->SizeInBytes();
    JSObject *obj = JSVAL_TO_OBJECT(*rval);
    jsval tmp=JSVAL_VOID;
    JS_AddRoot(cx, &tmp);
    for(int i = 0; i != ta->length; i++) {
      int ok = JSX_Get(cx, p + i * fieldsize, ta->member, &tmp);
      if(!ok) {
        JS_RemoveRoot(cx, &tmp);
        goto failure;
      }
      JS_SetElement(cx, obj, i, &tmp);
    }
    JS_RemoveRoot(cx, &tmp);
    return 1;
  }

  case STRUCTTYPE:
  case UNIONTYPE:
  {
    *rval=OBJECT_TO_JSVAL(JS_NewObject(cx, 0, 0, 0));
    JSObject *obj = JSVAL_TO_OBJECT(*rval);

    JsciTypeStructUnion *tsu = (JsciTypeStructUnion *) type;
    jsval tmp=JSVAL_VOID;
    JS_AddRoot(cx, &tmp);

    for(int i = 0; i != tsu->nMember; ++i) {
      JSX_SuMember mtype = tsu->member[i];
      JS_GetProperty(cx, obj, mtype.name, &tmp);
      int ok = JSX_Get(cx, p + mtype.offset / 8, mtype.membertype, &tmp);
      if(!ok) {
        JS_RemoveRoot(cx, &tmp);
        goto failure;
      }
      if(mtype.membertype->type == BITFIELDTYPE) {
        int length = ((JsciTypeBitfield *) mtype.membertype)->length;
        int offset = mtype.offset % 8;
        int mask = ~(-1 << length);
        tmp = INT_TO_JSVAL((JSVAL_TO_INT(tmp) >> offset) & mask);
      }
      JS_SetProperty(cx, obj, mtype.name, &tmp);
    }

    JS_RemoveRoot(cx, &tmp);

    return 1;
  }

  default:
    goto failure;
  }

 failure:
  JSX_ReportException(cx, "Get: Could not convert C value of type %s to JS", JSX_typenames[type->type]);
  return 0;
}
