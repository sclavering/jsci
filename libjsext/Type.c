#include "Type.h"
#include <string.h>
#include "Pointer.h"
#include <stdarg.h>
#include <stdlib.h>
#include "util.h"


static void JSX_Type_finalize(JSContext *cx, JSObject *obj);
static JSBool JSX_Type_SetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *rval);
static JSBool JSX_SetMember(JSContext *cx, JSObject *obj, int memberno, jsval member);
int JSX_TypeAlign(JSX_Type *type);


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


static JSObject *jsx_Type_objects[7][6][3]; // type, size, signedness


static JSBool JSX_Type_length(JSContext *cx,  JSObject *obj, jsval id, jsval *rval) {
  JSX_TypeArray *type; // also works with structs / unions / functions / bitfields
  type = (JSX_TypeArray *) JS_GetPrivate(cx, obj);
  if(type->type != FUNCTIONTYPE && type->type != STRUCTTYPE && type->type != UNIONTYPE && type->type != BITFIELDTYPE && type->type != ARRAYTYPE) {
    *rval = JSVAL_VOID;
  } else {
    *rval = INT_TO_JSVAL(type->length);
  }
  return JS_TRUE;
}


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
    return &((JSX_TypeInt *) type)->ffiType;
  case FLOATTYPE:
    return &((JSX_TypeFloat *) type)->ffiType;
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
	if (bitsused && bitsused+length<8) 
	  al=0;
	else {
	  al=(bitsused+length)/8;
	  bitsused=(bitsused+length)%8;
	}
      } else {
	bitsused=0;
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
	if (bitsused && bitsused+length<8) 
	  al=0;
	else {
	  al=(bitsused+length)/8;
	  bitsused=(bitsused+length)%8;
	}
	t=&ffi_type_uchar;
      } else {
	bitsused=0;
	t=JSX_GetFFIType(cx, memb);
      }
      for(j = 0; j < al; j++) ((JSX_TypeStructUnion *) type)->ffiType.elements[nmember++] = t;
    }
    ((JSX_TypeStructUnion *) type)->ffiType.elements[nmember] = NULL;
    return &((JSX_TypeStructUnion *) type)->ffiType;
  }
}


static JSBool JSX_Type_callConv(JSContext *cx,  JSObject *obj, jsval id, jsval *rval) {
  JSX_TypeFunction *type;
  char *name;

  type = (JSX_TypeFunction *) JS_GetPrivate(cx, obj);
  if (type->type!=FUNCTIONTYPE)
    return JS_TRUE;

  switch(type->callConv) {
  case CDECLCONV:
    name="cdecl";
    break;
  case STDCALLCONV:
    name="stdcall";
    break;
  }

  *rval=STRING_TO_JSVAL(JS_NewStringCopyZ(cx, name));
  return JS_TRUE;
}


static JSBool JSX_Type_elipsis(JSContext *cx,  JSObject *obj, jsval id, jsval *rval) {
  JSX_TypeFunction *type;
  type = (JSX_TypeFunction *) JS_GetPrivate(cx, obj);
  if (type->type!=FUNCTIONTYPE)
    return JS_TRUE;

  *rval=BOOLEAN_TO_JSVAL(type->elipsis);
  return JS_TRUE;
}


static JSBool JSX_Type_SetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
  JSX_Type *type = JS_GetPrivate(cx, obj);

  if(JSVAL_IS_INT(id)) {
    int index=JSVAL_TO_INT(id);

    switch(type->type) {
    case FUNCTIONTYPE:
    case STRUCTTYPE:
    case UNIONTYPE:
      return JSX_SetMember(cx, obj, index, *vp);

    case ARRAYTYPE:
    case POINTERTYPE:
      if (index!=0)
	break;

      if (!JSVAL_IS_OBJECT(*vp) || 
	  JSVAL_IS_NULL(*vp) ||
	  !JS_InstanceOf(cx, JSVAL_TO_OBJECT(*vp), &JSX_TypeClass, NULL)) {
	JSX_ReportException(cx, "Wrong type");
	return JS_FALSE;
      }

      ((JSX_TypeArray *) type)->member = (JSX_Type *) JS_GetPrivate(cx, JSVAL_TO_OBJECT(*vp));

      break;
    }
  }

  return JS_TRUE;
}


