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
  JsciPointer *ptr = new JsciPointer;
  ptr->ptr = *(void **) data;
  ptr->type = this->direct;
  ptr->finalize = 0;
  JS_SetPrivate(cx, obj, ptr);
  return 1;
}


int JsciTypePointer::JStoC(JSContext *cx, char *data, jsval v) {
  switch(JSX_JSType(cx, v)) {
    case JSFUNC: {
      if(this->direct->type != FUNCTIONTYPE) return JSX_ReportException(cx, "Cannot convert JS function to C non-function pointer type");
      jsval tmpval = JSVAL_VOID;
      JSFunction *fun = JS_ValueToFunction(cx, v);
      JSObject *obj = JS_GetFunctionObject(fun);
      JS_GetProperty(cx, obj, "__ptr__", &tmpval);
      if(tmpval == JSVAL_VOID) {
        // Create pointer
        JSObject *newptr = JS_NewObject(cx, JSX_GetPointerClass(), 0, 0);
        tmpval = OBJECT_TO_JSVAL(newptr);
        JS_DefineProperty(cx, obj, "__ptr__", tmpval, 0, 0, JSPROP_READONLY | JSPROP_PERMANENT);
        if(!JSX_InitPointerCallback(cx, newptr, fun, this->direct)) return 0;
      }
      v = tmpval;
      JsciPointer *ptr = (JsciPointer *) JS_GetPrivate(cx, JSVAL_TO_OBJECT(v));
      *(void **)data = ptr->ptr;
      return sizeof(void *);
    }

    // Copy a pointer object to a type *
    case JSPOINTER: {
      JsciPointer *ptr = (JsciPointer *) JS_GetPrivate(cx, JSVAL_TO_OBJECT(v));
      *(void **)data = ptr->ptr;
      return sizeof(void *);
    }

    // Copy a null to a type *
    case JSNULL: {
      *(void **)data = NULL;
      return sizeof(void *);
    }

    // Copy a string to a void* (same as char* in this context)
    case JSVAL_STRING: {
      if(!is_void_or_char(this->direct)) return JSX_ReportException(cx, "Cannot convert JS string to C non-char non-void pointer type");
      *(char **)data = JS_GetStringBytes(JSVAL_TO_STRING(v));
      return sizeof(char *);
    }

    // Copy array elements to a variable array
    case JSARRAY: {
      JSObject *obj = JSVAL_TO_OBJECT(v);
      jsuint size;
      JS_GetArrayLength(cx, obj, &size);
      int elemsize = this->direct->SizeInBytes();

      for(jsuint i = 0; i != size; ++i) {
        jsval tmp;
        JS_GetElement(cx, obj, i, &tmp);
        if(!this->direct->JStoC(cx, *(char **)data + i * elemsize, tmp)) {
          delete data;
          return 0;
        }
      }
      return sizeof(void *);
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


JSBool JsciTypePointer::ContainsPointer() {
  return JS_TRUE;
}
