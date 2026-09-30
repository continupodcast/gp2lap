#include <assert.h>
#include <stdio.h>
#include "f1pits.h"
#define U(p,n,t) F1PitsUpdate(12,p,n,t,0)
int main(void)
{
    int i;
    F1PitsReset();assert(U(0,0,0)==0);
    assert(U(1,0,100)==1);
    for(i=200;i<=1000;i+=100)assert(U(1,0,i)==1);
    assert(U(1,1,1100)==1);assert(U(0,1,1200)==1);
    /* Repair followed by scheduled stop: retain the extra repair count. */
    assert(U(1,1,10000)==2);assert(U(0,1,10100)==2);
    assert(U(0,1,20000)==2);
    assert(U(1,1,30000)==3);assert(U(1,2,30100)==3);
    assert(U(0,2,30200)==3);
    /* Several entirely unseen accelerated stops. */
    assert(U(0,5,300000)==6);
    assert(U(0,5,310000)==6);
    assert(U(1,5,310100)==7);assert(U(0,5,310200)==7);
    assert(U(0,6,310300)==7); /* native increment after exit */
    assert(F1PitsUpdate(34,0,3,310300,0)==3);
    assert(F1PitsUpdate(12,1,9,1,1)==7); /* replay */
    F1PitsReset();assert(U(0,0,0)==0);
    assert(U(1,1,100)==1); /* simultaneous */
    assert(U(0,1,200)==1);
    assert(U(0,2,10000)==2);assert(U(1,2,10100)==2); /* native first */
    assert(U(0,2,10200)==2);
    assert(U(1,2,20000)==3);
    assert(U(0,5,300000)==5); /* observed stop + 2 skipped stops */
    assert(U(0,0,0)==0); /* restart */
    puts("PASS: live, native-only, repair + scheduled, multiple skipped stops, both event orders, late increment, replay, reset");
    return 0;
}
