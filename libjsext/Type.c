#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include "util.h"
#include "jsci.h"


static void JSX_Type_finalize(JSContext *cx, JSObject *obj);
static JSBool JSX_Type_SetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *rval);
static JSBool TypeStructUnion_SetMember(JSContext *cx, JSX_TypeStructUnion *type, int memberno, jsval member);
static int JSX_TypeAlign(JSX_Type *type);
static JSBool FuncParam_Init(JSContext *cx, JSX_FuncParam *dest, JSObject *membertype);
static void TypeStructUnion_init_ffiType_elements(JSContext *cx, JSX_TypeStructUnion *typesu);

static JSX_Type *sTypeVoid = NULL;
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
    JSX_Type_SetProperty,
    JS_EnumerateStub,
    JS_ResolveStub,
    JS_ConvertStub,
    JSX_Type_finalize
};


static inline JSBool jsval_is_Type(JSContext *cx, jsval v) {
  return JSVAL_IS_OBJECT(v) && !JSVAL_IS_NULL(v) && JS_InstanceOf(cx, JSVAL_TO_OBJECT(v), &JSX_TypeClass, NULL);
}


ffi_cif *JSX_GetCIF(JSContext *cx, JSX_TypeFunction *type) {
  if (type->cif.arg_types)
    return &type->cif;

  type->cif.arg_types=JS_malloc(cx, sizeof(ffi_type)*(type->nParam));
  int i;
  for (i=0; i<type->nParam; i++) {
    if(type->param[i].paramtype->type == ARRAYTYPE)
      type->cif.arg_types[i]=&ffi_type_pointer;
    else
      type->cif.arg_types[i] = JSX_GetFFIType(cx, type->param[i].paramtype);
  }
  ffi_prep_cif(&type->cif, FFI_DEFAULT_ABI, type->nParam, JSX_GetFFIType(cx, type->returnType), type->cif.arg_types);
  return &type->cif;
}


ffi_type *JSX_GetFFIType(JSContext *cx, JSX_Type *type) {
  switch(type->type) {
    case POINTERTYPE:
      return &ffi_type_pointer;
    case VOIDTYPE:
      return &ffi_type_void;
    case INTTYPE:
    case UINTTYPE:
    case FLOATTYPE:
      return &((JSX_TypeNumeric *) type)->ffiType;
    case STRUCTTYPE: {
      JSX_TypeStructUnion *typesu = (JSX_TypeStructUnion *) type;
      if(!typesu->ffiType.elements) TypeStructUnion_init_ffiType_elements(cx, typesu);
      return &typesu->ffiType;
    }
    default:
      return NULL;
  }
}


static void TypeStructUnion_init_ffiType_elements(JSContext *cx, JSX_TypeStructUnion *typesu) {
    JSX_Type *type = (JSX_Type *) typesu;

    int nmember = 0;
    int bitsused = 0;
    int i;
    for(i = 0; i < typesu->nMember; i++) {
      int al=1;
      JSX_Type *memb = typesu->member[i].membertype;
      while (memb->type==ARRAYTYPE) {
        al *= ((JSX_TypeArray *) memb)->length;
        memb = ((JSX_TypeArray *) memb)->member;
      }
      if (memb->type==BITFIELDTYPE) {
        int length = ((JSX_TypeBitfield *) memb)->length;
        if(bitsused && bitsused + length < 8) {
          al = 0;
        } else {
          al = (bitsused + length) / 8;
          bitsused = (bitsused + length) % 8;
        }
      } else {
        bitsused = 0;
      }
      nmember+=al;
    }

    typesu->ffiType.elements = JS_malloc(cx, sizeof(ffi_type *) * (nmember + 1));
    // must specify size and alignment because
    // bitfields introduce alignment requirements
    // which are not reflected by the ffi members.
    typesu->ffiType.size = JSX_TypeSize(type);
    typesu->ffiType.alignment = JSX_TypeAlign(type);
    typesu->ffiType.type = FFI_TYPE_STRUCT;

    bitsused=0;
    nmember=0;
    for(i = 0; i < typesu->nMember; i++) {
      int al=1;
      int j;
      ffi_type *t;
      JSX_Type *memb = typesu->member[i].membertype;
      while (memb->type==ARRAYTYPE) {
        al *= ((JSX_TypeArray *) memb)->length;
        memb = ((JSX_TypeArray *) memb)->member;
      }
      if (memb->type==BITFIELDTYPE) {
        int length = ((JSX_TypeBitfield *) memb)->length;
        if(bitsused && bitsused + length < 8) {
          al = 0;
        } else {
          al = (bitsused + length) / 8;
          bitsused = (bitsused + length) % 8;
        }
        t = &ffi_type_uchar;
      } else {
        bitsused = 0;
        t = JSX_GetFFIType(cx, memb);
      }
      for(j = 0; j < al; j++) typesu->ffiType.elements[nmember++] = t;
    }
    typesu->ffiType.elements[nmember] = NULL;
}


