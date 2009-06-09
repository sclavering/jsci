#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include "jsci.h"


static void JSX_Type_finalize(JSContext *cx, JSObject *obj);

static JsciType *sTypeVoid = NULL;
// __proto__ for results of Type.function(...) and similar
static JSObject *s_Type_array_proto = NULL;
static JSObject *s_Type_bitfield_proto = NULL;
static JSObject *s_Type_pointer_proto = NULL;
static JSObject *s_Type_function_proto = NULL;
static JSObject *s_Type_struct_proto = NULL;
static JSObject *s_Type_union_proto = NULL;


static JSClass JSX_TypeClass={
    "Type",
    JSCLASS_HAS_PRIVATE,
    JS_PropertyStub,
    JS_PropertyStub,
    JS_PropertyStub,
    JS_PropertyStub,
    JS_EnumerateStub,
    JS_ResolveStub,
    JS_ConvertStub,
    JSX_Type_finalize
};


static void WrapType(JSContext *cx, JsciType *t, JSObject* proto, jsval *rval) {
  *rval = OBJECT_TO_JSVAL(JS_NewObject(cx, &JSX_TypeClass, proto, 0));
  JS_SetPrivate(cx, JSVAL_TO_OBJECT(*rval), t);
}


JSBool JSX_InitMemberType(JSContext *cx, JSX_SuMember *dest, JSObject *membertype) {
  jsval tmp;

  JS_GetProperty(cx, membertype, "name", &tmp);
  if(tmp == JSVAL_VOID || !JSVAL_IS_STRING(tmp)) return JSX_ReportException(cx, "Wrong or missing 'name' property in member type object");
  dest->name = strdup(JS_GetStringBytes(JSVAL_TO_STRING(tmp)));

  JS_GetProperty(cx, membertype, "type", &tmp);
  if(!jsval_is_Type(cx, tmp)) return JSX_ReportException(cx, "Wrong or missing 'type' property in member type object");
  dest->membertype = (JsciType *) JS_GetPrivate(cx, JSVAL_TO_OBJECT(tmp));

  return JS_TRUE;
}


