unsigned long StrategyRequireCompoundChange=1;
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#define _VESA_H
static unsigned char screenMemory[307200],*pLFBuf=screenMemory;
static int svga=1,*pUseSVGA=&svga,number=1,*pNumCars=&number;
static unsigned char quick,*autoQuickFlag=&quick;
typedef struct {unsigned char id,numPitStops,pitStop1,pitStop2,pitStop3;unsigned fuelLoadLaps;} GP2Car;
typedef struct {unsigned char compound;} GP2CSx;
static GP2Car car,*pCarStructs=&car;static GP2CSx setup[40],*pCarSetups=setup;
typedef struct {double score;unsigned char count,compound[4],length[4];} AutoPlan;
static AutoPlan playerEditedPlan;
static unsigned int inp(unsigned int p){(void)p;return 0;}
static void outp(unsigned int p,unsigned int v){(void)p;(void)v;}
static void delay(unsigned int ms){(void)ms;assert(0 && "script exhausted");}
static unsigned int GetSvgaGranularity(void){return 64;}
static void F1PlayerCircuitPrepare(void){}
#include "../../src/f1playermodal.h"
unsigned int F1StrategyBankGet(void){return 0;}
unsigned int F1StrategyBankSet(unsigned int b){(void)b;return 1;}
static const int *keys;static int at,painted,errorSeen,useSpace;
int KeyEventDequeue(KeyEvent *e){if(!F1StrategyMenuActive)return 0;assert(keys[at]>=0);e->scancode=keys[at++];if(useSpace && e->scancode==0x1c)e->scancode=0x39;else if(useSpace && e->scancode==0x9c)e->scancode=0xb9;return 1;}
void F1RenderStrategyEditor(unsigned char *s,const unsigned char *p,const F1StrategyOption *o,int f,int id,int laps,int preset,int n,int edited,int quick,const char *error)
{(void)p;(void)f;(void)id;(void)laps;(void)preset;(void)n;(void)edited;(void)quick;memset(s,8,307200);if(!painted){assert(preset==0 && o->fuel==26 && o->count==2 && o->compound[0]==0 && o->compound[1]==0);}
 assert(o->count>=1 && o->count<=4);painted++;if(*error)errorSeen++;}
void F1RenderPlayerStrategy(unsigned char *s,const unsigned char *p,const F1StrategyOption *o,int n,int sel,int id,int laps,const F1StrategyOption *orig)
{(void)s;(void)p;(void)o;(void)n;(void)sel;(void)id;(void)laps;(void)orig;}
void F1RenderPlayerPitStrategy(unsigned char *s,const unsigned char *p,const F1StrategyOption *o,int n,int sel,int id,int rec,int req)
{(void)s;(void)p;(void)o;(void)n;(void)sel;(void)id;(void)rec;(void)req;}
int main(void){AutoPlan p,b;GP2Car before;int choice,i;
 static const int native[]={0x50,0x50,0x50,0x50,0x4d,0x1c,0x9c,-1};
 static const int edit[]={0x4d,0x1c,0x9c,0x50,0x50,0x1c,0x9c,0x1c,0x9c,0x4d,0x4d,0x1c,0x9c,0x50,0x50,0x1c,0x9c,-1};
 static const int invalid[]={0x4d,0x1c,0x9c,0x50,0x50,0x1c,0x9c,0x50,0x50,0x1c,0x9c,1,0x81,-1};
 static const int restore[]={0x4d,0x1c,0x9c,0x50,0x50,0x1c,0x9c,0x50,0x50,0x4b,0x1c,0x9c,0x4d,0x1c,0x9c,-1};
 static const int zero[]={0x4d,0x1c,0x9c,0x50,0x4b,0x1c,0x9c,0x50,0x50,0x4d,0x1c,0x9c,-1};
 memset(&p,0,sizeof(p));p.count=2;p.length[0]=20;p.length[1]=33;p.compound[1]=1;b=p;
 memset(&car,0,sizeof(car));car.id=129;car.numPitStops=1;car.pitStop1=20;car.fuelLoadLaps=26*64;before=car;memset(screenMemory,77,sizeof(screenMemory));
 keys=native;at=0;assert(F1PlayerStrategyChoose(&p,1,1,53,&choice) && choice==-1);
 keys=edit;at=0;assert(F1PlayerStrategyChoose(&p,1,1,53,&choice) && choice==-2);
 assert(playerEditedPlan.compound[0]==2 && playerEditedPlan.length[0]==21 && playerEditedPlan.length[1]==32);
 assert(!memcmp(&p,&b,sizeof(p)) && !memcmp(&car,&before,sizeof(car)));
 for(i=0;i<307200;i++)assert(screenMemory[i]==77);
 keys=invalid;at=0;assert(F1PlayerStrategyChoose(&p,1,1,53,&choice) && choice==-1 && errorSeen);
 keys=restore;at=0;assert(F1PlayerStrategyChoose(&p,1,1,53,&choice) && choice==-1);
 quick=1;keys=zero;at=0;assert(F1PlayerStrategyChoose(&p,1,1,53,&choice) && choice==-2);
 assert(playerEditedPlan.count==1 && playerEditedPlan.length[0]==53);
 useSpace=1;quick=0;keys=edit;at=0;assert(F1PlayerStrategyChoose(&p,1,1,53,&choice) && choice==-2);
 assert(playerEditedPlan.compound[0]==2 && playerEditedPlan.length[0]==21);
 keys=native;at=0;assert(F1PlayerStrategyChoose(&p,1,1,53,&choice) && choice==-1);
 assert(!F1StrategyMenuActive && painted>0);puts("PASS T20.4 spatial arrows, TYRE to PIT AT LAP and vertical row moves; T20.3 separate minus/plus activation and press/release; T20.2 native default, actual native fuel, native restore and modal key flow: edit, confirm on key release, invalid compound rejection, Escape, restore default, no car/pool writes, framebuffer restoration");return 0;}
