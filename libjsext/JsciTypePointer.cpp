#include "jsci.h"


ffi_type *JSX_TypePointer::GetFFIType(JSContext *cx) {
  return &ffi_type_pointer;
}
