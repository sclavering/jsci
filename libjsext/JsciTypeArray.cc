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


int JsciTypeArray::JStoC(JSContext *cx, char *data, jsval v, int will_clean) {
  switch(JSX_JSType(cx, v)) {
    // Copy a string to a char array
    case JSVAL_STRING: {
      if(!type_is_char(this->member)) return JSX_ReportException(cx, "Could not convert JS string to a C array of non-chars");
      int size = JS_GetStringLength(JSVAL_TO_STRING(v));
      if(size < this->length) {
        memcpy(*(char **)data, JS_GetStringBytes(JSVAL_TO_STRING(v)), size * sizeof(char));
        memset(*(char **)data + size, 0, (this->length - size) * sizeof(char));
      } else {
        memcpy(*(char **)data, JS_GetStringBytes(JSVAL_TO_STRING(v)), this->length * sizeof(char));
      }
      return sizeof(char) * this->length;
    }
    // Copy array elements to a fixed size array
    case JSARRAY: {
      int totsize = 0;
      JSObject *obj = JSVAL_TO_OBJECT(v);
      for(int i = 0; i != this->length; ++i) {
        jsval tmp;
        JS_GetElement(cx, obj, i, &tmp);
        int thissize = JSX_Set(cx, data + totsize, will_clean, this->member, tmp);
        if(!thissize) return 0;
        totsize += thissize;
      }
      return totsize;
    }
    case JSPOINTER: {
      // Copy contents pointed to into array
      int size = this->SizeInBytes();
      JsciPointer *ptr = (JsciPointer *) JS_GetPrivate(cx, JSVAL_TO_OBJECT(v));
      memcpy(data, ptr->ptr, size);
      return size;
    }
    case JSNULL: {
      int size = this->SizeInBytes();
      memset(data, 0, size);
      return size;
    }
  }
  return JSX_ReportException(cx, "Could not convert JS value to C array");
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
