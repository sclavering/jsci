#include <cdb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <jsapi.h>
#include <stdarg.h>

# define declspec(x)

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

static void activeCdb_finalize(JSContext *cx, JSObject *obj) {
  struct cdb *cdbfile=JS_GetPrivate(cx, obj);
  if (cdbfile) {
//    close(cdbfile->cdb_fd);
    cdb_free(cdbfile);
	free(cdbfile);
  }
}

static JSBool
activeCdb_resolve(JSContext *cx, JSObject *obj, jsval id, uintN flags,
            JSObject **objp)
{
  JSString *idstr;
  char *name;
  const char *nametmp;
  jsval rval=JSVAL_VOID;
  struct cdb *cdbfile;

  idstr = JS_ValueToString(cx, id);
  if (!idstr)
    return JS_FALSE;

  nametmp = JS_GetStringBytes(idstr);
  name = JS_malloc(cx, JS_GetStringLength(idstr)+1);
  memcpy(name, nametmp, JS_GetStringLength(idstr));
  name[JS_GetStringLength(idstr)]=0;

  cdbfile=JS_GetPrivate(cx, obj);
  if (!cdbfile) {
    *objp=NULL;
    JS_free(cx, name);
    return JS_TRUE;
  }

  if (cdb_find(cdbfile, name, JS_GetStringLength(idstr))) {
    const void *d;
    JSScript *script;
    JSObject *scrobj;
    JSBool ok;

    d=cdb_get(cdbfile, cdb_datalen(cdbfile), cdb_datapos(cdbfile));

    script=JS_CompileScript(cx, obj, (char *)d, cdb_datalen(cdbfile), name, 1);
    if (!script) {
      JS_free(cx, name);
      return JS_FALSE;
    }
    scrobj=JS_NewScriptObject(cx, script);
    JS_AddRoot(cx, &scrobj);
    JS_AddRoot(cx, &rval);

    ok=JS_ExecuteScript(cx, obj, script, &rval);

    JS_RemoveRoot(cx, &scrobj);
      
    if (!ok) {
      JS_RemoveRoot(cx, &rval);
      *objp=NULL;
      JS_free(cx, name);
      return JS_FALSE;
    }
      
    JS_DefineProperty(cx, obj, name, rval, NULL, NULL, JSPROP_ENUMERATE);
    JS_RemoveRoot(cx, &rval);
    
    JS_free(cx, name);
    return JS_TRUE;
  }

  *objp=NULL;
  JS_free(cx, name);
  return JS_TRUE;
}


JSBool
JS_DLL_CALLBACK activeCdb_enumerate(JSContext *cx, JSObject *obj,
				     JSIterateOp enum_op,
				     jsval *statep, jsid *idp);

static JSClass activeCdb_class = {
    "ActiveCdb",
    JSCLASS_HAS_PRIVATE | JSCLASS_NEW_RESOLVE | JSCLASS_NEW_RESOLVE_GETS_START | JSCLASS_NEW_ENUMERATE,
    JS_PropertyStub,
    JS_PropertyStub,
    JS_PropertyStub,
    JS_PropertyStub,
    (JSEnumerateOp) activeCdb_enumerate,
    (JSResolveOp)  activeCdb_resolve,
    JS_ConvertStub,
    activeCdb_finalize
};

JSBool
JS_DLL_CALLBACK activeCdb_enumerate(JSContext *cx, JSObject *obj,
				     JSIterateOp enum_op,
				     jsval *statep, jsid *idp) {
  unsigned *c;
  struct cdb *cdbfile;
  char *buf;
  int buflen;
  JSString *str;

  //  printf("op=%d\n",enum_op);

  switch(enum_op) {
  case JSENUMERATE_INIT:
    c=(unsigned *)JS_malloc(cx, sizeof(unsigned));
    *c=0;
    if (idp)
      *idp=JSVAL_ZERO;
    *statep=PRIVATE_TO_JSVAL(c);
    return JS_TRUE;
  case JSENUMERATE_NEXT:
    cdbfile=JS_GetPrivate(cx, obj);
    c=JSVAL_TO_PRIVATE(*statep);
    if (!cdbfile) {
      JS_free(cx, c);
      *statep=JSVAL_NULL;
      return JS_TRUE;
    }
    if (*c==0) {
      cdb_seqinit(c, cdbfile);
    } else {
      if (!cdb_seqnext(c, cdbfile)) {
	JS_free(cx, c);
	*statep=JSVAL_NULL;
	return JS_TRUE;
      }
    }
    buflen=cdb_keylen(cdbfile);
    buf=JS_malloc(cx,buflen);
    if (!buf)
      return JS_FALSE;
    cdb_readkey(cdbfile, buf);
    str=JS_NewString(cx, buf, buflen);
    if (!str)
      return JS_FALSE;
    return JS_ValueToId(cx, STRING_TO_JSVAL(str), idp);
  case JSENUMERATE_DESTROY:
    JS_free(cx, JSVAL_TO_PRIVATE(*statep));
    return JS_TRUE;
  }

  return JS_FALSE;
}



/*
JSBool
js_Execute(JSContext *cx, JSObject *chain, JSScript *script,
           JSStackFrame *down, uintN flags, jsval *result);
*/

static JSBool activeCdb_new(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
  char *filename;
  struct cdb *cdbfile;
  JSObject *object;
  int fd;

  if (!JS_ConvertArguments(cx, argc, argv, "s", &filename)) {
    return JS_FALSE;
  }
  
  if (!JS_IsConstructing(cx)) { // not called with new
    object=JS_NewObject(cx, &activeCdb_class, 0, 0);
    *rval=OBJECT_TO_JSVAL(object);
  } else {
    object=obj;
  }

  fd=open(filename, O_RDONLY);
  if (!fd) {
    JSX_ReportException(cx, "Cannot open ",filename);
    return JS_FALSE;
  }
    
  cdbfile=malloc(sizeof(struct cdb));
  cdb_init(cdbfile, fd);
  JS_SetPrivate(cx, object, cdbfile);

  *rval=OBJECT_TO_JSVAL(object);

  return JS_TRUE;
}

JSBool JSX_init(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
  JSObject *classobj;

  classobj=JS_InitClass(cx, obj, NULL, &activeCdb_class, activeCdb_new, 0, NULL, NULL, NULL, NULL);
  if (!classobj)
    return JS_FALSE;

  *rval=OBJECT_TO_JSVAL(JS_GetConstructor(cx, classobj));

  return JS_TRUE;
}
  
