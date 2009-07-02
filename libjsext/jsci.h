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
  virtual JSBool JStoC(JSContext *cx, char *data, jsval v);
  virtual ffi_type *GetFFIType();
  virtual int SizeInBits();
  virtual int SizeInBytes();
  virtual int AlignmentInBits();
  virtual int AlignmentInBytes();
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
  JSBool JStoC(JSContext *cx, char *data, jsval v);

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
  JSBool JStoC(JSContext *cx, char *data, jsval v);

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
  JSBool Call(JSContext *cx, void *cfunc, uintN argc, jsval *argv, jsval *rval);

  ffi_cif *GetCIF();
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
  JSBool JStoC(JSContext *cx, char *data, jsval v);
  int SizeInBytes();
  int AlignmentInBytes();

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
  JSBool JStoC(JSContext *cx, char *data, jsval v);
  ffi_type *GetFFIType();
  int SizeInBytes();
  int AlignmentInBytes();
};

struct JsciTypeArray : JsciType {
  JsciType *member;
  int length;

  JsciTypeArray();

  int CtoJS(JSContext *cx, char *data, jsval *rval);
  JSBool JStoC(JSContext *cx, char *data, jsval v);
  int SizeInBytes();
  int AlignmentInBytes();
};

struct JsciTypeBitfield : JsciType {
  JsciType *member;
  int length;

  JsciTypeBitfield();

  int CtoJS(JSContext *cx, char *data, jsval *rval);
  JSBool JStoC(JSContext *cx, char *data, jsval v);
  int SizeInBits();
  int AlignmentInBits() { return 1; }
  int AlignmentInBytes();
};


JSClass *JSX_GetTypeClass(void);
JSClass *JSX_GetPointerClass(void);

JsciType *GetVoidType(void); // the C "void" type


struct JsciPointer {
  void *ptr; // 0 means unresolved. NULL pointer is repr by null value.
  JsciType *type;
  void (*finalize) (void *);

  JsciPointer(JsciType *type, void *ptr);
  virtual ~JsciPointer();
};

// A version of JsciPointer that allocates and frees its ->ptr
struct JsciPointerAlloc : JsciPointer {
  JsciPointerAlloc(JsciType *type, int num_bytes);
  ~JsciPointerAlloc();
};

struct JsciCallback {
 public:
  // the jsval must be for a js function
  static JsciCallback *GetForFunction(JSContext *cx, jsval fun, JsciTypeFunction *t);
  static JsciCallback *Create(JSContext *cx, jsval fun, JsciTypeFunction *t);
  ~JsciCallback();
  void *codeptr();
  void exec(ffi_cif *cif, void *ret, void **args);
 protected:
  JsciCallback(JSContext *cx, jsval fun, JsciTypeFunction *t);
  JSContext *cx;
  JsciTypeFunction *type;
  jsval fun;
  void *code;
  void *writeable; // Points to writeable code
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


static inline JSBool WrapPointer(JSContext *cx, JsciPointer *p, jsval *rval) {
  *rval = OBJECT_TO_JSVAL(JS_NewObject(cx, JSX_GetPointerClass(), 0, 0));
  return JS_SetPrivate(cx, JSVAL_TO_OBJECT(*rval), p);
}


// Used when we need to save a Type instance from GC, because we're sharing its JsciType*
static inline JSBool WrapPointerAndSaveType(JSContext *cx, JsciPointer *p, jsval *rval, jsval typeobj) {
  return WrapPointer(cx, p, rval) && JS_SetReservedSlot(cx, JSVAL_TO_OBJECT(*rval), 0, typeobj);
}


static inline JsciPointer *jsval_to_JsciPointer(JSContext *cx, jsval v) {
  if(!JSVAL_IS_OBJECT(v)) return 0;
  return (JsciPointer*) JS_GetInstancePrivate(cx, JSVAL_TO_OBJECT(v), JSX_GetPointerClass(), 0);
}


static inline JsciType *jsval_to_JsciType(JSContext *cx, jsval v) {
  if(!JSVAL_IS_OBJECT(v)) return 0;
  return (JsciType*) JS_GetInstancePrivate(cx, JSVAL_TO_OBJECT(v), JSX_GetTypeClass(), 0);
}


#endif
