#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <dlfcn.h>
#include <alloca.h>
#include "util.h"
#include "jsci.h"


typedef struct {
  void *ptr; // Points to executable code
  JSX_Type *type;
  void (*finalize) (void *);
  JSContext *cx;
  JSFunction *fun;
  void *writeable; // Points to writeable code
} JSX_Callback;

static JSBool JSX_Pointer_new(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
static void JSX_Pointer_finalize(JSContext *cx, JSObject *obj);
static JSBool JSX_Pointer_call(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
static int JSX_Get_multi(JSContext *cx, int do_clean, uintN nargs, JSX_FuncParam *type, jsval *rval, int convconst, void **argptr);
static int JSX_Set_multi(JSContext *cx, char *ptr, int will_clean, uintN nargs, JSX_FuncParam *type, jsval *vp, void **argptr);
static JSBool JSX_Pointer_getProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp);
static JSBool JSX_Pointer_setProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp);
static void JSX_Pointer_Callback(ffi_cif *cif, void *ret, void **args, void *user_data);
static JSBool JSX_NativeFunction(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
static JSBool JSX_InitPointerAlloc(JSContext *cx, JSObject *obj, JSObject *type);
static JSBool JSX_InitPointerCallback(JSContext *cx, JSObject *obj, JSFunction *fun, JSX_Type *type);


static JSClass JSX_PointerClass={
    "Pointer",
    JSCLASS_HAS_PRIVATE,
    JS_PropertyStub,
    JS_PropertyStub,
    JSX_Pointer_getProperty,
    JSX_Pointer_setProperty,
    JS_EnumerateStub,
    JS_ResolveStub,
    JS_ConvertStub,
    JSX_Pointer_finalize
};

static char *JSX_typenames[]={
  "signed integer",
  "unsigned integer",
  "floating point",
  "function",
  "struct",
  "union",
  "void",
  "pointer",
  "array",
  "char pointer",
  "short pointer",
  "char array",
  "short array",
  "undefined type"
};

static char *JSX_jstypenames[]={
  "Object",
  "int",
  "Number",
  "?",
  "String",
  "?",
  "Boolean",
  "?",
  "null",
  "undefined",
  "Pointer",
  "Type",
  "Array",
  "Function"
};

// rval should be rooted
// if p is NULL, type must also be NULL. Nothing is done, only size is returned.

int JSX_Get(JSContext *cx, char *p, char *oldptr, int do_clean, JSX_Type *type, jsval *rval) {
  jsdouble tmpdouble;
  int tmpint;
  int tmpuint;
  JSObject *obj, *funobj;
  JSFunction *fun;
  JSX_Pointer *ptr;
  int typepair;
  int size=-1;
  int i;
  int elemsize;
  int totsize;

  typepair=TYPEPAIR(JSX_JSType(cx, *rval), JSX_CType(type));
  
  // Determine the appropriate conversion

  switch(typepair) {
  case TYPEPAIR(JSFUNC,POINTERTYPE):
    if(((JSX_TypePointer *) type)->direct->type != FUNCTIONTYPE)
      goto failure;
    return sizeof(void *);

  case TYPEPAIR(JSPOINTER,INTTYPE):
  case TYPEPAIR(JSPOINTER,UINTTYPE):
    if (JSX_TypeSize(type)!=sizeof(void *))
      goto failure;

    // Update pointer object from an int

    // Fall through

  case TYPEPAIR(JSPOINTER,POINTERTYPE):

    // Update pointer object from a type * or int

    if (do_clean!=2) {
      if(*(void **)p == NULL) {
        *rval = JSVAL_NULL;
      } else {
        ptr = JS_GetPrivate(cx, JSVAL_TO_OBJECT(*rval));
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
        obj = JS_NewObject(cx, JSX_GetPointerClass(), 0, 0);
        *rval = OBJECT_TO_JSVAL(obj);
        ptr = (JSX_Pointer *) JS_malloc(cx, sizeof(JSX_Pointer));
        ptr->ptr = *(void **)p;
        ptr->type = ((JSX_TypePointer *) type)->direct;
        ptr->finalize = 0;
        JS_SetPrivate(cx, obj, ptr);
      }
    }
    return sizeof(void *);

  case TYPEPAIR(JSVAL_STRING,UNDEFTYPE):

    // Return a string from an undefined type
    // assume void *

    if (!p)
      return sizeof(void *);

    goto stringcommon;

    // fall through

  case TYPEPAIR(JSVAL_STRING,POINTERTYPE):
    if(!is_void_or_char(((JSX_TypePointer *) type)->direct)) goto failure;

    // Return a string from a void *
    // same as char * in this context

    // Return a string from a char *

  stringcommon:
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

  case TYPEPAIR(JSVAL_INT,UNDEFTYPE):
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

  case TYPEPAIR(JSVAL_DOUBLE,UNDEFTYPE):

    // Return a number from an undefined type
    // assume double

    if (!p)
      return sizeof(double);

    size=1;

    // fall through

  case TYPEPAIR(JSVOID,FLOATTYPE):
  case TYPEPAIR(JSNULL,FLOATTYPE):
  case TYPEPAIR(JSVAL_BOOLEAN,FLOATTYPE):
  case TYPEPAIR(JSVAL_INT,FLOATTYPE):
  case TYPEPAIR(JSVAL_DOUBLE,FLOATTYPE):

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

  case TYPEPAIR(JSVOID,FUNCTIONTYPE):
    // Create a new JS function which calls a C function
    fun = JS_NewFunction(cx, JSX_NativeFunction, ((JSX_TypeFunction *) type)->nParam, 0, 0, "JSEXT_NATIVE");
    funobj=JS_GetFunctionObject(fun);
    *rval=OBJECT_TO_JSVAL(funobj);

    return -1;
    // who knows the size of a function, really?
    // anyway, functions will only ever be returned
    // from a get() operation, so the size doesn't matter
    // as long as it is non-null.

  case TYPEPAIR(JSARRAY,POINTERTYPE):

    // Update array elements from a variable array

    obj=JSVAL_TO_OBJECT(*rval);
    JS_GetArrayLength(cx, obj, &size);
    elemsize = JSX_TypeSize(((JSX_TypeArray *) type)->member);

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

      if(JSX_TypeContainsPointer(((JSX_TypeArray *) type)->member)) {
        oldptr = *(char **)p + elemsize * size;
      } else {
        oldptr = 0;
      }
    }

    totsize=0;

    {
      jsval tmp=JSVAL_VOID;
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
    }

    if (do_clean) {
      JS_free(cx, *(char **)p);
    }

    return sizeof(void *);

  vararrayfailure1:
    if (do_clean) {
      int elemsize = JSX_TypeSize(((JSX_TypePointer *) type)->direct);
      for (;++i<size;) {
        jsval tmp;
        JS_GetElement(cx, obj, i, &tmp);
        JSX_Get(cx, *(char **)p + i * elemsize, oldptr ? *(char **)oldptr + i * elemsize : 0, 2, ((JSX_TypePointer *) type)->direct, &tmp);
      }

      JS_free(cx, *(char **)p);
    }

    goto failure;
    // error already thrown

  case TYPEPAIR(JSVOID,ARRAYTYPE):
  case TYPEPAIR(JSNULL,ARRAYTYPE):
    if(type_is_char(((JSX_TypeArray *) type)->member)) goto fromchararray;

    // Create new array and populate with values

    *rval=OBJECT_TO_JSVAL(JS_NewArrayObject(cx, 0, 0));
    
    // fall through

  case TYPEPAIR(JSARRAY,ARRAYTYPE):

    // Update array elements from a fixed size array
    
    totsize=0;
    size = ((JSX_TypeArray *) type)->length;
    obj=JSVAL_TO_OBJECT(*rval);

    {
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
    }

    return totsize;

  fixedarrayfailure:
    if (do_clean) {
      int elemsize = JSX_TypeSize(((JSX_TypeArray *) type)->member);
      for (;++i<size;) {
        jsval tmp;
        JS_GetElement(cx, obj, i, &tmp);
        JSX_Get(cx, p + i * elemsize, oldptr ? oldptr + i * elemsize : 0, 2, ((JSX_TypeArray *) type)->member, &tmp);
      }
    }
    goto failure;
    // Error already thrown

  case TYPEPAIR(JSVOID,STRUCTTYPE):
  case TYPEPAIR(JSVOID,UNIONTYPE):
  case TYPEPAIR(JSNULL,STRUCTTYPE):
  case TYPEPAIR(JSNULL,UNIONTYPE):

    // Create new object and populate with values

    *rval=OBJECT_TO_JSVAL(JS_NewObject(cx, 0, 0, 0));

    // fall trough

  case TYPEPAIR(JSVAL_OBJECT,STRUCTTYPE):
  case TYPEPAIR(JSVAL_OBJECT,UNIONTYPE):
    // Update object elements from a struct or union
    size = ((JSX_TypeStructUnion *) type)->nMember;
    obj=JSVAL_TO_OBJECT(*rval);

    {
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
    }

    return JSX_TypeSize(type);
    
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

  case TYPEPAIR(JSARRAY,STRUCTTYPE):
  case TYPEPAIR(JSARRAY,UNIONTYPE):
    // Update array elements from struct or union
    size = ((JSX_TypeStructUnion *) type)->nMember;
    obj=JSVAL_TO_OBJECT(*rval);

    {
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
    }
      
    return JSX_TypeSize(type);

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

  case TYPEPAIR(JSPOINTER,ARRAYTYPE):

    // Copy contents of array into memory pointed to
    // Leave pointer unchanged

    size=JSX_TypeSize(type);

    if (!p)
      return size;

    ptr=JS_GetPrivate(cx, JSVAL_TO_OBJECT(*rval));
    memcpy(ptr->ptr, p, size);

    return size;

  default:

    // Could not find appropriate conversion

    if (do_clean==2) // Okay if we are not going to use the value
      return JSX_TypeSize(type);

    goto failure;

  }

 failure:
  JSX_ReportException(cx, "Get: Could not convert value from C %s to JS %s",JSX_typenames[typepair%TYPECOUNT2], JSX_jstypenames[typepair/TYPECOUNT2]);
  return 0;
}


