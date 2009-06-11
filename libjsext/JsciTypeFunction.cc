#include "jsci.h"


JsciTypeFunction::JsciTypeFunction(int nParam) : JsciType(FUNCTIONTYPE), nParam(nParam) {
  this->param = new JsciType*[this->nParam];
}


JsciTypeFunction::~JsciTypeFunction() {
  if(this->param) delete this->param;
  if(this->cif.arg_types) delete this->cif.arg_types;
}


int JsciTypeFunction::CtoJS(JSContext *cx, char *data, jsval *rval) {
  // Create a new JS function which calls a C function
  JSFunction *fun = JS_NewFunction(cx, JSX_NativeFunction, this->nParam, 0, 0, "JSEXT_NATIVE");
  JSObject *funobj = JS_GetFunctionObject(fun);
  *rval = OBJECT_TO_JSVAL(funobj);
  return -1;
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


int JsciTypeFunction::GetParamSizes(JSContext *cx) {
  int totalsize = 0;
  for(uintN i = 0; i != this->nParam; ++i) {
    JsciType *t = this->param[i];
    int siz = t->SizeInBytes();
    if(!siz) return 0; // error
    totalsize += siz;
  }
  return totalsize;
}


JSBool JsciTypeFunction::Call(JSContext *cx, void *cfunc, uintN argc, jsval *argv, jsval *rval) {
  if(this->nParam != argc) return JSX_ReportException(cx, "C function with %i parameters called with %i arguments", this->nParam, argc);

  size_t arg_size = this->GetParamSizes(cx);
  int retsize = this->returnType->SizeInBytes();

  char *argptr_mem = new char[arg_size + argc * sizeof(void*) + retsize + 8];
  void **argptr = (void **) argptr_mem;

  char *retbuf = (char *) (argptr + argc);
  char *argbuf = retbuf + retsize + 8; // ffi overwrites a few bytes on some archs.

  if(arg_size) {
    char *ptr = argbuf;
    jsval *vp = argv;
    void **argptr2 = argptr;
    int cursiz;
    for(int i = 0; i != this->nParam; ++i) {
      JsciType *t = this->param[i];
      if(t->type == ARRAYTYPE) {
        // In function calls, arrays are passed by pointer
        ptr = new char[t->SizeInBytes()];
        cursiz = JSX_Set(cx, (char*) *(void **)ptr, 1, t, *vp);
        if(cursiz) {
          cursiz = sizeof(void *);
        } else {
          delete ptr;
          goto failure;
        }
      } else {
        cursiz = JSX_Set(cx, (char*) ptr, 1, t, *vp);
      }
      if(!cursiz) goto failure;
      *(argptr2++) = ptr;
      ptr += cursiz;
      vp++;
    }
  }

  ffi_call(this->GetCIF(), FFI_FN(cfunc), (void *)retbuf, argptr);

  *rval=JSVAL_VOID;

  if(this->returnType->type != VOIDTYPE) this->returnType->CtoJS(cx, retbuf, rval);

  delete argptr_mem;

  return JS_TRUE;

 failure:
  delete argptr_mem;

  return JS_FALSE;
}
