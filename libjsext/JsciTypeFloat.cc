#include "jsci.h"


JsciTypeFloat::JsciTypeFloat(int size, ffi_type ffit) : JsciTypeNumeric(FLOATTYPE, size, ffit) {
}


int JsciTypeFloat::CtoJS(JSContext *cx, char *data, jsval *rval) {
  jsdouble tmpdouble = 0;
  switch(this->size) {
    case 0: tmpdouble = *(float *) data; break;
    case 1: tmpdouble = *(double *) data; break;
    // case 2: tmpdouble = *(long double *)p; break;
    default:
      return JSX_ReportException(cx, "Could not convert C float of unknown size to a javascript value");
  }
  JS_NewDoubleValue(cx, tmpdouble, rval);
  return 1;
}
