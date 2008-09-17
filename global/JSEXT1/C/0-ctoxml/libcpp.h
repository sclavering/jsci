#ifndef LIBCPP_H
#define LIBCPP_H


/*
  C is a 0-terminated string
  errorpos, if not null, will be used to store strdup of 1st error message or null if parsing was ok
  Returns 0-terminated string allocated with malloc.
 */

#if defined _WIN32 && defined MAKE_LIB
__declspec(dllexport)
#endif
char *cpp(char *C, char **errormsg, char **include_path);

#if defined _WIN32 && defined MAKE_LIB
__declspec(dllexport)
#endif
void cpp_free(char *C);

#endif

#ifdef _WIN32
#pragma JSEXT dl "./libcpp.dll"
#else
#pragma JSEXT dl "./libcpp.so"
#endif
