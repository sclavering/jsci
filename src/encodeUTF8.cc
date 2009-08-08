/*
pseudo_string = encodeUTF8(string [, modified])

Takes a normal js string (i.e. UTF16), converts it to UTF8, and then returns it as a javascript string where each UTF8 has been padded out to a 16bit character.  (The ffi layer discards the high bytes of strings, so this ultimately does the right thing for sending UTF8 strings to C code, including outputting text.)

If the optional |modified| argument is |true|, any zero characters in the input string are encoded as 0xc0 0x80 to avoid null bytes in the eventual C string (decodeUTF8() will automatically undo this transformation).
*/

#include <jsapi.h>

static JSBool encodeUTF8(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
  int length;
  int outlen;
  int i;
  jschar *ret;
  jschar *str;
  jschar *p;
  int mod=0;

  if (argc<1 || !JSVAL_IS_STRING(argv[0]))
    return JS_TRUE;

  if (argc>1 && argv[1]==JSVAL_TRUE)
    mod=1;

  length=JS_GetStringLength(JSVAL_TO_STRING(argv[0]));
  if (length==0) {
    *rval=JS_GetEmptyStringValue(cx);
    return JS_TRUE;
  }
  str=JS_GetStringChars(JSVAL_TO_STRING(argv[0]));
  outlen=0;

  for (i=0; i<length; i++) {
    if (mod && str[i]==0)
      outlen+=2;
    else if (str[i]<0x80)
      outlen++;
    else if (str[i]<0x800)
      outlen+=2;
    else
      outlen+=3;
  }

  ret = (jschar*) JS_malloc(cx, sizeof(jschar) * outlen);
  *rval=STRING_TO_JSVAL(JS_NewUCString(cx, ret, outlen));
  p=ret;

  for (i=0; i<length; i++) {
    if (mod && *str==0) {
      *(p++)=0xc0;
      *(p++)=0x80;
    } else if (*str<0x80) {
      *(p++)=*str;
    } else if (*str<0x800) {
      *(p++)=0xc0 | (*str >> 6);
      *(p++)=0x80 | (*str & 0x3f);
    } else {
      *(p++)=0xe0 | (*str >> 12);
      *(p++)=0x80 | ((*str >> 6) & 0x3f);
      *(p++)=0x80 | (*str & 0x3f);
    }
    str++;
  }

  return JSVAL_TRUE;
}


jsval make_encodeUTF8(JSContext *cx) {
  JSFunction *jsfun = JS_NewFunction(cx, encodeUTF8, 0, 0, 0, 0);
  if(!jsfun) return JS_FALSE;
  return OBJECT_TO_JSVAL(JS_GetFunctionObject(jsfun));
}
