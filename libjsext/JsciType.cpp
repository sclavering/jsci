#include "jsci.h"


ffi_type *JSX_Type::GetFFIType() {
  return 0;
}


int JSX_Type::SizeInBits() {
  return this->SizeInBytes() * 8;
}


int JSX_Type::SizeInBytes() {
  return 0; // a nonsensical value
}


int JSX_Type::AlignmentInBits() {
  return this->AlignmentInBytes() * 8;
}


int JSX_Type::AlignmentInBytes() {
  return 0; // nonsensical default
}
