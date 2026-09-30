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
 unsigned long t,generation;long saved;int k;FIPassage *p,*q;
 FIReset();
 for(t=3000;t<=34000;t+=100) {
  sample(1,t/1000.0,t,0);sample(2,t/1000.0-1,t,0);sample(3,t/1000.0-3,t,0);
  FICollect(2,1,0,t);FICollect(3,1,0,t);FICollect(3,1,1,t);
  if(t<31000)assert(fiBucket[2][0].display==-1);
  else assert(fiBucket[2][0].display==1000);
  if(t<33000)assert(fiBucket[3][0].display==-1);
  else assert(fiBucket[3][0].display==3000);
 }
 FICollect(3,2,0,34000);assert(fiBucket[3][0].display==2000);
 assert(fiBucket[3][1].display==3000);
 saved=fiBucket[2][0].display;generation=fiCars[2].generation;
 for(t=34100;t<=50000;t+=100) {
  sample(1,t/1000.0,t,0);FIPitSample(2,2,-1,t,1,1);FICollect(2,1,0,t);
  assert(fiBucket[2][0].display==saved);assert(fiCars[2].generation==generation);
  assert(fiCars[2].passage[33].valid);
 }
 /* Rejoin at point 39.5: no interpolation over the pit-lane path. */
 for(t=50100;t<=57000;t+=100) {
  sample(1,t/1000.0,t,0);sample(2,39.5+(t-50100)/1000.0,t,0);FICollect(2,1,0,t);
  assert(fiCars[2].generation==generation);
  if(t<50600)assert(fiBucket[2][0].display==saved);
  else assert(fiBucket[2][0].display==10600);
 }
 assert(fiBucket[2][0].count==6);
 /* Preceding car in pit lane: missing common passages cannot erase gap. */
 saved=fiBucket[2][0].display;
 for(t=57100;t<=63000;t+=100) {
  FIPitSample(1,2,-1,t,1,1);sample(2,39.5+(t-50100)/1000.0,t,0);FICollect(2,1,0,t);
  assert(fiBucket[2][0].display>=0);
 }
 /* New opponent with no common history: first actual point is publishable. */
 FIReset();fiCars[2].lap=2;fiCars[1].lap=2;
 for(k=31;k<=39;k++) {
  fiCars[2].seen=fiCars[1].seen=1;fiCars[2].clock=fiCars[1].clock=k*1000;
  fiCars[2].progress=k;fiCars[1].progress=k+1;
  p=&fiCars[2].passage[k];q=&fiCars[1].passage[k];p->valid=q->valid=1;p->key=q->key=k;
  p->time=k*1000;q->time=p->time-k*10;
  fiCars[2].count=1;fiCars[2].events[0]=k;saved=fiBucket[2][0].display;
  FICollect(2,1,0,k*1000);
  assert(fiBucket[2][0].count==(k-30<6?k-30:6));
  if(k==31)assert(fiBucket[2][0].display==300);
  if(k>=37 && k%3)assert(fiBucket[2][0].display==saved);
 }
 FIPitSample(2,2,10,40000,0,0);FICollect(2,1,0,40000);assert(fiBucket[2][0].display==-1);
 assert(FIPosition(12,1,1,100,1000)==0);assert(FIPosition(12,2,2,200,1000)==-1);
 puts("PASS: per-car first lap, new-pair rebase, partial averages, three-point cadence, pit history and gap preservation, safe rejoin and retirement");return 0;
}
