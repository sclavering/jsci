#include <jsapi.h>

#ifdef _WIN32
# include <windows.h>
# include <io.h>
# include <direct.h>
#else
# define _GNU_SOURCE
# include <getopt.h>
# include <pthread.h>
# include <unistd.h>
extern char **environ;
#endif
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#else
typedef char TCHAR;
#endif

#include <stdarg.h>

#ifdef UNICODE
#define JS_GetStringTChars JS_GetStringChars
#define JS_NewStringTCopyZ JS_NewUCStringCopyZ
#define JS_DefineProperty(cx, obj, name, value, getter, setter, attrs) \
  JS_DefineUCProperty((cx),(obj),(name),wcslen(name),(value),(getter),(setter),(attrs))
#else
#define JS_NewStringTCopyZ JS_NewStringCopyZ
#define JS_GetStringTChars JS_GetStringBytes
#endif

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
    const TCHAR *name, *value;
    int rv;

    idstr = JS_ValueToString(cx, id);
    valstr = JS_ValueToString(cx, *vp);
    if (!idstr || !valstr)
        return JS_FALSE;
    name = JS_GetStringTChars(idstr);
    value = JS_GetStringTChars(valstr);
#ifdef XP_WIN
	if (SetEnvironmentVariable(name,value)) rv=0;
	else rv=-1;
#elif defined HPUX || defined OSF1 || defined IRIX
    {
        char *waste = JS_smprintf("%s=%s", name, value);
        if (!waste) {
            JS_ReportOutOfMemory(cx);
            return JS_FALSE;
        }

        rv = putenv(waste);
        /*
         * HPUX9 at least still has the bad old non-copying putenv.
         *
         * Per mail from <s.shanmuganathan@digital.com>, OSF1 also has a putenv
         * that will crash if you pass it an auto char array (so it must place
         * its argument directly in the char *environ[] array).
         */
//        free(waste);
    }
#else
    rv = setenv(name, value, 1);
#endif
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
    TCHAR **evp, *name, *value, *es, **origevp;
	int nenv=0;
	int envcap=0;
    JSString *valstr;
    JSBool ok;

    if (reflected)
        return JS_TRUE;

#ifdef _WIN32
	es= name = (TCHAR *)GetEnvironmentStrings();
	envcap=128;
	origevp=(TCHAR **)malloc(sizeof(TCHAR *)*envcap);
	for(;;) {
		if (nenv==envcap) {
			envcap*=2;
			origevp=(TCHAR **)realloc(origevp, sizeof(TCHAR *)*envcap);
		}
		if (*name) {
			origevp[nenv++]=name;
			while (*name) name++;
			name++;
		} else {
			origevp[nenv]=0;
			break;
		}
	};
	evp=origevp;
#else
	evp = environ;//(TCHAR **)JS_GetPrivate(cx, obj);
#endif
    for (; (name = *evp) != NULL; evp++) {
		value=name;
		while (*value && *value!='=') value++;
        if (!*value)
            continue;
        *value++ = '\0';
        valstr = JS_NewStringTCopyZ(cx, value);
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

#ifdef _WIN32
	free(origevp);
	FreeEnvironmentStrings(es);
#endif

	return JS_TRUE;
failure:
#ifdef _WIN32
	free(origevp);
	FreeEnvironmentStrings(es);
#endif
	return JS_FALSE;
}

static JSBool
env_resolve(JSContext *cx, JSObject *obj, jsval id, uintN flags,
            JSObject **objp)
{
    int vallen;
	JSString *idstr, *valstr;
    TCHAR *name, *value=0;

    if (flags & JSRESOLVE_ASSIGNING)
        return JS_TRUE;

    idstr = JS_ValueToString(cx, id);
    if (!idstr)
        return JS_FALSE;
    name = JS_GetStringTChars(idstr);
#ifdef _WIN32
	vallen=GetEnvironmentVariable(name,0,0);
	if (vallen) {
		value=malloc(vallen*sizeof(TCHAR));
		GetEnvironmentVariable(name,value,vallen);
	}
#else
    value = getenv(name);
#endif
    if (value) {
        valstr = JS_NewStringTCopyZ(cx, value);
        if (!valstr)
			goto failure;
        if (!JS_DefineProperty(cx, obj, name, STRING_TO_JSVAL(valstr),
                               NULL, NULL, JSPROP_ENUMERATE)) {
            goto failure;
        }
        *objp = obj;
    }

#ifdef _WIN32
	if (value) free(value);
#endif
    return JS_TRUE;

failure:
#ifdef _WIN32
	if (value) free(value);
#endif
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

#ifdef _WIN32
__declspec(dllexport)
#endif
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
