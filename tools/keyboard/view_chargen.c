#include <stdio.h>
#include <stdlib.h>

int main() {
   FILE* fp = fopen("CHARGEN","r");
   unsigned char* d[4096];

   fread(d,1,4096,fp);

   fclose(fp);

   unsigned char *p = &d[0];
   for (int i=0;i<512;i++) {
       printf ("%d\n",i);
       for (int r=0;r<8;r++) {
          unsigned char v = *p;
          for (int q=7;q>=0;q--) {
            if (v & 1<<q) printf ("*");
            else printf (" ");
          }
          printf ("\n");
          p++;
       }
       printf ("\n");
   }
}
