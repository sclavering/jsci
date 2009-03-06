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
#ifdef _WIN32
# include <windows.h>
#else
# include <getopt.h>
#endif
#include "unicodedef.h"
#include "libjsext.h"
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef _WIN32
# include <io.h>
#else
# include <unistd.h>
#endif
#include <string.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>

static int call(JSContext *cx, JSObject *obj, jsval fun, int argc, char *argv[]);
static int eval(JSContext *cx, JSObject *obj, char *expr);

static void printhelp() {
#ifdef _WIN32
  puts("jsext [OPTION]... [FILE] [ARGUMENT]...\n\n"
       "Evaluates FILE. If it is an anonymous JavaScript function, arguments are passed to it\n\n"
       "Options:\n"
       "/r RCFILE\n"
       "\tExecute commands from RCFILE instead of the\n"
	   "\tfile named 0-init.js in the 'global' subdirectory\n"
       "/e EXPR\n"
       "\tEvaluate the expression\n"
       "/?\n"
       "\tPrint help\n");
#else
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
#endif
}

void ReportError(char *format, ...) {
  va_list va;

  va_start(va, format);
  vprintf(format,va);

  va_end(va);
}

#ifdef _WINDOWS
static void
my_ErrorReporter(JSContext *cx, const char *message, JSErrorReport *report)
{
  TCHAR errbuf[1024];
  errbuf[0]=0;
#ifdef UNICODE
  if (report->lineno && report->filename) _snwprintf(errbuf, 1024,L"Line %d in %S:",report->lineno,report->filename);
  if (message) _snwprintf(errbuf+wcslen(errbuf),1024-wcslen(errbuf),L"%S\n",message);
  MessageBox(0,errbuf,L"Error",MB_OK);
#else
  if (report->lineno && report->filename) _snprintf(errbuf, 1024,"Line %d in %s:",report->lineno,report->filename);
  if (message) _snprintf(errbuf+strlen(errbuf),1024-strlen(errbuf),"%s\n",message);
  MessageBox(0,errbuf,"Error",MB_OK);
#endif
}
#else
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
#endif

static void *startthread(void *v) {
}

#ifdef _WINDOWS

int
main(int _argc, char **_argv, char **_envp);

/*
  This function is based on a forum posting by Bo Peng.
  http://www.mail-archive.com/lyx-devel@lists.lyx.org/msg97790.html
 */

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prev_instance, char* command_line, int show_command)
{

  int argc=0;
  int argcap=16;
  char **argv;
  int result;
  char *cs;
  char filename[_MAX_PATH];
  
  // tokenize the arguments
  
  cs=command_line;
  argcap=128;
  argv=(char **)malloc(sizeof(char *)*argcap);
  
  // put the program name into argv[0]

  GetModuleFileNameA(NULL, filename, _MAX_PATH);
  argv[0] = strdup(filename);
  argc=1;

  for(;;) {
    while (*cs==' ') ++cs; // eat spaces between arguments
    if (!*cs) break;
    
    if (argc==argcap) {
      argcap*=2;
      argv=(char **)realloc(argv, sizeof(char *)*argcap);
    }
    if (*cs=='"') {
      char *out;
      cs++;
      argv[argc++]=cs;
      out=cs;
      
      for(;;) {
	if (!*cs) break;
	if (cs[0]=='"' && cs[1]=='"' && cs[2]=='"') cs+=2;
	else if (cs[0]=='"') break;
	*(out++)=*(cs++);
      }
      *out=0;
    } else {
      argv[argc++]=cs;
      if (strchr(cs,' ')) {
	cs=strchr(cs,' ');
	*(cs++)=0;
      } else {
	break;
      }
    }
  };

  result = main(argc, argv, 0);
  
  free(argv);
  return result;
}
#endif

#ifdef _WIN32
int optind=1;
#else
extern int optind;
#endif

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

#ifdef _WIN32

  while (optind < argc) {
    if (strcmp(argv[optind],"/r")==0 && argc>optind+1) {
      rcfile=argv[++optind];
      optind++;
    } else if (strcmp(argv[optind],"/?")==0) {
      printhelp();
      exit(0);
    } else if (strcmp(argv[optind],"/e")==0 && argc>optind+1) {
      evalexpr=argv[++optind];
      optind++;
    } else break;
  }
  
#else

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

#endif


  rt=JS_NewRuntime(64000000);
  if (!rt) goto failure;

  cx=JS_NewContext(rt, 65536);
  if (!cx) goto failure;

  #ifdef JS_THREADSAFE
  JS_BeginRequest(cx);
  //  JS_SetContextThread(cx);
  #endif

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

#ifdef JS_THREADSAFE
  JS_EndRequest(cx);
#endif
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

