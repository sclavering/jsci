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


#include <jsapi.h>
#include <getopt.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <dlfcn.h>
#include "util.h"


/* these are defined respectively in Type.c Pointer.c Dl.c and load.c */
jsval JSX_make_Type(JSContext *cx, JSObject *glo);
jsval JSX_make_Pointer(JSContext *cx, JSObject *glo);
jsval make_Dl(JSContext *cx, JSObject *glo);
jsval JSX_make_load(JSContext *cx);
jsval JSX_make_environment(JSContext *cx, JSObject *obj);
jsval make_encodeUTF8(JSContext *cx);
jsval make_decodeUTF8(JSContext *cx);
jsval make_encodeJSON(JSContext *cx);
jsval make_decodeJSON(JSContext *cx);
jsval make_encodeBase64(JSContext *cx);
jsval make_decodeBase64(JSContext *cx);
jsval make_cToXML(JSContext *cx);
static jsval make_gc(JSContext *cx);
static jsval make_isCompilableUnit(JSContext *cx);


static char *strip_file_name(char *ini_file);

static JSBool exec(JSContext *cx, JSObject *obj, char *filename, jsval *rval);

JSBool JSX_init(JSContext *cx, JSObject *obj, jsval *rval);

static void printhelp(void) {
  puts("jsext [OPTION]... [FILE] [ARGUMENT]...\n\n"
       "Evaluates FILE. If it is an anonymous JavaScript function, arguments are passed to it\n\n"
       "Options:\n"
       "-h, --help\n"
       "\tPrint help\n");
}

static void
my_ErrorReporter(JSContext *cx, const char *message, JSErrorReport *report)
{
  if (report->lineno && report->filename)
    fprintf(stderr,"Line %d in %s:",report->lineno,report->filename);
  else
    fprintf(stderr,"Unknown file:");
  if (message)
    fprintf(stderr,"%s\n",message);
}


int main(int argc, char **argv, char **envp) {
  JSRuntime *rt;
  JSContext *cx=0;
  JSObject *glob;
  int exitcode=0;
  jsval rval;

  for (;;) {
    static struct option long_options[] = {
      {"help", 0, 0, 'h'},
      {0, 0, 0, 0}
    };

    int c = getopt_long(argc, argv, "r:he:", long_options, 0);

    switch(c) {
    case 'h':
      printhelp();
      exit(0);
      break;
    }

    if (c == -1)
      break;
  }

  rt=JS_NewRuntime(64000000);
  if (!rt) goto failure;

  cx=JS_NewContext(rt, 65536);
  if (!cx) goto failure;

  //  JS_SetOptions(cx, JSOPTION_VAROBJFIX);
  JS_SetErrorReporter(cx, my_ErrorReporter);

  glob=JS_NewObject(cx, NULL, NULL, NULL);
  if (!glob)
    goto failure;

  JS_SetGlobalObject(cx, glob);

  if (!JS_InitStandardClasses(cx, glob))
    goto failure;

  JS_AddRoot(cx, &rval);
  exitcode = !JSX_init(cx, glob, &rval);
  JS_RemoveRoot(cx, &rval);

  JS_DestroyContext(cx);
  JS_DestroyRuntime(rt);

  return exitcode;

 failure:
  if (cx) JS_DestroyContext(cx);
  if (rt) JS_DestroyRuntime(rt);
  return 1;
}


