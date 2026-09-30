#include <assert.h>
#include <stdio.h>
#include "f1battle.h"
static void tick(F1BattleState *s,unsigned long a,unsigned long b,F1BattlePair *p,int n)
{ unsigned long t;for(t=a;t<=b;t+=100) F1BattleTick(s,t,1,p,n); }
int main(void)
{
    F1BattleState s;
    F1BattlePair p[2]={{12,34,8,199,1},{22,25,2,199,1}};
    F1BattleReset(&s);
    assert(F1BattleKey(&s,3,1) && s.enabled);
    assert(!F1BattleKey(&s,3,1) && s.enabled);
    tick(&s,0,5000,p,2);assert(!s.visible);
    tick(&s,5100,5100,p,2);assert(s.visible && s.attacker==22);
    p[1].gapMs=1000;tick(&s,5200,5200,p,2);assert(s.visible);
    p[1].gapMs=1001;tick(&s,5300,5300,p,2);assert(!s.visible);
    /* Exact 200ms is outside the entry threshold. */
    p[0].gapMs=200;tick(&s,5400,11000,p,1);assert(!s.visible);
    p[0].gapMs=199;tick(&s,11100,15000,p,1);
    p[0].valid=0;tick(&s,15100,15100,p,1);p[0].valid=1;
    tick(&s,15200,20200,p,1);assert(!s.visible);
    tick(&s,20300,20300,p,1);assert(s.visible);
    /* Overtake reverses the pair; close immediately. */
    p[0].attacker=34;p[0].defender=12;
    tick(&s,20400,20400,p,1);assert(!s.visible);
    tick(&s,20500,25600,p,1);assert(s.visible);
    F1BattleKey(&s,0x83,1);assert(F1BattleKey(&s,3,1));
    assert(!s.enabled && !s.visible);
    tick(&s,25700,32000,p,1);assert(!s.visible);
    F1BattleKey(&s,0x83,1);F1BattleKey(&s,3,1);
    tick(&s,32100,37200,p,1);assert(s.visible);
    F1BattleTick(&s,38000,1,p,1);assert(!s.visible);
    tick(&s,38100,43200,p,1);assert(s.visible);
    F1BattleTick(&s,1,1,p,1);assert(!s.visible);
    tick(&s,101,5201,p,1);assert(s.visible);
    F1BattleTick(&s,5301,0,p,1);assert(!s.enabled && !s.visible);
    /* Duplicate IDs cannot supply a valid continuous observation. */
    F1BattleKey(&s,3,1);p[1]=p[0];
    tick(&s,6000,12000,p,2);assert(!s.visible);
    puts("PASS: thresholds, continuity, priority, overtake, manual OFF, repeat, time jumps, session exit, duplicates");
    return 0;
}
