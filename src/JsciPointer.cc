#include "jsci.h"


JsciPointer::JsciPointer(JsciType *type, void *ptr) : ptr(ptr), type(type), finalize(0) {
}


JsciPointer::~JsciPointer() {
  if(this->finalize) (*this->finalize)(this->ptr);
}
