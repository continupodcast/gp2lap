#include <string.h>
#include "f1battle.h"

void F1BattleReset(F1BattleState *s) { memset(s,0,sizeof(*s)); }
static void clearTracking(F1BattleState *s)
{
    memset(s->tracking,0,sizeof(s->tracking));
    memset(s->opponent,0,sizeof(s->opponent));
}
static void closeBattle(F1BattleState *s)
{
    s->visible=0;s->attacker=0;s->defender=0;s->position=0;
}
int F1BattleKey(F1BattleState *s,unsigned int scan,int racing)
{
    if(scan==0x83) { s->held=0;return 0; }
    if(scan!=3 || s->held) return 0;
    s->held=1;
    if(!racing) return 0;
    s->enabled=!s->enabled;
    closeBattle(s);clearTracking(s);s->haveClock=0;
    return 1;
}
void F1BattleTick(F1BattleState *s,unsigned long now,int racing,
                  const F1BattlePair *pairs,int count)
{
    int i,id,best=-1,keep=0,closed=0;
    int occurrences[41],observed[41];
    const F1BattlePair *p;
    if(!racing) { F1BattleReset(s);return; }
    if(!s->enabled) return;
    /* No interpolation of a battle across an unobserved time jump. */
    if(s->haveClock && (now<s->clock || now-s->clock>500UL)) {
        closeBattle(s);clearTracking(s);
    }
    s->clock=now;s->haveClock=1;
    if(!pairs || count<0 || count>26) count=0;
    memset(occurrences,0,sizeof(occurrences));
    memset(observed,0,sizeof(observed));
    for(i=0;i<count;i++) {
        id=pairs[i].attacker;
        if(id>0 && id<=40) occurrences[id]++;
    }
    for(i=0;i<count;i++) {
        p=&pairs[i];id=p->attacker;
        if(id<1 || id>40 || occurrences[id]!=1 || !p->valid ||
           p->defender<1 || p->defender>40 || p->defender==id ||
           p->position<1 || p->position>25 || p->gapMs<0) continue;
        if(s->visible && id==s->attacker && p->defender==s->defender &&
           p->gapMs<=1000) {keep=1;s->position=p->position;}
        if(p->gapMs>=200) continue;
        observed[id]=1;
        if(!s->tracking[id] || s->opponent[id]!=p->defender) {
            s->tracking[id]=1;s->opponent[id]=p->defender;s->since[id]=now;
        }
        if(now-s->since[id]>5000UL &&
           (best<0 || p->position<pairs[best].position)) best=i;
    }
    for(id=1;id<=40;id++) if(!observed[id]) s->tracking[id]=0;
    if(s->visible && !keep) {
        closeBattle(s);clearTracking(s);closed=1;
    }
    /* Keep an active battle until its own exit condition; do not flicker
       between qualifying pairs, or reopen in the same tick as an overtake. */
    if(!s->visible && !closed && best>=0) {
        p=&pairs[best];s->visible=1;s->attacker=p->attacker;
        s->defender=p->defender;s->position=p->position;
    }
}