JSBool JSX_init(JSContext *cx, JSObject *obj, jsval *rval) {
  JS_SetOptions(cx, JS_GetOptions(cx) | JSOPTION_VAROBJFIX);

  JSObject *argobj;
  argobj = JS_NewObject(cx, 0, 0, 0);
  JS_AddRoot(cx, &argobj);

  jsval tmp = JSVAL_VOID;
  JS_AddRoot(cx, &tmp);

  tmp = JSX_make_Type(cx, obj);
  JS_SetProperty(cx, argobj, "Type", &tmp);
  tmp = JSX_make_Pointer(cx, obj);
  JS_SetProperty(cx, argobj, "Pointer", &tmp);
  tmp = make_Dl(cx, obj);
  JS_SetProperty(cx, argobj, "Dl", &tmp);
  tmp = JSX_make_load(cx);
  JS_SetProperty(cx, argobj, "load", &tmp);
  tmp = make_cToXML(cx);
  JS_SetProperty(cx, argobj, "cToXML", &tmp);
  tmp = make_gc(cx);
  JS_SetProperty(cx, argobj, "gc", &tmp);
  tmp = make_isCompilableUnit(cx);
  JS_SetProperty(cx, argobj, "isCompilableUnit", &tmp);
  tmp = JSX_make_environment(cx, obj);
  JS_SetProperty(cx, argobj, "environment", &tmp);
  tmp = make_encodeUTF8(cx);
  JS_SetProperty(cx, argobj, "encodeUTF8", &tmp);
  tmp = make_decodeUTF8(cx);
  JS_SetProperty(cx, argobj, "decodeUTF8", &tmp);
  tmp = make_encodeJSON(cx);
  JS_SetProperty(cx, argobj, "encodeJSON", &tmp);
  tmp = make_decodeJSON(cx);
  JS_SetProperty(cx, argobj, "decodeJSON", &tmp);
  tmp = make_encodeBase64(cx);
  JS_SetProperty(cx, argobj, "encodeBase64", &tmp);
  tmp = make_decodeBase64(cx);
  JS_SetProperty(cx, argobj, "decodeBase64", &tmp);
  char cwd[1024];
  tmp = STRING_TO_JSVAL(JS_NewStringCopyZ(cx, getcwd(cwd, 1024)));
  JS_SetProperty(cx, argobj, "cwd", &tmp);

  char *ifdup=0;
  char *lastslash;
  char *filename;
  char *ini_file = getenv("JSEXT_INI");
  if(ini_file == NULL) ini_file = libdir "/jsext/0-init.js";

  ifdup=strdup(ini_file);
  lastslash=strip_file_name(ifdup);
  if (lastslash) {
    *lastslash=0;
    chdir(ifdup);
    filename=lastslash+1;
  } else {
    filename=ifdup;
  }

  exec(cx, obj, filename, rval);

  JSFunction *jsfun;

  JSBool rv = JS_FALSE;
  jsfun=JS_ValueToFunction(cx, *rval);
  if(jsfun) {
    tmp = OBJECT_TO_JSVAL(argobj);
    rv = JS_CallFunction(cx, obj, jsfun, 1, &tmp, rval);
  } else {
    JSX_ReportException(cx, "Ini file does not evaluate to a function");
  }

  JS_RemoveRoot(cx, &argobj);
  JS_RemoveRoot(cx, &tmp);
  if(ifdup) free(ifdup);

  return rv;
}


static JSBool exec(JSContext *cx, JSObject *obj, char *filename, jsval *rval) {
  int fd=open(filename,O_RDONLY);
  char *buf;
  JSScript *script=0;
  struct stat S;
  JSObject *scrobj=0;

  if (fd==-1) {
    JSX_ReportException(cx, "Unable to open %s",filename);
    return JS_FALSE; // File does not exist
  }
  fstat(fd, &S);
  buf=malloc(S.st_size);
  if (!buf) {
    JSX_ReportException(cx, "Out of memory for %s", filename);
    goto failure;
  }
  if (read(fd, buf, S.st_size)!=S.st_size) {
    JSX_ReportException(cx, "Error loading %s",filename);
    goto failure;
  }

  script=JS_CompileScript(cx, obj, buf, S.st_size, filename, 1);
  if (!script)
    goto failure;
  scrobj=JS_NewScriptObject(cx, script);
  JS_AddRoot(cx, &scrobj);

  free(buf);
  buf=0;

  close(fd);
  fd=0;

  if (!JS_ExecuteScript(cx, obj, script, rval)) goto failure;

  JS_RemoveRoot(cx, &scrobj);
  //  JS_DestroyScript(cx, script);
  return JS_TRUE;

 failure:
  if (buf) free(buf);
  if (fd) close(fd);
  if (scrobj) JS_RemoveRoot(cx, &scrobj);
  //  if (script) JS_DestroyScript(cx, script);
  return JS_FALSE;
}


static char *strip_file_name(char *ini_file) {
  char *lastslash=strrchr(ini_file,'/');
  if(lastslash) return lastslash;
  return 0;
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
