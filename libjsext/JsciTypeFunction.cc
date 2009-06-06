#include "jsci.h"


JSX_TypeFunction::~JSX_TypeFunction() {
  if(this->param) delete this->param;
  if(this->cif.arg_types) delete this->cif.arg_types;
}


ffi_cif *JSX_TypeFunction::GetCIF() {
  if(this->cif.arg_types) return &this->cif;

  this->cif.arg_types = new ffi_type*[this->nParam];
  for(int i = 0; i != this->nParam; ++i) {
    if(this->param[i].paramtype->type == ARRAYTYPE) {
      this->cif.arg_types[i] = &ffi_type_pointer;
    } else {
      this->cif.arg_types[i] = this->param[i].paramtype->GetFFIType();
    }
  }
  ffi_prep_cif(&this->cif, FFI_DEFAULT_ABI, this->nParam, this->returnType->GetFFIType(), this->cif.arg_types);
  return &this->cif;
}


int JSX_TypeFunction::GetParamSizesAndFFITypes(JSContext *cx, ffi_type **arg_types) {
  int totalsize = 0;
  for(uintN i = 0; i != this->nParam; ++i) {
    JSX_Type *t = this->param[i].paramtype;
    int siz = t->SizeInBytes();
    if(!siz) return 0; // error
    *(arg_types++) = t->GetFFIType();
    totalsize += siz;
  }
  *arg_types = 0;
  return totalsize;
}
