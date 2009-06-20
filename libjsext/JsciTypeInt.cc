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
      return JSX_ReportException(cx, "Cannot convert C signed integer of unknown size to a javascript value");
  }
  if(INT_FITS_IN_JSVAL(tmp)) {
    *rval = INT_TO_JSVAL(tmp);
  } else {
    jsdouble d = (jsdouble) tmp;
    JS_NewDoubleValue(cx, d, rval);
  }
  return 1;
}


JSBool JsciTypeInt::JStoC(JSContext *cx, char *data, jsval v) {
  int tmpint;
  if(!this->JsToInt(cx, v, &tmpint)) return JS_FALSE;
  switch(this->size) {
    case 0:
      *(char *)data = tmpint;
      return JS_TRUE;
    case 1:
      *(short *)data = tmpint;
      return JS_TRUE;
    case 2:
      *(int *)data = tmpint;
      return JS_TRUE;
    case 3:
      *(long *)data = tmpint;
      return JS_TRUE;
    case 4:
      *(long long *)data = tmpint;
      return JS_TRUE;
    case 5:
      *(int64 *)data = tmpint;
      return JS_TRUE;
  }
  return JSX_ReportException(cx, "Cannot convert JS value to a C integer");
}


JSBool JsciTypeInt::JsToInt(JSContext *cx, jsval v, int *rv) {
  if(JSVAL_IS_INT(v)) { *rv = JSVAL_TO_INT(v); return JS_TRUE; }
  if(JSVAL_IS_DOUBLE(v)) { *rv = (int) *JSVAL_TO_DOUBLE(v); return JS_TRUE; }
  if(JSVAL_IS_BOOLEAN(v)) { *rv = v == JSVAL_TRUE ? 1 : 0; return JS_TRUE; }
  if(v == JSVAL_NULL) { *rv = 0; return JS_TRUE; }
  return JSX_ReportException(cx, "Cannot convert JS value of unrecognised type to a C integer");
}
