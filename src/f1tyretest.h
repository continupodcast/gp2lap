#ifndef F1TYRETEST_H
#define F1TYRETEST_H
/* Read-only raw telemetry. No claim that GP2Car.grip is the compound multiplier. */
/* Resolve table operands only after verifying the native reader instructions.
   Reads only; no code hooks and no engine/set-state writes. */
static unsigned char *tyreSetIndex=0;
static unsigned long *tyreSetWear=0,*tyreWearCoeff=0,*tyreGripCoeff=0;
static int tyreTablesReady=0,tyreTablesAttempted=0;
static void F1TyreTablesInit(void)
{
 unsigned char *a,*b,*c,*d;
 if(tyreTablesAttempted)return;tyreTablesAttempted=1;
 a=IDAtoFlat(0x30421UL);b=IDAtoFlat(0x3042BUL);
 c=IDAtoFlat(0x30442UL);d=IDAtoFlat(0x304A1UL);
 if(!a||!b||!c||!d || memcmp(a,"\x0f\xb6\x90",3) ||
    memcmp(b,"\x8b\xac\x90",3) || memcmp(c,"\x8b\x04\x9d",3) ||
    memcmp(d,"\x8b\x04\x9d",3))return;
 tyreSetIndex=(unsigned char *)IDACodeReftoDataRef(0x30424UL);
 tyreSetWear=(unsigned long *)IDACodeReftoDataRef(0x3042EUL);
 tyreWearCoeff=(unsigned long *)IDACodeReftoDataRef(0x30445UL);
 tyreGripCoeff=(unsigned long *)IDACodeReftoDataRef(0x304A4UL);
 tyreTablesReady=tyreSetIndex && tyreSetWear && tyreWearCoeff && tyreGripCoeff;
}
#include "f1tyrelaps.h"
static FILE *tyreRawFile=0;
static char tyreRawBuffer[32768];
static unsigned long tyreRawFlushAt;
static int tyreRawExitRegistered;
static void F1TyreRawClose(void)
{
 if(tyreRawFile){fclose(tyreRawFile);tyreRawFile=0;}
}
static void F1TyreRawFlush(void)
{
 if(tyreRawFile)fflush(tyreRawFile);
}
static void F1TyreTestTick(void)
{
 static unsigned long last,session;static int ready,mode=-1;
 unsigned long now,start,cumulative,wc,gc;int i,n,m,reset,id,compound,set;FILE *f;GP2Car *c;
 if(!TyreDiagnostics || !pCarStructs || !pCurTime || !pSessionMode || !pNumCars){F1TyreRawClose();F1TyreLapsReset();return;}
 if(pIsReplay && *pIsReplay){F1TyreRawClose();F1TyreLapsReset();return;}
 if(pIsAccTime && *pIsAccTime){F1TyreRawClose();F1TyreLapsReset();return;}
 F1TyreTablesInit();
 n=*pNumCars;if(n<1 || n>28)return;if(n>26)n=26;
 now=*pCurTime;start=pSesStartTime?*pSesStartTime:0;m=*pSessionMode;
 reset=mode!=m || session!=start || now<last;
 if(reset)F1TyreLapsReset();
 if(pPaused && *pPaused)F1TyreRawFlush();
 if(ready && !reset && now-last<1000)return;
 if(!tyreRawFile){
  tyreRawFile=fopen("F1TYTEST.CSV",ready?"a":"w");if(!tyreRawFile)return;
  setvbuf(tyreRawFile,tyreRawBuffer,_IOFBF,sizeof(tyreRawBuffer));
  if(!tyreRawExitRegistered){atexit(F1TyreRawClose);tyreRawExitRegistered=1;}
 }
 f=tyreRawFile;
 if(!ready)fputs("time_ms,session_start,mode,id,raw_id,management,test_compound,compound,lap,last_lap_ms,pit_done,speed_raw,fuel_raw,grip_raw,grip_factor_raw,wear0_raw,wear1_raw,wear2_raw,wear3_raw,flags7c,flags90,slot,tables_valid,set_index,set_wear_raw,wear_coefficient_raw,grip_coefficient_raw\n",f);
 for(i=0;i<n;i++){
  c=&pCarStructs[i];if((c->id&63)<1 || (c->id&63)>40)continue;
  /* Retired stationary cars no longer contribute repetitive CSV rows. */
  if((c->flags_90&0x80) && !c->speed){tyreLaps[c->id&63].n=0;tyreLaps[c->id&63].bad|=64;continue;}
  id=c->id&63;
  compound=F1AssignedCompound(PerCarTyreCompounds?c->id:0x81,pCarSetups,(const GP2CSx *)pPlayerSetup);
  set=tyreTablesReady?(int)tyreSetIndex[id-1]:-1;
  cumulative=0;wc=0;gc=0;
  if(tyreTablesReady && set>=0 && set<=6)cumulative=tyreSetWear[(id-1)*8+set];
  if(tyreTablesReady && compound>=0 && compound<4){wc=tyreWearCoeff[compound];gc=tyreGripCoeff[compound];}
  F1TyreLapSample(i,c,now,set,compound,cumulative,tyreTablesReady);
  fprintf(f,"%lu,%lu,%u,%u,%u,%lu,%lu,%d,%u,%lu,%u,%u,%lu,%u,%u,%lu,%lu,%lu,%lu,%u,%u,%d,%d,%d,%lu,%lu,%lu\n",
   now,start,(unsigned)m,(unsigned)(c->id&63),(unsigned)c->id,PerCarTyreCompounds,TyreTestCompound,
   F1AssignedCompound(PerCarTyreCompounds?c->id:0x81,pCarSetups,(const GP2CSx *)pPlayerSetup),(unsigned)c->lapNr,c->timeLast,
   (unsigned)c->numPitStopsDone,(unsigned)c->speed,c->fuelLoad,(unsigned)c->grip,(unsigned)c->gripFactor,
   c->tyreWear[0],c->tyreWear[1],c->tyreWear[2],c->tyreWear[3],(unsigned)c->flags_7C,(unsigned)c->flags_90,i,tyreTablesReady,set,cumulative,wc,gc);
 }
 if(reset || now<tyreRawFlushAt || now-tyreRawFlushAt>=5000UL){F1TyreRawFlush();tyreRawFlushAt=now;}
 ready=1;last=now;session=start;mode=m;
}
#endif
