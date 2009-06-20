#include "jsci.h"


static void exec_JsciCallback(ffi_cif *cif, void *ret, void **args, void *user_data);


JsciCallback::JsciCallback(JSContext *cx, JSFunction *fun, JsciType *t) : JsciPointer(), cx(cx), fun(fun) {
  void *code;
  this->type = t;
  this->writeable = ffi_closure_alloc(sizeof(ffi_closure), &code);
  this->ptr = code;
}


JSBool JsciCallback::Init() {
  if(ffi_prep_closure_loc((ffi_closure*) this->writeable, ((JsciTypeFunction *) this->type)->GetCIF(), exec_JsciCallback, this, this->ptr) != FFI_OK) return JS_FALSE;
  return JS_TRUE;
}


JsciCallback::~JsciCallback() {
  ffi_closure_free(this->writeable);
}


static void exec_JsciCallback(ffi_cif *cif, void *ret, void **args, void *user_data) {
  JsciCallback *cb = (JsciCallback *) user_data;
  JsciTypeFunction *type = (JsciTypeFunction *) cb->type;

  jsval rval = JSVAL_VOID;
  jsval *tmp_argv = new jsval[type->nParam];
  if(!tmp_argv) return;

  for(int i = 0; i != type->nParam; ++i) {
    tmp_argv[i]=JSVAL_VOID;
    JS_AddRoot(cb->cx, tmp_argv+i);
  }
  JS_AddRoot(cb->cx, &rval);

  for(int i = 0; i < type->nParam; i++) {
    JsciType *t = type->param[i];
    if(t->type == ARRAYTYPE) return; // xxx why don't we just treat it as a pointer type?
    t->CtoJS(cb->cx, (char*) *args, tmp_argv);
  }

  if (!JS_CallFunction(cb->cx, JS_GetGlobalObject(cb->cx), cb->fun, type->nParam, tmp_argv, &rval)) {
    //    printf("FAILCALL\n");
  }

  for(int i = 0; i != type->nParam; ++i) {
    JsciType *t = type->param[i];
    if(t->type == ARRAYTYPE) return;
    if(!t->JStoC(cb->cx, (char*) *args, tmp_argv[i])) return;
    args++;
  }

  JS_RemoveRoot(cb->cx, &rval);
  for(int i = 0; i != type->nParam; ++i) {
    JS_RemoveRoot(cb->cx, tmp_argv+i);
  }
  delete tmp_argv;

  if(type->returnType->type != VOIDTYPE) type->returnType->JStoC(cb->cx, (char*) ret, rval);
}

