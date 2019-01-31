/*
  indent options for VICE style:

    -gnu        start with GNU defaults

    -i4         indent 4 spaces
    -nut        Use spaces instead of tabs.
    
    -cli4       indent case label 4 spaces
    
    -bli0       no extra indent for braces
    -br         braces in same line as if
    -ce         cuddling else
    
    -npsl       function name on same line as function type
    
    -npcs       Do not put space after the function in function calls.
    -bad        blank line after a block of declarations and after each function body
*/
int funcA(int a){if(a==0){b=1;}else{b=0;}return b;}int funcB(int a){switch(a){case 0:b=0;break;case 1:b=1;break;}return b;}
void funcC(int a){int b;while(a<b){b++;}do{b++;}while(a<=b);}void funcD(int a){int b;for(a=0;a<10;a++){b++;}}
int func(int a,int b,int c){funcD(funcA(a+b));funcB(b);}
