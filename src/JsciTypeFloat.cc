#include "jsci.h"


JsciTypeFloat::JsciTypeFloat() : JsciTypeNumeric(ffi_type_float) {
}


JSBool JsciTypeFloat::CtoJS(JSContext *cx, char *data, jsval *rval) {
  return JS_NewNumberValue(cx, *(float*) data, rval);
}


JSBool JsciTypeFloat::JStoC(JSContext *cx, char *data, jsval v) {
  jsdouble tmpdouble;
  if(!JS_ValueToNumber(cx, v, &tmpdouble)) return JS_FALSE;
  *(float*)data = tmpdouble;
  return JS_TRUE;
}



JsciTypeDouble::JsciTypeDouble() : JsciTypeNumeric(ffi_type_double) {
}


JSBool JsciTypeDouble::CtoJS(JSContext *cx, char *data, jsval *rval) {
  return JS_NewNumberValue(cx, *(double*) data, rval);
}


JSBool JsciTypeDouble::JStoC(JSContext *cx, char *data, jsval v) {
  return JS_ValueToNumber(cx, v, (double*) data);
}
