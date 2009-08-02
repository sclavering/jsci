#!/usr/bin/env python

from fabricate import *



import os
os_arch = os.uname()[0] # "Darwin" or "Linux"
OBJDIR = 'build-' + os_arch
JS_SRC_DIR = 'js/src/'
JS_OS_CFLAGS = ""
if os_arch == "Linux":
  JS_OS_CFLAGS += " -DHAVE_LOCALTIME_R"
  if os.uname()[4] == "x86_64": JS_OS_CFLAGS += " -DHAVE_VA_COPY -DVA_COPY=va_copy -DPIC -fPIC"
elif os_arch == "Darwin":
  JS_NO_LIBM = 1 # don't link libm, though i suspect it doesn't matter
else:
  raise "We only support building on/for Darwin and Linux, not '%s'" % os_arch



setup(dirs = ['.', 'libjsext', 'global', OBJDIR, JS_SRC_DIR])


def clean():
  autoclean()


def build():
  if not os.path.exists(OBJDIR): os.mkdir(OBJDIR)
  if not os.path.isdir(OBJDIR): raise ("build directory %s is not a directory" % OBJDIR)

  print "Building js/ (SpiderMonkey) ..."
  js()
  print "ok"

  print "Building jsx binaries ..."
  jsx()
  print "ok"

  print "Building clib.jswrapper ..."
  shell('make -C global')
  print "ok"



js_C = ['jsapi', 'jsarena', 'jsarray', 'jsatom', 'jsbool', 'jscntxt', 'jsdate', 'jsdbgapi', 'jsdhash', 'jsdtoa', 'jsemit', 'jsexn', 'jsfun', 'jsgc', 'jshash', 'jsinterp', 'jsinvoke', 'jsiter', 'jslock', 'jslog2', 'jslong', 'jsmath', 'jsnum', 'jsobj', 'jsopcode', 'jsparse', 'jsprf', 'jsregexp', 'jsscan', 'jsscope', 'jsscript', 'jsstr', 'jsutil', 'jsxdrapi', 'jsxml', 'prmjtime']

js_CFLAGS = '-Wall -Wno-format -MMD -Os -UDEBUG -DXP_UNIX -DSVR4 -DSYSV -D_BSD_SOURCE -DPOSIX_SOURCE -I' + OBJDIR

def js():
  # shell('make -C js/src -f Makefile.shiny', silent = False)
  print "Creating jsautokw.h" 
  #   run('gcc -o %s/jskwgen.o %s %s/jskwgen.c' % (OBJDIR, CFLAGS, OBJDIR))
  #   gcc -o $(OBJDIR)/jskwgen $(CFLAGS) $(LDFLAGS) $(OBJDIR)/jskwgen.o
  run('gcc -o %s/jskwgen %s %s/jskwgen.c' % (OBJDIR, js_CFLAGS, JS_SRC_DIR))
  run('%s/jskwgen %s/jsautokw.h' % (OBJDIR, OBJDIR))
  # build jsautocfg.h
  run('gcc -o %s/jscpucfg %s/jscpucfg.c' % (OBJDIR, JS_SRC_DIR))
  run('%s/jscpucfg > %s/jsautocfg.h' % (OBJDIR, OBJDIR))
  # xxx need to append JS_OS_CFLAGS
  for s in js_C: run('gcc -c %s -o %s/%s.o %s/%s.c' % (js_CFLAGS, OBJDIR, s, JS_SRC_DIR, s))



jsx_C = ['clib', 'encodeJSON', 'decodeJSON', 'encodeUTF8', 'decodeUTF8', 'encodeBase64', 'decodeBase64']

jsx_CC = ['jsext', 'Dl', 'Pointer', 'Type', 'stringifyHTML', 'JsciType', 'JsciTypeVoid', 'JsciTypeNumeric', 'JsciTypeInt', 'JsciTypeUint', 'JsciTypeFloat', 'JsciTypePointer', 'JsciTypeStructUnion', 'JsciTypeStruct', 'JsciTypeUnion', 'JsciTypeBitfield', 'JsciTypeArray', 'JsciTypeFunction', 'JsciPointer', 'JsciPointerAlloc', 'JsciCallback']

def jsx():
  flags = "-Wall -O3 -DXP_UNIX -I%s -I%s" % (OBJDIR, JS_SRC_DIR)
  for s in jsx_C:  run("gcc -c %s libjsext/%s.c -o %s/%s.o" % (flags, s, OBJDIR, s))
  for s in jsx_CC: run("g++ -c %s -fno-exceptions libjsext/%s.cc -o %s/%s.o" % (flags, s, OBJDIR, s))
  js_objects = ' '.join([OBJDIR + '/' + s + '.o' for s in js_C])
  jsx_objects = ' '.join([OBJDIR + '/' + s + '.o' for s in jsx_C + jsx_CC])
  run('g++ -lm -pthread -rdynamic -ldl -lffi -o %s/jsext %s %s' % (OBJDIR, js_objects, jsx_objects))


def clib_wrapper():
  shell("cd global ; JSEXT_INI='0-makeclib.js' ../jsext > clib.jswrapper");


main()
