#include "f1csv.h"
#ifndef F1TYRELAPS_H
#define F1TYRELAPS_H
/* T11: lap boundaries own output; frame samples supply optional grip statistics. */
typedef struct {
 int active,id,lap,set,compound,pits,bad,closedLap;
 unsigned long begin,last,wear,lastWear,fuel,n;
 double gripSum;
 unsigned gripMin,gripMax;
} F1TyreLap;
static F1TyreLap tyreLaps[41];
static int tyreLapFileReady;
static unsigned long tyreBoundaryClock;
static int tyreBoundaryMode=-1;
/* Suspending telemetry invalidates sampled averages, not lap boundaries. */
static void F1TyreLapsReset(void)
{
 int i;for(i=1;i<=40;i++){tyreLaps[i].n=0;tyreLaps[i].gripSum=0;tyreLaps[i].bad|=64;}
}
static void F1TyreLapBegin(F1TyreLap *s,GP2Car *c,unsigned long now,int set,int compound,unsigned long wear,int bad)
{
 int closed=s->closedLap;
 memset(s,0,sizeof(*s));s->active=1;s->id=c->id&63;s->lap=c->lapNr;s->closedLap=closed;
 s->set=set;s->compound=compound;s->pits=c->numPitStopsDone;s->begin=now;s->last=now;
 s->wear=s->lastWear=wear;s->fuel=c->fuelLoad;s->bad=bad;
}
static void F1TyreLapSample(int slot,GP2Car *c,unsigned long now,int set,int compound,unsigned long wear,int tables)
{
 int id=c->id&63;F1TyreLap *s;
 (void)slot;if(id<1 || id>40)return;s=&tyreLaps[id];
 if(!s->active)F1TyreLapBegin(s,c,now,set,compound,wear,1);
 if(c->lapNr!=s->lap || now<s->last){s->bad|=64;return;}
 if(set!=s->set || compound!=s->compound || c->numPitStopsDone!=s->pits || wear<s->lastWear)s->bad|=2;
 if(!tables || set<0 || set>6)s->bad|=4;
 if(now-s->last>2000UL)s->bad|=64;
 if(!s->n){s->gripMin=c->grip;s->gripMax=c->grip;}
 s->lastWear=wear;s->last=now;s->gripSum+=c->grip;s->n++;
 if(c->grip<s->gripMin)s->gripMin=c->grip;
 if(c->grip>s->gripMax)s->gripMax=c->grip;
}
static void F1TyreLapBoundary(GP2Car *c,unsigned long now,int set,int compound,unsigned long wear,int tables,int source)
{
 int id=c->id&63,bad,meanOK;F1TyreLap *s;FILE *f;unsigned long elapsed;
 if(id<1 || id>40)return;s=&tyreLaps[id];
 if(s->active && now<s->begin)s->active=0;
 if(!s->active){s->closedLap=-1;F1TyreLapBegin(s,c,now,set,compound,wear,1);return;}
 /* Native event can be delivered twice; never manufacture or duplicate laps. */
 if(s->closedLap==(int)c->lapNr || c->lapNr==s->lap)return;
 bad=s->bad;if(source)bad|=128;
 if(c->lapNr!=s->lap+1)bad|=16;
 if(c->timeLast==0 || c->timeLast>=268435456UL)bad|=32;
 if(set!=s->set || compound!=s->compound || c->numPitStopsDone!=s->pits || wear<s->lastWear)bad|=2;
 if(!tables || set<0 || set>6)bad|=4;
 meanOK=s->n>=2 && now>=s->last && now-s->last<=2000UL && !(bad&(1|2|4|16|64));
 if(!meanOK)bad|=64;
 elapsed=now-s->begin;
 f=F1CsvOpen("F1TYLAPS.CSV",&tyreLapFileReady,"time_ms,id,lap_before,lap_after,compound,set_index,last_lap_ms,samples,grip_mean_raw,grip_min_raw,grip_max_raw,wear_start_raw,wear_end_raw,fuel_start_raw,fuel_end_raw,invalid_mask,elapsed_clock_ms,grip_end_raw,compound_end,set_end,accelerated,native_accelerated,session_start,boundary_source\n");if(f){
  fprintf(f,"%lu,%d,%d,%u,%d,%d,%lu,%lu,",now,id,s->lap,(unsigned)c->lapNr,s->compound,s->set,c->timeLast,s->n);
  if(meanOK)fprintf(f,"%.3f,%u,%u,",s->gripSum/s->n,s->gripMin,s->gripMax);else fputs(",,,",f);
  fprintf(f,"%lu,%lu,%lu,%lu,%d,%lu,%u,%d,%d,%d,%d,%lu,%d\n",s->wear,wear,s->fuel,c->fuelLoad,bad,elapsed,
   (unsigned)c->grip,compound,set,F1StrategyAccelerationMode(),pIsAccTime?*pIsAccTime!=0:0,pSesStartTime?*pSesStartTime:0,source);
  fclose(f);tyreLapFileReady=1;
 }
 s->closedLap=c->lapNr;F1TyreLapBegin(s,c,now,set,compound,wear,0);
}
#endif
