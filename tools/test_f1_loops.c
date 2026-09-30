#include <assert.h>
#include <stdio.h>
#include "../src/f1loops.h"
static void sample(int id,double progress,unsigned long t)
{
 int lap=(int)(progress/30);FISample(id,lap+1,progress-lap*30,t,1);
}
int main(void)
{
 unsigned long t;long old2,old3;int changed2=0,changed3=0,separate=0,k,j;FIBucket *b;
 FIReset();
 for(t=0;t<=80000;t+=100) {
  sample(1,t/2000.0,t);sample(2,(t-0.0)/2000.0-0.65,t);sample(3,t/2000.0-1.85,t);
  old2=fiBucket[2][0].display;old3=fiBucket[3][0].display;
  FICollect(2,1,0,t);FICollect(3,2,0,t);FICollect(3,1,1,t);
  if(t<61300)assert(fiBucket[2][0].display==-1);
  if(fiBucket[2][0].display!=old2){changed2++;if(fiBucket[3][0].display==old3)separate++;}
  if(fiBucket[3][0].display!=old3)changed3++;
  if(fiBucket[2][0].display>=0)assert(fiBucket[2][0].display==1300);
  if(fiBucket[3][0].display>=0)assert(fiBucket[3][0].display==2400);
  if(fiBucket[3][1].display>=0)assert(fiBucket[3][1].display==3700);
 }
 assert(changed2 && changed3 && separate);
 /* Exact sliding windows at point 12 (7..12) and 15 (10..15). */
 FIReset();fiCars[2].lap=2;
 for(k=1;k<=15;k++) {
  fiCars[1].seen=fiCars[2].seen=1;fiCars[1].clock=fiCars[2].clock=k*2000;
  fiCars[1].passage[k].key=fiCars[2].passage[k].key=k;
  fiCars[1].passage[k].valid=fiCars[2].passage[k].valid=1;
  fiCars[1].passage[k].time=k*2000-k*100;fiCars[2].passage[k].time=k*2000;
  fiCars[2].count=1;fiCars[2].events[0]=k;old2=fiBucket[2][0].display;
  FICollect(2,1,0,k*2000);b=&fiBucket[2][0];
  if(k>6 && k%3)assert(b->display==old2);
  if(k==12)assert(b->display==1000);
  if(k==15)assert(b->display==1300);
 }
 FICollect(2,3,0,30000);assert(fiBucket[2][0].display==-1);
 FISample(1,1,16,30100,0);FICollect(2,1,0,30100);assert(fiBucket[2][0].display==-1);
 FIReset();sample(1,29.9,100);sample(1,30.1,300);
 assert(fiCars[1].passage[30].valid && fiCars[1].passage[30].time==200);
 sample(1,34,10000);assert(!fiCars[1].passage[30].valid);
 for(j=1;j<=5;j++)assert(FIPosition(12,j,1,j*100,1000)==0);
 assert(FIPosition(12,4,2,1000,1000)==1);
 assert(FIPosition(12,4,2,2000,1000)==0);
 assert(FIPosition(12,5,2,2100,1000)==-1);
 puts("PASS: independent crossings, 6-point means every 3, partial warmup, leader gaps, first lap, lap wrap, discontinuities and triangles");
 return 0;
}
