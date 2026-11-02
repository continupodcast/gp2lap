#include "f1modules.h"
#include "f1native.h"
/* F1 Tower prototype: native race telemetry, no host JSON polling. */
#include "stdinc.h"
#include <stddef.h>
#include "gp2lap.h"
#include "gp2glob.h"
#include "miscahf.h"
#include "f1pitcamera.h"
#include "pages.h"
#include "f1render.h"
#include "f1tower.h"
#include "f1tyres.h"
#include "f1tyreage.h"
extern struct gp2setupdata *pPlayerSetup;
#include "f1compoundstate.h"
#include "f1simulationmode.h"
#include "f1tyretest.h"
#include "f1tyrenativelaps.h"
#include "f1tyrefuel.h"
#include "f1strategytest.h"
#include "f1autostrategy.h"
#include "f1playerstrategy.h"
#include "trackinf.h"
#include "gp2str.h"
#include "f1playerui.h"
static void F1PlayerCircuitPrepare(void)
{
 static double xy[4096];int i,n=0;long slot=pTrackNr?*pTrackNr:-1;
 Gp2InfoTags *tags=slot>=0 && slot<16?GetTrackTags((unsigned char)slot):NULL;
 const char *name=tags && tags->name[0]?tags->name:slot>=0 && slot<16?GP2_GetTrackString(slot):"Circuit";
 if(pTrackSegs && pNumTrackSegs && pNumTrackSegs[0]>=2 && pNumTrackSegs[0]<=2048){
  n=pNumTrackSegs[0];for(i=0;i<n;i++){xy[2*i]=pTrackSegs[i].xPos;xy[2*i+1]=pTrackSegs[i].yPos;}
 }
 F1StrategyCircuitSet(name,xy,n);
}
#include "f1playermodal.h"
#include "f1inlinepit.h"
#include "f1strategyservice.h"
extern void F1CompoundRaceSync(void);
#include "f1qualy.h"
#include "f1race.h"
#include "f1pits.h"
#include "f1pitdisplay.h"
#include "f1gapdiag.h"
#include "f1loops.h"
#include "f1config.h"
#include "f1actions.h"
#include "f1advanced.h"
#include "keyhand.h"
#include "fonts/myfntlib.h"
#include "svga/vesa.h"
/* f1grid.inc retained for separate diagnostic builds. */
extern unsigned short *GP2_LapsInThisRace;
#include "f1camtest.h"
/* Fail compilation if the memory layout used by this module changes. */
typedef char F1CompoundOffsetCheck[offsetof(GP2CSx,compound)==8?1:-1];
typedef char F1SizeCheck[sizeof(GP2Car)==0x330?1:-1];
typedef char F1DistanceCheck[offsetof(GP2Car,field_9E)==0x9e?1:-1];
typedef char F1IdCheck[offsetof(GP2Car,id)==0xa6?1:-1];
typedef char F1PitDigitalCheck[offsetof(GP2Car,digCtrlInput)==0x7d?1:-1];
typedef char F1PitConfirmCheck[offsetof(GP2Car,flags_169)==0x169?1:-1];
typedef char F1StopsCheck[offsetof(GP2Car,numPitStopsDone)==0xd7?1:-1];
typedef char F1TimeCheck[offsetof(GP2Car,timeLast)==0x2d4?1:-1];
typedef char F1FuelCheck[offsetof(GP2Car,fuelLoad)==0x298?1:-1];
static F1FuelState fuelState;
static unsigned long fuelSession, fuelTrack;
static char fuelNotice[80];
static unsigned long fuelNoticeAt;
static char pitNotice[40];
static unsigned long pitNoticeAt;
int F1CockpitView;
static int fullMap;
static F1Row rows[26];
static GP2Car *ordered[26];
static unsigned long lastTick, sessionStart, lastClock, trackSlot;
static long trackLength, previousLineDistance;
static int previousLap=-1, previousLeader=-1, count, leaderLap;
static int validClock, haveLine;
/* 90s theme: FASTEST LAP banner. Initialised silently after a reset so that
   loading a session in progress does not announce an old lap. */
