#include "jsci.h"


int JSX_TypeArray::SizeInBytes() {
  return this->length * this->member->SizeInBytes();
}
