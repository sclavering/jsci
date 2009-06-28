#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <dlfcn.h>
#include <alloca.h>
#include "jsci.h"



static JSBool JSX_Pointer_new(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
static void JSX_Pointer_finalize(JSContext *cx, JSObject *obj);
static JSBool JSX_Pointer_getProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp);
static JSBool JSX_Pointer_setProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp);
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


JSClass * JSX_GetPointerClass(void) {
  return &JSX_PointerClass;
}


static void WrapPointer(JSContext *cx, JsciPointer *p, jsval *rval) {
  *rval = OBJECT_TO_JSVAL(JS_NewObject(cx, &JSX_PointerClass, 0, 0));
  JS_SetPrivate(cx, JSVAL_TO_OBJECT(*rval), p);
}


static JSBool JSX_InitPointerAlloc(JSContext *cx, JSObject *retobj, JSObject *type) {
  if (!JS_InstanceOf(cx, type, JSX_GetTypeClass(), NULL)) {
    JSX_ReportException(cx, "Wrong type argument");
    return JS_FALSE;
  }

  JsciType *t = (JsciType *) JS_GetPrivate(cx, type);
  JsciPointer *retpriv = new JsciPointerAlloc(t->SizeInBytes());

  if (!retpriv)
    return JS_FALSE;

  JS_SetPrivate(cx, retobj, retpriv);

  retpriv->type = (JsciType *) JS_GetPrivate(cx, type);

  return JS_TRUE;
}


JSBool JSX_InitPointerCallback(JSContext *cx, JSObject *retobj, jsval fun, JsciType *type) {
  if(type->type != FUNCTIONTYPE) return JSX_ReportException(cx, "Type is not a C function");
  if(!JS_DefineProperty(cx, retobj, "function", fun, 0, 0, JSPROP_READONLY | JSPROP_PERMANENT)) return JS_FALSE;
  JsciCallback *retpriv = new JsciCallback(cx, fun, type);
  if(!retpriv || !retpriv->Init()) return JS_FALSE;
  JS_SetPrivate(cx, retobj, retpriv);
  return JS_TRUE;
}


JSBool JSX_InitPointer(JSContext *cx, JSObject *retobj, JSObject *typeobj) {
  // xxx a hack to save typeobj from garbage collection, and thus stop the free()ing of the JsciType struct we share 
  if(!JS_DefineProperty(cx, retobj, "xxx", OBJECT_TO_JSVAL(typeobj), 0, 0, JSPROP_READONLY | JSPROP_PERMANENT))
    return JS_FALSE;

  JsciPointer *ret = new JsciPointer();
  if (!ret)
    return JS_FALSE;
  ret->type = (JsciType *) JS_GetPrivate(cx, typeobj);
  JS_SetPrivate(cx, retobj, ret);

  return JS_TRUE;
}


static JSBool Pointer_malloc(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
  if(argc < 1 || !JSVAL_IS_INT(argv[0]) || JSVAL_TO_INT(argv[0]) <= 0) return JSX_ReportException(cx, "Pointer.malloc(): argument must be a positive integer number of bytes to allocate");

  int length = INT_TO_JSVAL(argv[0]);
  JsciPointer *p = new JsciPointerAlloc(length);
  if(!p) return JS_FALSE;
  p->type = GetVoidType();
  WrapPointer(cx, p, rval);
  return JS_TRUE;
}


static JSBool Pointer_proto_cast(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
  if(!JSVAL_IS_OBJECT(argv[0]) || JSVAL_IS_NULL(argv[0]) || !JS_InstanceOf(cx, JSVAL_TO_OBJECT(argv[0]), JSX_GetTypeClass(), NULL)) {
    JSX_ReportException(cx, "Pointer.prototype.cast(): argument must be a Type instance");
    return JS_FALSE;
  }

  JsciPointer *ptr = (JsciPointer *) JS_GetPrivate(cx, obj);
  JSObject *newobj = JS_NewObject(cx, &JSX_PointerClass, 0, 0);
  *rval=OBJECT_TO_JSVAL(newobj);
  if (!JSX_InitPointer(cx, newobj, JSVAL_TO_OBJECT(argv[0]))) {
    return JS_FALSE;
  }

  JsciPointer *newptr = (JsciPointer *) JS_GetPrivate(cx, newobj);
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
    JsciPointer *ptr = (JsciPointer *) JS_GetPrivate(cx, typeObject);
    JsciType *type = ptr->type;
    // Accept both function type and pointer-to-function type
    if(type->type == POINTERTYPE) type = ((JsciTypePointer *) type)->direct;
    if(JSX_InitPointerCallback(cx, obj, argv[1], type)) return JS_FALSE;
    return JS_TRUE;
  }

  // Allocate memory and create Pointer instance
  if(!JSX_InitPointerAlloc(cx, obj, JSVAL_TO_OBJECT(argv[0]))) return JS_FALSE;
  // Set initial value, if provided
  if(argc >= 2 && argv[1] != JSVAL_VOID) {
    JsciPointer *ptr = (JsciPointer *) JS_GetPrivate(cx,obj);
    if(!ptr->type->JStoC(cx, (char*) ptr->ptr, argv[1])) return JS_FALSE;
  }

  return JS_TRUE;
}


static void JSX_Pointer_finalize(JSContext *cx, JSObject *obj) {
  JsciPointer *ptr = (JsciPointer *) JS_GetPrivate(cx, obj);
  delete ptr;
}


