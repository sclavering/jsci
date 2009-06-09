#include "jsci.h"


JsciCallback::JsciCallback(JSContext *cx, JSFunction *fun, JsciType *t) : JsciPointer(), cx(cx), fun(fun) {
  void *code;
  this->type = t;
  this->writeable = ffi_closure_alloc(sizeof(ffi_closure), &code);
  this->ptr = code;
  // This would free the code address, not always identical to writeable address. So it is checked in the destructor (which is currently JsciPointer's destructor)
  this->finalize = ffi_closure_free;
}
