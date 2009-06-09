#include <string.h>
#include "jsci.h"


// Setting to undefined does nothing, only returns sizeof.
// Setting to null zeroes memory.

int JSX_Set(JSContext *cx, char *p, int will_clean, JsciType *type, jsval v) {
  if(!type) return JSX_ReportException(cx, "Cannot convert JS value to C value, because the C type is not known");

  int size=-1;
  int tmpint;
  int totsize;
  int i;
  JSObject *obj;
  JsciPointer *ptr;
  jsval tmpval;
  JSFunction *fun;

  int typepair = TYPEPAIR(JSX_JSType(cx, v), type->type);

  // Determine the appropriate conversion

  switch(typepair) {
  case TYPEPAIR(JSFUNC,POINTERTYPE):
    if(((JsciTypePointer *) type)->direct->type != FUNCTIONTYPE)
      goto failure;

    fun=JS_ValueToFunction(cx,v);
    obj=JS_GetFunctionObject(fun);
    JS_GetProperty(cx, obj, "__ptr__", &tmpval);
    if (tmpval==JSVAL_VOID) {
      // Create pointer
      JSObject *newptr=JS_NewObject(cx, JSX_GetPointerClass(), 0, 0);
      tmpval=OBJECT_TO_JSVAL(newptr);
      JS_DefineProperty(cx, obj, "__ptr__", tmpval, 0, 0, JSPROP_READONLY | JSPROP_PERMANENT);

      if(!JSX_InitPointerCallback(cx, newptr, fun, ((JsciTypePointer *) type)->direct)) {
        return 0;
      }
    }
    v=tmpval;
    goto pointercommon;

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
    
    if(!is_void_or_char(((JsciTypePointer *) type)->direct)) goto failure;

    // Copy a string to a void *
    // same as char * in this context

    if (!will_clean)
      goto failure;

    *(char **)p=JS_GetStringBytes(JSVAL_TO_STRING(v));
    return sizeof(char *);

  case TYPEPAIR(JSVAL_STRING,INTTYPE):

    switch(((JsciTypeNumeric *) type)->size) {
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
  {
    JsciTypeArray *ta = (JsciTypeArray *) type;
    if(!type_is_char(ta->member)) goto failure;

    // Copy a string to a char array

    size=JS_GetStringLength(JSVAL_TO_STRING(v));
    if(size < ta->length) {
      memcpy(*(char **)p, JS_GetStringBytes(JSVAL_TO_STRING(v)), size * sizeof(char));
      memset(*(char **)p + size, 0, (ta->length - size) * sizeof(char));
    } else {
      memcpy(*(char **)p, JS_GetStringBytes(JSVAL_TO_STRING(v)), ta->length * sizeof(char));
    }
    return sizeof(char) * ta->length;
  }

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

  case TYPEPAIR(JSVAL_DOUBLE,POINTERTYPE):

    // Copy a number to a type *
    // assume int

    size=2;

    // fall through

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
    return ((JsciTypeFloat *) type)->JStoC(cx, p, v, will_clean);

  case TYPEPAIR(JSARRAY,POINTERTYPE):
  {
    // Copy array elements to a variable array
      JsciTypePointer *tp = (JsciTypePointer *) type;
      int containsPointers=0;
      obj=JSVAL_TO_OBJECT(v);
      JS_GetArrayLength(cx, obj, (jsuint*) &size);
      int elemsize = tp->direct->SizeInBytes();

      if (will_clean) {
        // The variable array needs to be allocated
        containsPointers = tp->direct->ContainsPointer();
        if(containsPointers) {
          // Allocate twice the space in order to store old pointers
          *(void **)p = new char[elemsize * size * 2];
        } else {
          *(void **)p = new char[elemsize * size];
        }
      }
      
      totsize=0;
    
      for (i=0; i<size; i++) {
        jsval tmp;
        JS_GetElement(cx, obj, i, &tmp);
        int thissize = JSX_Set(cx, *(char **)p + totsize, will_clean, tp->direct, tmp);
        if(!thissize) goto vararrayfailure2;
        totsize += thissize;
      }

      // Make backup of old pointers
      if(containsPointers) memcpy(*(char **)p + elemsize * size, *(char **)p, elemsize * size);
      
      return sizeof(void *);

    vararrayfailure2:
      if(will_clean) delete p;
      return 0;
  }

  case TYPEPAIR(JSARRAY,ARRAYTYPE):
  {
    // Copy array elements to a fixed size array
    JsciTypeArray *ta = (JsciTypeArray *) type;
    
    totsize=0;
    size = ta->length;
    obj=JSVAL_TO_OBJECT(v);

    for (i=0; i<size; i++) {
      jsval tmp;
      JS_GetElement(cx, obj, i, &tmp);
      int thissize = JSX_Set(cx, p + totsize, will_clean, ta->member, tmp);
      if(!thissize) return 0;
      totsize+=thissize;
    }

    return totsize;
  }

  case TYPEPAIR(JSVAL_OBJECT,SUTYPE):
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
        int length = ((JsciTypeBitfield *) tsu->member[i].membertype)->length;
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

  case TYPEPAIR(JSNULL,SUTYPE):
  case TYPEPAIR(JSNULL,ARRAYTYPE):
  case TYPEPAIR(JSNULL,INTTYPE):
  case TYPEPAIR(JSNULL,FLOATTYPE):

    // Initialize with zero

    size = type->SizeInBytes();
    memset(p, 0, size);
    return size;

  case TYPEPAIR(JSVOID,SUTYPE):
  case TYPEPAIR(JSVOID,ARRAYTYPE):
  case TYPEPAIR(JSVOID,INTTYPE):
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
