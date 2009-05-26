#include "Type.h"
#include <string.h>
#include "Pointer.h"
#include <stdarg.h>
#include <stdlib.h>
#include "util.h"


static void JSX_Type_finalize(JSContext *cx, JSObject *obj);
static JSBool JSX_Type_SetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *rval);
static JSBool TypeStructUnion_SetMember(JSContext *cx, JSX_TypeStructUnion *type, int memberno, jsval member);
static int JSX_TypeAlign(JSX_Type *type);


static JSObject *sTypeChar = NULL;
static JSObject *sTypeVoid = NULL;


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


ffi_cif *JSX_GetCIF(JSContext *cx, JSX_TypeFunction *type) {
  if (type->cif.arg_types)
    return &type->cif;

  type->cif.arg_types=JS_malloc(cx, sizeof(ffi_type)*(type->nParam));
  int callconv=FFI_DEFAULT_ABI;
  int i;
  for (i=0; i<type->nParam; i++) {
    if (type->param[i].type->type==ARRAYTYPE)
      type->cif.arg_types[i]=&ffi_type_pointer;
    else
      type->cif.arg_types[i]=JSX_GetFFIType(cx, type->param[i].type);
  }
  ffi_prep_cif(&type->cif, callconv, type->nParam, JSX_GetFFIType(cx, type->returnType), type->cif.arg_types);
  return &type->cif;
}


ffi_type *JSX_GetFFIType(JSContext *cx, JSX_Type *type) {
  int nmember;
  int i;
  int bitsused=0;

  switch(type->type) {
  case POINTERTYPE:
    return &ffi_type_pointer;
  case VOIDTYPE:
    return &ffi_type_void;
  case INTTYPE:
  case UINTTYPE:
  case FLOATTYPE:
    return &((JSX_TypeNumeric *) type)->ffiType;
  case STRUCTTYPE:
    if(((JSX_TypeStructUnion *) type)->ffiType.elements)
      return &((JSX_TypeStructUnion *) type)->ffiType;

    nmember=0;

    for(i = 0; i < ((JSX_TypeStructUnion *) type)->nMember; i++) {
      int al=1;
      JSX_Type *memb = ((JSX_TypeStructUnion *) type)->member[i].type;
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

    ((JSX_TypeStructUnion *) type)->ffiType.elements = JS_malloc(cx, sizeof(ffi_type *) * (nmember + 1));
    // must specify size and alignment because
    // bitfields introduce alignment requirements
    // which are not reflected by the ffi members.
    ((JSX_TypeStructUnion *) type)->ffiType.size = JSX_TypeSize(type);
    ((JSX_TypeStructUnion *) type)->ffiType.alignment = JSX_TypeAlign(type);
    ((JSX_TypeStructUnion *) type)->ffiType.type = FFI_TYPE_STRUCT;

    bitsused=0;
    nmember=0;
    for(i = 0; i < ((JSX_TypeStructUnion *) type)->nMember; i++) {
      int al=1;
      int j;
      ffi_type *t;
      JSX_Type *memb = ((JSX_TypeStructUnion *) type)->member[i].type;
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
      for(j = 0; j < al; j++) ((JSX_TypeStructUnion *) type)->ffiType.elements[nmember++] = t;
    }
    ((JSX_TypeStructUnion *) type)->ffiType.elements[nmember] = NULL;
    return &((JSX_TypeStructUnion *) type)->ffiType;
  }
}


static JSBool JSX_Type_SetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
  JSX_Type *type = JS_GetPrivate(cx, obj);

  if(JSVAL_IS_INT(id)) {
    int index=JSVAL_TO_INT(id);

    switch(type->type) {
    case STRUCTTYPE:
    case UNIONTYPE:
      return TypeStructUnion_SetMember(cx, (JSX_TypeStructUnion *) type, index, *vp);

    case ARRAYTYPE:
    case POINTERTYPE:
      if(index != 0) break;
      if(!JSVAL_IS_OBJECT(*vp) || JSVAL_IS_NULL(*vp) || !JS_InstanceOf(cx, JSVAL_TO_OBJECT(*vp), &JSX_TypeClass, NULL)) {
        JSX_ReportException(cx, "Wrong type");
        return JS_FALSE;
      }
      ((JSX_TypeArray *) type)->member = (JSX_Type *) JS_GetPrivate(cx, JSVAL_TO_OBJECT(*vp));
      break;
    }
  }

  return JS_TRUE;
}


