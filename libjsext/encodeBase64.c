#include <jsapi.h>

static char *alpha="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static JSBool encodeBase64(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
  int length;
  int i;
  jschar *ret;
  jschar *str;
  jschar *in, *out;
  jschar *end;
  int outlen;
  
  union {
    char c[4];
    int i;
  } buf;

  if (argc<1 || !JSVAL_IS_STRING(argv[0]))
    return JS_TRUE;

  length=JS_GetStringLength(JSVAL_TO_STRING(argv[0]));
  if (length==0) {
    *rval=JS_GetEmptyStringValue(cx);
    return JS_TRUE;
  }

  str=JS_GetStringChars(JSVAL_TO_STRING(argv[0]));
  outlen=((length+2)/3)*4;
  outlen+=2*((outlen-1)/76);

  ret=JS_malloc(cx, sizeof(jschar)*outlen);
  *rval=STRING_TO_JSVAL(JS_NewUCString(cx, ret, outlen));

  out=ret;
  in=str;
  end=in+length;

  buf.i=0;

  while (in<end) {
    switch(end-in) {
    default:
      buf.c[1]=in[2];
    case 2:
      buf.c[2]=in[1];
    case 1:
      buf.c[3]=in[0];
    case 0:
      break;
    }

    if (out-ret && (out-ret+2)%78==0) {
      *(out++)='\r';
      *(out++)='\n';
    }

    out[0]=alpha[(buf.i >> 26) & 0x3f];
    out[1]=alpha[(buf.i >> 20) & 0x3f];
    out[2]=alpha[(buf.i >> 14) & 0x3f];
    out[3]=alpha[(buf.i >> 8) & 0x3f];
    out+=4;

    buf.i <<= 24;
    in+=3;
  }

  switch(in-end) {
  case 2:
    out[-2]='=';
  case 1:
    out[-1]='=';
  }

  return JSVAL_TRUE;
}


jsval make_encodeBase64(JSContext *cx) {
  JSFunction *jsfun = JS_NewFunction(cx, encodeBase64, 0, 0, 0, 0);
  if(!jsfun) return JS_FALSE;
  return OBJECT_TO_JSVAL(JS_GetFunctionObject(jsfun));
}
