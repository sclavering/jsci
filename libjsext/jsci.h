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

#define TYPECOUNT2 (TYPECOUNT+5)

// We store an instance of this, or a subclass, inside each js Type object
struct JsciType {
  enum JSX_TypeID type;

  JsciType(JSX_TypeID);
  virtual ~JsciType();

  virtual int CtoJS(JSContext *cx, char *data, jsval *rval) = 0;
  virtual ffi_type *GetFFIType();
  virtual int SizeInBits();
  virtual int SizeInBytes();
  virtual int AlignmentInBits();
  virtual int AlignmentInBytes();
  virtual JSBool ContainsPointer();
};

struct JsciTypeVoid : JsciType {
  // VOIDTYPE
  JsciTypeVoid();

  int CtoJS(JSContext *cx, char *data, jsval *rval);
  ffi_type *GetFFIType();
};

struct JsciTypeNumeric : JsciType {
  // INTTYPE, UINTTYPE, or FLOATTYPE
  int size;
  ffi_type ffiType;

  ffi_type *GetFFIType();
  int SizeInBytes();
  int AlignmentInBytes();

 protected:
  JsciTypeNumeric(JSX_TypeID, int size, ffi_type ffit);
};

struct JsciTypeInt : JsciTypeNumeric {
  JsciTypeInt(int size, ffi_type);

  int CtoJS(JSContext *cx, char *data, jsval *rval);
};

struct JsciTypeUint : JsciTypeNumeric {
  JsciTypeUint(int size, ffi_type);

  int CtoJS(JSContext *cx, char *data, jsval *rval);
};

struct JsciTypeFloat : JsciTypeNumeric {
  JsciTypeFloat(int size, ffi_type);

  int CtoJS(JSContext *cx, char *data, jsval *rval);
};

struct JsciTypeFunction : JsciType {
  // FUNCTIONTYPE
  JsciType **param;
  int nParam;
  JsciType *returnType;
  ffi_cif cif;

  JsciTypeFunction();
  ~JsciTypeFunction();

  int CtoJS(JSContext *cx, char *data, jsval *rval);
  ffi_cif *GetCIF();
  int GetParamSizesAndFFITypes(JSContext *cx, ffi_type **arg_types);
};

struct JSX_SuMember {
  JsciType *membertype;
  char *name;
  int offset; // in bits

  JSX_SuMember() : membertype(0), name(0), offset(0) {}
};

struct JsciTypeStructUnion : JsciType {
  // STRUCTTYPE or UNIONTYPE
  JSX_SuMember *member;
  int nMember;
  int sizeOf; // in bits
  ffi_type ffiType;

  JsciTypeStructUnion(JSX_TypeID);
  ~JsciTypeStructUnion();

  int SizeInBytes();
  int AlignmentInBytes();
  JSBool ContainsPointer();

  int CtoJS(JSContext *cx, char *data, jsval *rval);
  virtual JSBool SetSizeAndAligments(JSContext *cx) = 0;
  JSBool ReplaceMembers(JSContext *cx, JSObject *obj, int nMember, jsval *members);
};

struct JsciTypeStruct : JsciTypeStructUnion {
  JsciTypeStruct();

  ffi_type *GetFFIType();
  JSBool SetSizeAndAligments(JSContext *cx);
};

struct JsciTypeUnion : JsciTypeStructUnion {
  JsciTypeUnion();

  JSBool SetSizeAndAligments(JSContext *cx);
};

struct JsciTypePointer : JsciType {
  // POINTERTYPE
  JsciType *direct;

  JsciTypePointer();

  int CtoJS(JSContext *cx, char *data, jsval *rval);
  ffi_type *GetFFIType();
  int SizeInBytes();
  int AlignmentInBytes();
  JSBool ContainsPointer();
};

struct JsciTypeArray : JsciType {
  // ARRAYTYPE
  JsciType *member;
  int length;

  JsciTypeArray();

  int CtoJS(JSContext *cx, char *data, jsval *rval);
  int SizeInBytes();
  int AlignmentInBytes();
  JSBool ContainsPointer();
};

struct JsciTypeBitfield : JsciType {
  // BITFIELDTYPE
  JsciType *member;
  int length;

  JsciTypeBitfield();

  int CtoJS(JSContext *cx, char *data, jsval *rval);
  int SizeInBits();
  int AlignmentInBits() { return 1; }
  int AlignmentInBytes();
};


extern const char *JSX_typenames[];
extern const char *JSX_jstypenames[];

int JSX_Set(JSContext *cx, char *p, int will_clean, JsciType *type, jsval v);
JSBool JSX_Set_multi(JSContext *cx, char *ptr, int will_clean, JsciTypeFunction *tf, jsval *vp, void **argptr);

JSBool JSX_NativeFunction(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool JSX_InitPointerCallback(JSContext *cx, JSObject *obj, JSFunction *fun, JsciType *type);
JSBool JSX_InitPointer(JSContext *cx, JSObject *retobj, JSObject *typeobj);

JSClass *JSX_GetTypeClass(void);
JSClass *JSX_GetPointerClass(void);

int JSX_CType(JsciType *type);
int JSX_JSType(JSContext *cx, jsval rval);
JsciType *GetVoidType(void); // the C "void" type
JSBool JSX_InitMemberType(JSContext *cx, JSX_SuMember *dest, JSObject *membertype);

#define JSNULL (JSVAL_TAGMASK+1) // because JSVAL_NULL == JSVAL_OBJECT
#define JSVOID (JSVAL_TAGMASK+2)
#define JSPOINTER (JSVAL_TAGMASK+3)
#define JSARRAY (JSVAL_TAGMASK+5)
#define JSFUNC (JSVAL_TAGMASK+6)

#define TYPEPAIR(a,b) ((TYPECOUNT2 * (a)) + (b))


struct JsciPointer {
  void *ptr; // 0 means unresolved. NULL pointer is repr by null value.
  JsciType *type;
  void (*finalize) (void *);

  ~JsciPointer();
};

struct JsciCallback : JsciPointer {
  JSContext *cx;
  JSFunction *fun;
  void *writeable; // Points to writeable code
};


static inline int type_is_char(JsciType *t) {
  return (t->type == INTTYPE || t->type == UINTTYPE) && 0 == ((JsciTypeNumeric *) t)->size;
}


static inline int is_void_or_char(JsciType *t) {
  return t->type == VOIDTYPE || type_is_char(t);
}

#endif
