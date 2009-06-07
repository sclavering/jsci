#include <string.h>
#include "jsci.h"


// rval should be rooted
// if p is NULL, type must also be NULL. Nothing is done, only size is returned.

int JSX_Get(JSContext *cx, char *p, int do_clean, JSX_Type *type, jsval *rval) {
  if(!type) return JSX_ReportException(cx, "Cannot convert C value to JS value, because the C type is not known");

  int size=-1;
  
  // Determine the appropriate conversion

  switch(type->type) {

  case POINTERTYPE:
  {
    // Return a pointer object from a type *
    if (do_clean!=2) {
      if(*(void **)p == NULL) {
        *rval = JSVAL_NULL;
      } else {
        JSObject *obj = JS_NewObject(cx, JSX_GetPointerClass(), 0, 0);
        *rval = OBJECT_TO_JSVAL(obj);
        JsciPointer *ptr = new JsciPointer;
        ptr->ptr = *(void **)p;
        ptr->type = ((JSX_TypePointer *) type)->direct;
        ptr->finalize = 0;
        JS_SetPrivate(cx, obj, ptr);
      }
    }
    return sizeof(void *);
  }

  case INTTYPE:
  {
    int tmpint;
    // Return a number from an int (of various sizes)
    switch(size != -1 ? size : ((JSX_TypeNumeric *) type)->size) {

    case 0:
      tmpint=*(char *)p;
      size=sizeof(char);
      break;

    case 1:
      tmpint=*(short *)p;
      size=sizeof(short);
      break;

    case 2:
      tmpint=*(int *)p;
      size=sizeof(int);
      break;

    case 3:
      tmpint=*(long *)p;
      size=sizeof(long);
      break;

    case 4:
      tmpint=*(long long *)p;
      size=sizeof(long long);
      break;

    case 5:
      tmpint=*(int64 *)p;
      size=sizeof(int64);
      break;

    }

    if (INT_FITS_IN_JSVAL(tmpint))
      *rval=INT_TO_JSVAL(tmpint);
    else {
      jsdouble d=(jsdouble)tmpint;
      JS_NewDoubleValue(cx, d, rval);
    }

    return size;
  }

  case BITFIELDTYPE:

    size = ((JSX_TypeNumeric *) ((JSX_TypeBitfield *) type)->member)->size;

    // fall through

  case UINTTYPE:
  {
    int tmpuint;
    // Return a number from an unsigned int (of various sizes)
    switch(size != -1 ? size : ((JSX_TypeNumeric *) type)->size) {

    case 0:
      tmpuint=*(unsigned char *)p;
      size=sizeof(unsigned char);
      break;

    case 1:
      tmpuint=*(unsigned short *)p;
      size=sizeof(unsigned short);
      break;

    case 2:
      tmpuint=*(unsigned int *)p;
      size=sizeof(unsigned int);
      break;

    case 3:
      tmpuint=*(unsigned long *)p;
      size=sizeof(unsigned long);
      break;

    case 4:
      tmpuint=*(unsigned long long *)p;
      size=sizeof(unsigned long long);
      break;

    case 5:
      tmpuint=*(int64 *)p;
      size=sizeof(int64);
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

    return size;
  }

  case FLOATTYPE:
  {
    jsdouble tmpdouble;

    // Return a number from a float (of various sizes)

    switch(size != -1 ? size : ((JSX_TypeNumeric *) type)->size) {

    case 0:
      tmpdouble=*(float *)p;
      size=sizeof(float);
      break;

    case 1:
      tmpdouble=*(double *)p;
      size=sizeof(double);
      break;

      /*
    case 2:
      tmpdouble=*(long double *)p;
      size=sizeof(long double);
      break;
      */

    }

    JS_NewDoubleValue(cx, tmpdouble, rval);

    return size;
  }

  case FUNCTIONTYPE:
  {
    // Create a new JS function which calls a C function
    JSFunction *fun = JS_NewFunction(cx, JSX_NativeFunction, ((JSX_TypeFunction *) type)->nParam, 0, 0, "JSEXT_NATIVE");
    JSObject *funobj = JS_GetFunctionObject(fun);
    *rval=OBJECT_TO_JSVAL(funobj);
    return -1;
    // who knows the size of a function, really?
    // anyway, functions will only ever be returned
    // from a get() operation, so the size doesn't matter
    // as long as it is non-null.
  }

  case ARRAYTYPE:
  {
    if(type_is_char(((JSX_TypeArray *) type)->member)) {
      // Return a string from a char array
      *rval = STRING_TO_JSVAL(JS_NewStringCopyN(cx, (char *) p, ((JSX_TypeArray *) type)->length));
      return sizeof(char) * ((JSX_TypeArray *) type)->length;
    }

    // Create new array and populate with values

    *rval=OBJECT_TO_JSVAL(JS_NewArrayObject(cx, 0, 0));
    
    int i;
    // Update array elements from a fixed size array
    int totsize = 0;
    size = ((JSX_TypeArray *) type)->length;
    JSObject *obj = JSVAL_TO_OBJECT(*rval);

    jsval tmp=JSVAL_VOID;
    JS_AddRoot(cx, &tmp);
    for (i=0; i<size; i++) {
      JS_GetElement(cx, obj, i, &tmp);
      int thissize = JSX_Get(cx, p + totsize, do_clean, ((JSX_TypeArray *) type)->member, &tmp);
      if(!thissize) {
        JS_RemoveRoot(cx, &tmp);
        goto fixedarrayfailure;
      }
      if(do_clean != 2) JS_SetElement(cx, obj, i, &tmp);
      totsize += thissize;
    }
    JS_RemoveRoot(cx, &tmp);

    return totsize;

  fixedarrayfailure:
    if (do_clean) {
      int elemsize = ((JSX_TypeArray *) type)->member->SizeInBytes();
      for (;++i<size;) {
        jsval tmp;
        JS_GetElement(cx, obj, i, &tmp);
        JSX_Get(cx, p + i * elemsize, 2, ((JSX_TypeArray *) type)->member, &tmp);
      }
    }
    goto failure;
    // Error already thrown
  }

  case STRUCTTYPE:
  case UNIONTYPE:
  {
    // Create new object and populate with values

    *rval=OBJECT_TO_JSVAL(JS_NewObject(cx, 0, 0, 0));

    JsciTypeStructUnion *tsu = (JsciTypeStructUnion *) type;
    int i;
    // Update object elements from a struct or union
    size = tsu->nMember;
    JSObject *obj = JSVAL_TO_OBJECT(*rval);

    jsval tmp=JSVAL_VOID;
    JS_AddRoot(cx, &tmp);

    for (i=0; i<size; i++) {
      JSX_SuMember mtype = tsu->member[i];
      JS_GetProperty(cx, obj, mtype.name, &tmp);
      int thissize = JSX_Get(cx, p + mtype.offset / 8, do_clean, mtype.membertype, &tmp);
      if(!thissize) {
        goto structfailure;
        JS_RemoveRoot(cx, &tmp);
      }
      if(do_clean != 2) {
        if(mtype.membertype->type == BITFIELDTYPE) {
          int length = ((JSX_TypeBitfield *) mtype.membertype)->length;
          int offset = mtype.offset % 8;
          int mask = ~(-1 << length);
          tmp = INT_TO_JSVAL((JSVAL_TO_INT(tmp) >> offset) & mask);
        }
        JS_SetProperty(cx, obj, mtype.name, &tmp);
      }
    }

    JS_RemoveRoot(cx, &tmp);

    return type->SizeInBytes();
    
  structfailure:
    if (do_clean) {
      for (;++i<size;) {
        jsval tmp;
        JSX_SuMember mtype = tsu->member[i];
        JS_GetProperty(cx, obj, mtype.name, &tmp);
        JSX_Get(cx, p + mtype.offset / 8, 2, mtype.membertype, &tmp);
      }
    }
    goto failure;
    // Error already thrown
  }

  default:

    // Could not find appropriate conversion

    if (do_clean==2) // Okay if we are not going to use the value
      return type->SizeInBytes();

    goto failure;

  }

 failure:
  JSX_ReportException(cx, "Get: Could not convert C value of type %s to JS", JSX_typenames[type->type]);
  return 0;
}


int JSX_Get_multi(JSContext *cx, JSX_TypeFunction *funct, jsval *rval, void **argptr) {
  int ret = 0;
  for(int i = 0; i < funct->nParam; i++) {
    JSX_FuncParam *thistype = &funct->param[i];
    if(thistype->paramtype->type == ARRAYTYPE) return 0; // xxx why don't we just treat it as a pointer type?
    int siz = JSX_Get(cx, (char*) *argptr, 0, thistype->paramtype, rval);
    if(!siz) return 0;
    ret += siz;
  }
  return ret;
}
