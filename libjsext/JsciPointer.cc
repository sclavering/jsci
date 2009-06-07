#include "jsci.h"


JSX_Pointer::~JSX_Pointer() {
  if(this->finalize) {
    if(this->finalize == ffi_closure_free) {
      ffi_closure_free(((JSX_Callback *) this)->writeable);
    } else {
      (*this->finalize)(this->ptr);
    }
  }
}
