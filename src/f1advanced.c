#include "stdinc.h"
#include <stddef.h>
#include <math.h>
#include "gp2lap.h"
#include "gp2glob.h"
#include "f1config.h"
#include "f1micro.h"
#include "f1advanced.h"
#include "f1render.h"
#include "f1qualy.h"
typedef char F1SpeedOffset[offsetof(GP2Car,speed)==0x16?1:-1];
typedef char F1SegOffset[offsetof(GP2Car,pSeg)==0x10?1:-1];
static F1MicroState micro;
int F1MicroMode;
static double distances[2049],bounds[4];
static unsigned long oldClock,oldSession,oldTrack;
static int ready,mode,segments,splitReady;
static GP2Seg *trackBase;
static unsigned long reportClock,accepted,rejected[6];
static int reports;
static unsigned long resetCount;
static const char *sampleStatus="NOT UPDATED";
static void logstate(const char *s)
{
    FILE *f=fopen("F1HUD.LOG","a");if(f) {fputs(s,f);fputc('\n',f);fclose(f);}
}
/* Diagnostic state is separate from timing state: observing a sample must not
   change its acceptance, continuity, reference times or displayed colours. */
enum { D_OK,D_SEG,D_DUP,D_FRAC,D_PIT,D_OUT,D_ROAD,D_OUTLAP,D_MISSING,
       D_START,D_REWIND,D_GAP,D_LAP,D_BACK,D_JUMP,D_WRAP,D_SAME,D_COUNT };
static const char *reasonNames[D_COUNT]={"OK","SEGMENT","DUPLICATE","FRACTION",
    "PIT","RETIRED","OUTSIDE","OUTLAP","MISSING","START","REWIND","TIME_GAP",
    "LAP_JUMP","BACKWARD","POSITION_JUMP","LAP_MISMATCH","SAME_CLOCK"};
