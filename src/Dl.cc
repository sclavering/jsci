#include <stdarg.h>
#include <dlfcn.h>
#include <string.h>
#include "jsci.h"


static JSBool Dl_new(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
static void Dl_finalize(JSContext *cx, JSObject *obj);
static JSBool Dl_proto_symbolExists(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
static JSBool Dl_proto_pointer(JSContext *cx, JSObject *dl, uintN argc, jsval *argv, jsval *rval);

static JSClass JSEXT_dl_class = {
    "Dl",
    JSCLASS_HAS_PRIVATE,
    JS_PropertyStub,
    JS_PropertyStub,
    JS_PropertyStub,
    JS_PropertyStub,
    JS_EnumerateStub,
    JS_ResolveStub,
    JS_ConvertStub,
    Dl_finalize
};


jsval make_Dl(JSContext *cx, JSObject *glob) {
  JSObject *JSEXT_dl_proto=0;
  static struct JSFunctionSpec memberfunc[]={
    JS_FS("symbolExists", Dl_proto_symbolExists, 1, 0, 0),
    JS_FS("pointer", Dl_proto_pointer, 2, 0, 0),
    {0,0,0,0,0}
  };

  JSEXT_dl_proto = JS_InitClass(cx, glob, NULL, &JSEXT_dl_class, &Dl_new, 1, NULL, memberfunc, NULL, NULL);
  if(JSEXT_dl_proto == 0) return JSVAL_VOID;
  JS_SetPrivate(cx, JSEXT_dl_proto, NULL);

  return OBJECT_TO_JSVAL(JS_GetConstructor(cx, JSEXT_dl_proto));
}


static JSBool Dl_new(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
  char *filename;

  if((argc >= 1 && (JSVAL_IS_NULL(argv[0]) || JSVAL_IS_VOID(argv[0]))) || argc == 0) {
    filename = 0;
  } else if(!JS_ConvertArguments(cx, argc, argv, "s", &filename)) {
    JSX_ReportException(cx, "Wrong parameter for dl");
    return JS_FALSE;
  }
  if(filename && filename[0] == 0) filename = 0;

  // treat "Dl(...)" the same as "new Dl(...)"
  if(!JS_IsConstructing(cx)) {
    obj = JS_NewObject(cx, &JSEXT_dl_class, 0, 0);
    *rval = OBJECT_TO_JSVAL(obj);
  }

  void *dl;
  if(filename) {
    dl = dlopen(filename, RTLD_LAZY | RTLD_GLOBAL);
  } else {
    dl = dlopen(0, RTLD_LAZY);
  }
  JS_SetPrivate(cx, obj, dl);

  if(!dl) {
    JSX_ReportException(cx, "Dl: can't open %s", dlerror());
    return JS_FALSE;
  }

  return JS_TRUE;
}


static JSBool Dl_proto_symbolExists(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
  if(argc != 1) return JSX_ReportException(cx, "Wrong number of arguments");
  char *symbol;
  if(!JS_ConvertArguments(cx, argc, argv, "s", &symbol)) return JS_FALSE;
  *rval = dlsym((void *) JS_GetPrivate(cx, obj), symbol) != 0 ? JSVAL_TRUE : JSVAL_FALSE;
  return JS_TRUE;
}


static void Dl_finalize(JSContext *cx, JSObject *obj) {
  void *dl = JS_GetPrivate(cx, obj);
  if(dl) dlclose(dl);
}


static JSBool Dl_proto_pointer(JSContext *cx, JSObject *dl, uintN argc, jsval *argv, jsval *rval) {
  if(!JSVAL_IS_STRING(argv[0])) return JSX_ReportException(cx, "Dl.prototype.pointer(): first argument must be a string");
  JsciType *t = jsval_to_JsciType(cx, argv[1]);
  if(!t) return JSX_ReportException(cx, "Dl.prototype.pointer(): second argument must be a Type instance");

  void *sym = dlsym(JS_GetPrivate(cx, dl), JS_GetStringBytes(JSVAL_TO_STRING(argv[0])));
  if(!sym) return JSX_ReportException(cx, "Dl.prototype.pointer(): couldn't resolve symbol");

  return WrapPointerAndSaveType(cx, new JsciPointer(t, sym), rval, argv[1]);
}
