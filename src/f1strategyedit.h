#include "f1compoundrule.h"
#ifndef F1STRATEGYEDIT_H
#define F1STRATEGYEDIT_H
/* Pure draft operations: never touch live car state before confirmation. */
static int F1DraftValid(const AutoPlan *p,int laps,int quick)
{
 int i,total=0,mask=0;if(p->count<1 || p->count>4)return 0;
 for(i=0;i<p->count;i++){
  if(p->compound[i]>2 || p->length[i]<1 || p->length[i]>90)return 0;
  total+=p->length[i];mask|=1<<p->compound[i];
 }
 return total==laps && (quick || p->count>=2) &&
  (!StrategyRequireCompoundChange || p->count==1 || (mask&(mask-1)));
}
static int F1DraftStops(AutoPlan *p,int count,int laps,int quick)
{
 int i;AutoPlan next=*p;
 if(count<(quick?1:2) || count>4 || laps<count || laps>90*count)return 0;
 next.count=(unsigned char)count;
 for(i=0;i<count;i++){
  next.length[i]=(unsigned char)(laps/count+(i<laps%count));
  if(i>=p->count)next.compound[i]=(unsigned char)((next.compound[i-1]+1)%3);
 }
 *p=next;return 1;
}
static int F1DraftMovePit(AutoPlan *p,int stint,int delta)
{
 int a,b;if(stint<0 || stint>=p->count-1)return 0;
 a=p->length[stint]+delta;b=p->length[stint+1]-delta;
 if(a<1 || a>90 || b<1 || b>90)return 0;
 p->length[stint]=(unsigned char)a;p->length[stint+1]=(unsigned char)b;return 1;
}
#endif
