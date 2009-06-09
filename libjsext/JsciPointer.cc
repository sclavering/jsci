#include "jsci.h"


JsciPointer::JsciPointer() : ptr(0), finalize(0) {
}


JsciPointer::~JsciPointer() {
  if(this->finalize) (*this->finalize)(this->ptr);
}
