#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include "jsci.h"


static void Type__finalize(JSContext *cx, JSObject *obj);

JsciType *gTypeVoid = NULL;
JsciType *gTypeChar = NULL;
// __proto__ for results of Type.function(...) and similar
static JSObject *s_Type_array_proto = NULL;
static JSObject *s_Type_bitfield_proto = NULL;
static JSObject *s_Type_pointer_proto = NULL;
static JSObject *s_Type_function_proto = NULL;
static JSObject *s_Type_struct_proto = NULL;
static JSObject *s_Type_union_proto = NULL;


static JSClass JSX_TypeClass={
    "Type",
    // we store a private JsciType*, and use slots to save other js Type objects from GC when we're sharing their JsciType's
    JSCLASS_HAS_PRIVATE | JSCLASS_HAS_RESERVED_SLOTS(2),
    JS_PropertyStub,
    JS_PropertyStub,
    JS_PropertyStub,
    JS_PropertyStub,
    JS_EnumerateStub,
    JS_ResolveStub,
    JS_ConvertStub,
    Type__finalize
};


static JSBool WrapType(JSContext *cx, JsciType *t, JSObject* proto, jsval *rval) {
  *rval = OBJECT_TO_JSVAL(JS_NewObject(cx, &JSX_TypeClass, proto, 0));
  return JS_SetPrivate(cx, JSVAL_TO_OBJECT(*rval), t);
}


static JSBool Type_function(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
  JsciType *rt = jsval_to_JsciType(cx, argv[0]);
  if(!rt) return JSX_ReportException(cx, "Type.function(): the first argument must be a Type instance");
  if(!JSVAL_IS_OBJECT(argv[1]) || !JS_IsArrayObject(cx, JSVAL_TO_OBJECT(argv[1]))) return JSX_ReportException(cx, "Type.function(): the second arg must be an array");

  jsuint nParam;
  JSObject *paramobj = JSVAL_TO_OBJECT(argv[1]);
  if(!JS_GetArrayLength(cx, paramobj, &nParam)) return JS_FALSE;

  JsciTypeFunction *type = new JsciTypeFunction(rt, nParam);
  if(!WrapType(cx, type, s_Type_function_proto, rval)) return JS_FALSE;
  JSObject *retobj = JSVAL_TO_OBJECT(*rval);
  // ensure the Type objects aren't GC'd, since we're sharing their JsciType instances
  JS_SetReservedSlot(cx, retobj, 0, argv[0]);
  JS_SetReservedSlot(cx, retobj, 1, argv[1]);

  for(int i = 0; i < nParam; i++) {
    jsval tmp;
    JS_GetElement(cx, paramobj, i, &tmp);
    JsciType *pt = jsval_to_JsciType(cx, tmp);
    if(!pt) return JSX_ReportException(cx, "Type.function(): parameter %i is not a Type instance", i);
    type->param[i] = pt;
  }

  return JS_TRUE;
}


JSClass *JSX_GetTypeClass(void) {
  return &JSX_TypeClass;
}


static JSBool JSX_NewTypeStructUnion(JSContext *cx, int nMember, jsval *member, jsval *rval, JsciTypeStructUnion *type, JSObject* proto) {
  WrapType(cx, type, proto, rval);
  type->ffiType.elements = 0;
  return nMember ? type->ReplaceMembers(cx, JSVAL_TO_OBJECT(*rval), nMember, member) : JS_TRUE;
}


static JSBool Type_replace_members(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
  JsciType *t = jsval_to_JsciType(cx, argv[0]);
  if(!t) return JSX_ReportException(cx, "Type.replace_members(): the first argument must be a struct/union Type instance");
  JsciTypeStructUnion *tsu = dynamic_cast<JsciTypeStructUnion*>(t);
  if(!tsu) return JSX_ReportException(cx, "Type.replace_members(): the first argument must be a struct/union Type instance");
  if(tsu->nMember) return JSX_ReportException(cx, "Type.replace_members(): the struct/union already has members");
  return tsu->ReplaceMembers(cx, JSVAL_TO_OBJECT(argv[0]), argc - 1, &argv[1]);
}


