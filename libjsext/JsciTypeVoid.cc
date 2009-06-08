#include "jsci.h"


JsciTypeVoid::JsciTypeVoid() : JsciType(VOIDTYPE) {
}


ffi_type *JsciTypeVoid::GetFFIType() {
  return &ffi_type_void;
}
