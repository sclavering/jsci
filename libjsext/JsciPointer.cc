#include "jsci.h"


JsciPointer::~JsciPointer() {
  if(this->finalize) {
    if(this->finalize == ffi_closure_free) {
      ffi_closure_free(((JsciCallback *) this)->writeable);
    } else {
      (*this->finalize)(this->ptr);
    }
  }
}
