/*
* Copyright (c) 2006, Berges Allmenndigitale Rådgivningstjeneste
* All rights reserved.
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * Neither the name of Berges Allmenndigitale Rådgivningstjeneste nor the
*       names of its contributors may be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE BERGES AND CONTRIBUTORS ``AS IS'' AND ANY
* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE BERGES AND CONTRIBUTORS BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include "util.h"
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
extern char **environ;



// These are mostly defined in other files
jsval make_Type(JSContext *cx, JSObject *glo);
jsval make_Pointer(JSContext *cx, JSObject *glo);
jsval make_Dl(JSContext *cx, JSObject *glo);
jsval make_encodeUTF8(JSContext *cx);
jsval make_decodeUTF8(JSContext *cx);
jsval make_encodeJSON(JSContext *cx);
jsval make_decodeJSON(JSContext *cx);
jsval make_stringifyHTML(JSContext *cx);

static JSBool make_environment_vars_obj(JSContext *cx, jsval *rv);

static jsval make_load(JSContext *cx);
static jsval make_gc(JSContext *cx);
static jsval make_isCompilableUnit(JSContext *cx);

static JSBool eval_file(JSContext *cx, JSObject *obj, const char *filename, jsval *rval);
static JSBool make_jsxlib(JSContext *cx, JSObject *gl);


static void my_ErrorReporter(JSContext *cx, const char *message, JSErrorReport *report) {
  const char *filename = report->filename ? report->filename : "<unknown>";
  fprintf(stderr, "Line %d in %s:", report->lineno, filename);
  if(message) fprintf(stderr, "%s\n", message);
}


int main(int argc, char **argv) {
  int exitcode = 1;
  JSRuntime *rt = JS_NewRuntime(64000000);
  if(rt) {
    JSContext *cx = JS_NewContext(rt, 65536);
    if(cx) {
      JS_SetOptions(cx, JS_GetOptions(cx) | JSOPTION_VAROBJFIX);
      JS_SetErrorReporter(cx, my_ErrorReporter);
      JS_SetVersion(cx, JSVERSION_LATEST);

      JSObject *glob = JS_NewObject(cx, NULL, NULL, NULL);
      if(glob && JS_InitStandardClasses(cx, glob) && make_jsxlib(cx, glob)) {
        jsval rv;
        JS_AddRoot(cx, &rv);
        // Evaluate a .js startup file, and use the exit code it returns, if any.
        if(eval_file(cx, glob, getenv("JSEXT_INI"), &rv)) {
          exitcode = JSVAL_IS_VOID(rv) ? 0 : JSVAL_IS_INT(rv) ? JSVAL_TO_INT(rv) : 1;
        }
        JS_RemoveRoot(cx, &rv);
      }
      JS_DestroyContext(cx);
    }
    JS_DestroyRuntime(rt);
  }
  JS_ShutDown();
  return exitcode;
}


// create the .jsxlib property on the global object
static JSBool make_jsxlib(JSContext *cx, JSObject *obj) {
  jsval tmp = JSVAL_VOID;
  JS_AddRoot(cx, &tmp);
  JSObject *argobj = JS_NewObject(cx, 0, 0, 0); // needs renaming

  if(!JS_DefineProperty(cx, obj, "jsxlib", OBJECT_TO_JSVAL(argobj), 0, 0, JSPROP_READONLY | JSPROP_PERMANENT)) return JS_FALSE;

  tmp = make_Type(cx, obj);
  JS_SetProperty(cx, argobj, "Type", &tmp);
  tmp = make_Pointer(cx, obj);
  JS_SetProperty(cx, argobj, "Pointer", &tmp);
  tmp = make_Dl(cx, obj);
  JS_SetProperty(cx, argobj, "Dl", &tmp);
  tmp = make_load(cx);
  JS_SetProperty(cx, argobj, "load", &tmp);
  tmp = make_gc(cx);
  JS_SetProperty(cx, argobj, "gc", &tmp);
  tmp = make_isCompilableUnit(cx);
  JS_SetProperty(cx, argobj, "isCompilableUnit", &tmp);
  if(!make_environment_vars_obj(cx, &tmp)) return JS_FALSE;
  JS_SetProperty(cx, argobj, "environment", &tmp);
  tmp = make_encodeUTF8(cx);
  JS_SetProperty(cx, argobj, "encodeUTF8", &tmp);
  tmp = make_decodeUTF8(cx);
  JS_SetProperty(cx, argobj, "decodeUTF8", &tmp);
  tmp = make_encodeJSON(cx);
  JS_SetProperty(cx, argobj, "encodeJSON", &tmp);
  tmp = make_decodeJSON(cx);
  JS_SetProperty(cx, argobj, "decodeJSON", &tmp);
  tmp = make_stringifyHTML(cx);
  JS_SetProperty(cx, argobj, "stringifyHTML", &tmp);
  char cwd[1024];
  tmp = STRING_TO_JSVAL(JS_NewStringCopyZ(cx, getcwd(cwd, 1024)));
  JS_SetProperty(cx, argobj, "cwd", &tmp);

  JS_RemoveRoot(cx, &tmp);
  return JS_TRUE;
}


static JSBool eval_file(JSContext *cx, JSObject *obj, const char *filename, jsval *rval) {
  int fd = open(filename, O_RDONLY);
  if(fd == -1) return JSX_ReportException(cx, "Could not open %s", filename);

  JSBool ok = JS_FALSE;
  struct stat S;
  fstat(fd, &S);
  char *buf = (char*) malloc(S.st_size);
  if (!buf) {
    JSX_ReportException(cx, "Out of memory when loading %s", filename);
    goto exit;
  }
  if(read(fd, buf, S.st_size) != S.st_size) {
    JSX_ReportException(cx, "Error loading %s", filename);
    goto exit;
  }
  *rval=JSVAL_ZERO;
  if(!JS_EvaluateScript(cx, obj, buf, S.st_size, filename, 1, rval)) goto exit;

  ok = JS_TRUE;
 exit:
  if(buf) free(buf);
  if(fd) close(fd);
  return ok;
}


static JSBool jsx_load(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
  if(!JSVAL_IS_STRING(argv[0])) return JSX_ReportException(cx, "load(): first argument must be a string");
  return eval_file(cx, obj, JS_GetStringBytes(JSVAL_TO_STRING(argv[0])), rval);
}


static jsval make_load(JSContext *cx) {
  JSFunction *jsfun = JS_NewFunction(cx, jsx_load, 1, 0, 0, 0);
  if(!jsfun) return JSVAL_VOID;
  return OBJECT_TO_JSVAL(JS_GetFunctionObject(jsfun));
}


static JSBool jsx_gc(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
  JS_GC(cx);
  return JS_TRUE;
}


static jsval make_gc(JSContext *cx) {
  JSFunction *jsfun = JS_NewFunction(cx, jsx_gc, 0, 0, 0, 0);
  if(!jsfun) return JSVAL_VOID;
  return OBJECT_TO_JSVAL(JS_GetFunctionObject(jsfun));
}


static JSBool jsx_isCompilableUnit(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
  if(argc != 1) {
    JSX_ReportException(cx, "Wrong number of arguments");
    return JS_FALSE;
  }
  char *expr;
  if(!JS_ConvertArguments(cx, argc, argv, "s", &expr)) return JS_FALSE;
  *rval = JS_BufferIsCompilableUnit(cx, obj, expr, strlen(expr)) ? JSVAL_TRUE : JSVAL_FALSE;
  return JS_TRUE;
}


static jsval make_isCompilableUnit(JSContext *cx) {
  JSFunction *jsfun = JS_NewFunction(cx, jsx_isCompilableUnit, 0, 0, 0, 0);
  if(!jsfun) return JSVAL_VOID;
  return OBJECT_TO_JSVAL(JS_GetFunctionObject(jsfun));
}


static JSBool make_environment_vars_obj(JSContext *cx, jsval *rv) {
  JSObject *obj = JS_NewObject(cx, 0, 0, 0);
  if(!obj) return JS_FALSE;
  *rv = OBJECT_TO_JSVAL(obj);

  char **evp = environ;
  char *name;
  for( ; (name = *evp) != NULL; evp++) {
    char *eqchar = name;
    while(*eqchar && *eqchar != '=') eqchar++;
    if(!*eqchar) continue;
    JSString *valstr = JS_NewStringCopyZ(cx, eqchar + 1);
    if(!valstr) return JS_FALSE;
    *eqchar = 0;
    jsval val = STRING_TO_JSVAL(valstr);
    JSBool ok = JS_SetProperty(cx, obj, name, &val);
    *eqchar = '=';
    if(!ok) return JS_FALSE;
  }

  return JS_TRUE;
}