static JSBool JSX_InitMemberType(JSContext *cx, JSX_MemberType *dest, JSObject *membertype) {
  jsval tmp;

  JS_GetProperty(cx, membertype, "name", &tmp);
  if(tmp == JSVAL_VOID || !JSVAL_IS_STRING(tmp)) {
    JSX_ReportException(cx, "Wrong or missing 'name' property in member type object");
    return JS_FALSE;
  }
  dest->name = strdup(JS_GetStringBytes(JSVAL_TO_STRING(tmp)));

  JS_GetProperty(cx, membertype, "type", &tmp);
  if(!JSVAL_IS_OBJECT(tmp) || tmp == JSVAL_NULL || !JS_InstanceOf(cx, JSVAL_TO_OBJECT(tmp), &JSX_TypeClass, NULL)) {
    JSX_ReportException(cx, "Wrong or missing 'type' property in member type object");
    // name is freed later
    return JS_FALSE;
  }
  dest->type = JS_GetPrivate(cx, JSVAL_TO_OBJECT(tmp));

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
  if(!JSX_InitParamType(cx, type->param + memberno, JSVAL_TO_OBJECT(member))) return JS_FALSE;
  if(memberno == type->nParam - 1) {
    type->param[type->nParam].type = JS_GetPrivate(cx, JSX_GetVoidType());
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

  if(!JSVAL_IS_OBJECT(returnType) || returnType == JSVAL_NULL || !JS_InstanceOf(cx, JSVAL_TO_OBJECT(returnType), &JSX_TypeClass, NULL)) {
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
  retobj=JS_NewObject(cx, &JSX_TypeClass, 0, 0);
  *rval=OBJECT_TO_JSVAL(retobj);

  JSObject *paramobj;
  int nParam;
  paramobj = JSVAL_TO_OBJECT(params);
  if(!JS_GetArrayLength(cx, paramobj, &nParam)) return JS_FALSE;

  type->nParam=nParam;
  type->typeObject=retobj;
  JS_SetPrivate(cx, retobj, type);

  JS_DefineProperty(cx, retobj, "returnType", returnType, 0, 0, JSPROP_ENUMERATE | JSPROP_PERMANENT);
  type->returnType=JS_GetPrivate(cx, JSVAL_TO_OBJECT(returnType));

  type->param = (JSX_ParamType *) JS_malloc(cx, sizeof(JSX_ParamType) * (type->nParam + 1));
  memset(type->param, 0, sizeof(JSX_ParamType) * type->nParam);

  type->param[type->nParam].type = JS_GetPrivate(cx, JSX_GetVoidType());
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


JSBool JSX_InitParamType(JSContext *cx, JSX_ParamType *dest, JSObject *membertype) {
  jsval tmp;
  JS_GetProperty(cx, membertype, "type", &tmp);
  if(!JSVAL_IS_OBJECT(tmp) || tmp == JSVAL_NULL || !JS_InstanceOf(cx, JSVAL_TO_OBJECT(tmp), &JSX_TypeClass, NULL)) {
    JSX_ReportException(cx, "Type.function(): one of the argument descriptors has a bad or missing .type property");
    return JS_FALSE;
  }
  dest->type = JS_GetPrivate(cx, JSVAL_TO_OBJECT(tmp));
  JS_GetProperty(cx, membertype, "const", &tmp);
  dest->isConst = tmp == JSVAL_TRUE ? 1 : 0;
  return JS_TRUE;
}


int JSX_TypeAlignBits(JSX_Type *type) {
  switch(type->type) {
  case BITFIELDTYPE:
    return 1;
  default:
    return JSX_TypeAlign(type)*8;
  }
}


static int JSX_TypeAlign(JSX_Type *type) {
  int len;
  int ret;
  int i;

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
  case UNIONTYPE:
    len = ((JSX_TypeStructUnion *) type)->nMember;
    ret=0;
    for (i=0; i<len; i++) {
      int thisalign = JSX_TypeAlign(((JSX_TypeStructUnion *) type)->member[i].type);
      if (thisalign>ret) ret=thisalign;
    }
    return ret;
  default: // VOIDTYPE, FUNCTIONTYPE
    return 0; // Error
  }
}


int JSX_TypeSizeBits(JSX_Type *type) {
  switch(type->type) {
  case BITFIELDTYPE:
    return ((JSX_TypeBitfield *) type)->length;
  default:
    return JSX_TypeSize(type)*8;
  }
}


int JSX_TypeSize(JSX_Type *type) {
  int align;

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
  case UNIONTYPE:
    align=JSX_TypeAlign(type);
    return (((((JSX_TypeStructUnion *) type)->sizeOf + 7) / 8 + align - 1) / align) * align;
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
      type->member = JS_realloc(cx, type->member, sizeof(JSX_MemberType) * type->member_capacity); // membertype same size as paramtype
      memset(type->member + old_capacity, 0, sizeof(JSX_MemberType) * (type->member_capacity - old_capacity));
    } else {
      type->member = JS_malloc(cx, sizeof(JSX_MemberType) * type->member_capacity); // membertype same size as paramtype
      memset(type->member, 0, sizeof(JSX_MemberType) * type->member_capacity);
    }
  }

  if (memberno>=type->nMember)
    type->nMember=memberno+1;

  if(!JSVAL_IS_OBJECT(member) || JSVAL_IS_NULL(member)) return JS_FALSE;
  
  if(type->type == UNIONTYPE) {
    if(!JSX_InitMemberType(cx, type->member+memberno, JSVAL_TO_OBJECT(member))) return JS_FALSE;

    type->member[memberno].offset=0;
    thissize=JSX_TypeSizeBits(type->member[memberno].type);
    if (thissize > type->sizeOf)
      type->sizeOf=thissize;

    JS_DefineProperty(cx, JSVAL_TO_OBJECT(member), "offset", INT_TO_JSVAL(0), 0, 0, JSPROP_READONLY | JSPROP_PERMANENT);

  } else { // STRUCTTYPE
    if (!JSX_InitMemberType(cx, type->member+memberno, JSVAL_TO_OBJECT(member)))
      return JS_FALSE;

    if(((JSX_TypeStructUnion *) type)->ffiType.elements) {
      JS_free(cx, ((JSX_TypeStructUnion *) type)->ffiType.elements);
      ((JSX_TypeStructUnion *) type)->ffiType.elements = 0;
    }

    if (memberno!=type->nMember-1 && memberno>0 && type->member[memberno-1].type) {
      type->sizeOf=type->member[memberno-1].offset + JSX_TypeSizeBits(type->member[memberno-1].type);
    }
      
    for (i=memberno; i<type->nMember && type->member[i].type; i++) {
      thisalign=JSX_TypeAlignBits(type->member[i].type);

      if (thisalign==0) {
        JSX_ReportException(cx, "Division by zero");
        return JS_FALSE;
      }

      type->sizeOf+=(thisalign - type->sizeOf % thisalign) % thisalign;
      type->member[i].offset=type->sizeOf;
      type->sizeOf+=JSX_TypeSizeBits(type->member[i].type);
      
      JS_DefineProperty(cx, JSVAL_TO_OBJECT(member), "offset", INT_TO_JSVAL(type->member[i].offset), 0, 0, JSPROP_READONLY | JSPROP_PERMANENT);
    }
  }

  return JS_TRUE;
}


