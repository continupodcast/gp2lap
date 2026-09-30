#include <assert.h>
#include <stdio.h>
#include "../src/f1loops.h"
static void sample(int id,double progress,unsigned long clock,int pit)
{
 int lap=(int)(progress/30);
 FIPitSample(id,lap+1,progress-lap*30,clock,1,pit);
}
int main(void)
{
 unsigned long t;long saved;
 FIReset();
 for(t=3000;t<=34000;t+=100) {
  sample(1,t/1000.0,t,0);sample(2,t/1000.0-1,t,0);
  sample(3,t/1000.0-2,t,0);FICollect(2,1,0,t);
 }
 saved=fiBucket[2][0].display;assert(saved==1000);
 /* Rank temporarily points to another driver while the predecessor pits. */
 for(t=34100;t<=40000;t+=100) {
  FIPitSample(1,2,-1,t,1,1);sample(2,t/1000.0-1,t,0);
  sample(3,t/1000.0-2,t,0);
  FICollect(2,3,0,t);assert(fiBucket[2][0].display==-1);
  FICollect(2,1,0,t);assert(fiBucket[2][0].display==saved);
  assert(fiBucket[2][0].count<=6); /* v0.32 keeps available common points. */
 }
 /* Rejoin ahead: even after six own crossings the follower has no common
    new passage yet. Keep the pair's gap across finish and rank churn. */
 for(t=40100;t<=70000;t+=100) {
  sample(1,50+(t-40100)/1000.0,t,0);
  sample(2,t/1000.0-1,t,0);sample(3,t/1000.0-2,t,0);
  FICollect(2,1,0,t);
  assert(fiBucket[2][0].display>=0);
  if(t==47000) {
   assert(!fiCars[1].pitHold);
   FICollect(2,3,0,t);assert(fiBucket[2][0].display==-1);
   FICollect(2,1,0,t);assert(fiBucket[2][0].display==saved);
  }
 }
 assert(fiBucket[2][0].display==10900);
 /* Retirement must not resurrect a cached comparison. */
 FIPitSample(1,3,20,70100,0,0);FICollect(2,1,0,70100);
 assert(fiBucket[2][0].display==-1);assert(!fiPairs[2][1].valid);
 FIReset();assert(!fiPairs[2][1].valid);
 assert(FIPosition(1,1,1,0,1000)==0);
 puts("PASS: pair restore, pit freeze, rejoin beyond six points, finish line, partial recovery, no wrong opponent, retirement and reset");
 return 0;
}
