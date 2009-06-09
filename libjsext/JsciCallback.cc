#include "jsci.h"


JsciCallback::JsciCallback(JSContext *cx, JSFunction *fun, JsciType *t) : JsciPointer(), cx(cx), fun(fun) {
  void *code;
  this->type = t;
  this->writeable = ffi_closure_alloc(sizeof(ffi_closure), &code);
  this->ptr = code;
}


JsciCallback::~JsciCallback() {
  ffi_closure_free(this->writeable);
}
