//
// view_chargen.c
//
// BMC64 - A bare metal Commodore emulator for Raspberry Pi
// Copyright (C) 2019 Randy Rossi <randy.rossi@gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

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
