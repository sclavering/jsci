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
  FLOATTYPE,
  FUNCTIONTYPE,
  SUTYPE,
  VOIDTYPE,
  POINTERTYPE,
  ARRAYTYPE,
  BITFIELDTYPE
};

// We store an instance of this, or a subclass, inside each js Type object
struct JsciType {
  enum JSX_TypeID type;

  JsciType(JSX_TypeID);
  virtual ~JsciType();

  virtual int CtoJS(JSContext *cx, char *data, jsval *rval) = 0;
  virtual int JStoC(JSContext *cx, char *data, jsval v, int will_clean);
  virtual ffi_type *GetFFIType();
  virtual int SizeInBits();
  virtual int SizeInBytes();
  virtual int AlignmentInBits();
  virtual int AlignmentInBytes();
  virtual JSBool ContainsPointer();
};

struct JsciTypeVoid : JsciType {
  JsciTypeVoid();

  int CtoJS(JSContext *cx, char *data, jsval *rval);
  ffi_type *GetFFIType();
};

struct JsciTypeNumeric : JsciType {
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

  virtual int CtoJS(JSContext *cx, char *data, jsval *rval);
  int JStoC(JSContext *cx, char *data, jsval v, int will_clean);

 private:
  JSBool JsToInt(JSContext *cx, jsval v, int *rv);
};

// extends JsciTypeInt rather than just JsciTypeNumeric so that we can share JStoC(), though doing so is probably wrong
struct JsciTypeUint : JsciTypeInt {
  JsciTypeUint(int size, ffi_type);

  int CtoJS(JSContext *cx, char *data, jsval *rval);
};

struct JsciTypeFloat : JsciTypeNumeric {
  JsciTypeFloat(int size, ffi_type);

  int CtoJS(JSContext *cx, char *data, jsval *rval);
  int JStoC(JSContext *cx, char *data, jsval v, int will_clean);

 private:
  JSBool JsToDouble(JSContext *cx, jsval v, jsdouble *rv);
};

struct JsciTypeFunction : JsciType {
  JsciType **param;
  int nParam;
  JsciType *returnType;
  ffi_cif cif;

  JsciTypeFunction(int nParam);
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
  JSX_SuMember *member;
  int nMember;
  int sizeOf; // in bits
  ffi_type ffiType;

  JsciTypeStructUnion();
  ~JsciTypeStructUnion();

  int CtoJS(JSContext *cx, char *data, jsval *rval);
  int JStoC(JSContext *cx, char *data, jsval v, int will_clean);
  int SizeInBytes();
  int AlignmentInBytes();
  JSBool ContainsPointer();

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
  JsciType *direct;

  JsciTypePointer();

  int CtoJS(JSContext *cx, char *data, jsval *rval);
  int JStoC(JSContext *cx, char *data, jsval v, int will_clean);
  ffi_type *GetFFIType();
  int SizeInBytes();
  int AlignmentInBytes();
  JSBool ContainsPointer();
};

struct JsciTypeArray : JsciType {
  JsciType *member;
  int length;

  JsciTypeArray();

  int CtoJS(JSContext *cx, char *data, jsval *rval);
  int JStoC(JSContext *cx, char *data, jsval v, int will_clean);
  int SizeInBytes();
  int AlignmentInBytes();
  JSBool ContainsPointer();
};

struct JsciTypeBitfield : JsciType {
  JsciType *member;
  int length;

  JsciTypeBitfield();

  int CtoJS(JSContext *cx, char *data, jsval *rval);
  int JStoC(JSContext *cx, char *data, jsval v, int will_clean);
  int SizeInBits();
  int AlignmentInBits() { return 1; }
  int AlignmentInBytes();
};


int JSX_Set(JSContext *cx, char *p, int will_clean, JsciType *type, jsval v);

JSBool JSX_NativeFunction(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool JSX_InitPointerCallback(JSContext *cx, JSObject *obj, JSFunction *fun, JsciType *type);
JSBool JSX_InitPointer(JSContext *cx, JSObject *retobj, JSObject *typeobj);

JSClass *JSX_GetTypeClass(void);
JSClass *JSX_GetPointerClass(void);

int JSX_JSType(JSContext *cx, jsval rval);
JsciType *GetVoidType(void); // the C "void" type
JSBool JSX_InitMemberType(JSContext *cx, JSX_SuMember *dest, JSObject *membertype);

#define JSNULL (JSVAL_TAGMASK+1) // because JSVAL_NULL == JSVAL_OBJECT
#define JSVOID (JSVAL_TAGMASK+2)
#define JSPOINTER (JSVAL_TAGMASK+3)
#define JSARRAY (JSVAL_TAGMASK+5)
#define JSFUNC (JSVAL_TAGMASK+6)


struct JsciPointer {
  void *ptr; // 0 means unresolved. NULL pointer is repr by null value.
  JsciType *type;
  void (*finalize) (void *);

  JsciPointer();
  virtual ~JsciPointer();
};

// A version of JsciPointer that allocates and frees its ->ptr
struct JsciPointerAlloc : JsciPointer {
  JsciPointerAlloc(int num_bytes);
  ~JsciPointerAlloc();
};

struct JsciCallback : JsciPointer {
  JSContext *cx;
  JSFunction *fun;
  void *writeable; // Points to writeable code

  JsciCallback(JSContext *cx, JSFunction *fun, JsciType *t);
  ~JsciCallback();
};


static inline int type_is_char(JsciType *t) {
  return t->type == INTTYPE && 0 == ((JsciTypeNumeric *) t)->size;
}


static inline int is_void_or_char(JsciType *t) {
  return t->type == VOIDTYPE || type_is_char(t);
}


static inline JSBool jsval_is_Type(JSContext *cx, jsval v) {
  return JSVAL_IS_OBJECT(v) && !JSVAL_IS_NULL(v) && JS_InstanceOf(cx, JSVAL_TO_OBJECT(v), JSX_GetTypeClass(), NULL);
}


#endif
