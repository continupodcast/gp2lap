unsigned long StrategyRequireCompoundChange=1;
#include <assert.h>
#include <stdio.h>
#include <string.h>
typedef struct {unsigned char compound,num_pitstops,pit_laps[3];} GP2CSx;
typedef struct {unsigned char id,racePos,lapNr,numPitStopsDone,flags_90,numPitStops,pitStop1,pitStop2,pitStop3;unsigned speed,fuelLoadLaps;unsigned long fuelLoad;} GP2Car;
static unsigned long StrategyDistributionEnabled=0,StrategyDiagnostics=0;
static unsigned long StrategyAutoEnabled=0,StrategyAutoSeed=12345;
static unsigned char quickFlag=0,quickOp[7]={198,5,1,0,0,0,128};
static unsigned long StrategyRepairTestEnabled=0,TyreDiagnostics=0;
static unsigned char setTable[40],*tyreSetIndex=setTable;
static int tyreTablesReady=1;
static void F1TyreTablesInit(void){}
static unsigned long StrategyTestEnabled=0,PerCarTyreCompounds=1,TyreTestFuelLaps=0,TyreTestCompound=0;
static unsigned long clockValue=41,start=0,unit=1746944,*pCurTime=&clockValue,*pSesStartTime=&start;
static unsigned char mode=128,replay=0,acc=0,*pSessionMode=&mode,*pIsReplay=&replay,*pIsAccTime=&acc;
static unsigned short laps=53,*GP2_LapsInThisRace=&laps;static int count=2,*pNumCars=&count;
static GP2Car cars[2],*pCarStructs=cars;static GP2CSx setups[40],player,*pCarSetups=setups;static void *pPlayerSetup=&player;
static unsigned char a[6]={247,37,216,87,1,0},b[6]={247,53,216,87,1,0};
static unsigned char *IDAtoFlat(unsigned long x){return x==0x6AE32UL?quickOp:x==0x2C4EFUL?a:b;}
static void *IDACodeReftoDataRef(unsigned long x){if(x==0x6AE34UL)return &quickFlag;assert(x==0x2C4F1UL);return &unit;}
#include "../../src/f1simulationmode.h"
#include "../../src/f1strategytest.h"
#include "../../src/f1autostrategy.h"
#define __near
#define _cdecl
void StrategyServiceHook(void){}
static unsigned long StrategyPlayerEnabled=0,StrategyPlayerPitChoice=0;
#include "../../src/f1playerstrategy.h"
#include "../../src/f1strategyservice.h"

