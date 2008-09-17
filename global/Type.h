#ifndef __Type_h
#define __Type_h

#include <jsapi.h>
#include <ffi.h>

#ifdef _WIN32
# ifdef TYPE_EXPORTS
#  define TYPE_API __declspec(dllexport)
# else
#  define TYPE_API __declspec(dllimport)
# endif
#else
# define TYPE_API
#endif


#define ALIGN_TYPE int
#define PARAMALIGN sizeof(int)

enum JSX_TypeID {
  INTTYPE,
  UINTTYPE,
  FLOATTYPE,
  FUNCTIONTYPE,
  STRUCTTYPE,
  UNIONTYPE,
  VOIDTYPE,
  POINTERTYPE,
  ARRAYTYPE,
  BITFIELDTYPE,
  TYPECOUNT
};

#define PCHARTYPE (TYPECOUNT+0)
#define PSHORTTYPE (TYPECOUNT+1)
#define ACHARTYPE (TYPECOUNT+2)
#define ASHORTTYPE (TYPECOUNT+3)
#define UNDEFTYPE (TYPECOUNT+4)
#define TYPECOUNT2 (TYPECOUNT+5)

enum JSX_CallConv {
  CDECLCONV,
  STDCALLCONV
};

struct JSX_Type { // Private part of Type objects
  enum JSX_TypeID type; // VOID
  JSObject *typeObject;
};

struct JSX_TypeInt {
  enum JSX_TypeID type; // INTTYPE
  JSObject *typeObject;
  int size;
  int signedness;
  ffi_type ffiType;
};

struct JSX_TypeFloat {
  enum JSX_TypeID type; // FLOATTYPE
  JSObject *typeObject;
  int size;
  ffi_type ffiType;
};

struct JSX_NamedType {
  struct JSX_Type *type;
  char *name;
};

struct JSX_ParamType { // inherits NamedType
  struct JSX_Type *type;
  char *name;
  int isConst;
};

struct JSX_TypeFunction {
  enum JSX_TypeID type; // FUNCTIONTYPE
  JSObject *typeObject;
  struct JSX_ParamType *param;
  int nParam;
  int param_capacity;
  enum JSX_CallConv callConv;
  struct JSX_Type *returnType;
  int elipsis;
  ffi_cif cif;
};

struct JSX_MemberType { // inherits NamedType
  struct JSX_Type *type;
  char *name;
  int offset; // in bits
};

struct JSX_TypeStructUnion {
  enum JSX_TypeID type; // STRUCTTYPE or UNIONTYPE
  JSObject *typeObject;
  struct JSX_MemberType *member;
  int nMember;
  int member_capacity;
  int sizeOf; // in bits
  ffi_type ffiType;
};

struct JSX_TypePointer {
  enum JSX_TypeID type; // POINTERTYPE
  JSObject *typeObject;
  struct JSX_Type *direct;
};

struct JSX_TypeArray {
  enum JSX_TypeID type; // ARRAYTYPE
  JSObject *typeObject;
  struct JSX_Type *member;
  int length;
};

struct JSX_TypeBitfield {
  enum JSX_TypeID type; // BITFIELDTYPE
  JSObject *typeObject;
  struct JSX_Type *member;
  int length;
};



TYPE_API int JSX_TypeSize(struct JSX_Type *type);
TYPE_API int JSX_TypeAlign(struct JSX_Type *type);

TYPE_API int JSX_TypeSize_multi(JSContext *cx, uintN nargs, struct JSX_ParamType *type, jsval *vp, ffi_type **arg_types);
TYPE_API JSClass *JSX_GetTypeClass(void);

TYPE_API int JSX_CType(struct JSX_Type *type);
TYPE_API int JSX_JSType(JSContext *cx, jsval rval);
TYPE_API JSBool JSX_TypeContainsPointer(struct JSX_Type *type);
TYPE_API JSObject *JSX_GetType(enum JSX_TypeID, int size, int signedness);
TYPE_API ffi_type *JSX_GetFFIType(JSContext *cx, struct JSX_Type *type);
TYPE_API ffi_cif *JSX_GetCIF(JSContext *cx, struct JSX_TypeFunction *type);

#ifdef _WIN32
TYPE_API JSBool
JSX_init_Type(JSContext *cx,  JSObject *obj, int argc, jsval *argv, jsval *rval);
#endif

#define JSNULL (JSVAL_TAGMASK+1) // because JSVAL_NULL == JSVAL_OBJECT
#define JSVOID (JSVAL_TAGMASK+2)
#define JSPOINTER (JSVAL_TAGMASK+3)
#define JSTYPE (JSVAL_TAGMASK+4)
#define JSARRAY (JSVAL_TAGMASK+5)
#define JSFUNC (JSVAL_TAGMASK+6)

#define TYPEPAIR(a,b) ((TYPECOUNT2 * (a)) + (b))

#define Functiontype ((struct JSX_TypeFunction *)type)
#define Arraytype ((struct JSX_TypeArray *)type)
#define Bitfieldtype ((struct JSX_TypeBitfield *)type)
#define Pointertype ((struct JSX_TypePointer *)type)
#define Inttype ((struct JSX_TypeInt *)type)
#define Floattype ((struct JSX_TypeFloat *)type)
#define StructUniontype ((struct JSX_TypeStructUnion *)type)

#endif

/*
  Used by struct, union and function creators, which are used by deserialize and C.
 */
