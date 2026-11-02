#include <assert.h>
typedef unsigned char BYTE;
typedef struct {unsigned char id,numPitStops;} GP2Car;
typedef struct {unsigned char compound;} GP2CSx;
static GP2Car cars[2],*pCarStructs=cars;static int count=2,*pNumCars=&count;
static GP2CSx setups[40],*pCarSetups=setups;
#include "../../src/f1compoundstate.h"
int main(void){int map[4]={0,1,2,0};cars[0].id=3;cars[1].id=128|34;
 cars[1].numPitStops=2;assert(F1EffectivePlayerCompound(map)==2 && setups[33].compound==2);
 count=29;assert(F1EffectivePlayerCompound(map)==-1);count=2;
 cars[1].numPitStops=4;assert(F1EffectivePlayerCompound(map)==-1);
 assert(F1EffectivePlayerCompound(0)==-1);return 0;}
