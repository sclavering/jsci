#include <jsapi.h>

static char *alpha="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static JSBool decodeBase64(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
  int length;
  jschar *str;
  jschar *out;
  unsigned char *ptr;
  unsigned char *in;
  unsigned char *end;
  unsigned char *in2;
  static int conv[256];
  static int init=0;

  union {
    unsigned char c[4];
    int i;
  } buf;

  if (!init) {
    int i;

    init=1;

    for (i=0; i<256; i++)
      conv[i]=-1;

    for (i=0; i<64; i++)
      conv[alpha[i]]=i;
  }

  if (argc<1 || !JSVAL_IS_STRING(argv[0]))
    return JS_TRUE;

  length=JS_GetStringLength(JSVAL_TO_STRING(argv[0]));
  if (length==0) {
    *rval=JS_GetEmptyStringValue(cx);
    return JS_TRUE;
  }

  ptr=JS_GetStringBytes(JSVAL_TO_STRING(argv[0]));

  in=ptr;
  end=ptr+length;

  in2=in;
  while (in<end) {
    if (conv[*in]==-1)
      in++;
    else
      *(in2++)=*(in++);
  }

  end=in2;
  str=JS_malloc(cx, sizeof(jschar)*((3+end-ptr)/4)*3);

  out=str;
  in=ptr;

  while (in<end) {
    buf.i=0;

    switch(end-in) {
    default:
      buf.i |= conv[in[3]];
    case 3:
      buf.i |= conv[in[2]] << 6;
    case 2:
      buf.i |= conv[in[1]] << 12;
    case 1:
      buf.i |= conv[in[0]] << 18;
    }

    in+=4;

    *(out++)=buf.c[2];
    *(out++)=buf.c[1];
    *(out++)=buf.c[0];

  }

  switch(in-end) {
  case 2:
    out--;
  case 1:
    out--;
  }

  *rval=STRING_TO_JSVAL(JS_NewUCString(cx, str, out-str));

  return JS_TRUE;
}


jsval make_decodeBase64(JSContext *cx) {
  JSFunction *jsfun = JS_NewFunction(cx, decodeBase64, 0, 0, 0, 0);
  if(!jsfun) return JSVAL_VOID;
  return OBJECT_TO_JSVAL(JS_GetFunctionObject(jsfun));
}
