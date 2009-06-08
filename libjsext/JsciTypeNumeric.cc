#include "jsci.h"


JsciTypeNumeric::JsciTypeNumeric(JSX_TypeID t_id, int size, ffi_type ffit)
  : JsciType(t_id), size(size), ffiType(ffit) {
}


ffi_type *JsciTypeNumeric::GetFFIType() {
  return &this->ffiType;
}


int JsciTypeNumeric::SizeInBytes() {
  return this->ffiType.size;
}


int JsciTypeNumeric::AlignmentInBytes() {
  return this->ffiType.alignment;
}



// subclasses

JsciTypeInt::JsciTypeInt(int size, ffi_type ffit) : JsciTypeNumeric(INTTYPE, size, ffit) {}
JsciTypeUint::JsciTypeUint(int size, ffi_type ffit) : JsciTypeNumeric(UINTTYPE, size, ffit) {}
JsciTypeFloat::JsciTypeFloat(int size, ffi_type ffit) : JsciTypeNumeric(FLOATTYPE, size, ffit) {}
