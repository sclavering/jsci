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
# include <getopt.h>
#include "unicodedef.h"
#include "libjsext.h"
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
# include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>

static int call(JSContext *cx, JSObject *obj, jsval fun, int argc, char *argv[]);
static int eval(JSContext *cx, JSObject *obj, char *expr);

static void printhelp() {
  puts("jsext [OPTION]... [FILE] [ARGUMENT]...\n\n"
       "Evaluates FILE. If it is an anonymous JavaScript function, arguments are passed to it\n\n"
       "Options:\n"
       "-r, --rcfile=RCFILE\n"
       "\tExecute commands from RCFILE instead of the system wide\n"
       "\tinitialization file" libdir "/jsext/0-init.js\n"
       "-e, --eval=EXPR\n"
       "\tEvaluate the expression\n"
       "-h, --help\n"
       "\tPrint help\n");
}

void ReportError(char *format, ...) {
  va_list va;

  va_start(va, format);
  vprintf(format,va);

  va_end(va);
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

static void *startthread(void *v) {
}

extern int optind;

int
main(int argc, char **argv, char **envp)
{
  JSRuntime *rt;

  char *buf=0;
  int fd=0;
  JSContext *cx=0;
  char *rcfile=0;
  char *evalexpr=0;
  JSObject *glob;
  int exitcode=0;
	jsval rval;

  for (;;) {
    static struct option long_options[] = {
      {"rcfile", 1, 0, 'r'},
      {"help", 0, 0, 'h'},
      {"eval", 1, 0, 'e'},
      {0, 0, 0, 0}
    };
    
    int c = getopt_long (argc, argv, "r:he:",
			 long_options,0);
    
    switch(c) {
    case 'e':
      evalexpr=optarg;
      break;
    case 'r':
      rcfile=optarg;
      break;
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
  exitcode=!JSX_init(cx, glob, rcfile, &rval);

  if (evalexpr && !exitcode) {
    exitcode=eval(cx, glob, evalexpr);
  }

  if (!exitcode && (!evalexpr || argc>optind)) {
    JSObject *obj=glob;
		/*
    jsval v;
    JS_GetProperty(cx, obj, "JSEXT", &v);
    obj=JSVAL_TO_OBJECT(v);
    JS_GetProperty(cx, obj, "shell", &v);
		*/
    exitcode=call(cx, obj, rval, argc-optind, argv+optind);
  }

  JS_RemoveRoot(cx, &rval);

  JS_DestroyContext(cx);
  JS_DestroyRuntime(rt);
  
  return exitcode;
  
 failure:
  if (cx) JS_DestroyContext(cx);
  if (rt) JS_DestroyRuntime(rt);
  return 1;
}

static int eval(JSContext *cx, JSObject *obj, char *expr) {
  JSScript *script=0;
  JSObject *scrobj=0;
  jsval tmpval=JSVAL_VOID;

  JS_AddRoot(cx, &tmpval);

  script=JS_CompileScript(cx, obj, expr, strlen(expr), "<expr>", 1);
  if (!script)
    goto failure;

  scrobj=JS_NewScriptObject(cx, script);
  JS_AddRoot(cx, &scrobj);
  
  if (!JS_ExecuteScript(cx, obj, script, &tmpval))
    goto failure;

  if (tmpval!=JSVAL_VOID && expr[strlen(expr)-1]!=';' && expr[strlen(expr)-1]!='}') {
	  puts(JS_GetStringBytes(JS_ValueToString(cx, tmpval)));
  }

  JS_RemoveRoot(cx, &scrobj);
  //  JS_DestroyScript(cx, script);

  JS_RemoveRoot(cx, &tmpval);
  return 0;

 failure:
  if (scrobj)
    JS_RemoveRoot(cx, &scrobj);
  //  if (script)
    //    JS_DestroyScript(cx, script);

  JS_RemoveRoot(cx, &tmpval);
  return 1;
}



static int call(JSContext *cx, JSObject *obj, jsval fun, int argc, char *argv[]) {
  jsval rval=JSVAL_VOID;
  jsval *jsargv=0;
  JSFunction *jsfun;
  int i;

  JS_AddRoot(cx, &rval);

  jsargv=(jsval *)JS_malloc(cx, sizeof(jsval) * (argc+1));
  if (!jsargv) {
    ReportError("Out of memory");
    goto failure;
  }

  for (i=0; i<argc; i++) {
    jsargv[i]=JSVAL_VOID;
    JS_AddRoot(cx, jsargv+i);
  }

  for (i=0; i<argc; i++) {
    JSString *S;
    S=JS_NewStringCopyZ(cx, argv[i]);
    if (!S)
      goto failure;
    jsargv[i]=STRING_TO_JSVAL(S);
  }

  jsfun=JS_ValueToFunction(cx, fun);
	if (!jsfun)
		goto failure;

  if (!JS_CallFunction(cx, obj, jsfun, argc, jsargv, &rval))
    goto failure;

  JS_RemoveRoot(cx, &rval);

  for (i=0; i<argc; i++) {
    JS_RemoveRoot(cx, jsargv+i);
  }  

  JS_free(cx,jsargv);

  if (JSVAL_IS_INT(rval))
    return INT_TO_JSVAL(rval);
  else
    return 0;

 failure:
  JS_RemoveRoot(cx, &rval);
  if (jsargv) {
    for (i=0; i<argc; i++)
      JS_RemoveRoot(cx, jsargv+i);
    JS_free(cx, jsargv);
  }

  return 1;
}

