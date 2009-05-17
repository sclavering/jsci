#ifndef __Type_h
#define __Type_h

#include <jsapi.h>
#include <ffi.h>


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
typedef struct JSX_Type JSX_Type;

struct JSX_TypeInt {
  enum JSX_TypeID type; // INTTYPE
  JSObject *typeObject;
  int size;
  int signedness;
  ffi_type ffiType;
};
typedef struct JSX_TypeInt JSX_TypeInt;

struct JSX_TypeFloat {
  enum JSX_TypeID type; // FLOATTYPE
  JSObject *typeObject;
  int size;
  ffi_type ffiType;
};
typedef struct JSX_TypeFloat JSX_TypeFloat;

struct JSX_NamedType {
  JSX_Type *type;
  char *name;
};

struct JSX_ParamType { // inherits NamedType
  JSX_Type *type;
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
  JSX_Type *returnType;
  int elipsis;
  ffi_cif cif;
};
typedef struct JSX_TypeFunction JSX_TypeFunction;

struct JSX_MemberType { // inherits NamedType
  JSX_Type *type;
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
typedef struct JSX_TypeStructUnion JSX_TypeStructUnion;

struct JSX_TypePointer {
  enum JSX_TypeID type; // POINTERTYPE
  JSObject *typeObject;
  JSX_Type *direct;
};
typedef struct JSX_TypePointer JSX_TypePointer;

struct JSX_TypeArray {
  enum JSX_TypeID type; // ARRAYTYPE
  JSObject *typeObject;
  JSX_Type *member;
  int length;
};
typedef struct JSX_TypeArray JSX_TypeArray;

struct JSX_TypeBitfield {
  enum JSX_TypeID type; // BITFIELDTYPE
  JSObject *typeObject;
  JSX_Type *member;
  int length;
};
typedef struct JSX_TypeBitfield JSX_TypeBitfield;



int JSX_TypeSize(JSX_Type *type);
int JSX_TypeSize_multi(JSContext *cx, uintN nargs, struct JSX_ParamType *type, jsval *vp, ffi_type **arg_types);
JSClass *JSX_GetTypeClass(void);

int JSX_CType(JSX_Type *type);
int JSX_JSType(JSContext *cx, jsval rval);
JSBool JSX_TypeContainsPointer(JSX_Type *type);
JSObject *JSX_GetType(enum JSX_TypeID, int size, int signedness);
ffi_type *JSX_GetFFIType(JSContext *cx, JSX_Type *type);
ffi_cif *JSX_GetCIF(JSContext *cx, JSX_TypeFunction *type);

#define JSNULL (JSVAL_TAGMASK+1) // because JSVAL_NULL == JSVAL_OBJECT
#define JSVOID (JSVAL_TAGMASK+2)
#define JSPOINTER (JSVAL_TAGMASK+3)
#define JSTYPE (JSVAL_TAGMASK+4)
#define JSARRAY (JSVAL_TAGMASK+5)
#define JSFUNC (JSVAL_TAGMASK+6)

#define TYPEPAIR(a,b) ((TYPECOUNT2 * (a)) + (b))

#endif
