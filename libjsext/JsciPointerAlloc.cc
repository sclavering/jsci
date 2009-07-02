#include "jsci.h"


JsciPointerAlloc::JsciPointerAlloc(JsciType *type, int num_bytes) : JsciPointer(type, 0) {
  this->ptr = new char[num_bytes];
}


JsciPointerAlloc::~JsciPointerAlloc() {
  delete (char*) this->ptr;
}
