"""Exercise the unmodified adapter body with host track and car fixtures."""
from pathlib import Path
import tempfile, subprocess
root=Path(__file__).resolve().parents[1]
source=(root/'src/f1advanced.c').read_text()
body='\n'.join(line for line in source.splitlines() if not line.startswith('#include') and not line.startswith('typedef char'))
prefix=r'''
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "f1micro.h"
#include "f1qualy.h"
#define GP2_SEG_FACTOR 0.0381
typedef struct {short xPos,yPos;unsigned char fl_split;int width,width_60;} GP2Seg;
typedef struct {GP2Seg *pSeg;int id,lapNr,flags_16A,flags_90;unsigned short segDistFactor,segPosX;} GP2Car;
static GP2Seg trackData[60],*pTrackSegs=trackData;
static GP2Car carData[26],*pCarStructs=carData;
static unsigned long tick=1000,session=1,track=1;
static unsigned long *pCurTime=&tick,*pSesStartTime=&session,*pTrackNr=&track;
static int modeValue=0x40,replay,paused,num=1,nseg=60,outlap;
static int *pSessionMode=&modeValue,*pIsReplay=&replay,*pPaused=&paused,*pNumCars=&num,*pNumTrackSegs=&nseg;
static int f1_micro_enabled=1;
int F1QGetCard(int id,unsigned long now,F1QCard *card) {(void)id;(void)now;card->outlap=outlap;return 1;}
void F1RenderMicro(unsigned char *dst,const unsigned char *pal,const unsigned char *colors,int cockpit) {(void)dst;(void)pal;(void)colors;(void)cockpit;}
'''
main=r'''
static void advance(int seg) {
 GP2Car before;
 carData[0].pSeg=&trackData[seg];tick+=100;before=carData[0];
 F1AdvancedUpdate();assert(!memcmp(&before,&carData[0],sizeof(before)));
}
static void checkTracing(void) {
 F1MicroState expected;int i;
 double pos[11]={0.8,1.2,1.8,2.2,2.1,2.4,2.6,2.8,2.9,3.0,3.1};
 unsigned long times[11]={0,100,200,300,400,1000,1100,1100,1200,1300,1400};
 F1MicroReset(&micro);F1MicroReset(&expected);F1AdvancedReport();
 for(i=0;i<11;i++) {
  int lap=i==10?5:0,valid=i!=8;
  F1MicroSample(&expected,2,lap,pos[i],times[i],valid);
  tracedSample(2,lap,pos[i],times[i],valid,valid?D_OK:D_DUP);
  assert(!memcmp(&micro,&expected,sizeof(micro)));
 }
 assert(microDiag[2].counts[D_BACK]==1 && microDiag[2].counts[D_GAP]==1);
 assert(microDiag[2].counts[D_SAME]==1 && microDiag[2].counts[D_DUP]==1);
 assert(microDiag[2].counts[D_LAP]==1 && microDiag[2].first[D_BACK].clock==400);
 reportCars();
 F1MicroReset(&micro);
}
int main(void) {
 int i;unsigned long old;
 checkTracing();
 for(i=0;i<60;i++) {
  trackData[i].xPos=(short)(1000*cos(i*6.283185307179586/60));
  trackData[i].yPos=(short)(1000*sin(i*6.283185307179586/60));
  trackData[i].width=trackData[i].width_60=1472;
 }
 trackData[20].fl_split=1;trackData[40].fl_split=2;
 carData[0].segPosX=(unsigned short)-888; /* supplied log: valid lateral position, rejected by old doubled test */
 carData[0].id=138;carData[0].lapNr=1;carData[0].flags_90=128; /* show fuel must not reject samples */
 for(i=0;i<12;i++)advance(i);
 assert(ready && splitReady && micro.cars[10].personal[2]>0);
 F1MicroMode=1;F1AdvancedReport();F1AdvancedCard(NULL,NULL,10,0);
 carData[0].segPosX=1473;advance(11);assert(micro.cars[10].seen);
 carData[0].segPosX=1472;advance(11);assert(micro.cars[10].seen);
 carData[0].segPosX=(unsigned short)-1473;advance(11);assert(micro.cars[10].seen);
 carData[0].segPosX=(unsigned short)-1472;advance(11);assert(micro.cars[10].seen);
 old=micro.cars[10].clock;paused=1;advance(12);assert(micro.cars[10].clock==old);paused=0;
 carData[0].flags_16A=8;advance(13);assert(!micro.cars[10].seen && micro.cars[10].colors[2]);
 carData[0].flags_16A=0;outlap=1;advance(14);assert(!micro.cars[10].seen);outlap=0;
 session++;advance(15);assert(!micro.best[2]);
 for(i=16;i<22;i++)advance(i);
 assert(micro.cars[10].haveCross);
 replay=1;advance(23);assert(!ready && !micro.cars[10].seen);replay=0;
 advance(24);track++;advance(25);assert(!micro.best[11]);
 /* Multiple slots for one CarId: inactive before active and stale after.
    Brief unusable fractions must not destroy otherwise continuous laps. */
 session++;num=3;memset(carData,0,sizeof(carData));
 carData[0].id=carData[1].id=carData[2].id=22;
 carData[0].flags_16A=8;carData[2].flags_16A=8;
 for(i=0;i<=180;i++) {
  GP2Car snapshot[3];
  carData[1].lapNr=i/60+1;carData[1].pSeg=&trackData[i%60];
  carData[1].segPosX=2000; /* lateral excursions retain longitudinal timing */
  carData[1].segDistFactor=i%11==4?65535:0;
  memcpy(snapshot,carData,sizeof(snapshot));tick+=100;F1AdvancedUpdate();
  assert(!memcmp(snapshot,carData,sizeof(snapshot)));
 }
 for(i=0;i<30;i++) assert(micro.cars[22].personal[i]>0);
 assert(microDiag[22].counts[D_DUP]>0);
 /* A long loss of telemetry still breaks continuity. */
 carData[1].segDistFactor=65535;tick+=600;F1AdvancedUpdate();
 carData[1].segDistFactor=0;tick+=100;F1AdvancedUpdate();
 assert(!micro.cars[22].haveCross);
 num=28;tick+=100;F1AdvancedUpdate();assert(ready && !strcmp(sampleStatus,"SAMPLING"));num=3;
 nseg=2049;F1AdvancedUpdate();assert(!ready);nseg=60;
 trackData[40].fl_split=0;F1AdvancedUpdate();assert(ready && !splitReady);
 return 0;
}
'''
with tempfile.TemporaryDirectory() as tmp:
    tmp=Path(tmp);c=tmp/'adapter.c';c.write_text(prefix+body+main)
    subprocess.run(['gcc','-std=c89','-Wall','-Wextra','-Werror','-I'+str(root/'src'),str(c),str(root/'src/f1micro.c'),'-lm','-o',str(tmp/'adapter')],check=True)
    subprocess.run([str(tmp/'adapter')],check=True,cwd=tmp)
    log=(tmp/'F1HUD.LOG').read_text()
    assert 'MICRO BREAK: id=2 reason=BACKWARD' in log and 'reason=DUPLICATE' in log
    assert 'status=SAMPLING' in log and 'MICRO RAW: id=138' in log and 'flags90=128' in log and 'coloured=0/30' not in log
print('PASS: native adapter geometry, CarId masking, pause, pits/outlap, session/track resets, replay, malformed geometry and no car-memory writes.')
