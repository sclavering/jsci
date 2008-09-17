#include "libcpp.h"
#include <stdio.h>

main() {
  char *C=(char *)malloc(2000);
  FILE *fp=fopen("../libcpp.h","r");
  int c=fread(C,1,2000,fp);
  C[c]=0;
  //  char *C="hei\n#include\"t.h\"\n";
  //  char *C="#include <stdio.h>\n";
  //  char *C="#hei";

  int i;
  for (i=0; i<1; i++) {
    char *error=0;
    char *xml=cpp(C,&error);
    if (error) {
      printf("error:%s\n",error);
      free(error);
    }

    fputs(xml,stderr);
    free(xml);
  }

  return 0;
}
