#include "f1modules.h"
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
#if F1_MODULE_PLAYER_RACE_STRATEGY
#include "../modules/player-race-strategy/f1playerstrategy_F1PlayerPitEnter.inc"
#else

#endif
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
#if F1_MODULE_PLAYER_RACE_STRATEGY
#include "../modules/player-race-strategy/f1playerstrategy_F1PlayerStrategyService.inc"
#else

#endif
}
void F1PlayerPitLeave(GP2Car *c)
{
#if F1_MODULE_PLAYER_RACE_STRATEGY
#include "../modules/player-race-strategy/f1playerstrategy_F1PlayerPitLeave.inc"
#else

#endif
}
static void F1PlayerStrategyTick(void)
{
#if F1_MODULE_PLAYER_RACE_STRATEGY
#include "../modules/player-race-strategy/f1playerstrategy_F1PlayerStrategyTick.inc"
#else

#endif
}
#endif
