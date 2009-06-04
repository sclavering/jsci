#include "jsci.h"


ffi_type *JSX_TypeNumeric::GetFFIType(JSContext *cx) {
  return &this->ffiType;
}
