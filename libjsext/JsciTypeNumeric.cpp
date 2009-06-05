#include "jsci.h"


ffi_type *JSX_TypeNumeric::GetFFIType() {
  return &this->ffiType;
}


int JSX_TypeNumeric::SizeInBytes() {
  return this->ffiType.size;
}
