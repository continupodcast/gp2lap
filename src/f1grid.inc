/* Read-only roster probe. Never extend the known 26-slot car/order arrays.
   Best-lap storage is independently known to contain 40 CarId entries. */
#include "timing/gp2struc.h"
extern struct gp2timetable *GP2_BestLaptimes;
static unsigned long gridClock,gridDump,gridStart,gridTrack,gridStarted;
static unsigned gridMode,gridCount;
static unsigned char gridIds[26];
static int gridReady;
static void F1GridBoot(void)
{
    FILE *f=fopen("F1GRID.LOG","w");
    gridReady=0;
    if(f) {
        fputs("F1 GRID DIAG 0.21-D1\nRead-only; active/order reads capped at 26; best-lap table has 40 CarId entries.\nSnapshots on session/count/slot changes and every 10 simulation seconds.\n",f);
        fclose(f);
    }
}
static void F1GridUpdate(void)
{
    unsigned long now,start,track,started;
    unsigned mode,n;
    unsigned char ids[26];
    int i,limit,changed;
    FILE *f;
    if(!pCurTime || !pSessionMode || !pNumCars || !pCarStructs)return;
    now=*pCurTime;mode=*pSessionMode;n=*pNumCars;
    start=pSesStartTime?*pSesStartTime:0;
    track=pTrackNr?*pTrackNr:0;
    started=pNumCars2?*pNumCars2:0;
    changed=!gridReady || now<gridClock || start!=gridStart || track!=gridTrack ||
        mode!=gridMode || n!=gridCount || started!=gridStarted;
    if(!changed && now-gridClock<1000)return;
    gridClock=now;
    memset(ids,0,sizeof(ids));
    limit=(n>=1 && n<=28)?(n>26?26:(int)n):0;
    for(i=0;i<limit;i++)ids[i]=pCarStructs[i].id;
    if(memcmp(ids,gridIds,sizeof(ids)))changed=1;
    if(!changed && now-gridDump<10000)return;
    f=fopen("F1GRID.LOG","a");if(!f)return;
    fprintf(f,"\nSNAP clock=%lu start=%lu track=%lu mode=%02X race=%u count=%u started=%lu replay=%u accelerated=%u\n",
        now,start,track,mode,(mode&0x80)?1:0,n,started,
        pIsReplay?(unsigned)*pIsReplay:0,pIsAccTime?(unsigned)*pIsAccTime:0);
    fprintf(f,"POINTERS cars=%p ids=%p order=%p best=%p\n",
        (void *)pCarStructs,(void *)pCarIDs,(void *)pFastLapCars,(void *)GP2_BestLaptimes);
    fprintf(f,"ACTIVE inspected=%d uninspected=%u\n",limit,n>26?n-26:0);
    for(i=0;i<limit;i++) {
        GP2Car *c=&pCarStructs[i];
        fprintf(f,"SLOT %02d raw=%u id=%u lap=%u split=%u best=%lu last=%lu\n",
            i+1,(unsigned)c->id,(unsigned)(c->id&63),(unsigned)c->lapNr,
            (unsigned)c->splitNr,c->timeBest,c->timeLast);
    }
    fputs("CAR_IDS_FIRST26",f);
    if(pCarIDs)for(i=0;i<26;i++)fprintf(f," %u",(unsigned)pCarIDs[i]);
    else fputs(" UNAVAILABLE",f);
    fputs("\nFAST_ORDER_FIRST26",f);
    if(pFastLapCars)for(i=0;i<26;i++)fprintf(f," %u",(unsigned)pFastLapCars[i]);
    else fputs(" UNAVAILABLE",f);
    fputs("\nBEST_BY_CARID raw_values",f);
    if(GP2_BestLaptimes)for(i=0;i<40;i++)fprintf(f," %d:%lu",i+1,GP2_BestLaptimes->car[i]);
    else fputs(" UNAVAILABLE",f);
    fputc('\n',f);fclose(f);
    gridReady=1;gridDump=now;gridStart=start;gridTrack=track;
    gridMode=mode;gridCount=n;gridStarted=started;
    memcpy(gridIds,ids,sizeof(ids));
}
