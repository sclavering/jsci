/*
str = decodeUTF8(pseudo_str)

Javascript strings are supposed to be UTF16, but when the ffi layer converts a C string to a javascript one, it just pads each byte out with a high zero byte.  This function takes such a string, treats it as UTF8, ignoring the padding, and converts it to a proper UTF16 string.
*/

#include <jsapi.h>

static JSBool decodeUTF8(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
  int length;
  jschar *p;
  jschar *end=0;
  jschar *str;
  jschar *s;
  jschar *ptr;

  if (argc<1 || !JSVAL_IS_STRING(argv[0]))
    return JS_TRUE;

  length=JS_GetStringLength(JSVAL_TO_STRING(argv[0]));
  ptr=JS_GetStringChars(JSVAL_TO_STRING(argv[0]));

  if (length==0) {
    *rval=JS_GetEmptyStringValue(cx);
    return JS_TRUE;
  }

  p=ptr;

  end=p+length;

  length=0;
  while (p < end) {
    length++;

    switch(*p >> 4) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
      p++;
      break;
    case 8:  //1000
    case 9:  //1001
      // syntax error, read as 8-bit character
    case 10: //1010
    case 11: //1011
      p++;
      break;
    case 12: //1100
    case 13: //1101
      p+=2;
      break;
    case 14: //1110
      p+=3;
      break;
    case 15:
      p+=4;
      break;
    }
  }

  p=ptr;

  str = (jschar*) JS_malloc(cx, sizeof(jschar) * length);
  s=str;

  while (p < end) {
    switch(*p >> 4) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
      *(s++)=*(p++);
      break;
    case 8:  //1000
    case 9:  //1001
      // syntax error, read as 8-bit character
    case 10: //1010
    case 11: //1011
      *(s++)=*(p++);
      break;
    case 12: //1100
    case 13: //1101
      *(s++)=((p[0] & 0x1f) << 6) | (p[1] & 0x3f);
      p+=2;
      break;
    case 14: //1110
      *(s++)=((p[0] & 0x0f) << 12) | ((p[1] & 0x3f) << 6) | (p[2] & 0x3f);
      p+=3;
      break;
    case 15: // outside JS character range, use lower 16 bits
      *(s++)=((p[1] & 0x0f) << 12) | ((p[2] & 0x3f) << 6) | (p[3] & 0x3f);
      p+=4;
      break;
    }
  }

  *rval=STRING_TO_JSVAL(JS_NewUCString(cx, str, length));

  return JS_TRUE;
}


jsval make_decodeUTF8(JSContext *cx) {
  JSFunction *jsfun = JS_NewFunction(cx, decodeUTF8, 0, 0, 0, 0);
  if(!jsfun) return JS_FALSE;
  return OBJECT_TO_JSVAL(JS_GetFunctionObject(jsfun));
}
