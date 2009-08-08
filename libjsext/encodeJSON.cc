#include <jsapi.h>
#include <jsobj.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "util.h"
#include "jsdate.h"
#include "jsnum.h" // for JSDOUBLE_IS_NaN

struct JSON {
  JSContext *cx;
  jsval v;
  jschar *buf;
  jschar *p;
  int capacity;
  int recursion;
  int stacksize;
  int mod;
  jsval *stack;
};

#define swap(a,b) ((a)^=(b), (b)^=(a), (a)^=(b))

static inline void expand_buf(struct JSON *s, int n) {
  int len = s->p - s->buf;
  if(len+n > s->capacity) {
    s->capacity *= 2;
    if(len + n > s->capacity) s->capacity = len + n;
    s->buf = (jschar*) JS_realloc(s->cx, s->buf, sizeof(jschar) * s->capacity);
    s->p = s->buf + len;
  }
}

static inline void inflate_buf(struct JSON *s, int n) {
  int m = n;
  while(n--) s->p[n] = ((char *) s->p)[n];
  s->p += m;
}

static inline JSBool value_to_JSON(struct JSON *s);

static inline JSBool check_cycle(struct JSON *s) {
  jsval v = s->stack[s->recursion - 1];
  for(int i = s->recursion - 1; i--;) {
    if(s->stack[i] == v) {
      JSX_ReportException(s->cx, "Object contains cycles");
      return JS_TRUE;
    }
  }
  s->stacksize += 16;
  s->stack = (jsval*) JS_realloc(s->cx, s->stack, sizeof(jsval) * s->stacksize);
  return JS_FALSE;
}

static JSBool array_to_JSON(struct JSON *s) {
  int empty=1;
  JSObject *array=JSVAL_TO_OBJECT(s->v);

  s->stack[s->recursion]=s->v;
  s->recursion++;
  if (s->recursion==s->stacksize && check_cycle(s))
    return JS_FALSE;

  expand_buf(s, 1);
  *(s->p++)='[';

  jsuint len;
  if(!JS_GetArrayLength(s->cx, array, &len)) return JS_FALSE;

  for(int i = 0; i < len; i++) {
    jschar *before=s->p;

    OBJ_GET_PROPERTY(s->cx, array, INT_TO_JSID(i), &s->v);
    if (!value_to_JSON(s))
      return JS_FALSE;

    if (before!=s->p) { // Nothing written, skip comma
      expand_buf(s, 1);
      *(s->p++)=',';
      empty=0;
    }
  }

  s->recursion--;
  if (empty) {
    expand_buf(s, 1);
    *(s->p++)=']';
  } else {
    s->p[-1]=']';
  }
  return JS_TRUE;
}

static JSBool object_to_JSON(struct JSON *s) {
  int i, len;
  JSObject *object=JSVAL_TO_OBJECT(s->v);
  JSIdArray *id;

  s->stack[s->recursion]=s->v;
  s->recursion++;
  if (s->recursion==s->stacksize && check_cycle(s))
    return JS_FALSE;

  expand_buf(s, 1);
  *(s->p++)='{';

  id=JS_Enumerate(s->cx, object);

  len=id->length;

  i=0;
  if (i<len) for (;;) {
    jschar *rewind=s->p;
    jschar *before;

    if (!JS_IdToValue(s->cx, id->vector[i], &s->v))
      return JS_FALSE;
    if (JSVAL_IS_INT(s->v)) {
      expand_buf(s, 1);
      *(s->p++)='\"';
      if (!value_to_JSON(s))
	return JS_FALSE;
      expand_buf(s, 1);
      *(s->p++)='\"';
    } else
      if (!value_to_JSON(s))
	return JS_FALSE;
    expand_buf(s, 1);
    *(s->p++)=':';

    before=s->p;

    OBJ_GET_PROPERTY(s->cx, object, id->vector[i], &s->v);
    if (!value_to_JSON(s))
      return JS_FALSE;

    if (s->p==before)
      s->p=rewind;

    i++;
    if (i==len) break;

    if (s->p!=rewind) {
      expand_buf(s, 1);
      *(s->p++)=',';
    }
  }

  JS_DestroyIdArray(s->cx, id);

  s->recursion--;
  expand_buf(s, 1);
  *(s->p++)='}';

  return JS_TRUE;
}

static JSBool date_to_JSON(struct JSON *s) {
  int n;
  JSObject *date=JSVAL_TO_OBJECT(s->v);

  if (!js_DateIsValid(s->cx, date))
    return JS_FALSE;

  expand_buf(s, 22);
  n=sprintf((char *)s->p, "\"%04d-%02d-%02dT%02d:%02d:%02d\"",
	    js_DateGetYear(s->cx, date),
	    js_DateGetMonth(s->cx, date)+1,
	    js_DateGetDate(s->cx, date),
	    js_DateGetHours(s->cx, date),
	    js_DateGetMinutes(s->cx, date),
	    js_DateGetSeconds(s->cx, date));

  inflate_buf(s,n);
  return JS_TRUE;
}


