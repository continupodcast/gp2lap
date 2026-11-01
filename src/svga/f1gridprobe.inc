/* THE GRID draw completion probe. No menu state writes. */
#include "../f1render.h"
#include "../gp2glob.h"
#include "../gp2str.h"
extern unsigned short *GP2_LapsInThisRace;
static unsigned char *gridMenuDepth,*gridMenuStack,*gridMenuPage;
static long gridMenuTrack;
static int gridMenuLaps;
static const char *gridMenuCountry,*gridMenuCircuit;
static unsigned char **gridSelectedObject;
static long *gridOriginX,*gridOriginY;
static unsigned char *gridSelectedIndex;
static int gridFocusRect[4]={-1,-1,-1,-1};
typedef struct {unsigned char *object;void *original;int rect[4],kind,index;} GridNativeControl;
static GridNativeControl gridControls[12];
static unsigned int gridControlCount;
static int gridControlsScanned;
void __cdecl F1GridControlWrapped(void);
static unsigned char *gridProbePalette;
static long *gridProbeScroll;
static unsigned long *gridProbeCount;
static unsigned char *gridProbeOrder,*gridProbeCanvas;
static unsigned long *gridProbeTimes;
static unsigned long gridProbeDraws,gridProbeRecords;
static long gridProbePrevious=-2147483647L;
void __cdecl F1GridProbeWrapped(void);
void (*fpF1GridProbeOriginal)(void);
void F1GridMenuPresent(void)
{
    unsigned char *page;unsigned int depth,n;long track;int laps;Gp2InfoTags *tags;
    const char *country,*circuit;
    if(!gridMenuDepth || !gridMenuStack || !gridProbeCanvas || !gridProbePalette)return;
    depth=*gridMenuDepth;if(depth<1 || depth>16)return;
    memcpy(&page,gridMenuStack+(depth-1)*40+4,4);
    if(page!=gridMenuPage)return;
    n=(unsigned)*gridProbeCount;if(n<1 || n>40)return;
    track=pTrackNr?*pTrackNr:-1;tags=track>=0 && track<16?GetTrackTags((unsigned char)track):NULL;
    country=tags && tags->country[0]?tags->country:GP2_GetCountryString(track);
    circuit=tags && tags->name[0]?tags->name:GP2_GetTrackString(track);
    laps=GP2_LapsInThisRace?*GP2_LapsInThisRace:0;
    if(laps<1 || laps>999)laps=0;
    gridMenuTrack=track;gridMenuLaps=laps;gridMenuCountry=country;gridMenuCircuit=circuit;
    F1RenderGridMenu(gridProbeCanvas,gridProbePalette,gridProbeOrder,gridProbeTimes,n,*gridProbeScroll,gridProbeScroll[1],country,circuit,laps,gridFocusRect);
}
static void __cdecl GridControlNoop(void) {}
static void *__cdecl GridControlOriginal(unsigned char *object)
{
    unsigned int i;for(i=0;i<gridControlCount;i++)if(gridControls[i].object==object)return gridControls[i].kind==8?(void *)GridControlNoop:gridControls[i].original;
    return (void *)GridControlNoop;
}
void *(__cdecl *fpF1GridControlOriginal)(unsigned char *)=GridControlOriginal;
static void __cdecl GridControlAfter(unsigned char *object)
{
    unsigned int i;
    for(i=0;i<gridControlCount;i++)if(gridControls[i].object==object) {
        if(gridControls[i].kind==8)return;
        F1RenderGridControl(gridProbeCanvas,gridProbePalette,gridControls[i].rect,gridControls[i].kind,
            gridSelectedIndex && *gridSelectedIndex==gridControls[i].index,*gridProbeScroll,gridProbeScroll[1]);return;
    }
}
void (__cdecl *fpF1GridControlAfter)(unsigned char *)=GridControlAfter;
static void GridControlsInstall(void)
{
    unsigned char *p,*next;void *fn;unsigned short r[4];int i,rect[4],w,h,kind;FILE *f;
    if(gridControlsScanned || !gridMenuPage || !gridOriginX || !gridOriginY)return;
    gridControlsScanned=1;gridControlCount=0;p=gridMenuPage;
    f=((FILE *)0) /* obsolete diagnostic disabled in T4 */;if(f)fprintf(f,"v0.63 native control draw replacement, origin=%ld,%ld\n",*gridOriginX,*gridOriginY);
    for(i=1;i<=64;i++) {
        memcpy(&next,p,4);p=next;
        if(!p || p==gridMenuPage || (unsigned long)p<(unsigned long)gridMenuPage-65536UL || (unsigned long)p>(unsigned long)gridMenuPage+65536UL)break;
        memcpy(r,p+4,8);rect[0]=r[0]+(int)*gridOriginX;rect[2]=r[2]+(int)*gridOriginX;
        rect[1]=r[1]+(int)*gridOriginY;rect[3]=r[3]+(int)*gridOriginY;
        w=rect[2]-rect[0]+1;h=rect[3]-rect[1]+1;
        memcpy(&fn,p+0x16,4);
        if(f)fprintf(f,"object=%d rect=%d,%d..%d,%d type=%u draw=%p\n",i,rect[0],rect[1],rect[2],rect[3],(unsigned)p[0xc],fn);
        if(rect[0]<0 || rect[1]<0 || rect[2]>=640 || rect[3]>=480 || w<1 || h<1 || !fn)continue;
        kind=-1;
        if(rect[1]>=420 && w>=60 && h<=50)kind=rect[0]<250?0:rect[0]<365?1:2;
        else if(rect[0]>=460 && rect[1]>=90 && rect[3]<440 && w<=40) {
            if(h>60)kind=7;
            else kind=rect[1]<125?3:rect[1]<150?4:rect[1]<405?5:6;
        }
        if(i==10 || i==11)kind=8; /* Native country/circuit text draw only. */
        if(kind>=0 && gridControlCount<12) {
            GridNativeControl *c=&gridControls[gridControlCount++];c->object=p;c->original=fn;c->kind=kind;c->index=i;memcpy(c->rect,rect,sizeof(rect));
            fn=(void *)F1GridControlWrapped;memcpy(p+0x16,&fn,4);
            if(f)fprintf(f,"HOOKED object=%d kind=%d\n",i,kind);
        }
    }
    if(f){fprintf(f,"controls=%u; native input/coordinates unchanged\n",gridControlCount);fclose(f);}
}
static void __cdecl GridProbeAfter(void)
{
    FILE *f;int i;unsigned id,n;long scroll;
    if(!gridProbeCanvas || !gridProbePalette || !gridProbeScroll || !gridProbeCount)return;
    scroll=*gridProbeScroll;n=(unsigned)*gridProbeCount;gridProbeDraws++;
    if(n>40)return;
    GridControlsInstall();
    F1GridMenuPresent();
    /* Paint every real control on the initial list draw, before cursor presentation. */
    for(i=0;i<(int)gridControlCount;i++)GridControlAfter(gridControls[i].object);
    if(scroll==gridProbePrevious || gridProbeRecords>=256)return;
    gridProbePrevious=scroll;gridProbeRecords++;
    f=((FILE *)0) /* obsolete diagnostic disabled in T4 */;if(!f)return;
    fprintf(f,"DRAW=%lu scroll=%ld count=%u track=%ld laps=%d country=%s circuit=%s\n",gridProbeDraws,scroll,n,gridMenuTrack,gridMenuLaps,gridMenuCountry?gridMenuCountry:"?",gridMenuCircuit?gridMenuCircuit:"?");
    for(i=0;i<(int)n;i++) {
        id=gridProbeOrder[i]&63;
        fprintf(f,"position=%d raw_id=%u CarId=%u combined_raw=%lu\n",i+1,(unsigned)gridProbeOrder[i],id,id>=1 && id<=40?gridProbeTimes[id-1]:0);
    }
    fclose(f);
}
void (__cdecl *fpF1GridProbeAfter)(void)=GridProbeAfter;
int F1GridProbeInit(void)
{
    unsigned char *p,*target;long relative;FILE *f;
    if(!GP2_CodeStartAdr)return 0;
    p=IDAtoFlat(0x83859);target=IDAtoFlat(0x7bdff);
    if(p[0]!=0xe8 || p+5+*(long *)(p+1)!=target || memcmp(p+5,"\x61\xc3",2))return 0;
    if(memcmp(IDAtoFlat(0x835ac),"\xa1",1) || IDAtoFlat(0x8368a)[0]!=0xbe ||
       memcmp(IDAtoFlat(0x835d9),"\x3b\x05",2) || memcmp(IDAtoFlat(0x83804),"\x81\xc6",2))return 0;
    gridProbeScroll=(long *)IDACodeReftoDataRef(0x835ad);
    gridProbeCount=(unsigned long *)IDACodeReftoDataRef(0x835db);
    gridProbeOrder=IDACodeReftoDataRef(0x8368b);
    gridProbeTimes=(unsigned long *)IDACodeReftoDataRef(0x83806);
    gridProbeCanvas=IDACodeReftoDataRef(0x7ad0b);
    if(memcmp(IDAtoFlat(0x87920),"\x80\x3d",2) || IDAtoFlat(0x8794e)[0]!=0x05 || IDAtoFlat(0x834df)[0]!=0xbe)return 0;
    gridMenuDepth=IDACodeReftoDataRef(0x87922);
    gridMenuStack=IDACodeReftoDataRef(0x8794f);
    gridMenuPage=IDACodeReftoDataRef(0x834e0);
    if(memcmp(IDAtoFlat(0x851cd),"\x8b\x1d",2) || memcmp(IDAtoFlat(0x812ac),"\x03\x1d",2) || memcmp(IDAtoFlat(0x812b2),"\x03\x05",2))return 0;
    gridSelectedObject=(unsigned char **)IDACodeReftoDataRef(0x851cf);
    gridOriginX=(long *)IDACodeReftoDataRef(0x812ae);
    gridOriginY=(long *)IDACodeReftoDataRef(0x812b4);
    if(memcmp(IDAtoFlat(0x85587),"\x0f\xb6\x0d",3))return 0;
    gridSelectedIndex=IDACodeReftoDataRef(0x8558a);
    if(IDAtoFlat(0x7f623)[0]!=0xba)return 0;
    gridProbePalette=IDACodeReftoDataRef(0x7f624);
    if(!gridProbeScroll || !gridProbeCount || !gridProbeOrder || !gridProbeTimes || !gridProbeCanvas)return 0;
    fpF1GridProbeOriginal=(void (*)(void))target;
    relative=(unsigned char *)F1GridProbeWrapped-(p+5);memcpy(p+1,&relative,4);
    f=((FILE *)0) /* obsolete diagnostic disabled in T4 */;if(f){fputs("GRID MENU v0.63: modern panel after native draw; original controls retained; scroll/grid snapshots\n",f);fclose(f);}
    return 1;
}
