#ifndef F1RACE_H
#define F1RACE_H
#include "f1render.h"
void F1RaceReset(int fresh);
void F1RaceStats(F1Row *rows,int count);
void F1RaceKey(unsigned int scan,int enabled);
int F1RaceMode(void);
#endif
