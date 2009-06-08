#include "jsci.h"


JsciTypeFunction::JsciTypeFunction(int nParam) : JsciType(FUNCTIONTYPE), nParam(nParam) {
  this->param = new JsciType*[this->nParam + 1];
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


int JsciTypeFunction::GetParamSizesAndFFITypes(JSContext *cx, ffi_type **arg_types) {
  int totalsize = 0;
  for(uintN i = 0; i != this->nParam; ++i) {
    JsciType *t = this->param[i];
    int siz = t->SizeInBytes();
    if(!siz) return 0; // error
    *(arg_types++) = t->GetFFIType();
    totalsize += siz;
  }
  *arg_types = 0;
  return totalsize;
}
