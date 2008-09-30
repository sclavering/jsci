#include <jsapi.h>
#include <jscntxt.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>

static JSBool JSX_ReportException(JSContext *cx, char *format, ...) {
  int len;
  char *msg;
  JSString *Str;
  va_list va;
  jsval str;

  va_start(va, format);
#ifdef WIN32
  msg=JS_malloc(cx, 81);
  va_start(va, format);
  len=_vsnprintf(msg,80,format,va);
  msg[80]=0;
#else
  msg=JS_malloc(cx, 81);
  va_start(va, format);
  len=vsnprintf(msg,80,format,va);
  msg[80]=0;

    /*
  len=vsnprintf(NULL,0,format,va);
  va_end(va);
  msg=JS_malloc(cx, len+1);
  va_start(va, format);
  vsprintf(msg,format,va);
    */
#endif
  va_end(va);
  Str=JS_NewString(cx, msg, len);
  str=STRING_TO_JSVAL(Str);
  JS_SetPendingException(cx, str);

  return JS_FALSE;
}

struct startstruct {
  JSContext *outer_cx;
  JSContext *cx;
  JSObject *obj;
  uintN argc;
  jsval *argv;
  JSFunction *fun;
};

static void *doit(void *_s) {
  struct startstruct *s=_s;
  jsval rval=JSVAL_VOID;
  int i;

  JS_SetContextThread(s->cx);
  JS_BeginRequest(s->cx);

  JS_AddRoot(s->cx, &rval);

  JS_CallFunction(s->cx, s->obj, s->fun, s->argc-1, s->argv+1, &rval);

  JS_RemoveRoot(s->cx, &rval);
  JS_RemoveRoot(s->cx, &s->obj);
  for (i=0; i<s->argc; i++)
    JS_RemoveRoot(s->cx, s->argv+i);

  JS_EndRequest(s->cx);
  JS_ClearContextThread(s->cx);

  JS_free(s->cx, s->argv);
  JS_DestroyContext(s->cx);
  JS_free(s->outer_cx, s);
  return 0;
}

/*
  arg 0: function
  arg 1: pointer to pthread_t
 */

static JSBool JSX_pthread_create(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
  JSRuntime *rt;
  struct startstruct *s;
  pthread_t t;
  JSContext *newcx;
  pthread_t **ptr;
  int i;

  JSErrorReporter er=JS_SetErrorReporter(cx, 0);
  JS_SetErrorReporter(cx, er);

  rt=JS_GetRuntime(cx);
  newcx=JS_NewContext(rt, 65536);
  JS_SetGlobalObject(newcx, JS_GetGlobalObject(cx));
  JS_SetErrorReporter(newcx, er);
  JS_SetOptions(newcx, JSOPTION_VAROBJFIX);
  s=JS_malloc(newcx, sizeof(struct startstruct));
  s->outer_cx=cx;
  s->cx=newcx;
  s->obj=JSVAL_TO_OBJECT(argv[0]);
  s->argc=argc-2;
  s->argv=JS_malloc(newcx, sizeof(jsval)*(argc-2));
  s->fun=JS_ValueToFunction(cx, argv[2]);

  for (i=2; i<argc; i++) {
    s->argv[i-2]=argv[i];
    JS_AddRoot(newcx, s->argv+i-2);
  }

  JS_AddRoot(newcx, &s->obj);

  if (pthread_create(&t, 0, doit, s)) {
    JSX_ReportException(cx, "pthread_create");
    goto failure;
  }

  ptr=JS_GetPrivate(cx, JSVAL_TO_OBJECT(argv[1]));
  *ptr[0]=t;

  return JS_TRUE;

 failure:
  for (i=2; i<argc; i++) {
    JS_RemoveRoot(newcx, s->argv+i-2);
  }
  JS_RemoveRoot(newcx, &s->obj);
  JS_DestroyContext(s->cx);
  JS_free(cx, s);

  return JS_FALSE;
  
}

JSBool
JSX_init(JSContext *cx,  JSObject *obj, int argc, jsval *argv, jsval *rval) {
  JSFunction *jsfun=JS_NewFunction(cx, JSX_pthread_create, 0, 0, 0, 0);
  if (!jsfun)
    return JS_FALSE;
  *rval=OBJECT_TO_JSVAL(JS_GetFunctionObject(jsfun));
  return JS_TRUE;
}

