#include "jsci.h"


JsciTypeUnion::JsciTypeUnion() : JsciTypeStructUnion(UNIONTYPE) {
}


JSBool JsciTypeUnion::SetSizeAndAligments(JSContext *cx) {
  for(int i = 0; i != this->nMember; ++i) {
    this->member[i].offset = 0;
    int sz = this->member[i].membertype->SizeInBits();
    if(sz > this->sizeOf) this->sizeOf = sz;
  }
  return JS_TRUE;
}
