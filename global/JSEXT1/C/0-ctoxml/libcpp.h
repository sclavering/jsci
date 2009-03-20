#ifndef LIBCPP_H
#define LIBCPP_H

/*
  C is a 0-terminated string
  errorpos, if not null, will be used to store strdup of 1st error message or null if parsing was ok
  Returns 0-terminated string allocated with malloc.
 */

char *cpp(char *C, char **errormsg, char **include_path);

void cpp_free(char *C);

#endif

#pragma JSEXT dl "./libcpp.so"
