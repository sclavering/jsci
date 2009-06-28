#include "jsci.h"


JsciTypeFloat::JsciTypeFloat(int size, ffi_type ffit) : JsciTypeNumeric(FLOATTYPE, size, ffit) {
}


int JsciTypeFloat::CtoJS(JSContext *cx, char *data, jsval *rval) {
  jsdouble tmpdouble = 0;
  switch(this->size) {
    case 0: tmpdouble = *(float *) data; break;
    case 1: tmpdouble = *(double *) data; break;
    default:
      return JSX_ReportException(cx, "Cannot convert C float of unknown size to a javascript value");
  }
  JS_NewDoubleValue(cx, tmpdouble, rval);
  return 1;
}


JSBool JsciTypeFloat::JStoC(JSContext *cx, char *data, jsval v) {
  jsdouble tmpdouble;
  if(!this->JsToDouble(cx, v, &tmpdouble)) return JS_FALSE;
  switch(this->size) {
    case 0:
      *(float *)data = tmpdouble;
      return JS_TRUE;
    case 1:
      *(double *)data = tmpdouble;
      return JS_TRUE;
  }
  return JSX_ReportException(cx, "Cannot convert JS value to a C float/double");
}


JSBool JsciTypeFloat::JsToDouble(JSContext *cx, jsval v, jsdouble *rv) {
  if(JSVAL_IS_DOUBLE(v)) { *rv = *JSVAL_TO_DOUBLE(v); return JS_TRUE; }
  if(JSVAL_IS_INT(v)) { *rv = (jsdouble) JSVAL_TO_INT(v); return JS_TRUE; }
  if(JSVAL_IS_BOOLEAN(v)) { *rv = v == JSVAL_TRUE ? 1.0 : 0.0; return JS_TRUE; }
  if(JSVAL_IS_NULL(v)) { *rv = 0; return JS_TRUE; }
  return JSX_ReportException(cx, "Cannot convert JS value to a C float/double");
}
