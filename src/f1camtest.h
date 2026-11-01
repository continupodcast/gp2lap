#include "f1features.h"
extern int F1CockpitView;

static int camTargetOn,camTargetId;
static int camTargetExternal;
static long camTargetGap=-1;
static double camTargetYaw;
static unsigned long camTargetLogAt;
static GP2Car *camTargetOwn;
/* Integrated camera controls: original definitions remain untouched. */
#define PAGE_F1CAMNOTICE 0x100UL
static int camDynamicOn,cam360On;
static unsigned long cam360Start;
static int cam360Radius=500;
static int camChaseRadius=1000,camCycle;
static unsigned char camSavedMode;
static unsigned long camSavedIndex;
static int camSavedView;
static char camNotice[64];
static unsigned long camNoticeAt;
volatile short camPitch;
static void CamTestUpdate(void);
static void CamShowNotice(const char *message)
{
    strcpy(camNotice,message);camNoticeAt=pCurTime?*pCurTime:0;
    activepage|=PAGE_F1CAMNOTICE;
}
static GP2Car **camViewed;
static unsigned short camRawHeading;
static double camLagAngle;
#include <math.h>
volatile short camOrbitYaw;
static unsigned long camFollowClock;
static GP2Car *camFollowCar;
static double camFollowHeading;
static int camFollowValid;
static int camOrbitHeld;
volatile short camOffsetX,camOffsetY,camOffsetZ;
volatile unsigned long camTestEnabled,camTestExpected,camTestApplied;
volatile short camTestCopy[8];
static long *camFinalX,*camFinalY,*camFinalZ;
volatile unsigned long camReadPtr,camReadCount;
volatile short camReadWords[8];
void (*camReadOriginal)(void);
void __cdecl F1CamReadWrapped(void);
static unsigned char *camTable;
/* C2: read-only native camera mode, zoom and height telemetry. */
static unsigned char *camMode;
static long *camZoom,*camHeight;
static unsigned long *camIndex;
static int camReady;
static void CamLog(const char *event,unsigned int scan,int control)
{
    FILE *f=((FILE *)0) /* obsolete diagnostic disabled in T4 */;
    if(!f)return;
    fprintf(f,"%s t=%lu scan=%u ctrl=%d mode=%02X index=%lu zoom0=%ld zoom1=%ld height0=%ld height1=%ld calls=%lu readptr=%08lX expected9=%08lX readX=%d readY=%d readZ=%d enabled=%lu applied=%lu outX=%ld outY=%ld outZ=%ld dx=%d dy=%d dz=%d yaw=%d valid=%d heading=%u lag=%.1f\n",
      event,pCurTime?*pCurTime:0,scan,control,
      camReady?*camMode:0,camReady?*camIndex:999UL,
      camReady?camZoom[0]:0,camReady?camZoom[1]:0,
      camReady?camHeight[0]:0,camReady?camHeight[1]:0,
      camReadCount,camReadPtr,(unsigned long)(camTable?camTable+144:0),
      camReadWords[0],camReadWords[2],camReadWords[1],camTestEnabled,camTestApplied,
      camFinalX?*camFinalX:0,camFinalY?*camFinalY:0,camFinalZ?*camFinalZ:0,camOffsetX,camOffsetY,camOffsetZ,camOrbitYaw,camFollowValid,camRawHeading,camLagAngle);
    fprintf(f,"CUSTOM cycle=%d chase_distance=%d orbit_distance=%d cockpit=%d\n",camCycle,camChaseRadius,cam360Radius,F1CockpitView);
    fclose(f);
}
static void CamTestInit(void)
{
    unsigned char *site;
    FILE *f=((FILE *)0) /* obsolete diagnostic disabled in T4 */;
    if(f){fputs("GP2Lap v0.75 Cameras - rear follow smoothing, Ctrl+L toggle / Ctrl+R restore; 3/4 height, 6/7 zoom, 5 reset.\n",f);fclose(f);}
    if(memcmp(IDAtoFlat(0x38635),"\x8b\x2c\xbd",3) ||
       memcmp(IDAtoFlat(0x386ab),"\x8b\x2c\xbd",3) ||
       memcmp(IDAtoFlat(0x3861e),"\x80\x3d",2) ||
       *IDAtoFlat(0x38589)!=0xa1){CamLog("SIGNATURE_FAILED",0,0);return;}
    camZoom=(long *)IDACodeReftoDataRef(0x38638);
    camHeight=(long *)IDACodeReftoDataRef(0x386ae);
    camMode=IDACodeReftoDataRef(0x38620);
    camIndex=(unsigned long *)IDACodeReftoDataRef(0x3858a);
    site=IDAtoFlat(0x385b4);
    if(site[0]!=0xe8 || site+5+*(long *)(site+1)!=IDAtoFlat(0x384a1) ||
       memcmp(IDAtoFlat(0x385a9),"\x81\xc3",2)) {CamLog("HOOK_SIGNATURE_FAILED",0,0);return;}
    camTable=IDACodeReftoDataRef(0x385ab);
    camViewed=(GP2Car **)IDACodeReftoDataRef(0x3855d);
    camTestExpected=(unsigned long)(camTable+144);
    camFinalX=(long *)IDACodeReftoDataRef(0x38530);
    camFinalY=(long *)IDACodeReftoDataRef(0x38540);
    camFinalZ=(long *)IDACodeReftoDataRef(0x38550);
    camReadOriginal=(void (*)(void))IDAtoFlat(0x384a1);
    *(long *)(site+1)=(unsigned char *)F1CamReadWrapped-(site+5);
    camReady=1;CamLog("READY",0,0);
}
static void CamOrbitCompute(unsigned long clock)
{
    GP2Car *car;unsigned long dt;unsigned short raw;double heading,diff,relative,radians,alpha,limit,tau;
    if(!camViewed || !pCarStructs || !pNumCars)return;
    car=*camViewed;
    if(*pNumCars<1 || *pNumCars>26 || (unsigned long)car<(unsigned long)pCarStructs ||
       (unsigned long)car>=(unsigned long)(pCarStructs+*pNumCars) ||
       ((unsigned long)car-(unsigned long)pCarStructs)%sizeof(GP2Car))return;
    memcpy(&raw,(unsigned char *)car+0x15e,2);heading=(double)raw;camRawHeading=raw;
    if(!camFollowValid || car!=camFollowCar || clock<camFollowClock || clock-camFollowClock>1000UL){
        camFollowHeading=heading;camFollowValid=1;camFollowCar=car;camFollowClock=clock;
    }
    dt=clock-camFollowClock;camFollowClock=clock;
    diff=heading-camFollowHeading;
    while(diff>32768.0)diff-=65536.0;
    while(diff< -32768.0)diff+=65536.0;
    /* Faster recentering when the heading error is already shrinking. */
    tau=fabs(diff)<fabs(camLagAngle)?250.0:900.0;
    alpha=1.0-exp(-(double)dt/tau);
    camFollowHeading+=diff*alpha;
    while(camFollowHeading>=65536.0)camFollowHeading-=65536.0;
    while(camFollowHeading<0.0)camFollowHeading+=65536.0;
    relative=camFollowHeading-heading;
    while(relative>32768.0)relative-=65536.0;
    while(relative< -32768.0)relative+=65536.0;
    /* Close views require less lateral angle to keep the car stable in frame. */
    limit=910.222222222222*(double)camChaseRadius/1000.0;
    if(limit>910.222222222222)limit=910.222222222222;
    if(relative>limit)relative=limit;
    if(relative< -limit)relative=-limit;
    /* Evaluation: mirror chase movement and matching aim compensation. */
    relative=-relative;
    camLagAngle=relative;
    radians=relative*6.283185307179586/65536.0;
    camOffsetX=(short)((double)camChaseRadius*sin(radians));
    camOffsetY=(short)(-(double)camChaseRadius*cos(radians));camOffsetZ=200;
    /* Partial aim compensation leaves slight lateral movement in frame. */
    camOrbitYaw=(short)(-relative*0.85);
}
static void Cam360Compute(unsigned long clock)
{
    unsigned long phase;double angle;
    if(clock<cam360Start)cam360Start=clock;
    phase=(clock-cam360Start)%16000UL;
    angle=(double)phase*6.283185307179586/16000.0;
    camOffsetX=(short)(cam360Radius*sin(angle));
    camOffsetY=(short)(-cam360Radius*cos(angle));camOffsetZ=100;
    camOrbitYaw=(short)(unsigned short)(0UL-(phase*65536UL)/16000UL);
    camPitch=0;
}
static int CamCarValid(GP2Car *car)
{
    return pCarStructs && pNumCars && *pNumCars>=1 && *pNumCars<=26 &&
        (unsigned long)car>=(unsigned long)pCarStructs &&
        (unsigned long)car<(unsigned long)(pCarStructs+*pNumCars) &&
        !(((unsigned long)car-(unsigned long)pCarStructs)%sizeof(GP2Car));
}
typedef struct {long x,y;int id,seen;double speed;} CamMotion;
static CamMotion camMotion[26];
static GP2Car *camLock;
static int camLockId,camReplayState=-1;
static unsigned long camSampleClock,camTargetTimeline,camTargetSession;
static int camMotionReady;
static int CamEligible(GP2Car *car)
{
    return CamCarValid(car) && !(car->flags_AD&4) && !(car->flags_90&32) && !(car->flags_5E&1);
}
static void CamTargetReset(void)
{
    memset(camMotion,0,sizeof(camMotion));camMotionReady=0;
    camLock=0;camLockId=0;camTargetYaw=0;camTargetOwn=0;
}
static double CamSeparationMs(GP2Car *own,GP2Car *other)
{
    int a=(int)(own-pCarStructs),b=(int)(other-pCarStructs);
    double dx=(double)other->xPos-own->xPos,dy=(double)other->yPos-own->yPos;
    double speed=(camMotion[a].speed+camMotion[b].speed)*0.5;
    if(speed<=0.0)return -1;
    return sqrt(dx*dx+dy*dy)/speed;
}
static unsigned long CamMotionSample(unsigned long clock,GP2Car *own)
{
    unsigned long dt=0;int i,changed=0,replay=pIsReplay && *pIsReplay;
    unsigned long session=pSesStartTime?*pSesStartTime:0;
    double dx,dy,instant,alpha;
    if(replay!=camReplayState || session!=camTargetSession || (camMotionReady && clock<camSampleClock)){
        CamTargetReset();camReplayState=replay;camTargetSession=session;
    }
    for(i=0;i<*pNumCars;i++)if(camMotion[i].seen &&
       (pCarStructs[i].xPos!=camMotion[i].x || pCarStructs[i].yPos!=camMotion[i].y))changed=1;
    if(camMotionReady){
        if(clock>camSampleClock)dt=clock-camSampleClock;
        else if(replay && changed && pFrameTime)
            dt=(*pFrameTime*100UL)/256UL;
    }
    if(dt>1000UL){CamTargetReset();dt=0;}
    if((!replay && pPaused && *pPaused) || (replay && !changed))dt=0;
    alpha=dt?1.0-exp(-(double)dt/200.0):0;
    for(i=0;i<*pNumCars;i++){
        CamMotion *m=&camMotion[i];GP2Car *c=&pCarStructs[i];
        if(dt && m->seen && m->id==(c->id&63)){
            dx=(double)c->xPos-m->x;dy=(double)c->yPos-m->y;
            instant=sqrt(dx*dx+dy*dy)/(double)dt;
            /* Seed immediately on launch, then smooth brief braking differences. */
            if(m->speed<=0)m->speed=instant;else m->speed+=(instant-m->speed)*alpha;
        }else if(m->id!=(c->id&63))m->speed=0;
        m->x=c->xPos;m->y=c->yPos;m->id=c->id&63;m->seen=1;
    }
    camSampleClock=clock;camMotionReady=1;camTargetTimeline+=dt;
    if(own!=camTargetOwn){camLock=0;camLockId=0;camTargetYaw=0;camTargetOwn=own;}
    return dt;
}
static void CamTargetCompute(unsigned long clock)
{
    GP2Car *own=camViewed?*camViewed:0,*target=0,*candidate;
    unsigned short raw;unsigned long dt;int i;double desired=0,dx,dy,h,right,forward,gap,best=500.0,diff;
    camOffsetX=0;camOffsetY=camTargetExternal?-1400:-170;
    camOffsetZ=camTargetExternal?200:120;camPitch=-910;
    camTargetId=0;camTargetGap=-1;
    if(!CamCarValid(own)){CamTargetReset();camOrbitYaw=0;return;}
    dt=CamMotionSample(clock,own);
    memcpy(&raw,(unsigned char *)own+0x15e,2);h=(double)raw*6.283185307179586/65536.0;
    if(CamEligible(own)){
        if(CamEligible(camLock) && (camLock->id&63)==camLockId){
            gap=CamSeparationMs(own,camLock);
            if(gap>=0 && gap<500.0){target=camLock;camTargetGap=(long)gap;}
        }
        if(!target){
            camLock=0;camLockId=0;
            for(i=0;i<*pNumCars;i++){
                candidate=pCarStructs+i;if(candidate==own || !CamEligible(candidate))continue;
                dx=(double)candidate->xPos-own->xPos;dy=(double)candidate->yPos-own->yPos;
                right=dx*cos(h)-dy*sin(h);forward=dx*sin(h)+dy*cos(h);
                /* Acquire ahead only, within a +/-60-degree cone. Keep behind once locked. */
                if(forward<=0 || fabs(right)>forward*1.732050807568877)continue;
                gap=CamSeparationMs(own,candidate);
                if(gap>=0 && gap<best){best=gap;target=candidate;}
            }
            if(target){camLock=target;camLockId=target->id&63;camTargetGap=(long)best;}
        }
    }else {camLock=0;camLockId=0;}
    if(target){
        dx=((double)target->xPos-own->xPos)/256.0;dy=((double)target->yPos-own->yPos)/256.0;
        right=dx*cos(h)-dy*sin(h);forward=dx*sin(h)+dy*cos(h)+(camTargetExternal?0.0:170.0);
        desired=atan2(right,forward)*65536.0/6.283185307179586;
        camTargetId=target->id&63;
    }
    diff=desired-camTargetYaw;
    while(diff>32768.0)diff-=65536.0;
    while(diff< -32768.0)diff+=65536.0;
    camTargetYaw+=diff*(1.0-exp(-(double)dt/250.0));
    while(camTargetYaw>32767.0)camTargetYaw-=65536.0;
    while(camTargetYaw< -32768.0)camTargetYaw+=65536.0;
    if(camTargetExternal){
        /* Orbit opposite the rival bearing; selected car remains the pivot.
           Native yaw points along the same axis through pivot and rival. */
        double bearing=camTargetYaw*6.283185307179586/65536.0;
        camOffsetX=(short)(-1400.0*sin(bearing));
        camOffsetY=(short)(-1400.0*cos(bearing));
    }
    camOrbitYaw=(short)camTargetYaw;
    if(camTargetTimeline<camTargetLogAt || camTargetTimeline-camTargetLogAt>=1000UL){
        FILE *f=((FILE *)0) /* obsolete diagnostic disabled in T4 */;camTargetLogAt=camTargetTimeline;
        if(f){fprintf(f,"TARGET t=%lu timeline=%lu replay=%d own=%d target=%d proximity_ms=%ld wanted=%.1f yaw=%d dt=%lu\n",
            clock,camTargetTimeline,camReplayState,own->id&63,camTargetId,camTargetGap,desired,camOrbitYaw,dt);fclose(f);}
    }
}
/* Restore exactly the native view selected before entering the custom cycle. */
static void CamCycleOriginal(void)
{
    camDynamicOn=cam360On=camTargetOn=0;camCycle=0;camTestEnabled=0;
    camFollowValid=0;CamTargetReset();camTargetExternal=0;
    if(camSavedView){*camMode=camSavedMode;*camIndex=camSavedIndex;camSavedView=0;}
    CamShowNotice("Original Camera");
}
void F1CameraTestKey(unsigned int scan,int control)
{
    char message[64];int requested;
    if(scan==0xa4 || scan==0xa6 || scan==0x96){
        if(camOrbitHeld==(int)(scan&127))camOrbitHeld=0;return;
    }
    if(!EnhancedCameras || !camReady || (scan&128) || !control)return;
    requested=scan==0x24?1:scan==0x26?2:scan==0x16?3:0;
    if(requested && camOrbitHeld!=(int)scan){
        camOrbitHeld=(int)scan;
        if(F1CockpitView){CamShowNotice("External view required");CamLog("BLOCKED_COCKPIT",scan,control);return;}
        if(!camCycle){camSavedMode=*camMode;camSavedIndex=*camIndex;camSavedView=1;}
        if(requested==3){
            if(camCycle!=3){camCycle=3;camTargetExternal=0;}
            else if(!camTargetExternal)camTargetExternal=1;
            else {camCycle=0;camTargetExternal=0;}
        }else {camCycle=camCycle==requested?0:requested;camTargetExternal=0;}
        camDynamicOn=camCycle==1;cam360On=camCycle==2;camTargetOn=camCycle==3;
        camFollowValid=0;CamTargetReset();
        if(!camCycle)CamCycleOriginal();
        else {
            *camMode=0xc0;*camIndex=camCycle==1?9:camCycle==2?8:7;
            cam360Start=pCurTime?*pCurTime:0;
            CamShowNotice(camCycle==1?"Dynamic Chase Camera":camCycle==2?"360 Camera":camTargetExternal?"External Target Camera":"Target Camera");
        }
        CamTestUpdate();CamLog("CAMERA_DIRECT",scan,control);
    }else if(scan==0x13){
        CamCycleOriginal();CamTestUpdate();CamLog("CAMERA_RESTORE",scan,control);
    }else if((scan==0x17 || scan==0x25) && *camMode==0xc0 &&
             ((*camIndex==8 && cam360On) || (*camIndex==9 && camDynamicOn))){
        int *radius=(*camIndex==8)?&cam360Radius:&camChaseRadius;
        *radius+=(scan==0x25)?100:-100;
        if(*radius<300)*radius=300;
        if(*radius>2000)*radius=2000;
        sprintf(message,"%s distance: %d",*camIndex==8?"360":"Chase",*radius);CamShowNotice(message);
        CamTestUpdate();CamLog("CAMERA_DISTANCE",scan,control);
    }
}
static void CamTestUpdate(void)
{
    unsigned long index;
    if(camNotice[0] && (!pCurTime || *pCurTime<camNoticeAt || *pCurTime-camNoticeAt>=2500UL)){
        camNotice[0]=0;activepage&=~PAGE_F1CAMNOTICE;
    }
    if(!EnhancedCameras || !camReady || !pCurTime)return;
    index=(*camMode==0xc0)?*camIndex:0;
    camTestEnabled=0;
    if(index==9 && camDynamicOn){
        camTestExpected=(unsigned long)(camTable+9*16);camPitch=-1820;
        /* Default pose is safe even while waiting for valid car telemetry. */
        camOffsetX=0;camOffsetY=(short)-camChaseRadius;camOffsetZ=200;camOrbitYaw=0;
        CamOrbitCompute(*pCurTime);camTestEnabled=1;
    }else {camFollowValid=0;}
    if(index==7 && camTargetOn){
        camTestExpected=(unsigned long)(camTable+7*16);
        CamTargetCompute(*pCurTime);camTestEnabled=1;
    }else {CamTargetReset();}
    if(index==8 && cam360On){
        camTestExpected=(unsigned long)(camTable+8*16);
        Cam360Compute(*pCurTime);camTestEnabled=1;
    }
}
