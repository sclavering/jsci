#include <jsapi.h>
#include <stdarg.h>

#define __declspec(x)

#include "Type.h"
#include "Pointer.h"

# include <dlfcn.h>

#include <string.h>


static JSBool JS_DLL_CALLBACK JSX_dl_call(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
static JSBool dl_new(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
static void JS_DLL_CALLBACK JSEXT_dl_finalize(JSContext *cx, JSObject *obj);
static JSBool JS_DLL_CALLBACK JSX_dl_symbolExists(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
static JSBool JS_DLL_CALLBACK JSX_dl_function(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);

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

static JSBool JSX_ReportException(JSContext *cx, char *format, ...) {
  int len;
  char *msg;
  JSString *Str;
  va_list va;
  jsval str;

  va_start(va, format);
  msg=JS_malloc(cx, 801);
  va_start(va, format);
  len=vsnprintf(msg,800,format,va);
  if (len>800) len=800;
  msg[len]=0;
  va_end(va);
  Str=JS_NewString(cx, msg, len);
  str=STRING_TO_JSVAL(Str);
  JS_SetPendingException(cx, str);

  return JS_FALSE;
}

static JSBool JSX_dl_pointer(JSContext *cx, JSObject *origobj, uintN argc, jsval *argv, jsval *rval) {
  JSObject *obj;

  if (argc>=2 &&
      JSVAL_IS_STRING(argv[0]) &&
      JSVAL_IS_OBJECT(argv[1]) &&
      !JSVAL_IS_NULL(argv[1]) &&
      JS_InstanceOf(cx, JSVAL_TO_OBJECT(argv[1]), JSX_GetTypeClass(), NULL)) {

    // Lazy resolution constructor

    char *symbol=JS_GetStringBytes(JSVAL_TO_STRING(argv[0]));

    obj=JS_NewObject(cx, JSX_GetPointerClass(), 0, 0);
    *rval=OBJECT_TO_JSVAL(obj);

    if (!JSX_InitPointer(cx, obj, JSVAL_TO_OBJECT(argv[1]))) {
      return JS_FALSE;
    }
    if (!JS_DefineProperty(cx, obj, "dl", OBJECT_TO_JSVAL(origobj), 0, 0, JSPROP_ENUMERATE | JSPROP_READONLY | JSPROP_PERMANENT)) {
      return JS_FALSE;
    }
    if (!JS_DefineProperty(cx, obj, "symbol", argv[0], 0, 0, JSPROP_ENUMERATE | JSPROP_READONLY | JSPROP_PERMANENT)) {
      return JS_FALSE;
    }
    return JS_TRUE;
  }

  JS_ReportError(cx, "Wrong arguments to dl.pointer");
  return JS_FALSE;
  
}


__declspec(dllexport)
JSBool JSX_init(JSContext *cx, JSObject *glob, uintN argc, jsval *argv, jsval *rval) {
  JSObject *JSEXT_dl_proto=0;
  static struct JSFunctionSpec memberfunc[]={
    {"symbolExists", JSX_dl_symbolExists, 1, 0, 0},
    //    {"call", JSX_dl_call, 2, 0, 0},
    {"function", JSX_dl_function, 1, 0, 0},
    {"pointer",JSX_dl_pointer,2,0,0},
    {0,0,0,0,0}
  };

  JSEXT_dl_proto=JS_InitClass(cx, glob, NULL, &JSEXT_dl_class, dl_new, 1, NULL, memberfunc, NULL, NULL);
  if (JSEXT_dl_proto==0)
    return JS_FALSE;
  JS_SetPrivate(cx, JSEXT_dl_proto, NULL);

  *rval=OBJECT_TO_JSVAL(JS_GetConstructor(cx, JSEXT_dl_proto));

  return JS_TRUE;
}
  
static JSBool
JSEXT_dl_new(JSContext *cx, JSObject *obj, char *filename, jsval *rval) {
  JSObject *object;
  JSNative JSEXT_init=0;
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

  if (filename)
	  JSEXT_init=(JSNative) dlsym(dl,"JSEXT_init");

  if (JSEXT_init) { // Call init with filename as only argument
    JSString *tmpstr=0;
    jsval argv[1];
    jsval rval=JSVAL_VOID;

    tmpstr = JS_NewStringCopyZ(cx, filename);
    JS_AddRoot(cx, &tmpstr);
    argv[0]=STRING_TO_JSVAL(tmpstr);

    (*JSEXT_init)(cx, obj, 1, argv, &rval);

    JS_RemoveRoot(cx, &tmpstr);
  }

  if (filename) {
    JSfilename = JS_NewStringCopyZ(cx, filename);
    JS_DefineProperty(cx, object, "filename", STRING_TO_JSVAL(JSfilename), 0, 0, JSPROP_READONLY | JSPROP_PERMANENT);
  } else {
    JS_DefineProperty(cx, object, "filename", JSVAL_VOID, 0, 0, JSPROP_READONLY | JSPROP_PERMANENT);
  }

  return JS_TRUE;
}

static JSBool
dl_new(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
  char *filename;
  JSBool res;

  if ((argc>=1 && (JSVAL_IS_NULL(argv[0]) || JSVAL_IS_VOID(argv[0]))) || argc==0)
	filename=0;
  else if (!JS_ConvertArguments(cx, argc, argv, "s", &filename)) {
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

  if (dlsym((void *)JS_GetPrivate(cx, obj),symbol)!=0)
	*rval=JSVAL_TRUE;
  else
	  *rval=JSVAL_FALSE;

  return JS_TRUE;
}


static JSBool JS_DLL_CALLBACK JSX_dl_call(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
  JSNative fun;
  char *name;
  JSObject *desobj;

  if (argc<1) {
    JSX_ReportException(cx, "Too few arguments");
    return JS_FALSE;
  }

  if (!JS_ConvertArguments(cx, argc, argv, "s", &name)) {
    return JS_FALSE;
  }

  if (argc<2)
    desobj=JS_GetGlobalObject(cx);
  else {
    if (!JSVAL_IS_OBJECT(argv[1]) ||
	JSVAL_IS_NULL(argv[1])) {
      JSX_ReportException(cx, "Illegal 'this' object");
      return JS_FALSE;
    }
    desobj=JSVAL_TO_OBJECT(argv[1]);
    argc--;
    argv++;
  }

  if (dlsym((void *)JS_GetPrivate(cx, obj),(void *)fun)!=0)
    *rval=JSVAL_TRUE;
  else
    *rval=JSVAL_FALSE;

  return fun(cx, obj, argc-1, argv+1, rval);
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
  void *dl=JS_GetPrivate(cx, obj);
  if (!dl) return;
  //  dlclose(dl);
}