static int chooseCalls,chooseResult=1,chooseRank=0;
int F1PlayerPitChoose(int id,int recommended,int required,int initial,int *choice){(void)id;(void)recommended;(void)required;(void)initial;*choice=-1;return 1;}
int F1PlayerStrategyChoose(const AutoPlan *p,int n,int id,int distance,int *choice)
{chooseCalls++;assert(n>0 && id==34 && distance==laps);*choice=chooseRank;return chooseResult;}
static void reset(void)
{
 memset(cars,0,sizeof(cars));memset(setups,0,sizeof(setups));memset(&player,0,sizeof(player));
 cars[0].id=33;cars[1].id=128|34;cars[1].racePos=2;
 count=2;clockValue=41;start=0;mode=128;replay=0;quickFlag=0;laps=53;
 playerMenuDone=0;playerArmedId=0;playerMode=-1;autoBuilt=0;autoTopCount=0;
 StrategyAutoEnabled=0;StrategyTestEnabled=0;StrategyPlayerEnabled=1;
 TyreDiagnostics=0;StrategyDiagnostics=1;chooseCalls=0;chooseResult=1;chooseRank=0;
 memset(autoSetSeen,0,sizeof(autoSetSeen));
}
int main(void){int k,step;GP2Car saved,npc;GP2CSx original;int next;AutoPlan selected;
 reset();StrategyPlayerEnabled=0;saved=cars[1];F1PlayerStrategyTick();assert(!chooseCalls && !memcmp(&saved,&cars[1],sizeof(saved)));
 reset();replay=1;F1PlayerStrategyTick();assert(!chooseCalls);
 reset();start=17054;F1PlayerStrategyTick();assert(!chooseCalls && !playerArmedId);
 reset();chooseRank=-1;saved=cars[1];original=player;F1PlayerStrategyTick();assert(chooseCalls==1 && !playerArmedId && !memcmp(&saved,&cars[1],sizeof(saved)) && !memcmp(&original,&player,sizeof(player)));
 reset();npc=cars[0];F1PlayerStrategyTick();assert(chooseCalls==1 && playerArmedId==34 && !memcmp(&npc,&cars[0],sizeof(npc)));
 selected=autoCarPlan[34];assert(selected.count==2 && selected.compound[0]!=selected.compound[1]);
 assert(player.compound==selected.compound[0] && player.num_pitstops==1);
 assert(cars[1].fuelLoad>=unit*(selected.length[0]+1));next=AutoNextLap(34);
 F1PlayerStrategyTick();assert(chooseCalls==1 && !memcmp(&selected,&autoCarPlan[34],sizeof(selected)));
 start=17054;clockValue=100000;acc=1;cars[1].lapNr=(unsigned char)(next+1);cars[1].numPitStopsDone=1;cars[1].fuelLoad=123456;
 F1StrategyServiceAfter();assert(autoStint[34]==1 && player.compound==selected.compound[1]);assert(cars[1].fuelLoad==123456 && player.num_pitstops==1);
 F1StrategyServiceAfter();assert(autoStint[34]==1);
 reset();quickFlag=1;laps=12;F1PlayerStrategyTick();assert(playerArmedId==34 && autoCarPlan[34].count==1 && cars[1].numPitStops==0 && !AutoNextLap(34));
 for(k=0;k<=8;k+=4){
  reset();chooseRank=k;F1PlayerStrategyTick();selected=autoCarPlan[34];assert(selected.count==2+k/4);
  start=17054;acc=1;
  for(step=1;step<selected.count;step++){
   clockValue+=100000;cars[1].lapNr=(unsigned char)(AutoNextLap(34)+1);cars[1].numPitStopsDone=(unsigned char)step;cars[1].fuelLoad=7654321;
   F1StrategyServiceAfter();assert(autoStint[34]==step && player.compound==selected.compound[step] && cars[1].fuelLoad==7654321);
  }
  assert(AutoNextLap(34)==0);F1PlayerStrategyReset();assert(!playerArmedId);
 }
 reset();chooseResult=0;saved=cars[1];F1PlayerStrategyTick();assert(!playerArmedId && !memcmp(&saved,&cars[1],sizeof(saved)));
 reset();cars[0].id|=128;F1PlayerStrategyTick();assert(!chooseCalls && !playerArmedId);
 reset();StrategyTestEnabled=1;F1PlayerStrategyTick();assert(!chooseCalls);
 reset();chooseRank=-2;memset(&playerEditedPlan,0,sizeof(playerEditedPlan));
 playerEditedPlan.count=3;playerEditedPlan.length[0]=12;playerEditedPlan.length[1]=18;playerEditedPlan.length[2]=23;
 playerEditedPlan.compound[0]=2;playerEditedPlan.compound[1]=1;playerEditedPlan.compound[2]=0;
 npc=cars[0];F1PlayerStrategyTick();
 assert(playerArmedId==34 && !memcmp(&autoCarPlan[34],&playerEditedPlan,sizeof(AutoPlan)));
 assert(cars[1].pitStop1==12 && cars[1].pitStop2==30 && cars[1].numPitStops==2);
 assert(player.compound==2 && player.num_pitstops==2 && !memcmp(&npc,&cars[0],sizeof(npc)));
 assert(cars[1].fuelLoadLaps==(unsigned)(14*64));
 puts("PASS T20 custom draft applies pit laps, compound and initial fuel, NPC untouched");
 remove("F1PLANS.CSV");remove("F1AUTO.CSV");puts("PASS T14 player selection, native cancel no-op, initial fuel, compound transition without frames, no refill, no duplicate service, Quickrace zero-stop, opt-out/replay/mid-race/legacy conflict guards, safe multi-player fallback");return 0;}
