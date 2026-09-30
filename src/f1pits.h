/* Reconcile observed IN PIT entries with native stops, per CarId. */
#include <string.h>
typedef struct {
    int ready,total,native,inside,pendingEntry,pendingNative;
    unsigned long lastClock,exitClock,nativeClock;
} F1PitState;
static F1PitState f1pits[41];
static void F1PitsReset(void) { memset(f1pits,0,sizeof(f1pits)); }
static int F1PitsUpdate(int id,int pit,int native,unsigned long clock,int replay)
{
    F1PitState *s;int delta,sparse,matched=0;
    if(id<1 || id>40)return native;
    s=&f1pits[id];
    if(replay)return s->ready?s->total:native;
    if(!s->ready || clock<s->lastClock || native<s->native) {
        memset(s,0,sizeof(*s));s->ready=1;s->total=native;s->native=native;
        /* Native baseline on saved races; count a first live entry only
           when there are no pre-existing native stops. */
        s->inside=pit && native>0;
    }
    sparse=clock-s->lastClock>2000;
    if(s->pendingEntry && !s->inside && clock-s->exitClock>5000)
        s->pendingEntry=0;
    if(s->pendingNative && clock-s->nativeClock>2000)
        s->pendingNative=0;
    delta=native-s->native;
    if(delta>0) {
        /* At most one native stop can belong to the pending observed visit.
           Any further increments during accelerated time are new stops. */
        if(s->pendingEntry) {delta--;s->pendingEntry=0;matched=1;}
        s->total+=delta;
        if(delta>0) {s->pendingNative=1;s->nativeClock=clock;}
    }
    if(pit && !s->inside) {
        if(s->pendingNative) s->pendingNative=0;
        else {s->total++;s->pendingEntry=1;}
    } else if(pit && sparse && delta>0) {
        /* Current pit state belongs to the last native stop in a skipped
           interval. Do not leave credit to consume a future entry. */
        s->pendingNative=0;
    }
    if(s->inside && !pit) {
        s->exitClock=clock;
        /* If the entire exit was skipped, old credit cannot be timed from
           the resume instant. A contemporaneous native match is already used. */
        if(sparse && !matched)s->pendingEntry=0;
        s->pendingNative=0;
    }
    s->inside=pit!=0;s->native=native;s->lastClock=clock;
    return s->total;
}
