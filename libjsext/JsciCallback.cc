#include "jsci.h"


static void exec_JsciCallback(ffi_cif *cif, void *ret, void **args, void *user_data);


/* static */ JsciCallback *JsciCallback::GetForFunction(JSContext *cx, jsval fun, JsciTypeFunction *t) {
  jsval tmp = JSVAL_VOID;
  // We can't use JS_GetPrivate directly on the function, because that just returns the JSFunction, and JS_SetReservedSlot wants a jsval, so we store private data on a JSObject in a reserved slot
  if(JS_GetReservedSlot(cx, JSVAL_TO_OBJECT(fun), 0, &tmp) && JSVAL_IS_OBJECT(tmp)) {
    return (JsciCallback*) JS_GetPrivate(cx, JSVAL_TO_OBJECT(tmp));
  }
  JSObject *obj = JS_NewObject(cx, 0, 0, 0); // xxx should hook up a finalizer to delete the JsciCallback
  JsciCallback *cb = JsciCallback::Create(cx, fun, t);
  if(!cb || !JS_SetReservedSlot(cx, JSVAL_TO_OBJECT(fun), 0, OBJECT_TO_JSVAL(obj)) || !JS_SetPrivate(cx, obj, cb)) {
    JSX_ReportException(cx, "Error creating callback object for js function");
    delete cb;
    return 0;
  }
  return cb;
}


/* static */ JsciCallback *JsciCallback::Create(JSContext *cx, jsval fun, JsciTypeFunction *t) {
  JsciCallback *cb = new JsciCallback(cx, fun, t);
  if(ffi_prep_closure_loc((ffi_closure*) cb->writeable, cb->type->GetCIF(), exec_JsciCallback, cb, cb->code) == FFI_OK) return cb;
  delete cb;
  return 0;
}


JsciCallback::JsciCallback(JSContext *cx, jsval fun, JsciTypeFunction *t) : cx(cx), type(t), fun(fun), code(0), writeable(0) {
  this->type = t;
  this->writeable = ffi_closure_alloc(sizeof(ffi_closure), &this->code);
}


JsciCallback::~JsciCallback() {
  ffi_closure_free(this->writeable);
}


static void exec_JsciCallback(ffi_cif *cif, void *ret, void **args, void *user_data) {
  ((JsciCallback *) user_data)->exec(cif, ret, args);
}


void *JsciCallback::codeptr() {
  return this->code;
}


void JsciCallback::exec(ffi_cif *cif, void *ret, void **args) {
  JsciTypeFunction *type = (JsciTypeFunction *) this->type;

  jsval rval = JSVAL_VOID;
  jsval *tmp_argv = new jsval[type->nParam];
  if(!tmp_argv) return;

  for(int i = 0; i != type->nParam; ++i) {
    tmp_argv[i]=JSVAL_VOID;
    JS_AddRoot(this->cx, tmp_argv+i);
  }
  JS_AddRoot(this->cx, &rval);

  for(int i = 0; i != type->nParam; ++i) {
    JsciType *t = type->param[i];
    if(dynamic_cast<JsciTypeArray*>(t)) return; // xxx why don't we just treat it as a pointer type?
    t->CtoJS(this->cx, (char*) args[i], tmp_argv + i);
  }

  if(!JS_CallFunctionValue(this->cx, JS_GetGlobalObject(this->cx), this->fun, type->nParam, tmp_argv, &rval)) {
    //    printf("FAILCALL\n");
  }

  for(int i = 0; i != type->nParam; ++i) {
    JsciType *t = type->param[i];
    if(dynamic_cast<JsciTypeArray*>(t)) return;
    if(!t->JStoC(this->cx, (char*) args[i], tmp_argv[i])) return;
  }

  JS_RemoveRoot(this->cx, &rval);
  for(int i = 0; i != type->nParam; ++i) {
    JS_RemoveRoot(this->cx, tmp_argv+i);
  }
  delete tmp_argv;

  if(!dynamic_cast<JsciTypeVoid*>(type->returnType)) type->returnType->JStoC(this->cx, (char*) ret, rval);
}
