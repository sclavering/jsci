#include "jsci.h"


JsciPointerAlloc::JsciPointerAlloc(JsciType *type, int num_bytes) : JsciPointer(type) {
  this->ptr = new char[num_bytes];
}


JsciPointerAlloc::~JsciPointerAlloc() {
  delete (char*) this->ptr;
}