typedef struct {
    unsigned long clock,previousClock;double position,previousPosition;
    int lap,previousLap,seen,haveCross;
} MicroBreak;
typedef struct {
    unsigned long counts[D_COUNT];MicroBreak first[D_COUNT];
    unsigned char captured[D_COUNT];int touched;
} MicroDiag;
static MicroDiag microDiag[41];
static int diagArmed;
static void tracedSample(int id,int lap,double position,unsigned long clock,int valid,int reason)
{
    F1MicroCar *c;MicroDiag *d;MicroBreak *b;double current;
    if(id>=1 && id<=40 && diagArmed) {
        c=&micro.cars[id];d=&microDiag[id];current=position;
        if(reason!=D_MISSING) d->touched=1;
        if(valid>0) {
            if(c->seen && lap==c->lap+1 && position<c->position) current+=30;
            if(!c->seen) reason=D_START;
            else if(clock<c->clock || lap<c->lap) reason=D_REWIND;
            else if(clock-c->clock>500) reason=D_GAP;
            else if(lap>c->lap+1) reason=D_LAP;
            else if(current<c->position) reason=D_BACK;
            else if(current-c->position>3) reason=D_JUMP;
            else if(lap!=c->lap && current<30) reason=D_WRAP;
            else if(clock==c->clock) reason=D_SAME;
        }
        d->counts[reason]++;
        if(reason!=D_OK && reason!=D_SAME && !d->captured[reason]) {
            d->captured[reason]=1;b=&d->first[reason];
            b->clock=clock;b->previousClock=c->clock;b->position=position;
            b->previousPosition=c->position;b->lap=lap;b->previousLap=c->lap;
            b->seen=c->seen;b->haveCross=c->haveCross;
        }
    }
    /* Negative validity records a skipped sample without discarding the
       previous valid endpoint. The core still enforces its 500 ms limit. */
    if(valid>=0) F1MicroSample(&micro,id,lap,position,clock,valid);
}
static void reportCars(void)
{
    int id,i,n;FILE *f;MicroDiag *d;MicroBreak *b;F1MicroCar *c;
    f=fopen("F1HUD.LOG","a");if(!f) return;
    for(id=1;id<=40;id++) {
        d=&microDiag[id];if(!d->touched) continue;c=&micro.cars[id];n=0;
        for(i=0;i<30;i++) if(c->colors[i]) n++;
        fprintf(f,"MICRO CAR: id=%d lap=%d pos=%.6f clock=%lu seen=%d cross=%d newlap=%d colours=%d mask=",
            id,c->lap,c->position,c->clock,c->seen,c->haveCross,c->newLap,n);
        for(i=0;i<30;i++) fprintf(f,"%d",c->colors[i]);
        fputs(" counts=",f);
        for(i=0;i<D_COUNT;i++) if(d->counts[i]) fprintf(f,"%s:%lu,",reasonNames[i],d->counts[i]);
        fputc('\n',f);
        for(i=0;i<D_COUNT;i++) if(d->captured[i]) {
            b=&d->first[i];
            fprintf(f,"MICRO BREAK: id=%d reason=%s clock=%lu prevclock=%lu lap=%d prevlap=%d pos=%.6f prevpos=%.6f seen=%d cross=%d\n",
                id,reasonNames[i],b->clock,b->previousClock,b->lap,b->previousLap,
                b->position,b->previousPosition,b->seen,b->haveCross);
        }
        memset(d->captured,0,sizeof(d->captured));
    }
    fclose(f);
}
void F1AdvancedReport(void)
{
    reportClock=0;reports=0;diagArmed=1;memset(microDiag,0,sizeof(microDiag));
    logstate("MICRO DIAGNOSTIC 0.22: PER-CAR CAPTURE ARMED FOR TWELVE MINUTES; COUNTS CUMULATIVE");
}
void F1AdvancedReset(void)
{
    F1MicroReset(&micro);ready=0;trackBase=NULL;
    accepted=0;memset(rejected,0,sizeof(rejected));resetCount++;
}
static int geometry(void)
{
    int i,j,s1=-1,s2=-1;double dx,dy,len;
    if(!pTrackSegs || !pNumTrackSegs || *pNumTrackSegs<2 || *pNumTrackSegs>2048) return 0;
    segments=*pNumTrackSegs;distances[0]=0;
    for(i=0;i<segments;i++) {
        j=(i+1)%segments;dx=pTrackSegs[j].xPos-pTrackSegs[i].xPos;dy=pTrackSegs[j].yPos-pTrackSegs[i].yPos;
        len=sqrt(dx*dx+dy*dy)*GP2_SEG_FACTOR;
        if(len<=0) len=0.001;
        distances[i+1]=distances[i]+len;
        if(i>0 && s1<0 && (pTrackSegs[i].fl_split&1)) s1=i;
        if(i>0 && s2<0 && (pTrackSegs[i].fl_split&2)) s2=i;
    }
    splitReady=s1>0 && s2>s1 && s2<segments;
    bounds[0]=0;bounds[1]=s1>0?distances[s1]:0;bounds[2]=s2>0?distances[s2]:0;bounds[3]=distances[segments];
    if(bounds[3]<=1) return 0;
    trackBase=pTrackSegs;
    logstate(splitReady?"MICROSECTORS: NATIVE SPLIT GEOMETRY READY":"MICROSECTORS: SPLIT MARKERS UNAVAILABLE");return 1;
}
#include "f1progress.h"
double F1TrackFraction(unsigned long address,unsigned int factor)
{
    unsigned long base=(unsigned long)pTrackSegs;int seg;double fraction;
    if(!ready || trackBase!=pTrackSegs || !F1SegmentFactor(factor,&fraction) || address<base ||
       address>=base+segments*sizeof(GP2Seg) || (address-base)%sizeof(GP2Seg) || bounds[3]<=0)return -1;
    seg=(int)((address-base)/sizeof(GP2Seg));
    return F1SegmentDistance(distances,segments,seg,fraction)/bounds[3];
}
double F1TrackMicroPosition(unsigned long address,unsigned int factor)
{
    double fraction=F1TrackFraction(address,factor),distance;int sector;
    if(fraction<0 || !splitReady)return -1;
    distance=fraction*bounds[3];
    if(distance>=bounds[3])distance=bounds[3]-0.000001;
    sector=distance<bounds[1]?0:distance<bounds[2]?1:2;
    return sector*10+10*(distance-bounds[sector])/(bounds[sector+1]-bounds[sector]);
}
void F1AdvancedUpdate(void)
{
    int i,id,seg,sector,valid,pit,out,onRoad,chosen[41],rank[41],score;unsigned long clock,start,track,addr,base;
    double distance,fraction,position;GP2Car *c;F1QCard card;
    unsigned char seen[41];
    if(!pCurTime || !pSesStartTime || !pTrackNr || !pSessionMode || !pCarStructs || !pNumCars || *pNumCars>28 || *pNumCars<1) {sampleStatus="POINTERS/COUNT";F1AdvancedReset();return;}
    if(!pIsReplay || *pIsReplay) {sampleStatus="REPLAY/POINTER";F1AdvancedReset();return;}
    if(pPaused && *pPaused) {sampleStatus="PAUSED";return;}
    clock=*pCurTime;start=*pSesStartTime;track=*pTrackNr;
    if(!ready || clock<oldClock || start!=oldSession || track!=oldTrack || mode!=*pSessionMode || trackBase!=pTrackSegs || !pNumTrackSegs || segments!=*pNumTrackSegs) {
        F1AdvancedReset();if(!geometry()) {sampleStatus="GEOMETRY";return;}ready=1;
    }
    sampleStatus=!f1_micro_enabled?"DISABLED":splitReady?"SAMPLING":"NO SPLIT MARKERS";
    oldClock=clock;oldSession=start;oldTrack=track;mode=*pSessionMode;
    memset(seen,0,sizeof(seen));
    for(id=0;id<=40;id++) {chosen[id]=-1;rank[id]=-1;}
    /* Resolve slots first. An inactive duplicate must never cancel another
       slot's accepted sample; sample each CarId exactly once per update. */
    base=(unsigned long)pTrackSegs;
    for(i=0;i<*pNumCars && i<26;i++) {
        c=&pCarStructs[i];id=c->id&63;
        if(id<1 || id>40) continue;
        addr=(unsigned long)c->pSeg;
        valid=addr>=base && addr<base+segments*sizeof(GP2Seg) && (addr-base)%sizeof(GP2Seg)==0;
        score=valid?2:0;
        if(valid && !(c->flags_16A&8) && !(c->flags_90&32)) score++;
        if(chosen[id]>=0) tracedSample(id,c->lapNr,0,clock,-1,D_DUP);
        if(score>rank[id]) {chosen[id]=i;rank[id]=score;}
    }
    for(id=1;id<=40;id++) {
        if(chosen[id]<0) continue;
        i=chosen[id];
        c=&pCarStructs[i];
        id=c->id&63;pit=(c->flags_16A&8)!=0;out=(c->flags_90&32)!=0;
        addr=(unsigned long)c->pSeg;base=(unsigned long)pTrackSegs;
        valid=id>=1 && id<=40 && !seen[id] && addr>=base && addr<base+segments*sizeof(GP2Seg) && (addr-base)%sizeof(GP2Seg)==0;
        if(!valid) {rejected[0]++;seen[id]=1;tracedSample(id,c->lapNr,0,clock,0,D_SEG);continue;}
        seen[id]=1;seg=(int)((addr-base)/sizeof(GP2Seg));fraction=c->segDistFactor/16384.0;
        if(fraction<0 || fraction>1) {rejected[1]++;tracedSample(id,c->lapNr,0,clock,pit || out?0:-1,pit?D_PIT:out?D_OUT:D_FRAC);continue;}
        distance=distances[seg]+fraction*(distances[seg+1]-distances[seg]);
        if(distance>=bounds[3]) distance=bounds[3]-0.000001;
        /* GP2 uses the same units for segPosX and segment side limits:
           0x13154 stores the lateral coordinate directly at car+0x0a;
           0x2ad09..0x2ad29 compares it to segment+0x60/0x62 without
           doubling. The header's width * 2 comment is not a conversion
           from segPosX. Doubling here incorrectly halved usable width. */
        onRoad=(short)c->segPosX<=pTrackSegs[seg].width && -(short)c->segPosX<=pTrackSegs[seg].width_60;
        if(f1_micro_enabled && splitReady) {
            sector=distance<bounds[1]?0:distance<bounds[2]?1:2;
            position=sector*10+10*(distance-bounds[sector])/(bounds[sector+1]-bounds[sector]);
            if(!(*pSessionMode&0x80) && (!F1QGetCard(id,clock,&card) || card.outlap)) valid=0;
            if(pit) rejected[2]++;else if(out) rejected[3]++;
            else if(!valid) rejected[5]++;else accepted++;
            /* Lateral excursions do not invalidate longitudinal progress.
               These are HUD estimates, not official lap-validity decisions. */
            if(!onRoad) rejected[4]++;
            tracedSample(id,c->lapNr,position,clock,valid && !pit && !out,
                pit?D_PIT:out?D_OUT:!valid?D_OUTLAP:D_OK);
        }
    }
    for(id=1;id<=40;id++) if(!seen[id]) tracedSample(id,0,0,clock,0,D_MISSING);
 }
