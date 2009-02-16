#pragma JSEXT recursion 1
#include <fcgiapp.h>

#ifdef _WIN32
#pragma JSEXT dl "./libfcgi.dll"
#else
#pragma JSEXT dl "libfcgi.so.0"
#endif