static JSBool JSX_Type_SetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
  JSX_Type *type = JS_GetPrivate(cx, obj);

  if(JSVAL_IS_INT(id)) {
    int index = JSVAL_TO_INT(id);
    if(type->type == STRUCTTYPE || type->type == UNIONTYPE)
      return TypeStructUnion_SetMember(cx, (JSX_TypeStructUnion *) type, index, *vp);
  }

  return JS_TRUE;
}


static JSBool JSX_InitMemberType(JSContext *cx, JSX_SuMember *dest, JSObject *membertype) {
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
  dest->membertype = JS_GetPrivate(cx, JSVAL_TO_OBJECT(tmp));

  return JS_TRUE;
}


static void JSX_DestroyTypeStructUnion(JSContext *cx, JSX_TypeStructUnion *type) {
  int i;
  if (type) {
    if (type->member) {
      for (i=0; i<type->nMember; i++) {
        if(type->member[i].name)
          free(type->member[i].name); // strdup'd earlier
      }
      JS_free(cx,type->member);
    }
    if (type->ffiType.elements) {
      JS_free(cx, type->ffiType.elements);
    }
    JS_free(cx, type);
  }
}


static void JSX_DestroyTypeFunction(JSContext *cx, JSX_TypeFunction *type) {
  if (type) {
    if (type->param)
      JS_free(cx,type->param);
    if (type->cif.arg_types) {
      JS_free(cx, type->cif.arg_types);
    }
    JS_free(cx, type);
  }
}


static JSBool TypeFunction_SetMember(JSContext *cx, JSObject *obj, int memberno, jsval member) {
  JSX_TypeFunction *type;
  type = JS_GetPrivate(cx, obj);
  if(memberno >= type->nParam) type->nParam = memberno + 1;
  if(!JSVAL_IS_OBJECT(member) || JSVAL_IS_NULL(member)) return JS_FALSE;
  if(!FuncParam_Init(cx, type->param + memberno, JSVAL_TO_OBJECT(member))) return JS_FALSE;
  if(memberno == type->nParam - 1) {
    type->param[type->nParam].paramtype = sTypeVoid;
    type->param[type->nParam].isConst = 0;
  }
  if(type->cif.arg_types) {
    JS_free(cx, type->cif.arg_types);
    type->cif.arg_types = 0;
  }
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

  JSX_TypeFunction *type;
  type = (JSX_TypeFunction *) JS_malloc(cx, sizeof(JSX_TypeFunction));
  type->type=FUNCTIONTYPE;

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
  type->returnType=JS_GetPrivate(cx, JSVAL_TO_OBJECT(returnType));

  type->param = (JSX_FuncParam *) JS_malloc(cx, sizeof(JSX_FuncParam) * (type->nParam + 1));
  memset(type->param, 0, sizeof(JSX_FuncParam) * type->nParam);

  type->param[type->nParam].paramtype = sTypeVoid;
  type->param[type->nParam].isConst=0;
  type->cif.arg_types=0;

  int i;
  for(i = 0; i < nParam; i++) {
    jsval thisparam;
    JS_GetElement(cx, paramobj, i, &thisparam);
    TypeFunction_SetMember(cx, retobj, i, thisparam);
    JS_DefineElement(cx, retobj, i, thisparam, 0, 0, JSPROP_ENUMERATE | JSPROP_PERMANENT);
  }

  return JS_TRUE;
}


JSClass *JSX_GetTypeClass(void) {
  return &JSX_TypeClass;
}


static JSBool FuncParam_Init(JSContext *cx, JSX_FuncParam *dest, JSObject *membertype) {
  jsval tmp;
  JS_GetProperty(cx, membertype, "type", &tmp);
  if(!jsval_is_Type(cx, tmp)) {
    JSX_ReportException(cx, "Type.function(): one of the argument descriptors has a bad or missing .type property");
    return JS_FALSE;
  }
  dest->paramtype = JS_GetPrivate(cx, JSVAL_TO_OBJECT(tmp));
  JS_GetProperty(cx, membertype, "const", &tmp);
  dest->isConst = tmp == JSVAL_TRUE ? 1 : 0;
  return JS_TRUE;
}


