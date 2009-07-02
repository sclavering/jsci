#include <string.h>
#include "jsci.h"


JsciTypePointer::JsciTypePointer() : JsciType(POINTERTYPE) {
}


int JsciTypePointer::CtoJS(JSContext *cx, char *data, jsval *rval) {
  if(*(void **)data == NULL) {
    *rval = JSVAL_NULL;
    return 1;
  }

  JSObject *obj = JS_NewObject(cx, JSX_GetPointerClass(), 0, 0);
  *rval = OBJECT_TO_JSVAL(obj);
  JsciPointer *ptr = new JsciPointer(this->direct);
  ptr->ptr = *(void **) data;
  ptr->finalize = 0;
  JS_SetPrivate(cx, obj, ptr);
  return 1;
}


JSBool JsciTypePointer::JStoC(JSContext *cx, char *data, jsval v) {
  if(JSVAL_IS_NULL(v)) {
    *(void **)data = NULL;
    return JS_TRUE;
  }

  // Copy a string to a void* (same as char* in this context)
  if(JSVAL_IS_STRING(v)) {
    if(!is_void_or_char(this->direct)) return JSX_ReportException(cx, "Cannot convert JS string to C non-char non-void pointer type");
    *(char **)data = JS_GetStringBytes(JSVAL_TO_STRING(v));
    return JS_TRUE;
  }

  if(JSVAL_IS_OBJECT(v)) {
    JSObject *obj = JSVAL_TO_OBJECT(v);

    if(JS_ObjectIsFunction(cx, obj)) {
      if(this->direct->type != FUNCTIONTYPE) return JSX_ReportException(cx, "Cannot convert JS function to C non-function pointer type");
      JsciCallback *cb = JsciCallback::GetForFunction(cx, v, (JsciTypeFunction*) this->direct);
      *(void **)data = cb->codeptr();
      return JS_TRUE;
    }

    // Copy a pointer object to a type *
    if(JS_InstanceOf(cx, obj, JSX_GetPointerClass(), NULL)) {
      JsciPointer *ptr = (JsciPointer *) JS_GetPrivate(cx, obj);
      *(void **)data = ptr->ptr;
      return JS_TRUE;
    }

    // Copy array elements to a variable array
    if(JS_IsArrayObject(cx, obj)) {
      jsuint size;
      JS_GetArrayLength(cx, obj, &size);
      int elemsize = this->direct->SizeInBytes();

      for(jsuint i = 0; i != size; ++i) {
        jsval tmp;
        JS_GetElement(cx, obj, i, &tmp);
        if(!this->direct->JStoC(cx, *(char **)data + i * elemsize, tmp)) {
          delete data;
          return JS_FALSE;
        }
      }
      return JS_TRUE;
    }
  }

  return JSX_ReportException(cx, "Cannot convert JS value to C pointer type");
}


ffi_type *JsciTypePointer::GetFFIType() {
  return &ffi_type_pointer;
}


int JsciTypePointer::SizeInBytes() {
  return ffi_type_pointer.size;
}


int JsciTypePointer::AlignmentInBytes() {
  return ffi_type_pointer.alignment;
}
