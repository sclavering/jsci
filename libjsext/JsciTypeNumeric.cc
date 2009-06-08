#include "jsci.h"


ffi_type *JsciTypeNumeric::GetFFIType() {
  return &this->ffiType;
}


int JsciTypeNumeric::SizeInBytes() {
  return this->ffiType.size;
}


int JsciTypeNumeric::AlignmentInBytes() {
  return this->ffiType.alignment;
}
