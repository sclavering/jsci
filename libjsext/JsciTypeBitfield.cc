#include "jsci.h"


JsciTypeBitfield::JsciTypeBitfield() : JsciType(BITFIELDTYPE) {
}


int JsciTypeBitfield::CtoJS(JSContext *cx, char *data, jsval *rval) {
  return this->member->CtoJS(cx, data, rval);
}


int JsciTypeBitfield::SizeInBits() {
  return this->length;
}


int JsciTypeBitfield::AlignmentInBytes() {
  // when calculating struct alignment, this is it.
  return this->member->AlignmentInBytes();
}
