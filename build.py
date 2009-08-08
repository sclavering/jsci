#!/usr/bin/env python

from fabricate import *


import os
os_arch = os.uname()[0] # "Darwin" or "Linux"
OBJDIR = 'build-' + os_arch
JS_SRC_DIR = 'spidermonkey18'
JS_OS_CFLAGS = ""
if os_arch == "Linux":
  JS_OS_CFLAGS += " -DHAVE_LOCALTIME_R"
  if os.uname()[4] == "x86_64": JS_OS_CFLAGS += " -DHAVE_VA_COPY -DVA_COPY=va_copy -DPIC -fPIC"
elif os_arch == "Darwin":
  JS_NO_LIBM = 1 # don't link libm, though i suspect it doesn't matter
else:
  raise "We only support building on/for Darwin and Linux, not '%s'" % os_arch


setup(dirs = ['.', 'src', 'global', OBJDIR, JS_SRC_DIR])


def clean():
  autoclean()


def build():
  if not os.path.exists(OBJDIR): os.mkdir(OBJDIR)
  if not os.path.isdir(OBJDIR): raise ("build directory %s is not a directory" % OBJDIR)
  js()
  jsx()
  link()
  clib_wrapper()


js_C = ['jsapi', 'jsarena', 'jsarray', 'jsatom', 'jsbool', 'jscntxt', 'jsdate', 'jsdbgapi', 'jsdhash', 'jsdtoa', 'jsemit', 'jsexn', 'jsfun', 'jsgc', 'jshash', 'jsinterp', 'jsinvoke', 'jsiter', 'jslock', 'jslog2', 'jslong', 'jsmath', 'jsnum', 'jsobj', 'jsopcode', 'jsparse', 'jsprf', 'jsregexp', 'jsscan', 'jsscope', 'jsscript', 'jsstr', 'jsutil', 'jsxdrapi', 'jsxml', 'prmjtime']

js_CFLAGS = '-Wall -Wno-format -Os -UDEBUG -DXP_UNIX -DSVR4 -DSYSV -D_BSD_SOURCE -DPOSIX_SOURCE -I' + OBJDIR

def js():
  run('gcc -o %s/jskwgen %s %s/jskwgen.c' % (OBJDIR, js_CFLAGS, JS_SRC_DIR))
  run('%s/jskwgen %s/jsautokw.h' % (OBJDIR, OBJDIR))
  # build jsautocfg.h
  run('gcc -o %s/jscpucfg %s/jscpucfg.c' % (OBJDIR, JS_SRC_DIR))
  run('%s/jscpucfg > %s/jsautocfg.h' % (OBJDIR, OBJDIR))
  # xxx need to append JS_OS_CFLAGS
  for s in js_C: run('gcc -c %s -o %s/%s.o %s/%s.c' % (js_CFLAGS, OBJDIR, s, JS_SRC_DIR, s))


jsx_CC = ['jsext', 'Dl', 'Pointer', 'Type', 'clib', 'encodeJSON', 'decodeJSON', 'encodeUTF8', 'decodeUTF8', 'stringifyHTML', 'JsciType', 'JsciTypeVoid', 'JsciTypeNumeric', 'JsciTypeInt', 'JsciTypeUint', 'JsciTypeFloat', 'JsciTypePointer', 'JsciTypeStructUnion', 'JsciTypeStruct', 'JsciTypeUnion', 'JsciTypeBitfield', 'JsciTypeArray', 'JsciTypeFunction', 'JsciPointer', 'JsciPointerAlloc', 'JsciCallback']

def jsx():
  flags = "-Wall -O3 -DXP_UNIX -I%s -I%s" % (OBJDIR, JS_SRC_DIR)
  for s in jsx_CC: run("g++ -x c++ -c %s -fno-exceptions src/%s.cc -o %s/%s.o" % (flags, s, OBJDIR, s))


def link():
  objects = ' '.join([OBJDIR + '/' + s + '.o' for s in js_C + jsx_CC])
  run('g++ -lm -pthread -rdynamic -ldl -lffi -o %s/jsext %s' % (OBJDIR, objects))


def clib_wrapper():
  run("JSEXT_INI='wrap_libc.js' ./jsext > global/clib.jswrapper");


main()
