#include "jsci.h"


JsciTypeBitfield::JsciTypeBitfield(JsciType *type, int length) : member(type), length(length) {
}


JSBool JsciTypeBitfield::CtoJS(JSContext *cx, char *data, jsval *rval) {
  return this->member->CtoJS(cx, data, rval);
}


JSBool JsciTypeBitfield::JStoC(JSContext *cx, char *data, jsval v) {
  return JSX_ReportException(cx, "Cannot convert JS value to C bitfield value because the bitfield is not within a struct/union");
}


int JsciTypeBitfield::SizeInBits() {
  return this->length;
}


int JsciTypeBitfield::AlignmentInBytes() {
  // when calculating struct alignment, this is it.
  return this->member->AlignmentInBytes();
}
