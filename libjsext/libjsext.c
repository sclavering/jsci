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
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdarg.h>
#include "unicodedef.h"

static char *strip_file_name(char *ini_file);

# include <dlfcn.h>
# include <unistd.h>

#include <string.h>


static JSBool exec(JSContext *cx, JSObject *obj, char *filename, jsval *rval);
static JSBool JSX_ReportException(JSContext *cx, char *format, ...);
static JSBool dl(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);

/*
  Initializes jsext with ini_file from
  global_path if not null
  environment variable JSEXT_INI if set
  compiler option $prefix/lib/jsext-$version.js
 */

JSBool JSX_init(JSContext *cx, JSObject *obj, char *ini_file, jsval *rval) {
  JSObject *config;
  jsval *argv;
  int i;
  char *ifdup=0;
  char *lastslash;
  char *filename;
  JSFunction *jsfun;
  char cwd[1024];

  JS_SetOptions(cx, JS_GetOptions(cx) | JSOPTION_VAROBJFIX);

#ifdef JS_THREADSAFExx
  JS_BeginRequest(cx);
  JS_SetContextThread(cx);
#endif

  argv=JS_malloc(cx, sizeof(jsval)*4);
  for(i=0; i<4; i++) {
    argv[i]=JSVAL_VOID;
    JS_AddRoot(cx, argv+i);
  }

  config=JS_NewObject(cx, NULL, NULL, NULL);
  argv[0]=OBJECT_TO_JSVAL(config);
#ifdef __unix__
  *rval=JSVAL_TRUE;
  JS_SetProperty (cx, config, "__unix__", rval);
#endif
#ifdef __linux__
  *rval=JSVAL_TRUE;
  JS_SetProperty (cx, config, "__linux__", rval);
#endif
#ifdef __FreeBSD__
  *rval=JSVAL_TRUE;
  JS_SetProperty (cx, config, "__FreeBSD__", rval);
#endif
#ifdef JS_THREADSAFE
  *rval=JSVAL_TRUE;
  JS_SetProperty (cx, config, "JS_THREADSAFE", rval);
#endif
  *rval=JSVAL_TRUE;
  JS_SetProperty (cx, config, "host", rval);

  if (ini_file==NULL) {
    ini_file=getenv("JSEXT_INI");
  }
  
  if (ini_file==NULL) {
    ini_file=libdir "/jsext/0-init.js";
  }

  argv[3]=STRING_TO_JSVAL(JS_NewStringCopyZ(cx,getcwd(cwd, 1024)));

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

  jsfun=JS_NewFunction(cx, dl, 0, 0, 0, 0);
  
  argv[2]=OBJECT_TO_JSVAL(JS_GetFunctionObject(jsfun));
  argv[1]=OBJECT_TO_JSVAL(config);
  argv[0]=STRING_TO_JSVAL(JS_NewStringCopyZ(cx,ini_file));

  jsfun=JS_ValueToFunction(cx, *rval);
  if (!jsfun) {
    JSX_ReportException(cx, "Ini file does not evaluate to a function");
    goto failure;
  }

  if (!JS_CallFunction(cx, obj, jsfun, 4, argv, rval))
    goto failure;

  for (i=4; i--;)
    JS_RemoveRoot(cx, argv+i);
  JS_free(cx, argv);
  free(ifdup);
  
#ifdef JS_THREADSAFExx
  JS_EndRequest(cx);
  JS_ClearContextThread(cx);
#endif

  return JS_TRUE;

 failure:
  for (i=4; i--;)
    JS_RemoveRoot(cx, argv+i);
  JS_free(cx, argv);
  if (ifdup)
    free(ifdup);
  
#ifdef JS_THREADSAFExx
  JS_EndRequest(cx);
  JS_ClearContextThread(cx);
#endif

  return JS_FALSE;
  
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

static JSBool
dl(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
  JSNative JSX_init=0;
  TCHAR *filename;

#ifdef UNICODE
  if (!JS_ConvertArguments(cx, argc, argv, "W", &filename)) {
#else
  if (!JS_ConvertArguments(cx, argc, argv, "s", &filename)) {
#endif
    JSX_ReportException(cx, "Wrong parameter");
    return JS_FALSE;
  }
  
  void *dl;
    
  dl=dlopen(filename, RTLD_LAZY);

  if (!dl) {
    JSX_ReportException(cx, "Unable to open dl %s", dlerror());
    return JS_FALSE;
  }

  JSX_init=(JSNative) dlsym(dl,"JSX_init");

  if (JSX_init) { // Call init
    if (!(*JSX_init)(cx, obj, 0, 0, rval))
      return JS_FALSE;

  } else {
#ifdef UNICODE
    JSX_ReportException(cx, "Unable to locate JSX_init in '%S'",filename);
#else
    JSX_ReportException(cx, "Unable to locate JSX_init in '%s'",filename);
#endif
    return JS_FALSE;
  }

  return JS_TRUE;
}

static char *strip_file_name(char *ini_file) {
  char *lastslash=strrchr(ini_file,'/');
  if (lastslash)
	return lastslash;
  else
    return 0;
}
