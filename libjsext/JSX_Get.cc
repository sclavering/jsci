#include <string.h>
#include "jsci.h"


// rval should be rooted
// if p is NULL, type must also be NULL. Nothing is done, only size is returned.

int JSX_Get(JSContext *cx, char *p, char *oldptr, int do_clean, JSX_Type *type, jsval *rval) {
  if(!type) return JSX_ReportException(cx, "Cannot convert C value to JS value, because the C type is not known");

  int size=-1;
  int typepair = TYPEPAIR(JSX_JSType(cx, *rval), type->type);
  
  // Determine the appropriate conversion

  switch(typepair) {
  case TYPEPAIR(JSFUNC,POINTERTYPE):
    if(((JSX_TypePointer *) type)->direct->type != FUNCTIONTYPE)
      goto failure;
    return sizeof(void *);

  case TYPEPAIR(JSPOINTER,INTTYPE):
  case TYPEPAIR(JSPOINTER,UINTTYPE):
    if(type->SizeInBytes() != sizeof(void*)) goto failure;

    // Update pointer object from an int

    // Fall through

  case TYPEPAIR(JSPOINTER,POINTERTYPE):

    // Update pointer object from a type * or int

    if (do_clean!=2) {
      if(*(void **)p == NULL) {
        *rval = JSVAL_NULL;
      } else {
        JSX_Pointer *ptr = (JSX_Pointer *) JS_GetPrivate(cx, JSVAL_TO_OBJECT(*rval));
        ptr->ptr = *(void **)p;
      }
    }
    return sizeof(void *);

  case TYPEPAIR(JSVOID,POINTERTYPE):
  case TYPEPAIR(JSNULL,POINTERTYPE):

    // Return a pointer object from a type *
    if (do_clean!=2) {
      if(*(void **)p == NULL) {
        *rval = JSVAL_NULL;
      } else {
        JSObject *obj = JS_NewObject(cx, JSX_GetPointerClass(), 0, 0);
        *rval = OBJECT_TO_JSVAL(obj);
        JSX_Pointer *ptr = new JSX_Pointer;
        ptr->ptr = *(void **)p;
        ptr->type = ((JSX_TypePointer *) type)->direct;
        ptr->finalize = 0;
        JS_SetPrivate(cx, obj, ptr);
      }
    }
    return sizeof(void *);

  case TYPEPAIR(JSVAL_STRING,POINTERTYPE):
    if(!is_void_or_char(((JSX_TypePointer *) type)->direct)) goto failure;

    // Return a string from a void* or char* (equivalent in this context)

    if (do_clean!=2)
      *rval=STRING_TO_JSVAL(JS_NewStringCopyZ(cx, *(char **)p));
    return sizeof(char *);

  case TYPEPAIR(JSVAL_STRING,INTTYPE):
  case TYPEPAIR(JSVAL_STRING,UINTTYPE):

    switch(((JSX_TypeNumeric *) type)->size) {
    case 0:

      // Return a string from a char

      *rval=STRING_TO_JSVAL(JS_NewStringCopyN(cx, (char *)p, 1));
      return sizeof(char);

    case 1:

      // Return a string from a short

      *rval=STRING_TO_JSVAL(JS_NewUCStringCopyN(cx, (jschar *)p, 1));
      return sizeof(jschar);

    default:
      goto failure;
    }

  case TYPEPAIR(JSVAL_STRING,ARRAYTYPE):
    if(!type_is_char(((JSX_TypeArray *) type)->member)) goto failure;
  fromchararray:
    // Return a string from a char array
    *rval = STRING_TO_JSVAL(JS_NewStringCopyN(cx, (char *) p, ((JSX_TypeArray *) type)->length));
    return sizeof(char) * ((JSX_TypeArray *) type)->length;

  case TYPEPAIR(JSVAL_INT,POINTERTYPE):

    // Return a number from an undefined type
    // assume int

    if (!p)
      return sizeof(int);

    size=2;

    // fall through

  case TYPEPAIR(JSNULL,INTTYPE):
  case TYPEPAIR(JSVOID,INTTYPE):
  case TYPEPAIR(JSVAL_BOOLEAN,INTTYPE):
  case TYPEPAIR(JSVAL_INT,INTTYPE):
  case TYPEPAIR(JSVAL_DOUBLE,INTTYPE):
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
  case TYPEPAIR(JSNULL,BITFIELDTYPE):
  case TYPEPAIR(JSVOID,BITFIELDTYPE):
  case TYPEPAIR(JSVAL_BOOLEAN,BITFIELDTYPE):
  case TYPEPAIR(JSVAL_INT,BITFIELDTYPE):
  case TYPEPAIR(JSVAL_DOUBLE,BITFIELDTYPE):

    size = ((JSX_TypeNumeric *) ((JSX_TypeBitfield *) type)->member)->size;

    // fall through

  case TYPEPAIR(JSNULL,UINTTYPE):
  case TYPEPAIR(JSVOID,UINTTYPE):
  case TYPEPAIR(JSVAL_BOOLEAN,UINTTYPE):
  case TYPEPAIR(JSVAL_INT,UINTTYPE):
  case TYPEPAIR(JSVAL_DOUBLE,UINTTYPE):
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

  case TYPEPAIR(JSVOID,FLOATTYPE):
  case TYPEPAIR(JSNULL,FLOATTYPE):
  case TYPEPAIR(JSVAL_BOOLEAN,FLOATTYPE):
  case TYPEPAIR(JSVAL_INT,FLOATTYPE):
  case TYPEPAIR(JSVAL_DOUBLE,FLOATTYPE):
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

  case TYPEPAIR(JSVOID,FUNCTIONTYPE):
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

  case TYPEPAIR(JSARRAY,POINTERTYPE):
  {
    int totsize;
    int i;
    jsval tmp = JSVAL_VOID;

    // Update array elements from a variable array

    JSObject *obj = JSVAL_TO_OBJECT(*rval);
    JS_GetArrayLength(cx, obj, (jsuint*) &size);
    int elemsize = ((JSX_TypeArray *) type)->member->SizeInBytes();

    if (do_clean) {
      if (oldptr && (do_clean==2 || *(void **)oldptr!=*(void **)p)) {
        // Pointer has changed, don't update array, just return pointer and clean up
        if(do_clean != 2) {
          *rval = JSVAL_VOID;
          JSX_Get(cx, p, 0, 0, type, rval);
        }

        totsize = 0;

        for (i=0; i<size; i++) {
          jsval tmp;
          JS_GetElement(cx, obj, i, &tmp);
          int thissize = JSX_Get(cx, 0, *(char **)oldptr + totsize, 2, ((JSX_TypePointer *) type)->direct, &tmp);
          if(!thissize) goto vararrayfailure1;
          totsize+=thissize;
        }

        JS_free(cx, *(char **)p);
        return sizeof(void *);
      }

      if(((JSX_TypeArray *) type)->member->ContainsPointer()) {
        oldptr = *(char **)p + elemsize * size;
      } else {
        oldptr = 0;
      }
    }

    totsize=0;

    JS_AddRoot(cx, &tmp);
    for (i=0; i<size; i++) {
      JS_GetElement(cx, obj, i, &tmp);
      int thissize = JSX_Get(cx, *(char **)p + totsize, oldptr ? *(char **)oldptr + totsize : 0, do_clean, ((JSX_TypePointer *) type)->direct, &tmp);
      if(!thissize) {
        JS_RemoveRoot(cx, &tmp);
        goto vararrayfailure1;
      }
      if(do_clean != 2) JS_SetElement(cx, obj, i, &tmp);
      totsize += thissize;
    }
    JS_RemoveRoot(cx, &tmp);

    if(do_clean) JS_free(cx, *(char **)p);

    return sizeof(void *);

  vararrayfailure1:
    if (do_clean) {
      int elemsize = ((JSX_TypePointer *) type)->direct->SizeInBytes();
      for (;++i<size;) {
        jsval tmp;
        JS_GetElement(cx, obj, i, &tmp);
        JSX_Get(cx, *(char **)p + i * elemsize, oldptr ? *(char **)oldptr + i * elemsize : 0, 2, ((JSX_TypePointer *) type)->direct, &tmp);
      }

      JS_free(cx, *(char **)p);
    }

    goto failure;
    // error already thrown
  }

  case TYPEPAIR(JSVOID,ARRAYTYPE):
  case TYPEPAIR(JSNULL,ARRAYTYPE):
    if(type_is_char(((JSX_TypeArray *) type)->member)) goto fromchararray;

    // Create new array and populate with values

    *rval=OBJECT_TO_JSVAL(JS_NewArrayObject(cx, 0, 0));
    
    // fall through

  case TYPEPAIR(JSARRAY,ARRAYTYPE):
  {
    int i;
    // Update array elements from a fixed size array
    int totsize = 0;
    size = ((JSX_TypeArray *) type)->length;
    JSObject *obj = JSVAL_TO_OBJECT(*rval);

    jsval tmp=JSVAL_VOID;
    JS_AddRoot(cx, &tmp);
    for (i=0; i<size; i++) {
      JS_GetElement(cx, obj, i, &tmp);
      int thissize = JSX_Get(cx, p + totsize, oldptr ? oldptr + totsize : 0, do_clean, ((JSX_TypeArray *) type)->member, &tmp);
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
        JSX_Get(cx, p + i * elemsize, oldptr ? oldptr + i * elemsize : 0, 2, ((JSX_TypeArray *) type)->member, &tmp);
      }
    }
    goto failure;
    // Error already thrown
  }

  case TYPEPAIR(JSVOID,STRUCTTYPE):
  case TYPEPAIR(JSVOID,UNIONTYPE):
  case TYPEPAIR(JSNULL,STRUCTTYPE):
  case TYPEPAIR(JSNULL,UNIONTYPE):

    // Create new object and populate with values

    *rval=OBJECT_TO_JSVAL(JS_NewObject(cx, 0, 0, 0));

    // fall trough

  case TYPEPAIR(JSVAL_OBJECT,STRUCTTYPE):
  case TYPEPAIR(JSVAL_OBJECT,UNIONTYPE):
  {
    int i;
    // Update object elements from a struct or union
    size = ((JSX_TypeStructUnion *) type)->nMember;
    JSObject *obj = JSVAL_TO_OBJECT(*rval);

    jsval tmp=JSVAL_VOID;
    JS_AddRoot(cx, &tmp);

    for (i=0; i<size; i++) {
      JSX_SuMember mtype = ((JSX_TypeStructUnion *) type)->member[i];
      JS_GetProperty(cx, obj, mtype.name, &tmp);
      int thissize = JSX_Get(cx, p + mtype.offset / 8, oldptr ? oldptr + mtype.offset / 8 : 0, do_clean, mtype.membertype, &tmp);
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
        JSX_SuMember mtype = ((JSX_TypeStructUnion *) type)->member[i];
        JS_GetProperty(cx, obj, mtype.name, &tmp);
        JSX_Get(cx, p + mtype.offset / 8, oldptr ? oldptr + mtype.offset / 8 : 0, 2, mtype.membertype, &tmp);
      }
    }
    goto failure;
    // Error already thrown
  }

  case TYPEPAIR(JSARRAY,STRUCTTYPE):
  case TYPEPAIR(JSARRAY,UNIONTYPE):
  {
    int i;
    // Update array elements from struct or union
    size = ((JSX_TypeStructUnion *) type)->nMember;
    JSObject *obj = JSVAL_TO_OBJECT(*rval);

    jsval tmp=JSVAL_VOID;
    JS_AddRoot(cx, &tmp);

    for (i=0; i<size; i++) {
      JS_GetElement(cx, obj, i, &tmp);
      JSX_SuMember mtype = ((JSX_TypeStructUnion *) type)->member[i];
      int thissize = JSX_Get(cx, p + mtype.offset / 8, oldptr ? oldptr + mtype.offset / 8 : 0, do_clean, mtype.membertype, &tmp);
      if(!thissize) {
        JS_RemoveRoot(cx, &tmp);
        goto structfailure2;
      }
      if(do_clean != 2) JS_SetElement(cx, obj, i, &tmp);
    }

    JS_RemoveRoot(cx, &tmp);
      
    return type->SizeInBytes();

  structfailure2:
    if (do_clean) {
      for (;++i<size;) {
        jsval tmp;
        JS_GetElement(cx, obj, i, &tmp);
        JSX_SuMember mtype = ((JSX_TypeStructUnion *) type)->member[i];
        JSX_Get(cx, p + mtype.offset / 8, oldptr ? oldptr + mtype.offset / 8 : 0, 2, mtype.membertype, &tmp);
      }
    }
    goto failure;
    // error already thrown
  }

  case TYPEPAIR(JSPOINTER,ARRAYTYPE):
  {
    // Copy contents of array into memory pointed to
    // Leave pointer unchanged

    size = type->SizeInBytes();

    if (!p)
      return size;

    JSX_Pointer *ptr = (JSX_Pointer *) JS_GetPrivate(cx, JSVAL_TO_OBJECT(*rval));
    memcpy(ptr->ptr, p, size);

    return size;
  }

  default:

    // Could not find appropriate conversion

    if (do_clean==2) // Okay if we are not going to use the value
      return type->SizeInBytes();

    goto failure;

  }

 failure:
  JSX_ReportException(cx, "Get: Could not convert value from C %s to JS %s",JSX_typenames[typepair%TYPECOUNT2], JSX_jstypenames[typepair/TYPECOUNT2]);
  return 0;
}