static JSBool JSX_Pointer_getdollar(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
  *vp=JSVAL_VOID;
  JsciPointer *ptr = (JsciPointer *) JS_GetPrivate(cx, obj);
  int ret = ptr->type->CtoJS(cx, (char*) ptr->ptr, vp);
  if(!ret) return JS_FALSE;
  if(ret == -1) {
    // Created new function
    JS_DefineProperty(cx, JSVAL_TO_OBJECT(*vp), "__ptr__", OBJECT_TO_JSVAL(obj), 0, 0, JSPROP_READONLY | JSPROP_PERMANENT);
  }
  return JS_TRUE;
}


static JSBool JSX_Pointer_setdollar(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
  JsciPointer *ptr = (JsciPointer *) JS_GetPrivate(cx, obj);
  if(!ptr->type->JStoC(cx, (char*) ptr->ptr, *vp)) return JS_FALSE;
  return JS_TRUE;
}


static JSBool Pointer_proto_setFinalizer(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
  JsciPointer *ptr = (JsciPointer *) JS_GetPrivate(cx, obj);
  if(argv[0] == JSVAL_NULL || argv[0] == JSVAL_VOID) {
    ptr->finalize=0;
    return JS_TRUE;
  }

  jsval ptrv;
  if(!JSVAL_IS_OBJECT(argv[0]) || !JS_ObjectIsFunction(cx, JSVAL_TO_OBJECT(argv[0])) || !JS_LookupProperty(cx, JSVAL_TO_OBJECT(argv[0]), "__ptr__", &ptrv) || !JS_InstanceOf(cx, JSVAL_TO_OBJECT(ptrv), JSX_GetPointerClass(), NULL)) {
    return JSX_ReportException(cx, "Pointer.prototype.setFinalizer(): argument must be function");
  }

  JsciPointer *finptr = (JsciPointer *) JS_GetPrivate(cx, JSVAL_TO_OBJECT(ptrv));
  JsciTypeFunction *ft = (JsciTypeFunction *) finptr->type;

  if(ft->type != FUNCTIONTYPE || ft->nParam != 1 || ft->param[0]->type != POINTERTYPE) {
    return JSX_ReportException(cx, "Wrong function type for finalize property");
  }

  ptr->finalize = (void (*)(void *)) finptr->ptr;

  return JS_TRUE;
}


static JSBool Pointer_proto_string(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
  JsciPointer *ptr = (JsciPointer *) JS_GetPrivate(cx, obj);
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
  JsciPointer *ptr = (JsciPointer *) JS_GetPrivate(cx, obj);
  jsdouble val = (jsdouble) (long) ptr->ptr;
  JS_NewNumberValue(cx, val, rval);
  return JS_TRUE;
}


// Read a field from a struct/union that this pointer points to (without converting the entire struct into a javascript ibject)
static JSBool Pointer_proto_field(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
  if(argc != 1 || !JSVAL_IS_STRING(argv[0]))
    return JSX_ReportException(cx, "Pointer.prototype.field(): must be passed a single argument, of type string");

  JsciPointer *ptr = (JsciPointer *) JS_GetPrivate(cx, obj);

  if(!ptr->type->type == POINTERTYPE) return JS_FALSE; // should be impossible

  if(ptr->type->type != SUTYPE) return JSX_ReportException(cx, "Pointer.prototype.field(): must only be called on pointers to struct or union types");

  JsciTypeStructUnion *sutype = (JsciTypeStructUnion *) ptr->type;

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

  JsciPointer *newptr = new JsciPointer();
  newptr->type = sutype->member[ix].membertype;
  newptr->ptr = (char*) ptr->ptr + sutype->member[ix].offset / 8;
  WrapPointer(cx, newptr, rval);

  return JS_TRUE;
}


static JSBool JSX_Pointer_getProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
  if (!JSVAL_IS_INT(id))
    return JS_TRUE; // Only handle numerical properties

  JsciPointer *ptr = (JsciPointer *) JS_GetPrivate(cx, obj);

  int ret = ptr->type->CtoJS(cx, (char *) ptr->ptr + ptr->type->SizeInBytes() * JSVAL_TO_INT(id), vp);
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
  JsciPointer *ptr = (JsciPointer *) JS_GetPrivate(cx, obj);
  return ptr->type->JStoC(cx, (char *) ptr->ptr + ptr->type->SizeInBytes() * JSVAL_TO_INT(id), *vp);
}


extern "C" jsval JSX_make_Pointer(JSContext *cx, JSObject *obj) {
  static struct JSFunctionSpec staticfunc[]={
    {"malloc", Pointer_malloc, 1, 0, 0},
    {0,0,0,0,0}
  };

  static struct JSFunctionSpec memberfunc[]={
    {"cast", Pointer_proto_cast, 1, 0, 0},
    {"field", Pointer_proto_field, 1, 0, 0},
    {"string", Pointer_proto_string, 1, 0, 0},
    {"valueOf", Pointer_proto_valueOf, 0, 0, 0},
    {"setFinalizer", Pointer_proto_setFinalizer, 1, 0, 0},
    {0,0,0,0,0}
  };

  static struct JSPropertySpec memberprop[]={
    {"$",0, JSPROP_PERMANENT, JSX_Pointer_getdollar,JSX_Pointer_setdollar},
    {0,0,0,0,0}
  };

  JSObject *protoobj = JS_NewObject(cx, &JSX_PointerClass, 0, 0);
  JSObject *classobj = JS_InitClass(cx, obj, protoobj, &JSX_PointerClass, JSX_Pointer_new, 0, memberprop, memberfunc, 0, staticfunc);
  if(!classobj) return JSVAL_VOID;

  return OBJECT_TO_JSVAL(JS_GetConstructor(cx, classobj));
}

