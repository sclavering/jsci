#include "jsci.h"


JsciTypeArray::JsciTypeArray() : JsciType(ARRAYTYPE) {
}


int JsciTypeArray::CtoJS(JSContext *cx, char *data, jsval *rval) {
  if(type_is_char(this->member)) {
    // Return a string from a char array
    *rval = STRING_TO_JSVAL(JS_NewStringCopyN(cx, data, this->length));
    return 1;
  }

  *rval = OBJECT_TO_JSVAL(JS_NewArrayObject(cx, 0, 0));
  int fieldsize = this->SizeInBytes();
  JSObject *obj = JSVAL_TO_OBJECT(*rval);
  jsval tmp = JSVAL_VOID;
  JS_AddRoot(cx, &tmp);
  for(int i = 0; i != this->length; i++) {
    int ok = this->member->CtoJS(cx, data + i * fieldsize, &tmp);
    if(!ok) {
      JS_RemoveRoot(cx, &tmp);
      return 0; // exception should already have been set
    }
    JS_SetElement(cx, obj, i, &tmp);
  }
  JS_RemoveRoot(cx, &tmp);
  return 1;
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
