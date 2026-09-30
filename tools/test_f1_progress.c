#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "../src/f1progress.h"
#include "../src/f1loops.h"
int main(int argc,char **argv)
{
 double fraction,d[4]={0,100,200,300};FILE *f;char line[2048],*p;int n=0;unsigned int raw;
 assert(F1SegmentFactor(65520,&fraction) && fraction==-16/16384.0);
 assert(fabs(F1SegmentDistance(d,3,1,fraction)-99.90234375)<0.000001);
 assert(F1SegmentFactor(16592,&fraction) && fraction>1);
 assert(F1SegmentDistance(d,3,1,fraction)>200);
 assert(!F1SegmentFactor(32768,&fraction));assert(!F1SegmentFactor(20000,&fraction));
 assert(F1SegmentFactor(65360,&fraction));assert(F1SegmentDistance(d,3,0,fraction)==0);
 assert(F1SegmentFactor(16592,&fraction));assert(F1SegmentDistance(d,3,2,fraction)<300);
 FIReset();FISample(16,1,4.529673,37455,1);FISample(16,1,4.529645,37581,1);
 assert(fiCars[16].generation==0 && fiCars[16].progress==4.529673);
 FISample(16,1,4.6,37707,1);assert(fiCars[16].generation==0);
 FISample(16,1,4.5,37833,1);assert(fiCars[16].generation==1);
 FIReset();FISample(12,1,29.99,100,1);FISample(12,1,29.999999,200,1);FISample(12,2,0.01,300,1);
 assert(fiCars[12].passage[30].valid && fiCars[12].generation==0);
 FICollect(12,13,0,300);FIPosition(12,1,2,300,1000);
 if(argc>1){f=fopen(argv[1],"r");assert(f);while(fgets(line,sizeof(line),f)){
  if(strstr(line," RAW ") && strstr(line,"micro=-1.000000") && (p=strstr(line,"factor="))!=NULL){
   raw=(unsigned int)strtoul(p+7,NULL,10);assert(F1SegmentFactor(raw,&fraction));n++;
  }
 }fclose(f);assert(n>=238);printf("PASS: %d previously rejected raw log coordinates decode within bounded overlap\n",n);}
 puts("PASS: signed overlap, finish boundaries, real backward rejection and recorded jitter regression");return 0;
}
