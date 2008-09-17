#include "libctoxml.h"
#include <stdio.h>

main() {
  int i;
  for (i=0; i<10; i++) {
    char *C=(char *)malloc(2000);
    FILE *fp=fopen("../libcpp.h","rb");
    int c=fread(C,1,2000,fp);
    C[c]=0;
    //  "/*hei\n*/#define a 32\n";
    int errpos;
    char *xml=ctoxml(C,&errpos);
    printf("errpos=%d\n",errpos);
    fputs(xml,stderr);
    free(xml);
    fclose(fp);
    free(C);
  }
}
