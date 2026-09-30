#include <string.h>
#include "f1micro.h"
void F1MicroReset(F1MicroState *s) { memset(s,0,sizeof(*s)); }
void F1MicroSample(F1MicroState *s,int id,int lap,double position,unsigned long clock,int valid)
{
    F1MicroCar *c;double previous,current,t;int k,first,last,index,other;long elapsed;
    if(id<1 || id>40) return;
    c=&s->cars[id];
    /* A missing sample cancels only the current interval, not completed
       results. Keep the last lap number to clear stale colours on re-entry. */
    if(!valid || position<0 || position>=30) { c->seen=0;c->haveCross=0;return; }
    if(lap<c->lap || clock<c->clock || lap>c->lap+1 || (!c->seen && lap!=c->lap)) {
        memset(c->colors,0,30);c->newLap=0;
    }
    previous=c->position;current=position;
    if(c->seen && lap==c->lap+1 && position<previous) current+=30;
    if(!c->seen || clock<c->clock || clock-c->clock>500 || lap<c->lap || lap>c->lap+1 ||
       current<previous || current-previous>3 || (lap!=c->lap && current<30)) {
        c->haveCross=0;
    } else if(current>previous && clock>c->clock) {
        first=(int)previous+1;last=(int)current;
        for(k=first;k<=last;k++) {
            t=c->clock+(clock-c->clock)*(k-previous)/(current-previous);
            index=(k-1)%30;
            if(index==0 && c->newLap) {memset(c->colors,0,30);c->newLap=0;}
            if(c->haveCross) {
                elapsed=(long)(t-c->cross+0.5);
                if(elapsed>0) {
                    /* Purple denotes the current session record, not a
                       historical record at the instant this car crossed.
                       Downgrade only displayed purple marks for this split;
                       timings and all other colours remain untouched. */
                    if(s->best[index] && elapsed<s->best[index]) {
                        for(other=1;other<=40;other++)
                            if(s->cars[other].colors[index]==3)
                                s->cars[other].colors[index]=1;
                    }
                    c->colors[index]=!s->best[index] || elapsed<=s->best[index]?3:
                        !c->personal[index] || elapsed<c->personal[index]?1:2;
                    if(!s->best[index] || elapsed<s->best[index]) s->best[index]=elapsed;
                    if(!c->personal[index] || elapsed<c->personal[index]) c->personal[index]=elapsed;
                }
            }
            c->cross=t;c->haveCross=1;
            /* Clear the next lap only when its first microsector completes. */
            if(index==29) c->newLap=1;
        }
    }
    c->seen=1;c->lap=lap;c->position=position;c->clock=clock;
}