static JSBool Type_pointer(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
  JsciType *t = jsval_to_JsciType(cx, argv[0]);
  if(!(t || argv[0] == JSVAL_VOID)) return JSX_ReportException(cx, "Type.pointer(): argument must be undefined, or a Type instance");
  return WrapType(cx, new JsciTypePointer(t ? t : gTypeVoid), s_Type_pointer_proto, rval)
      && JS_SetReservedSlot(cx, JSVAL_TO_OBJECT(*rval), 0, argv[0]);
}


static JSBool Type_array(JSContext *cx,  JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
  JsciType *t = jsval_to_JsciType(cx, argv[0]);
  if(!t) return JSX_ReportException(cx, "Type.array(): first argument must be a Type instance");
  int len = JSVAL_IS_INT(argv[1]) ? JSVAL_TO_INT(argv[1]) : -1;
  if(len < 0) return JSX_ReportException(cx, "Type.array(): second argument must be a non-negative integer");
  return WrapType(cx, new JsciTypeArray(t, len), s_Type_array_proto, rval);
}


static JSBool Type_bitfield(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
  JsciType *t = jsval_to_JsciType(cx, argv[0]);
  if(!t) return JSX_ReportException(cx, "Type.bitfield(): first argument must be a Type instance");
  int len = JSVAL_IS_INT(argv[1]) ? JSVAL_TO_INT(argv[1]) : -1;
  if(len < 0) return JSX_ReportException(cx, "Type.bitfield(): second argument must be a non-negative integer");
  return WrapType(cx, new JsciTypeBitfield(t, len), s_Type_bitfield_proto, rval);
}


static void init_types(JSContext *cx, JSObject *typeobj) {
  jsval tmp = JSVAL_VOID;
  JS_AddRoot(cx, &tmp);

  WrapType(cx, new JsciTypeUint(0, ffi_type_uchar), 0, &tmp);
  JS_SetProperty(cx, typeobj, "unsigned_char", &tmp);
  WrapType(cx, new JsciTypeUint(1, ffi_type_ushort), 0, &tmp);
  JS_SetProperty(cx, typeobj, "unsigned_short", &tmp);
  WrapType(cx, new JsciTypeUint(2, ffi_type_uint), 0, &tmp);
  JS_SetProperty(cx, typeobj, "unsigned_int", &tmp);
  WrapType(cx, new JsciTypeUint(3, ffi_type_ulong), 0, &tmp);
  JS_SetProperty(cx, typeobj, "unsigned_long", &tmp);
  WrapType(cx, new JsciTypeUint(4, ffi_type_uint64), 0, &tmp);
  JS_SetProperty(cx, typeobj, "unsigned_long_long", &tmp);
  gTypeChar = new JsciTypeInt(0, ffi_type_schar);
  WrapType(cx, gTypeChar, 0, &tmp);
  JS_SetProperty(cx, typeobj, "signed_char", &tmp);
  WrapType(cx, new JsciTypeInt(1, ffi_type_sshort), 0, &tmp);
  JS_SetProperty(cx, typeobj, "signed_short", &tmp);
  WrapType(cx, new JsciTypeInt(2, ffi_type_sint), 0, &tmp);
  JS_SetProperty(cx, typeobj, "signed_int", &tmp);
  WrapType(cx, new JsciTypeInt(3, ffi_type_slong), 0, &tmp);
  JS_SetProperty(cx, typeobj, "signed_long", &tmp);
  WrapType(cx, new JsciTypeInt(4, ffi_type_sint64), 0, &tmp);
  JS_SetProperty(cx, typeobj, "signed_long_long", &tmp);
  // xxx currently we let 0-ffi.js alias Type.int etc to Type.signed_int, which isn't portable.
  // limits.h has constants we could use to detect this.  char is particularly odd, since for C type checking it'd distinct from both "signed char" and "unsigned char", though always has the same representation as one or other of them.

  WrapType(cx, new JsciTypeFloat(), 0, &tmp);
  JS_SetProperty(cx, typeobj, "float", &tmp);
  WrapType(cx, new JsciTypeDouble(), 0, &tmp);
  JS_SetProperty(cx, typeobj, "double", &tmp);

  gTypeVoid = new JsciTypeVoid();
  WrapType(cx, gTypeVoid, 0, &tmp);
  JS_SetProperty(cx, typeobj, "void", &tmp);

  JS_RemoveRoot(cx, &tmp);
}


