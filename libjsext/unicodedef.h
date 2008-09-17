#ifndef _unicodedef_h
#define _unicodedef_h

#ifdef _WIN32
#include <windows.h>
#else
typedef char TCHAR;
#endif

#ifdef UNICODE
#define JS_GetStringTChars JS_GetStringChars
#define JS_NewStringTCopyZ JS_NewUCStringCopyZ
#define JS_DefineProperty(cx, obj, name, value, getter, setter, attrs) \
  JS_DefineUCProperty((cx),(obj),(name),wcslen(name),(value),(getter),(setter),(attrs))
#else
#define JS_NewStringTCopyZ JS_NewStringCopyZ
#define JS_GetStringTChars JS_GetStringBytes
#endif

#endif
