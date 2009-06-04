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


extern "C" jsval make_Dl(JSContext *cx, JSObject *glob) {
  JSObject *JSEXT_dl_proto=0;
  static struct JSFunctionSpec memberfunc[]={
    {"symbolExists", Dl_proto_symbolExists, 1, 0, 0},
    {"pointer", Dl_proto_pointer, 2, 0, 0},
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
  char *symbol;

  if (argc!=1) {
    JSX_ReportException(cx, "Wrong number of arguments");
    return JS_FALSE;
  }

  if (!JS_ConvertArguments(cx, argc, argv, "s", &symbol)) {
    return JS_FALSE;
  }

  *rval = dlsym((void *) JS_GetPrivate(cx, obj), symbol) != 0 ? JSVAL_TRUE : JSVAL_FALSE;

  return JS_TRUE;
}


static void Dl_finalize(JSContext *cx, JSObject *obj) {
  void *dl = JS_GetPrivate(cx, obj);
  if(dl) dlclose(dl);
}


static JSBool Dl_proto_pointer(JSContext *cx, JSObject *dl, uintN argc, jsval *argv, jsval *rval) {
  if(!JSVAL_IS_STRING(argv[0])) {
    JS_ReportError(cx, "Dl.prototype.pointer(): first argument must be a string");
    return JS_FALSE;
  }
  if(!JSVAL_IS_OBJECT(argv[1]) || JSVAL_IS_NULL(argv[1]) || !JS_InstanceOf(cx, JSVAL_TO_OBJECT(argv[1]), JSX_GetTypeClass(), NULL)) {
    JS_ReportError(cx, "Dl.prototype.pointer(): second argument must be a Type instance");
    return JS_FALSE;
  }

  void *sym = dlsym(JS_GetPrivate(cx, dl), JS_GetStringBytes(JSVAL_TO_STRING(argv[0])));
  if(!sym) {
    JSX_ReportException(cx, "Dl.prototype.pointer(): couldn't resolve symbol");
    return JS_FALSE;
  }

  JSObject *obj;
  obj = JS_NewObject(cx, JSX_GetPointerClass(), 0, 0);
  *rval = OBJECT_TO_JSVAL(obj);
  if(!JSX_InitPointer(cx, obj, JSVAL_TO_OBJECT(argv[1]))) return JS_FALSE;
  JSX_Pointer *ptr;
  ptr = (JSX_Pointer *) JS_GetPrivate(cx, obj);
  if(!ptr) return JS_FALSE;
  ptr->ptr = sym;
  return JS_TRUE;
}