int JSX_TypeAlignBits(JSX_Type *type) {
  if(type->type == BITFIELDTYPE) return 1;
  return JSX_TypeAlign(type) * 8;
}


static int JSX_TypeAlign(JSX_Type *type) {
  switch(type->type) {
  case POINTERTYPE:
    return ffi_type_pointer.alignment;
  case BITFIELDTYPE: // when calculating struct alignment, this is it.
    return JSX_TypeAlign(((JSX_TypeBitfield *) type)->member);
  case ARRAYTYPE:
    return JSX_TypeAlign(((JSX_TypeArray *) type)->member);
  case UINTTYPE:
  case INTTYPE:
  case FLOATTYPE:
    return ((JSX_TypeNumeric *) type)->ffiType.alignment;
  case STRUCTTYPE:
  case UNIONTYPE: {
    int i, ret = 0, len = ((JSX_TypeStructUnion *) type)->nMember;
    for (i=0; i<len; i++) {
      int thisalign = JSX_TypeAlign(((JSX_TypeStructUnion *) type)->member[i].membertype);
      if (thisalign>ret) ret=thisalign;
    }
    return ret;
  }
  default: // VOIDTYPE, FUNCTIONTYPE
    return 0; // Error
  }
}


int JSX_TypeSizeBits(JSX_Type *type) {
  if(type->type == BITFIELDTYPE) return ((JSX_TypeBitfield *) type)->length;
  return JSX_TypeSize(type) * 8;
}


int JSX_TypeSize(JSX_Type *type) {
  switch(type->type) {
  case POINTERTYPE:
    return ffi_type_pointer.size;
  case ARRAYTYPE:
    return ((JSX_TypeArray *) type)->length * JSX_TypeSize(((JSX_TypeArray *) type)->member);
  case UINTTYPE:
  case INTTYPE:
  case FLOATTYPE:
    return ((JSX_TypeNumeric *) type)->ffiType.size;
  case STRUCTTYPE:
  case UNIONTYPE: {
    int align = JSX_TypeAlign(type);
    return (((((JSX_TypeStructUnion *) type)->sizeOf + 7) / 8 + align - 1) / align) * align;
  }
  default: // VOIDTYPE, FUNCTIONTYPE
    return 0; // Error
  }
}


static JSBool TypeStructUnion_SetMember(JSContext *cx, JSX_TypeStructUnion *type, int memberno, jsval member) {
  int i;
  int thisalign;
  int thissize;
  
  if (type->member_capacity <= memberno) {
    int old_capacity=type->member_capacity;

    if (type->member_capacity==0)
      type->member_capacity=8;
    else
      type->member_capacity*=2;

    if (type->member_capacity < memberno + 1)
      type->member_capacity = memberno + 1;

    if (type->member) {
      type->member = JS_realloc(cx, type->member, sizeof(JSX_SuMember) * type->member_capacity); // membertype same size as paramtype
      memset(type->member + old_capacity, 0, sizeof(JSX_SuMember) * (type->member_capacity - old_capacity));
    } else {
      type->member = JS_malloc(cx, sizeof(JSX_SuMember) * type->member_capacity); // membertype same size as paramtype
      memset(type->member, 0, sizeof(JSX_SuMember) * type->member_capacity);
    }
  }

  if (memberno>=type->nMember)
    type->nMember=memberno+1;

  if(!JSVAL_IS_OBJECT(member) || JSVAL_IS_NULL(member)) return JS_FALSE;
  
  if(type->type == UNIONTYPE) {
    if(!JSX_InitMemberType(cx, type->member+memberno, JSVAL_TO_OBJECT(member))) return JS_FALSE;

    type->member[memberno].offset=0;
    thissize = JSX_TypeSizeBits(type->member[memberno].membertype);
    if (thissize > type->sizeOf)
      type->sizeOf=thissize;

  } else { // STRUCTTYPE
    if (!JSX_InitMemberType(cx, type->member+memberno, JSVAL_TO_OBJECT(member)))
      return JS_FALSE;

    if(((JSX_TypeStructUnion *) type)->ffiType.elements) {
      JS_free(cx, ((JSX_TypeStructUnion *) type)->ffiType.elements);
      ((JSX_TypeStructUnion *) type)->ffiType.elements = 0;
    }

    if(memberno != type->nMember - 1 && memberno > 0 && type->member[memberno - 1].membertype) {
      type->sizeOf = type->member[memberno - 1].offset + JSX_TypeSizeBits(type->member[memberno - 1].membertype);
    }
      
    for(i = memberno; i < type->nMember && type->member[i].membertype; i++) {
      thisalign = JSX_TypeAlignBits(type->member[i].membertype);

      if (thisalign==0) {
        JSX_ReportException(cx, "Division by zero");
        return JS_FALSE;
      }

      type->sizeOf+=(thisalign - type->sizeOf % thisalign) % thisalign;
      type->member[i].offset=type->sizeOf;
      type->sizeOf += JSX_TypeSizeBits(type->member[i].membertype);
    }
  }

  return JS_TRUE;
}


