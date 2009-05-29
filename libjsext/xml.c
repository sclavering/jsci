#include "ctoxml.h"
#include "xml.h"
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "strbuf.h"


void xml_free(XmlNode *e) {
  int i;
  for(i = 0; i < e->nAttrib; i++) {
    if(e->attrib[i].name) free(e->attrib[i].name);
    if(e->attrib[i].value) free(e->attrib[i].value);
  }
  if(e->nAttrib) free(e->attrib);
  if(e->text) free(e->text);
  if(e->inner) {
    XmlNode *i = e->inner;
    do {
      XmlNode *inext = i->next;
      xml_free(i);
      i = inext;
    } while(i != e->inner);
  }
  free(e);
}


XmlNode *xml_attrs(XmlNode *node, ...) {
  XmlNode *ret = node;
  va_list l;

  va_start(l, node);
  int nAttr = 0;
  for(;;) {
    char *value;
    char *name = va_arg(l, char *);
    if(name == 0) break;
    value = va_arg(l, char *);
    if(value == 0) break;
    nAttr++;
  }
  va_end(l);

  if(!nAttr) return ret;

  ret->nAttrib = nAttr;
  ret->attrib = (XmlAttr *) malloc(sizeof(XmlAttr) * nAttr);
  va_start(l, node);
  int i;
  for(i = 0; i != nAttr; ++i) {
    char *name = va_arg(l, char *);
    char *value = va_arg(l, char *);
    ret->attrib[i].name = strdup(name);
    ret->attrib[i].value = strdup(value);
  }
  va_end(l);

  return ret;
}


XmlNode *xml(char *tag, ...) {
  XmlNode *ret = xml_text(tag, 0);
  va_list l;
  va_start(l, tag);
  for(;;) {
    XmlNode *inner = va_arg(l, XmlNode *);
    if(inner == NULL) break;
    if(ret->inner == NULL) {
      ret->inner = inner;
    } else {
      ret->inner = xml_link(ret->inner, inner);
    }
  }
  va_end(l);
  return ret;
}


XmlNode *xml_text(char *tag, char *txt) {
  XmlNode *ret = (XmlNode *) malloc(sizeof(XmlNode));
  ret->text = txt ? strdup(txt) : 0;
  ret->nAttrib = 0;
  ret->inner = 0;
  ret->tag = tag;
  ret->next = ret;
  ret->last = ret;
  return ret;
}


XmlNode *xml_link(XmlNode *e1, XmlNode *e2) {
  XmlNode *newLast = e2->last;
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


static void _xml_print(XmlNode *e) {
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
    XmlNode *i = e->inner;
    do {
      _xml_print(i);
      i = i->next;
    } while(i != e->inner);
  }

  PUTS("</");
  PUTS(e->tag);
  PUTS(">");
}


void xml_print(XmlNode *e) {
  _xml_print(e);
  PUTS("\n");
}
