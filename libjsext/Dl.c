#include <jsapi.h>
#include <stdarg.h>
#include "Type.h"
#include "Pointer.h"
#include <dlfcn.h>
#include <string.h>
#include "util.h"


static JSBool dl_new(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
static void JS_DLL_CALLBACK JSEXT_dl_finalize(JSContext *cx, JSObject *obj);
static JSBool JS_DLL_CALLBACK JSX_dl_symbolExists(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
static JSBool JS_DLL_CALLBACK JSX_dl_function(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
static JSBool JSX_dl_pointer(JSContext *cx, JSObject *dl, uintN argc, jsval *argv, jsval *rval);

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
    JSEXT_dl_finalize
};


jsval JSX_make_Dl(JSContext *cx, JSObject *glob) {
  JSObject *JSEXT_dl_proto=0;
  static struct JSFunctionSpec memberfunc[]={
    {"symbolExists", JSX_dl_symbolExists, 1, 0, 0},
    {"function", JSX_dl_function, 1, 0, 0},
    {"pointer", JSX_dl_pointer, 2, 0, 0},
    {0,0,0,0,0}
  };

  JSEXT_dl_proto=JS_InitClass(cx, glob, NULL, &JSEXT_dl_class, dl_new, 1, NULL, memberfunc, NULL, NULL);
  if(JSEXT_dl_proto == 0) return JSVAL_VOID;
  JS_SetPrivate(cx, JSEXT_dl_proto, NULL);

  return OBJECT_TO_JSVAL(JS_GetConstructor(cx, JSEXT_dl_proto));
}


static JSBool
JSEXT_dl_new(JSContext *cx, JSObject *obj, char *filename, jsval *rval) {
  JSObject *object;
  JSString *JSfilename;

  if (!JS_IsConstructing(cx)) { // not called with new
    object=JS_NewObject( cx, &JSEXT_dl_class, 0, 0);
    *rval=OBJECT_TO_JSVAL(object);
  } else {
    object=obj;
  }

  void *dl;
    
  if (filename)
    dl=dlopen(filename, RTLD_LAZY | RTLD_GLOBAL);
  else
    dl=dlopen(0,RTLD_LAZY);

  JS_SetPrivate(cx, object, dl);

  if (!dl) {
    return JS_FALSE;
  }

  if (filename) {
    JSfilename = JS_NewStringCopyZ(cx, filename);
    JS_DefineProperty(cx, object, "filename", STRING_TO_JSVAL(JSfilename), 0, 0, JSPROP_READONLY | JSPROP_PERMANENT);
  } else {
    JS_DefineProperty(cx, object, "filename", JSVAL_VOID, 0, 0, JSPROP_READONLY | JSPROP_PERMANENT);
  }

  return JS_TRUE;
}


static JSBool dl_new(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
  char *filename;
  JSBool res;

  if((argc >= 1 && (JSVAL_IS_NULL(argv[0]) || JSVAL_IS_VOID(argv[0]))) || argc == 0) {
    filename = 0;
  } else if(!JS_ConvertArguments(cx, argc, argv, "s", &filename)) {
    JSX_ReportException(cx, "Wrong parameter for dl");
    return JS_FALSE;
  }
  if (filename && filename[0]==0)
    filename=0;

  res=JSEXT_dl_new(cx, obj, filename, rval);
  if (res==JS_FALSE) {
    JSX_ReportException(cx, "can't open dl %s", dlerror());
    return JS_FALSE;
  }

  return JS_TRUE;
}


static JSBool JS_DLL_CALLBACK JSX_dl_symbolExists(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
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


static JSBool JS_DLL_CALLBACK JSX_dl_function(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
  JSNative fun;
  JSFunction *jsfun;
  char *name;

  if (argc<1) {
    JSX_ReportException(cx, "Too few arguments");
    return JS_FALSE;
  }

  if (!JS_ConvertArguments(cx, argc, argv, "s", &name)) {
    JSX_ReportException(cx, "Illegal arguments");
    return JS_FALSE;
  }

  fun=dlsym((void *)JS_GetPrivate(cx, obj),name);
  
  if (!fun) {
    JSX_ReportException(cx, "Unknown symbol '%s'",name);
    return JS_FALSE;
  }

  jsfun=JS_NewFunction(cx, fun, 0, 0, 0, name);
  *rval=OBJECT_TO_JSVAL(JS_GetFunctionObject(jsfun));
  
  return JS_TRUE;
}


static void JS_DLL_CALLBACK JSEXT_dl_finalize(JSContext *cx, JSObject *obj) {
  void *dl = JS_GetPrivate(cx, obj);
  if(dl) dlclose(dl);
}


static JSBool JSX_dl_pointer(JSContext *cx, JSObject *dl, uintN argc, jsval *argv, jsval *rval) {
  if(argc < 2 || !JSVAL_IS_STRING(argv[0]) || !JSVAL_IS_OBJECT(argv[1]) || JSVAL_IS_NULL(argv[1]) || !JS_InstanceOf(cx, JSVAL_TO_OBJECT(argv[1]), JSX_GetTypeClass(), NULL)) {
    JS_ReportError(cx, "Dl.prototype.pointer(): bad arguments");
    return JS_FALSE;
  }

  JSObject *obj;
  obj = JS_NewObject(cx, JSX_GetPointerClass(), 0, 0);
  *rval = OBJECT_TO_JSVAL(obj);

  if(!JSX_InitPointer(cx, obj, JSVAL_TO_OBJECT(argv[1]))) return JS_FALSE;

  JSX_Pointer *ptr;
  ptr = (JSX_Pointer *) JS_GetPrivate(cx, obj);
  if(!ptr) return JS_FALSE;

  ptr->ptr = (void *) dlsym(JS_GetPrivate(cx, dl), JS_GetStringBytes(JSVAL_TO_STRING(argv[0])));

  if(!ptr->ptr) {
    JSX_ReportException(cx, "Dl.prototype.pointer(): couldn't resolve symbol");
    return JS_FALSE;
  }

  return JS_TRUE;
}
