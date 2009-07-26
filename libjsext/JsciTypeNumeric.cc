#include "jsci.h"


JsciTypeNumeric::JsciTypeNumeric(JSX_TypeID t_id, ffi_type ffit) : JsciType(t_id), ffiType(ffit) {
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
