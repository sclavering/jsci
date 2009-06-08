#include "jsci.h"


JsciTypeBitfield::JsciTypeBitfield() : JsciType(BITFIELDTYPE) {
}


int JsciTypeBitfield::SizeInBits() {
  return this->length;
}


int JsciTypeBitfield::AlignmentInBytes() {
  // when calculating struct alignment, this is it.
  return this->member->AlignmentInBytes();
}
