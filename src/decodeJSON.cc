/*
val = decodeJSON(str)

Decodes a string in JSON format into a JavaScript object, array, string, number or null.  Written in C for maximum speed.
*/

#include <math.h>
#include <jsapi.h>
#include <jsobj.h>
#include <jsfun.h>
#include <stdarg.h>
#include "util.h"

struct JSON {
  jschar *p;
  jschar *start;
  JSContext *cx;
  jsval *vp;
};

static JSBool syntaxerror(struct JSON *s) {
  JSX_ReportException(s->cx, "JSON: Syntax error at offset %d",s->p-s->start);
  return JS_FALSE;
}

static JSBool parse_array(struct JSON *s);
static JSBool parse_value(struct JSON *s, jschar end);

static inline int parse_unescape(struct JSON *s) {
  int val=0;
  int i;
  jschar *p=s->p;
  jschar *start=p;

  s->p++; // "
  for (;;) {
    switch (*s->p) {
    case '\\':
      s->p++;
      switch(*(s->p++)) {
      case '\"':
	*(p++)='\"';
	break;
      case '\\':
	*(p++)='\\';
	break;
      case '/':
      case 'b':
	*(p++)='/';
	break;
      case 'f':
	*(p++)='\f';
	break;
      case 'n':
	*(p++)='\n';
	break;
      case 'r':
	*(p++)='\r';
	break;
      case 't':
	*(p++)='\t';
	break;
      case 'u':
	val=0;
	for (i=4; --i;) {
	  switch(*s->p) {
	  case '0':
	  case '1':
	  case '2':
	  case '3':
	  case '4':
	  case '5':
	  case '6':
	  case '7':
	  case '8':
	  case '9':
	    val = val<<4 | (*(s->p)-'0');
	    s->p++;
	    break;
	  case 'a':
	  case 'b':
	  case 'c':
	  case 'd':
	  case 'e':
	  case 'f':
	    val = val<<4 | (*(s->p)-'a'+10);
	    s->p++;
	    break;
	  case 'A':
	  case 'B':
	  case 'C':
	  case 'D':
	  case 'E':
	  case 'F':
	    val = val<<4 | (*(s->p)-'A'+10);
	    s->p++;
	    break;
	  default:
	    return -1;
	  }
	}
	*(p++)=val;
	break;

      default:
	*(p++)=s->p[-1];
      }
      break;

    case 0:
      return -1;

    case '\"':
      s->p++;
      return p-start;

    default:
      *(p++)=*(s->p++);
    }
  }
}

static JSBool parse_string(struct JSON *s) {
  jschar *start=s->p;
  int len=parse_unescape(s);
  JSString *ret;
  
  if (len==-1)
    return syntaxerror(s);

  ret=JS_NewUCStringCopyN(s->cx, start, len);
  if (!ret)
    return JS_FALSE;

  *s->vp=STRING_TO_JSVAL(ret);

  return JS_TRUE;
}

static JSBool parse_object(struct JSON *s) {
  JSObject *object=JS_NewObject(s->cx, 0, 0, 0);
  jsval prop;
  jschar *name;
  int namelen;

  *s->vp=OBJECT_TO_JSVAL(object);

  s->p++; // {

  for (;;) {
    switch(*s->p) {
    case ' ':
    case '\t':
    case '\n':
    case '\r':
      s->p++;
      break;

    case '}':
      s->p++;
      return JS_TRUE;
    case '"':
      name=s->p;
      namelen=parse_unescape(s);
      if (namelen==-1)
	return JS_FALSE;
      
      // scan for colon
      while (*s->p!=':') {
	switch(*(s->p++)) {
	case ' ':
	case '\t':
	case '\n':
	case '\r':
	  break;
	default:
	  return syntaxerror(s);
	}
      }
      s->p++;

      s->vp=&prop;
      if (!parse_value(s, '}'))
	return JS_FALSE;
      if (!JS_SetUCProperty(s->cx, object, name, namelen, &prop))
	return JS_FALSE;

      // Scan for comma
      
      while (*s->p!=',') {
	switch(*(s->p++)) {
	case '}':
	  return JS_TRUE;
	case ' ':
	case '\t':
	case '\n':
	case '\r':
	  break;
	default:
	  return syntaxerror(s);
	}
      }
      s->p++;
      break;

    default:
      return syntaxerror(s);
    }
  }

  // not reached
  return JS_FALSE;
}

static JSBool parse_true(struct JSON *s) {
  s->p++; //t
  if (*(s->p++)!='r')
    return syntaxerror(s);
  if (*(s->p++)!='u')
    return syntaxerror(s);
  if (*(s->p++)!='e')
    return syntaxerror(s);
  *s->vp=JSVAL_TRUE;
  return JS_TRUE;
}

static JSBool parse_false(struct JSON *s) {
  s->p++; //f
  if (*(s->p++)!='a')
    return syntaxerror(s);
  if (*(s->p++)!='l')
    return syntaxerror(s);
  if (*(s->p++)!='s')
    return syntaxerror(s);
  if (*(s->p++)!='e')
    return syntaxerror(s);
  *s->vp=JSVAL_FALSE;
  return JS_TRUE;
}

static JSBool parse_null(struct JSON *s) {
  s->p++; //n
  if (*(s->p++)!='u')
    return syntaxerror(s);
  if (*(s->p++)!='l')
    return syntaxerror(s);
  if (*(s->p++)!='l')
    return syntaxerror(s);
  *s->vp=JSVAL_NULL;
  return JS_TRUE;
}

