#include <assert.h>
#include <stdio.h>
#include "../../src/f1render.h"
#include "../../src/f1qualy.h"
unsigned long PerCarTyreCompounds=0,TyreDiagnostics=0,TyreTestCompound=0,EnhancedCameras=1;
#include "../../src/f1race.c"
int main(void){int i;int off[5]={1,2,3,0,1};
assert(F1_RACE_W==104 && F1_Q_TOTAL_W==116);
for(i=0;i<5;i++){F1RaceKey(0x0f,1);assert(F1RaceMode()==off[i]);F1RaceKey(0x8f,1);}
mode=0;PerCarTyreCompounds=1;assert(F1_RACE_W==119 && F1_Q_TOTAL_W==131);
F1RaceKey(0x0f,1);F1RaceKey(0x8f,1);F1RaceKey(0x0f,1);F1RaceKey(0x8f,1);F1RaceKey(0x0f,1);assert(F1RaceMode()==4);
puts("PASS tyre opt-in: native table widths and no TYRE AGE when off; columns and age enabled when on");return 0;}
