#ifndef _XML_H
#define _XML_H

#include <stdio.h>

struct Attribute {
	char *name;
	char *value;
};

struct Xml {
	char *tag;
	int nAttrib;
	struct Attribute *attrib;
	struct Xml *inner;
	char *text;
	struct Xml *next;
	struct Xml *last;
};

struct Xml *xml(char *tag, ...);
struct Xml *xml_text(char *tag, ...);
struct Xml *xml_link(struct Xml *e1, struct Xml *e2);
void xml_print(struct Xml *e);
void xml_free(struct Xml *e);
void c_unescape(char *in);

#endif
