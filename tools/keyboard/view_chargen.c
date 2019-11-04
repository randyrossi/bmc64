#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
   FILE* fp = fopen(argv[1],"r");
   int size = atoi(argv[2]);
   int off = atoi(argv[3]);

   unsigned char* d = (unsigned char*) malloc(size);
   fread(d,1,size,fp);
   fclose(fp);

   unsigned char *p = &d[off];
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