// typeid must obviously be STRUCTTYPE or UNIONTYPE
static JSBool JSX_NewTypeStructUnion(JSContext *cx, int nMember, jsval *member, jsval *rval, int typeid, JSObject* proto) {
  JSObject *retobj;
  JSX_TypeStructUnion *type;
  int i;
  
  retobj = JS_NewObject(cx, &JSX_TypeClass, proto, 0);
  *rval=OBJECT_TO_JSVAL(retobj);

  type = (JSX_TypeStructUnion *) JS_malloc(cx, sizeof(JSX_TypeStructUnion));

  type->type = typeid;
  type->nMember=nMember;
  type->member_capacity=nMember;
  type->member = nMember ? (JSX_SuMember *) JS_malloc(cx, sizeof(JSX_SuMember) * nMember) : 0;
  memset(type->member, 0, sizeof(JSX_SuMember) * nMember);
  type->sizeOf=0;
  JS_SetPrivate(cx, retobj, type);
  type->ffiType.elements=0;

  for (i=0; i<nMember; i++) {
    if(!TypeStructUnion_SetMember(cx, type, i, member[i]))
      goto failure;
    JS_DefineElement(cx, retobj, i, member[i], 0, 0, JSPROP_ENUMERATE | JSPROP_PERMANENT);
  }
  
  return JS_TRUE;

 failure:
  JSX_DestroyTypeStructUnion(cx, type);
  return JS_FALSE;
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

  JSX_TypePointer *type;
  type = (JSX_TypePointer *) JS_malloc(cx, sizeof(JSX_TypePointer));
  type->type=POINTERTYPE;
  JS_SetPrivate(cx, retobj, type);
  type->direct = sTypeVoid;
  JS_DefineElement(cx, retobj, 0, direct, 0, 0, JSPROP_ENUMERATE | JSPROP_PERMANENT);
  if(direct != JSVAL_VOID) type->direct = (JSX_Type *) JS_GetPrivate(cx, JSVAL_TO_OBJECT(direct));

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
  JSX_TypeArray *type;
  retobj = JS_NewObject(cx, &JSX_TypeClass, s_Type_array_proto, 0);
  *rval=OBJECT_TO_JSVAL(retobj);
  type = (JSX_TypeArray *) JS_malloc(cx, sizeof(JSX_TypeArray));
  type->type=ARRAYTYPE;
  JS_SetPrivate(cx, retobj, type);
  type->length = JSVAL_TO_INT(len);
  type->member = (JSX_Type *) JS_GetPrivate(cx, JSVAL_TO_OBJECT(member));
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
  JSX_TypeBitfield *type;
  type = (JSX_TypeBitfield *) JS_malloc(cx, sizeof(JSX_TypeBitfield));
  type->type=BITFIELDTYPE;
  JS_SetPrivate(cx, retobj, type);
  type->length = JSVAL_TO_INT(len);
  type->member = (JSX_Type *) JS_GetPrivate(cx, JSVAL_TO_OBJECT(member));
  return JS_TRUE;
}


JSX_Type *GetVoidType(void) {
  return sTypeVoid;
}


static jsval init_numeric_Type(JSContext *cx, int typeid, int size, ffi_type ffit) {
  JSObject *newtype;
  newtype = JS_NewObject(cx, &JSX_TypeClass, 0, 0);
  jsval newval = OBJECT_TO_JSVAL(newtype);
  JSX_TypeNumeric *type;
  type = (JSX_TypeNumeric *) JS_malloc(cx, sizeof(JSX_TypeNumeric));
  type->type = typeid;
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
  JSObject *newtype;
  jsval newval;
  JSX_Type *type;

  newtype = JS_NewObject(cx, &JSX_TypeClass, 0, 0);
  newval = OBJECT_TO_JSVAL(newtype);
  JS_SetProperty(cx, typeobj, "void", &newval);

  type = (JSX_Type *) JS_malloc(cx, sizeof(JSX_Type));
  type->type = VOIDTYPE;
  JS_SetPrivate(cx, newtype, type);

  sTypeVoid = type;
}


