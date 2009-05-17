#include "Pointer.h"
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

# include <dlfcn.h>
# include <alloca.h>

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

static JSBool JSX_Pointer_new(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
static void JSX_Pointer_finalize(JSContext *cx, JSObject *obj);
static JSBool JSX_Pointer_call(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
static JSBool JSX_Pointer_resolve(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
static int JSX_Get_multi(JSContext *cx, int do_clean, uintN nargs, JSX_ParamType *type, jsval *rval, int convconst, void **argptr);
static int JSX_Set_multi(JSContext *cx, char *ptr, int will_clean, uintN nargs, JSX_ParamType *type, jsval *vp, int convconst, void **argptr);
static JSBool JSX_Pointer_pr_UCString(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
static JSBool JSX_Pointer_pr_string(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
static JSBool JSX_Pointer_UCString(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
static JSBool JSX_Pointer_string(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
static JSBool JSX_Pointer_toString(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
static JSBool JSX_Pointer_valueOf(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
static JSBool JSX_Pointer_getProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp);
static JSBool JSX_Pointer_setProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp);
static void JSX_Pointer_Callback(ffi_cif *cif, void *ret, void **args, void *user_data);
static JSBool JSX_NativeFunction(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
static JSBool JSX_InitPointerAlloc(JSContext *cx, JSObject *obj, JSObject *type);
static JSBool JSX_InitPointerCallback(JSContext *cx, JSObject *obj, JSFunction *fun, JSObject *type);
static JSBool JSX_InitPointerString(JSContext *cx, JSObject *obj, JSString *str);
static JSBool JSX_InitPointerUCString(JSContext *cx, JSObject *obj, JSString *str);
static JSBool JSX_PointerResolve(JSContext *cx, JSObject *obj);


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

static JSBool JSX_ReportException(JSContext *cx, char *format, ...) {
  int len;
  char *msg;
  JSString *Str;
  va_list va;
  jsval str;

  va_start(va, format);
  msg=JS_malloc(cx, 81);
  va_start(va, format);
  len=vsnprintf(msg,80,format,va);
  msg[80]=0;

  va_end(va);
  Str=JS_NewString(cx, msg, len);
  str=STRING_TO_JSVAL(Str);
  JS_SetPendingException(cx, str);

  return JS_FALSE;
}

// rval should be rooted
// if p is NULL, type must also be NULL. Nothing is done, only size is returned.

int JSX_Get(JSContext *cx, char *p, char *oldptr, int do_clean, struct JSX_Type *type, jsval *rval) {
  jsdouble tmpdouble;
  int tmpint;
  int tmpuint;
  JSObject *obj, *funobj;
  JSFunction *fun;
  struct JSX_Pointer *ptr;
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

  case TYPEPAIR(JSPOINTER,PCHARTYPE):
  case TYPEPAIR(JSPOINTER,PSHORTTYPE):
  case TYPEPAIR(JSPOINTER,POINTERTYPE):

    // Update pointer object from a type * or int

    if (do_clean!=2) {
      if (*(void **)p==NULL) {
	*rval=JSVAL_NULL;
      } else {
	ptr=JS_GetPrivate(cx, JSVAL_TO_OBJECT(*rval));
	ptr->ptr=*(void **)p;
      }
    }
    return sizeof(void *);

  case TYPEPAIR(JSVOID,PCHARTYPE):
  case TYPEPAIR(JSVOID,PSHORTTYPE):
  case TYPEPAIR(JSVOID,POINTERTYPE):
  case TYPEPAIR(JSNULL,PCHARTYPE):
  case TYPEPAIR(JSNULL,PSHORTTYPE):
  case TYPEPAIR(JSNULL,POINTERTYPE):

    // Return a pointer object from a type *
    if (do_clean!=2) {
      if (*(void **)p==NULL) {
	*rval=JSVAL_NULL;
      } else {
	obj=JS_NewObject(cx, JSX_GetPointerClass(), 0, 0);
	*rval=OBJECT_TO_JSVAL(obj);
	ptr=(struct JSX_Pointer *)JS_malloc(cx, sizeof(struct JSX_Pointer));
	ptr->ptr=*(void **)p;
	ptr->type = ((JSX_TypePointer *) type)->direct;
	ptr->finalize=0;
	JS_DefineProperty(cx, obj, "type", OBJECT_TO_JSVAL(ptr->type->typeObject), 0, 0, JSPROP_ENUMERATE | JSPROP_READONLY | JSPROP_PERMANENT);
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

    if(((JSX_TypePointer *) type)->direct->type != VOIDTYPE) {
      goto failure;
    }

    // Return a string from a void *
    // same as char * in this context

    // fall through

  case TYPEPAIR(JSVAL_STRING,PCHARTYPE):

    // Return a string from a char *

  stringcommon:
    if (do_clean!=2)
      *rval=STRING_TO_JSVAL(JS_NewStringCopyZ(cx, *(char **)p));
    return sizeof(char *);

  case TYPEPAIR(JSVAL_STRING,PSHORTTYPE):

    // Return a string from a short *

    if (do_clean!=2)
      *rval=STRING_TO_JSVAL(JS_NewUCStringCopyZ(cx, *(jschar **)p));
    return sizeof(jschar *);

  case TYPEPAIR(JSVAL_STRING,INTTYPE):
  case TYPEPAIR(JSVAL_STRING,UINTTYPE):

    switch(((JSX_TypeInt *) type)->size) {
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

  case TYPEPAIR(JSVOID,ACHARTYPE):
  case TYPEPAIR(JSNULL,ACHARTYPE):
  case TYPEPAIR(JSVAL_STRING,ACHARTYPE):
    // Return a string from a char array
    *rval = STRING_TO_JSVAL(JS_NewStringCopyN(cx, (char *) p, ((JSX_TypeArray *) type)->length));
    return sizeof(char) * ((JSX_TypeArray *) type)->length;

  case TYPEPAIR(JSVOID,ASHORTTYPE):
  case TYPEPAIR(JSNULL,ASHORTTYPE):
  case TYPEPAIR(JSVAL_STRING,ASHORTTYPE):
    // Return a string from a short array
    *rval = STRING_TO_JSVAL(JS_NewUCStringCopyN(cx, (jschar *) p, ((JSX_TypeArray *) type)->length));
    return sizeof(jschar) * ((JSX_TypeArray *) type)->length;

  case TYPEPAIR(JSVAL_INT,UNDEFTYPE):
  case TYPEPAIR(JSVAL_INT,POINTERTYPE):
  case TYPEPAIR(JSVAL_INT,PCHARTYPE):
  case TYPEPAIR(JSVAL_INT,PSHORTTYPE):

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
    switch(size != -1 ? size : ((JSX_TypeInt *) type)->size) {

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

    size = ((JSX_TypeInt *) ((JSX_TypeBitfield *) type)->member)->size;

    // fall through

  case TYPEPAIR(JSNULL,UINTTYPE):
  case TYPEPAIR(JSVOID,UINTTYPE):
  case TYPEPAIR(JSVAL_BOOLEAN,UINTTYPE):
  case TYPEPAIR(JSVAL_INT,UINTTYPE):
  case TYPEPAIR(JSVAL_DOUBLE,UINTTYPE):

    // Return a number from an unsigned int (of various sizes)
    switch(size != -1 ? size : ((JSX_TypeInt *) type)->size) {

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

    switch(size != -1 ? size : ((JSX_TypeFloat *) type)->size) {

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
  case TYPEPAIR(JSARRAY,PCHARTYPE):
  case TYPEPAIR(JSARRAY,PSHORTTYPE):

    // Update array elements from a variable array

    obj=JSVAL_TO_OBJECT(*rval);
    JS_GetArrayLength(cx, obj, &size);
    elemsize = JSX_TypeSize(((JSX_TypeArray *) type)->member);

    if (do_clean) {
      if (oldptr && (do_clean==2 || *(void **)oldptr!=*(void **)p)) {
	// Pointer has changed, don't update array, just return pointer and clean up

	if (do_clean!=2) {
	  *rval=JSVAL_VOID;
	  JSX_Get(cx, p, 0, 0, type, rval);
	}

	totsize=0;

	for (i=0; i<size; i++) {
	  jsval tmp;
	  JS_GetElement(cx, obj, i, &tmp);
	  int thissize = JSX_Get(cx, 0, *(char **)oldptr + totsize, 2, ((JSX_TypePointer *) type)->direct, &tmp);
	  if (!thissize) goto vararrayfailure1;
	  totsize+=thissize;
	}

	JS_free(cx, *(char **)p);
	return sizeof(void *);
      }

      if(JSX_TypeContainsPointer(((JSX_TypeArray *) type)->member)) {
	oldptr=*(char **)p+elemsize*size;
      } else {
	oldptr=0;
      }
    }

    totsize=0;

    {
      jsval tmp=JSVAL_VOID;
      JS_AddRoot(cx, &tmp);
      for (i=0; i<size; i++) {
	JS_GetElement(cx, obj, i, &tmp);
	int thissize = JSX_Get(cx, *(char **)p + totsize, oldptr ? *(char **)oldptr + totsize : 0, do_clean, ((JSX_TypePointer *) type)->direct, &tmp);
	if (!thissize) {
	  JS_RemoveRoot(cx, &tmp);
	  goto vararrayfailure1;
	}
	if (do_clean != 2)
	  JS_SetElement(cx, obj, i, &tmp);
	totsize+=thissize;
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
    return 0;
    // error already thrown

  case TYPEPAIR(JSVOID,ARRAYTYPE):
  case TYPEPAIR(JSNULL,ARRAYTYPE):

    // Create new array and populate with values

    *rval=OBJECT_TO_JSVAL(JS_NewArrayObject(cx, 0, 0));
    
    // fall through

  case TYPEPAIR(JSARRAY,ARRAYTYPE):
  case TYPEPAIR(JSARRAY,ACHARTYPE):
  case TYPEPAIR(JSARRAY,ASHORTTYPE):

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
	if (!thissize) {
	  JS_RemoveRoot(cx, &tmp);
	  goto fixedarrayfailure;
	}
	if (do_clean != 2)
	  JS_SetElement(cx, obj, i, &tmp);
	totsize+=thissize;
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
    return 0;
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
	JS_GetProperty(cx, obj, ((JSX_TypeStructUnion *) type)->member[i].name, &tmp);
	int thissize = JSX_Get(cx,
			 p + ((JSX_TypeStructUnion *) type)->member[i].offset / 8,
			 oldptr ? oldptr + ((JSX_TypeStructUnion *) type)->member[i].offset / 8 : 0,
			 do_clean,
			 ((JSX_TypeStructUnion *) type)->member[i].type,
			 &tmp);
	if (!thissize) {
	  goto structfailure;
	  JS_RemoveRoot(cx, &tmp);
	}
	if (do_clean != 2) {
	  if(((JSX_TypeStructUnion *) type)->member[i].type->type == BITFIELDTYPE) {
	    int length = ((JSX_TypeBitfield *) ((JSX_TypeStructUnion *) type)->member[i].type)->length;
	    int offset = ((JSX_TypeStructUnion *) type)->member[i].offset % 8;
	    int mask=~(-1<<length);
	    tmp = INT_TO_JSVAL((JSVAL_TO_INT(tmp)>>offset)&mask);
	  }
	  JS_SetProperty(cx, obj, ((JSX_TypeStructUnion *) type)->member[i].name, &tmp);
	}
      }

      JS_RemoveRoot(cx, &tmp);
    }

    return JSX_TypeSize(type);
    
  structfailure:
    if (do_clean) {
      for (;++i<size;) {
	jsval tmp;

	JS_GetProperty(cx, obj, ((JSX_TypeStructUnion *) type)->member[i].name, &tmp);
	JSX_Get(cx,
		p + ((JSX_TypeStructUnion *) type)->member[i].offset / 8,
		oldptr ? oldptr + ((JSX_TypeStructUnion *) type)->member[i].offset / 8 : 0,
		2,
		((JSX_TypeStructUnion *) type)->member[i].type,
		&tmp);
      }
    }
    goto failure;
    return 0;
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
	int thissize;
	
	JS_GetElement(cx, obj, i, &tmp);
	thissize=JSX_Get(cx,
			 p + ((JSX_TypeStructUnion *) type)->member[i].offset / 8,
			 oldptr ? oldptr + ((JSX_TypeStructUnion *) type)->member[i].offset / 8 : 0,
			 do_clean,
			 ((JSX_TypeStructUnion *) type)->member[i].type,
			 &tmp);
	if (!thissize) {
	  JS_RemoveRoot(cx, &tmp);
	  goto structfailure2;
	}
	if (do_clean != 2)
	  JS_SetElement(cx, obj, i, &tmp);
      }

      JS_RemoveRoot(cx, &tmp);
    }
      
    return JSX_TypeSize(type);

  structfailure2:
    if (do_clean) {
      for (;++i<size;) {
	jsval tmp;

	JS_GetElement(cx, obj, i, &tmp);
	JSX_Get(cx,
		p + ((JSX_TypeStructUnion *) type)->member[i].offset / 8,
		oldptr ? oldptr + ((JSX_TypeStructUnion *) type)->member[i].offset / 8 : 0,
		2,
		((JSX_TypeStructUnion *) type)->member[i].type,
		&tmp);
      }
    }
    goto failure;
    return 0;
    // error already thrown

  case TYPEPAIR(JSPOINTER,ACHARTYPE):
  case TYPEPAIR(JSPOINTER,ASHORTTYPE):
  case TYPEPAIR(JSPOINTER,ARRAYTYPE):

    // Copy contents of array into memory pointed to
    // Leave pointer unchanged

    size=JSX_TypeSize(type);

    if (!p)
      return size;

    ptr=JS_GetPrivate(cx, JSVAL_TO_OBJECT(*rval));
    if (ptr->ptr==NULL && !JSX_PointerResolve(cx, JSVAL_TO_OBJECT(*rval)))
      return 0; // Error message in resolve.
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


static int JSX_Get_multi(JSContext *cx, int do_clean, uintN nargs, JSX_ParamType *type, jsval *rval, int convconst, void **argptr) {
  int ret=0;
  int siz;
  uintN i;
  int isconst;
  JSX_ParamType tmptype = { 0, 0, 0 };
  JSX_ParamType *thistype;

  for (i=0; i<nargs; i++) {

    if (type && type->type->type==VOIDTYPE) // End of param list
      type=0;

    if (JSVAL_IS_OBJECT(*rval) &&
	*rval!=JSVAL_NULL &&
	JS_InstanceOf(cx, JSVAL_TO_OBJECT(*rval), JSX_GetTypeClass(), NULL)) {
      // Paramlist-specified type
      thistype=&tmptype;
      tmptype.type=JS_GetPrivate(cx,JSVAL_TO_OBJECT(*rval));
      rval++;
      i++;
      if (i==nargs) break;
    } else {
      if (type)
	thistype=type;
      else
	thistype=0;
    }

    if (!convconst &&
	thistype && 
	(isconst = thistype->isConst || 
	 !JSVAL_IS_OBJECT(*rval) ||
	 *rval==JSVAL_NULL)) { // Const or immutable
      if (do_clean)
	siz=JSX_Get(cx, *argptr, 0, 2, thistype ? thistype->type : 0, rval);
      else {
	if (!thistype)
	  siz=JSX_Get(cx, 0, 0, 0, 0, rval); // Get size of C type guessed from js type
	else
	  siz=JSX_TypeSize(thistype->type);
	rval++;
      }
    } else {

      if (thistype && thistype->type->type == ARRAYTYPE) {
	if (do_clean) {
	  // In function calls, arrays are passed by pointer
	  siz=JSX_Get(cx, *(void **)*argptr, 0, do_clean, thistype->type, rval);
	  if (siz)
	    siz=sizeof(void *);
	} else
	  goto failure;
      } else {
	siz=JSX_Get(cx, *argptr, 0, do_clean, thistype ? thistype->type : 0, rval);
      }

    }

    // In function calls, arrays are passed by pointer

    if (do_clean && thistype && thistype->type->type == ARRAYTYPE) {
      JS_free(cx, *(void **)*argptr);
      //      if (siz)
      siz=sizeof(void *);
    }

    if (type)
      type++;
    rval++;
    if (!siz)
      goto failure;
    //    siz=(siz+align-1)&(~(align-1));
    argptr++;
    //    ptr+=siz;
    ret+=siz;
  }

  return ret;

 failure:
  if (!do_clean)
     return 0;

  for (;++i<nargs;) {
    JSX_Get(cx, NULL, 0, 2, type ? type->type : 0, rval);
    if (type && type->type->type == ARRAYTYPE)
      JS_free(cx, *(void **)*argptr);
    rval++;
    if (type)
      type++;
    argptr++;
  }

  return 0;
}

// Setting to undefined does nothing, only returns sizeof.
// Setting to null zeroes memory.

static int JSX_Set(JSContext *cx, char *p, int will_clean, struct JSX_Type *type, jsval v) {
  int size=-1;
  int tmpint;
  int totsize;
  int i;
  JSObject *obj;
  struct JSX_Pointer *ptr;
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

      if(!JSX_InitPointerCallback(cx, newptr, fun, ((JSX_TypePointer *) type)->direct->typeObject)) {
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

  case TYPEPAIR(JSPOINTER,PCHARTYPE):
  case TYPEPAIR(JSPOINTER,PSHORTTYPE):
  case TYPEPAIR(JSPOINTER,POINTERTYPE):

    // Copy a pointer object to a type *

  pointercommon:

    ptr=JS_GetPrivate(cx, JSVAL_TO_OBJECT(v));
    if (ptr->ptr==NULL && !JSX_PointerResolve(cx, JSVAL_TO_OBJECT(v)))
      return 0; // Error message in resolve.
    *(void **)p=ptr->ptr;
    return sizeof(void *);

  case TYPEPAIR(JSNULL,PCHARTYPE):
  case TYPEPAIR(JSNULL,PSHORTTYPE):
  case TYPEPAIR(JSNULL,POINTERTYPE):

    // Copy a null to a type *

    *(void **)p=NULL;

    // fall through

  case TYPEPAIR(JSVOID,PCHARTYPE):
  case TYPEPAIR(JSVOID,PSHORTTYPE):
  case TYPEPAIR(JSVOID,POINTERTYPE):
  case TYPEPAIR(JSVOID,FUNCTIONTYPE):

    // Do nothing

    return sizeof(void *);

  case TYPEPAIR(JSVAL_STRING,POINTERTYPE):
    
    if(((JSX_TypePointer *) type)->direct->type != VOIDTYPE) {
      goto failure;
    }

    // Copy a string to a void *
    // same as char * in this context

    // fall through

  case TYPEPAIR(JSVAL_STRING,UNDEFTYPE):
  case TYPEPAIR(JSVAL_STRING,PCHARTYPE):

    // Copy a string to a char *

    if (!will_clean)
      goto failure;

    *(char **)p=JS_GetStringBytes(JSVAL_TO_STRING(v));
    return sizeof(char *);

  case TYPEPAIR(JSVAL_STRING,PSHORTTYPE):

    // Copy a string to a short *

    *(jschar **)p=JS_GetStringChars(JSVAL_TO_STRING(v));
    return sizeof(jschar *);

  case TYPEPAIR(JSVAL_STRING,INTTYPE):
  case TYPEPAIR(JSVAL_STRING,UINTTYPE):

    switch(((JSX_TypeInt *) type)->size) {
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

  case TYPEPAIR(JSVAL_STRING,ACHARTYPE):

    // Copy a string to a char array

    size=JS_GetStringLength(JSVAL_TO_STRING(v));
    if(size < ((JSX_TypeArray *) type)->length) {
      memcpy(*(char **)p,
	     JS_GetStringBytes(JSVAL_TO_STRING(v)),
	     size*sizeof(char));
      memset(*(char **)p + size, 0, (((JSX_TypeArray *) type)->length - size) * sizeof(char));
    } else {
      memcpy(*(char **)p, JS_GetStringBytes(JSVAL_TO_STRING(v)), ((JSX_TypeArray *) type)->length * sizeof(char));
    }
    return sizeof(char) * ((JSX_TypeArray *) type)->length;

  case TYPEPAIR(JSVAL_STRING,ASHORTTYPE):

    // Copy a string to a short array

    size=JS_GetStringLength(JSVAL_TO_STRING(v));
    if(size < ((JSX_TypeArray *) type)->length) {
      memcpy(*(jschar **)p,
	     JS_GetStringChars(JSVAL_TO_STRING(v)),
	     size*sizeof(jschar));
      memset(*(jschar **)p + size, 0, (((JSX_TypeArray *) type)->length - size) * sizeof(jschar));
    } else {
      memcpy(*(jschar **)p, JS_GetStringChars(JSVAL_TO_STRING(v)), ((JSX_TypeArray *) type)->length * sizeof(jschar));
    }
    return sizeof(char) * ((JSX_TypeArray *) type)->length;

  case TYPEPAIR(JSVAL_INT,UNDEFTYPE):
  case TYPEPAIR(JSVAL_INT,POINTERTYPE):
  case TYPEPAIR(JSVAL_INT,PCHARTYPE):
  case TYPEPAIR(JSVAL_INT,PSHORTTYPE):

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
    size = ((JSX_TypeInt *) ((JSX_TypeBitfield *) type)->member)->size;
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
    switch(size != -1 ? size : ((JSX_TypeInt *) type)->size) {

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

    switch(size != -1 ? size : ((JSX_TypeFloat *) type)->size) {

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
  case TYPEPAIR(JSARRAY,PCHARTYPE):
  case TYPEPAIR(JSARRAY,PSHORTTYPE):
    {

    // Copy array elements to a variable array
      
      int containsPointers=0;
      obj=JSVAL_TO_OBJECT(v);
      JS_GetArrayLength(cx, obj, &size);
      elemsize = JSX_TypeSize(((JSX_TypeArray *) type)->member);

      if (will_clean) {
	// The variable array needs to be allocated
	containsPointers = JSX_TypeContainsPointer(((JSX_TypeArray *) type)->member);
	if (containsPointers) {
	  // Allocate twice the space in order to store old pointers
	  *(void **)p=JS_malloc(cx, elemsize*size*2);
	} else {
	  *(void **)p=JS_malloc(cx, elemsize*size);
	}
      }
      
      totsize=0;
    
      for (i=0; i<size; i++) {
	jsval tmp;
	JS_GetElement(cx, obj, i, &tmp);
	int thissize = JSX_Set(cx, *(char **)p + totsize, will_clean, ((JSX_TypePointer *) type)->direct, tmp);
	if (!thissize)
	  goto vararrayfailure2;
	totsize+=thissize;
      }
      
      if (containsPointers) {
	// Make backup of old pointers
	memcpy(*(char **)p+elemsize*size, *(char **)p, elemsize*size);
      }
      
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
  case TYPEPAIR(JSARRAY,ACHARTYPE):
  case TYPEPAIR(JSARRAY,ASHORTTYPE):

    // Copy array elements to a fixed size array
    
    totsize=0;
    size = ((JSX_TypeArray *) type)->length;
    obj=JSVAL_TO_OBJECT(v);

    for (i=0; i<size; i++) {
      jsval tmp;
      JS_GetElement(cx, obj, i, &tmp);
      int thissize = JSX_Set(cx, p + totsize, will_clean, ((JSX_TypeArray *) type)->member, tmp);
      if (!thissize)
	goto fixedarrayfailure;
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

      if(((JSX_TypeStructUnion *) type)->member[i].type->type == BITFIELDTYPE) {
	int length = ((JSX_TypeBitfield *) ((JSX_TypeStructUnion *) type)->member[i].type)->length;
	int offset = ((JSX_TypeStructUnion *) type)->member[i].offset % 8;
	int mask=~(-1<<length);
	int imask=~(imask << offset);
	int tmpint;
	int tmpint2;

	thissize=JSX_Set(cx, (char *)&tmpint, will_clean, ((JSX_TypeStructUnion *) type)->member[i].type, tmp);
	memcpy((char *)&tmpint2, p + ((JSX_TypeStructUnion *) type)->member[i].offset / 8, thissize);
	tmpint=(tmpint2 & imask) | ((tmpint & mask) << offset);
	memcpy(p + ((JSX_TypeStructUnion *) type)->member[i].offset / 8, (char *)&tmpint, thissize);
      } else {
	thissize = JSX_Set(cx, p + ((JSX_TypeStructUnion *) type)->member[i].offset / 8, will_clean, ((JSX_TypeStructUnion *) type)->member[i].type, tmp);
      }
      if (!thissize)
	goto structfailure;
    }

    return JSX_TypeSize(type);
    
  structfailure:

    if (will_clean) {
      for (;i--;) {
	jsval tmp;
	JS_GetProperty(cx, obj, ((JSX_TypeStructUnion *) type)->member[i].name, &tmp);
	JSX_Get(cx, NULL, 0, 2, ((JSX_TypeStructUnion *) type)->member[i].type, &tmp);
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
      int thissize = JSX_Set(cx, p + ((JSX_TypeStructUnion *) type)->member[i].offset / 8, will_clean, ((JSX_TypeStructUnion *) type)->member[i].type, tmp);
      if (!thissize)
	goto structfailure2;
    }

    return JSX_TypeSize(type);

  structfailure2:

    if (will_clean) {
      for (;i--;) {
	jsval tmp;
	JS_GetElement(cx, obj, i, &tmp);
	JSX_Get(cx, NULL, 0, 2, ((JSX_TypeStructUnion *) type)->member[i].type, &tmp);
      }
    }

    return 0;
    // error already thrown

  case TYPEPAIR(JSNULL,STRUCTTYPE):
  case TYPEPAIR(JSNULL,UNIONTYPE):
  case TYPEPAIR(JSNULL,ARRAYTYPE):
  case TYPEPAIR(JSNULL,ACHARTYPE):
  case TYPEPAIR(JSNULL,ASHORTTYPE):
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
  case TYPEPAIR(JSVOID,ACHARTYPE):
  case TYPEPAIR(JSVOID,ASHORTTYPE):
  case TYPEPAIR(JSVOID,INTTYPE):
  case TYPEPAIR(JSVOID,UINTTYPE):
  case TYPEPAIR(JSVOID,FLOATTYPE):

    // Do nothing

    return JSX_TypeSize(type);

  case TYPEPAIR(JSPOINTER,ACHARTYPE):
  case TYPEPAIR(JSPOINTER,ASHORTTYPE):
  case TYPEPAIR(JSPOINTER,ARRAYTYPE):

    // Copy contents pointed to into array

    size=JSX_TypeSize(type);
    ptr=JS_GetPrivate(cx, JSVAL_TO_OBJECT(v));
    if (ptr->ptr==NULL && !JSX_PointerResolve(cx, JSVAL_TO_OBJECT(v)))
      return 0; // Error message in resolve.
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


static int JSX_Set_multi(JSContext *cx, char *ptr, int will_clean, uintN nargs, JSX_ParamType *type, jsval *vp, int convconst, void **argptr) {
  int ret=0;
  int siz, cursiz;
  uintN i;
  JSX_ParamType tmptype = { 0, 0, 0 };
  JSX_ParamType *thistype;

  for (i=0; i<nargs; i++) {
    if (type && type->type->type==VOIDTYPE) // End of param list
      type=0;

    if (JSVAL_IS_OBJECT(*vp) &&
	*vp!=JSVAL_NULL &&
	JS_InstanceOf(cx, JSVAL_TO_OBJECT(*vp), JSX_GetTypeClass(), NULL)) {
      // Paramlist-specified type
      thistype=&tmptype;
      tmptype.type=JS_GetPrivate(cx,JSVAL_TO_OBJECT(*vp));
      vp++;
      i++;
      if (i==nargs) break;
    } else {
      if (type)
	thistype=type;
      else
	thistype=0;
    }

    if (thistype && thistype->type->type==ARRAYTYPE) {
      if (will_clean) {
	// In function calls, arrays are passed by pointer
	*(void **)ptr=JS_malloc(cx, JSX_TypeSize(thistype->type));
	cursiz=JSX_Set(cx, *(void **)ptr, will_clean, thistype->type, *vp);
	if (cursiz)
	  cursiz=sizeof(void *);
	else {
	  JS_free(cx, *(void **)ptr);
	  goto failure;
	}
      } else
	goto failure;
    } else {
      cursiz=JSX_Set(cx, ptr?ptr:*argptr, will_clean, thistype ? thistype->type : 0, *vp);
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
    siz=JSX_Get(cx, ptr?ptr:*argptr, 0, 2, type ? type->type : 0, --vp);
  }

  return 0;
}

JSClass * JSX_GetPointerClass(void) {
  return &JSX_PointerClass;
}

static JSBool JSX_InitPointerAlloc(JSContext *cx, JSObject *retobj, JSObject *type) {
  struct JSX_Pointer *retpriv;
  int size;
  
  if (!JS_InstanceOf(cx, type, JSX_GetTypeClass(), NULL)) {
    JSX_ReportException(cx, "Wrong type argument");
    goto end_false;
  }
      
  if (!JS_DefineProperty(cx, retobj, "type", OBJECT_TO_JSVAL(type), 0, 0, JSPROP_READONLY | JSPROP_PERMANENT))
    goto end_false;

  size=JSX_TypeSize((struct JSX_Type *)JS_GetPrivate(cx, type));
  retpriv=JS_malloc(cx, sizeof(struct JSX_Pointer)+size);

  if (!retpriv)
    goto end_false;

  JS_SetPrivate(cx, retobj, retpriv);

  retpriv->ptr=retpriv+1;
  retpriv->type=(struct JSX_Type *)JS_GetPrivate(cx, type);
  retpriv->finalize=0;

  goto end_true;

 end_true:
  return JS_TRUE;

 end_false:
  return JS_FALSE;

}

static JSBool JSX_InitPointerCallback(JSContext *cx, JSObject *retobj, JSFunction *fun, JSObject *typeobj) {
  struct JSX_Callback *retpriv;
  
  if (!JS_InstanceOf(cx, typeobj, JSX_GetTypeClass(), NULL) ||
      ((struct JSX_Type *)JS_GetPrivate(cx, typeobj))->type!=FUNCTIONTYPE) {
    JSX_ReportException(cx, "Type is not a C function");
    goto end_false;
  }
      
  if (!JS_DefineProperty(cx, retobj, "type", OBJECT_TO_JSVAL(typeobj), 0, 0, JSPROP_READONLY | JSPROP_PERMANENT))
    goto end_false;
  if (!JS_DefineProperty(cx, retobj, "function", OBJECT_TO_JSVAL(JS_GetFunctionObject(fun)), 0, 0, JSPROP_READONLY | JSPROP_PERMANENT))
    goto end_false;

  retpriv=JS_malloc(cx, sizeof(struct JSX_Callback));
  if (!retpriv)
    goto end_false;

  void *code;
  retpriv->writeable=ffi_closure_alloc(sizeof(ffi_closure), &code);
  retpriv->ptr=code;
  retpriv->cx=cx;
  retpriv->fun=fun;
  retpriv->finalize=ffi_closure_free; //This would free the code address, not always identical to writeable address. So it is checked in finalize.
  retpriv->type=(struct JSX_Type *)JS_GetPrivate(cx, typeobj);

  if(ffi_prep_closure_loc(retpriv->writeable, JSX_GetCIF(cx, (JSX_TypeFunction *) retpriv->type), JSX_Pointer_Callback, retpriv, retpriv->ptr) != FFI_OK)
    goto end_false;

  JS_SetPrivate(cx, retobj, retpriv);


  goto end_true;

 end_true:
  return JS_TRUE;

 end_false:
  return JS_FALSE;

}


static JSBool JSX_InitPointerString(JSContext *cx, JSObject *retobj, JSString *str) {
  if (!JS_DefineProperty(cx, retobj, "type", OBJECT_TO_JSVAL(JSX_GetType(INTTYPE,0,0)), 0, 0, JSPROP_READONLY | JSPROP_PERMANENT))
    return JS_FALSE;

  struct JSX_Pointer *ret;
  int length = JS_GetStringLength(str);
  ret=JS_malloc(cx, sizeof(struct JSX_Pointer)+sizeof(char)*(length+1));
  ret->ptr=ret+1;
  ret->type=(struct JSX_Type *)JS_GetPrivate(cx,JSX_GetType(INTTYPE,0,0));
  ret->finalize=0;
  JS_SetPrivate(cx, retobj, ret);
  memcpy(ret->ptr, JS_GetStringBytes(str), sizeof(char)*(length+1));
  return JS_TRUE;
}


// If typeobj is null, a type property must have been assigned to retobj before calling initpointer.

JSBool JSX_InitPointer(JSContext *cx, JSObject *retobj, JSObject *typeobj) {
  struct JSX_Pointer *ret;

  if (!typeobj) {
    jsval typeval;
    if (!JS_GetProperty(cx, retobj, "type", &typeval))
      goto end_false;
    if (typeval==JSVAL_VOID)
      goto end_false;
    typeobj=JSVAL_TO_OBJECT(typeval);
  }
  if (!JS_DefineProperty(cx, retobj, "type", OBJECT_TO_JSVAL(typeobj), 0, 0, JSPROP_READONLY | JSPROP_PERMANENT))
    goto end_false;

  ret=JS_malloc(cx, sizeof(struct JSX_Pointer));
  if (!ret)
    goto end_false;
  ret->ptr=0;
  ret->type=(struct JSX_Type *)JS_GetPrivate(cx,typeobj);
  ret->finalize=0;
  JS_SetPrivate(cx, retobj, ret);

  goto end_true;

 end_true:
  return JS_TRUE;

 end_false:
  return JS_FALSE;

}

static JSBool JSX_Pointer_malloc(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
  JSObject *newobj;
  int length;
  struct JSX_Pointer *ret;


  if (argc<1 || !JSVAL_IS_INT(argv[0]) || JSVAL_TO_INT(argv[0])<=0) {
    JSX_ReportException(cx, "Wrong argument type to malloc");
    goto end_false;
  }

  newobj=JS_NewObject(cx, &JSX_PointerClass, 0, 0);
  *rval=OBJECT_TO_JSVAL(newobj);

  if (!JS_DefineProperty(cx, obj, "type", OBJECT_TO_JSVAL(JSX_GetType(VOIDTYPE,0,0)), 0, 0, JSPROP_READONLY | JSPROP_PERMANENT))
    goto end_false;

  length=INT_TO_JSVAL(argv[0]);
  ret=JS_malloc(cx, sizeof(struct JSX_Pointer)+length);
  if (!ret)
    goto end_false;
  ret->ptr=ret+1;
  ret->type=(struct JSX_Type *)JS_GetPrivate(cx,JSX_GetType(VOIDTYPE,0,0));
  ret->finalize=0;
  JS_SetPrivate(cx, newobj, ret);

  goto end_true;

 end_true:
  return JS_TRUE;

 end_false:
  return JS_FALSE;

}


static JSBool JSX_Pointer_calloc(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
  JSObject *newobj;
  int length;
  struct JSX_Pointer *ret;

  if (argc<1 || !JSVAL_IS_INT(argv[0]) || JSVAL_TO_INT(argv[0])<=0) {
    JSX_ReportException(cx, "Wrong argument type to calloc");
    goto end_false;
  }

  newobj=JS_NewObject(cx, &JSX_PointerClass, 0, 0);
  *rval=OBJECT_TO_JSVAL(newobj);

  if (!JS_DefineProperty(cx, obj, "type", OBJECT_TO_JSVAL(JSX_GetType(VOIDTYPE,0,0)), 0, 0, JSPROP_READONLY | JSPROP_PERMANENT))
    goto end_false;

  length=INT_TO_JSVAL(argv[0]);
  ret=JS_malloc(cx, sizeof(struct JSX_Pointer)+length);
  if (!ret)
    goto end_false;
  ret->ptr=ret+1;
  ret->type=(struct JSX_Type *)JS_GetPrivate(cx,JSX_GetType(VOIDTYPE,0,0));
  ret->finalize=0;
  JS_SetPrivate(cx, newobj, ret);
  memset(ret->ptr,0,length);

  goto end_true;

 end_true:
  return JS_TRUE;

 end_false:
  return JS_FALSE;

}


static JSBool JSX_Pointer_realloc(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
  int length;
  struct JSX_Pointer *ret;
  void *newptr;


  if (argc<1 || !JSVAL_IS_INT(argv[0])) {
    JSX_ReportException(cx, "Wrong argument type to realloc");
    goto end_false;
  }

  ret=JS_GetPrivate(cx, obj);

  if (ret->ptr!=ret+1) {
    JSX_ReportException(cx, "Pointer was not allocated with malloc or calloc");
    goto end_false;
  }
    
  length=INT_TO_JSVAL(argv[0]);

  ret=JS_GetPrivate(cx, obj);

  newptr=JS_realloc(cx, ret, sizeof(struct JSX_Pointer)+length);
  if (!newptr)
    goto end_false;
  ret=newptr;
  ret->ptr=ret+1;
  JS_SetPrivate(cx, obj, ret);

  goto end_true;

 end_true:
  return JS_TRUE;

 end_false:
  return JS_FALSE;

}


static JSBool JSX_InitPointerUCString(JSContext *cx, JSObject *retobj, JSString *str) {
  int length;
  struct JSX_Pointer *ret;

  if (!JS_DefineProperty(cx, retobj, "type", OBJECT_TO_JSVAL(JSX_GetType(INTTYPE,1,0)), 0, 0, JSPROP_READONLY | JSPROP_PERMANENT))
    goto end_false;

  length=JS_GetStringLength(str);
  ret=JS_malloc(cx, sizeof(struct JSX_Pointer)+sizeof(short)*(length+1));
  ret->ptr=ret+1;
  ret->type=(struct JSX_Type *)JS_GetPrivate(cx,JSX_GetType(INTTYPE,1,0));
  ret->finalize=0;
  JS_SetPrivate(cx, retobj, ret);

  memcpy(ret->ptr, JS_GetStringChars(str), sizeof(short)*(length+1));

  goto end_true;

 end_true:
  return JS_TRUE;

 end_false:
  return JS_FALSE;

}


static JSBool JSX_PointerResolve(JSContext *cx, JSObject *obj) {
  char *nametmp;
  char *name;
  JSObject *dl;
  jsval tmp;
  struct JSX_Pointer *ptr;

  ptr=(struct JSX_Pointer *)JS_GetPrivate(cx, obj);
  if (!ptr)
    goto end_false;

  if (ptr->ptr)
    goto end_true;

  JS_LookupProperty(cx, obj, "dl", &tmp);
  if (tmp==JSVAL_VOID || tmp==JSVAL_NULL || !JSVAL_IS_OBJECT(tmp))
    dl=0;
  else
    dl=JSVAL_TO_OBJECT(tmp);

  JS_LookupProperty(cx, obj, "symbol", &tmp);
  if (tmp==JSVAL_VOID || !JSVAL_IS_STRING(tmp))
    goto end_false;
  else
    nametmp=JS_GetStringBytes(JSVAL_TO_STRING(tmp));

  name=JS_malloc(cx, JS_GetStringLength(JSVAL_TO_STRING(tmp))+1);
  memcpy(name,nametmp,JS_GetStringLength(JSVAL_TO_STRING(tmp)));
  name[JS_GetStringLength(JSVAL_TO_STRING(tmp))]=0;

  {
    static void *globaldl=0;
    if (globaldl==0) {
      globaldl=dlopen(0,RTLD_LAZY);
    }
    if (dl)
      ptr->ptr=(void *)dlsym(JS_GetPrivate(cx, dl),name);
    else
      ptr->ptr=(void *)dlsym(globaldl,name);
  }

  if (!ptr->ptr) {
    JSX_ReportException(cx, "Unresolved symbol %s",name);
    JS_free(cx, name);
    goto end_false;
  }

  JS_free(cx, name);
  goto end_true;

 end_true:
  return JS_TRUE;

 end_false:
  return JS_FALSE;

}


static JSBool JSX_Pointer_cast(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
  JSObject *newobj;
  struct JSX_Pointer *ptr, *newptr;
  

  if (argc<1 ||
      !JS_InstanceOf(cx, JSVAL_TO_OBJECT(argv[0]), JSX_GetTypeClass(), NULL)) {
    JSX_ReportException(cx, "Wrong argument to cast");
    goto end_false;
  }

  ptr=JS_GetPrivate(cx, obj);
  if (!ptr->ptr && !JSX_PointerResolve(cx, obj))
    goto end_false;

  newobj=JS_NewObject(cx, &JSX_PointerClass, 0, 0);
  *rval=OBJECT_TO_JSVAL(newobj);
  if (!JSX_InitPointer(cx, newobj, JSVAL_TO_OBJECT(argv[0]))) {
    goto end_false;
  }

  newptr=JS_GetPrivate(cx, newobj);
  newptr->ptr=ptr->ptr;

  goto end_true;

 end_true:
  return JS_TRUE;

 end_false:
  return JS_FALSE;

}


static JSBool JSX_Pointer_new(JSContext *cx, JSObject *origobj, uintN argc, jsval *argv, jsval *rval) {
  JSObject *obj;


  if (!JS_IsConstructing(cx)) {
    obj=JS_NewObject(cx, &JSX_PointerClass, 0, 0);
    *rval=OBJECT_TO_JSVAL(obj);
  } else {
    obj=origobj;
  }

  if (argc==0)
    goto end_true;

  if (argc>=1 &&
      JSVAL_IS_OBJECT(argv[0]) &&
      !JSVAL_IS_NULL(argv[0]) &&
      JS_InstanceOf(cx, JSVAL_TO_OBJECT(argv[0]), JSX_GetTypeClass(), NULL)) {


    JSObject *typeObject=JSVAL_TO_OBJECT(argv[0]);

    if (argc>=2 &&
	JSVAL_IS_OBJECT(argv[1]) &&
	JS_ObjectIsFunction(cx, JSVAL_TO_OBJECT(argv[1]))) {

      // Callback constructor
      
      struct JSX_Pointer *ptr=JS_GetPrivate(cx, typeObject);
      struct JSX_Type *type=ptr->type;
      
      if (type->type==POINTERTYPE) { // Accept both function type and pointer to function type
	typeObject = ((JSX_TypePointer *) type)->direct->typeObject;
	type=JS_GetPrivate(cx, typeObject);
      }
      
      if (!JSX_InitPointerCallback(cx, obj, JS_ValueToFunction(cx, argv[1]), typeObject)) {
	goto end_false;
      }
      
      goto end_true;
    } else {

      // Allocation constructor
      
      if (!JSX_InitPointerAlloc(cx, obj, JSVAL_TO_OBJECT(argv[0]))) {
	goto end_false;
      }
      
      if (argc>=2 && argv[1]!=JSVAL_VOID) {
	struct JSX_Pointer *ptr=JS_GetPrivate(cx,obj);
	if (!JSX_Set(cx, ptr->ptr, 0, ptr->type, argv[1])) {
	  goto end_false;
	}
      }
    }

    goto end_true;
  }

  JSX_ReportException(cx, "Wrong arguments to Pointer");
  goto end_false;

 end_true:
  return JS_TRUE;

 end_false:
  return JS_FALSE;

}


static void JSX_Pointer_finalize(JSContext *cx, JSObject *obj) {
  struct JSX_Pointer *ptr=(struct JSX_Pointer *)JS_GetPrivate(cx, obj);

  if (ptr) {
    if (ptr->finalize) {
      if (ptr->finalize==ffi_closure_free) {
	ffi_closure_free(((struct JSX_Callback *)ptr)->writeable);
      } else {
	(*ptr->finalize)(ptr->ptr);
      }
    }
    JS_free(cx, ptr);
  }
}


static JSBool JSX_Pointer_call(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
  char *argbuf;
  void **argptr=0;
  char *retbuf=0;
  size_t arg_size=0;
  ffi_type **arg_types=0;
  ffi_cif *cif;
  struct JSX_Type *type;
  struct JSX_Pointer *ptr=JS_GetPrivate(cx,obj);

  if (ptr->ptr==0 && !JSX_PointerResolve(cx, obj)) {
    goto failure;
  }

  type=ptr->type;

  if (type->type!=FUNCTIONTYPE) {
    JSX_ReportException(cx, "call: Wrong pointer type");
    goto failure;
  }

  arg_types=JS_malloc(cx, sizeof(ffi_cif)+sizeof(ffi_type *)*(argc+1));
  cif=(ffi_cif *)(arg_types+(argc+1));

  int real_argc;
  arg_size = JSX_TypeSize_multi(cx, argc, ((JSX_TypeFunction *) type)->param, argv, arg_types);

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

  argptr=(void **)JS_malloc(cx, arg_size + argc*sizeof(void *) + retsize + 8);

  retbuf=(char *)(argptr + argc);
  argbuf=retbuf + retsize + 8; // ffi overwrites a few bytes on some archs.

  if (arg_size) {
    if(!JSX_Set_multi(cx, (void *) argbuf, 1, argc, ((JSX_TypeFunction *) type)->param, argv, 1, argptr))
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

  struct JSX_Pointer *ptr;
  int ret;


  ptr=(struct JSX_Pointer *)JS_GetPrivate(cx, obj);

  *vp=JSVAL_VOID;
  
  if (ptr->type->type!=FUNCTIONTYPE) {
    if (ptr->ptr==0 && !JSX_PointerResolve(cx, obj))
      goto end_false;
  }

  ret=JSX_Get(cx, ptr->ptr, 0, 0, ptr->type, vp);

  if (!ret)
    goto end_false;

  if (ret==-1) {
    // Created new function
    JS_DefineProperty(cx, JSVAL_TO_OBJECT(*vp), "__ptr__", OBJECT_TO_JSVAL(obj), 0, 0, JSPROP_READONLY | JSPROP_PERMANENT);
  }

  goto end_true;

 end_true:
  return JS_TRUE;

 end_false:
  return JS_FALSE;

}


static JSBool JSX_Pointer_setdollar(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {

  struct JSX_Pointer *ptr;


  ptr=(struct JSX_Pointer *)JS_GetPrivate(cx, obj);

  if (ptr->ptr==0 && !JSX_PointerResolve(cx, obj))
    goto end_false;

  if (!JSX_Set(cx, ptr->ptr, 0, ptr->type, *vp))
    goto end_false;

  goto end_true;

 end_true:
  return JS_TRUE;

 end_false:
  return JS_FALSE;

}


static JSBool JSX_Pointer_getfinalize(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
  *vp=JSVAL_VOID;
  return JS_TRUE;
}

static JSBool JSX_Pointer_setfinalize(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {

  struct JSX_Pointer *ptr;
  struct JSX_Pointer *finptr;
  struct JSX_Type *type;
  jsval ptrobj;


  ptr=(struct JSX_Pointer *)JS_GetPrivate(cx, obj);

  if (*vp==JSVAL_NULL || *vp==JSVAL_VOID) {
    ptr->finalize=0;
    goto end_true;
  }

  if (!JSVAL_IS_OBJECT(*vp) ||
      !JS_ObjectIsFunction(cx, JSVAL_TO_OBJECT(*vp)) ||
      !JS_LookupProperty(cx, JSVAL_TO_OBJECT(*vp), "__ptr__", &ptrobj) ||
      !JS_InstanceOf(cx, JSVAL_TO_OBJECT(ptrobj), JSX_GetPointerClass(), NULL)) {
    JSX_ReportException(cx, "Wrong value type for finalize property");
    goto end_false;
  }

  finptr=JS_GetPrivate(cx, JSVAL_TO_OBJECT(ptrobj));
  type=finptr->type;
  if (type->type!=FUNCTIONTYPE ||
      ((JSX_TypeFunction *) type)->nParam != 1 ||
      ((JSX_TypeFunction *) type)->param[0].type->type != POINTERTYPE ||
      ((JSX_TypeFunction *) type)->param[0].isConst) {
    JSX_ReportException(cx, "Wrong function type for finalize property");
    goto end_false;
  }

  if (!finptr->ptr && !JSX_PointerResolve(cx, JSVAL_TO_OBJECT(ptrobj)))
    goto end_false;

  ptr->finalize=finptr->ptr;

  goto end_true;

 end_true:
  return JS_TRUE;

 end_false:
  return JS_FALSE;

}


static JSBool JSX_Pointer_resolve(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {


  if (!JSX_PointerResolve(cx, obj)) {
    JSX_ReportException(cx, "Unable to resolve");
    goto end_false;
  }

  goto end_true;

 end_true:
  return JS_TRUE;

 end_false:
  return JS_FALSE;

}

static JSBool JSX_Pointer_pr_UCString(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
  struct JSX_Pointer *ptr;
  int length;

  ptr=(struct JSX_Pointer *)JS_GetPrivate(cx, obj);

  if (argc<1 || !JSVAL_IS_INT(argv[0]))
    *rval=STRING_TO_JSVAL(JS_NewUCStringCopyZ(cx, (jschar *)ptr->ptr));
  else {
    length=JSVAL_TO_INT(argv[0]);
    *rval=STRING_TO_JSVAL(JS_NewUCStringCopyN(cx, (jschar *)ptr->ptr, length));
  }

  return JS_TRUE;
}


static JSBool JSX_Pointer_pr_string(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
  struct JSX_Pointer *ptr;
  int length;

  ptr=(struct JSX_Pointer *)JS_GetPrivate(cx, obj);

  if (argc<1 || !JSVAL_IS_INT(argv[0]))
    *rval=STRING_TO_JSVAL(JS_NewStringCopyZ(cx, (char *)ptr->ptr));
  else {
    length=JSVAL_TO_INT(argv[0]);
    *rval=STRING_TO_JSVAL(JS_NewStringCopyN(cx, (char *)ptr->ptr, length));
  }

  return JS_TRUE;
}


static JSBool JSX_Pointer_UCString(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

  JSObject *newobj;


  if (argc<1 || !JSVAL_IS_STRING(argv[0])) {
    JSX_ReportException(cx, "Wrong argument type to UCString");
    goto end_false;
  }

  newobj=JS_NewObject(cx, &JSX_PointerClass, 0, 0);
  *rval=OBJECT_TO_JSVAL(newobj);

  if (!JSX_InitPointerUCString(cx, newobj, JSVAL_TO_STRING(argv[0]))) {
    goto end_false;
  }

  goto end_true;

 end_true:
  return JS_TRUE;

 end_false:
  return JS_FALSE;

}


static JSBool JSX_Pointer_string(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
  if (argc<1 || !JSVAL_IS_STRING(argv[0])) {
    JSX_ReportException(cx, "Wrong argument type to string");
    return JS_FALSE;
  }

  JSObject *newobj;
  newobj=JS_NewObject(cx, &JSX_PointerClass, 0, 0);
  *rval=OBJECT_TO_JSVAL(newobj);
  if(!JSX_InitPointerString(cx, newobj, JSVAL_TO_STRING(argv[0]))) return JS_FALSE;
  return JS_TRUE;
}


static JSBool JSX_Pointer_toString(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
  struct JSX_Pointer *ptr;
  char buf[20];
  ptr=(struct JSX_Pointer *)JS_GetPrivate(cx, obj);
  if(!ptr->ptr && !JSX_PointerResolve(cx, obj)) return JS_FALSE;
  sprintf(buf,"%08x",ptr->ptr);
  *rval=STRING_TO_JSVAL(JS_NewStringCopyZ(cx, buf));
  return JS_TRUE;
}


static JSBool JSX_Pointer_valueOf(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
  struct JSX_Pointer *ptr;
  jsdouble val;
  ptr=(struct JSX_Pointer *)JS_GetPrivate(cx, obj);
  val=((jsdouble)(long)ptr->ptr)/JSX_TypeSize(ptr->type);
  JS_NewNumberValue(cx, val, rval);
  return JS_TRUE;
}


/*
  Pointer.prototype.pointer

  Returns pointer object
 */

static JSBool JSX_Pointer_member(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
  JSObject *newobj;
  struct JSX_Pointer *ptr;
  char *thisptr;
  struct JSX_Type *type;
  JSX_TypePointer tmptype;
  int n;
  uintN i;


  ptr=JS_GetPrivate(cx, obj);
  if (ptr->ptr==0 && !JSX_PointerResolve(cx,obj))
    goto end_false;

  thisptr=ptr->ptr;

  type=(struct JSX_Type *)&tmptype;
  ((JSX_TypePointer *) type)->type = POINTERTYPE;
  ((JSX_TypePointer *) type)->direct = ptr->type;

  if (argc==0) {
    JSX_ReportException(cx, "Missing argument to pointer.member");
    goto end_false;
  }

  for (i=0; i<argc; i++) {
    switch(type->type) {
    case POINTERTYPE:
    case ARRAYTYPE:
      if (!JSVAL_IS_INT(argv[i])) {
	JSX_ReportException(cx, "Wrong argument no %d to pointer.member", i);
	goto end_false;
      }
      type = ((JSX_TypePointer *) type)->direct;
      thisptr+=JSX_TypeSize(type)*JSVAL_TO_INT(argv[i]);
      break;

    case STRUCTTYPE:
    case UNIONTYPE:
      if (JSVAL_IS_INT(argv[i])) {
	n=JSVAL_TO_INT(argv[i]);
      } else if (JSVAL_IS_STRING(argv[i])) {
        char *myname=JS_GetStringBytes(JSVAL_TO_STRING(argv[i]));

	for(n = 0; n < ((JSX_TypeStructUnion *) type)->nMember; n++) {
	  if(strcmp(((JSX_TypeStructUnion *) type)->member[n].name, myname) == 0)
	    break;
	}

	if(n == ((JSX_TypeStructUnion *) type)->nMember) {
 	  JSX_ReportException(cx, "Unknown member %s as index no %d", myname, i);
 	  goto end_false;
	}

	if(((JSX_TypeStructUnion *) type)->member[n].type->type == BITFIELDTYPE) {
 	  JSX_ReportException(cx, "Bitfield member %s as index no %d", myname, i);
 	  goto end_false;
	}

      } else {
	JSX_ReportException(cx, "Wrong argument no %d to pointer.member", i);
	goto end_false;
      }

      if(n < 0 || n >= ((JSX_TypeStructUnion *) type)->nMember) {
	JSX_ReportException(cx, "Index %d out of bounds: %d not in range [%d, %d]", i, n, 0, ((JSX_TypeStructUnion *) type)->nMember - 1);
	goto end_false;
      }
      thisptr += ((JSX_TypeStructUnion *) type)->member[n].offset / 8;
      type = ((JSX_TypeStructUnion *) type)->member[n].type;
      break;

    default:
      JSX_ReportException(cx, "Index no %d to memberless C type", i);
      goto end_false;
    }
  }

  newobj=JS_NewObject(cx, &JSX_PointerClass, 0, 0);
  *rval=OBJECT_TO_JSVAL(newobj);

  ptr=(struct JSX_Pointer *)malloc(sizeof(struct JSX_Pointer));
  ptr->type=type;
  ptr->ptr=thisptr;
  ptr->finalize=0;

  JS_DefineProperty(cx, newobj, "type", OBJECT_TO_JSVAL(ptr->type->typeObject), 0, 0, JSPROP_READONLY | JSPROP_PERMANENT);
  JS_SetPrivate(cx, newobj, ptr);
  goto end_true;

 end_true:
  return JS_TRUE;

 end_false:
  return JS_FALSE;

}


static JSBool JSX_Pointer_getProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
  struct JSX_Pointer *ptr;
  int ret;


  if (!JSVAL_IS_INT(id))
    goto end_true; // Only handle numerical properties

  ptr=(struct JSX_Pointer *)JS_GetPrivate(cx, obj);
  if(ptr->ptr == 0 && !JSX_PointerResolve(cx, obj)) return JS_FALSE;

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

  goto end_true;

 end_true:
  return JS_TRUE;

 end_false:
  return JS_FALSE;

}


static JSBool JSX_Pointer_setProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
  if(!JSVAL_IS_INT(id)) return JS_TRUE; // Only handle numerical properties

  struct JSX_Pointer *ptr;
  ptr = (struct JSX_Pointer *) JS_GetPrivate(cx, obj);
  if(ptr->ptr == 0 && !JSX_PointerResolve(cx, obj)) return JS_FALSE;

  int ret;
  ret = JSX_Set(cx, (char *) ptr->ptr + JSX_TypeSize(ptr->type) * JSVAL_TO_INT(id), 0, ptr->type, *vp);
  if(ret == 0) return JS_FALSE;

  return JS_TRUE;
}


/*
  Indirect entry point for callback from C program
 */

static void JSX_Pointer_Callback(ffi_cif *cif, void *ret, void **args, void *user_data) {
  struct JSX_Callback *cb=user_data;
  jsval *tmp_argv;
  int i;
  int argsize;
  jsval rval=JSVAL_VOID;
  JSX_TypeFunction *type = (JSX_TypeFunction *) cb->type;

  tmp_argv=(jsval *)JS_malloc(cb->cx, sizeof(jsval)*type->nParam);
  if(!tmp_argv) return;
  
  for (i=0; i<type->nParam; i++) {
    tmp_argv[i]=JSVAL_VOID;
    JS_AddRoot(cb->cx, tmp_argv+i);
  }
  JS_AddRoot(cb->cx, &rval);
  
  argsize=JSX_Get_multi(cb->cx, 0, type->nParam, type->param, tmp_argv, 1, args);

  if (!JS_CallFunction(cb->cx, JS_GetGlobalObject(cb->cx), cb->fun, type->nParam, tmp_argv, &rval)) {
    //    printf("FAILCALL\n");
  }
  
  JSX_Set_multi(cb->cx, 0, 0, type->nParam, type->param, tmp_argv, 0, args);

  JS_RemoveRoot(cb->cx, &rval);
  for (i=0; i<type->nParam; i++) {
    JS_RemoveRoot(cb->cx, tmp_argv+i);
  }
  JS_free(cb->cx, tmp_argv);
  
  // dont convert return value if void

  if (type->returnType->type != VOIDTYPE) {
    JSX_Set(cb->cx, ret, 0, type->returnType, rval);
  }
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
    {"UCstring",JSX_Pointer_UCString,1,0,0},
    {"string",JSX_Pointer_string,1,0,0},
    {"malloc",JSX_Pointer_malloc,1,0,0},
    {"calloc",JSX_Pointer_calloc,1,0,0},
    {0,0,0,0,0}
  };

  static struct JSFunctionSpec memberfunc[]={
    {"UCstring",JSX_Pointer_pr_UCString,1,0,0},
    {"cast",JSX_Pointer_cast,1,0,0},
    {"member",JSX_Pointer_member,1,0,0},
    {"realloc",JSX_Pointer_realloc,1,0,0},
    {"resolve",JSX_Pointer_resolve,0,0,0},
    {"string",JSX_Pointer_pr_string,1,0,0},
    {"valueOf",JSX_Pointer_valueOf,0,0,0},
    {"toString",JSX_Pointer_toString,0,0,0},
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

