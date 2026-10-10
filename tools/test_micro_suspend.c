#include <assert.h>
#include <string.h>
#include "f1micro.h"
int main(void) {
 F1MicroState s;int id,k;
 F1MicroReset(&s);
 for(id=1;id<=40;id++) {
  s.cars[id].seen=s.cars[id].haveCross=1;s.cars[id].lap=2;s.cars[id].clock=1000;
  for(k=0;k<30;k++){s.cars[id].personal[k]=1234;s.cars[id].colors[k]=3;s.best[k]=1234;}
 }
 F1MicroSuspend(&s);
 for(id=1;id<=40;id++) {
  assert(!s.cars[id].seen && !s.cars[id].haveCross);
  for(k=0;k<30;k++)assert(s.cars[id].personal[k]==1234 && s.cars[id].colors[k]==3 && s.best[k]==1234);
 }
 F1MicroSample(&s,1,5,4.2,200000,1);
 assert(!s.cars[1].haveCross && s.cars[1].personal[3]==1234 && s.best[3]==1234);
 F1MicroSample(&s,1,5,4.8,200100,1);
 F1MicroSample(&s,1,5,5.2,200200,1);
 assert(s.cars[1].haveCross && s.cars[1].personal[4]==1234);
 F1MicroReset(&s);assert(s.best[0]==0 && s.cars[1].personal[0]==0);
 return 0;
}
