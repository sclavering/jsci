#include <string.h>
#include "jsci.h"


// Setting to undefined does nothing, only returns sizeof.
// Setting to null zeroes memory.

int JSX_Set(JSContext *cx, char *p, int will_clean, JSX_Type *type, jsval v) {
  if(!type) return JSX_ReportException(cx, "Cannot convert JS value to C value, because the C type is not known");

  int size=-1;
  int tmpint;
  int totsize;
  int i;
  JSObject *obj;
  JsciPointer *ptr;
  jsdouble tmpdbl;
  jsval tmpval;
  JSFunction *fun;

  int typepair = TYPEPAIR(JSX_JSType(cx, v), type->type);

  // Determine the appropriate conversion

  switch(typepair) {
  case TYPEPAIR(JSFUNC,POINTERTYPE):
    if(((JSX_TypePointer *) type)->direct->type != FUNCTIONTYPE)
      goto failure;

    fun=JS_ValueToFunction(cx,v);
    obj=JS_GetFunctionObject(fun);
    JS_GetProperty(cx, obj, "__ptr__", &tmpval);
    if (tmpval==JSVAL_VOID) {
      // Create pointer
      JSObject *newptr=JS_NewObject(cx, JSX_GetPointerClass(), 0, 0);
      tmpval=OBJECT_TO_JSVAL(newptr);
      JS_DefineProperty(cx, obj, "__ptr__", tmpval, 0, 0, JSPROP_READONLY | JSPROP_PERMANENT);

      if(!JSX_InitPointerCallback(cx, newptr, fun, ((JSX_TypePointer *) type)->direct)) {
        return 0;
      }
    }
    v=tmpval;
    goto pointercommon;

  case TYPEPAIR(JSPOINTER,INTTYPE):
  case TYPEPAIR(JSPOINTER,UINTTYPE):

    if(type->SizeInBytes() != sizeof(void*)) goto failure;

    // Copy a pointer object to an int

    // fall through

  case TYPEPAIR(JSPOINTER,POINTERTYPE):

    // Copy a pointer object to a type *

  pointercommon:

    ptr = (JsciPointer *) JS_GetPrivate(cx, JSVAL_TO_OBJECT(v));
    *(void **)p=ptr->ptr;
    return sizeof(void *);

  case TYPEPAIR(JSNULL,POINTERTYPE):

    // Copy a null to a type *

    *(void **)p=NULL;

    // fall through

  case TYPEPAIR(JSVOID,POINTERTYPE):
  case TYPEPAIR(JSVOID,FUNCTIONTYPE):

    // Do nothing

    return sizeof(void *);

  case TYPEPAIR(JSVAL_STRING,POINTERTYPE):
    
    if(!is_void_or_char(((JSX_TypePointer *) type)->direct)) goto failure;

    // Copy a string to a void *
    // same as char * in this context

    if (!will_clean)
      goto failure;

    *(char **)p=JS_GetStringBytes(JSVAL_TO_STRING(v));
    return sizeof(char *);

  case TYPEPAIR(JSVAL_STRING,INTTYPE):
  case TYPEPAIR(JSVAL_STRING,UINTTYPE):

    switch(((JSX_TypeNumeric *) type)->size) {
    case 0:

      // Copy a string to a char

      *(char *)p=JS_GetStringBytes(JSVAL_TO_STRING(v))[0];
      return sizeof(char);

    case 1:

      // Copy a string to a short

      *(jschar *)p=JS_GetStringChars(JSVAL_TO_STRING(v))[0];
      return sizeof(jschar);

    default:
      goto failure;
    }

  case TYPEPAIR(JSVAL_STRING,ARRAYTYPE):
    if(!type_is_char(((JSX_TypeArray *) type)->member)) goto failure;

    // Copy a string to a char array

    size=JS_GetStringLength(JSVAL_TO_STRING(v));
    if(size < ((JSX_TypeArray *) type)->length) {
      memcpy(*(char **)p, JS_GetStringBytes(JSVAL_TO_STRING(v)), size * sizeof(char));
      memset(*(char **)p + size, 0, (((JSX_TypeArray *) type)->length - size) * sizeof(char));
    } else {
      memcpy(*(char **)p, JS_GetStringBytes(JSVAL_TO_STRING(v)), ((JSX_TypeArray *) type)->length * sizeof(char));
    }
    return sizeof(char) * ((JSX_TypeArray *) type)->length;

  case TYPEPAIR(JSVAL_INT,POINTERTYPE):

    // Copy a number to an undefined type or type *
    // assume int

    size=2;

    // fall through

  case TYPEPAIR(JSVAL_INT,INTTYPE):
  case TYPEPAIR(JSVAL_INT,UINTTYPE):

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
    size = ((JSX_TypeNumeric *) ((JSX_TypeBitfield *) type)->member)->size;
    goto intcommon;

  case TYPEPAIR(JSVAL_BOOLEAN,INTTYPE):
  case TYPEPAIR(JSVAL_BOOLEAN,UINTTYPE):

    tmpint=v==JSVAL_TRUE?1:0;
    goto intcommon;

  case TYPEPAIR(JSVAL_DOUBLE,POINTERTYPE):

    // Copy a number to a type *
    // assume int

    size=2;

    // fall through

  case TYPEPAIR(JSVAL_DOUBLE,INTTYPE):
  case TYPEPAIR(JSVAL_DOUBLE,UINTTYPE):

    tmpint=(int)*JSVAL_TO_DOUBLE(v);

  intcommon:

    // Return a number from an int (of various sizes)
    switch(size != -1 ? size : ((JSX_TypeNumeric *) type)->size) {

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

    tmpdbl = *JSVAL_TO_DOUBLE(v);
    goto floatcommon;

  case TYPEPAIR(JSVAL_BOOLEAN,FLOATTYPE):

    tmpdbl = v==JSVAL_TRUE?1.:0.;
    goto floatcommon;

  case TYPEPAIR(JSVAL_INT,FLOATTYPE):

    tmpdbl = (jsdouble) JSVAL_TO_INT(v);

    // Copy a number to a float (of various sizes)

  floatcommon:

    switch(size != -1 ? size : ((JSX_TypeNumeric *) type)->size) {

    case 0:
      *(float *)p=tmpdbl;
      size=sizeof(float);
      break;

    case 1:
      *(double *)p=tmpdbl;
      size=sizeof(double);
      break;

      /*
    case 2:
      *(long double *)p=tmpdbl;
      size=sizeof(long double);
      break;
      */

    }

    return size;

  case TYPEPAIR(JSARRAY,POINTERTYPE):
  {
    // Copy array elements to a variable array
      
      int containsPointers=0;
      obj=JSVAL_TO_OBJECT(v);
      JS_GetArrayLength(cx, obj, (jsuint*) &size);
      int elemsize = ((JSX_TypeArray *) type)->member->SizeInBytes();

      if (will_clean) {
        // The variable array needs to be allocated
        containsPointers = ((JSX_TypeArray *) type)->member->ContainsPointer();
        if(containsPointers) {
          // Allocate twice the space in order to store old pointers
          *(void **)p = JS_malloc(cx, elemsize * size * 2);
        } else {
          *(void **)p = JS_malloc(cx, elemsize * size);
        }
      }
      
      totsize=0;
    
      for (i=0; i<size; i++) {
        jsval tmp;
        JS_GetElement(cx, obj, i, &tmp);
        int thissize = JSX_Set(cx, *(char **)p + totsize, will_clean, ((JSX_TypePointer *) type)->direct, tmp);
        if(!thissize) goto vararrayfailure2;
        totsize += thissize;
      }

      // Make backup of old pointers
      if(containsPointers) memcpy(*(char **)p + elemsize * size, *(char **)p, elemsize * size);
      
      return sizeof(void *);

    vararrayfailure2:
      if (will_clean) {
        JS_free(cx, *(void **)p);
      }
    
      return 0;
  }

  case TYPEPAIR(JSARRAY,ARRAYTYPE):
  {
    // Copy array elements to a fixed size array
    
    totsize=0;
    size = ((JSX_TypeArray *) type)->length;
    obj=JSVAL_TO_OBJECT(v);

    for (i=0; i<size; i++) {
      jsval tmp;
      JS_GetElement(cx, obj, i, &tmp);
      int thissize = JSX_Set(cx, p + totsize, will_clean, ((JSX_TypeArray *) type)->member, tmp);
      if(!thissize) return 0;
      totsize+=thissize;
    }

    return totsize;
  }

  case TYPEPAIR(JSVAL_OBJECT,STRUCTTYPE):
  case TYPEPAIR(JSVAL_OBJECT,UNIONTYPE):
  {
    // Copy object elements to a struct or union
    JsciTypeStructUnion *tsu = (JsciTypeStructUnion *) type;
    obj=JSVAL_TO_OBJECT(v);
    size = tsu->nMember;

    for (i=0; i<size; i++) {
      jsval tmp;
      int thissize;

      JS_GetProperty(cx, obj, tsu->member[i].name, &tmp);

      if(tsu->member[i].membertype->type == BITFIELDTYPE) {
        int length = ((JSX_TypeBitfield *) tsu->member[i].membertype)->length;
        int offset = tsu->member[i].offset % 8;
        int mask = ~(-1 << length);
        int imask = ~(imask << offset);
        int tmpint;
        int tmpint2;
        thissize = JSX_Set(cx, (char *) &tmpint, will_clean, tsu->member[i].membertype, tmp);
        memcpy((char *) &tmpint2, p + tsu->member[i].offset / 8, thissize);
        tmpint = (tmpint2 & imask) | ((tmpint & mask) << offset);
        memcpy(p + tsu->member[i].offset / 8, (char *) &tmpint, thissize);
      } else {
        thissize = JSX_Set(cx, p + tsu->member[i].offset / 8, will_clean, tsu->member[i].membertype, tmp);
      }
      if(!thissize) return 0;
    }

    return type->SizeInBytes();
  }

  case TYPEPAIR(JSARRAY,STRUCTTYPE):
  case TYPEPAIR(JSARRAY,UNIONTYPE):
  {
    JsciTypeStructUnion *tsu = (JsciTypeStructUnion *) type;
    // Copy array elements to struct or union
    obj=JSVAL_TO_OBJECT(v);
    size = tsu->nMember;

    for (i=0; i<size; i++) {
      jsval tmp;
      JS_GetElement(cx, obj, i, &tmp);
      int thissize = JSX_Set(cx, p + tsu->member[i].offset / 8, will_clean, tsu->member[i].membertype, tmp);
      if(!thissize) return 0;
    }

    return type->SizeInBytes();
  }

  case TYPEPAIR(JSNULL,STRUCTTYPE):
  case TYPEPAIR(JSNULL,UNIONTYPE):
  case TYPEPAIR(JSNULL,ARRAYTYPE):
  case TYPEPAIR(JSNULL,INTTYPE):
  case TYPEPAIR(JSNULL,UINTTYPE):
  case TYPEPAIR(JSNULL,FLOATTYPE):

    // Initialize with zero

    size = type->SizeInBytes();
    memset(p, 0, size);
    return size;

    break;

  case TYPEPAIR(JSVOID,STRUCTTYPE):
  case TYPEPAIR(JSVOID,UNIONTYPE):
  case TYPEPAIR(JSVOID,ARRAYTYPE):
  case TYPEPAIR(JSVOID,INTTYPE):
  case TYPEPAIR(JSVOID,UINTTYPE):
  case TYPEPAIR(JSVOID,FLOATTYPE):

    // Do nothing

    return type->SizeInBytes();

  case TYPEPAIR(JSPOINTER,ARRAYTYPE):

    // Copy contents pointed to into array
    size = type->SizeInBytes();
    ptr = (JsciPointer *) JS_GetPrivate(cx, JSVAL_TO_OBJECT(v));
    memcpy(p, ptr->ptr, size);
    return size;

  default:
    // Could not find appropriate conversion

    goto failure;

  }

 failure:
  JSX_ReportException(cx, "Set: Could not convert value from JS %s to C %s",JSX_jstypenames[typepair/TYPECOUNT2],JSX_typenames[typepair%TYPECOUNT2]);
  return 0;
}


JSBool JSX_Set_multi(JSContext *cx, char *ptr, int will_clean, JSX_TypeFunction *funct, jsval *vp, void **argptr) {
  int cursiz;
  for(int i = 0; i < funct->nParam; ++i) {
    JSX_Type *t = funct->param[i];

    if(t->type == ARRAYTYPE) {
      if(!will_clean) return JS_FALSE;
      // In function calls, arrays are passed by pointer
      *(void **)ptr = JS_malloc(cx, t->SizeInBytes());
      cursiz = JSX_Set(cx, (char*) *(void **)ptr, will_clean, t, *vp);
      if(cursiz) {
        cursiz = sizeof(void *);
      } else {
        JS_free(cx, *(void **)ptr);
        return JS_FALSE;
      }
    } else {
      cursiz = JSX_Set(cx, (char*) (ptr ? ptr : *argptr), will_clean, t, *vp);
    }
    if(!cursiz) return JS_FALSE;

    if (ptr) {
      *(argptr++)=ptr;
      ptr += cursiz;
    } else {
      argptr++;
    }
    vp++;
  }

  return JS_TRUE;
}