int JSX_Get_multi(JSContext *cx, int do_clean, JSX_TypeFunction *funct, jsval *rval, int convconst, void **argptr) {
  JSX_FuncParam *type = funct->param;

  int ret=0;
  int siz;
  int i;

  for(i = 0; i < funct->nParam; i++) {
    JSX_FuncParam *thistype = &funct->param[i];

    if(!convconst && (thistype->isConst || !JSVAL_IS_OBJECT(*rval) || *rval == JSVAL_NULL)) { // Const or immutable
      if(do_clean) {
        siz = JSX_Get(cx, (char*) *argptr, 0, 2, thistype->paramtype, rval);
      } else {
        siz = thistype->paramtype->SizeInBytes();
        rval++;
      }
    } else {
      if(thistype->paramtype->type == ARRAYTYPE) {
        if(!do_clean) goto failure;
        // In function calls, arrays are passed by pointer
        siz = JSX_Get(cx, (char*) *(void **)*argptr, 0, do_clean, thistype->paramtype, rval);
        if(siz) siz = sizeof(void *);
      } else {
        siz = JSX_Get(cx, (char*) *argptr, 0, do_clean, thistype->paramtype, rval);
      }
    }

    // In function calls, arrays are passed by pointer

    if(do_clean && thistype->paramtype->type == ARRAYTYPE) {
      JS_free(cx, *(void **)*argptr);
      siz=sizeof(void *);
    }

    if (type)
      type++;
    rval++;
    if (!siz)
      goto failure;
    argptr++;
    ret+=siz;
  }

  return ret;

 failure:
  if (!do_clean)
     return 0;

  for(; ++i < funct->nParam; ) {
    JSX_Get(cx, NULL, 0, 2, type ? type->paramtype : 0, rval);
    if(type && type->paramtype->type == ARRAYTYPE) JS_free(cx, *(void **)*argptr);
    rval++;
    if (type)
      type++;
    argptr++;
  }

  return 0;
}
