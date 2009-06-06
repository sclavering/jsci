#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <dlfcn.h>
#include <alloca.h>
#include "jsci.h"



static JSBool JSX_Pointer_new(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
static void JSX_Pointer_finalize(JSContext *cx, JSObject *obj);
static JSBool JSX_Pointer_call(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
static JSBool JSX_Pointer_getProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp);
static JSBool JSX_Pointer_setProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp);
static void JSX_Pointer_Callback(ffi_cif *cif, void *ret, void **args, void *user_data);
static JSBool JSX_InitPointerAlloc(JSContext *cx, JSObject *obj, JSObject *type);


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

const char *JSX_typenames[] = {
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

const char *JSX_jstypenames[] = {
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


JSClass * JSX_GetPointerClass(void) {
  return &JSX_PointerClass;
}


static JSBool JSX_InitPointerAlloc(JSContext *cx, JSObject *retobj, JSObject *type) {
  if (!JS_InstanceOf(cx, type, JSX_GetTypeClass(), NULL)) {
    JSX_ReportException(cx, "Wrong type argument");
    return JS_FALSE;
  }

  JSX_Type *t = (JSX_Type *) JS_GetPrivate(cx, type);
  int size = t->SizeInBytes();
  JSX_Pointer *retpriv = (JSX_Pointer *) new char[sizeof(JSX_Pointer) + size];

  if (!retpriv)
    return JS_FALSE;

  JS_SetPrivate(cx, retobj, retpriv);

  retpriv->ptr=retpriv+1;
  retpriv->type = (JSX_Type *) JS_GetPrivate(cx, type);
  retpriv->finalize=0;

  return JS_TRUE;
}


JSBool JSX_InitPointerCallback(JSContext *cx, JSObject *retobj, JSFunction *fun, JSX_Type *type) {
  if(type->type != FUNCTIONTYPE) {
    JSX_ReportException(cx, "Type is not a C function");
    return JS_FALSE;
  }

  if (!JS_DefineProperty(cx, retobj, "function", OBJECT_TO_JSVAL(JS_GetFunctionObject(fun)), 0, 0, JSPROP_READONLY | JSPROP_PERMANENT))
    return JS_FALSE;

  JSX_Callback *retpriv = new JSX_Callback;
  if (!retpriv)
    return JS_FALSE;

  void *code;
  retpriv->writeable=ffi_closure_alloc(sizeof(ffi_closure), &code);
  retpriv->ptr=code;
  retpriv->cx=cx;
  retpriv->fun=fun;
  retpriv->finalize=ffi_closure_free; //This would free the code address, not always identical to writeable address. So it is checked in finalize.
  retpriv->type = type;

  if(ffi_prep_closure_loc((ffi_closure*) retpriv->writeable, ((JSX_TypeFunction *) retpriv->type)->GetCIF(), JSX_Pointer_Callback, retpriv, retpriv->ptr) != FFI_OK)
    return JS_FALSE;

  JS_SetPrivate(cx, retobj, retpriv);

  return JS_TRUE;
}


JSBool JSX_InitPointer(JSContext *cx, JSObject *retobj, JSObject *typeobj) {
  // xxx a hack to save typeobj from garbage collection, and thus stop the free()ing of the JSX_Type struct we share 
  if(!JS_DefineProperty(cx, retobj, "xxx", OBJECT_TO_JSVAL(typeobj), 0, 0, JSPROP_READONLY | JSPROP_PERMANENT))
    return JS_FALSE;

  JSX_Pointer *ret = new JSX_Pointer;
  if (!ret)
    return JS_FALSE;
  ret->ptr=0;
  ret->type = (JSX_Type *) JS_GetPrivate(cx, typeobj);
  ret->finalize=0;
  JS_SetPrivate(cx, retobj, ret);

  return JS_TRUE;
}


static JSBool Pointer_malloc(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
  if (argc<1 || !JSVAL_IS_INT(argv[0]) || JSVAL_TO_INT(argv[0])<=0) {
    JSX_ReportException(cx, "Wrong argument type to malloc");
    return JS_FALSE;
  }

  JSObject *newobj = JS_NewObject(cx, &JSX_PointerClass, 0, 0);
  *rval=OBJECT_TO_JSVAL(newobj);
  int length = INT_TO_JSVAL(argv[0]);
  JSX_Pointer *ret = (JSX_Pointer *) new char[sizeof(JSX_Pointer) + length];
  if (!ret)
    return JS_FALSE;
  ret->ptr=ret+1;
  ret->type = GetVoidType();
  ret->finalize=0;
  JS_SetPrivate(cx, newobj, ret);

  return JS_TRUE;
}


static JSBool Pointer_proto_cast(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
  if(!JSVAL_IS_OBJECT(argv[0]) || JSVAL_IS_NULL(argv[0]) || !JS_InstanceOf(cx, JSVAL_TO_OBJECT(argv[0]), JSX_GetTypeClass(), NULL)) {
    JSX_ReportException(cx, "Pointer.prototype.cast(): argument must be a Type instance");
    return JS_FALSE;
  }

  JSX_Pointer *ptr = (JSX_Pointer *) JS_GetPrivate(cx, obj);
  JSObject *newobj = JS_NewObject(cx, &JSX_PointerClass, 0, 0);
  *rval=OBJECT_TO_JSVAL(newobj);
  if (!JSX_InitPointer(cx, newobj, JSVAL_TO_OBJECT(argv[0]))) {
    return JS_FALSE;
  }

  JSX_Pointer *newptr = (JSX_Pointer *) JS_GetPrivate(cx, newobj);
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
    JSX_Pointer *ptr = (JSX_Pointer *) JS_GetPrivate(cx, typeObject);
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
    JSX_Pointer *ptr = (JSX_Pointer *) JS_GetPrivate(cx,obj);
    if(!JSX_Set(cx, (char*) ptr->ptr, 0, ptr->type, argv[1])) return JS_FALSE;
  }

  return JS_TRUE;
}


static void JSX_Pointer_finalize(JSContext *cx, JSObject *obj) {
  JSX_Pointer *ptr = (JSX_Pointer *) JS_GetPrivate(cx, obj);
  delete ptr;
}


static JSBool JSX_Pointer_call(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
  JSX_Pointer *ptr = (JSX_Pointer *) JS_GetPrivate(cx, obj);
  JSX_Type *type = ptr->type;

  if(type->type != FUNCTIONTYPE) {
    JSX_ReportException(cx, "call: Wrong pointer type");
    return JS_FALSE;
  }

  JSX_TypeFunction *ft = (JSX_TypeFunction *) type;

  ffi_type **arg_types = new ffi_type*[argc + 1];
  ffi_cif *cif = new ffi_cif; // xxx we don't seem to free this

  size_t arg_size = JSX_TypeSize_multi(cx, argc, ft->param, argv, arg_types);

  int real_argc;
  for (real_argc=0; arg_types[real_argc]; real_argc++)
    ;

  if(real_argc > ft->nParam) {
    memcpy(arg_types, ft->GetCIF()->arg_types, sizeof(ffi_type *) * ft->nParam);
    ffi_prep_cif(cif, FFI_DEFAULT_ABI, real_argc, ft->returnType->GetFFIType(), arg_types);
  } else {
    cif = ft->GetCIF();
  }

  int retsize = ft->returnType->SizeInBytes();

  void **argptr = 0;
  argptr=(void **)JS_malloc(cx, arg_size + argc*sizeof(void *) + retsize + 8);

  char *retbuf = 0;
  retbuf=(char *)(argptr + argc);
  char *argbuf;
  argbuf=retbuf + retsize + 8; // ffi overwrites a few bytes on some archs.

  if (arg_size) {
    if(!JSX_Set_multi(cx, argbuf, 1, argc, ft->param, argv, argptr))
      goto failure;
  }

  ffi_call(cif, (void (*)()) ptr->ptr, (void *)retbuf, argptr);

  delete arg_types;
  arg_types=0;

  *rval=JSVAL_VOID;

  if(ft->returnType->type != VOIDTYPE) {
    JSX_Get(cx, retbuf, 0, 0, ft->returnType, rval);
  }

  if (arg_size) {
    if(!JSX_Get_multi(cx, 1, argc, ft->param, argv, 0, argptr))
      goto failure;
  }
  JS_free(cx, argptr);

  return JS_TRUE;

 failure:
  if (argptr) 
    JS_free(cx, argptr);
  if(arg_types) delete arg_types;

  return JS_FALSE;
}


static JSBool JSX_Pointer_getdollar(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
  *vp=JSVAL_VOID;
  JSX_Pointer *ptr = (JSX_Pointer *) JS_GetPrivate(cx, obj);
  int ret = JSX_Get(cx, (char*) ptr->ptr, 0, 0, ptr->type, vp);
  if(!ret) return JS_FALSE;
  if(ret == -1) {
    // Created new function
    JS_DefineProperty(cx, JSVAL_TO_OBJECT(*vp), "__ptr__", OBJECT_TO_JSVAL(obj), 0, 0, JSPROP_READONLY | JSPROP_PERMANENT);
  }
  return JS_TRUE;
}


static JSBool JSX_Pointer_setdollar(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
  JSX_Pointer *ptr = (JSX_Pointer *) JS_GetPrivate(cx, obj);
  if(!JSX_Set(cx, (char*) ptr->ptr, 0, ptr->type, *vp)) return JS_FALSE;
  return JS_TRUE;
}


static JSBool JSX_Pointer_getfinalize(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
  *vp=JSVAL_VOID;
  return JS_TRUE;
}


static JSBool JSX_Pointer_setfinalize(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
  JSX_Pointer *ptr = (JSX_Pointer *) JS_GetPrivate(cx, obj);
  if (*vp==JSVAL_NULL || *vp==JSVAL_VOID) {
    ptr->finalize=0;
    return JS_TRUE;
  }

  jsval ptrv;
  if(!JSVAL_IS_OBJECT(*vp) || !JS_ObjectIsFunction(cx, JSVAL_TO_OBJECT(*vp)) || !JS_LookupProperty(cx, JSVAL_TO_OBJECT(*vp), "__ptr__", &ptrv) || !JS_InstanceOf(cx, JSVAL_TO_OBJECT(ptrv), JSX_GetPointerClass(), NULL)) {
    return JSX_ReportException(cx, "Wrong value type for finalize property");
  }

  JSX_Pointer *finptr = (JSX_Pointer *) JS_GetPrivate(cx, JSVAL_TO_OBJECT(ptrv));
  JSX_TypeFunction *ft = (JSX_TypeFunction *) finptr->type;

  if(ft->type != FUNCTIONTYPE || ft->nParam != 1 || ft->param[0].paramtype->type != POINTERTYPE || ft->param[0].isConst) {
    return JSX_ReportException(cx, "Wrong function type for finalize property");
  }

  ptr->finalize = (void (*)(void *)) finptr->ptr;

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

  JSX_Pointer *ptr = (JSX_Pointer *) JS_GetPrivate(cx, obj);

  if(!ptr->type->type == POINTERTYPE) return JS_FALSE; // should be impossible

  if(ptr->type->type != STRUCTTYPE && ptr->type->type != UNIONTYPE)
    return JSX_ReportException(cx, "Pointer.prototype.field(): must only be called on pointers to struct or union types");

  JSX_TypeStructUnion *sutype = (JSX_TypeStructUnion *) ptr->type;

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

  JSObject *newobj = JS_NewObject(cx, &JSX_PointerClass, 0, 0);
  *rval = OBJECT_TO_JSVAL(newobj);

  JSX_Pointer *newptr;
  newptr = (JSX_Pointer *) malloc(sizeof(JSX_Pointer));
  newptr->type = sutype->member[ix].membertype;
  newptr->ptr = (char*) ptr->ptr + sutype->member[ix].offset / 8;
  newptr->finalize = 0;
  JS_SetPrivate(cx, newobj, newptr);

  return JS_TRUE;
}


static JSBool JSX_Pointer_getProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
  if (!JSVAL_IS_INT(id))
    return JS_TRUE; // Only handle numerical properties

  JSX_Pointer *ptr = (JSX_Pointer *) JS_GetPrivate(cx, obj);

  int ret = JSX_Get(cx, (char *) ptr->ptr + ptr->type->SizeInBytes() * JSVAL_TO_INT(id), 0, 0, ptr->type, vp);
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
  int ret = JSX_Set(cx, (char *) ptr->ptr + ptr->type->SizeInBytes() * JSVAL_TO_INT(id), 0, ptr->type, *vp);
  return ret == 0 ? JS_FALSE : JS_TRUE;
}


