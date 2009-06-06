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


int JSX_TypeFunction::ParamSizes(JSContext *cx, uintN nargs, jsval *vp, ffi_type **arg_types) {
  JSX_FuncParam *type = this->param;
  int ret = 0;

  for(uintN i = 0; i < nargs; ++i) {
    if(type && type->paramtype->type == VOIDTYPE) type = 0; // End of param list

    int siz;
    JSX_Type *thistype;

    if(JSVAL_IS_OBJECT(*vp) && *vp != JSVAL_NULL && JS_InstanceOf(cx, JSVAL_TO_OBJECT(*vp), JSX_GetTypeClass(), NULL)) {
      thistype = (JSX_Type *) JS_GetPrivate(cx, JSVAL_TO_OBJECT(*vp));
      vp++;
      i++;
      if(i == nargs) break;
    } else {
      thistype = type ? type->paramtype : 0;
    }

    if(!thistype) {
      siz = JSX_Get(cx, 0, 0, 0, 0, vp); // Get size of C type guessed from js type
      if(arg_types) {
        int jstype = JSX_JSType(cx, *vp);
        switch(jstype) {
          case JSVAL_STRING:
            *(arg_types++) = &ffi_type_pointer;
            break;
          case JSVAL_INT:
            *(arg_types++) = &ffi_type_sint;
            break;
          case JSVAL_DOUBLE:
            *(arg_types++) = &ffi_type_double;
            break;
        }
      }
    } else {
      siz = thistype->SizeInBytes();
      if(arg_types) *(arg_types++) = thistype->GetFFIType();
    }
    if(!siz) return 0; // error
    ret += siz;
    vp++;
    if(type) type++;
  }

  if(arg_types) *arg_types = 0;
  return ret;
}
