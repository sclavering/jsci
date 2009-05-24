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

typedef struct { // Private part of Type objects
  enum JSX_TypeID type; // VOID
  JSObject *typeObject;
} JSX_Type;

typedef struct {
  enum JSX_TypeID type; // INTTYPE
  JSObject *typeObject;
  int size;
  ffi_type ffiType;
} JSX_TypeInt;

typedef struct {
  enum JSX_TypeID type; // FLOATTYPE
  JSObject *typeObject;
  int size;
  ffi_type ffiType;
} JSX_TypeFloat;

typedef struct {
  JSX_Type *type;
  int isConst;
} JSX_ParamType;

typedef struct {
  enum JSX_TypeID type; // FUNCTIONTYPE
  JSObject *typeObject;
  JSX_ParamType *param;
  int nParam;
  enum JSX_CallConv callConv;
  JSX_Type *returnType;
  int elipsis;
  ffi_cif cif;
} JSX_TypeFunction;

typedef struct {
  JSX_Type *type;
  char *name;
  int offset; // in bits
} JSX_MemberType;

typedef struct {
  enum JSX_TypeID type; // STRUCTTYPE or UNIONTYPE
  JSObject *typeObject;
  JSX_MemberType *member;
  int nMember;
  int member_capacity;
  int sizeOf; // in bits
  ffi_type ffiType;
} JSX_TypeStructUnion;

typedef struct {
  enum JSX_TypeID type; // POINTERTYPE
  JSObject *typeObject;
  JSX_Type *direct;
} JSX_TypePointer;

typedef struct {
  enum JSX_TypeID type; // ARRAYTYPE
  JSObject *typeObject;
  JSX_Type *member;
  int length;
} JSX_TypeArray;

typedef struct {
  enum JSX_TypeID type; // BITFIELDTYPE
  JSObject *typeObject;
  JSX_Type *member;
  int length;
} JSX_TypeBitfield;



int JSX_TypeSize(JSX_Type *type);
int JSX_TypeSize_multi(JSContext *cx, uintN nargs, JSX_ParamType *type, jsval *vp, ffi_type **arg_types);
JSClass *JSX_GetTypeClass(void);

int JSX_CType(JSX_Type *type);
int JSX_JSType(JSContext *cx, jsval rval);
JSBool JSX_TypeContainsPointer(JSX_Type *type);
JSObject *JSX_GetCharType(void); // return a Type instance for a 1-byte integer
JSObject *JSX_GetVoidType(void); // return a Type instance for the C "void" type
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
