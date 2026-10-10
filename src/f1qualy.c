#include "f1advanced.h"
/* Adapter for the already-resolved GP2Lap memory pointers. */
#include "stdinc.h"
#include <stddef.h>
#include "gp2lap.h"
#include "gp2glob.h"
#include "miscahf.h"
#include "timing/gp2struc.h"
#include "f1qualy.h"
extern struct gp2timetable *GP2_BestLaptimes,*GP2_BestSplit1,*GP2_BestSplit2;
typedef char F1QStartCheck[offsetof(GP2Car,timeLapStart)==0x54?1:-1];
typedef char F1QBestCheck[offsetof(GP2Car,timeBest)==0x2e0?1:-1];
static unsigned long oldClock,oldStart,oldTrack;
static int initialized,oldMode;
static long remaining=-1;
void F1QualyUpdate(void)
{
    F1QCar cars[F1_Q_MAX];GP2Car *c;int i,j,id,n,m,entrants,seen[41],mode,chosen,score,bestScore;
    unsigned long clock,start,track;long minutes,elapsed;
    if(!pSessionMode || !pCurTime || !pCarStructs) { initialized=0;F1QReset();return; }
    mode=*pSessionMode;
    if(mode&0x80) { initialized=0;F1QReset();return; }
    clock=*pCurTime;start=pSesStartTime?*pSesStartTime:0;track=pTrackNr?*pTrackNr:0;
    if(!initialized || clock<oldClock || start!=oldStart || track!=oldTrack || mode!=oldMode) F1QReset();
    initialized=1;oldClock=clock;oldStart=start;oldTrack=track;oldMode=mode;
    memset(cars,0,sizeof(cars));memset(seen,0,sizeof(seen));n=0;
    entrants=pNumCars?*pNumCars:26;
    if(entrants<1 || entrants>F1_Q_MAX) {F1QReset();return;}
    m=entrants>26?26:entrants;
    /* Fast-lap order retains drivers whose active car slot is no longer present. */
    /* Order is a 40-byte table immediately preceding the 40 best times. */
    for(i=0;i<entrants;i++) {
        id=pFastLapCars?pFastLapCars[i]&0x3f:0;
        if(id<1 || id>40 || seen[id]) continue;
        cars[n++].id=id;seen[id]=1;
    }
    for(i=0;i<m && n<entrants;i++) {
        id=pCarStructs[i].id&0x3f;if(id<1 || id>40 || seen[id]) continue;
        cars[n++].id=id;seen[id]=1;
    }
    for(i=0;i<n;i++) {
        id=cars[i].id;cars[i].pit=1;cars[i].split=2;
        chosen=-1;bestScore=-1;
        for(j=0;j<m;j++) if((pCarStructs[j].id&0x3f)==id) {
            c=&pCarStructs[j];score=0;
            if(pTrackSegs && pNumTrackSegs && (unsigned long)c->pSeg>=(unsigned long)pTrackSegs &&
               (unsigned long)c->pSeg<(unsigned long)(pTrackSegs+*pNumTrackSegs) &&
               ((unsigned long)c->pSeg-(unsigned long)pTrackSegs)%sizeof(GP2Seg)==0) score=2;
            if(score && !(c->flags_16A&8) && !(c->flags_90&32))score++;
            if(score>bestScore){chosen=j;bestScore=score;}
        }
        if(chosen>=0) {
            j=chosen;
            c=&pCarStructs[j];cars[i].active=1;cars[i].lap=c->lapNr;cars[i].split=c->splitNr;
            cars[i].pit=(((unsigned char *)c)[0x16a]&8)!=0;cars[i].start=c->timeLapStart;
            cars[i].last=c->timeLast;cars[i].s1=c->timeLastSpl1;cars[i].s2=c->timeLastSpl2;
            cars[i].best=c->timeBest;cars[i].best1=c->timeBestSpl1;cars[i].best2=c->timeBestSpl2;
        }
        if(GP2_BestLaptimes && GP2_BestSplit1 && GP2_BestSplit2) {
            cars[i].best=GP2_BestLaptimes->car[id-1];cars[i].best1=GP2_BestSplit1->car[id-1];cars[i].best2=GP2_BestSplit2->car[id-1];
        }
    }
    F1QTick(cars,n,clock);
    remaining=-1;
    /* GP2Mem: qualifying duration is the WORD following race-distance percent. */
    if((mode&0x40) && pRaceDistPerc) {
        minutes=*((unsigned short *)pRaceDistPerc+1);
        if(minutes>0 && minutes<=240) {
            elapsed=(long)(clock-start);
            remaining=minutes*60000L-elapsed-60000L;if(remaining<0) remaining=0;
        }
    }
}
void F1QualyCompose(unsigned char *dst,const unsigned char *pal,int focus,int table,int card,int cockpit)
{
    int n;const F1QRow *rows;F1QCard detail;
    if(!pSessionMode || (*pSessionMode&0x80) || !pCurTime) return;
    rows=F1QRows(&n);
    if(table) F1RenderQualy(dst,pal,rows,n,remaining,!(*pSessionMode&0x40),(*pCurTime%15000)<10000,cockpit,focus);
    if(card && F1QGetCard(focus,*pCurTime,&detail)) {
        F1RenderQualyCard(dst,pal,&detail,cockpit);F1AdvancedCard(dst,pal,focus,cockpit);
    }
}
