#include "jsci.h"


JsciTypeBitfield::JsciTypeBitfield() : JsciType(BITFIELDTYPE) {
}


int JsciTypeBitfield::CtoJS(JSContext *cx, char *data, jsval *rval) {
  return this->member->CtoJS(cx, data, rval);
}


int JsciTypeBitfield::JStoC(JSContext *cx, char *data, jsval v, int will_clean) {
  return JSX_ReportException(cx, "Could not convert JS value to C bitfield value because the bitfield is not within a struct/union");
}


int JsciTypeBitfield::SizeInBits() {
  return this->length;
}


int JsciTypeBitfield::AlignmentInBytes() {
  // when calculating struct alignment, this is it.
  return this->member->AlignmentInBytes();
}
