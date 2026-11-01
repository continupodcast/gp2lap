#include <assert.h>
#include <string.h>
#define F1_Q_MAX 28
typedef struct {int id,pos,pit,outlap;long best,pb[3],live[2];int color[2];} F1QRow;
#include "../src/f1combined.h"
int main(void){
 F1QRow src[28],out[28];unsigned long fri[40]={0},sat[40]={0};int i;
 memset(src,0,sizeof(src));for(i=0;i<28;i++)src[i].id=i+1;
 src[0].best=92000;src[1].best=91000;fri[0]=90000;sat[1]=89000;
 src[0].pit=1;fri[2]=0x20000000;fri[3]=0xf0000010;sat[4]=88000;
 assert(F1CombinedBuild(out,src,28,fri,sat,0x44)==28);
 assert(out[0].id==5 && out[0].best==88000);
 assert(out[1].id==2 && out[1].best==89000);
 assert(out[2].id==1 && out[2].best==90000 && out[2].pit==1);
 assert(out[3].best==0 && out[3].pos==0);
 assert(src[0].best==92000);
 F1CombinedBuild(out,src,28,fri,sat,0x40);
 assert(out[0].id==1 && out[0].best==90000);assert(out[1].id==2 && out[1].best==91000);
 F1CombinedBuild(out,src,28,0,0,0x44);assert(out[0].id==2 && out[0].best==91000);
 assert(!F1CombinedValid(0));assert(!F1CombinedValid(0x20000000));
 return 0;
}
