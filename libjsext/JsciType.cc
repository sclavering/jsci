#include "jsci.h"


JsciType::JsciType(JSX_TypeID t_id) : type(t_id) {
}


JsciType::~JsciType() {
}


JSBool JsciType::JStoC(JSContext *cx, char *data, jsval v) {
  return JSX_ReportException(cx, "Bad JStoC call");
}


ffi_type *JsciType::GetFFIType() {
  return 0;
}


int JsciType::SizeInBits() {
  return this->SizeInBytes() * 8;
}


int JsciType::SizeInBytes() {
  return 0; // a nonsensical value
}


int JsciType::AlignmentInBits() {
  return this->AlignmentInBytes() * 8;
}


int JsciType::AlignmentInBytes() {
  return 0; // nonsensical default
}


JSBool JsciType::ContainsPointer() {
  return JS_FALSE;
}
