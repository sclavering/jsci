#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <dlfcn.h>
#include <alloca.h>
#include "jsci.h"



static JSBool Pointer__new(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
static void Pointer__finalize(JSContext *cx, JSObject *obj);
static JSBool Pointer__getProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp);
static JSBool Pointer__setProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp);
static JSBool Pointer__call(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);


static JSClass PointerClass = {
    "Pointer",
    JSCLASS_HAS_PRIVATE | JSCLASS_HAS_RESERVED_SLOTS(1),
    JS_PropertyStub,
    JS_PropertyStub,
    Pointer__getProperty,
    Pointer__setProperty,
    JS_EnumerateStub,
    JS_ResolveStub,
    JS_ConvertStub,
    Pointer__finalize,
    NULL,
    NULL,
    Pointer__call,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
};


JSClass * JSX_GetPointerClass(void) {
  return &PointerClass;
}


static JSBool Pointer_malloc(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
  int sz = JSVAL_IS_INT(argv[0]) ? JSVAL_TO_INT(argv[0]) : 0;
  if(sz <= 0) return JSX_ReportException(cx, "Pointer.malloc(): argument must be a positive integer number of bytes to allocate");
  return WrapPointer(cx, new JsciPointerAlloc(gTypeVoid, sz), rval);
}


static JSBool Pointer_proto_cast(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
  JsciType *t = jsval_to_JsciType(cx, argv[0]);
  JsciPointer *orig = jsval_to_JsciPointer(cx, argv[-1]);
  if(!t) return JSX_ReportException(cx, "Pointer.prototype.cast(): argument must be a Type instance");
  return WrapPointer(cx, new JsciPointer(t, orig->ptr), rval);
}


static JSBool Pointer__new(JSContext *cx, JSObject *origobj, uintN argc, jsval *argv, jsval *rval) {
  // note: we ignore origobj for simplicity of implementation
  JsciType *t = jsval_to_JsciType(cx, argv[0]);
  if(!t) return JSX_ReportException(cx, "Pointer(): first argument must be a Type");

  JSBool isFunc = JSVAL_IS_OBJECT(argv[1]) && JS_ObjectIsFunction(cx, JSVAL_TO_OBJECT(argv[1]));
  // Accept both function type and pointer-to-function type
  if(isFunc && t->type == POINTERTYPE) t = ((JsciTypePointer*) t)->direct;

  JsciPointer *ptr = new JsciPointerAlloc(t, t->SizeInBytes());
  if(!WrapPointer(cx, ptr, rval)) return JS_FALSE;
  // We need to ensure the Type object doesn't get GC'd, because we're sharing its JsciType*
  if(!JS_SetReservedSlot(cx, JSVAL_TO_OBJECT(*rval), 0, argv[0])) return JS_FALSE;
  if(argv[1] != JSVAL_VOID) return ptr->type->JStoC(cx, (char*) ptr->ptr, argv[1]);
  return JS_TRUE;
}


static void Pointer__finalize(JSContext *cx, JSObject *obj) {
  JsciPointer *ptr = (JsciPointer *) JS_GetPrivate(cx, obj);
  delete ptr;
}


static JSBool Pointer__getDollar(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
  *vp=JSVAL_VOID;
  JsciPointer *ptr = (JsciPointer *) JS_GetPrivate(cx, obj);
  return ptr->type->CtoJS(cx, (char*) ptr->ptr, vp);
}


static JSBool Pointer__setDollar(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
  JsciPointer *ptr = (JsciPointer *) JS_GetPrivate(cx, obj);
  return ptr->type->JStoC(cx, (char*) ptr->ptr, *vp);
}


static JSBool Pointer_proto_setFinalizer(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
  JsciPointer *ptr = (JsciPointer *) JS_GetPrivate(cx, obj);
  if(argv[0] == JSVAL_NULL || argv[0] == JSVAL_VOID) {
    ptr->finalize=0;
    return JS_TRUE;
  }

  JSObject *arg = JSVAL_IS_OBJECT(argv[0]) ? JSVAL_TO_OBJECT(argv[0]) : 0;
  if(!arg || !JS_InstanceOf(cx, arg, JSX_GetPointerClass(), NULL)) return JSX_ReportException(cx, "Pointer.prototype.setFinalizer(): argument must be Pointer to a function");

  JsciPointer *finptr = (JsciPointer *) JS_GetPrivate(cx, arg);
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

  JsciPointer *newptr = new JsciPointer(sutype->member[ix].membertype, (char*) ptr->ptr + sutype->member[ix].offset / 8);
  return WrapPointer(cx, newptr, rval);
}


static JSBool Pointer__getProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
  if(!JSVAL_IS_INT(id)) return JS_TRUE; // Only handle numerical properties
  JsciPointer *ptr = (JsciPointer *) JS_GetPrivate(cx, obj);
  return ptr->type->CtoJS(cx, (char *) ptr->ptr + ptr->type->SizeInBytes() * JSVAL_TO_INT(id), vp);
}


static JSBool Pointer__setProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
  if(!JSVAL_IS_INT(id)) return JS_TRUE; // Only handle numerical properties
  JsciPointer *ptr = (JsciPointer *) JS_GetPrivate(cx, obj);
  return ptr->type->JStoC(cx, (char *) ptr->ptr + ptr->type->SizeInBytes() * JSVAL_TO_INT(id), *vp);
}


static JSBool Pointer__call(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
  // we want the JSObject for the Pointer instance being called, not the "this" js variable (which is probably null)
  obj = JSVAL_TO_OBJECT(JS_ARGV_CALLEE(argv));
  JsciPointer *ptr = (JsciPointer *) JS_GetPrivate(cx, obj);
  if(ptr->type->type != FUNCTIONTYPE) return JSX_ReportException(cx, "Cannot call a Pointer instance that isn't a function pointer %x", ptr->type->type);
  return ((JsciTypeFunction *) ptr->type)->Call(cx, ptr->ptr, argc, argv, rval);
}


jsval make_Pointer(JSContext *cx, JSObject *obj) {
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
    {"$", 0, JSPROP_PERMANENT, Pointer__getDollar, Pointer__setDollar},
    {0,0,0,0,0}
  };

  JSObject *protoobj = JS_NewObject(cx, &PointerClass, 0, 0);
  JSObject *classobj = JS_InitClass(cx, obj, protoobj, &PointerClass, Pointer__new, 2, memberprop, memberfunc, 0, staticfunc);
  if(!classobj) return JSVAL_VOID;

  return OBJECT_TO_JSVAL(JS_GetConstructor(cx, classobj));
}

