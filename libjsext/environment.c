#include <jsapi.h>

# define _GNU_SOURCE
# include <getopt.h>
# include <unistd.h>
extern char **environ;
#include <stdlib.h>

#include <stdarg.h>
#include "util.h"


static JSBool
env_setProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
{
    JSString *idstr, *valstr;
    const char *name, *value;
    int rv;

    idstr = JS_ValueToString(cx, id);
    valstr = JS_ValueToString(cx, *vp);
    if (!idstr || !valstr)
        return JS_FALSE;
    name = JS_GetStringBytes(idstr);
    value = JS_GetStringBytes(valstr);
    rv = setenv(name, value, 1);
    if (rv < 0) {
        JSX_ReportException(cx, "can't set envariable %s to %s", name, value);
        return JS_FALSE;
    }
    *vp = STRING_TO_JSVAL(valstr);

    return JS_TRUE;
}


static JSBool env_enumerate(JSContext *cx, JSObject *obj) {
    static JSBool reflected;
    char **evp, *name, *value;
    JSString *valstr;
    JSBool ok;

    if (reflected)
        return JS_TRUE;

    evp = environ; // (char **) JS_GetPrivate(cx, obj);

    for( ; (name = *evp) != NULL; evp++) {
        value = name;
        while(*value && *value != '=') value++;
        if (!*value)
            continue;
        *value++ = '\0';
        valstr = JS_NewStringCopyZ(cx, value);
        if (!valstr) {
            ok = JS_FALSE;
        } else {
            ok = JS_DefineProperty(cx, obj, name, STRING_TO_JSVAL(valstr),
                                   NULL, NULL, JSPROP_ENUMERATE);
        }
        value[-1] = '=';
        if(!ok) return JS_FALSE;
    }

    reflected = JS_TRUE;

    return JS_TRUE;
}

static JSBool env_resolve(JSContext *cx, JSObject *obj, jsval id, uintN flags, JSObject **objp) {
    JSString *idstr, *valstr;
    char *name, *value = 0;

    if (flags & JSRESOLVE_ASSIGNING)
        return JS_TRUE;

    idstr = JS_ValueToString(cx, id);
    if (!idstr)
        return JS_FALSE;
    name = JS_GetStringBytes(idstr);
    value = getenv(name);
    if (value) {
        valstr = JS_NewStringCopyZ(cx, value);
        if(!valstr || !JS_DefineProperty(cx, obj, name, STRING_TO_JSVAL(valstr), NULL, NULL, JSPROP_ENUMERATE)) {
            return JS_FALSE;
        }
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
