#include "jsci.h"


ffi_type *JSX_TypeVoid::GetFFIType(JSContext *cx) {
  return &ffi_type_void;
}