// typeid must obviously be STRUCTTYPE or UNIONTYPE
static JSBool JSX_NewTypeStructUnion(JSContext *cx, int nMember, jsval *member, jsval *rval, int typeid) {
  JSObject *retobj;
  JSX_TypeStructUnion *type;
  int i;
  
  retobj=JS_NewObject(cx, &JSX_TypeClass, 0, 0);
  *rval=OBJECT_TO_JSVAL(retobj);

  type = (JSX_TypeStructUnion *) JS_malloc(cx, sizeof(JSX_TypeStructUnion));

  type->type = typeid;;
  type->nMember=nMember;
  type->member_capacity=nMember;
  type->member = nMember ? (JSX_MemberType *) JS_malloc(cx, sizeof(JSX_MemberType) * nMember) : 0;
  memset(type->member, 0, sizeof(JSX_MemberType) * nMember);
  type->sizeOf=0;
  JS_SetPrivate(cx, retobj, type);
  type->typeObject=retobj;
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


static JSBool Type_pointer(JSContext *cx,  JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
  jsval direct = argc > 0 ? argv[0] : JSVAL_VOID;
  JSObject *retobj;
  JSX_TypePointer *type;

  retobj=JS_NewObject(cx, &JSX_TypeClass, 0, 0);
  *rval=OBJECT_TO_JSVAL(retobj);

  type = (JSX_TypePointer *) JS_malloc(cx, sizeof(JSX_TypePointer));

  type->type=POINTERTYPE;
  JS_SetPrivate(cx, retobj, type);

  type->direct = JS_GetPrivate(cx, JSX_GetVoidType());
  JS_DefineElement(cx, retobj, 0, direct, 0, 0, JSPROP_ENUMERATE | JSPROP_PERMANENT);
  type->typeObject=retobj;

  if (direct==JSVAL_VOID) {
    return JS_TRUE;
  }

  if (!JSVAL_IS_OBJECT(direct) || 
      JSVAL_IS_NULL(direct) ||
      !JS_InstanceOf(cx, JSVAL_TO_OBJECT(direct), &JSX_TypeClass, NULL)) {
    JSX_ReportException(cx, "Wrong type to Type.pointer");
    goto failure;
  }
    
  type->direct = (JSX_Type *) JS_GetPrivate(cx, JSVAL_TO_OBJECT(direct));

  return JS_TRUE;

 failure:
  if (type)
    JS_free(cx, type);

  return JS_FALSE;
}


static JSBool Type_array(JSContext *cx,  JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
  jsval member = argv[0];
  jsval len = argv[1];

  if(!JSVAL_IS_OBJECT(member) || JSVAL_IS_NULL(member) || !JS_InstanceOf(cx, JSVAL_TO_OBJECT(member), &JSX_TypeClass, NULL)) {
    JSX_ReportException(cx, "Type.array(): first argument must be a Type instance");
    return JS_FALSE;
  }
  if(!JSVAL_IS_INT(len)) {
    JSX_ReportException(cx, "Type.array(): second argument must be an integer");
    return JS_FALSE;
  }

  JSObject *retobj;
  JSX_TypeArray *type;
  retobj=JS_NewObject(cx, &JSX_TypeClass, 0, 0);
  *rval=OBJECT_TO_JSVAL(retobj);
  type = (JSX_TypeArray *) JS_malloc(cx, sizeof(JSX_TypeArray));
  type->type=ARRAYTYPE;
  JS_SetPrivate(cx, retobj, type);
  type->length = JSVAL_TO_INT(len);
  type->member = JS_GetPrivate(cx, JSX_GetVoidType());
  type->typeObject=retobj;
  type->member = (JSX_Type *) JS_GetPrivate(cx, JSVAL_TO_OBJECT(member));
  return JS_TRUE;
}


static JSBool Type_bitfield(JSContext *cx,  JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
  jsval member = argc > 0 ? argv[0] : JSVAL_VOID;
  jsval len = argc > 1 ? argv[1] : JSVAL_VOID;

  JSObject *retobj;
  JSX_TypeBitfield *type;

  retobj=JS_NewObject(cx, &JSX_TypeClass, 0, 0);
  *rval=OBJECT_TO_JSVAL(retobj);

  type = (JSX_TypeBitfield *) JS_malloc(cx, sizeof(JSX_TypeBitfield));

  type->type=BITFIELDTYPE;
  JS_SetPrivate(cx, retobj, type);

  if (JSVAL_IS_INT(len)) 
    type->length=JSVAL_TO_INT(len);
  else
    type->length=0;

  type->member = JS_GetPrivate(cx, JSX_GetVoidType());
  type->typeObject=retobj;

  if (member==JSVAL_VOID) {
    return JS_TRUE;
  }

  if (!JSVAL_IS_OBJECT(member) || 
      JSVAL_IS_NULL(member) ||
      !JS_InstanceOf(cx, JSVAL_TO_OBJECT(member), &JSX_TypeClass, NULL) ||
      !JSVAL_IS_INT(len)) {
    JSX_ReportException(cx, "Wrong type to Type.bitfield");
    goto failure;
  }
    
  type->member = (JSX_Type *) JS_GetPrivate(cx, JSVAL_TO_OBJECT(member));

  return JS_TRUE;

 failure:
  if (type)
    JS_free(cx, type);
  return JS_FALSE;
}


JSObject *JSX_GetCharType(void) {
  return sTypeChar;
}


JSObject *JSX_GetVoidType(void) {
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
  type->typeObject = newtype;
  JS_SetPrivate(cx, newtype, type);
  return newval;
}


static void init_int_types(JSContext *cx, JSObject *typeobj) {
  jsval tmp, uchar;
  tmp = uchar = init_numeric_Type(cx, UINTTYPE, 0, ffi_type_uchar);
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

  // It seems a bit iffy that this is uchar rather than char (which is typically signed), but that's what the old code did
  sTypeChar = JSVAL_TO_OBJECT(uchar);
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
  type->typeObject = newtype;
  JS_SetPrivate(cx, newtype, type);

  sTypeVoid = newtype;
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
  return JSX_NewTypeStructUnion(cx, argc, argv, rval, STRUCTTYPE);
}


static JSBool JSX_Type_union(JSContext *cx,  JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
  return JSX_NewTypeStructUnion(cx, argc, argv, rval, UNIONTYPE);
}


static JSBool JSX_Type_toString(JSContext *cx,  JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
  JSX_Type *type;
  char *name;
  type = (JSX_Type *) JS_GetPrivate(cx, obj);

  switch(type->type) {
  case FUNCTIONTYPE:
    name="function";
    break;
  case STRUCTTYPE:
    name="struct";
    break;
  case UNIONTYPE:
    name="union";
    break;
  case POINTERTYPE:
    name="pointer";
    break;
  case ARRAYTYPE:
    name="array";
    break;
  case BITFIELDTYPE:
    name="bitfield";
    break;
  default:
    name="[Type unknown]";
  }

  *rval=STRING_TO_JSVAL(JS_NewStringCopyZ(cx, name));

  return JS_TRUE;
}


static JSBool JSX_Type_sizeof(JSContext *cx,  JSObject *obj, jsval id, jsval *rval) {
  *rval = JSVAL_VOID;
  int size = JSX_TypeSize((JSX_Type *) JS_GetPrivate(cx, obj));
  if(size) *rval = INT_TO_JSVAL(size);
  return JS_TRUE;
}


int JSX_TypeSize_multi(JSContext *cx, uintN nargs, JSX_ParamType *type, jsval *vp, ffi_type **arg_types) {
  int ret=0;
  int siz;
  uintN i;
  JSX_Type *thistype;

  for (i=0; i<nargs; i++) {
    if (type && type->type->type==VOIDTYPE) {// End of param list
      type=0;
    }

    if(JSVAL_IS_OBJECT(*vp) && *vp != JSVAL_NULL && JS_InstanceOf(cx, JSVAL_TO_OBJECT(*vp), JSX_GetTypeClass(), NULL)) {
      thistype=JS_GetPrivate(cx,JSVAL_TO_OBJECT(*vp));
      vp++;
      i++;
      if (i==nargs) break;
    } else {
      if (type)
        thistype=type->type;
      else
        thistype=0;
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
      if(JS_InstanceOf(cx, JSVAL_TO_OBJECT(v), JSX_GetTypeClass(), NULL)) return JSTYPE;
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


// Determine the C type to an appropriate level of detail for the big 2D switch
int JSX_CType(JSX_Type *type) {
  if(type == NULL) return UNDEFTYPE;

  if(type->type == ARRAYTYPE) {
    JSX_TypeArray *atype = (JSX_TypeArray *) type;
    if(atype->member->type == INTTYPE || atype->member->type == UINTTYPE) {
      int size = ((JSX_TypeNumeric *) atype->member)->size;
      if(size == 0) return ACHARTYPE;
      if(size == 1) return ASHORTTYPE;
    }
  }
  if(type->type == POINTERTYPE) {
    JSX_TypePointer *ptype = (JSX_TypePointer *) type;
    if(ptype->direct->type == INTTYPE || ptype->direct->type == UINTTYPE) {
      int size = ((JSX_TypeNumeric *) ptype->direct)->size;
      if(size == 0) return PCHARTYPE;
      if(size == 1) return PSHORTTYPE;
    }
  }
  return type->type;
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
        if(JSX_TypeContainsPointer(sutype->member[i].type))
          return JS_TRUE;
      return JS_FALSE;
    default:
      return JS_FALSE;
    }
  }
}


jsval JSX_make_Type(JSContext *cx, JSObject *obj) {
  JSObject *typeobj;
  JSObject *protoobj;

  static struct JSFunctionSpec staticfunc[]={
    {"array", Type_array, 2, 0, 0},
    {"bitfield", Type_bitfield, 1, 0, 0},
    {"pointer", Type_pointer, 1, 0, 0},
    {"struct",JSX_Type_struct,1,0,0},
    {"union",JSX_Type_union,1,0,0},
    {"function", Type_function, 2, 0, 0},
    {0,0,0,0,0}
  };

  static struct JSFunctionSpec memberfunc[]={
    {"toString",JSX_Type_toString,0,0,0},
    {0,0,0,0,0}
  };

  static struct JSPropertySpec memberprop[]={
    {"sizeof",0,JSPROP_READONLY | JSPROP_PERMANENT, JSX_Type_sizeof,0},
    {0,0,0,0,0}
  };

  protoobj=JS_NewObject(cx, 0, 0, 0);
  if(!protoobj) return JSVAL_VOID;

  typeobj=JS_InitClass(cx, obj, protoobj, &JSX_TypeClass, JSX_Type_new, 0, memberprop, memberfunc, 0, staticfunc);

  if(!typeobj) return JSVAL_VOID;

  typeobj=JS_GetConstructor(cx, typeobj);

  init_int_types(cx, typeobj);
  init_float_types(cx, typeobj);
  init_other_types(cx, typeobj);

  return OBJECT_TO_JSVAL(typeobj);
}
