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
  JsciPointer *ptr = new JsciPointer;
  ptr->ptr = *(void **) data;
  ptr->type = this->direct;
  ptr->finalize = 0;
  JS_SetPrivate(cx, obj, ptr);
  return 1;
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


JSBool JsciTypePointer::ContainsPointer() {
  return JS_TRUE;
}