static JSBool Type__new(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
  return JSX_ReportException(cx, "Type is not a constructor");
}


static void Type__finalize(JSContext *cx,  JSObject *obj) {
  JsciType *type = (JsciType *) JS_GetPrivate(cx, obj);
  if(type) delete type;
}


static JSBool Type_struct(JSContext *cx,  JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
  return JSX_NewTypeStructUnion(cx, argc, argv, rval, new JsciTypeStruct, s_Type_struct_proto);
}


static JSBool Type_union(JSContext *cx,  JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
  return JSX_NewTypeStructUnion(cx, argc, argv, rval, new JsciTypeUnion, s_Type_union_proto);
}


static JSBool Type_sizeof(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
  JsciType *t = jsval_to_JsciType(cx, argv[0]);
  if(!t) return JSX_ReportException(cx, "Type.sizeof(): the argument must be a Type instance");
  int size = t->SizeInBytes();
  *rval = size ? INT_TO_JSVAL(size) : JSVAL_VOID;
  return JS_TRUE;
}


jsval make_Type(JSContext *cx, JSObject *obj) {
  JSObject *typeobj;
  JSObject *typeproto;

  static struct JSFunctionSpec staticfunc[]={
    {"array", Type_array, 2, 0, 0},
    {"bitfield", Type_bitfield, 2, 0, 0},
    {"function", Type_function, 2, 0, 0},
    {"pointer", Type_pointer, 1, 0, 0},
    {"struct", Type_struct, 1, 0, 0},
    {"union", Type_union, 1, 0, 0},
    {"sizeof", Type_sizeof, 1, 0, 0},
    {"replace_members", Type_replace_members, 1, 0, 0},
    {0,0,0,0,0}
  };

  uintN flags = JSPROP_READONLY | JSPROP_PERMANENT;
  if(!(1
      // init Type itself
      && (typeproto = JS_NewObject(cx, 0, 0, 0))
      && (typeobj = JS_InitClass(cx, obj, typeproto, &JSX_TypeClass, Type__new, 0, 0, 0, 0, staticfunc))
      && (typeobj = JS_GetConstructor(cx, typeobj))
      // expose __proto__ objects for Type.function(...) and similar, so 0-ffi can extend them
      && (s_Type_array_proto = JS_NewObject(cx, 0, typeproto, 0))
      && JS_DefineProperty(cx, typeobj, "array_prototype", OBJECT_TO_JSVAL(s_Type_array_proto), 0, 0, flags)
      && (s_Type_bitfield_proto = JS_NewObject(cx, 0, typeproto, 0))
      && JS_DefineProperty(cx, typeobj, "bitfield_prototype", OBJECT_TO_JSVAL(s_Type_bitfield_proto), 0, 0, flags)
      && (s_Type_function_proto = JS_NewObject(cx, 0, typeproto, 0))
      && JS_DefineProperty(cx, typeobj, "function_prototype", OBJECT_TO_JSVAL(s_Type_function_proto), 0, 0, flags)
      && (s_Type_pointer_proto = JS_NewObject(cx, 0, typeproto, 0))
      && JS_DefineProperty(cx, typeobj, "pointer_prototype", OBJECT_TO_JSVAL(s_Type_pointer_proto), 0, 0, flags)
      && (s_Type_struct_proto = JS_NewObject(cx, 0, typeproto, 0))
      && JS_DefineProperty(cx, typeobj, "struct_prototype", OBJECT_TO_JSVAL(s_Type_struct_proto), 0, 0, flags)
      && (s_Type_union_proto = JS_NewObject(cx, 0, typeproto, 0))
      && JS_DefineProperty(cx, typeobj, "union_prototype", OBJECT_TO_JSVAL(s_Type_union_proto), 0, 0, flags)
      )) {
    return JSVAL_VOID;
  }

  init_types(cx, typeobj);

  return OBJECT_TO_JSVAL(typeobj);
}