/*
  Indirect entry point for callback from C program
 */

static void JSX_Pointer_Callback(ffi_cif *cif, void *ret, void **args, void *user_data) {
  JSX_Callback *cb = (JSX_Callback *) user_data;
  jsval rval=JSVAL_VOID;
  JSX_TypeFunction *type = (JSX_TypeFunction *) cb->type;

  jsval *tmp_argv = new jsval[type->nParam];
  if(!tmp_argv) return;
  
  for(int i = 0; i != type->nParam; ++i) {
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
  for(int i = 0; i != type->nParam; ++i) {
    JS_RemoveRoot(cb->cx, tmp_argv+i);
  }
  JS_free(cb->cx, tmp_argv);

  if(type->returnType->type != VOIDTYPE) JSX_Set(cb->cx, (char*) ret, 0, type->returnType, rval);
}


JSBool JSX_NativeFunction(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
  JSObject *funcobj = JSVAL_TO_OBJECT(argv[-2]);
  jsval ptr;
  JS_LookupProperty(cx, funcobj, "__ptr__", &ptr);
  return JSX_Pointer_call(cx, JSVAL_TO_OBJECT(ptr), argc, argv, rval);
}


extern "C" jsval JSX_make_Pointer(JSContext *cx, JSObject *obj) {
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

