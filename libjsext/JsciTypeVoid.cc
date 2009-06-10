#include "jsci.h"


JsciTypeVoid::JsciTypeVoid() : JsciType(VOIDTYPE) {
}


int JsciTypeVoid::CtoJS(JSContext *cx, char *data, jsval *rval) {
  return JSX_ReportException(cx, "Cannot convert C value of type void to a JS value");
}


ffi_type *JsciTypeVoid::GetFFIType() {
  return &ffi_type_void;
}