static JSBool JSX_Type_new(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
  // Someone is calling the constructor against all common sense
  JSX_ReportException(cx, "Type is not a constructor");
  return JS_FALSE;
}


static void JSX_Type_finalize(JSContext *cx,  JSObject *obj) {
  JSX_Type *type = (JSX_Type *) JS_GetPrivate(cx, obj);
  if (type==0)
    return;

  switch(type->type) {
  case FUNCTIONTYPE:
    JSX_DestroyTypeFunction(cx, (JSX_TypeFunction *) type);
    break;
  case UNIONTYPE:
  case STRUCTTYPE:
    JSX_DestroyTypeStructUnion(cx, (JSX_TypeStructUnion *) type);
    break;
  default:
    JS_free(cx, type);
  }
}


static JSBool JSX_Type_struct(JSContext *cx,  JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
  return JSX_NewTypeStructUnion(cx, argc, argv, rval, STRUCTTYPE, s_Type_struct_proto);
}


static JSBool JSX_Type_union(JSContext *cx,  JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
  return JSX_NewTypeStructUnion(cx, argc, argv, rval, UNIONTYPE, s_Type_union_proto);
}


static JSBool Type_sizeof(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
  jsval arg = argv[0];
  *rval = JSVAL_VOID;
  if(!jsval_is_Type(cx, arg)) {
    JSX_ReportException(cx, "Type.sizeof(): the argument must be a Type instance");
    return JS_FALSE;
  }
  int size = JSX_TypeSize((JSX_Type *) JS_GetPrivate(cx, JSVAL_TO_OBJECT(arg)));
  if(size) *rval = INT_TO_JSVAL(size);
  return JS_TRUE;
}


int JSX_TypeSize_multi(JSContext *cx, uintN nargs, JSX_FuncParam *type, jsval *vp, ffi_type **arg_types) {
  int ret=0;
  int siz;
  uintN i;
  JSX_Type *thistype;

  for (i=0; i<nargs; i++) {
    if(type && type->paramtype->type == VOIDTYPE) type = 0; // End of param list

    if(JSVAL_IS_OBJECT(*vp) && *vp != JSVAL_NULL && JS_InstanceOf(cx, JSVAL_TO_OBJECT(*vp), JSX_GetTypeClass(), NULL)) {
      thistype=JS_GetPrivate(cx,JSVAL_TO_OBJECT(*vp));
      vp++;
      i++;
      if (i==nargs) break;
    } else {
      thistype = type ? type->paramtype : 0;
    }

    if (!thistype) {
      siz=JSX_Get(cx, 0, 0, 0, 0, vp); // Get size of C type guessed from js type
      if (arg_types) {
        int jstype = JSX_JSType(cx, *vp);
        switch(jstype) {
          case JSVAL_STRING:
            *(arg_types++) = &ffi_type_pointer;
            break;
          case JSVAL_INT:
            *(arg_types++) = &ffi_type_sint;
            break;
          case JSVAL_DOUBLE:
            *(arg_types++) = &ffi_type_double;
            break;
        }
      }
    } else {
      siz=JSX_TypeSize(thistype);
      if(arg_types) *(arg_types++) = JSX_GetFFIType(cx, thistype);
    }
    if (!siz) return 0; // error
    ret+=siz;
    vp++;
    if (type)
      type++;
  }

  if (arg_types)
    *arg_types=0;
  return ret;
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


JSBool JSX_TypeContainsPointer(JSX_Type *type) {
  switch(type->type) {
    case POINTERTYPE:
      return JS_TRUE;
    case ARRAYTYPE:
      return JSX_TypeContainsPointer(((JSX_TypeArray *) type)->member);
    case UNIONTYPE:
    case STRUCTTYPE: {
      int i;
      JSX_TypeStructUnion *sutype = (JSX_TypeStructUnion *) type;
      for(i = 0; i < sutype->nMember; i++)
        if(JSX_TypeContainsPointer(sutype->member[i].membertype))
          return JS_TRUE;
      return JS_FALSE;
    }
    default:
      return JS_FALSE;
  }
}


jsval JSX_make_Type(JSContext *cx, JSObject *obj) {
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
