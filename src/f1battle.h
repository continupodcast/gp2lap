#ifndef F1BATTLE_H
#define F1BATTLE_H
/* Experimental controller only: not yet connected to GP2's renderer. */
typedef struct {
    int attacker, defender, position;
    long gapMs;
    int valid;
} F1BattlePair;
typedef struct {
    int enabled, held, visible, attacker, defender, position;
    int haveClock;
    unsigned long clock;
    unsigned long since[41];
    int opponent[41], tracking[41];
} F1BattleState;
void F1BattleReset(F1BattleState *s);
/* Set-1 scan code 3 = number 2; returns 1 only on an actual toggle. */
int F1BattleKey(F1BattleState *s, unsigned int scan, int racing);
/* Complete snapshot of adjacent, same-lap racing pairs. Simulation ms.
   Unavailable/invalid observations break continuity. No image is rendered. */
void F1BattleTick(F1BattleState *s, unsigned long now, int racing,
                  const F1BattlePair *pairs, int count);
#endif
