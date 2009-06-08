#include "jsci.h"


JsciTypeInt::JsciTypeInt(int size, ffi_type ffit) : JsciTypeNumeric(INTTYPE, size, ffit) {
}


int JsciTypeInt::CtoJS(JSContext *cx, char *data, jsval *rval) {
  int tmp;
  // Return a number from an unsigned int (of various sizes)
  switch(this->size) {
    case 0: tmp = *(char *) data; break;
    case 1: tmp = *(short *) data; break;
    case 2: tmp = *(int *) data; break;
    case 3: tmp = *(long *) data; break;
    case 4: tmp = *(long long *) data; break;
    case 5: tmp = *(int64 *) data; break;
    default:
      return JSX_ReportException(cx, "Could not convert C signed integer of unknown size to a javascript value");
  }
  if(INT_FITS_IN_JSVAL(tmp)) {
    *rval = INT_TO_JSVAL(tmp);
  } else {
    jsdouble d = (jsdouble) tmp;
    JS_NewDoubleValue(cx, d, rval);
  }
  return 1;
}
