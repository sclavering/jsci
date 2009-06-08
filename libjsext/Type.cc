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


static inline JSBool jsval_is_Type(JSContext *cx, jsval v) {
  return JSVAL_IS_OBJECT(v) && !JSVAL_IS_NULL(v) && JS_InstanceOf(cx, JSVAL_TO_OBJECT(v), &JSX_TypeClass, NULL);
}


JSBool JSX_InitMemberType(JSContext *cx, JSX_SuMember *dest, JSObject *membertype) {
  jsval tmp;

  JS_GetProperty(cx, membertype, "name", &tmp);
  if(tmp == JSVAL_VOID || !JSVAL_IS_STRING(tmp)) {
    JSX_ReportException(cx, "Wrong or missing 'name' property in member type object");
    return JS_FALSE;
  }
  dest->name = strdup(JS_GetStringBytes(JSVAL_TO_STRING(tmp)));

  JS_GetProperty(cx, membertype, "type", &tmp);
  if(!jsval_is_Type(cx, tmp)) {
    JSX_ReportException(cx, "Wrong or missing 'type' property in member type object");
    // name is freed later
    return JS_FALSE;
  }
  dest->membertype = (JsciType *) JS_GetPrivate(cx, JSVAL_TO_OBJECT(tmp));

  return JS_TRUE;
}


static JSBool Type_function(JSContext *cx,  JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
  jsval returnType = argv[0];
  jsval params = argv[1];

  if(!jsval_is_Type(cx, returnType)) {
    JSX_ReportException(cx, "Type.function: the returnType arg must be a Type instance");
    return JS_FALSE;
  }

  if(!JSVAL_IS_OBJECT(params) || params == JSVAL_NULL || !JS_IsArrayObject(cx, JSVAL_TO_OBJECT(params))) {
    JSX_ReportException(cx, "Type.function: the params arg must be an array");
    return JS_FALSE;
  }

  JsciTypeFunction *type = new JsciTypeFunction;

  JSObject *retobj;
  retobj = JS_NewObject(cx, &JSX_TypeClass, s_Type_function_proto, 0);
  *rval=OBJECT_TO_JSVAL(retobj);

  JSObject *paramobj;
  jsuint nParam;
  paramobj = JSVAL_TO_OBJECT(params);
  if(!JS_GetArrayLength(cx, paramobj, &nParam)) return JS_FALSE;

  type->nParam=nParam;
  JS_SetPrivate(cx, retobj, type);

  JS_DefineProperty(cx, retobj, "returnType", returnType, 0, 0, JSPROP_ENUMERATE | JSPROP_PERMANENT);
  type->returnType = (JsciType *) JS_GetPrivate(cx, JSVAL_TO_OBJECT(returnType));

  type->param = new JsciType*[type->nParam + 1];

  type->cif.arg_types=0;

  for(int i = 0; i < nParam; i++) {
    jsval tmp;
    JS_GetElement(cx, paramobj, i, &tmp);
    if(!jsval_is_Type(cx, tmp)) return JSX_ReportException(cx, "Type.function(): parameter %i is not a Type instance", i);
    type->param[i] = (JsciType *) JS_GetPrivate(cx, JSVAL_TO_OBJECT(tmp));
    JS_DefineElement(cx, retobj, i, tmp, 0, 0, JSPROP_ENUMERATE | JSPROP_PERMANENT);
  }
  type->param[type->nParam] = sTypeVoid;

  return JS_TRUE;
}


JSClass *JSX_GetTypeClass(void) {
  return &JSX_TypeClass;
}


// typeid must obviously be STRUCTTYPE or UNIONTYPE
static JSBool JSX_NewTypeStructUnion(JSContext *cx, int nMember, jsval *member, jsval *rval, JsciTypeStructUnion *type, JSObject* proto) {
  JSObject *retobj = JS_NewObject(cx, &JSX_TypeClass, proto, 0);
  *rval = OBJECT_TO_JSVAL(retobj);
  type->ffiType.elements = 0;
  JS_SetPrivate(cx, retobj, type);
  JSBool rv = JS_TRUE;
  if(nMember) rv = type->ReplaceMembers(cx, retobj, nMember, member);
  return rv;
}


