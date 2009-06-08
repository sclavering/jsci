#include "jsci.h"


JsciTypeArray::JsciTypeArray() : JsciType(ARRAYTYPE) {
}


int JsciTypeArray::SizeInBytes() {
  return this->length * this->member->SizeInBytes();
}


int JsciTypeArray::AlignmentInBytes() {
  return this->member->AlignmentInBytes();
}


JSBool JsciTypeArray::ContainsPointer() {
  return this->member->ContainsPointer();
}
