#include "jsci.h"


int JSX_TypeArray::SizeInBytes() {
  return this->length * this->member->SizeInBytes();
}


int JSX_TypeArray::AlignmentInBytes() {
  return this->member->AlignmentInBytes();
}
