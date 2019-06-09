#include <stdio.h>
#include "font.h"

unsigned int rev(char num) 
{ 
    unsigned int NO_OF_BITS = 8;
    unsigned int reverse_num = 0, i, temp; 
  
    for (i = 0; i < NO_OF_BITS; i++) 
    { 
        temp = (num & (1 << i)); 
        if(temp) 
            reverse_num |= (1 << ((NO_OF_BITS - 1) - i)); 
    } 
   
    return reverse_num; 
}

int main(int argc, char* argv[]) {
printf ("char font8x8_basic[128][8] = {\n");
   for (int i=0;i<128;i++) {
      printf ("{0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x}, // %c\n",
         rev(font8x8_basic[i][0])&0xff,
         rev(font8x8_basic[i][1])&0xff,
         rev(font8x8_basic[i][2])&0xff,
         rev(font8x8_basic[i][3])&0xff,
         rev(font8x8_basic[i][4])&0xff,
         rev(font8x8_basic[i][5])&0xff,
         rev(font8x8_basic[i][6])&0xff,
         rev(font8x8_basic[i][7])&0xff,
         (i >= 33 && i <=126 && i != 92) ? i : ' '
      );
   }
printf ("};\n");

}
