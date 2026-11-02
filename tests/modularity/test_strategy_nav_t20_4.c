#include <assert.h>
#include <stdio.h>
#include "../../src/f1strategynav.h"
int main(void){int c,f,s;
 for(c=2;c<=4;c++){
  f=2;s=1;F1StrategyNavigate(c,0x4d,&f,&s);assert(f==3 && s==0);
  F1StrategyNavigate(c,0x4d,&f,&s);assert(f==3 && s==1);
  F1StrategyNavigate(c,0x4b,&f,&s);assert(f==3 && s==0);
  F1StrategyNavigate(c,0x4b,&f,&s);assert(f==2 && s==1);
  f=2;s=0;F1StrategyNavigate(c,0x50,&f,&s);assert(f==4 && s==0);
  F1StrategyNavigate(c,0x48,&f,&s);assert(f==2 && s==0);
 }
 for(c=1;c<=4;c++){
  f=2*c+2;s=0;F1StrategyNavigate(c,0x4d,&f,&s);assert(f==2*c+3);
  F1StrategyNavigate(c,0x4b,&f,&s);assert(f==2*c+2);
  f=0;s=0;F1StrategyNavigate(c,0x48,&f,&s);assert(f==0 && s==0);
  F1StrategyNavigate(c,0x4b,&f,&s);assert(f==0 && s==0);
 }
 puts("PASS horizontal TYRE/pit navigation, vertical stint rows, footer left/right, no edge wrap for 0..3 stops");return 0;}
