# define _GNU_SOURCE
# include <getopt.h>
# include <unistd.h>
extern char **environ;
#include <stdlib.h>

#include <stdarg.h>
#include "util.h"


static JSBool env_setProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
  return JSX_ReportException(cx, "You cannot modify environment variables via the 'environment' object");
}


static JSBool env_enumerate(JSContext *cx, JSObject *obj) {
  static JSBool reflected;
  if(reflected) return JS_TRUE;

  char **evp = environ; // (char **) JS_GetPrivate(cx, obj);
  char *name;
  for( ; (name = *evp) != NULL; evp++) {
    char *value = name;
    while(*value && *value != '=') value++;
    if(!*value) continue;
    *value++ = '\0';
    JSString *valstr = JS_NewStringCopyZ(cx, value);
    JSBool ok;
    if(!valstr) {
      ok = JS_FALSE;
    } else {
      ok = JS_DefineProperty(cx, obj, name, STRING_TO_JSVAL(valstr), NULL, NULL, JSPROP_ENUMERATE);
    }
    value[-1] = '=';
    if(!ok) return JS_FALSE;
  }
  reflected = JS_TRUE;

  return JS_TRUE;
}


static JSBool env_resolve(JSContext *cx, JSObject *obj, jsval id, uintN flags, JSObject **objp) {
  if(flags & JSRESOLVE_ASSIGNING) return JS_TRUE;
  JSString *idstr = JS_ValueToString(cx, id);
  if(!idstr) return JS_FALSE;
  char *name = JS_GetStringBytes(idstr);
  char *value = getenv(name);
  if(value) {
    JSString *valstr = JS_NewStringCopyZ(cx, value);
    if(!valstr || !JS_DefineProperty(cx, obj, name, STRING_TO_JSVAL(valstr), NULL, NULL, JSPROP_ENUMERATE)) return JS_FALSE;
    *objp = obj;
  }
  return JS_TRUE;
}


static JSClass jsext_env_class = {
  "environment",
  JSCLASS_NEW_RESOLVE | JSCLASS_NEW_RESOLVE_GETS_START,
  JS_PropertyStub,
  JS_PropertyStub,
  JS_PropertyStub,
  env_setProperty,
  env_enumerate,
  (JSResolveOp) env_resolve,
  JS_ConvertStub,
  JS_FinalizeStub
};


jsval JSX_make_environment(JSContext *cx, JSObject *obj) {
  JSObject *envobj = JS_NewObject(cx, &jsext_env_class, NULL, NULL);
  if(!envobj) return JSVAL_VOID;

  JSObject *classobj = JS_InitClass(cx, obj, envobj, &jsext_env_class, 0, 0, 0, 0, 0, 0);
  if(!classobj) return JSVAL_VOID;

  return OBJECT_TO_JSVAL(classobj);
}
