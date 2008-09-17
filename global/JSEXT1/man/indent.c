#include "indent.h"
#include <stdlib.h>
#include <string.h>

int findIndent(char *s, int tabwidth) {
  int r=0;
  while (*s==' ' || *s=='\t') {
    if (*s==' ') r++;
    if (*s=='\t') r=(r/tabwidth+1)*tabwidth;
    s++;
  }
  if (*s==0) return -1;
  return r;
}

char *removeIndent(char *s, int tabwidth, int indent) {
  int hasindent=findIndent(s, tabwidth);
  if (hasindent==-1 || hasindent<indent)
    return 0;
  while (*s==' ' || *s=='\t')
    s++;

  char *ret=malloc(strlen(s)+hasindent-indent+1);
  int i;
  for (i=indent; i<hasindent; i++)
    ret[i-indent]=' ';
  ret[i-indent]=0;
  strcat(ret, s);
  return ret;
}
