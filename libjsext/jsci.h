/*
jsci.h

Our JavaScript <-> C interface, using libffi.
*/

#ifndef __jsci_h
#define __jsci_h

#include <ffi.h>
#include "util.h"

// We store an instance of this, or a subclass, inside each js Type object
struct JsciType {
  virtual ~JsciType();

  virtual JSBool CtoJS(JSContext *cx, char *data, jsval *rval) = 0;
  virtual JSBool JStoC(JSContext *cx, char *data, jsval v);
  virtual ffi_type *GetFFIType();
  virtual int SizeInBits();
  virtual int SizeInBytes();
  virtual int AlignmentInBits();
  virtual int AlignmentInBytes();
};

struct JsciTypeVoid : JsciType {
  JsciTypeVoid();

  JSBool CtoJS(JSContext *cx, char *data, jsval *rval);
  ffi_type *GetFFIType();
};

struct JsciTypeNumeric : JsciType {
  ffi_type ffiType;

  ffi_type *GetFFIType();
  int SizeInBytes();
  int AlignmentInBytes();

 protected:
  JsciTypeNumeric(ffi_type ffit);
};

struct JsciTypeInt : JsciTypeNumeric {
  int size;

  JsciTypeInt(int size, ffi_type);

  virtual JSBool CtoJS(JSContext *cx, char *data, jsval *rval);
  JSBool JStoC(JSContext *cx, char *data, jsval v);

 private:
  JSBool JsToInt(JSContext *cx, jsval v, int *rv);
};

struct JsciTypeUint : JsciTypeInt {
  JsciTypeUint(int size, ffi_type);

  JSBool CtoJS(JSContext *cx, char *data, jsval *rval);
};

struct JsciTypeFloat : JsciTypeNumeric {
  JsciTypeFloat();

  JSBool CtoJS(JSContext *cx, char *data, jsval *rval);
  JSBool JStoC(JSContext *cx, char *data, jsval v);
};

struct JsciTypeDouble : JsciTypeNumeric {
  JsciTypeDouble();

  JSBool CtoJS(JSContext *cx, char *data, jsval *rval);
  JSBool JStoC(JSContext *cx, char *data, jsval v);
};

struct JsciTypeFunction : JsciType {
  JsciType *returnType;
  JsciType **param;
  int nParam;
  ffi_cif cif;

  JsciTypeFunction(JsciType *returnType, int nParam);
  ~JsciTypeFunction();

  JSBool CtoJS(JSContext *cx, char *data, jsval *rval);
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

  JSBool CtoJS(JSContext *cx, char *data, jsval *rval);
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

  JsciTypePointer(JsciType *direct);

  JSBool CtoJS(JSContext *cx, char *data, jsval *rval);
  JSBool JStoC(JSContext *cx, char *data, jsval v);
  ffi_type *GetFFIType();
  int SizeInBytes();
  int AlignmentInBytes();
};

struct JsciTypeArray : JsciType {
  JsciType *member;
  int length;

  JsciTypeArray(JsciType *type, int length);

  JSBool CtoJS(JSContext *cx, char *data, jsval *rval);
  JSBool JStoC(JSContext *cx, char *data, jsval v);
  int SizeInBytes();
  int AlignmentInBytes();
};

struct JsciTypeBitfield : JsciType {
  JsciType *member;
  int length;

  JsciTypeBitfield(JsciType *type, int length);

  JSBool CtoJS(JSContext *cx, char *data, jsval *rval);
  JSBool JStoC(JSContext *cx, char *data, jsval v);
  int SizeInBits();
  int AlignmentInBits() { return 1; }
  int AlignmentInBytes();
};


extern JsciType *gTypeVoid; // void
extern JsciType *gTypeChar; // (signed) char


JSClass *JSX_GetTypeClass(void);
JSClass *JSX_GetPointerClass(void);


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
