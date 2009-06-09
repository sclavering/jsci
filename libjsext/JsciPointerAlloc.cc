#include "jsci.h"


JsciPointerAlloc::JsciPointerAlloc(int num_bytes) : JsciPointer() {
  this->ptr = new char[num_bytes];
}


JsciPointerAlloc::~JsciPointerAlloc() {
  delete this->ptr;
}