static JSBool Type_replace_members(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
  jsval suv = argv[0];
  if(!jsval_is_Type(cx, suv))
    return JSX_ReportException(cx, "Type.replace_members(): the first argument must be a struct/union Type instance");
  JsciType *t = (JsciType *) JS_GetPrivate(cx, JSVAL_TO_OBJECT(suv));
  if(t->type != STRUCTTYPE && t->type != UNIONTYPE)
    return JSX_ReportException(cx, "Type.replace_members(): the first argument must be a struct/union Type instance");
  JsciTypeStructUnion *tsu = (JsciTypeStructUnion *) t;
  if(tsu->nMember)
    return JSX_ReportException(cx, "Type.replace_members(): the struct/union already has members");

  return tsu->ReplaceMembers(cx, JSVAL_TO_OBJECT(argv[0]), argc - 1, &argv[1]);
}


static JSBool Type_pointer(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
  jsval direct = argv[0];

  if(direct != JSVAL_VOID && !jsval_is_Type(cx, direct)) {
    JSX_ReportException(cx, "Type.pointer(): argument must be undefined, or a Type instance");
    return JS_FALSE;
  }

  JSObject *retobj;
  retobj = JS_NewObject(cx, &JSX_TypeClass, s_Type_pointer_proto, 0);
  *rval=OBJECT_TO_JSVAL(retobj);

  JsciTypePointer *type = new JsciTypePointer;
  JS_SetPrivate(cx, retobj, type);
  type->direct = sTypeVoid;
  JS_DefineElement(cx, retobj, 0, direct, 0, 0, JSPROP_ENUMERATE | JSPROP_PERMANENT);
  if(direct != JSVAL_VOID) type->direct = (JsciType *) JS_GetPrivate(cx, JSVAL_TO_OBJECT(direct));

  return JS_TRUE;
}


static JSBool Type_array(JSContext *cx,  JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
  jsval member = argv[0];
  jsval len = argv[1];

  if(!jsval_is_Type(cx, member)) {
    JSX_ReportException(cx, "Type.array(): first argument must be a Type instance");
    return JS_FALSE;
  }
  if(!JSVAL_IS_INT(len)) {
    JSX_ReportException(cx, "Type.array(): second argument must be an integer");
    return JS_FALSE;
  }

  JSObject *retobj;
  retobj = JS_NewObject(cx, &JSX_TypeClass, s_Type_array_proto, 0);
  *rval=OBJECT_TO_JSVAL(retobj);
  JsciTypeArray *type = new JsciTypeArray;
  JS_SetPrivate(cx, retobj, type);
  type->length = JSVAL_TO_INT(len);
  type->member = (JsciType *) JS_GetPrivate(cx, JSVAL_TO_OBJECT(member));
  return JS_TRUE;
}


static JSBool Type_bitfield(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
  jsval member = argv[0];
  jsval len = argv[1];

  if(!jsval_is_Type(cx, member)) {
    JSX_ReportException(cx, "Type.bitfield(): first argument must be a Type instance");
    return JS_FALSE;
  }
  if(!JSVAL_IS_INT(len)) {
    JSX_ReportException(cx, "Type.bitfield(): second argument must be an integer");
    return JS_FALSE;
  }

  JSObject *retobj;
  retobj = JS_NewObject(cx, &JSX_TypeClass, s_Type_bitfield_proto, 0);
  *rval=OBJECT_TO_JSVAL(retobj);
  JsciTypeBitfield *type = new JsciTypeBitfield;
  JS_SetPrivate(cx, retobj, type);
  type->length = JSVAL_TO_INT(len);
  type->member = (JsciType *) JS_GetPrivate(cx, JSVAL_TO_OBJECT(member));
  return JS_TRUE;
}


JsciType *GetVoidType(void) {
  return sTypeVoid;
}


