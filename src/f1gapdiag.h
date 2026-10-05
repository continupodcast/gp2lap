/* Evaluation build only. Buffered DOS writes; no timing algorithm changes. */
#include <stdarg.h>
static char gapDiagBuffer[16384];
static unsigned int gapDiagUsed;
static unsigned long gapDiagLines,gapDiagFlushAt;
static int gapDiagActive;
static void GapDiagFlush(unsigned long clock)
{
 FILE *f;
 if(gapDiagUsed){f=fopen("F1GAPS.LOG","ab");if(f){fwrite(gapDiagBuffer,1,gapDiagUsed,f);fclose(f);}gapDiagUsed=0;}
 gapDiagFlushAt=clock;
}
static void GapDiagBoot(void)
{
 FILE *f=fopen("F1GAPS.LOG","wb");
 gapDiagUsed=0;gapDiagLines=0;gapDiagFlushAt=0;gapDiagActive=f!=NULL;
 if(f){fputs("F1 HUD 0.35 - early route hold; partial common-point means; millisecond precision\nchannel 0=preceding car, 1=leader; clocks/gaps in ms; key=(lap-1)*30+point; point 0=finish\nROUTE_HOLD=geometry unavailable before IN PIT; PIT_HOLD/PIT_REJOIN=pit continuity; PAIR_REBASE=new comparison; PUBLISH count=valid samples\nLimit: 100000 records. Pause before copying.\n",f);fclose(f);}
}
static void GapDiagTrace(unsigned long clock,const char *format,...)
{
 char line[1024];int n;va_list args;
 if(!gapDiagActive)return;
 n=sprintf(line,"t=%lu ",clock);va_start(args,format);vsprintf(line+n,format,args);va_end(args);
 strcat(line,"\n");n=strlen(line);
 if(gapDiagUsed+n>=sizeof(gapDiagBuffer))GapDiagFlush(clock);
 memcpy(gapDiagBuffer+gapDiagUsed,line,n);gapDiagUsed+=n;
 if(++gapDiagLines>=100000){GapDiagFlush(clock);gapDiagActive=0;
  {FILE *f=fopen("F1GAPS.LOG","ab");if(f){fputs("DIAGNOSTIC_LIMIT_REACHED: capture stopped, gameplay unchanged\n",f);fclose(f);}}
 }
}
#define FI_TRACE GapDiagTrace
