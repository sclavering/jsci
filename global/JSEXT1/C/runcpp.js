function(filename) {
  /*
  cpp is assumed to be the one from GCC.  Tested using "cpp (GCC) 4.2.4 (Ubuntu 4.2.4-1ubuntu3)"
  
  -dD
    keeps "#define FOO 42" and similar, so we can include such constants in the .jswrapper
    expands macros, so that function prototypes are in the form we can parse
  -undef stops cpp leaving __extension__ crap in libmysql.h's output (a gcc-ism)
  --std=gnu99 is just to try and get clib.h's output closer to what our old libcpp was doing
  */
  return JSEXT1.File.read("/usr/bin/cpp '" + filename + "' -dD -undef --std=gnu99 |");
}

/*
Note: the old libcpp hard-coded some standard .h files:

  float.h: empty
  stddef.h:
		 #ifndef _STDDEF_H
		 #define _STDDEF_H
		 typedef unsigned long size_t;
		 typedef short wchar_t;
		 typedef int ptrdiff_t;
     #define inline _inline
     #define NULL 0
		 #endif
   stdarg.h:
		 #ifndef _STDARG_H
		 #define _STDARG_H
		 typedef __builtin_va_list __gnuc_va_list;
		 typedef __builtin_va_list va_list;
		 #endif

Since switching to using GCC's cpp, we lack clib.va_list, and have loads of constants from float.h
*/
