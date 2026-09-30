#include <assert.h>
#include <stdio.h>
#include "f1pits.h"
int main(void)
{
    int i;
    F1PitsReset();
    assert(F1PitsUpdate(12,0,0)==0);
    assert(F1PitsUpdate(12,1,0)==1);
    for(i=0;i<1000;i++)assert(F1PitsUpdate(12,1,0)==1);
    assert(F1PitsUpdate(34,1,0)==1);
    assert(F1PitsUpdate(12,0,0)==1);
    assert(F1PitsUpdate(12,1,1)==1);
    assert(F1PitsUpdate(12,1,0)==2);
    assert(F1PitsUpdate(34,1,0)==1);
    F1PitsReset();assert(F1PitsUpdate(12,0,0)==0);
    assert(F1PitsUpdate(0,1,0)==0);
    puts("PASS: IN PIT edges, sustained state, re-entry, independent drivers, replay, reset");
    return 0;
}
