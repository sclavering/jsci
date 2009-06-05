#include "jsci.h"


ffi_type *JSX_TypeNumeric::GetFFIType(JSContext *cx) {
  return &this->ffiType;
}


int JSX_TypeNumeric::SizeInBytes() {
  return this->ffiType.size;
}
