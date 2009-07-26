#include <string.h>
#include "jsci.h"


JsciTypeArray::JsciTypeArray(JsciType *type, int length) : JsciType(ARRAYTYPE), member(type), length(length) {
}


JSBool JsciTypeArray::CtoJS(JSContext *cx, char *data, jsval *rval) {
  if(this->member == gTypeChar) {
    // Return a string from a char array
    *rval = STRING_TO_JSVAL(JS_NewStringCopyN(cx, data, this->length));
    return JS_TRUE;
  }

  *rval = OBJECT_TO_JSVAL(JS_NewArrayObject(cx, 0, 0));
  int fieldsize = this->SizeInBytes();
  JSObject *obj = JSVAL_TO_OBJECT(*rval);
  jsval tmp = JSVAL_VOID;
  JS_AddRoot(cx, &tmp);
  for(int i = 0; i != this->length; i++) {
    if(!this->member->CtoJS(cx, data + i * fieldsize, &tmp)) {
      JS_RemoveRoot(cx, &tmp);
      return JS_FALSE; // exception should already have been set
    }
    JS_SetElement(cx, obj, i, &tmp);
  }
  JS_RemoveRoot(cx, &tmp);
  return JS_TRUE;
}


JSBool JsciTypeArray::JStoC(JSContext *cx, char *data, jsval v) {
  if(JSVAL_IS_NULL(v)) {
    int size = this->SizeInBytes();
    memset(data, 0, size);
    return JS_TRUE;
  }

  if(JSVAL_IS_STRING(v)) {
      // Copy a string to a char array
      if(this->member != gTypeChar) return JSX_ReportException(cx, "Cannot convert JS string to a C array of non-chars");
      int size = JS_GetStringLength(JSVAL_TO_STRING(v));
      if(size < this->length) {
        memcpy(*(char **)data, JS_GetStringBytes(JSVAL_TO_STRING(v)), size * sizeof(char));
        memset(*(char **)data + size, 0, (this->length - size) * sizeof(char));
      } else {
        memcpy(*(char **)data, JS_GetStringBytes(JSVAL_TO_STRING(v)), this->length * sizeof(char));
      }
      return JS_TRUE;
  }

  if(JSVAL_IS_OBJECT(v)) {
    JSObject *obj = JSVAL_TO_OBJECT(v);
    // Copy array elements to a fixed size array
    if(JS_IsArrayObject(cx, obj)) {
      int elsize = this->member->SizeInBytes();
      for(int i = 0; i != this->length; ++i) {
        jsval tmp;
        JS_GetElement(cx, obj, i, &tmp);
        if(!this->member->JStoC(cx, data + elsize * i, tmp)) return JS_FALSE;
      }
      return JS_TRUE;
    }

    JsciPointer *ptr = jsval_to_JsciPointer(cx, v);
    if(ptr) {
      // Copy contents pointed to into array
      int size = this->SizeInBytes();
      memcpy(data, ptr->ptr, size);
      return JS_TRUE;
    }
  }

  return JSX_ReportException(cx, "Cannot convert JS value to C array");
}


int JsciTypeArray::SizeInBytes() {
  return this->length * this->member->SizeInBytes();
}


int JsciTypeArray::AlignmentInBytes() {
  return this->member->AlignmentInBytes();
}
