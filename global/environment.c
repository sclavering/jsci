#include <jsapi.h>

# define _GNU_SOURCE
# include <getopt.h>
# include <pthread.h>
# include <unistd.h>
extern char **environ;
#include <stdlib.h>

#include <stdarg.h>

static JSBool JSX_ReportException(JSContext *cx, char *format, ...) {
  int len;
  char *msg;
  JSString *Str;
  va_list va;
  jsval str;

  va_start(va, format);
  msg=JS_malloc(cx, 81);
  va_start(va, format);
  len=vsnprintf(msg,80,format,va);
  msg[80]=0;

  va_end(va);
  Str=JS_NewString(cx, msg, len);
  str=STRING_TO_JSVAL(Str);
  JS_SetPendingException(cx, str);

  return JS_FALSE;
}

static JSBool
env_setProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
{
/* XXX porting may be easy, but these don't seem to supply setenv by default */
#if !defined XP_BEOS && !defined XP_OS2 && !defined SOLARIS
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
#endif /* !defined XP_BEOS && !defined XP_OS2 && !defined SOLARIS */
    return JS_TRUE;
}

static JSBool
env_enumerate(JSContext *cx, JSObject *obj)
{
    static JSBool reflected;
    char **evp, *name, *value, *es, **origevp;
	int nenv=0;
	int envcap=0;
    JSString *valstr;
    JSBool ok;

    if (reflected)
        return JS_TRUE;

	evp = environ; // (char **) JS_GetPrivate(cx, obj);

    for (; (name = *evp) != NULL; evp++) {
		value=name;
		while (*value && *value!='=') value++;
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
        if (!ok) goto failure;
    }

    reflected = JS_TRUE;

	return JS_TRUE;

failure:
	return JS_FALSE;
}

static JSBool
env_resolve(JSContext *cx, JSObject *obj, jsval id, uintN flags,
            JSObject **objp)
{
    int vallen;
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
        if (!valstr)
			goto failure;
        if (!JS_DefineProperty(cx, obj, name, STRING_TO_JSVAL(valstr),
                               NULL, NULL, JSPROP_ENUMERATE)) {
            goto failure;
        }
        *objp = obj;
    }

    return JS_TRUE;

failure:
	return JS_FALSE;
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

JSBool
JSX_init(JSContext *cx,  JSObject *obj, int argc, jsval *argv, jsval *rval) {
  JSObject *envobj = JS_NewObject(cx, &jsext_env_class, NULL, NULL);
  JSObject *classobj;
  if (!envobj)
    return JS_FALSE;
  classobj=JS_InitClass(cx, obj, envobj, &jsext_env_class, 0, 0, 0, 0, 0, 0);
  if (!classobj)
    goto end_false;

  *rval=OBJECT_TO_JSVAL(classobj);

  return JS_TRUE;

 end_false:
  return JS_FALSE;
}