static char *JSX_intsizenames[]={
  "char",
  "short",
  "int",
  "long",
  "long_long",
  "int64"
};

static char *JSX_signnames[]={
  "unsigned_",
  "signed_",
  ""
};

static char *JSX_floatsizenames[]={
  "float",
  "double",
  "long_double"
};

static char *JSX_othertypenames[]={
  "void"
};

static enum JSX_TypeID JSX_othertypes[]={
  VOIDTYPE
};


static JSBool JSX_InitNamedType(JSContext *cx, JSX_NamedType *dest, JSObject *membertype, int requireName) {
  jsval tmp;
  JS_GetProperty(cx, membertype, "name", &tmp);

  if (tmp==JSVAL_VOID && !requireName) {
    dest->name=0;
  } else {
    if (tmp==JSVAL_VOID || !JSVAL_IS_STRING(tmp)) {
      JSX_ReportException(cx, "Wrong or missing 'name' property in member type object");
      return JS_FALSE;
    }
    dest->name=strdup(JS_GetStringBytes(JSVAL_TO_STRING(tmp)));
  }

  JS_GetProperty(cx, membertype, "type", &tmp);
  if (!JSVAL_IS_OBJECT(tmp) ||
      tmp==JSVAL_NULL ||
      !JS_InstanceOf(cx, JSVAL_TO_OBJECT(tmp), &JSX_TypeClass, NULL)) {
    JSX_ReportException(cx, "Wrong or missing 'type' property in member type object");
    // name is freed later
    return JS_FALSE;
  }

  dest->type=JS_GetPrivate(cx, JSVAL_TO_OBJECT(tmp));

  return JS_TRUE;
}