static int fastInit, fastShow, fastId;
static unsigned long fastBest, fastAt;
#define F1_FASTEST_BANNER_MS 6000UL
static unsigned long diagFrames, diagCompose;
static unsigned char diagImage[0x4b000];
static char diagLast[220];
static int diagBudget;
static void F1DiagLog(const char *text)
{
    FILE *f=((FILE *)0) /* obsolete diagnostic disabled in T4 */;
    if(f) { fputs(text,f); fputc('\n',f); fclose(f); }
}
void F1DiagBoot(void)
{
    FILE *f=((FILE *)0) /* obsolete diagnostic disabled in T4 */;
    GapDiagBoot();
    if(f) { fputs("F1 HUD 0.75 - modern panels / map cycle and zoom - booted\n",f); fclose(f); }
    /* Diagnostic builds alone create F1GRID.LOG. */
}
extern int F1GridProbeInit(void);
void F1ControlInit(void)
{
    CamTestInit();
    F1ConfigEnsure();
    F1DiagLog(F1GridProbeInit()?"GRID MENU D6: DRAW HOOK ACTIVE":"GRID MENU D6: SIGNATURE FAILED; ORIGINAL MENU");
    if(f1_allow_external_pit_camera)
        F1DiagLog(F1PitCameraProbeInit()?"PIT CAMERA: EXTERNAL VIEWS ENABLED FOR PLAYER AND AI":"PIT CAMERA: SIGNATURE FAILED; ORIGINAL RESTRICTIONS RETAINED");
    else F1DiagLog("PIT CAMERA: ORIGINAL RESTRICTIONS (AllowExternalPitCamera=0)");
    F1DiagLog(F1NativeFiltersInit()?"NATIVE CAPTIONS: CACHE-SAFE PIXEL FILTERS ACTIVE; NATIVE FASTEST TEXT ALWAYS HIDDEN":"NATIVE CAPTIONS: FILTER SIGNATURE FAILED; ORIGINAL MESSAGES RETAINED");
    F1DiagLog(F1NativeInit()?"NATIVE CAPTIONS: KH INTERFERENCE / COMPACT LAYOUT":"NATIVE CAPTIONS: UNKNOWN SIGNATURE - ORIGINAL STYLE");
}
void F1ControlCancel(void)
{
    F1FuelCancel(&fuelState);fuelState.held=0;fuelNotice[0]=0;
}
static int F1FuelAllowed(void)
{
    return f1_fuel_enabled && pCarStructs && pNumCars && *pNumCars>=1 && *pNumCars<=26 &&
        pCurTime && pSesStartTime && pTrackNr && pIsReplay && !*pIsReplay;
}
void F1ControlKey(unsigned int scan)
{
#if F1_MODULE_FUEL_DRAIN
#include "../modules/fuel-drain/f1tower_F1ControlKey.inc"
#else

#endif
}
static void F1ControlUpdate(void)
{
#if F1_MODULE_FUEL_DRAIN
#include "../modules/fuel-drain/f1tower_F1ControlUpdate.inc"
#else

#endif
}
void F1LogKey(unsigned int scan)
{
    char s[90];
    F1PitCameraKey(scan);
    if((scan>=2 && scan<=11) || scan==15) {
        sprintf(s,"KEY scan=%u page_before=%lu",scan,(unsigned long)activepage);
        F1DiagLog(s);
    }
}
static const char *F1Reason(void)
{
    if(!diagFrames) return "WAITING FOR FRAME HOOK";
    if(!pCarStructs) return "NO CAR ARRAY";
    if(!pSessionMode) return "NO SESSION POINTER";
    if(!(*pSessionMode&0x80)) return "QUALIFYING / PRACTICE";
    if(!pCurTime) return "NO CLOCK POINTER";
    if(pNumCars && (*pNumCars<1 || *pNumCars>26)) return "CAR COUNT OUT OF RANGE";
    if(!count) return "NO VALID CAR IDS";
    if(!pUseSVGA || !*pUseSVGA) return "VGA - SWITCH TO SVGA";
    if(!diagCompose) return "NO CAMERA DRAW HOOK YET";
    return "DATA AND CAMERA HOOK OK";
}
void F1OnRedraw(int below)
{
    if(below) mypicinsertbelow(picbuf,diagImage,GetCopySvgaLinesNum());
    else mypicinsertabove(picbuf,diagImage,GetCopySvgaLinesNum());
}
void F1NoKey(KeyEvent *event) { (void)event; }
void F1DrawDiagnostic(void)
{
    char s[220];
    if(!(activepage&PAGE_F1HUD)) return;
    memset(picbuf,0,sizeof(diagImage));
    sprintf(s,"STATUS: %s; mode=%02X cars=%d raw=%u svga=%u; frame=%s camera=%s; first_ids=%u,%u,%u",
        F1Reason(),pSessionMode?*pSessionMode:0,count,pNumCars?*pNumCars:0,pUseSVGA?*pUseSVGA:0,
        diagFrames?"YES":"NO",diagCompose?"YES":"NO",pCarStructs?pCarStructs[0].id:0,
        pCarStructs?pCarStructs[1].id:0,pCarStructs?pCarStructs[2].id:0);
    if(diagBudget && strcmp(s,diagLast)) { F1DiagLog(s); strcpy(diagLast,s); diagBudget--; }
    SaveThisPage(diagImage,picbuf,GetCopySvgaLinesNum()); ReDrawAllPages(F1OnRedraw);
}
static void F1TogglePart(unsigned long part)
{
    if(activepage&part) {
        PAGESETOFF(part);
        if(!(activepage&PAGE_F1HUD)) RemoveKbdHandler(F1NoKey);
        memset(picbuf,0,sizeof(diagImage));ReDrawAllPages(NULL);
        F1DiagLog(part==PAGE_F1TOWER?"KEY 3: OFF":part==PAGE_F1CARD?"KEY 4: OFF":part==PAGE_F1DRIVER?"KEY 5: OFF":"KEY 6: OFF");return;
    }
    if(!(activepage&PAGE_F1HUD) && !RegisterKbdHandler(F1NoKey,F1OnRedraw)) {
        F1DiagLog("ERROR: PAGE REGISTRATION FAILED");return;
    }
    PAGESETON(part);diagCompose=0;diagBudget=20;diagLast[0]=0;
    F1DiagLog(part==PAGE_F1TOWER?"KEY 3: ON":part==PAGE_F1CARD?"KEY 4: ON":part==PAGE_F1DRIVER?"KEY 5: ON":"KEY 6: ON");F1DrawDiagnostic();
}
static int replayObserved,replayTowerAllowed;
static void F1ReplayObserve(void)
{
    int replay=pIsReplay && *pIsReplay;
    if(replay && !replayObserved) replayTowerAllowed=0;
    replayObserved=replay;
}
static int F1TowerVisible(void)
{
#if F1_MODULE_TIMING_TOWER
#include "../modules/timing-tower/f1tower_F1TowerVisible.inc"
#else
return 0;
#endif
}
void F1Toggle(void)
{
#if F1_MODULE_TIMING_TOWER
#include "../modules/timing-tower/f1tower_F1Toggle.inc"
#else

#endif
}
void F1ToggleCard(void)
{
    if(pSessionMode && (*pSessionMode&0x80) ? !F1_MODULE_PIT_TIMING : !F1_MODULE_SECTORS)return;
    if(pSessionMode && (*pSessionMode&0x80)) {
        F1TogglePart(PAGE_F1CARD);
        strcpy(pitNotice,(activepage&PAGE_F1CARD)?"Pit signal ON":"Pit signal OFF");
        pitNoticeAt=pCurTime?*pCurTime:0;F1DiagLog(pitNotice);return;
    }
    /* Qualifying/practice cards share the upper-centre display slot. */
    if(!(activepage&PAGE_F1CARD) && pSessionMode && !(*pSessionMode&0x80))
        PAGESETOFF(PAGE_F1DRIVER);
    if((activepage&PAGE_F1CARD) && !F1MicroMode && f1_micro_enabled) {
        F1MicroMode=1;F1AdvancedReport();F1DiagLog("KEY 4: MICROSECTORS");return;
    }
    F1MicroMode=0;F1TogglePart(PAGE_F1CARD);
}
void F1ToggleMap(void)
{
#if F1_MODULE_CIRCUIT_MAPS
#include "../modules/circuit-maps/f1tower_F1ToggleMap.inc"
#else

#endif
}
void F1MapZoom(void) {
#if F1_MODULE_CIRCUIT_MAPS
#include "../modules/circuit-maps/f1tower_F1MapZoom.inc"
#else

#endif
}
/* TV race: key 5 cycles plate -> gap panel -> off. Modern CURRENT GAP and
   90s DIFFERENCE share the bottom-centre slot with the driver plate.
   Qualifying and onboard keep the plain plate on/off. */
