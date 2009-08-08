#include "jsci.h"


JsciTypeNumeric::JsciTypeNumeric(ffi_type ffit) : ffiType(ffit) {
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
