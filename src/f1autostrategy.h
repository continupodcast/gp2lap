#include "f1modules.h"
#include "f1csv.h"
/* T9: opt-in generated NPC plans. Player remains on optional T8. */
#include <time.h>
#include "f1strategycalc.h"
extern unsigned short *GP2_LapsInThisRace;
static AutoPlan autoCarPlan[41];
static unsigned char autoArmed[41],autoStint[41],autoStops[41],autoSet[41],autoSetSeen[41];
static int autoPlayerLogId;
static int autoBuilt,autoMode=-1,autoSignatureChecked,autoPlanFileReady,autoServiceFileReady;
static unsigned long autoClock,autoRandom;
static unsigned long *autoFuelUnit;
static unsigned char *autoQuickFlag;
static unsigned long AutoRand(void)
{
 autoRandom=(autoRandom*1664525UL+1013904223UL)&0xffffffffUL;return autoRandom;
}
#include "f1strategydistribution.h"
static void AutoPlanLog(int id,int rank,AutoPlan *p,int fuel)
{
 FILE *f;int i,sum=0,pit[3]={0,0,0};
 if(!TyreDiagnostics && !StrategyDiagnostics)return;
 for(i=0;i<p->count-1;i++){sum+=p->length[i];pit[i]=sum;}
 f=F1CsvOpen("F1PLANS.CSV",&autoPlanFileReady,"id,rank,quickrace,stints,c1,c2,c3,c4,n1,n2,n3,n4,pit1,pit2,pit3,initial_fuel_laps,estimated_seconds,grid_position,grid_valid,distribution,base_estimated_seconds,allocation_seed,is_player\n");if(!f)return;
 fprintf(f,"%d,%d,%d,%u,%u,%u,%u,%u,%u,%u,%u,%u,%d,%d,%d,%d,%.3f,%u,%u,%lu,%.3f,%lu,%d\n",id,rank,autoCalcQuick,(unsigned)p->count,
  (unsigned)p->compound[0],(unsigned)p->compound[1],(unsigned)p->compound[2],(unsigned)p->compound[3],
  (unsigned)p->length[0],(unsigned)p->length[1],(unsigned)p->length[2],(unsigned)p->length[3],pit[0],pit[1],pit[2],fuel,p->score,
  id?(unsigned)autoGrid[id]:0,id?(unsigned)autoGridValid:0,StrategyDistributionEnabled,
  id && StrategyDistributionEnabled?autoTop[autoPoolRank[id]].score:p->score,autoAllocationSeed,id && id==autoPlayerLogId);
 fclose(f);autoPlanFileReady=1;
}
static int AutoNextLap(int id)
{
 int i,sum=0;AutoPlan *p=&autoCarPlan[id];
 if(autoStint[id]>=p->count-1)return 0;
 for(i=0;i<=autoStint[id];i++)sum+=p->length[i];return sum;
}
static void AutoSetLivePit(GP2Car *c,int index,int lap)
{
 if(index==0)c->pitStop1=(unsigned char)lap;
 if(index==1)c->pitStop2=(unsigned char)lap;
 if(index==2)c->pitStop3=(unsigned char)lap;
}
static int AutoSchedule(GP2Car *c,GP2CSx *s,int id,int laps)
{
 int done=c->numPitStopsDone,remaining=autoCarPlan[id].count-1-autoStint[id],i,sum=0,k;
 if(done+remaining>3)return 0;
 c->numPitStops=(unsigned char)(done+remaining);s->num_pitstops=c->numPitStops;
 for(i=0;i<autoCarPlan[id].count;i++){
  sum+=autoCarPlan[id].length[i];
  if(i<autoStint[id] || i>=autoCarPlan[id].count-1)continue;
  k=done+i-autoStint[id];AutoSetLivePit(c,k,sum);
  s->pit_laps[k]=(unsigned char)((sum*100+laps/2)/laps);
 }
 return 1;
}
static void AutoServiceLog(GP2Car *c,int id,int action,int oldset,int set)
{
 FILE *f;if(!TyreDiagnostics && !StrategyDiagnostics)return;
 f=F1CsvOpen("F1AUTO.CSV",&autoServiceFileReady,"time_ms,id,lap,action,old_set,new_set,old_stops,new_stops,stint,compound,next_stop_lap,fuel_raw,accelerated,is_player\n");if(!f)return;
 fprintf(f,"%lu,%d,%u,%d,%d,%d,%u,%u,%u,%u,%d,%lu,%d,%d\n",*pCurTime,id,(unsigned)c->lapNr,action,oldset,set,
  (unsigned)autoStops[id],(unsigned)c->numPitStopsDone,(unsigned)autoStint[id],
  (unsigned)autoCarPlan[id].compound[autoStint[id]],AutoNextLap(id),c->fuelLoad,F1StrategyAccelerationMode(),id==autoPlayerLogId);
 fclose(f);autoServiceFileReady=1;
}
static void F1AutoStrategyTick(void)
{
#if F1_MODULE_RACE_STRATEGY_ENGINE
#include "../modules/race-strategy-engine/f1autostrategy_F1AutoStrategyTick.inc"
#else

#endif
}
