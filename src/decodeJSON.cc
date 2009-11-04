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


static int find_unescaped_string_length(struct JSON *s) {
  jschar *p = s->p;
  int len = 0;

  p++; // skip "
  for(;;) {
    switch(*p) {
      case '\"':
        return len;
      case 0:
        return -1;
      case '\\':
        switch(*(++p)) {
          // '\b' and '\/' seem a bit odd, but they're in the JSON spec
          case '\"': case '\\': case '/': case 'b': case 'f': case 'n': case 'r': case 't':
            break;
          case 'u': {
            for(int i = 0; i != 4; ++i) {
              switch(*(++p)) {
                case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
                case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
                case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
                  break;
                default:
                  return -1;
              }
            }
            break;
          }
          default:
            return -1;
        }
        // fall through
      default:
        ++p;
        ++len;
    }
  }
  return len;
}


// call this after find_unescaped_string_length()
static jschar* parse_and_unescape_string(struct JSON *s, int unescaped_length) {
  jschar *buf = (jschar*) JS_malloc(s->cx, sizeof(jschar) * unescaped_length);
  if(!buf) return 0;
  jschar *ret = buf;

  s->p++; // skip "
  for (;;) {
    switch (*s->p) {
      case '\"':
        s->p++;
        return ret;
      case '\\':
        s->p++;
        switch(*(s->p++)) {
          case '\"':
            *(buf++) = '\"';
            break;
          case '\\':
            *(buf++) = '\\';
            break;
          case '/':
          case 'b':
            *(buf++) = '/';
            break;
          case 'f':
            *(buf++) = '\f';
            break;
          case 'n':
            *(buf++) = '\n';
            break;
          case 'r':
            *(buf++) = '\r';
            break;
          case 't':
            *(buf++) = '\t';
            break;
          case 'u': {
            int val = 0;
            for(int i = 0; i != 4; ++i) {
              val = val << 4;
              switch(*s->p) {
                case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
                  val |= *(s->p) - '0';
                  break;
                case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
                  val |= *(s->p) - 'a' + 10;
                  break;
                case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
                  val |= *(s->p) - 'A' + 10;
                  break;
              }
              s->p++;
            }
            *(buf++) = val;
            break;
          }
          default:
            *(buf++) = s->p[-1];
        }
        break;
      default:
        *(buf++) = *(s->p++);
    }
  }
  return 0; // unreachable
}


static JSBool parse_string(struct JSON *s) {
  int len = find_unescaped_string_length(s);
  if(len == -1) return syntaxerror(s);
  jschar *buf = parse_and_unescape_string(s, len);
  JSString *ret = JS_NewUCString(s->cx, buf, len);
  if(!ret) return JS_FALSE;
  *s->vp = STRING_TO_JSVAL(ret);
  return JS_TRUE;
}


static JSBool parse_object(struct JSON *s) {
  JSObject *object=JS_NewObject(s->cx, 0, 0, 0);
  jsval prop;

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
      case '"': {
        int namelen = find_unescaped_string_length(s);
        if(namelen == -1) return JS_FALSE;
        jschar *name = parse_and_unescape_string(s, namelen);

        // scan for colon
        while(*s->p != ':') {
          switch(*(s->p++)) {
            case ' ': case '\t': case '\n': case '\r':
              break;
            default:
              return syntaxerror(s);
          }
        }
        s->p++;

        s->vp = &prop;
        if(!parse_value(s, '}')) return JS_FALSE;
        if(!JS_SetUCProperty(s->cx, object, name, namelen, &prop)) return JS_FALSE;

        // Scan for comma
        while(*s->p != ',') {
          switch(*(s->p++)) {
            case '}':
              return JS_TRUE;
            case ' ': case '\t': case '\n': case '\r':
              break;
            default:
              return syntaxerror(s);
          }
        }
        s->p++;
        break;
      }
      default:
        return syntaxerror(s);
    }
  }

  // not reached
  return JS_FALSE;
}


static JSBool parse_true(struct JSON *s) {
  s->p++; //t
  if(*(s->p++) != 'r') return syntaxerror(s);
  if(*(s->p++) != 'u') return syntaxerror(s);
  if(*(s->p++) != 'e') return syntaxerror(s);
  *s->vp = JSVAL_TRUE;
  return JS_TRUE;
}


static JSBool parse_false(struct JSON *s) {
  s->p++; //f
  if(*(s->p++) != 'a') return syntaxerror(s);
  if(*(s->p++) != 'l') return syntaxerror(s);
  if(*(s->p++) != 's') return syntaxerror(s);
  if(*(s->p++) != 'e') return syntaxerror(s);
  *s->vp=JSVAL_FALSE;
  return JS_TRUE;
}


static JSBool parse_null(struct JSON *s) {
  s->p++; //n
  if(*(s->p++) != 'u') return syntaxerror(s);
  if(*(s->p++) != 'l') return syntaxerror(s);
  if(*(s->p++) != 'l') return syntaxerror(s);
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
    case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
      n=(jsdouble) (*s->p-'0');
      s->p++;
      for (;;) {
        switch(*s->p) {
          case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
            n=n*10+(*s->p-'0');
            s->p++;
            break;
          case '.': case 'e': case 'E':
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
      case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
        n+=mul*(*s->p-'0');
        s->p++;
        mul/=10.;
        break;
      case 'e': case 'E':
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
    case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
      break;
    case '-':
      expsgn = -1;
      s->p++;
      break;
    default:
      return syntaxerror(s);
  }

  for (;;) {
    switch(*s->p) {
      case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
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
        }
        return syntaxerror(s);
      case ']':
      case '}':
        if(*s->p == end) return JS_TRUE;
        return syntaxerror(s);
      case '"':
        return parse_string(s);
      case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
      case '-': case '+':
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
    if((*s->p) != ' ' && (*s->p) != '\t' && (*s->p) != '\n' && (*s->p) != '\r') break;
    s->p++;
  }

  if (*s->p==']') {
    s->p++;
    return JS_TRUE;
  }

  for (;;) {
    s->vp=&elem;
    if(!parse_value(s, ']')) return JS_FALSE;
    if(!JS_SetElement(s->cx, array, index++, &elem)) return JS_FALSE;

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


static JSBool decodeJSON(JSContext *cx,  JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
  if(argc < 1 || !JSVAL_IS_STRING(argv[0])) return JSX_ReportException(cx, "Missing or illegal argument to decodeJSON");

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