static int JSX_Get_multi(JSContext *cx, int do_clean, uintN nargs, JSX_FuncParam *type, jsval *rval, int convconst, void **argptr) {
  int ret=0;
  int siz;
  uintN i;
  JSX_FuncParam tmptype = { 0, 0 };
  JSX_FuncParam *thistype;

  for (i=0; i<nargs; i++) {
    // xxx this either is, or should become, obsolete.  we ought to fix up |sometype foo(void)| sooner than this
    if(type && type->paramtype->type == VOIDTYPE) type = 0; // End of param list

    if(JSVAL_IS_OBJECT(*rval) && *rval != JSVAL_NULL && JS_InstanceOf(cx, JSVAL_TO_OBJECT(*rval), JSX_GetTypeClass(), NULL)) {
      // Paramlist-specified type
      thistype=&tmptype;
      tmptype.paramtype = JS_GetPrivate(cx,JSVAL_TO_OBJECT(*rval));
      rval++;
      i++;
      if (i==nargs) break;
    } else {
      thistype = type ? type : 0;
    }

    if (!convconst && thistype && (thistype->isConst || !JSVAL_IS_OBJECT(*rval) || *rval==JSVAL_NULL)) { // Const or immutable
      if(do_clean) {
        siz = JSX_Get(cx, *argptr, 0, 2, thistype ? thistype->paramtype : 0, rval);
      } else {
        if(!thistype) {
          siz = JSX_Get(cx, 0, 0, 0, 0, rval); // Get size of C type guessed from js type
        } else {
          siz = JSX_TypeSize(thistype->paramtype);
        }
        rval++;
      }
    } else {
      if(thistype && thistype->paramtype->type == ARRAYTYPE) {
        if(!do_clean) goto failure;
        // In function calls, arrays are passed by pointer
        siz = JSX_Get(cx, *(void **)*argptr, 0, do_clean, thistype->paramtype, rval);
        if(siz) siz = sizeof(void *);
      } else {
        siz = JSX_Get(cx, *argptr, 0, do_clean, thistype ? thistype->paramtype : 0, rval);
      }
    }

    // In function calls, arrays are passed by pointer

    if(do_clean && thistype && thistype->paramtype->type == ARRAYTYPE) {
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

  for (;++i<nargs;) {
    JSX_Get(cx, NULL, 0, 2, type ? type->paramtype : 0, rval);
    if(type && type->paramtype->type == ARRAYTYPE) JS_free(cx, *(void **)*argptr);
    rval++;
    if (type)
      type++;
    argptr++;
  }

  return 0;
}

// Setting to undefined does nothing, only returns sizeof.
// Setting to null zeroes memory.

static int JSX_Set(JSContext *cx, char *p, int will_clean, JSX_Type *type, jsval v) {
  int size=-1;
  int tmpint;
  int totsize;
  int i;
  JSObject *obj;
  JSX_Pointer *ptr;
  jsdouble tmpdbl;
  jsval tmpval;
  int elemsize;
  JSFunction *fun;

  int typepair=TYPEPAIR(JSX_JSType(cx, v), JSX_CType(type));

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
      JSObject *newptr=JS_NewObject(cx, &JSX_PointerClass, 0, 0);
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

    if (JSX_TypeSize(type)!=sizeof(void *))
      goto failure;

    // Copy a pointer object to an int

    // fall through

  case TYPEPAIR(JSPOINTER,POINTERTYPE):

    // Copy a pointer object to a type *

  pointercommon:

    ptr=JS_GetPrivate(cx, JSVAL_TO_OBJECT(v));
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

    // fall through

  case TYPEPAIR(JSVAL_STRING,UNDEFTYPE):

    // Copy a string to a char *

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

  case TYPEPAIR(JSVAL_INT,UNDEFTYPE):
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

  case TYPEPAIR(JSVAL_DOUBLE,UNDEFTYPE):

    // Copy a number to an undefined type
    // assume double

    size=1;

    // fall through

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
      JS_GetArrayLength(cx, obj, &size);
      elemsize = JSX_TypeSize(((JSX_TypeArray *) type)->member);

      if (will_clean) {
        // The variable array needs to be allocated
        containsPointers = JSX_TypeContainsPointer(((JSX_TypeArray *) type)->member);
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
        for (;i--;) {
          jsval tmp;
          JS_GetElement(cx, obj, i, &tmp);
          JSX_Get(cx, NULL, 0, 2, ((JSX_TypePointer *) type)->direct, &tmp);
        }
        JS_free(cx, *(void **)p);
      }
    
      return 0;
    }
    // error already thrown
    

  case TYPEPAIR(JSARRAY,ARRAYTYPE):

    // Copy array elements to a fixed size array
    
    totsize=0;
    size = ((JSX_TypeArray *) type)->length;
    obj=JSVAL_TO_OBJECT(v);

    for (i=0; i<size; i++) {
      jsval tmp;
      JS_GetElement(cx, obj, i, &tmp);
      int thissize = JSX_Set(cx, p + totsize, will_clean, ((JSX_TypeArray *) type)->member, tmp);
      if(!thissize) goto fixedarrayfailure;
      totsize+=thissize;
    }

    return totsize;

  fixedarrayfailure:

    if (will_clean) {
      for (;i--;) {
        jsval tmp;
        JS_GetElement(cx, obj, i, &tmp);
        JSX_Get(cx, NULL, 0, 2, ((JSX_TypeArray *) type)->member, &tmp);
      }
    }
    
    return 0;
    // error already thrown

  case TYPEPAIR(JSVAL_OBJECT,STRUCTTYPE):
  case TYPEPAIR(JSVAL_OBJECT,UNIONTYPE):

    // Copy object elements to a struct or union

    obj=JSVAL_TO_OBJECT(v);
    size = ((JSX_TypeStructUnion *) type)->nMember;

    for (i=0; i<size; i++) {
      jsval tmp;
      int thissize;

      JS_GetProperty(cx, obj, ((JSX_TypeStructUnion *) type)->member[i].name, &tmp);

      if(((JSX_TypeStructUnion *) type)->member[i].membertype->type == BITFIELDTYPE) {
        int length = ((JSX_TypeBitfield *) ((JSX_TypeStructUnion *) type)->member[i].membertype)->length;
        int offset = ((JSX_TypeStructUnion *) type)->member[i].offset % 8;
        int mask = ~(-1 << length);
        int imask = ~(imask << offset);
        int tmpint;
        int tmpint2;
        thissize = JSX_Set(cx, (char *) &tmpint, will_clean, ((JSX_TypeStructUnion *) type)->member[i].membertype, tmp);
        memcpy((char *) &tmpint2, p + ((JSX_TypeStructUnion *) type)->member[i].offset / 8, thissize);
        tmpint = (tmpint2 & imask) | ((tmpint & mask) << offset);
        memcpy(p + ((JSX_TypeStructUnion *) type)->member[i].offset / 8, (char *) &tmpint, thissize);
      } else {
        thissize = JSX_Set(cx, p + ((JSX_TypeStructUnion *) type)->member[i].offset / 8, will_clean, ((JSX_TypeStructUnion *) type)->member[i].membertype, tmp);
      }
      if(!thissize) goto structfailure;
    }

    return JSX_TypeSize(type);
    
  structfailure:

    if (will_clean) {
      for (;i--;) {
        jsval tmp;
        JS_GetProperty(cx, obj, ((JSX_TypeStructUnion *) type)->member[i].name, &tmp);
        JSX_Get(cx, NULL, 0, 2, ((JSX_TypeStructUnion *) type)->member[i].membertype, &tmp);
      }
    }

    return 0;
    // error already thrown

  case TYPEPAIR(JSARRAY,STRUCTTYPE):
  case TYPEPAIR(JSARRAY,UNIONTYPE):
    // Copy array elements to struct or union
    obj=JSVAL_TO_OBJECT(v);
    size = ((JSX_TypeStructUnion *) type)->nMember;

    for (i=0; i<size; i++) {
      jsval tmp;
      JS_GetElement(cx, obj, i, &tmp);
      int thissize = JSX_Set(cx, p + ((JSX_TypeStructUnion *) type)->member[i].offset / 8, will_clean, ((JSX_TypeStructUnion *) type)->member[i].membertype, tmp);
      if(!thissize) goto structfailure2;
    }

    return JSX_TypeSize(type);

  structfailure2:

    if (will_clean) {
      for (;i--;) {
        jsval tmp;
        JS_GetElement(cx, obj, i, &tmp);
        JSX_Get(cx, NULL, 0, 2, ((JSX_TypeStructUnion *) type)->member[i].membertype, &tmp);
      }
    }

    return 0;
    // error already thrown

  case TYPEPAIR(JSNULL,STRUCTTYPE):
  case TYPEPAIR(JSNULL,UNIONTYPE):
  case TYPEPAIR(JSNULL,ARRAYTYPE):
  case TYPEPAIR(JSNULL,INTTYPE):
  case TYPEPAIR(JSNULL,UINTTYPE):
  case TYPEPAIR(JSNULL,FLOATTYPE):

    // Initialize with zero

    size = JSX_TypeSize(type);
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

    return JSX_TypeSize(type);

  case TYPEPAIR(JSPOINTER,ARRAYTYPE):

    // Copy contents pointed to into array
    size=JSX_TypeSize(type);
    ptr=JS_GetPrivate(cx, JSVAL_TO_OBJECT(v));
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


static int JSX_Set_multi(JSContext *cx, char *ptr, int will_clean, uintN nargs, JSX_FuncParam *type, jsval *vp, void **argptr) {
  int ret=0;
  int siz, cursiz;
  uintN i;
  JSX_FuncParam tmptype = { 0, 0 };
  JSX_FuncParam *thistype;

  for (i=0; i<nargs; i++) {
    if(type && type->paramtype->type == VOIDTYPE) type = 0; // End of param list

    if(JSVAL_IS_OBJECT(*vp) && *vp != JSVAL_NULL && JS_InstanceOf(cx, JSVAL_TO_OBJECT(*vp), JSX_GetTypeClass(), NULL)) {
      // Paramlist-specified type
      thistype=&tmptype;
      tmptype.paramtype = JS_GetPrivate(cx, JSVAL_TO_OBJECT(*vp));
      vp++;
      i++;
      if (i==nargs) break;
    } else {
      thistype = type ? type : 0;
    }

    if(thistype && thistype->paramtype->type == ARRAYTYPE) {
      if(!will_clean) goto failure;
      // In function calls, arrays are passed by pointer
      *(void **)ptr = JS_malloc(cx, JSX_TypeSize(thistype->paramtype));
      cursiz = JSX_Set(cx, *(void **)ptr, will_clean, thistype->paramtype, *vp);
      if(cursiz) {
        cursiz = sizeof(void *);
      } else {
        JS_free(cx, *(void **)ptr);
        goto failure;
      }
    } else {
      cursiz = JSX_Set(cx, ptr ? ptr : *argptr, will_clean, thistype ? thistype->paramtype : 0, *vp);
    }
    if (!cursiz)
      goto failure;

    siz=cursiz;
    if (ptr) {
      *(argptr++)=ptr;
      ptr+=siz;
    } else {
      argptr++;
    }
    ret+=siz;
    if (type)
      type++;
    vp++;
  }

  return ret;

 failure:
  if (!will_clean)
     return 0;

  for (;i--;) {
    if (type)
      --type;
    if (ptr) {
      ptr-=siz;
    } else {
      --argptr;
    }
    siz = JSX_Get(cx, ptr ? ptr : *argptr, 0, 2, type ? type->paramtype : 0, --vp);
  }

  return 0;
}

JSClass * JSX_GetPointerClass(void) {
  return &JSX_PointerClass;
}


static JSBool JSX_InitPointerAlloc(JSContext *cx, JSObject *retobj, JSObject *type) {
  JSX_Pointer *retpriv;
  int size;
  
  if (!JS_InstanceOf(cx, type, JSX_GetTypeClass(), NULL)) {
    JSX_ReportException(cx, "Wrong type argument");
    return JS_FALSE;
  }

  size = JSX_TypeSize((JSX_Type *) JS_GetPrivate(cx, type));
  retpriv = JS_malloc(cx, sizeof(JSX_Pointer) + size);

  if (!retpriv)
    return JS_FALSE;

  JS_SetPrivate(cx, retobj, retpriv);

  retpriv->ptr=retpriv+1;
  retpriv->type = (JSX_Type *) JS_GetPrivate(cx, type);
  retpriv->finalize=0;

  return JS_TRUE;
}


static JSBool JSX_InitPointerCallback(JSContext *cx, JSObject *retobj, JSFunction *fun, JSX_Type *type) {
  JSX_Callback *retpriv;
  
  if(type->type != FUNCTIONTYPE) {
    JSX_ReportException(cx, "Type is not a C function");
    return JS_FALSE;
  }

  if (!JS_DefineProperty(cx, retobj, "function", OBJECT_TO_JSVAL(JS_GetFunctionObject(fun)), 0, 0, JSPROP_READONLY | JSPROP_PERMANENT))
    return JS_FALSE;

  retpriv = JS_malloc(cx, sizeof(JSX_Callback));
  if (!retpriv)
    return JS_FALSE;

  void *code;
  retpriv->writeable=ffi_closure_alloc(sizeof(ffi_closure), &code);
  retpriv->ptr=code;
  retpriv->cx=cx;
  retpriv->fun=fun;
  retpriv->finalize=ffi_closure_free; //This would free the code address, not always identical to writeable address. So it is checked in finalize.
  retpriv->type = type;

  if(ffi_prep_closure_loc(retpriv->writeable, JSX_GetCIF(cx, (JSX_TypeFunction *) retpriv->type), JSX_Pointer_Callback, retpriv, retpriv->ptr) != FFI_OK)
    return JS_FALSE;

  JS_SetPrivate(cx, retobj, retpriv);

  return JS_TRUE;
}


JSBool JSX_InitPointer(JSContext *cx, JSObject *retobj, JSObject *typeobj) {
  JSX_Pointer *ret;

  // xxx a hack to save typeobj from garbage collection, and thus stop the free()ing of the JSX_Type struct we share 
  if(!JS_DefineProperty(cx, retobj, "xxx", OBJECT_TO_JSVAL(typeobj), 0, 0, JSPROP_READONLY | JSPROP_PERMANENT))
    return JS_FALSE;

  ret = JS_malloc(cx, sizeof(JSX_Pointer));
  if (!ret)
    return JS_FALSE;
  ret->ptr=0;
  ret->type = (JSX_Type *) JS_GetPrivate(cx, typeobj);
  ret->finalize=0;
  JS_SetPrivate(cx, retobj, ret);

  return JS_TRUE;
}


static JSBool Pointer_malloc(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
  JSObject *newobj;
  int length;
  JSX_Pointer *ret;

  if (argc<1 || !JSVAL_IS_INT(argv[0]) || JSVAL_TO_INT(argv[0])<=0) {
    JSX_ReportException(cx, "Wrong argument type to malloc");
    return JS_FALSE;
  }

  newobj=JS_NewObject(cx, &JSX_PointerClass, 0, 0);
  *rval=OBJECT_TO_JSVAL(newobj);
  length=INT_TO_JSVAL(argv[0]);
  ret = JS_malloc(cx, sizeof(JSX_Pointer) + length);
  if (!ret)
    return JS_FALSE;
  ret->ptr=ret+1;
  ret->type = GetVoidType();
  ret->finalize=0;
  JS_SetPrivate(cx, newobj, ret);

  return JS_TRUE;
}


static JSBool Pointer_proto_cast(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
  JSObject *newobj;
  JSX_Pointer *ptr, *newptr;

  if(!JSVAL_IS_OBJECT(argv[0]) || JSVAL_IS_NULL(argv[0]) || !JS_InstanceOf(cx, JSVAL_TO_OBJECT(argv[0]), JSX_GetTypeClass(), NULL)) {
    JSX_ReportException(cx, "Pointer.prototype.cast(): argument must be a Type instance");
    return JS_FALSE;
  }

  ptr=JS_GetPrivate(cx, obj);

  newobj=JS_NewObject(cx, &JSX_PointerClass, 0, 0);
  *rval=OBJECT_TO_JSVAL(newobj);
  if (!JSX_InitPointer(cx, newobj, JSVAL_TO_OBJECT(argv[0]))) {
    return JS_FALSE;
  }

  newptr=JS_GetPrivate(cx, newobj);
  newptr->ptr=ptr->ptr;

  return JS_TRUE;
}


static JSBool JSX_Pointer_new(JSContext *cx, JSObject *origobj, uintN argc, jsval *argv, jsval *rval) {
  JSObject *obj;
  if(JS_IsConstructing(cx)) {
    obj = origobj;
  } else {
    obj = JS_NewObject(cx, &JSX_PointerClass, 0, 0);
    *rval = OBJECT_TO_JSVAL(obj);
  }

  if(argc < 1 || !JSVAL_IS_OBJECT(argv[0]) || JSVAL_IS_NULL(argv[0]) || !JS_InstanceOf(cx, JSVAL_TO_OBJECT(argv[0]), JSX_GetTypeClass(), NULL)) {
    JSX_ReportException(cx, "Pointer(): first argument must be a Type");
    return JS_FALSE;
  }

  JSObject *typeObject = JSVAL_TO_OBJECT(argv[0]);

  // Are we creating a C wrapper for a JS function so it can be used as a callback?
  if(argc >= 2 && JSVAL_IS_OBJECT(argv[1]) && JS_ObjectIsFunction(cx, JSVAL_TO_OBJECT(argv[1]))) {
    JSX_Pointer *ptr = JS_GetPrivate(cx, typeObject);
    JSX_Type *type = ptr->type;
    // Accept both function type and pointer-to-function type
    if(type->type == POINTERTYPE) type = ((JSX_TypePointer *) type)->direct;
    if(JSX_InitPointerCallback(cx, obj, JS_ValueToFunction(cx, argv[1]), type)) return JS_FALSE;
    return JS_TRUE;
  }

  // Allocate memory and create Pointer instance
  if(!JSX_InitPointerAlloc(cx, obj, JSVAL_TO_OBJECT(argv[0]))) return JS_FALSE;
  // Set initial value, if provided
  if(argc >= 2 && argv[1] != JSVAL_VOID) {
    JSX_Pointer *ptr = JS_GetPrivate(cx,obj);
    if(!JSX_Set(cx, ptr->ptr, 0, ptr->type, argv[1])) return JS_FALSE;
  }

  return JS_TRUE;
}


static void JSX_Pointer_finalize(JSContext *cx, JSObject *obj) {
  JSX_Pointer *ptr = (JSX_Pointer *) JS_GetPrivate(cx, obj);

  if (ptr) {
    if (ptr->finalize) {
      if (ptr->finalize==ffi_closure_free) {
        ffi_closure_free(((JSX_Callback *) ptr)->writeable);
      } else {
        (*ptr->finalize)(ptr->ptr);
      }
    }
    JS_free(cx, ptr);
  }
}


static JSBool JSX_Pointer_call(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
  JSX_Pointer *ptr = JS_GetPrivate(cx, obj);
  JSX_Type *type = ptr->type;

  if(type->type != FUNCTIONTYPE) {
    JSX_ReportException(cx, "call: Wrong pointer type");
    return JS_FALSE;
  }

  ffi_type **arg_types = 0;
  arg_types=JS_malloc(cx, sizeof(ffi_cif)+sizeof(ffi_type *)*(argc+1));
  ffi_cif *cif;
  cif=(ffi_cif *)(arg_types+(argc+1));

  size_t arg_size = JSX_TypeSize_multi(cx, argc, ((JSX_TypeFunction *) type)->param, argv, arg_types);

  int real_argc;
  for (real_argc=0; arg_types[real_argc]; real_argc++)
    ;


  if(real_argc > ((JSX_TypeFunction *) type)->nParam) {
    int callconv=FFI_DEFAULT_ABI;
    memcpy(arg_types, JSX_GetCIF(cx, ((JSX_TypeFunction *) type))->arg_types, sizeof(ffi_type *) * ((JSX_TypeFunction *) type)->nParam);
    ffi_prep_cif(cif, callconv, real_argc, JSX_GetFFIType(cx, ((JSX_TypeFunction *) type)->returnType), arg_types);
  } else {
    cif = JSX_GetCIF(cx, (JSX_TypeFunction *) type);
  }

  int retsize = JSX_TypeSize(((JSX_TypeFunction *) type)->returnType);

  void **argptr = 0;
  argptr=(void **)JS_malloc(cx, arg_size + argc*sizeof(void *) + retsize + 8);

  char *retbuf = 0;
  retbuf=(char *)(argptr + argc);
  char *argbuf;
  argbuf=retbuf + retsize + 8; // ffi overwrites a few bytes on some archs.

  if (arg_size) {
    if(!JSX_Set_multi(cx, (void *) argbuf, 1, argc, ((JSX_TypeFunction *) type)->param, argv, argptr))
      goto failure;
  }

  ffi_call(cif, ptr->ptr, (void *)retbuf, argptr);

  JS_free(cx,arg_types);
  arg_types=0;

  *rval=JSVAL_VOID;

  if(((JSX_TypeFunction *) type)->returnType->type != VOIDTYPE) {
    JSX_Get(cx, retbuf, 0, 0, ((JSX_TypeFunction *) type)->returnType, rval);
  }

  if (arg_size) {
    if(!JSX_Get_multi(cx, 1, argc, ((JSX_TypeFunction *) type)->param, argv, 0, argptr))
      goto failure;
  }
  JS_free(cx, argptr);

  return JS_TRUE;

 failure:
  if (argptr) 
    JS_free(cx, argptr);
  if (arg_types) 
    JS_free(cx, arg_types);

  return JS_FALSE;
}


static JSBool JSX_Pointer_getdollar(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
  *vp=JSVAL_VOID;
  JSX_Pointer *ptr = (JSX_Pointer *) JS_GetPrivate(cx, obj);
  int ret = JSX_Get(cx, ptr->ptr, 0, 0, ptr->type, vp);
  if(!ret) return JS_FALSE;
  if(ret == -1) {
    // Created new function
    JS_DefineProperty(cx, JSVAL_TO_OBJECT(*vp), "__ptr__", OBJECT_TO_JSVAL(obj), 0, 0, JSPROP_READONLY | JSPROP_PERMANENT);
  }
  return JS_TRUE;
}


static JSBool JSX_Pointer_setdollar(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
  JSX_Pointer *ptr = (JSX_Pointer *) JS_GetPrivate(cx, obj);
  if(!JSX_Set(cx, ptr->ptr, 0, ptr->type, *vp)) return JS_FALSE;
  return JS_TRUE;
}


static JSBool JSX_Pointer_getfinalize(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
  *vp=JSVAL_VOID;
  return JS_TRUE;
}


static JSBool JSX_Pointer_setfinalize(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
  JSX_Pointer *ptr;
  ptr = (JSX_Pointer *) JS_GetPrivate(cx, obj);
  if (*vp==JSVAL_NULL || *vp==JSVAL_VOID) {
    ptr->finalize=0;
    return JS_TRUE;
  }

  jsval ptrobj;
  if (!JSVAL_IS_OBJECT(*vp) ||
      !JS_ObjectIsFunction(cx, JSVAL_TO_OBJECT(*vp)) ||
      !JS_LookupProperty(cx, JSVAL_TO_OBJECT(*vp), "__ptr__", &ptrobj) ||
      !JS_InstanceOf(cx, JSVAL_TO_OBJECT(ptrobj), JSX_GetPointerClass(), NULL)) {
    JSX_ReportException(cx, "Wrong value type for finalize property");
    return JS_FALSE;
  }

  JSX_Pointer *finptr;
  finptr=JS_GetPrivate(cx, JSVAL_TO_OBJECT(ptrobj));
  JSX_Type *type;
  type=finptr->type;
  JSX_TypeFunction *functype = (JSX_TypeFunction *) type;

  if(type->type != FUNCTIONTYPE || functype->nParam != 1 || functype->param[0].paramtype->type != POINTERTYPE || functype->param[0].isConst) {
    JSX_ReportException(cx, "Wrong function type for finalize property");
    return JS_FALSE;
  }

  ptr->finalize=finptr->ptr;

  return JS_TRUE;
}


static JSBool Pointer_proto_string(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
  JSX_Pointer *ptr = (JSX_Pointer *) JS_GetPrivate(cx, obj);
  JSString* str;
  if(JSVAL_IS_INT(argv[0])) {
    str = JS_NewStringCopyN(cx, (char *) ptr->ptr, JSVAL_TO_INT(argv[0]));
  } else {
    str = JS_NewStringCopyZ(cx, (char *) ptr->ptr);
  }
  *rval = STRING_TO_JSVAL(str);
  return JS_TRUE;
}


static JSBool Pointer_proto_valueOf(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
  JSX_Pointer *ptr = (JSX_Pointer *) JS_GetPrivate(cx, obj);
  jsdouble val = (jsdouble) (long) ptr->ptr;
  JS_NewNumberValue(cx, val, rval);
  return JS_TRUE;
}


// Read a field from a struct/union that this pointer points to (without converting the entire struct into a javascript ibject)
static JSBool Pointer_proto_field(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
  if(argc != 1 || !JSVAL_IS_STRING(argv[0]))
    return JSX_ReportException(cx, "Pointer.prototype.field(): must be passed a single argument, of type string");

  JSX_Pointer *ptr;
  ptr = JS_GetPrivate(cx, obj);

  if(!ptr->type->type == POINTERTYPE) return JS_FALSE; // should be impossible

  if(ptr->type->type != STRUCTTYPE && ptr->type->type != UNIONTYPE)
    return JSX_ReportException(cx, "Pointer.prototype.field(): must only be called on pointers to struct or union types");

  JSX_TypeStructUnion *sutype;
  sutype = (JSX_TypeStructUnion *) ptr->type;

  char *myname = JS_GetStringBytes(JSVAL_TO_STRING(argv[0]));

  int ix;
  for(ix = 0; ix < sutype->nMember; ++ix) {
    if(strcmp(sutype->member[ix].name, myname) == 0)
      break;
  }

  if(ix == sutype->nMember)
    return JSX_ReportException(cx, "Pointer.prototype.field(): unknown struct/union member: %s", myname);
  if(sutype->member[ix].membertype->type == BITFIELDTYPE)
    return JSX_ReportException(cx, "Pointer.prototype.field(): requested member is a bitfield: %s", myname);

  JSObject *newobj;
  newobj = JS_NewObject(cx, &JSX_PointerClass, 0, 0);
  *rval = OBJECT_TO_JSVAL(newobj);

  JSX_Pointer *newptr;
  newptr = (JSX_Pointer *) malloc(sizeof(JSX_Pointer));
  newptr->type = sutype->member[ix].membertype;
  newptr->ptr = ptr->ptr + sutype->member[ix].offset / 8;
  newptr->finalize = 0;
  JS_SetPrivate(cx, newobj, newptr);

  return JS_TRUE;
}


static JSBool JSX_Pointer_getProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
  JSX_Pointer *ptr;
  int ret;

  if (!JSVAL_IS_INT(id))
    return JS_TRUE; // Only handle numerical properties

  ptr = (JSX_Pointer *) JS_GetPrivate(cx, obj);

  ret = JSX_Get(cx, (char *) ptr->ptr + JSX_TypeSize(ptr->type) * JSVAL_TO_INT(id), 0, 0, ptr->type, vp);
  if(ret == 0) return JS_FALSE;

  if (ret==-1 && id==JSVAL_ZERO) {

    // Created new function
    JS_DefineProperty(cx, JSVAL_TO_OBJECT(*vp), "__ptr__", OBJECT_TO_JSVAL(obj), 0, 0, JSPROP_READONLY | JSPROP_PERMANENT);
  }

  if (ret==-1 && id!=JSVAL_ZERO) {

    // Created new function through improper use of [] operator.
    JSX_ReportException(cx, "Function pointers can not be treated as arrays");
    return JS_FALSE;
  }

  return JS_TRUE;
}


static JSBool JSX_Pointer_setProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
  if(!JSVAL_IS_INT(id)) return JS_TRUE; // Only handle numerical properties
  JSX_Pointer *ptr = (JSX_Pointer *) JS_GetPrivate(cx, obj);
  int ret = JSX_Set(cx, (char *) ptr->ptr + JSX_TypeSize(ptr->type) * JSVAL_TO_INT(id), 0, ptr->type, *vp);
  return ret == 0 ? JS_FALSE : JS_TRUE;
}


