#ifndef _XML_H
#define _XML_H

#include <stdio.h>

typedef struct _XmlAttr {
  char *name;
  char *value;
} XmlAttr;

typedef struct _XmlNode {
  char *tag;
  int nAttrib;
  XmlAttr *attrib;
  struct _XmlNode *inner;
  char *text;
  struct _XmlNode *next;
  struct _XmlNode *last;
} XmlNode;

// Create an XmlNode with XmlNode children (or no children)
XmlNode *xml(char *tag, ...);

// Create an XmlNode with a text node child
XmlNode *xml_text(char *tag, char *txt);

// Replace the attributes of a node and return it
XmlNode *xml_attrs(XmlNode *node, ...);

XmlNode *xml_link(XmlNode *e1, XmlNode *e2);
void xml_print(XmlNode *e);
void xml_free(XmlNode *e);
void c_unescape(char *in);

#endif
