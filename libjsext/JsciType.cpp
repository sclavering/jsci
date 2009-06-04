#include "jsci.h"


ffi_type *JSX_Type::GetFFIType(JSContext *cx) {
  return 0;
}


int JSX_Type::SizeInBits() {
  return JSX_TypeSize(this) * 8;
}
