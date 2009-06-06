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

  virtual ~JSX_Type();

  virtual ffi_type *GetFFIType();
  virtual int SizeInBits();
  virtual int SizeInBytes();
  virtual int AlignmentInBits();
  virtual int AlignmentInBytes();
  virtual JSBool ContainsPointer();
};

struct JSX_TypeVoid : JSX_Type {
  // VOIDTYPE
  ffi_type *GetFFIType();
};

struct JSX_TypeNumeric : JSX_Type {
  // INTTYPE, UINTTYPE, or FLOATTYPE
  int size;
  ffi_type ffiType;

  ffi_type *GetFFIType();
  int SizeInBytes();
  int AlignmentInBytes();
};

struct JSX_FuncParam {
  JSX_Type *paramtype;
  int isConst;

  JSX_FuncParam() : paramtype(0), isConst(0) {}
};

struct JSX_TypeFunction : JSX_Type {
  // FUNCTIONTYPE
  JSX_FuncParam *param;
  int nParam;
  JSX_Type *returnType;
  ffi_cif cif;

  ~JSX_TypeFunction();

  ffi_cif *GetCIF();
  int ParamSizes(JSContext *cx, uintN nargs, jsval *vp, ffi_type **arg_types);
};

struct JSX_SuMember {
  JSX_Type *membertype;
  char *name;
  int offset; // in bits

  JSX_SuMember() : membertype(0), name(0), offset(0) {}
};

struct JSX_TypeStructUnion : JSX_Type {
  // STRUCTTYPE or UNIONTYPE
  JSX_SuMember *member;
  int nMember;
  int sizeOf; // in bits
  ffi_type ffiType;

  ~JSX_TypeStructUnion();

  int SizeInBytes();
  int AlignmentInBytes();
  JSBool ContainsPointer();

  virtual JSBool SetSizeAndAligments(JSContext *cx) = 0;
  JSBool ReplaceMembers(JSContext *cx, JSObject *obj, int nMember, jsval *members);
};

struct JSX_TypeStruct : JSX_TypeStructUnion {
  ffi_type *GetFFIType();
  JSBool SetSizeAndAligments(JSContext *cx);
};

struct JSX_TypeUnion : JSX_TypeStructUnion {
  JSBool SetSizeAndAligments(JSContext *cx);
};

struct JSX_TypePointer : JSX_Type {
  // POINTERTYPE
  JSX_Type *direct;

  ffi_type *GetFFIType();
  int SizeInBytes();
  int AlignmentInBytes();
  JSBool ContainsPointer();
};

struct JSX_TypeArray : JSX_Type {
  // ARRAYTYPE
  JSX_Type *member;
  int length;

  int SizeInBytes();
  int AlignmentInBytes();
  JSBool ContainsPointer();
};

struct JSX_TypeBitfield : JSX_Type {
  // BITFIELDTYPE
  JSX_Type *member;
  int length;

  int SizeInBits();
  int AlignmentInBits() { return 1; }
  int AlignmentInBytes();
};


extern const char *JSX_typenames[];
extern const char *JSX_jstypenames[];

int JSX_Get(JSContext *cx, char *p, char *oldptr, int do_clean, JSX_Type *type, jsval *rval);
int JSX_Get_multi(JSContext *cx, int do_clean, uintN nargs, JSX_FuncParam *type, jsval *rval, int convconst, void **argptr);

int JSX_Set(JSContext *cx, char *p, int will_clean, JSX_Type *type, jsval v);
int JSX_Set_multi(JSContext *cx, char *ptr, int will_clean, uintN nargs, JSX_FuncParam *type, jsval *vp, void **argptr);

JSBool JSX_NativeFunction(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool JSX_InitPointerCallback(JSContext *cx, JSObject *obj, JSFunction *fun, JSX_Type *type);
JSBool JSX_InitPointer(JSContext *cx, JSObject *retobj, JSObject *typeobj);

int JSX_TypeSize_multi(JSContext *cx, uintN nargs, JSX_FuncParam *type, jsval *vp, ffi_type **arg_types);
JSClass *JSX_GetTypeClass(void);
JSClass *JSX_GetPointerClass(void);

int JSX_CType(JSX_Type *type);
int JSX_JSType(JSContext *cx, jsval rval);
JSX_Type *GetVoidType(void); // the C "void" type
JSBool JSX_InitMemberType(JSContext *cx, JSX_SuMember *dest, JSObject *membertype);

#define JSNULL (JSVAL_TAGMASK+1) // because JSVAL_NULL == JSVAL_OBJECT
#define JSVOID (JSVAL_TAGMASK+2)
#define JSPOINTER (JSVAL_TAGMASK+3)
#define JSARRAY (JSVAL_TAGMASK+5)
#define JSFUNC (JSVAL_TAGMASK+6)

#define TYPEPAIR(a,b) ((TYPECOUNT2 * (a)) + (b))


struct JSX_Pointer {
  void *ptr; // 0 means unresolved. NULL pointer is repr by null value.
  JSX_Type *type;
  void (*finalize) (void *);

  ~JSX_Pointer();
};

struct JSX_Callback : JSX_Pointer {
  JSContext *cx;
  JSFunction *fun;
  void *writeable; // Points to writeable code
};


static inline int type_is_char(JSX_Type *t) {
  return (t->type == INTTYPE || t->type == UINTTYPE) && 0 == ((JSX_TypeNumeric *) t)->size;
}


static inline int is_void_or_char(JSX_Type *t) {
  return t->type == VOIDTYPE || type_is_char(t);
}

#endif
