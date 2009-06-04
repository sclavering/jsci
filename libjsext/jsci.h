/*
jsci.h

Our JavaScript <-> C interface, using libffi.
*/

#ifndef __jsci_h
#define __jsci_h

#include <ffi.h>
#include "util.h"


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

#define UNDEFTYPE (TYPECOUNT+4)
#define TYPECOUNT2 (TYPECOUNT+5)

// We store an instance of this, or a subclass, inside each js Type object
struct JSX_Type {
  enum JSX_TypeID type;

  virtual ffi_type *GetFFIType(JSContext *cx) {
    return 0;
  }
};

struct JSX_TypeVoid : JSX_Type {
  // VOIDTYPE
  virtual ffi_type *GetFFIType(JSContext *cx);
};

struct JSX_TypeNumeric : JSX_Type {
  // INTTYPE, UINTTYPE, or FLOATTYPE
  int size;
  ffi_type ffiType;

  virtual ffi_type *GetFFIType(JSContext *cx);
};

typedef struct {
  JSX_Type *paramtype;
  int isConst;
} JSX_FuncParam;

struct JSX_TypeFunction : JSX_Type {
  // FUNCTIONTYPE
  JSX_FuncParam *param;
  int nParam;
  JSX_Type *returnType;
  ffi_cif cif;
};

typedef struct {
  JSX_Type *membertype;
  char *name;
  int offset; // in bits
} JSX_SuMember;

struct JSX_TypeStructUnion : JSX_Type {
  // STRUCTTYPE or UNIONTYPE
  JSX_SuMember *member;
  int nMember;
  int sizeOf; // in bits
  ffi_type ffiType;

  virtual ffi_type *GetFFIType(JSContext *cx);
};

struct JSX_TypePointer : JSX_Type {
  // POINTERTYPE
  JSX_Type *direct;

  virtual ffi_type *GetFFIType(JSContext *cx);
};

struct JSX_TypeArray : JSX_Type {
  // ARRAYTYPE
  JSX_Type *member;
  int length;
};

struct JSX_TypeBitfield : JSX_Type {
  // BITFIELDTYPE
  JSX_Type *member;
  int length;
};



int JSX_TypeSize(JSX_Type *type);
int JSX_TypeSize_multi(JSContext *cx, uintN nargs, JSX_FuncParam *type, jsval *vp, ffi_type **arg_types);
JSClass *JSX_GetTypeClass(void);

int JSX_CType(JSX_Type *type);
int JSX_JSType(JSContext *cx, jsval rval);
JSBool JSX_TypeContainsPointer(JSX_Type *type);
JSX_Type *GetVoidType(void); // the C "void" type
ffi_cif *JSX_GetCIF(JSContext *cx, JSX_TypeFunction *type);
int JSX_TypeAlign(JSX_Type *type);

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


static inline int type_is_char(JSX_Type *t) {
  return (t->type == INTTYPE || t->type == UINTTYPE) && 0 == ((JSX_TypeNumeric *) t)->size;
}


static inline int is_void_or_char(JSX_Type *t) {
  return t->type == VOIDTYPE || type_is_char(t);
}

#endif
