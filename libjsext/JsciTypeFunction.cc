#include "jsci.h"


static JSBool JSX_NativeFunction(JSContext *cx, JSObject *thisobj, uintN argc, jsval *argv, jsval *rval);


JsciTypeFunction::JsciTypeFunction(int nParam) : JsciType(FUNCTIONTYPE), nParam(nParam) {
  this->param = new JsciType*[this->nParam];
}


JsciTypeFunction::~JsciTypeFunction() {
  if(this->param) delete this->param;
  if(this->cif.arg_types) delete this->cif.arg_types;
}


int JsciTypeFunction::CtoJS(JSContext *cx, char *data, jsval *rval) {
  return JSX_ReportException(cx, "Pointer.prototype.$ cannot be used on pointers of type function.");
}


ffi_cif *JsciTypeFunction::GetCIF() {
  if(this->cif.arg_types) return &this->cif;

  this->cif.arg_types = new ffi_type*[this->nParam];
  for(int i = 0; i != this->nParam; ++i) {
    if(this->param[i]->type == ARRAYTYPE) {
      this->cif.arg_types[i] = &ffi_type_pointer;
    } else {
      this->cif.arg_types[i] = this->param[i]->GetFFIType();
    }
  }
  ffi_prep_cif(&this->cif, FFI_DEFAULT_ABI, this->nParam, this->returnType->GetFFIType(), this->cif.arg_types);
  return &this->cif;
}


JSBool JsciTypeFunction::Call(JSContext *cx, void *cfunc, uintN argc, jsval *argv, jsval *rval) {
  if(this->nParam != argc) return JSX_ReportException(cx, "C function with %i parameters called with %i arguments", this->nParam, argc);

  int ok = JS_FALSE;

  int *argsizes = new int[argc];

  int arg_size = 0;
  for(uintN i = 0; i != this->nParam; ++i) {
    JsciType *t = this->param[i];
    int siz = argsizes[i] = t->SizeInBytes();
    if(!siz) { delete argsizes; return JS_FALSE; }
    arg_size += siz;
  }

  int retsize = this->returnType->SizeInBytes();

  void **argptr = new void*[argc];
  char *argbuf = new char[arg_size];
  char *retbuf = new char[retsize + 8]; // ffi overwrites a few bytes on some archs.

  if(arg_size) {
    char *ptr = argbuf;
    for(int i = 0; i != this->nParam; ++i) {
      JsciType *t = this->param[i];
      if(t->type == ARRAYTYPE) return JSX_ReportException(cx, "C function's parameter %i is of type array.  Make it a pointer, somehow", i);
      if(!t->JStoC(cx, ptr, argv[i])) goto failure;
      argptr[i] = ptr;
      ptr += argsizes[i];
    }
  }

  ffi_call(this->GetCIF(), FFI_FN(cfunc), (void *)retbuf, argptr);

  *rval=JSVAL_VOID;

  if(this->returnType->type != VOIDTYPE) this->returnType->CtoJS(cx, retbuf, rval);

  ok = JS_TRUE;

 failure:
  delete argsizes;
  delete argptr;
  delete argbuf;
  delete retbuf;
  return ok;
}
