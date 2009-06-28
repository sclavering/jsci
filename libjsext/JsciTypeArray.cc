#include <string.h>
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


JSBool JsciTypeArray::JStoC(JSContext *cx, char *data, jsval v) {
  if(JSVAL_IS_NULL(v)) {
    int size = this->SizeInBytes();
    memset(data, 0, size);
    return JS_TRUE;
  }

  if(JSVAL_IS_STRING(v)) {
      // Copy a string to a char array
      if(!type_is_char(this->member)) return JSX_ReportException(cx, "Cannot convert JS string to a C array of non-chars");
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

    if(JS_InstanceOf(cx, obj, JSX_GetPointerClass(), NULL)) {
      // Copy contents pointed to into array
      int size = this->SizeInBytes();
      JsciPointer *ptr = (JsciPointer *) JS_GetPrivate(cx, obj);
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


JSBool JsciTypeArray::ContainsPointer() {
  return this->member->ContainsPointer();
}
