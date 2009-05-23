#include "ctoxml.h"
#include "xml.h"
#include <stdarg.h>
#include <stdlib.h>


void xml_free(struct Xml *e) {
  int i;
  for(i = 0; i < e->nAttrib; i++) {
    if(e->attrib[i].value) free(e->attrib[i].value);
  }
  if(e->nAttrib) free(e->attrib);
  if(e->text) free(e->text);
  if(e->inner) {
    struct Xml *i = e->inner;
    do {
      struct Xml *inext = i->next;
      xml_free(i);
      i = inext;
    } while(i != e->inner);
  }

  free(e);
}


struct Xml *xml(char *tag, ...) {
  struct Xml *ret = (struct Xml *) malloc(sizeof(struct Xml));
  va_list l;

  va_start(l, tag);
  ret->nAttrib = 0;
  for(;;) {
    char *value;
    char *name = va_arg(l, char *);
    if(name == 0) break;
    ret->nAttrib++;
    value = va_arg(l, char *);
    if(value == 0) break;
  }
  va_end(l);

  if(ret->nAttrib) {
    ret->attrib = (struct Attribute *) malloc(sizeof(struct Attribute) * ret->nAttrib);
  }

  va_start(l, tag);
  ret->nAttrib = 0;
  for(;;) {
    char *value;
    char *name = va_arg(l, char *);
    if(name == 0) break;
    value = va_arg(l, char *);
    ret->attrib[ret->nAttrib].name = name;
    ret->attrib[ret->nAttrib].value = value;
    ret->nAttrib++;
    if(value == 0) break;
  }

  ret->inner = NULL;

  for(;;) {
    struct Xml *inner = va_arg(l, struct Xml *);
    if(inner == NULL) break;
    if(ret->inner == NULL) {
      ret->inner = inner;
    } else {
      ret->inner = xml_link(ret->inner, inner);
    }
  }

  va_end(l);

  ret->tag = tag;
  ret->text = 0;
  ret->next = ret;
  ret->last = ret;
  return ret;
}


struct Xml *xml_text(char *tag, ...) {
  struct Xml *ret = (struct Xml *) malloc(sizeof(struct Xml));
  va_list l;

  va_start(l, tag);
  ret->nAttrib = 0;
  for(;;) {
    char *value;
    char *name = va_arg(l, char *);
    if(name == 0) break;
    ret->nAttrib++;
    value = va_arg(l, char *);
    if(value==0) break;
  }
  va_end(l);

  if(ret->nAttrib) ret->attrib = (struct Attribute *) malloc(sizeof(struct Attribute) * ret->nAttrib);

  va_start(l, tag);
  ret->nAttrib = 0;
  for(;;) {
    char *value;
    char *name = va_arg(l, char *);
    if(name == 0) break;
    value = va_arg(l, char *);
    ret->attrib[ret->nAttrib].name = name;
    ret->attrib[ret->nAttrib].value = value;
    ret->nAttrib++;
    if(value == 0) break;
  }

  ret->text = va_arg(l, char *);
  va_end(l);

  ret->inner = 0;
  ret->tag = tag;
  ret->next = ret;
  ret->last = ret;
  return ret;
}


struct Xml *xml_link(struct Xml *e1, struct Xml *e2) {
  struct Xml *newLast = e2->last;
  e1->last->next = e2;
  e2->last = e1->last;
  e1->last = newLast;
  newLast->next = e1;
  return e1;
}


static void xml_print_escaped(char *s) {
  char str[2];
  while(*s) {
    switch(*s) {
    case '<': PUTS("&lt;"); break;
    case '>': PUTS("&gt;"); break;
    case '&': PUTS("&amp;"); break;
    case '\'': PUTS("&apos;"); break;
    case '"': PUTS("&quot;"); break;
    default:
      str[0] = *s;
      str[1] = 0;
      PUTS(str);
    }
    s++;
  }
}


static void _xml_print(struct Xml *e) {
  int i;
  PUTS("<");
  if(e->tag) PUTS(e->tag);

  for(i = 0; i < e->nAttrib; i++) {
    PUTS(" ");
    PUTS(e->attrib[i].name);
    PUTS("='");
    if(e->attrib[i].value) xml_print_escaped(e->attrib[i].value);
    PUTS("'");
  }

  if(e->text == 0 && e->inner == 0) {
    PUTS("/>");
    return;
  }

  PUTS(">");

  if(e->text) xml_print_escaped(e->text);
  if(e->inner) {
    struct Xml *i = e->inner;
    do {
      _xml_print(i);
      i = i->next;
    } while(i != e->inner);
  }

  PUTS("</");
  PUTS(e->tag);
  PUTS(">");
}


void xml_print(struct Xml *e) {
  _xml_print(e);
  PUTS("\n");
}
