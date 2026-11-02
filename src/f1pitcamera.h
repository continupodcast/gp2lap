#include "f1modules.h"
#ifndef F1PITCAMERA_H
#define F1PITCAMERA_H
#define F1_PIT_CAMERA_TRACE 0
/* P4: conditional camera-only checks; never modify car flags. */
static unsigned char *pitCamView,*pitCamRequests,*pitCamKeys1,*pitCamKeys2;
static unsigned long *pitCamMode;
static unsigned int pitCamRecords;
static char pitCamPrevious[256];
static int pitCamReady;
void __cdecl F1PitCameraChangeWrapped(void);
void (*fpF1PitCameraChangeOriginal)(void);

static int __cdecl PitCamBlocked(GP2Car *c)
{
    if(!c || !(c->flags_23&0x20))return 0;
    /* flags23:40 is player-specific. Native pit flag AD:04 also covers AI. */
    return !((c->flags_AD&0x04) && !(c->flags_90&0x20));
}
int (__cdecl *fpF1PitCamBlocked)(GP2Car *)=PitCamBlocked;
void __cdecl F1PitCamAutoCheck(void);
void __cdecl F1PitCamTVCheck(void);
void __cdecl F1PitCamRearCheck(void);
void __cdecl F1PitCamOtherCheck(void);
void __cdecl F1PitCamApplyCheck(void);
void *f1PitCamAutoTarget,*f1PitCamRejectTarget,*f1PitCamApplyTarget;

static void PitCamTrace(const char *event,unsigned long caller,int force)
{
    GP2Car *c;
    char state[256];FILE *f;unsigned long clock;
    if(!F1_PIT_CAMERA_TRACE || !pitCamReady || pitCamRecords>=5000)return;
    c=ppSelectedCS?*ppSelectedCS:NULL;
    clock=pCurTime?*pCurTime:0;
    sprintf(state,"id=%u view=%02X requests=%02X mode=%lu keys1=%02X keys2=%02X flags23=%02X flags90=%02X flagsAD=%02X flags16A=%02X cockpit_id=%u blocked20=%u",
        c?(unsigned)(c->id&63):0,(unsigned)*pitCamView,(unsigned)*pitCamRequests,*pitCamMode,
        (unsigned)*pitCamKeys1,(unsigned)*pitCamKeys2,c?(unsigned)c->flags_23:0,
        c?(unsigned)c->flags_90:0,c?(unsigned)c->flags_AD:0,c?(unsigned)c->flags_16A:0,
        ppCockpitCS && *ppCockpitCS?(unsigned)((*ppCockpitCS)->id&63):0,(unsigned)PitCamBlocked(c));
    if(!force && !strcmp(state,pitCamPrevious))return;
    strcpy(pitCamPrevious,state);
    f=((FILE *)0) /* obsolete diagnostic disabled in T4 */;if(!f)return;
    fprintf(f,"%s clock=%lu caller=%05lx %s\n",event,clock,caller,state);
    fclose(f);pitCamRecords++;
}
static void __cdecl PitCamBefore(unsigned long ret)
{
    PitCamTrace("BEFORE",ret-(unsigned long)IDAtoFlat(0x10000)+0x10000,1);
}
static void __cdecl PitCamAfter(unsigned long ret)
{
    PitCamTrace("AFTER",ret-(unsigned long)IDAtoFlat(0x10000)+0x10000,1);
}
void (__cdecl *fpF1PitCamBefore)(unsigned long)=PitCamBefore;
void (__cdecl *fpF1PitCamAfter)(unsigned long)=PitCamAfter;
static void F1PitCameraSample(void){PitCamTrace("FRAME",0,0);}
static void F1PitCameraKey(unsigned int scan)
{
    if(scan>=0x47 && scan<=0x53){char event[40];sprintf(event,"KEY scan=%u",scan);PitCamTrace(event,0,1);}
}
static int F1PitCameraProbeInit(void)
{
#if F1_MODULE_CAMERA_CONTROLS
#include "../modules/camera-controls/f1pitcamera_F1PitCameraProbeInit.inc"
#else
return 0;
#endif
}
#endif