/*
  Indirect entry point for callback from C program
 */

static void JSX_Pointer_Callback(ffi_cif *cif, void *ret, void **args, void *user_data) {
  JSX_Callback *cb = user_data;
  jsval *tmp_argv;
  int i;
  jsval rval=JSVAL_VOID;
  JSX_TypeFunction *type = (JSX_TypeFunction *) cb->type;

  tmp_argv=(jsval *)JS_malloc(cb->cx, sizeof(jsval)*type->nParam);
  if(!tmp_argv) return;
  
  for (i=0; i<type->nParam; i++) {
    tmp_argv[i]=JSVAL_VOID;
    JS_AddRoot(cb->cx, tmp_argv+i);
  }
  JS_AddRoot(cb->cx, &rval);
  
  // pretty sure this has side-effects
  JSX_Get_multi(cb->cx, 0, type->nParam, type->param, tmp_argv, 1, args);

  if (!JS_CallFunction(cb->cx, JS_GetGlobalObject(cb->cx), cb->fun, type->nParam, tmp_argv, &rval)) {
    //    printf("FAILCALL\n");
  }
  
  JSX_Set_multi(cb->cx, 0, 0, type->nParam, type->param, tmp_argv, args);

  JS_RemoveRoot(cb->cx, &rval);
  for (i=0; i<type->nParam; i++) {
    JS_RemoveRoot(cb->cx, tmp_argv+i);
  }
  JS_free(cb->cx, tmp_argv);

  if(type->returnType->type != VOIDTYPE) JSX_Set(cb->cx, ret, 0, type->returnType, rval);
}


