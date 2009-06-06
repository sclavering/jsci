#include "jsci.h"


ffi_type *JSX_TypePointer::GetFFIType() {
  return &ffi_type_pointer;
}


int JSX_TypePointer::SizeInBytes() {
  return ffi_type_pointer.size;
}


int JSX_TypePointer::AlignmentInBytes() {
  return ffi_type_pointer.alignment;
}


JSBool JSX_TypePointer::ContainsPointer() {
  return JS_TRUE;
}
