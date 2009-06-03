/*
jsci.h

Our JavaScript <-> C interface, using libffi.
*/

#ifndef __jsci_h
#define __jsci_h

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
#define ACHARTYPE (TYPECOUNT+2)
#define UNDEFTYPE (TYPECOUNT+4)
#define TYPECOUNT2 (TYPECOUNT+5)

typedef struct { // Private part of Type objects
  enum JSX_TypeID type; // VOID
} JSX_Type;

typedef struct {
  enum JSX_TypeID type; // INTTYPE, UINTTYPE, or FLOATTYPE
  int size;
  ffi_type ffiType;
} JSX_TypeNumeric;

typedef struct {
  JSX_Type *paramtype;
  int isConst;
} JSX_FuncParam;

typedef struct {
  enum JSX_TypeID type; // FUNCTIONTYPE
  JSX_FuncParam *param;
  int nParam;
  JSX_Type *returnType;
  ffi_cif cif;
} JSX_TypeFunction;

typedef struct {
  JSX_Type *membertype;
  char *name;
  int offset; // in bits
} JSX_SuMember;

typedef struct {
  enum JSX_TypeID type; // STRUCTTYPE or UNIONTYPE
  JSX_SuMember *member;
  int nMember;
  int member_capacity;
  int sizeOf; // in bits
  ffi_type ffiType;
} JSX_TypeStructUnion;

typedef struct {
  enum JSX_TypeID type; // POINTERTYPE
  JSX_Type *direct;
} JSX_TypePointer;

typedef struct {
  enum JSX_TypeID type; // ARRAYTYPE
  JSX_Type *member;
  int length;
} JSX_TypeArray;

typedef struct {
  enum JSX_TypeID type; // BITFIELDTYPE
  JSX_Type *member;
  int length;
} JSX_TypeBitfield;



int JSX_TypeSize(JSX_Type *type);
int JSX_TypeSize_multi(JSContext *cx, uintN nargs, JSX_FuncParam *type, jsval *vp, ffi_type **arg_types);
JSClass *JSX_GetTypeClass(void);

int JSX_CType(JSX_Type *type);
int JSX_JSType(JSContext *cx, jsval rval);
JSBool JSX_TypeContainsPointer(JSX_Type *type);
JSX_Type *GetVoidType(void); // the C "void" type
ffi_type *JSX_GetFFIType(JSContext *cx, JSX_Type *type);
ffi_cif *JSX_GetCIF(JSContext *cx, JSX_TypeFunction *type);

#define JSNULL (JSVAL_TAGMASK+1) // because JSVAL_NULL == JSVAL_OBJECT
#define JSVOID (JSVAL_TAGMASK+2)
#define JSPOINTER (JSVAL_TAGMASK+3)
#define JSARRAY (JSVAL_TAGMASK+5)
#define JSFUNC (JSVAL_TAGMASK+6)

#define TYPEPAIR(a,b) ((TYPECOUNT2 * (a)) + (b))


JSClass * JSX_GetPointerClass(void);
int JSX_Get(JSContext *cx, char *p, char *oldptr, int do_clean, JSX_Type *type, jsval *rval);
JSBool JSX_InitPointer(JSContext *cx, JSObject *retobj, JSObject *typeobj);

typedef struct {
  void *ptr; // 0 means unresolved. NULL pointer is repr by null value.
  JSX_Type *type;
  void (*finalize) (void *);
} JSX_Pointer;

#endif