static JSBool Type_function(JSContext *cx,  JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
  jsval returnType = argv[0];
  jsval params = argv[1];

  if(!jsval_is_Type(cx, returnType)) return JSX_ReportException(cx, "Type.function: the returnType arg must be a Type instance");
  if(!JSVAL_IS_OBJECT(params) || params == JSVAL_NULL || !JS_IsArrayObject(cx, JSVAL_TO_OBJECT(params))) return JSX_ReportException(cx, "Type.function: the params arg must be an array");

  jsuint nParam;
  JSObject *paramobj = JSVAL_TO_OBJECT(params);
  if(!JS_GetArrayLength(cx, paramobj, &nParam)) return JS_FALSE;

  JsciTypeFunction *type = new JsciTypeFunction(nParam);
  WrapType(cx, type, s_Type_function_proto, rval);
  JSObject *retobj = JSVAL_TO_OBJECT(*rval);

  JS_DefineProperty(cx, retobj, "returnType", returnType, 0, 0, JSPROP_ENUMERATE | JSPROP_PERMANENT);
  type->returnType = (JsciType *) JS_GetPrivate(cx, JSVAL_TO_OBJECT(returnType));
  type->cif.arg_types=0;

  for(int i = 0; i < nParam; i++) {
    jsval tmp;
    JS_GetElement(cx, paramobj, i, &tmp);
    if(!jsval_is_Type(cx, tmp)) return JSX_ReportException(cx, "Type.function(): parameter %i is not a Type instance", i);
    type->param[i] = (JsciType *) JS_GetPrivate(cx, JSVAL_TO_OBJECT(tmp));
    JS_DefineElement(cx, retobj, i, tmp, 0, 0, JSPROP_ENUMERATE | JSPROP_PERMANENT);
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
  jsval suv = argv[0];
  if(!jsval_is_Type(cx, suv)) return JSX_ReportException(cx, "Type.replace_members(): the first argument must be a struct/union Type instance");
  JsciType *t = (JsciType *) JS_GetPrivate(cx, JSVAL_TO_OBJECT(suv));
  if(t->type != SUTYPE) return JSX_ReportException(cx, "Type.replace_members(): the first argument must be a struct/union Type instance");
  JsciTypeStructUnion *tsu = (JsciTypeStructUnion *) t;
  if(tsu->nMember) return JSX_ReportException(cx, "Type.replace_members(): the struct/union already has members");

  return tsu->ReplaceMembers(cx, JSVAL_TO_OBJECT(argv[0]), argc - 1, &argv[1]);
}


static JSBool Type_pointer(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
  jsval direct = argv[0];

  if(direct != JSVAL_VOID && !jsval_is_Type(cx, direct)) return JSX_ReportException(cx, "Type.pointer(): argument must be undefined, or a Type instance");

  JsciTypePointer *type = new JsciTypePointer;
  WrapType(cx, type, s_Type_pointer_proto, rval);
  type->direct = sTypeVoid;
  JS_DefineElement(cx, JSVAL_TO_OBJECT(*rval), 0, direct, 0, 0, JSPROP_ENUMERATE | JSPROP_PERMANENT);
  if(direct != JSVAL_VOID) type->direct = (JsciType *) JS_GetPrivate(cx, JSVAL_TO_OBJECT(direct));

  return JS_TRUE;
}


static JSBool Type_array(JSContext *cx,  JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
  jsval member = argv[0];
  jsval len = argv[1];

  if(!jsval_is_Type(cx, member)) return JSX_ReportException(cx, "Type.array(): first argument must be a Type instance");
  if(!JSVAL_IS_INT(len)) return JSX_ReportException(cx, "Type.array(): second argument must be an integer");

  JsciTypeArray *type = new JsciTypeArray;
  type->length = JSVAL_TO_INT(len);
  type->member = (JsciType *) JS_GetPrivate(cx, JSVAL_TO_OBJECT(member));
  WrapType(cx, type, s_Type_array_proto, rval);
  return JS_TRUE;
}


static JSBool Type_bitfield(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
  jsval member = argv[0];
  jsval len = argv[1];

  if(!jsval_is_Type(cx, member)) return JSX_ReportException(cx, "Type.bitfield(): first argument must be a Type instance");
  if(!JSVAL_IS_INT(len)) return JSX_ReportException(cx, "Type.bitfield(): second argument must be an integer");

  JsciTypeBitfield *type = new JsciTypeBitfield;
  WrapType(cx, type, s_Type_bitfield_proto, rval);
  type->length = JSVAL_TO_INT(len);
  type->member = (JsciType *) JS_GetPrivate(cx, JSVAL_TO_OBJECT(member));
  return JS_TRUE;
}


JsciType *GetVoidType(void) {
  return sTypeVoid;
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
  WrapType(cx, new JsciTypeInt(0, ffi_type_schar), 0, &tmp);
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

  WrapType(cx, new JsciTypeFloat(0, ffi_type_float), 0, &tmp);
  JS_SetProperty(cx, typeobj, "float", &tmp);
  WrapType(cx, new JsciTypeFloat(1, ffi_type_double), 0, &tmp);
  JS_SetProperty(cx, typeobj, "double", &tmp);
  WrapType(cx, new JsciTypeFloat(2, ffi_type_longdouble), 0, &tmp);
  JS_SetProperty(cx, typeobj, "long_double", &tmp);

  sTypeVoid = new JsciTypeVoid;
  WrapType(cx, sTypeVoid, 0, &tmp);
  JS_SetProperty(cx, typeobj, "void", &tmp);

  JS_RemoveRoot(cx, &tmp);
}


static JSBool JSX_Type_new(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
  // Someone is calling the constructor against all common sense
  return JSX_ReportException(cx, "Type is not a constructor");
}


static void JSX_Type_finalize(JSContext *cx,  JSObject *obj) {
  JsciType *type = (JsciType *) JS_GetPrivate(cx, obj);
  if(type) delete type;
}


static JSBool JSX_Type_struct(JSContext *cx,  JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
  return JSX_NewTypeStructUnion(cx, argc, argv, rval, new JsciTypeStruct, s_Type_struct_proto);
}


static JSBool JSX_Type_union(JSContext *cx,  JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
  return JSX_NewTypeStructUnion(cx, argc, argv, rval, new JsciTypeUnion, s_Type_union_proto);
}


static JSBool Type_sizeof(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
  jsval arg = argv[0];
  *rval = JSVAL_VOID;
  if(!jsval_is_Type(cx, arg)) return JSX_ReportException(cx, "Type.sizeof(): the argument must be a Type instance");
  JsciType *t = (JsciType *) JS_GetPrivate(cx, JSVAL_TO_OBJECT(arg));
  int size = t->SizeInBytes();
  if(size) *rval = INT_TO_JSVAL(size);
  return JS_TRUE;
}


// Determine the JS type to an appropriate level of detail for the big 2D switch
int JSX_JSType(JSContext *cx, jsval v) {
  int jstype = JSVAL_TAG(v);
  switch(jstype) {
    case JSVAL_OBJECT:
      if(v == JSVAL_NULL) return JSNULL;
      if(JS_IsArrayObject(cx, JSVAL_TO_OBJECT(v))) return JSARRAY;
      if(JS_InstanceOf(cx, JSVAL_TO_OBJECT(v), JSX_GetPointerClass(), NULL)) return JSPOINTER;
      if(JS_ObjectIsFunction(cx, JSVAL_TO_OBJECT(v))) return JSFUNC;
      break;
    case 1:
    case 3:
    case 5:
    case 7:
      if(v == JSVAL_VOID) return JSVOID;
      return JSVAL_INT;
  }
  return jstype;
}


extern "C" jsval JSX_make_Type(JSContext *cx, JSObject *obj) {
  JSObject *typeobj;
  JSObject *typeproto;

  static struct JSFunctionSpec staticfunc[]={
    {"array", Type_array, 2, 0, 0},
    {"bitfield", Type_bitfield, 2, 0, 0},
    {"function", Type_function, 2, 0, 0},
    {"pointer", Type_pointer, 1, 0, 0},
    {"struct",JSX_Type_struct,1,0,0},
    {"union",JSX_Type_union,1,0,0},
    {"sizeof", Type_sizeof, 1, 0, 0},
    {"replace_members", Type_replace_members, 1, 0, 0},
    {0,0,0,0,0}
  };

  uintN flags = JSPROP_READONLY | JSPROP_PERMANENT;
  if(!(1
      // init Type itself
      && (typeproto = JS_NewObject(cx, 0, 0, 0))
      && (typeobj = JS_InitClass(cx, obj, typeproto, &JSX_TypeClass, JSX_Type_new, 0, 0, 0, 0, staticfunc))
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