static inline JSBool value_to_JSON(struct JSON *s) {
  jschar *str;
  int len;
  int tmpint;
  jsdouble tmpdbl;
  JSClass *clasp;
  JSObject *obj;

  switch(JSVAL_TAG(s->v)) {
  case JSVAL_OBJECT:
    if (s->v==JSVAL_NULL) {
      expand_buf(s, 4);
      *(s->p++)='n';
      *(s->p++)='u';
      *(s->p++)='l';
      *(s->p++)='l';
      break;
    }
    
    obj=JSVAL_TO_OBJECT(s->v);
    if (JS_IsArrayObject(s->cx, obj)) {
      if (!array_to_JSON(s))
	return JS_FALSE;
      break;
    }

    clasp=JS_GET_CLASS(s->cx, obj);
    if (strcmp(clasp->name, "Date")==0) {
      if (!date_to_JSON(s))
	return JS_FALSE;
      break;
    }

    if (!object_to_JSON(s))
      return JS_FALSE;
    break;

  case 1:
  case 3:
  case 5:
  case 7:
    if (s->v==JSVAL_VOID)
      break;

    expand_buf(s, 20);
    tmpint=JSVAL_TO_INT(s->v);
    tmpint = sprintf((char *)s->p, "%d", tmpint);
    inflate_buf(s, tmpint);
    break;

  case JSVAL_STRING:
    str=JS_GetStringChars(JSVAL_TO_STRING(s->v));
    len = JS_GetStringLength(JSVAL_TO_STRING(s->v));
    expand_buf(s, len * 6 + 2);
    *(s->p++)='\"';
    while(len--) {
      switch (*str) {
      case '\"':
	*(s->p++)='\\';
	*(s->p++)='\"';
	break;
      case '\\':
	*(s->p++)='\\';
	*(s->p++)='\\';
	break;
      case '\n':
	*(s->p++)='\\';
	*(s->p++)='n';
	break;
      case '\r':
	*(s->p++)='\\';
	*(s->p++)='r';
	break;
      case '/':
	if (s->mod)
	  *(s->p++)=*str;
	else {
	  *(s->p++)='\\';
	  *(s->p++)='/';
	}
	break;
      case '\b':
	if (s->mod)
	  *(s->p++)=*str;
	else {
	  *(s->p++)='\\';
	  *(s->p++)='b';
	}
	break;
      case '\f':
	if (s->mod)
	  *(s->p++)=*str;
	else {
	  *(s->p++)='\\';
	  *(s->p++)='f';
	}
	break;
      case '\t':
	if (s->mod)
	  *(s->p++)=*str;
	else {
	  *(s->p++)='\\';
	  *(s->p++)='t';
	}
	break;
      case 0:
	tmpint = sprintf((char *)s->p, "\\u%04x", *str);
	inflate_buf(s, tmpint);
	break;
      case 1:
      case 2:
      case 3:
      case 4:
      case 5:
      case 6:
      case 7:
      case 11:
      case 14:
      case 15:
      case 16:
      case 17:
      case 18:
      case 19:
      case 20:
      case 21:
      case 22:
      case 23:
      case 24:
      case 25:
      case 26:
      case 27:
      case 28:
      case 29:
      case 30:
      case 31:
	if (!s->mod) {
	  tmpint = sprintf((char *)s->p, "\\u%04x", *str);
	  inflate_buf(s, tmpint);
	  break;
	}

	// else fall through

      default:
	*(s->p++)=*str;
      }
      str++;
    }
    *(s->p++)='\"';
    break;

  case JSVAL_BOOLEAN:
    expand_buf(s, 5);

    switch(s->v) {
    case JSVAL_TRUE:
      *(s->p++)='t';
      *(s->p++)='r';
      *(s->p++)='u';
      *(s->p++)='e';
      break;
    case JSVAL_FALSE:
      *(s->p++)='f';
      *(s->p++)='a';
      *(s->p++)='l';
      *(s->p++)='s';
      *(s->p++)='e';
      break;
    }
    break;

  case JSVAL_DOUBLE:
    tmpdbl=*JSVAL_TO_DOUBLE(s->v);
    if(JSDOUBLE_IS_NaN(tmpdbl)) tmpdbl = 0.;
    tmpint = sprintf((char *)s->p, "%.20lg", tmpdbl);
    inflate_buf(s, tmpint);
    break;
  }

  return JS_TRUE;
}

static JSBool encodeJSON(JSContext *cx,  JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
  struct JSON s;

  if (argc<1)
    return JS_TRUE;

  s.cx=cx;
  s.v=argv[0];
  s.capacity=512;
  s.buf = s.p = (jschar*) JS_malloc(cx, sizeof(jschar) * s.capacity);
  if (!s.buf)
    return JS_FALSE;

  s.stack = (jsval*) JS_malloc(cx, sizeof(jsval) * 16);
  if (!s.stack) {
    JS_free(cx, s.buf);
    return JS_FALSE;
  }

  s.stacksize=16;
  s.recursion=0;
  s.mod=0;

  if (argc>1 && argv[1]==JSVAL_TRUE)
    s.mod=1;

  if (!value_to_JSON(&s)) {
    JS_free(cx, s.buf);
    JS_free(cx, s.stack);
    return JS_FALSE;
  }

  *rval=STRING_TO_JSVAL(JS_NewUCStringCopyN(cx, s.buf, s.p-s.buf));
  JS_free(cx, s.buf);
  JS_free(cx, s.stack);

  return JS_TRUE;
}


extern "C"
jsval make_encodeJSON(JSContext *cx) {
  JSFunction *jsfun = JS_NewFunction(cx, encodeJSON, 0, 0, 0, 0);
  if(!jsfun) return JS_FALSE;
  return OBJECT_TO_JSVAL(JS_GetFunctionObject(jsfun));
}
