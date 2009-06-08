#include "jsci.h"


JsciTypePointer::JsciTypePointer() : JsciType(POINTERTYPE) {
}


ffi_type *JsciTypePointer::GetFFIType() {
  return &ffi_type_pointer;
}


int JsciTypePointer::SizeInBytes() {
  return ffi_type_pointer.size;
}


int JsciTypePointer::AlignmentInBytes() {
  return ffi_type_pointer.alignment;
}


JSBool JsciTypePointer::ContainsPointer() {
  return JS_TRUE;
}
