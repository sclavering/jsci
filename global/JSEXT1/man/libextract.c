#include <dlfcn.h>
#include <string.h>

#include <jsapi.h>
#include <jsdbgapi.h>
#include <jscntxt.h>

//static int collect=0;

//static JSObject *outerobj;
//static JSContext *outercx;
//static JSFunction *callback;
static  JSPropertyOp objoldsetter;
static  JSPropertyOp funcoldsetter;

static  JSClass *objclass;
static  JSClass *funcclass;

JSObject *retarray;

static JSBool
setProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
  const char *filename="";
  uintN ln=0;
  jsval tmp;

  //  if (!collect)
  //    goto done;

  if (JS_IsExceptionPending(cx))
    goto done;

  if (!JSVAL_IS_STRING(id))
    goto done;

  JSStackFrame *fp=NULL;
  JS_FrameIterator(cx, &fp);
  if (fp!=0) {
    jsbytecode *pc=JS_GetFramePC(cx, fp);
    if (pc) {
      JSScript *script=JS_GetFrameScript(cx, fp);
      ln=JS_PCToLineNumber(cx, script, pc);
      filename=JS_GetScriptFilename(cx, script);
    }
  }

  jsid parentid;
  JS_GetObjectId(cx, obj, &parentid);
  
  jsuint len;
  JS_GetArrayLength(cx, retarray, &len);

  JSString *s2=JS_NewStringCopyZ(cx, filename);
  tmp=STRING_TO_JSVAL(s2);
  JS_SetElement(cx, retarray, len++, &tmp);
  tmp=INT_TO_JSVAL(ln);
  JS_SetElement(cx, retarray, len++, &tmp);
  tmp=id;
  JS_SetElement(cx, retarray, len++, &tmp);
  tmp=INT_TO_JSVAL(parentid);
  JS_SetElement(cx, retarray, len++, &tmp);

done:
  return JS_TRUE;

failure:
  return JS_FALSE;

}

// Callback function
//
// function(filename, linenumber, symbol, parentid);

static JSBool
objnewsetter(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
  JSBool ok=setProperty(cx, obj, id, vp);
  
  if (ok && objoldsetter)
    return objoldsetter(cx, obj, id, vp);
  
  return ok;
}

static JSBool
funcnewsetter(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
  JSBool ok=setProperty(cx, obj, id, vp);
  
  if (ok && funcoldsetter)
    return funcoldsetter(cx, obj, id, vp);
  
  return ok;
}


static JSBool JS_DLL_CALLBACK JSXC_extract(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
  // arg0 is callback function to call whenever an assignment is made

  //  outercx=cx;
  //  callback=JS_ValueToFunction(cx, argv[0]);
  //  outerobj=obj;

  // 1d. get pointer to function clasp and set setter property
  JSFunction *newfunction=JS_NewFunction(cx, NULL, 0, 0, 0, 0);
  funcclass=JS_GET_CLASS(cx, JS_GetFunctionObject(newfunction));
  funcoldsetter=funcclass->setProperty;
  funcclass->setProperty=funcnewsetter;

  // 1b. get pointer to object clasp and set setter property
  JSObject *newobject=JS_NewObject(cx, NULL, NULL, NULL);
  objclass=JS_GET_CLASS(cx, newobject);
  objoldsetter=objclass->setProperty;
  objclass->setProperty=objnewsetter;

  retarray=JS_NewArrayObject(cx, 0, 0);
  JS_AddRoot(cx, &retarray);

  //  collect=1;
  return JS_TRUE;
}

static JSBool JS_DLL_CALLBACK JSXC_unextract(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

  funcclass->setProperty=funcoldsetter;
  objclass->setProperty=objoldsetter;

  *rval=OBJECT_TO_JSVAL(retarray);
  JS_RemoveRoot(cx, &retarray);

  return JS_TRUE;

}

static JSBool JS_DLL_CALLBACK JSXC_object_id(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
  jsid objid;

  if (JSVAL_IS_OBJECT(argv[0]) && argv[0]!=JSVAL_NULL) {
    JS_GetObjectId(cx, JSVAL_TO_OBJECT(argv[0]), &objid);
    *rval=INT_TO_JSVAL(objid);
  } else {
    *rval=JSVAL_VOID;
  }

  return JS_TRUE;
}

JSBool JSX_init(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
  obj=JS_NewObject(cx, 0, 0, 0);
  JS_DefineFunction(cx, obj, "id", JSXC_object_id, 1, 0);
  JS_DefineFunction(cx, obj, "start", JSXC_extract, 0, 0);
  JS_DefineFunction(cx, obj, "stop", JSXC_unextract, 0, 0);
  *rval=OBJECT_TO_JSVAL(obj);
  return JS_TRUE;
}