static void diagnostic(int focus)
{
    char line[320];unsigned long clock,addr,base;int i,seg=-1,measured=0,coloured=0;
    GP2Car *c=NULL;F1QCard card;int haveCard=0;
    if(!diagArmed || !pCurTime) return;
    if(reports>=144) {diagArmed=0;logstate("MICRO DIAGNOSTIC: CAPTURE COMPLETE");return;}
    clock=*pCurTime;
    if(reports && clock>=reportClock && clock-reportClock<5000) return;
    reportClock=clock;reports++;reportCars();
    for(i=0;i<30;i++) if(micro.best[i]) measured++;
    if(focus>=1 && focus<=40) for(i=0;i<30;i++) if(micro.cars[focus].colors[i]) coloured++;
    sprintf(line,"MICRO STATE: clock=%lu status=%s enabled=%d ready=%d splits=%d segments=%d resets=%lu focus=%d measured=%d/30 coloured=%d/30",
        clock,sampleStatus,f1_micro_enabled,ready,splitReady,segments,resetCount,focus,measured,coloured);logstate(line);
    sprintf(line,"MICRO COUNTS: accepted=%lu bad_segment=%lu bad_fraction=%lu pit=%lu out=%lu lateral_observations=%lu outlap=%lu",
        accepted,rejected[0],rejected[1],rejected[2],rejected[3],rejected[4],rejected[5]);logstate(line);
    if(pCarStructs && pNumCars && *pNumCars>=1 && *pNumCars<=28)
        for(i=0;i<*pNumCars && i<26;i++) if((pCarStructs[i].id&63)==focus) {c=&pCarStructs[i];break;}
    if(!c) {logstate("MICRO FOCUS: NO ACTIVE CAR");return;}
    addr=(unsigned long)c->pSeg;base=(unsigned long)pTrackSegs;
    if(pTrackSegs && segments>0 && segments<=2048 && addr>=base && addr<base+segments*sizeof(GP2Seg) && (addr-base)%sizeof(GP2Seg)==0) seg=(int)((addr-base)/sizeof(GP2Seg));
    haveCard=F1QGetCard(focus,clock,&card);
    sprintf(line,"MICRO RAW: id=%d lap=%d seg=%d ptr=%lu base=%lu fraction=%u lateral=%d left=%u right=%u flags90=%u flags16A=%u card=%d outlap=%d",
        c->id,c->lapNr,seg,addr,base,(unsigned int)c->segDistFactor,(int)(short)c->segPosX,
        seg<0?0:(unsigned int)pTrackSegs[seg].width_60,seg<0?0:(unsigned int)pTrackSegs[seg].width,
        (unsigned int)c->flags_90,(unsigned int)c->flags_16A,haveCard,haveCard?card.outlap:-1);logstate(line);
}

void F1AdvancedCard(unsigned char *dst,const unsigned char *pal,int focus,int cockpit)
{
    static const unsigned char unavailable[30]={0};
    if(!F1MicroMode) return;
    diagnostic(focus);
    if(!f1_micro_enabled) return;
    F1RenderMicro(dst,pal,ready && splitReady && focus>=1 && focus<=40?micro.cars[focus].colors:unavailable,cockpit);
}