static int driverDiff;
void F1ToggleDriver(void)
{
#if F1_MODULE_DRIVER_PANELS
#include "../modules/driver-panels/f1tower_F1ToggleDriver.inc"
#else

#endif
}
void F1Tab(unsigned int scan)
{
    F1QualyTab(scan,(activepage&PAGE_F1TOWER)!=0);
    F1RaceKey(scan,(activepage&PAGE_F1TOWER) && pSessionMode && (*pSessionMode&0x80));
}
void F1RaceSession(int fresh)
{
    F1PitsReset();F1TyreAgeReset();
    F1AdvancedReset();
    F1ControlCancel();
    F1Reset();F1RaceReset(fresh);
}
void F1Reset(void)
{
    /* Sampling resets must not erase the session grid or cancel its capture. */
    FIReset();
    F1PitDisplayReset();
    trackLength=0; previousLineDistance=0; previousLap=-1; previousLeader=-1;
    count=0; haveLine=0; validClock=0;
    fastInit=0; fastShow=0;
}
static int before(GP2Car *a,GP2Car *b)
{
    if(a->racePos!=b->racePos) return a->racePos<b->racePos;
    if(a->lapNr!=b->lapNr) return a->lapNr>b->lapNr;
    if(a->csIndex!=b->csIndex) return a->csIndex>b->csIndex;
    return a->segDistFactor>b->segDistFactor;
}
void F1Update(void)
{
    int i,j,id,aheadId,focus,n,leaderId,fastestId; long dist,delta; unsigned long clock,start,track,time,best;
    GP2Car *c,*tmp; double microPosition;
    F1StrategyFrameObserve();
    CamTestUpdate();
    if(!pIsReplay || !*pIsReplay)F1CompoundRaceSync();
    F1StrategyServiceInit();
    F1InlinePitInit();
    F1AutoStrategyTick();
    F1PlayerStrategyTick();
    F1InlinePitTick();
    F1StrategyTestTick();
    F1TyreFuelTick();
    F1TyreTestTick();
    F1PitCameraSample();
    F1ReplayObserve();
    diagFrames++;
    /* F1GRID diagnostic disabled in the release build. */
    F1ControlUpdate();
#if !(F1_MODULE_TIMING_TOWER || F1_MODULE_DRIVER_PANELS || F1_MODULE_SECTORS || F1_MODULE_CIRCUIT_MAPS || F1_MODULE_PIT_TIMING)
    return;
#endif
    F1QualyUpdate();
    F1AdvancedUpdate();
    if(!pCarStructs || !pSessionMode || !(*pSessionMode&0x80) || !pCurTime) { F1Reset(); return; }
    clock=*pCurTime; start=pSesStartTime?*pSesStartTime:0; track=pTrackNr?*pTrackNr:0;
    if(clock<gapDiagFlushAt || clock-gapDiagFlushAt>=1000 || (pPaused && *pPaused))GapDiagFlush(clock);
    if(validClock && track!=trackSlot) F1RaceReset(0);
    if(!(pIsReplay && *pIsReplay) && validClock && (clock<lastClock || start!=sessionStart || track!=trackSlot)) {
        ((void)0);
        F1PitsReset();F1TyreAgeReset();F1Reset();
    }
    if(pIsReplay && *pIsReplay){if(count && fiCars[rows[0].id].seen)((void)0);FIReset();GapDiagFlush(clock);return;}
    if(pNumCars && *pNumCars>=1 && *pNumCars<=26) {
        for(i=0;i<*pNumCars;i++) {
            c=&pCarStructs[i];id=c->id&63;
            F1PitDisplayTick(id,(c->flags_16A&8)!=0 && !((c->flags_90&32) && !(c->flags_5E&1)),clock);
        }
    }
    lastClock=clock; sessionStart=start; trackSlot=track;
    if(validClock && clock-lastTick<100) return;
    if(validClock && clock-lastTick>500){((void)0);FIReset();}
    validClock=1; lastTick=clock;
    n=pNumCars?(int)*pNumCars:26; if(n<1 || n>26) { count=0; return; }
    count=0;
    for(i=0;i<n;i++) { id=pCarStructs[i].id&0x3f; if(id>=1 && id<=40) ordered[count++]=&pCarStructs[i]; }
    for(i=1;i<count;i++) { tmp=ordered[i]; j=i; while(j>0 && before(tmp,ordered[j-1])) { ordered[j]=ordered[j-1]; j--; } ordered[j]=tmp; }
    if(!count) return;
    for(i=0;i<count;i++) {
        c=ordered[i];id=c->id&63;
        microPosition=F1TrackMicroPosition((unsigned long)c->pSeg,c->segDistFactor);
        if(c->segDistFactor>16384 && microPosition>=0)
            ((void)0);
        if(microPosition<1 || microPosition>29 || (fiCars[id].seen && ((c->lapNr-1)*30.0+microPosition<fiCars[id].progress || (c->lapNr-1)*30.0+microPosition-fiCars[id].progress>3 || (c->flags_16A&8) || (c->flags_90&32))))
            ((void)0);
        FIPitSample(id,c->lapNr,microPosition,clock,
            !((c->flags_90&32) && !(c->flags_5E&1)),(c->flags_16A&8)!=0);
    }
    c=ordered[0]; leaderLap=c->lapNr; leaderId=c->id&0x3f; dist=(long)c->field_9E;
    if(previousLap>=0 && leaderLap>previousLap && leaderId==previousLeader) {
        if(haveLine && leaderLap==previousLap+1 && dist>previousLineDistance) trackLength=dist-previousLineDistance;
        else if(leaderLap==2 && previousLap==1 && dist>0 && !trackLength) trackLength=dist;
        previousLineDistance=dist; haveLine=1;
    } else if(leaderId!=previousLeader || leaderLap<previousLap) haveLine=0;
    previousLap=leaderLap; previousLeader=leaderId;
    focus=ppSelectedCS && *ppSelectedCS?(*ppSelectedCS)->id&0x3f:0;
    best=0;fastestId=0;
    for(i=0;i<count;i++) {
        time=ordered[i]->timeBest;
        if(time && !(time&0xf0000000UL) && time<=3600000UL && (!best || time<best)) {
            best=time;fastestId=ordered[i]->id&0x3f;
        }
    }
    if(!fastInit) { fastInit=1; fastBest=best; fastId=fastestId; }
    else if(best && (!fastBest || best<fastBest)) { fastBest=best; fastId=fastestId; fastAt=clock; fastShow=1; }
    for(i=0;i<count;i++) {
        c=ordered[i]; id=c->id&0x3f;
        rows[i].compound=F1AssignedCompound(c->id,pCarSetups,(const GP2CSx *)pPlayerSetup);
        rows[i].id=id; rows[i].pos=i+1; rows[i].lap=c->lapNr; rows[i].focused=id==focus;
        /* GP2Lap's own retirement/pit predicates, rather than a wall-clock timeout. */
        rows[i].out=(c->flags_90&0x20) && !(c->flags_5E&1);
        rows[i].pit=(((unsigned char *)c)[0x16a]&8)!=0 && !rows[i].out;
        {
            int old=f1pits[id].total;
            rows[i].stops=F1PitsUpdate(id,rows[i].pit,c->numPitStopsDone,clock,0);
            if(rows[i].stops!=old) {
                char msg[160];
                sprintf(msg,"PIT COUNT: id=%d total=%d native=%u AD=%02x speed=%u time=%lu",
                    id,rows[i].stops,(unsigned)c->numPitStopsDone,
                    (unsigned)c->flags_AD,(unsigned)c->speed,clock);
                F1DiagLog(msg);
            }
        }
        rows[i].tyreLaps=PerCarTyreCompounds?F1TyreAgeTick(id,c->lapNr,rows[i].stops):-1;
        rows[i].gap=-1;
        rows[i].leaderGap=i?-1:0;rows[i].lapsBehind=0;
        rows[i].fastest=id==fastestId;
        rows[i].finished=(c->flags_5E&1)!=0; /* Native per-car race-over bit. */
        rows[i].positionChange=FIPosition(id,i+1,c->lapNr,clock,f1_position_time);
        if(i && trackLength>0 && !rows[i].out) {
            delta=(long)ordered[0]->field_9E-(long)c->field_9E;
            if(delta<0) delta=0;
            rows[i].lapsBehind=(int)(delta/trackLength);
        }
        aheadId=i?ordered[i-1]->id&0x3f:0;
        FICollect(id,aheadId,0,clock);
        FICollect(id,i?leaderId:0,1,clock);
    }
    for(i=0;i<count;i++) {
        id=rows[i].id;
        rows[i].gap=rows[i].lap>=2?fiBucket[id][0].precise:-1;
        rows[i].leaderGap=i?(rows[i].lap>=2?fiBucket[id][1].precise:-1):0;
    }
    F1RaceStats(rows,count);
}
/* Native world coordinates use 11 more fractional bits than track segments. */
static void F1MapCompose(unsigned char *dst,unsigned char *pal,int focus)
{
#if F1_MODULE_CIRCUIT_MAPS
#include "../modules/circuit-maps/f1tower_F1MapCompose.inc"
#else

#endif
}
/* The existing GP2Lap draw hooks identify cockpit versus external view.
   EAX=ESI points to the native image buffer; cockpit refresh is skipped upstream. */
