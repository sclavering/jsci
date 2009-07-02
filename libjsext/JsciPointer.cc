#include "jsci.h"


JsciPointer::JsciPointer(JsciType *type) : type(type), ptr(0), finalize(0) {
}


JsciPointer::~JsciPointer() {
  if(this->finalize) (*this->finalize)(this->ptr);
}
