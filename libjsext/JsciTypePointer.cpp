#include "jsci.h"


ffi_type *JSX_TypePointer::GetFFIType() {
  return &ffi_type_pointer;
}


int JSX_TypePointer::SizeInBytes() {
  return ffi_type_pointer.size;
}
