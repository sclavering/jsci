#include "jsci.h"


JsciTypeUint::JsciTypeUint(int size, ffi_type ffit) : JsciTypeNumeric(UINTTYPE, size, ffit) {
}


int JsciTypeUint::CtoJS(JSContext *cx, char *data, jsval *rval) {
  int tmp;
  // Return a number from an unsigned int (of various sizes)
  switch(this->size) {
    case 0: tmp = *(unsigned char *) data; break;
    case 1: tmp = *(unsigned short *) data; break;
    case 2: tmp = *(unsigned int *) data; break;
    case 3: tmp = *(unsigned long *) data; break;
    case 4: tmp = *(unsigned long long *) data; break;
    case 5: tmp = *(int64 *) data; break;
    default:
      return JSX_ReportException(cx, "Could not convert C unsigned integer of unknown size to a javascript value");
  }
  if(INT_FITS_IN_JSVAL(tmp)) {
    *rval = INT_TO_JSVAL(tmp);
  } else {
    jsdouble d = (jsdouble) tmp;
    JS_NewDoubleValue(cx, d, rval);
  }
  return 1;
}
