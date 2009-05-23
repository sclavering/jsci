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
//void strbuf_catchar(struct strbuf *buf, char c);
void strbuf_clear(struct strbuf *buf);
void strbuf_free(struct strbuf *buf);

#define strbuf_catchar(_buf, c) {\
  if ((_buf)->len+1 < (_buf)->capacity) {\
    (_buf)->buf[(_buf)->len++]=c;\
    (_buf)->buf[(_buf)->len]=0;\
  } else {\
    char str[2]={c,0};\
    strbuf_cat((_buf),str);\
  }\
}

#endif