static JSBool JSX_NativeFunction(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
  JSObject *funcobj;
  jsval ptr;
  JSBool ok;

  funcobj=JSVAL_TO_OBJECT(argv[-2]);
  JS_LookupProperty(cx, funcobj, "__ptr__", &ptr);

  ok=JSX_Pointer_call(cx, JSVAL_TO_OBJECT(ptr), argc, argv, rval);

  return ok;
}


jsval JSX_make_Pointer(JSContext *cx, JSObject *obj) {
  JSObject *protoobj;
  JSObject *classobj;

  static struct JSFunctionSpec staticfunc[]={
    {"malloc", Pointer_malloc, 1, 0, 0},
    {0,0,0,0,0}
  };

  static struct JSFunctionSpec memberfunc[]={
    {"cast", Pointer_proto_cast, 1, 0, 0},
    {"field", Pointer_proto_field, 1, 0, 0},
    {"string", Pointer_proto_string, 1, 0, 0},
    {"valueOf", Pointer_proto_valueOf, 0, 0, 0},
    {0,0,0,0,0}
  };

  static struct JSPropertySpec memberprop[]={
    {"$",0, JSPROP_PERMANENT, JSX_Pointer_getdollar,JSX_Pointer_setdollar},
    {"finalize",0,0, JSX_Pointer_getfinalize, JSX_Pointer_setfinalize},
    {0,0,0,0,0}
  };


  protoobj=JS_NewObject(cx, &JSX_PointerClass, 0, 0);
  classobj=JS_InitClass(cx, obj, protoobj, &JSX_PointerClass, JSX_Pointer_new, 0, memberprop, memberfunc, 0, staticfunc);
  if(!classobj) return JSVAL_VOID;

  return OBJECT_TO_JSVAL(JS_GetConstructor(cx, classobj));
}

