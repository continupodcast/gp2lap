#include "f1csv.h"
/* Optional T8 experiment: native one-stop plan, C then B.
   Only arm on a fresh stationary grid. No state reconstruction from saves. */
extern unsigned short *GP2_LapsInThisRace;
static unsigned char strategyArmed[41];
/* Strategy stint is separate from native service count. */
static unsigned char strategyStint[41],strategyPrevStops[41],strategyPrevSet[41];
static unsigned char strategySetSeen[41];
static int strategyServiceFileReady;
static void F1StrategyServiceEvent(GP2Car *c,int oldSet,int set,int oldStops,int action)
{
 FILE *f;
 if(!TyreDiagnostics && !StrategyDiagnostics)return;
 f=F1CsvOpen("F1STOPS.CSV",&strategyServiceFileReady,"time_ms,id,lap,old_set,new_set,old_stops,new_stops,action,compound,fuel_raw,next_stop_lap,accelerated\n");if(!f)return;
 fprintf(f,"%lu,%u,%u,%d,%d,%d,%u,%d,%d,%lu,%u,%d\n",*pCurTime,(unsigned)(c->id&63),
  (unsigned)c->lapNr,oldSet,set,oldStops,(unsigned)c->numPitStopsDone,action,
  strategyStint[c->id&63]?1:2,c->fuelLoad,
  strategyStint[c->id&63]?0:(unsigned)((*GP2_LapsInThisRace+1)/2),F1StrategyAccelerationMode());
 fclose(f);strategyServiceFileReady=1;
}
static unsigned long strategyClock;
static int strategyMode=-1,strategyChecked;
static unsigned long *strategyFuelUnit;
static void F1StrategyTestTick(void)
{
 int i,id,m,n,laps,fuelLaps,set,changed,countChanged,planned,done,action;unsigned long now,unit;unsigned char *a,*b;
 GP2Car *c;GP2CSx *s,*player=(GP2CSx *)pPlayerSetup;
 if(!StrategyTestEnabled || !PerCarTyreCompounds || TyreTestFuelLaps || TyreTestCompound)return;
 if(!pSessionMode || !pCurTime || !pSesStartTime || !pNumCars || !pCarStructs || !pCarSetups ||
    !GP2_LapsInThisRace || !pIsReplay || *pIsReplay)return;
 now=*pCurTime;m=*pSessionMode;
 if(m!=strategyMode || now<strategyClock){
  memset(strategyArmed,0,sizeof(strategyArmed));memset(strategyStint,0,sizeof(strategyStint));
  memset(strategySetSeen,0,sizeof(strategySetSeen));
 }
 strategyMode=m;strategyClock=now;
 if(m!=0x80)return;
 n=*pNumCars;laps=*GP2_LapsInThisRace;if(n<1 || n>26 || laps<6 || laps>255)return;
 if(!strategyChecked){
  strategyChecked=1;a=IDAtoFlat(0x2C4EFUL);b=IDAtoFlat(0x2BEF0UL);
  if(!a || !b || memcmp(a,"\xf7\x25",2) || memcmp(b,"\xf7\x35",2) || memcmp(a+2,b+2,4))return;
  strategyFuelUnit=(unsigned long *)IDACodeReftoDataRef(0x2C4F1UL);
 }
 if(!strategyFuelUnit)return;unit=*strategyFuelUnit;
 /* First half plus two native fuel laps as a test reserve. */
 fuelLaps=(laps+1)/2+2;if(!unit || unit>0x7fffffffUL/(unsigned)fuelLaps)return;
 for(i=0;i<n;i++){
  c=&pCarStructs[i];id=c->id&63;if(id<1 || id>40)continue;
  if(StrategyAutoEnabled && !(c->id&128))continue;s=&pCarSetups[id-1];
  if(!strategyArmed[id]){
   if(*pSesStartTime || c->lapNr || c->speed || c->numPitStopsDone || (c->flags_90&0x80))continue;
   s->num_pitstops=1;s->pit_laps[0]=50;s->pit_laps[1]=s->pit_laps[2]=100;s->compound=2;
   /* Setup stores percentages; live car stores absolute lap numbers.
      Verified native 2C5B6 reads this byte and subtracts lapNr directly. */
   c->numPitStops=1;c->pitStop1=(unsigned char)((laps+1)/2);
   c->pitStop2=c->pitStop3=0;
   c->fuelLoad=unit*(unsigned long)fuelLaps;c->fuelLoadLaps=fuelLaps*64;
   if((c->id&128) && player){player->num_pitstops=1;player->pit_laps[0]=50;
    player->pit_laps[1]=player->pit_laps[2]=100;player->compound=2;}
   strategyArmed[id]=1;strategyStint[id]=0;strategyPrevStops[id]=c->numPitStopsDone;strategySetSeen[id]=0;
  }
  if(StrategyRepairTestEnabled){
   F1TyreTablesInit();set=tyreTablesReady?(int)tyreSetIndex[id-1]:-1;
   countChanged=c->numPitStopsDone>strategyPrevStops[id];
   changed=set>=0 && set<=6 && strategySetSeen[id] && set!=strategyPrevSet[id];
   planned=(laps+1)/2;
   if(countChanged || changed){
    action=1; /* Keep compound and scheduled strategy stop. */
    if(!strategyStint[id] && countChanged && (int)c->lapNr>=planned-7){
     strategyStint[id]=1;action=2; /* Native service accepted in its 8-lap window. */
    }else if(strategyStint[id])action=3; /* Extra service, retain B. */
    else if(countChanged){
     /* An unexpected early counted service must not consume the strategy stop.
        Retain the native completed count; append our pending stop at its index. */
     done=c->numPitStopsDone;
     if(done<3){
      c->numPitStops=(unsigned char)(done+1);s->num_pitstops=(unsigned char)(done+1);
      if(done==1)c->pitStop2=(unsigned char)planned;
      if(done==2)c->pitStop3=(unsigned char)planned;
      s->pit_laps[done]=50;
      if((c->id&128) && player){player->num_pitstops=s->num_pitstops;player->pit_laps[done]=50;}
     }else action=4; /* Native schedule capacity exhausted; record, do not overflow. */
    }
    F1StrategyServiceEvent(c,strategySetSeen[id]?strategyPrevSet[id]:-1,set,strategyPrevStops[id],action);
   }
   strategyPrevStops[id]=c->numPitStopsDone;
   if(set>=0 && set<=6){strategyPrevSet[id]=(unsigned char)set;strategySetSeen[id]=1;}
   s->compound=strategyStint[id]?1:2;
  }else s->compound=c->numPitStopsDone?1:2;
  if((c->id&128) && player)player->compound=s->compound;
 }
}
