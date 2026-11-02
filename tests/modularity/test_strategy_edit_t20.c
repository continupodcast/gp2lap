unsigned long StrategyRequireCompoundChange=1;
#include <assert.h>
#include <stdio.h>
#include <string.h>
typedef struct {double score;unsigned char count,compound[4],length[4];} AutoPlan;
#include "../../src/f1strategyedit.h"
int main(void){int laps,count,i,j,d,total;AutoPlan p,b;
 for(laps=6;laps<=255;laps++)for(count=1;count<=4;count++){
  memset(&p,0,sizeof(p));p.count=1;p.length[0]=6;
  if(laps>90*count)continue;
  assert(F1DraftStops(&p,count,laps,1));total=0;
  for(i=0;i<count;i++){assert(p.length[i]>=1 && p.length[i]<=90);total+=p.length[i];}
  assert(total==laps && F1DraftValid(&p,laps,1));
  for(i=0;i<count-1;i++)for(d=-1;d<=1;d+=2){b=p;F1DraftMovePit(&b,i,d);total=0;
   for(j=0;j<count;j++){assert(b.length[j]>=1 && b.length[j]<=90);total+=b.length[j];}
   assert(total==laps);
  }
 }
 memset(&p,0,sizeof(p));p.count=2;p.length[0]=20;p.length[1]=33;
 assert(!F1DraftValid(&p,53,0));p.compound[1]=2;assert(F1DraftValid(&p,53,0));
 b=p;assert(!F1DraftStops(&p,1,53,0));assert(!memcmp(&p,&b,sizeof(p)));
 p.count=1;p.length[0]=53;assert(F1DraftValid(&p,53,1));assert(!F1DraftValid(&p,53,0));
 puts("PASS draft distances 6..255, 0..3 stops, bounded pit edits, compound rule, Quickrace exception");return 0;
}
