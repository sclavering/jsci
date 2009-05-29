#ifndef STRBUF_H
#define STRBUF_H

struct strbuf {
  char *buf;
  int len;
  int capacity;
  char *ptr;
};

struct strbuf *strbuf_new(void);
void strbuf_cat(struct strbuf *buf, char *str);
void strbuf_free(struct strbuf *buf);

#endif
