#include "f1compoundrule.h"
#ifndef F1PLAYERSTRATEGY_H
#define F1PLAYERSTRATEGY_H
#include "f1playeroptions.h"
/* T14: before-start modal prototype for one player car. Shares the validated
   service/scheduling machinery with NPC plans, without random player choice. */
extern int F1PlayerStrategyChoose(const AutoPlan *,int,int,int,int *);
static AutoPlan playerEditedPlan;
static int playerMenuDone,playerMode=-1,playerArmedId,playerChoice,playerNativeId;
static unsigned long playerClock;
static int playerPitActive,playerPitOverride=-1,playerInitialCompound,playerCompoundRule;
static unsigned int playerUsedMask,playerPitUsedBefore;
extern int F1PlayerPitChoose(int,int,int,int,int *);
static int F1PlayerNextDifferent(int id,int current)
{
 int i;AutoPlan *p=&autoCarPlan[id];
 for(i=autoStint[id]+1;i<p->count;i++)if(p->compound[i]!=current)return p->compound[i];
 /* Repairs after the final planned stint still have a valid alternative. */
 for(i=0;i<p->count;i++)if(p->compound[i]!=current)return p->compound[i];
 return current==0?1:0;
}
static int F1PlayerNeedsDifferent(void)
{
 return playerCompoundRule && playerUsedMask && !(playerUsedMask&(playerUsedMask-1));
}

void F1PlayerStrategyReset(void)
{
 playerMenuDone=0;playerArmedId=0;playerNativeId=0;autoPlayerLogId=0;playerMode=-1;playerClock=0;playerPitActive=0;playerPitOverride=-1;playerUsedMask=0;playerPitUsedBefore=0;playerCompoundRule=0;
}
static void F1PlayerSchedule(GP2Car *c)
{
 GP2CSx *s=&pCarSetups[(c->id&63)-1],*ps=(GP2CSx *)pPlayerSetup;
 AutoSchedule(c,s,c->id&63,*GP2_LapsInThisRace);
 if(ps){ps->num_pitstops=s->num_pitstops;memcpy(ps->pit_laps,s->pit_laps,3);ps->compound=s->compound;}
}
static int F1PlayerApply(GP2Car *c,int rank)
{
 int id,fuel;unsigned long unit;
 if(!c)return 0;id=c->id&63;
 if(!c || !(c->id&128) || !pSesStartTime || *pSesStartTime || c->lapNr || c->speed ||
   c->numPitStopsDone || rank<0 || rank>=autoTopCount || !autoFuelUnit)return 0;
 unit=*autoFuelUnit;fuel=(int)((autoTop[rank].length[0]+1)*1.0395305+0.999999);
 if(!unit || fuel>99 || unit>0x7fffffffUL/(unsigned)fuel)return 0;
 autoCarPlan[id]=autoTop[rank];autoPoolRank[id]=(unsigned char)rank;
 autoStint[id]=autoStops[id]=0;autoSetSeen[id]=0;
 c->pitStop1=c->pitStop2=c->pitStop3=0;
 pCarSetups[id-1].pit_laps[0]=pCarSetups[id-1].pit_laps[1]=pCarSetups[id-1].pit_laps[2]=100;
 pCarSetups[id-1].compound=autoCarPlan[id].compound[0];
 F1PlayerSchedule(c);c->fuelLoad=unit*(unsigned long)fuel;c->fuelLoadLaps=fuel*64;
 playerInitialCompound=autoCarPlan[id].compound[0];playerUsedMask=1U<<playerInitialCompound;
 playerCompoundRule=StrategyRequireCompoundChange && (!(autoQuickFlag && *autoQuickFlag) || autoCarPlan[id].count>1);
 autoPlayerLogId=id;playerArmedId=id;playerChoice=rank+1;autoGrid[id]=(unsigned char)(c->racePos/2+1);
 AutoPlanLog(id,rank+1,&autoCarPlan[id],fuel);return 1;
}
/* PI/PO callbacks bracket the stationary native service. The native fuel menu
   may reset both planned/done counters to zero; do not mistake that for a stint. */