static jsval init_numeric_Type(JSContext *cx, JSX_TypeID type_id, int size, ffi_type ffit) {
  JSObject *newtype;
  newtype = JS_NewObject(cx, &JSX_TypeClass, 0, 0);
  jsval newval = OBJECT_TO_JSVAL(newtype);
  JsciTypeNumeric *type = new JsciTypeNumeric(type_id);
  type->size = size;
  type->ffiType = ffit;
  JS_SetPrivate(cx, newtype, type);
  return newval;
}


static void init_int_types(JSContext *cx, JSObject *typeobj) {
  jsval tmp;
  tmp = init_numeric_Type(cx, UINTTYPE, 0, ffi_type_uchar);
  JS_SetProperty(cx, typeobj, "unsigned_char", &tmp);
  tmp = init_numeric_Type(cx, UINTTYPE, 1, ffi_type_ushort);
  JS_SetProperty(cx, typeobj, "unsigned_short", &tmp);
  tmp = init_numeric_Type(cx, UINTTYPE, 2, ffi_type_uint);
  JS_SetProperty(cx, typeobj, "unsigned_int", &tmp);
  tmp = init_numeric_Type(cx, UINTTYPE, 3, ffi_type_ulong);
  JS_SetProperty(cx, typeobj, "unsigned_long", &tmp);
  tmp = init_numeric_Type(cx, UINTTYPE, 4, ffi_type_uint64);
  JS_SetProperty(cx, typeobj, "unsigned_long_long", &tmp);
  tmp = init_numeric_Type(cx, INTTYPE, 0, ffi_type_schar);
  JS_SetProperty(cx, typeobj, "signed_char", &tmp);
  tmp = init_numeric_Type(cx, INTTYPE, 1, ffi_type_sshort);
  JS_SetProperty(cx, typeobj, "signed_short", &tmp);
  tmp = init_numeric_Type(cx, INTTYPE, 2, ffi_type_sint);
  JS_SetProperty(cx, typeobj, "signed_int", &tmp);
  tmp = init_numeric_Type(cx, INTTYPE, 3, ffi_type_slong);
  JS_SetProperty(cx, typeobj, "signed_long", &tmp);
  tmp = init_numeric_Type(cx, INTTYPE, 4, ffi_type_sint64);
  JS_SetProperty(cx, typeobj, "signed_long_long", &tmp);

  // xxx currently we let 0-ffi.js alias Type.int etc to Type.signed_int, which isn't portable.
  // limits.h has constants we could use to detect this.  char is particularly odd, since for C type checking it'd distinct from both "signed char" and "unsigned char", though always has the same representation as one or other of them.
}


static void init_float_types(JSContext *cx, JSObject *typeobj) {
  jsval tmp;
  tmp = init_numeric_Type(cx, FLOATTYPE, 0, ffi_type_float);
  JS_SetProperty(cx, typeobj, "float", &tmp);
  tmp = init_numeric_Type(cx, FLOATTYPE, 1, ffi_type_double);
  JS_SetProperty(cx, typeobj, "double", &tmp);
  tmp = init_numeric_Type(cx, FLOATTYPE, 2, ffi_type_longdouble);
  JS_SetProperty(cx, typeobj, "long_double", &tmp);
}


static void init_other_types(JSContext *cx, JSObject *typeobj) {
  JSObject *newtype = JS_NewObject(cx, &JSX_TypeClass, 0, 0);
  jsval newval = OBJECT_TO_JSVAL(newtype);
  JS_SetProperty(cx, typeobj, "void", &newval);
  JsciType *type = new JsciTypeVoid;
  JS_SetPrivate(cx, newtype, type);
  sTypeVoid = type;
}


static JSBool JSX_Type_new(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
  // Someone is calling the constructor against all common sense
  JSX_ReportException(cx, "Type is not a constructor");
  return JS_FALSE;
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
  if(!jsval_is_Type(cx, arg)) {
    JSX_ReportException(cx, "Type.sizeof(): the argument must be a Type instance");
    return JS_FALSE;
  }
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

  init_int_types(cx, typeobj);
  init_float_types(cx, typeobj);
  init_other_types(cx, typeobj);

  return OBJECT_TO_JSVAL(typeobj);
}
