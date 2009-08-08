#include "jsci.h"


JsciPointer::JsciPointer(JsciType *type, void *ptr) : type(type), ptr(ptr), finalize(0) {
}


JsciPointer::~JsciPointer() {
  if(this->finalize) (*this->finalize)(this->ptr);
}