void F1PlayerPitEnter(GP2Car *c)
{
 int id;
 if(!StrategyPlayerEnabled || !PerCarTyreCompounds || !pCarSetups || !c || ((c->id&63)!=playerArmedId && (c->id&63)!=playerNativeId) || !(c->id&128) ||
   !pIsReplay || *pIsReplay || !pSessionMode || *pSessionMode!=0x80)return;
 id=c->id&63;playerPitActive=1;playerPitOverride=-1;playerPitUsedBefore=playerUsedMask;
 if(!StrategyPlayerPitChoice)return;
 /* T16: selection is non-modal, in GP2's live refuelling menu.
    The native draw hook shows the scheduled/current compound; no waiting. */

}
static void F1PlayerFinishWithoutStops(GP2Car *c)
{
 int id=c->id&63,current,next,remaining,forced=0;GP2CSx *s=&pCarSetups[id-1];
 if(!playerPitActive || c->numPitStops!=0)return;
 current=s->compound;
 if(F1PlayerNeedsDifferent() && current==playerInitialCompound){
  next=F1PlayerNextDifferent(id,current);s->compound=(unsigned char)next;
  autoCarPlan[id].compound[autoStint[id]]=(unsigned char)next;
  playerUsedMask|=1U<<next;
  if(pPlayerSetup)((GP2CSx *)pPlayerSetup)->compound=(unsigned char)next;
  forced=1;
 }
 /* The player's explicit zero replaces future planned stops, including when
    the native menu resets numPitStopsDone. Preserve GP2's chosen fuel. */
 autoCarPlan[id].count=(unsigned char)(autoStint[id]+1);
 remaining=*GP2_LapsInThisRace-(int)c->lapNr+1;if(remaining<1)remaining=1;
 if(remaining>255)remaining=255;autoCarPlan[id].length[autoStint[id]]=(unsigned char)remaining;
 c->pitStop1=c->pitStop2=c->pitStop3=0;s->num_pitstops=0;
 s->pit_laps[0]=s->pit_laps[1]=s->pit_laps[2]=100;
 if(pPlayerSetup){GP2CSx *ps=(GP2CSx *)pPlayerSetup;ps->num_pitstops=0;memcpy(ps->pit_laps,s->pit_laps,3);}
 if(forced)AutoServiceLog(c,id,5,autoSetSeen[id]?autoSet[id]:-1,tyreTablesReady?tyreSetIndex[id-1]:-1);
}
void F1PlayerStrategyService(void)
{
 int i,id,set,changed,advanced,next,action;GP2Car *c;GP2CSx *s;
 if(!StrategyPlayerEnabled || (!playerArmedId && !playerNativeId) || !PerCarTyreCompounds ||
  !pSessionMode || *pSessionMode!=0x80 || !pIsReplay || *pIsReplay || !pNumCars ||
  !pCarStructs || !pCarSetups || !pCurTime || !GP2_LapsInThisRace)return;
 if(*pNumCars<1 || *pNumCars>26)return;F1TyreTablesInit();
 for(i=0;i<*pNumCars;i++){
  c=&pCarStructs[i];id=c->id&63;if((id!=playerArmedId && id!=playerNativeId) || !(c->id&128))continue;
  s=&pCarSetups[id-1];set=tyreTablesReady?(int)tyreSetIndex[id-1]:-1;
  if(id==playerNativeId){
   /* Original strategy owns all scheduling/fuel. Only the explicit pit choice
      is applied after native service, and again on exit if no event is seen. */
   if(playerPitActive && playerPitOverride>=0){
    s->compound=(unsigned char)playerPitOverride;
    if(pPlayerSetup)((GP2CSx *)pPlayerSetup)->compound=s->compound;
   }
   continue;
  }
  changed=set>=0 && set<=6 && autoSetSeen[id] && set!=autoSet[id];
  advanced=c->numPitStopsDone>autoStops[id];next=AutoNextLap(id);
  if(changed || advanced){
   action=1;
   if(next && advanced && (int)c->lapNr>=next-7){autoStint[id]++;action=2;}
   else if(!next)action=3;
   if(playerPitOverride>=0){
    autoCarPlan[id].compound[autoStint[id]]=(unsigned char)playerPitOverride;action=6;
   }
   s->compound=autoCarPlan[id].compound[autoStint[id]];
   playerUsedMask=(playerPitActive?playerPitUsedBefore:playerUsedMask)|(1U<<s->compound);
   if(advanced && c->numPitStops!=0)F1PlayerSchedule(c);
   if(pPlayerSetup)((GP2CSx *)pPlayerSetup)->compound=s->compound;
   AutoServiceLog(c,id,action,autoSetSeen[id]?autoSet[id]:-1,set);
  }
  if(playerPitActive && playerPitOverride>=0){
   s->compound=(unsigned char)playerPitOverride;
   autoCarPlan[id].compound[autoStint[id]]=(unsigned char)playerPitOverride;
   playerUsedMask=playerPitUsedBefore|(1U<<s->compound);
   if(pPlayerSetup)((GP2CSx *)pPlayerSetup)->compound=s->compound;
  }
  F1PlayerFinishWithoutStops(c);
  autoStops[id]=c->numPitStopsDone;
  if(set>=0 && set<=6){autoSet[id]=(unsigned char)set;autoSetSeen[id]=1;}
 }
}
void F1PlayerPitLeave(GP2Car *c)
{
 if(!StrategyPlayerEnabled || !PerCarTyreCompounds || !pCarSetups || !c || ((c->id&63)!=playerArmedId && (c->id&63)!=playerNativeId) || !pIsReplay || *pIsReplay)return;
 F1PlayerStrategyService();playerPitActive=0;playerPitOverride=-1;
}
static void F1PlayerStrategyTick(void)
{
 int i,n,humans=0,id=0,choice,quick,savedCount;unsigned char *a,*b,*q;GP2Car *c=0;
 AutoPlan savedTop[10];
 if(!StrategyPlayerEnabled || !PerCarTyreCompounds || StrategyTestEnabled || TyreTestFuelLaps || TyreTestCompound ||
  !pCurTime || !pSessionMode || !pIsReplay || *pIsReplay || !pSesStartTime || !pNumCars ||
  !pCarStructs || !pCarSetups || !pPlayerSetup || !GP2_LapsInThisRace)return;
 if(playerMode!=*pSessionMode || *pCurTime<playerClock){F1PlayerStrategyReset();}
 playerMode=*pSessionMode;playerClock=*pCurTime;
 if(playerMode!=0x80)return;
 F1PlayerStrategyService();
 if(playerMenuDone || *pSesStartTime || *GP2_LapsInThisRace<6)return;
 n=*pNumCars;if(n<1 || n>26)return;
 for(i=0;i<n;i++)if(pCarStructs[i].id&128){humans++;c=&pCarStructs[i];}
 if(!humans)return;
 /* Multiple player cars share native pPlayerSetup. Do not promise independent
    per-player compounds until that path is implemented and verified. */
 if(humans!=1){playerMenuDone=1;return;}
 id=c->id&63;if(id<1 || id>40 || c->lapNr || c->speed || c->numPitStopsDone)return;
 a=IDAtoFlat(0x2C4EFUL);b=IDAtoFlat(0x2BEF0UL);q=IDAtoFlat(0x6AE32UL);
 if(!a || !b || !q || memcmp(a,"\xf7\x25",2) || memcmp(b,"\xf7\x35",2) || memcmp(a+2,b+2,4) ||
  q[0]!=0xC6 || q[1]!=0x05 || q[6]!=0x80){playerMenuDone=1;return;}
 autoFuelUnit=(unsigned long *)IDACodeReftoDataRef(0x2C4F1UL);
 autoQuickFlag=(unsigned char *)IDACodeReftoDataRef(0x6AE34UL);if(!autoFuelUnit || !autoQuickFlag)return;
 quick=*autoQuickFlag!=0;
 savedCount=autoTopCount;memcpy(savedTop,autoTop,sizeof(savedTop));
 if(!AutoBuildPlayerOptions(*GP2_LapsInThisRace,quick)){playerMenuDone=1;autoTopCount=savedCount;memcpy(autoTop,savedTop,sizeof(savedTop));return;}
 /* NPC plans are already copied into autoCarPlan. The chooser never mutates pool. */
 playerMenuDone=1;choice=-1;
 if(F1PlayerStrategyChoose(autoTop,autoTopCount,id,*GP2_LapsInThisRace,&choice) && (choice>=0 || choice==-2)){
  if(choice==-2){autoTop[0]=playerEditedPlan;choice=0;}
  if(!F1PlayerApply(c,choice))playerNativeId=id;
 }else playerNativeId=id;
 autoTopCount=savedCount;memcpy(autoTop,savedTop,sizeof(savedTop));
}
#endif
