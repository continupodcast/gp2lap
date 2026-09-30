#include <assert.h>
#include <stdio.h>
#include "../src/f1loops.h"
static void seed(int n)
{
 int k;FIPassage *a,*b;
 FIReset();
 fiCars[1].seen=fiCars[2].seen=1;fiCars[1].lap=fiCars[2].lap=21;
 fiCars[1].clock=fiCars[2].clock=1853411;
 fiCars[1].progress=fiCars[2].progress=626.864833;
 for(k=627-n;k<=626;k++) {
  a=&fiCars[1].passage[k%FI_HISTORY];b=&fiCars[2].passage[k%FI_HISTORY];
  a->valid=b->valid=1;a->key=b->key=k;
  a->time=1850000-(626-k)*1000;b->time=a->time+1234;
 }
 FICollect(2,1,0,1853411);FICollect(2,1,1,1853411);
}
int main(void)
{
 int n,k;unsigned long t;long saved;
 for(n=1;n<=6;n++) {
  seed(n);assert(fiBucket[2][0].count==n);
  assert(fiBucket[2][0].precise==1234);assert(fiBucket[2][0].display==1200);
  /* Same invalid-geometry transition and delayed flag as the supplied log. */
  for(t=1853537;t<1862618;t+=126) {
   FIPitSample(1,21,-1,t,1,0);fiCars[2].clock=t;
   FICollect(2,1,0,t);assert(fiBucket[2][0].precise==1234);
   assert(fiCars[1].generation==0);
  }
  FIPitSample(1,21,-1,1862618,1,1);fiCars[2].clock=1862618;
  FICollect(2,1,0,1862618);assert(fiBucket[2][0].precise==1234);
 }
 /* Follower crosses missing points: average shrinks 5,4,3,2,1.
    At zero common points retain the exact pair's last real measurement. */
 seed(6);FIPitSample(1,21,-1,1853537,1,0);
 for(k=627;k<=633;k++) {
  t=1853600+(k-627)*100;fiCars[1].clock=fiCars[2].clock=t;
  fiCars[2].progress=k;fiCars[2].events[0]=k;fiCars[2].count=1;
  FICollect(2,1,0,t);
  assert(fiBucket[2][0].count==(k<632?632-k:0));
  assert(fiBucket[2][0].precise==1234);
 }
 /* Recover available recent points even if the latest point is missing. */
 fiBucket[2][0].ahead=3;fiCars[2].progress=629;
 FICollect(2,1,0,t);assert(fiBucket[2][0].count==3);
 assert(fiBucket[2][0].precise==1234);
 /* Retirement cannot keep a pit-route reference alive. */
 FIPitSample(1,21,-1,t+100,0,0);FICollect(2,1,0,t+100);
 assert(fiBucket[2][0].precise==-1);
 seed(6);saved=fiBucket[2][0].precise;
 FIPitSample(1,21,-1,1853537,1,0);
 FIPitSample(1,22,1,1853663,1,0);
 assert(fiCars[1].count==0);assert(fiBucket[2][0].precise==saved);
 assert(FIPosition(1,1,1,0,1000)==0);
 puts("PASS: early route hold, delayed IN PIT, 6..1-point means, exact milliseconds, zero-point hold, pair rebase, retirement and safe rejoin");
 return 0;
}
