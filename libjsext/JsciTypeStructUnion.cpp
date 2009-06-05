#include "jsci.h"


int JSX_TypeStructUnion::SizeInBytes() {
  int align = JSX_TypeAlign(this);
  return (((this->sizeOf + 7) / 8 + align - 1) / align) * align;
}
