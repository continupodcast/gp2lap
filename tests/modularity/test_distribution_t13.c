unsigned long StrategyRequireCompoundChange=1;
#include <assert.h>
#include <math.h>
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
static unsigned short laps=53,*GP2_LapsInThisRace=&laps;static int count=26,*pNumCars=&count;
static GP2Car cars[26],*pCarStructs=cars;static GP2CSx setups[40],player,*pCarSetups=setups;static void *pPlayerSetup=&player;
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
void F1PlayerStrategyService(void){}
#include "../../src/f1strategyservice.h"

static void fresh(int total,int playerCar,int badGrid)
{
 int i;count=total;clockValue=41;start=0;acc=0;replay=0;quickFlag=0;
 StrategyAutoEnabled=1;StrategyDistributionEnabled=1;StrategyDiagnostics=1;TyreDiagnostics=0;StrategyTestEnabled=0;
 autoBuilt=0;autoMode=-1;memset(autoArmed,0,sizeof(autoArmed));memset(autoSetSeen,0,sizeof(autoSetSeen));
 memset(cars,0,sizeof(cars));memset(setups,0,sizeof(setups));
 for(i=0;i<total;i++){cars[i].id=(unsigned char)(i+1);cars[i].racePos=(unsigned char)(badGrid?0:i*2);}
 if(playerCar)cars[total-1].id|=128;
}
int main(void)
{
 int i,j,sum,mask,groups[5]={0},pool[5]={0},unique=0;double cost;AutoPlan saved[41];GP2Car playerBefore;
 fresh(26,0,0);F1AutoStrategyTick();assert(autoGridValid);
 for(i=0;i<autoTopCount;i++)pool[autoTop[i].count]++;
 assert(pool[2]==4 && pool[3]==4 && pool[4]==2);
 for(i=0;i<26;i++){
  int id=i+1;AutoPlan *p=&autoCarPlan[id];assert(autoArmed[id] && autoGrid[id]==id);
  groups[p->count]++;sum=mask=0;
  for(j=0;j<p->count;j++){sum+=p->length[j];mask|=1<<p->compound[j];assert(p->length[j]>=3 && p->length[j]<=90);}
  assert(sum==53 && (mask&(mask-1)));assert(cars[i].numPitStops==p->count-1);
  assert(cars[i].fuelLoad>=unit*(p->length[0]+1));
  cost=AutoPlanCost(p);assert(cost<=autoTop[autoPoolRank[id]].score+2.000001);
  assert(fabs(cost-p->score)<1e-7);
  if(i==0 || !AutoSamePlan(p,&autoCarPlan[1]))unique++;
 }
 assert(groups[2]>0 && groups[3]>0 && groups[4]>0 && unique>1);
 printf("53 laps: NPC families one=%d two=%d three=%d; pool=4/4/2\n",groups[2],groups[3],groups[4]);
 memcpy(saved,autoCarPlan,sizeof(saved));F1AutoStrategyTick();assert(!memcmp(saved,autoCarPlan,sizeof(saved)));
 fresh(26,0,0);/* reverse physics array, preserving each actual grid slot */
 for(i=0;i<13;i++){GP2Car tmp=cars[i];cars[i]=cars[25-i];cars[25-i]=tmp;}
 F1AutoStrategyTick();assert(!memcmp(saved,autoCarPlan,sizeof(saved)));
 fresh(26,1,0);playerBefore=cars[25];F1AutoStrategyTick();assert(!autoArmed[26]);assert(!memcmp(&playerBefore,&cars[25],sizeof(playerBefore)));
 fresh(26,0,1);F1AutoStrategyTick();assert(!autoGridValid);for(i=1;i<=26;i++)assert(autoGrid[i]==0 && autoArmed[i]);
 fresh(26,0,0);StrategyDistributionEnabled=0;F1AutoStrategyTick();for(i=1;i<=26;i++)assert(autoCarPlan[i].count>=2);
 fresh(26,0,0);StrategyAutoEnabled=0;playerBefore=cars[0];F1AutoStrategyTick();assert(!memcmp(&playerBefore,&cars[0],sizeof(playerBefore)));
 fresh(26,0,0);quickFlag=1;laps=12;F1AutoStrategyTick();sum=0;for(i=1;i<=26;i++)if(autoCarPlan[i].count==1)sum++;assert(sum>0);
 fresh(26,0,0);laps=255;F1AutoStrategyTick();for(i=1;i<=26;i++)assert(autoCarPlan[i].count>=3);
 remove("F1PLANS.CSV");remove("F1AUTO.CSV");puts("PASS T13 family presence, fuel, mandatory compound change, recomputed scores, reproducible seed, physics order independence, real grid, neutral invalid grid, player excluded, opt-out, Quickrace zero-stop, infeasible family fallback");return 0;
}
