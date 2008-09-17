#include "strbuf.h"
#include <stdlib.h>
#include <string.h>

struct strbuf *strbuf_new() {
  struct strbuf *buf=(struct strbuf *)malloc(sizeof(struct strbuf));
  buf->len=0;
  buf->capacity=256;
  buf->buf=calloc(256,1);
  buf->ptr=buf->buf;
  return buf;
}

void strbuf_free(struct strbuf *buf) {
  free(buf->buf);
  free(buf);
}

/*
void strbuf_cpy(struct strbuf *buf, char *str) {
  int len=strlen(str);
  if (len+1>buf->capacity) {
    buf->capacity=len+1;
    buf->buf=realloc(buf->buf, buf->capacity);
    if (!buf->buf) exit(1);
  }
  buf->ptr=buf->buf;
  memcpy(buf->buf, str, len+1);
  buf->len=len;
}
*/

void strbuf_ncpy(struct strbuf *buf, char *str, int n) {
  if (n+1>buf->capacity) {
    buf->capacity=n+1;
    buf->buf=realloc(buf->buf, buf->capacity);
    if (!buf->buf) exit(1);
  }
  buf->ptr=buf->buf;
  memcpy(buf->buf, str, n);
  buf->buf[n]=0;
  buf->len=n;
}

void strbuf_clear(struct strbuf *buf) {
  //  memset(buf->buf,0,buf->len);
  buf->ptr=buf->buf;
  *(buf->ptr)=0;
  buf->len=0;
}

/*
void strbuf_catchar(struct strbuf *buf, char c) {
  if (buf->len+1 < buf->capacity) {
    buf->buf[buf->len++]=c;
    buf->buf[buf->len]=0;
  } else {
    char str[2]={c,0};
    strbuf_cat(buf,str);
  }
}
*/

void strbuf_cat(struct strbuf *buf, char *str) {
  int len=strlen(str);
  while (len+buf->len+1 > buf->capacity) {
    int offset=buf->ptr-buf->buf;
    buf->capacity*=2;
    buf->buf=realloc(buf->buf, buf->capacity);
    if (!buf->buf) exit(1);
    buf->ptr=buf->buf+offset;
    //    memset(buf->buf+buf->len+len,0,buf->capacity-buf->len-len);
  }

  memcpy(buf->buf+buf->len, str, len+1);
  buf->len+=len;
}

void strbuf_ncat(struct strbuf *buf, char *str, int n) {
  int len=strlen(str);
  if (len>n) len=n;
  while (len+buf->len+1 > buf->capacity) {
    int offset=buf->ptr-buf->buf;
    buf->capacity*=2;
    buf->buf=realloc(buf->buf, buf->capacity);
    if (!buf->buf) exit(1);
    buf->ptr=buf->buf+offset;
    //    memset(buf->buf+buf->len+len,0,buf->capacity-buf->len-len);
  }

  memcpy(buf->buf+buf->len, str, len);
  buf->len+=len;
  buf->buf[buf->len]=0;
}
