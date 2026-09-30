#include <assert.h>
#include <stdio.h>
#include "../src/f1interval.h"
int main(void)
{
    unsigned long t;long held;int i;FIBucket b;
    FIReset();
    for(t=0;t<=12000;t+=100) {
        FISample(1,1,(long)t,t,1);
        FISample(2,1,(long)t-1200,t,1);
        FICollect(2,1,0,t);FICollect(2,1,1,t);
        FIPublish(t,t>=6000,4000);
        if(t<8000)assert(fiBucket[2][0].display==-1);
        else assert(fiBucket[2][0].display==1200);
        assert(fiBucket[2][0].display==fiBucket[2][1].display);
    }
    held=fiBucket[2][0].display;
    for(t=12100;t<16000;t+=100) {
        FISample(1,1,(long)t,t,1);FISample(2,1,(long)t-2200,t,1);
        FICollect(2,1,0,t);FIPublish(t,1,4000);
        /* The backwards jump intentionally invalidates the old history. */
        assert(fiBucket[2][0].display==-1);
    }
    FIPublish(16000,1,4000);assert(fiBucket[2][0].display==2200);assert(held==1200);
    FISample(2,1,14000,16100,0);FICollect(2,1,0,16100);
    assert(fiBucket[2][0].display==-1);
    /* Robust average rejects isolated corrupt measurements, not real trends. */
    memset(&b,0,sizeof(b));b.count=40;
    for(i=0;i<40;i++)b.values[i]=1200+(i%3-1)*50;
    b.values[5]=6000;assert(FIAverage(&b)==1200);
    for(i=0;i<40;i++)b.values[i]=1000+i*20;
    assert(FIAverage(&b)==1400);
    assert(FIPosition(12,4,1000,1000)==0);
    assert(FIPosition(12,3,1100,1000)==1);
    assert(FIPosition(12,3,2099,1000)==1);
    assert(FIPosition(12,3,2100,1000)==0);
    assert(FIPosition(12,4,2200,1000)==-1);
    assert(FIPosition(12,2,2300,1000)==1);
    FICollect(2,3,0,17000);assert(fiBucket[2][0].display==-1);
    FIReset();assert(FIPosition(12,2,20000,1000)==0);
    /* Normal lap transitions preserve cumulative passage history. */
    for(t=0;t<=6000;t+=100) {
        FISample(1,t<3000?1:2,(long)t,t,1);
        FISample(2,t<4200?1:2,(long)t-1200,t,1);
    }
    assert(FIRaw(2,1,6000)==1200);
    FISample(1,2,9000,9000,1);assert(fiHistory[1].count==1);
    assert(FIRaw(2,1,9000)==-1);
    FIReset();
    for(t=0;t<=12000;t+=100) {
        FISample(1,1,(long)t,t,1);
        FISample(2,1,(long)t-(1000+(long)t/10),t,1);
        FICollect(2,1,0,t);FICollect(2,1,1,t);
        held=fiBucket[2][0].display;FIPublish(t,1,4000);
        if(t%4000)assert(fiBucket[2][0].display==held);
        assert(fiBucket[2][0].display==fiBucket[2][1].display);
    }
    assert(fiBucket[2][0].display==2000);
    puts("PASS: interpolation, publication, half-lap gating, robustness, pits, opponents, laps, discontinuity and triangles");
    return 0;
}
