#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "../src/f1gapdiag.h"
#include "../src/f1loops.h"
int main(void)
{
 FILE *f;char buffer[20000];size_t n;
 GapDiagBoot();FIReset();
 FISample(12,1,29.9,100,1);FISample(12,2,0.1,300,1);
 assert(fiCars[12].passage[30].valid);
 /* Lap counter late: wrap first, lap changes a sample later. */
 FIReset();FISample(12,1,29.9,100,1);FISample(12,1,0.1,300,1);FISample(12,2,0.2,400,1);
 assert(!fiCars[12].passage[30].valid);
 /* Lap counter early: lap changes before position wraps. */
 FIReset();FISample(12,1,29.8,100,1);FISample(12,2,29.9,200,1);FISample(12,2,0.1,300,1);
 FISample(12,2,0.2,1000,1);FISample(12,2,-1,1100,1);
 FICollect(12,34,0,1100);FIPosition(12,2,2,1100,1000);
 GapDiagFlush(1100);f=fopen("F1GAPS.LOG","rb");assert(f);
 n=fread(buffer,1,sizeof(buffer)-1,f);buffer[n]=0;fclose(f);
 assert(strstr(buffer,"FINISH_TRANSITION"));assert(strstr(buffer,"key=30 point=0"));
 assert(strstr(buffer,"backwards=1"));assert(strstr(buffer,"jump=1"));
 assert(strstr(buffer,"timeout=1"));assert(strstr(buffer,"SAMPLE_INVALID"));assert(strstr(buffer,"REFERENCE"));
 puts("PASS: diagnostic records normal finish, late/early lap counter, backward jump, timeout and invalid sample without changing behavior");return 0;
}
