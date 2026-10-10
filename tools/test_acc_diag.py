from pathlib import Path
import subprocess,tempfile
root=Path(__file__).resolve().parents[1]
s=(root/'src/f1advanced.c').read_text();body=s[s.index('static unsigned long accCalls'):s.index('void F1AdvancedReport(void)')]
shim='''#include <stdio.h>
#include <string.h>
#include "f1micro.h"
typedef struct {unsigned char id,lapNr,flags_90,flags_16A;void *pSeg;unsigned short segDistFactor;unsigned long timeLast,timeLapStart;} GP2Car;
static F1MicroState micro;
static int ready=1,splitReady=1;static unsigned long resetCount;
static const char *sampleStatus="SAMPLING";
static unsigned long now=1000,start=0;static unsigned char track=1,mode=0x40,acc=0,replay=0,paused=0,count=1;
static unsigned long *pCurTime=&now,*pSesStartTime=&start;
static unsigned char *pTrackNr=&track,*pSessionMode=&mode,*pIsAccTime=&acc,*pIsReplay=&replay,*pPaused=&paused,*pNumCars=&count;
static GP2Car cars[1]={{34,2,0,0,0,100,92000,0}},*pCarStructs=cars;
'''
main='''
int main(void){F1MicroState before;
micro.best[0]=321;micro.cars[34].personal[0]=321;micro.cars[34].colors[0]=3;before=micro;
F1AccEvent("EOF");now=1100;F1AccEvent("EOF");acc=1;now=1200;F1AccEvent("EOF");
now=10000;cars[0].lapNr=3;F1AccEvent("LAP");F1AccEvent("RESET");
return memcmp(&before,&micro,sizeof(micro))!=0;}
'''
with tempfile.TemporaryDirectory() as td:
 td=Path(td);(td/'test.c').write_text(shim+body+main)
 subprocess.run(['gcc','-std=c89','-Wall','-Wextra','-Werror','-I'+str(root/'src'),str(td/'test.c'),'-o',str(td/'test')],check=True)
 subprocess.run([str(td/'test')],cwd=td,check=True)
 log=(td/'F1ACC.LOG').read_text();assert log.count('EVENT=EOF')==2
 assert 'acc=1' in log and 'EVENT=LAP' in log and 'EVENT=RESET' in log
 assert 'personal=1 colours=1' in log and 'records=1' in log
print('PASS: telemetry does not modify microsector state; mode changes, lap/reset events, record counts and EOF throttling recorded.')
