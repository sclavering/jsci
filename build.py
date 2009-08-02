#!/usr/bin/env python

from fabricate import *


setup(dirs = ['.', 'jsext', 'global'])


def clean():
  autoclean()


def build():
  print "Building js/ (SpiderMonkey) ..."
  js()
  print "ok"

  print "Building jsx binaries ..."
  jsx()
  print "ok"

  print "Building clib.jswrapper ..."
  shell('make -C global')
  print "ok"


def js():
  shell('make -C js/src -f Makefile.ref BUILD_OPT=1', silent = False)


jsx_C = ['clib', 'encodeJSON', 'decodeJSON', 'encodeUTF8', 'decodeUTF8', 'encodeBase64', 'decodeBase64']

jsx_CC = ['jsext', 'Dl', 'Pointer', 'Type', 'stringifyHTML', 'JsciType', 'JsciTypeVoid', 'JsciTypeNumeric', 'JsciTypeInt', 'JsciTypeUint', 'JsciTypeFloat', 'JsciTypePointer', 'JsciTypeStructUnion', 'JsciTypeStruct', 'JsciTypeUnion', 'JsciTypeBitfield', 'JsciTypeArray', 'JsciTypeFunction', 'JsciPointer', 'JsciPointerAlloc', 'JsciCallback']

def jsx():
  flags = "-Wall -O3 -DXP_UNIX -Ijs/src/Linux_All_OPT.OBJ -Ijs/src"
  for s in jsx_C:  run("gcc -c %s libjsext/%s.c -o libjsext/%s.o" % (flags, s, s))
  for s in jsx_CC: run("g++ -c %s -fno-exceptions libjsext/%s.cc -o libjsext/%s.o" % (flags, s, s))
  objects = ' '.join(['libjsext/' + s + '.o' for s in jsx_C + jsx_CC])
  run('g++ -Ljs/src/Linux_All_OPT.OBJ -lm -pthread -rdynamic -ljs -ldl -lffi -o libjsext/jsext ' + objects)


def clib_wrapper():
  shell("cd global ; JSEXT_INI='0-makeclib.js' ../jsext > clib.jswrapper");


main()
