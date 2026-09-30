#include <stdio.h>
#include <string.h>
#include <assert.h>
#define _GP2STRUC_H
typedef struct { unsigned char id,lapNr,splitNr;unsigned long timeBest,timeLast; } GP2Car;
struct gp2timetable {unsigned long car[40];};
static unsigned long clockValue,startValue,trackValue,startedValue;
static unsigned short countValue;
static unsigned char modeValue,replayValue,accValue,carIds[26],order[26];
static GP2Car cars[26];
static struct gp2timetable best;
static unsigned long *pCurTime=&clockValue,*pSesStartTime=&startValue,*pTrackNr=&trackValue,*pNumCars2=&startedValue;
static unsigned short *pNumCars=&countValue;
static unsigned char *pSessionMode=&modeValue,*pIsReplay=&replayValue,*pIsAccTime=&accValue,*pCarIDs=carIds,*pFastLapCars=order;
static GP2Car *pCarStructs=cars;
struct gp2timetable *GP2_BestLaptimes=&best;
#include "../src/f1grid.inc"
int main(void)
{
    int i;FILE *f;char text[16000];size_t n;
    for(i=0;i<26;i++){cars[i].id=i+1;carIds[i]=i+1;order[i]=26-i;}
    best.car[39]=90123;countValue=28;startedValue=28;modeValue=0x40;
    F1GridBoot();F1GridUpdate();assert(gridCount==28);
    clockValue=500;F1GridUpdate();assert(gridDump==0);
    clockValue=1000;cars[0].id=40;F1GridUpdate();assert(gridDump==1000);
    clockValue=11000;F1GridUpdate();assert(gridDump==11000);
    modeValue=0x80;countValue=26;startedValue=26;F1GridUpdate();assert(gridMode==0x80&&gridCount==26);
    clockValue=1;F1GridUpdate();assert(gridDump==1);
    countValue=500;F1GridUpdate();assert(gridCount==500);
    f=fopen("F1GRID.LOG","r");assert(f);n=fread(text,1,sizeof(text)-1,f);text[n]=0;fclose(f);
    assert(strstr(text,"ACTIVE inspected=26 uninspected=2"));
    assert(strstr(text,"40:90123"));assert(strstr(text,"mode=80 race=1 count=26 started=26"));
    assert(strstr(text,"ACTIVE inspected=0"));
    puts("PASS: 28-count bounded to 26 slots; 40 best times; periodic, slot, session and clock-reset logging; invalid count.");
    return 0;
}