static void JSX_DestroyTypeStructUnion(JSContext *cx, JSX_TypeStructUnion *type) {
  int i;
  if (type) {
    if (type->member) {
      for (i=0; i<type->nMember; i++) {
	if (type->member[i].name)
	  free(type->member[i].name); // strdup
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
  int i;
  if (type) {
    for (i=0; i<type->nParam; i++) {
      if (type->param[i].name)
	free(type->param[i].name); //strdup
    }
    if (type->param)
      JS_free(cx,type->param);
    if (type->cif.arg_types) {
      JS_free(cx, type->cif.arg_types);
    }
    JS_free(cx, type);
  }
}


static JSBool JSX_NewTypeFunction(JSContext *cx, jsval returnType, jsval params, jsval elipsis, char *callConv, jsval *rval) {
  JSObject *retobj;
  JSObject *paramobj;
  int nParam;
  int i;
  JSX_TypeFunction *type;

  if (!JSVAL_IS_OBJECT(params) ||
      !JS_IsArrayObject(cx, JSVAL_TO_OBJECT(params))) {
    JSX_ReportException(cx, "Invalid params argument");
    return JS_FALSE;
  }

  type = (JSX_TypeFunction *) JS_malloc(cx, sizeof(JSX_TypeFunction));
  type->type=FUNCTIONTYPE;

  retobj=JS_NewObject(cx, &JSX_TypeClass, 0, 0);
  *rval=OBJECT_TO_JSVAL(retobj);

  if (params==JSVAL_VOID)
    nParam=0;
  else {
    paramobj=JSVAL_TO_OBJECT(params);
    if (!JS_GetArrayLength(cx, paramobj, &nParam))
      return JS_FALSE;
  }

  if (callConv &&
      strcmp(callConv, "stdcall")==0)
    type->callConv=STDCALLCONV;
  else
    type->callConv=CDECLCONV;

  type->nParam=nParam;
  type->typeObject=retobj;
  JS_SetPrivate(cx, retobj, type);

  if (elipsis == JSVAL_TRUE)
    type->elipsis=1;
  else
    type->elipsis=0;

  if (JSVAL_IS_OBJECT(returnType) &&
      !JSVAL_IS_NULL(returnType)) {
    JS_DefineProperty(cx, retobj, "returnType", returnType, 0, 0, JSPROP_ENUMERATE | JSPROP_PERMANENT);
    type->returnType=JS_GetPrivate(cx, JSVAL_TO_OBJECT(returnType));
  }

  JS_DefineProperty(cx, retobj, "callConv", JSVAL_VOID, JSX_Type_callConv, 0, JSPROP_ENUMERATE | JSPROP_PERMANENT | JSPROP_READONLY);
  JS_DefineProperty(cx, retobj, "elipsis", JSVAL_VOID, JSX_Type_elipsis, 0, JSPROP_ENUMERATE | JSPROP_PERMANENT | JSPROP_READONLY);

  type->param_capacity=nParam+1;
  type->param = (JSX_ParamType *) JS_malloc(cx, sizeof(JSX_ParamType) * (type->nParam + 1));
  memset(type->param, 0, sizeof(JSX_ParamType) * type->nParam);

  type->param[type->nParam].type=JS_GetPrivate(cx, JSX_GetType(VOIDTYPE,0,0));
  type->param[type->nParam].name=0;
  type->param[type->nParam].isConst=0;
  type->cif.arg_types=0;

  for (i=0; i<nParam; i++) { 
    jsval thisparam;

    JS_GetElement(cx, paramobj, i, &thisparam);
    JSX_SetMember(cx, retobj, i, thisparam);
    JS_DefineElement(cx, retobj, i, thisparam, 0, 0, JSPROP_ENUMERATE | JSPROP_PERMANENT);
  }

  return JS_TRUE;
}


JSClass *JSX_GetTypeClass(void) {
  return &JSX_TypeClass;
}


JSBool JSX_InitMemberType(JSContext *cx, JSX_MemberType *dest, JSObject *membertype) {
  if(!JSX_InitNamedType(cx, (JSX_NamedType *) dest, membertype, 1)) return JS_FALSE;
  return JS_TRUE;
}


JSBool JSX_InitParamType(JSContext *cx, JSX_ParamType *dest, JSObject *membertype) {
  if(!JSX_InitNamedType(cx, (JSX_NamedType *) dest, membertype, 0)) return JS_FALSE;
  jsval tmp;
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


int JSX_TypeAlign(JSX_Type *type) {
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
    return ((JSX_TypeInt *)type)->ffiType.alignment;
  case FLOATTYPE:
    return ((JSX_TypeFloat *) type)->ffiType.alignment;
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
    return ((JSX_TypeInt *)type)->ffiType.size;
  case FLOATTYPE:
    return ((JSX_TypeFloat *) type)->ffiType.size;
  case STRUCTTYPE:
  case UNIONTYPE:
    align=JSX_TypeAlign(type);
    return (((((JSX_TypeStructUnion *) type)->sizeOf + 7) / 8 + align - 1) / align) * align;
  default: // VOIDTYPE, FUNCTIONTYPE
    return 0; // Error
  }
}


static JSBool JSX_SetMember(JSContext *cx, JSObject *obj, int memberno, jsval member) {
  JSX_TypeStructUnion *type; // Same as TypeFunction up to params.
  int i;
  int thisalign;
  int thissize;
  
  type=JS_GetPrivate(cx, obj);
  if (!type) {
    JSX_ReportException(cx, "Uninitialized type object");
    return JS_FALSE;
  }

  if (type->member_capacity <= memberno + (type->type == FUNCTIONTYPE ? 1 : 0)) {
    int old_capacity=type->member_capacity;

    if (type->member_capacity==0)
      type->member_capacity=8;
    else
      type->member_capacity*=2;

    if (type->member_capacity < memberno + (type->type == FUNCTIONTYPE ? 2 : 1))
      type->member_capacity=memberno + (type->type == FUNCTIONTYPE ? 2 : 1);

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

  if (!JSVAL_IS_OBJECT(member) || 
	JSVAL_IS_NULL(member))
    goto failure;
  
  switch(type->type) {
  case FUNCTIONTYPE:
    if(!JSX_InitParamType(cx, ((JSX_TypeFunction *) type)->param + memberno, JSVAL_TO_OBJECT(member)))
      goto failure;
    if (memberno==type->nMember-1) {
      ((JSX_TypeFunction *) type)->param[type->nMember].type = JS_GetPrivate(cx, JSX_GetType(VOIDTYPE, 0, 0));
      ((JSX_TypeFunction *) type)->param[type->nMember].name = 0;
      ((JSX_TypeFunction *) type)->param[type->nMember].isConst = 0;
    }
    if(((JSX_TypeFunction *) type)->cif.arg_types) {
      JS_free(cx, ((JSX_TypeFunction *) type)->cif.arg_types);
      ((JSX_TypeFunction *) type)->cif.arg_types=0;
    }
    break;

  case UNIONTYPE:
    if (!JSX_InitMemberType(cx, type->member+memberno, JSVAL_TO_OBJECT(member)))
      goto failure;

    type->member[memberno].offset=0;
    thissize=JSX_TypeSizeBits(type->member[memberno].type);
    if (thissize > type->sizeOf)
      type->sizeOf=thissize;

    JS_DefineProperty(cx, JSVAL_TO_OBJECT(member), "offset", INT_TO_JSVAL(0), 0, 0, JSPROP_READONLY | JSPROP_PERMANENT);

    break;
    
  case STRUCTTYPE:
    if (!JSX_InitMemberType(cx, type->member+memberno, JSVAL_TO_OBJECT(member)))
      goto failure;

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

    break;
  }

  return JS_TRUE;

 failure:
  return JS_FALSE;
}


JSBool JSX_NewTypeStructUnion(JSContext *cx, int nMember, jsval *member, jsval *rval, int isStruct) {
  JSObject *retobj;
  JSX_TypeStructUnion *type;
  int i;
  
  retobj=JS_NewObject(cx, &JSX_TypeClass, 0, 0);
  *rval=OBJECT_TO_JSVAL(retobj);

  type = (JSX_TypeStructUnion *) JS_malloc(cx, sizeof(JSX_TypeStructUnion));

  type->type=isStruct?STRUCTTYPE:UNIONTYPE;
  type->nMember=nMember;
  type->member_capacity=nMember;
  type->member = nMember ? (JSX_MemberType *) JS_malloc(cx, sizeof(JSX_MemberType) * nMember) : 0;
  memset(type->member, 0, sizeof(JSX_MemberType) * nMember);
  type->sizeOf=0;
  JS_SetPrivate(cx, retobj, type);
  type->typeObject=retobj;
  type->ffiType.elements=0;

  for (i=0; i<nMember; i++) {
    if (!JSX_SetMember(cx, retobj, i, member[i]))
      goto failure;
    JS_DefineElement(cx, retobj, i, member[i], 0, 0, JSPROP_ENUMERATE | JSPROP_PERMANENT);
  }
  
  return JS_TRUE;

 failure:
  JSX_DestroyTypeStructUnion(cx, type);
  return JS_FALSE;
}


JSBool JSX_NewTypePointer(JSContext *cx, jsval direct, jsval *rval) {
  JSObject *retobj;
  JSX_TypePointer *type;

  retobj=JS_NewObject(cx, &JSX_TypeClass, 0, 0);
  *rval=OBJECT_TO_JSVAL(retobj);

  type = (JSX_TypePointer *) JS_malloc(cx, sizeof(JSX_TypePointer));

  type->type=POINTERTYPE;
  JS_SetPrivate(cx, retobj, type);

  type->direct=JS_GetPrivate(cx, JSX_GetType(VOIDTYPE,0,0));
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


JSBool JSX_NewTypeArray(JSContext *cx, jsval member, jsval len, jsval *rval) {
  JSObject *retobj;
  JSX_TypeArray *type;

  retobj=JS_NewObject(cx, &JSX_TypeClass, 0, 0);
  *rval=OBJECT_TO_JSVAL(retobj);

  type = (JSX_TypeArray *) JS_malloc(cx, sizeof(JSX_TypeArray));

  type->type=ARRAYTYPE;
  JS_SetPrivate(cx, retobj, type);

  if (JSVAL_IS_INT(len)) 
    type->length=JSVAL_TO_INT(len);
  else
    type->length=0;

  JS_DefineProperty(cx, retobj, "length", JSVAL_VOID, JSX_Type_length, 0, JSPROP_ENUMERATE | JSPROP_PERMANENT | JSPROP_READONLY);
  JS_DefineElement(cx, retobj, 0, member, 0, 0, JSPROP_ENUMERATE | JSPROP_PERMANENT);

  type->member=JS_GetPrivate(cx, JSX_GetType(VOIDTYPE,0,0));
  type->typeObject=retobj;

  if (member==JSVAL_VOID) {
    return JS_TRUE;
  }

  if (!JSVAL_IS_OBJECT(member) || 
      JSVAL_IS_NULL(member) ||
      !JS_InstanceOf(cx, JSVAL_TO_OBJECT(member), &JSX_TypeClass, NULL) ||
      !JSVAL_IS_INT(len)) {
    JSX_ReportException(cx, "Wrong type to Type.array");
    goto failure;
  }
    
  type->member = (JSX_Type *) JS_GetPrivate(cx, JSVAL_TO_OBJECT(member));

  return JS_TRUE;

 failure:
  if (type)
    JS_free(cx, type);
  return JS_FALSE;
}


JSBool JSX_NewTypeBitfield(JSContext *cx, jsval member, jsval len, jsval *rval) {
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

  JS_DefineProperty(cx, retobj, "length", JSVAL_VOID, JSX_Type_length, 0, JSPROP_ENUMERATE | JSPROP_PERMANENT);
  JS_DefineElement(cx, retobj, 0, member, 0, 0, JSPROP_ENUMERATE | JSPROP_PERMANENT);

  type->member=JS_GetPrivate(cx, JSX_GetType(VOIDTYPE,0,0));
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


JSObject *JSX_GetType(enum JSX_TypeID type, int size, int signedness) {
  return jsx_Type_objects[type][size][signedness];
}


static void init_int_types(JSContext *cx, JSObject *typeobj) {
  int size;
  int signedness;
  int inttype;

  ffi_type ffitypes[3][6] = {
    { ffi_type_uchar, ffi_type_ushort, ffi_type_uint, ffi_type_ulong, ffi_type_uint64, ffi_type_uint64 },
    { ffi_type_schar, ffi_type_sshort, ffi_type_sint, ffi_type_slong, ffi_type_sint64, ffi_type_sint64 },
    { ffi_type_schar, ffi_type_sshort, ffi_type_sint, ffi_type_slong, ffi_type_sint64, ffi_type_sint64 }
  };

  for (inttype=INTTYPE; inttype<=UINTTYPE; inttype++) {
    for (size=0; size<6; size++) {
      for (signedness=0; signedness<3; signedness++) {
	JSObject *newtype;
	char name[80];
	jsval newval;

	JSX_TypeInt *type;
	
	sprintf(name, "%s%s", JSX_signnames[signedness], JSX_intsizenames[size]);
	newtype=JS_NewObject(cx, &JSX_TypeClass, 0, 0);
	newval=OBJECT_TO_JSVAL(newtype);
	
	JS_SetProperty(cx, typeobj, name, &newval);
	jsx_Type_objects[inttype][size][signedness]=newtype;
	
	type = (JSX_TypeInt *) JS_malloc(cx, sizeof(JSX_TypeInt));
	type->type=inttype;
	type->size=size;
	type->signedness=signedness;
	type->ffiType=ffitypes[signedness][size];
	
	type->typeObject=newtype;
	JS_SetPrivate(cx, newtype, type);
      }
    }
  }
}


static void init_float_types(JSContext *cx, JSObject *typeobj) {
  int size;

  ffi_type ffitypes[3]={
    ffi_type_float,
    ffi_type_double,
    ffi_type_longdouble
  };

  for (size=0; size<3; size++) {
    JSObject *newtype;
    jsval newval;
    JSX_TypeFloat *type;
  
    newtype=JS_NewObject(cx, &JSX_TypeClass, 0, 0);
    newval=OBJECT_TO_JSVAL(newtype);
  
    JS_SetProperty(cx, typeobj, JSX_floatsizenames[size], &newval);
    jsx_Type_objects[FLOATTYPE][size][0]=newtype;

    type = (JSX_TypeFloat *) JS_malloc(cx, sizeof(JSX_TypeFloat));
    type->type=FLOATTYPE;
    type->size=size;
    type->ffiType=ffitypes[size];

    type->typeObject=newtype;
    JS_SetPrivate(cx, newtype, type);
  }
}


static void init_other_types(JSContext *cx, JSObject *typeobj) {
  int ot;

  for (ot=0; ot<1; ot++) {
    JSObject *newtype;
    jsval newval;
    JSX_Type *type;
  
    newtype=JS_NewObject(cx, &JSX_TypeClass, 0, 0);
    newval=OBJECT_TO_JSVAL(newtype);
  
    JS_SetProperty(cx, typeobj, JSX_othertypenames[ot], &newval);
    jsx_Type_objects[JSX_othertypes[ot]][0][0]=newtype;

    type = (JSX_Type *) JS_malloc(cx, sizeof(JSX_Type));
    type->type=JSX_othertypes[ot];

    type->typeObject=newtype;
    JS_SetPrivate(cx, newtype, type);
  }
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


static JSBool JSX_Type_array(JSContext *cx,  JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
  return JSX_NewTypeArray(cx, argc > 0 ? argv[0] : JSVAL_VOID, argc > 1 ? argv[1] : JSVAL_VOID, rval);
}


static JSBool JSX_Type_bitfield(JSContext *cx,  JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
  return JSX_NewTypeBitfield(cx, argc > 0 ? argv[0] : JSVAL_VOID, argc > 1 ? argv[1] : JSVAL_VOID, rval);
}


static JSBool JSX_Type_pointer(JSContext *cx,  JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
  return JSX_NewTypePointer(cx, argc > 0 ? argv[0] : JSVAL_VOID, rval);
}


static JSBool JSX_Type_struct(JSContext *cx,  JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
  return JSX_NewTypeStructUnion(cx, argc, argv, rval, 1);
}


static JSBool JSX_Type_union(JSContext *cx,  JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
  return JSX_NewTypeStructUnion(cx, argc, argv, rval, 0);
}


static JSBool JSX_Type_function(JSContext *cx,  JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
  char *callConv=0;
  JSBool ok;

  if (argc>3 && 
      JSVAL_IS_STRING(argv[3]))
    callConv=JS_GetStringBytes(JSVAL_TO_STRING(argv[3]));

  ok=JSX_NewTypeFunction(cx, argc>1?argv[0]:JSVAL_VOID, argc>1?argv[1]:JSVAL_VOID, argc>2?argv[2]:JSVAL_VOID, callConv, rval);

  return ok;
}


static JSBool JSX_Type_toString(JSContext *cx,  JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
  JSX_Type *type;
  char *name;
  char namebuf[80];

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
  int size;

  size = JSX_TypeSize((JSX_Type *) JS_GetPrivate(cx, obj));
  if (!size)
    *rval=JSVAL_VOID;
  else
    *rval=INT_TO_JSVAL(size);

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

    if (JSVAL_IS_OBJECT(*vp) &&
	*vp!=JSVAL_NULL &&
        JS_InstanceOf(cx, JSVAL_TO_OBJECT(*vp), JSX_GetTypeClass(), NULL)) {
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
	int jstype=JSX_JSType(cx, *vp);

	switch(jstype) {
	case JSVAL_STRING:
	  *(arg_types++)=&ffi_type_pointer;
	  break;
	case JSVAL_INT:
	  *(arg_types++)=&ffi_type_sint;
	  break;
	case JSVAL_DOUBLE:
	  *(arg_types++)=&ffi_type_double;
	  break;
	}
      }
    } else {
      siz=JSX_TypeSize(thistype);
      if (arg_types) {
	*(arg_types++)=JSX_GetFFIType(cx, thistype);
      }
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


int JSX_JSType(JSContext *cx, jsval v) {
  int jstype;

  // Determine the JS type to an appropriate level of detail for the big 2D switch

  jstype=JSVAL_TAG(v);
  switch(jstype) {
  case JSVAL_OBJECT:
    if (v==JSVAL_NULL) {
      jstype=JSNULL;
      break;
    }
    if (JS_IsArrayObject(cx, JSVAL_TO_OBJECT(v))) {
      jstype=JSARRAY;
      break;
    }
    if (JS_InstanceOf(cx, JSVAL_TO_OBJECT(v), JSX_GetPointerClass(), NULL)) {
      jstype=JSPOINTER;
      break;
    }
    if (JS_InstanceOf(cx, JSVAL_TO_OBJECT(v), JSX_GetTypeClass(), NULL)) {
      jstype=JSTYPE;
      break;
    }
    if (JS_ObjectIsFunction(cx, JSVAL_TO_OBJECT(v))) {
      jstype=JSFUNC;
      break;
    }

    break;
  case 1:
  case 3:
  case 5:
  case 7:
    if (v==JSVAL_VOID)
      jstype=JSVOID;
    else
      jstype=JSVAL_INT;

    break;
  }

  return jstype;
}


int JSX_CType(JSX_Type *type) {
  int Ctype;

  // Determine the C type to an appropriate level of detail for the big 2D switch

  if (type==NULL)
    Ctype=UNDEFTYPE;
  else {
    Ctype=type->type;

    switch(Ctype) {
    case INTTYPE:
      if(((JSX_TypeInt *) type)->signedness == 0) Ctype = UINTTYPE;
      break;
    case ARRAYTYPE:
      if((((JSX_TypeArray *) type)->member->type == INTTYPE || ((JSX_TypeArray *) type)->member->type == UINTTYPE) &&
	  ((JSX_TypeInt *) ((JSX_TypeArray *) type)->member)->size == 0)
	Ctype=ACHARTYPE;
      else if((((JSX_TypeArray *) type)->member->type == INTTYPE || ((JSX_TypeArray *) type)->member->type == UINTTYPE) &&
	       ((JSX_TypeInt *) ((JSX_TypeArray *) type)->member)->size == 1)
	Ctype=ASHORTTYPE;
      break;
    case POINTERTYPE:
      if((((JSX_TypePointer *) type)->direct->type == INTTYPE || ((JSX_TypePointer *) type)->direct->type == UINTTYPE) &&
	  ((JSX_TypeInt *) ((JSX_TypePointer *) type)->direct)->size == 0)
	Ctype=PCHARTYPE;
      else if((((JSX_TypePointer *) type)->direct->type == INTTYPE || ((JSX_TypePointer *) type)->direct->type == UINTTYPE) &&
	       ((JSX_TypeInt *) ((JSX_TypePointer *) type)->direct)->size == 1)
	Ctype=PSHORTTYPE;
      break;
    }
  }

  return Ctype;
}


JSBool JSX_TypeContainsPointer(JSX_Type *type) {
  int i;

  for (;;) {
    switch (type->type) {

    case POINTERTYPE:
      return JS_TRUE;

    case ARRAYTYPE:
      type = ((JSX_TypeArray *) type)->member;
      break;

    case UNIONTYPE:
    case STRUCTTYPE:
      for(i = 0; i < ((JSX_TypeStructUnion *) type)->nMember; i++)
	if(JSX_TypeContainsPointer(((JSX_TypeStructUnion *) type)->member[i].type))
	  return JS_TRUE;

      // fall through

    default:
      return JS_FALSE;
    }
  }
}


jsval JSX_make_Type(JSContext *cx, JSObject *obj) {
  JSObject *typeobj;
  JSObject *protoobj;

  static struct JSFunctionSpec staticfunc[]={
    {"array",JSX_Type_array,1,0,0},
    {"bitfield",JSX_Type_bitfield,1,0,0},
    {"pointer",JSX_Type_pointer,1,0,0},
    {"struct",JSX_Type_struct,1,0,0},
    {"union",JSX_Type_union,1,0,0},
    {"function",JSX_Type_function,3,0,0},
    {0,0,0,0,0}
  };

  static struct JSFunctionSpec memberfunc[]={
    {"toString",JSX_Type_toString,0,0,0},
    {0,0,0,0,0}
  };

  static struct JSPropertySpec memberprop[]={
    {"sizeof",0,JSPROP_READONLY | JSPROP_PERMANENT, JSX_Type_sizeof,0},
    {"length",0,JSPROP_READONLY | JSPROP_PERMANENT, JSX_Type_length,0},
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
