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


int JsciTypeInt::JStoC(JSContext *cx, char *data, jsval v, int will_clean) {
  int tmpint;
  if(!this->JsToInt(cx, v, &tmpint)) return 0;
  switch(this->size) {
    case 0:
      *(char *)data = tmpint;
      return sizeof(char);
    case 1:
      *(short *)data = tmpint;
      return sizeof(short);
    case 2:
      *(int *)data = tmpint;
      return sizeof(int);
    case 3:
      *(long *)data = tmpint;
      return sizeof(long);
    case 4:
      *(long long *)data = tmpint;
      return sizeof(long long);
    case 5:
      *(int64 *)data = tmpint;
      return sizeof(int64);
  }
  return JSX_ReportException(cx, "Cannot convert JS value to a C integer");
}


JSBool JsciTypeInt::JsToInt(JSContext *cx, jsval v, int *rv) {
  switch(JSX_JSType(cx, v)) {
    case JSVAL_DOUBLE: *rv = (int) *JSVAL_TO_DOUBLE(v); return JS_TRUE;
    case JSVAL_BOOLEAN: *rv = v == JSVAL_TRUE ? 1 : 0; return JS_TRUE;
    case JSVAL_INT: *rv = JSVAL_TO_INT(v); return JS_TRUE;
    case JSNULL: *rv = 0; return JS_TRUE;
  }
  return JSX_ReportException(cx, "Cannot convert JS value to a C float/double");
}
