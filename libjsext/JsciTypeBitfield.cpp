#include "jsci.h"


int JSX_TypeBitfield::SizeInBits() {
  return this->length;
}


int JSX_TypeBitfield::AlignmentInBytes() {
  // when calculating struct alignment, this is it.
  return this->member->AlignmentInBytes();
}
