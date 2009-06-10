#include <string.h>
#include "jsci.h"


// Setting to undefined does nothing, only returns sizeof.
int JSX_Set(JSContext *cx, char *p, int will_clean, JsciType *type, jsval v) {
  if(!type) return JSX_ReportException(cx, "Cannot convert JS value to C value, because the C type is not known");
  if(v == JSVAL_VOID) return type->SizeInBytes();
  return type->JStoC(cx, p, v, will_clean);
}