void F1Compose(unsigned char *dst)
{
    unsigned char *pal;
    int start=0,n=count,focus=0,i,pos=0,qn=count,isQualy,slot90;
    const F1QRow *qr;
    F1Row visible[26];
    F1ReplayObserve();
    if(activepage&PAGE_F1HUD) diagCompose++;
    if(!pUseSVGA || !*pUseSVGA) return;
    if(fuelNotice[0] && (!pCurTime || *pCurTime<fuelNoticeAt || *pCurTime-fuelNoticeAt>=3000)) fuelNotice[0]=0;
    if(pitNotice[0] && (!pCurTime || *pCurTime<pitNoticeAt || *pCurTime-pitNoticeAt>=2500))pitNotice[0]=0;
    if(!(activepage&PAGE_F1HUD) && !fuelNotice[0] && !pitNotice[0] && !camNotice[0]) return;
    pal=IDACodeReftoDataRef(0x7f624); /* same palette reference as GP2Lap screenshot code */
    if(!pal) return;
    if(fuelNotice[0]) F1RenderNotice(dst,pal,fuelNotice);
    if(pitNotice[0]) F1RenderNotice(dst,pal,pitNotice);
    if(camNotice[0]) F1RenderNotice(dst,pal,camNotice);
    if(!(activepage&PAGE_F1HUD)) return;
    if(F1CockpitView && ppCockpitCS && *ppCockpitCS) focus=(*ppCockpitCS)->id&0x3f;
    else if(ppSelectedCS && *ppSelectedCS) focus=(*ppSelectedCS)->id&0x3f;
    if(activepage&PAGE_F1MAP) F1MapCompose(dst,pal,focus);
    /* TV race with key 5 on the gap panel: it replaces the plate in the same slot.
       Elsewhere (qualifying, onboard) that choice falls back to the plate. */
    slot90=!F1CockpitView && driverDiff && pSessionMode && (*pSessionMode&0x80);
    if(slot90 && (activepage&PAGE_F1DRIVER) && count) {
        if(f1_theme==F1_THEME_90S) F1Render90Difference(dst,pal,rows,count,focus);
        else F1RenderCurrentGap(dst,pal,rows,count,focus);
    }
    if((activepage&PAGE_F1DRIVER) && !slot90) {
        isQualy=pSessionMode && !(*pSessionMode&0x80);
        if(isQualy) {
            qr=F1QRows(&qn);
            for(i=0;i<qn;i++) if(qr[i].id==focus && qr[i].best) pos=qr[i].pos;
        } else for(i=0;i<count;i++) if(rows[i].id==focus) pos=rows[i].pos;
        F1RenderDriverAtXY(dst,pal,focus,pos,F1DriverLeft(F1CockpitView),F1DriverTop(F1CockpitView,isQualy,qn));
    }
    if(pSessionMode && !(*pSessionMode&0x80)) {
        F1QualyCompose(dst,pal,focus,F1TowerVisible(),activepage&PAGE_F1CARD,F1CockpitView);return;
    }
    if(!F1CockpitView && (activepage&PAGE_F1CARD) && count && !(pIsReplay && *pIsReplay) && pCurTime) {
        F1PitDisplay cards[26];int pc=0;
        for(i=0;i<count;i++) if(pitClocks[rows[i].id].active) {
            cards[pc].id=rows[i].id;cards[pc].position=rows[i].pos;
            cards[pc].elapsed=pitClocks[rows[i].id].elapsed;pc++;
        }
        F1RenderPitDisplays(dst,pal,cards,pc);
    }
    if(!F1TowerVisible() || !count) return;
    if(F1CockpitView) {
        if(ppCockpitCS && *ppCockpitCS) focus=(*ppCockpitCS)->id&0x3f;
        else if(ppSelectedCS && *ppSelectedCS) focus=(*ppSelectedCS)->id&0x3f;
        n=F1Window(rows,count,focus,&start);
    } else if(ppSelectedCS && *ppSelectedCS) focus=(*ppSelectedCS)->id&0x3f;
    for(i=0;i<n;i++) { visible[i]=rows[start+i]; visible[i].focused=visible[i].id==focus; }
    F1RenderRace(dst,pal,visible,n,leaderLap,GP2_LapsInThisRace?*GP2_LapsInThisRace:0,F1RaceMode());
    if(fastShow && pCurTime && *pCurTime>=fastAt && *pCurTime-fastAt<F1_FASTEST_BANNER_MS) {
        if(f1_theme==F1_THEME_90S)
            F1Render90Fastest(dst,pal,fastId,(long)fastBest,F1CockpitView?F1_DRIVER_Y+F1_DRIVER_H+4:F1_DRIVER_Y);
        else F1RenderModernFastest(dst,pal,fastId,(long)fastBest,F1_DRIVER_Y+F1_DRIVER_H+4);
    } else fastShow=0;
}
/* Install the original GP2Lap fastest-caption hook for both HUD themes.
   Keep the user's atlNoFastestLap switch and the original At The Line case. */
extern unsigned long SupressFastestLap;
unsigned long F1FastestHook=F1_MODULE_TIMING_TOWER;
int F1SuppressNativeFastest(void)
{
    return F1_MODULE_TIMING_TOWER;
}
int (*fpF1SuppressNativeFastest)(void)=F1SuppressNativeFastest;
void (*fpF1Compose)(unsigned char *)=F1Compose;
