#ifndef F1PITDISPLAY_H
#define F1PITDISPLAY_H
/* Per-CarId timing; independent of tower visibility and gap references. */
typedef struct { int active; unsigned long start,elapsed,exitAt; } F1PitClock;
static F1PitClock pitClocks[41];
static void F1PitDisplayReset(void) { memset(pitClocks,0,sizeof(pitClocks)); }
static void F1PitDisplayTick(int id,int pit,unsigned long now)
{
    F1PitClock *c;
    if(id<1 || id>40)return;
    c=&pitClocks[id];
    if(!pit) {
        if(c->active==1) {c->elapsed=now>=c->start?now-c->start:0;c->exitAt=now;c->active=2;}
        else if(c->active==2 && (now<c->exitAt || now-c->exitAt>=5000UL)) {
            c->active=0;c->elapsed=0;
        }
        return;
    }
    if(c->active!=1 || now<c->start) {c->active=1;c->start=now;}
    c->elapsed=now-c->start;
}
#endif
