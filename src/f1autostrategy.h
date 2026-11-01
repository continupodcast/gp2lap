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
 int i,id,laps,n,m,rank,fuel,quick,set,changed,advanced,next,action;
 unsigned long now,unit;unsigned char *a,*b,*q;GP2Car *c;GP2CSx *s;
 if(!StrategyAutoEnabled || !PerCarTyreCompounds || TyreTestFuelLaps || TyreTestCompound)return;
 if(!pSessionMode || !pCurTime || !pSesStartTime || !pNumCars || !pCarStructs || !pCarSetups ||
  !GP2_LapsInThisRace || !pIsReplay || *pIsReplay)return;
 now=*pCurTime;m=*pSessionMode;
 if(m!=autoMode || now<autoClock){autoBuilt=0;memset(autoArmed,0,sizeof(autoArmed));memset(autoSetSeen,0,sizeof(autoSetSeen));}
 autoMode=m;autoClock=now;if(m!=0x80)return;
 n=*pNumCars;laps=*GP2_LapsInThisRace;if(n<1 || n>26 || laps<6 || laps>255)return;
 if(!autoSignatureChecked){
  autoSignatureChecked=1;a=IDAtoFlat(0x2C4EFUL);b=IDAtoFlat(0x2BEF0UL);q=IDAtoFlat(0x6AE32UL);
  if(!a || !b || memcmp(a,"\xf7\x25",2) || memcmp(b,"\xf7\x35",2) || memcmp(a+2,b+2,4) ||
   !q || q[0]!=0xC6 || q[1]!=0x05 || q[6]!=0x80)return;
  autoFuelUnit=(unsigned long *)IDACodeReftoDataRef(0x2C4F1UL);
  autoQuickFlag=(unsigned char *)IDACodeReftoDataRef(0x6AE34UL);
 }
 if(!autoFuelUnit || !autoQuickFlag)return;unit=*autoFuelUnit;
 if(!unit || unit>0x7fffffffUL/99UL)return;
 if(!autoBuilt){
  if(*pSesStartTime || (pIsAccTime && *pIsAccTime))return;
  quick=*autoQuickFlag!=0;
  if(!(StrategyDistributionEnabled?AutoBuildDiverse(laps,quick):AutoBuild(laps,quick)))return;
  autoRandom=StrategyAutoSeed?StrategyAutoSeed:(unsigned long)time(NULL);
  if(!autoRandom)autoRandom=1;
  autoAllocationSeed=autoRandom;
  if(StrategyDistributionEnabled)AutoPrepareDistribution(n);
  for(i=0;i<autoTopCount;i++)AutoPlanLog(0,i+1,&autoTop[i],0);
  autoBuilt=1;
 }
 F1TyreTablesInit();
 for(i=0;i<n;i++){
  c=&pCarStructs[i];id=c->id&63;if(id<1 || id>40 || (c->id&128))continue;
  s=&pCarSetups[id-1];
  if(!autoArmed[id]){
   if(*pSesStartTime || c->lapNr || c->speed || c->numPitStopsDone || (c->flags_90&0x80))continue;
   if(StrategyDistributionEnabled){if(!autoChosen[id])continue;rank=autoPoolRank[id];}
   else{rank=(int)((AutoRand()>>8)%autoTopCount);autoCarPlan[id]=autoTop[rank];}
   autoStint[id]=0;autoStops[id]=0;
   fuel=(int)((autoCarPlan[id].length[0]+1)*1.0395305+0.999999);
   if(fuel>99)continue;
   c->pitStop1=c->pitStop2=c->pitStop3=0;
   s->pit_laps[0]=s->pit_laps[1]=s->pit_laps[2]=100;
   AutoSchedule(c,s,id,laps);
   c->fuelLoad=unit*(unsigned long)fuel;c->fuelLoadLaps=fuel*64;
   s->compound=autoCarPlan[id].compound[0];autoArmed[id]=1;
   AutoPlanLog(id,rank+1,&autoCarPlan[id],fuel);
  }
  set=tyreTablesReady?(int)tyreSetIndex[id-1]:-1;
  changed=set>=0 && set<=6 && autoSetSeen[id] && set!=autoSet[id];
  advanced=c->numPitStopsDone>autoStops[id];next=AutoNextLap(id);
  if(changed || advanced){
   action=1;
   if(next && advanced && (int)c->lapNr>=next-7){autoStint[id]++;action=2;}
   else if(!next)action=3;
   if(advanced && !AutoSchedule(c,s,id,laps))action=4;
   s->compound=autoCarPlan[id].compound[autoStint[id]];
   AutoServiceLog(c,id,action,autoSetSeen[id]?autoSet[id]:-1,set);
  }
  autoStops[id]=c->numPitStopsDone;
  if(set>=0 && set<=6){autoSet[id]=(unsigned char)set;autoSetSeen[id]=1;}
  s->compound=autoCarPlan[id].compound[autoStint[id]];
 }
}