static JSBool parse_number(struct JSON *s) {
  jsdouble n=0.;
  int sgn=-1;
  int expsgn=1;
  jsdouble mul;
  int expn=0;

  if (*s->p=='-')
    s->p++;
  else
    sgn=1;

  switch(*s->p) {
  case '0':
    s->p++;
    goto intend;

  case '1':
  case '2':
  case '3':
  case '4':
  case '5':
  case '6':
  case '7':
  case '8':
  case '9':
    n=(jsdouble) (*s->p-'0');
    s->p++;
    for (;;) {
      switch(*s->p) {
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
	n=n*10+(*s->p-'0');
	s->p++;
	break;

      case '.':
      case 'e':
      case 'E':
	goto intend;
	
      default:
	goto end;
      }
    }
  default:
    return syntaxerror(s);
  }

  
 intend:
  switch(*s->p) {
  case '.':
    s->p++;
    goto fraction;
  case 'e':
  case 'E':
    goto exp;
  default:
    goto end;
  }

 fraction:
  mul=0.1;
  for (;;) {
    switch(*s->p) {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      n+=mul*(*s->p-'0');
      s->p++;
      mul/=10.;
      break;
    case 'e':
    case 'E':
      goto exp;
    default:
      goto end;
    }
  }

 exp:
  s->p++; //e or E
  switch(*s->p) {
  case '+':
    s->p++;
    // fall through
  case '0':
  case '1':
  case '2':
  case '3':
  case '4':
  case '5':
  case '6':
  case '7':
  case '8':
  case '9':
    break;
  case '-':
    expsgn=-1;
    s->p++;
    break;
  default:
    return syntaxerror(s);
  }

  for (;;) {
    switch(*s->p) {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      expn=expn*10+*(s->p++)-'0';
      break;
    default:
      goto expend;
    }
  }

 expend:
  n*=pow(10., expn*expsgn);

 end:
  return JS_NewNumberValue(s->cx, sgn==1?n:-n, s->vp);
}

static JSBool parse_value(struct JSON *s, jschar end) {
  for (;;) {
    switch(*s->p) {
    case ',':
      if (end!=0) {
	*s->vp=JSVAL_VOID;
	return JS_TRUE;
      } else
	return syntaxerror(s);

    case ']':
    case '}':
      if (*s->p==end) {
	return JS_TRUE;
      } else
	return syntaxerror(s);

    case '"':
      return parse_string(s);

    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
    case '-':
    case '+':
      return parse_number(s);

    case 't':
      return parse_true(s);

    case 'f':
      return parse_false(s);

    case 'n':
      return parse_null(s);

    case '[':
      return parse_array(s);

    case '{':
      return parse_object(s);

    case ' ':
    case '\t':
    case '\n':
    case '\r':
      s->p++;
      break;

    default:
      return syntaxerror(s);
    }
  }
}

static JSBool parse_array(struct JSON *s) {
  JSObject *array=JS_NewArrayObject(s->cx, 0, 0);
  jsval elem;
  int index=0;

  *s->vp=OBJECT_TO_JSVAL(array);

  s->p++; // [

  for (;;) {
    if ((*s->p)!=' ' &&
	(*s->p)!='\t' &&
	(*s->p)!='\n' &&
	(*s->p)!='\r') break;
    s->p++;
  }

  if (*s->p==']') {
    s->p++;
    return JS_TRUE;
  }

  for (;;) {
    s->vp=&elem;
    if (!parse_value(s, ']'))
      return JS_FALSE;
    if (!JS_SetElement(s->cx, array, index++, &elem))
      return JS_FALSE;

    // Scan for comma

    while (*s->p!=',') {
      switch(*(s->p++)) {
      case ']':
	return JS_TRUE;
      case ' ':
      case '\t':
      case '\n':
      case '\r':
	break;
      default:
	return syntaxerror(s);
      }
    }
    s->p++;
  }

  // not reached
  return JS_FALSE;
}

struct rec {
  JSContext *cx;
  JSFunction *fun;
  JSObject *obj;
  jsval id;
  jsval v;
  jsval tmp;
};

static JSBool recurse(struct rec *r) {
  if (JSVAL_IS_OBJECT(r->v) &&
      !JSVAL_IS_NULL(r->v)) {
    JSObject *object=JSVAL_TO_OBJECT(r->v);
    JSIdArray *id;
    int i;
    int len;
    jsval idval=r->id;
    jsval v=r->v;

    id=JS_Enumerate(r->cx, object);

    len=id->length;
    
    for (i=0; i<len; i++) {
      if (!JS_IdToValue(r->cx, id->vector[i], &r->id))
	return JS_FALSE;

      OBJ_GET_PROPERTY(r->cx, object, id->vector[i], &r->v);

      if (!recurse(r))
	return JS_FALSE;

      OBJ_SET_PROPERTY(r->cx, object, id->vector[i], &r->v);
    }    

    JS_DestroyIdArray(r->cx, id);

    r->v=v;
    r->id=idval;
  }

  if (!JS_CallFunction(r->cx, r->obj, r->fun, 2, &r->id, &r->tmp))
    return JS_FALSE;

  r->v=r->tmp;
  return JS_TRUE;
}

static JSBool decodeJSON(JSContext *cx,  JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
  if(argc < 1 || !JSVAL_IS_STRING(argv[0])) {
    return JSX_ReportException(cx, "Missing or illegal argument to decodeJSON");
  }

  struct JSON s;
  s.vp=rval;
  s.cx=cx;
  s.p=JS_GetStringChars(JS_ValueToString(cx, argv[0]));
  s.start=s.p;

  return parse_value(&s, 0);
}


jsval make_decodeJSON(JSContext *cx) {
  JSFunction *jsfun = JS_NewFunction(cx, decodeJSON, 0, 0, 0, 0);
  if(!jsfun) return JSVAL_VOID;
  return OBJECT_TO_JSVAL(JS_GetFunctionObject(jsfun));
}
