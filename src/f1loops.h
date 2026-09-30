/* Passage clocks at 30 fixed circuit points, independent per-car publication. */
#ifndef F1LOOPS_H
#define F1LOOPS_H
#include <string.h>
#ifndef FI_TRACE
static void FINoTrace(unsigned long clock,const char *format,...) {(void)clock;(void)format;}
#define FI_TRACE FINoTrace
#endif
#define FI_HISTORY 120
typedef struct {long key;double time;int valid;} FIPassage;
typedef struct {FIPassage passage[FI_HISTORY];double progress;unsigned long clock,generation;long events[4];int count,seen,lap,pit,pitHold,afterPit;} FICar;
typedef struct {int ahead,count,next,pitPending;unsigned long own,front;long values[6],display,precise,last;} FIBucket;
typedef struct {long display,precise;unsigned long clock,own,front;int valid,pitPending;} FIPairCache;
static FICar fiCars[41];
static FIBucket fiBucket[41][2];
static FIPairCache fiPairs[41][41];
static int fiPosition[41],fiDirection[41];
static unsigned long fiChanged[41];
static void FIReset(void)
{
 int i,j;
 for(i=1;i<=40;i++)if(fiCars[i].seen){FI_TRACE(fiCars[i].clock,"RESET_STATE active_id=%d",i);break;}
 memset(fiCars,0,sizeof(fiCars));memset(fiBucket,0,sizeof(fiBucket));
 memset(fiPairs,0,sizeof(fiPairs));
 memset(fiPosition,0,sizeof(fiPosition));memset(fiDirection,0,sizeof(fiDirection));
 for(i=0;i<41;i++)for(j=0;j<2;j++){fiBucket[i][j].display=-1;fiBucket[i][j].precise=-1;fiBucket[i][j].last=-1;}
}
static void FIInvalidate(FICar *c)
{
 if(c->seen)c->generation++;
 c->seen=0;c->count=0;c->pitHold=0;c->afterPit=0;memset(c->passage,0,sizeof(c->passage));
}
static void FISample(int id,int lap,double position,unsigned long clock,int valid)
{
 FICar *c=&fiCars[id];double current=(lap-1)*30.0+position,t;long k;FIPassage *p;c->count=0;c->lap=lap;
 if(!valid || lap<1 || position<0 || position>=30){
  if(c->seen)FI_TRACE(clock,"SAMPLE_INVALID id=%d lap=%d pos=%.6f valid=%d previous=%.6f previous_clock=%lu",id,lap,position,valid,c->progress,c->clock);
  FIInvalidate(c);return;
 }
 /* Ignore sub-resolution reversal, without advancing the interpolation
    anchor. Sustained/real backwards travel still trips the original check. */
 if(c->seen && clock>=c->clock && clock-c->clock<=500 && current<c->progress && c->progress-current<=0.0001) {
  FI_TRACE(clock,"POSITION_JITTER id=%d delta=%.8f anchor_clock=%lu",id,current-c->progress,c->clock);
  return;
 }
 if(c->seen && ((long)(current/30)!=(long)(c->progress/30) || position<c->progress-(long)(c->progress/30)*30-15))
  FI_TRACE(clock,"FINISH_TRANSITION id=%d lap=%d pos=%.6f previous=%.6f current=%.6f dt=%lu",id,lap,position,c->progress,current,clock-c->clock);
 if(c->seen && (clock<c->clock || clock-c->clock>500 || current<c->progress || current-c->progress>3)) {
  FI_TRACE(clock,"SAMPLE_RESET id=%d lap=%d pos=%.6f previous=%.6f delta=%.6f dt=%lu clock_back=%d timeout=%d backwards=%d jump=%d",id,lap,position,c->progress,current-c->progress,clock-c->clock,clock<c->clock,clock>=c->clock && clock-c->clock>500,current<c->progress,current-c->progress>3);
  FIInvalidate(c);
 }
 if(c->seen && current>c->progress && clock>c->clock)for(k=(long)c->progress+1;k<=current;k++) {
  t=c->clock+(clock-c->clock)*(k-c->progress)/(current-c->progress);
  p=&c->passage[k%FI_HISTORY];p->key=k;p->time=t;p->valid=1;c->events[c->count++]=k;
  if(c->pitHold && ++c->afterPit>=6)c->pitHold=0;
  FI_TRACE(clock,"CROSS id=%d lap=%d key=%ld point=%ld cross=%.3f generation=%lu",id,lap,k,k%30,t,c->generation);
 }
 c->seen=1;c->progress=current;c->clock=clock;
}
/* Pit lane is a different path. Preserve completed passages and displayed
   comparisons, but never interpolate across the unseen pit-lane journey. */
static void FIPitSample(int id,int lap,double position,unsigned long clock,int valid,int pit)
{
 FICar *c=&fiCars[id];int j;c->lap=lap;
 if(!valid)for(j=0;j<41;j++){fiPairs[id][j].valid=0;fiPairs[j][id].valid=0;}
 /* The pit approach leaves the main-track geometry before IN PIT is set.
    Suspend interpolation as soon as geometry is unavailable; do not call
    this a pit stop or manufacture passage times. Require a recent anchor. */
 if(valid && lap>=1 && position<0 && (c->seen || c->pitHold) &&
    clock>=c->clock && clock-c->clock<=500 && !pit) {
  if(!c->pitHold)FI_TRACE(clock,"ROUTE_HOLD id=%d lap=%d generation=%lu",id,lap,c->generation);
  for(j=0;j<41;j++){fiPairs[id][j].pitPending=1;fiPairs[j][id].pitPending=1;}
  c->pitHold=1;c->afterPit=0;c->seen=0;c->count=0;c->clock=clock;return;
 }
 if(valid && pit) {
  for(j=0;j<41;j++){fiPairs[id][j].pitPending=1;fiPairs[j][id].pitPending=1;}
  if(!c->pit)FI_TRACE(clock,"PIT_HOLD id=%d lap=%d generation=%lu",id,lap,c->generation);
  c->pit=1;c->pitHold=1;c->afterPit=0;c->seen=0;c->count=0;c->clock=clock;return;
 }
 if(c->pit) {c->pit=0;FI_TRACE(clock,"PIT_REJOIN id=%d lap=%d",id,lap);}
 if(valid && c->pitHold && position<0) {
  c->seen=0;c->count=0;c->clock=clock;return;
 }
 FISample(id,lap,position,clock,valid);
}
static void FIClearBucket(FIBucket *b,int keepDisplay)
{
 b->count=b->next=0;b->last=-1;
 if(!keepDisplay){b->display=-1;b->precise=-1;}
}
static void FIAddGap(FIBucket *b,long key,long gap)
{
 b->last=key;b->values[b->next]=gap;b->next=(b->next+1)%6;
 if(b->count<6)b->count++;
}
static void FIPublishBucket(int id,int channel,unsigned long clock)
{
 FIBucket *b=&fiBucket[id][channel];long sum=0;int j;
 if(fiCars[id].lap<2 || !b->count)return;
 for(j=0;j<b->count;j++)sum+=b->values[j];
 b->precise=(sum+b->count/2)/b->count;
 b->display=((b->precise+50)/100)*100;
 if(b->ahead>0 && b->ahead<=40) {
  FIPairCache *p=&fiPairs[id][b->ahead];
  p->display=b->display;p->precise=b->precise;p->clock=clock;p->own=fiCars[id].generation;
  p->front=fiCars[b->ahead].generation;p->valid=1;
  p->pitPending=b->pitPending || fiCars[id].pitHold || fiCars[b->ahead].pitHold;
 }
 FI_TRACE(clock,"PUBLISH id=%d channel=%d ahead=%d key=%ld display=%ld count=%d",id,channel,b->ahead,b->last,b->display,b->count);
}
/* Rebuild only from common passages belonging to the NEW pair. Never move
   an interval measured against the old opponent into the new comparison. */
static void FIRebase(int id,int ahead,int channel,unsigned long clock)
{
 FICar *a=&fiCars[id],*f=&fiCars[ahead];FIBucket *b=&fiBucket[id][channel];
 long key,keys[6],gaps[6];int n=0,j;FIPassage *p,*q;
 key=(long)a->progress;
 for(j=0;j<6 && key>0;j++,key--) {
  p=&a->passage[key%FI_HISTORY];q=&f->passage[key%FI_HISTORY];
  if(!p->valid || !q->valid || p->key!=key || q->key!=key || q->time>p->time)continue;
  keys[n]=key;gaps[n++]=(long)(p->time-q->time+0.5);
 }
 for(j=n-1;j>=0;j--)FIAddGap(b,keys[j],gaps[j]);
 FIPublishBucket(id,channel,clock);
 FI_TRACE(clock,"PAIR_REBASE id=%d channel=%d ahead=%d count=%d display=%ld",id,channel,ahead,b->count,b->display);
}
static void FICollect(int id,int ahead,int channel,unsigned long clock)
{
 FIBucket *b=&fiBucket[id][channel];FICar *a=&fiCars[id],*f;
 FIPassage *own,*front;int i,changed,hold,partial;long key,gap;
 if(ahead<1 || ahead==id){FIClearBucket(b,0);b->ahead=0;b->pitPending=0;return;}
 f=&fiCars[ahead];changed=b->ahead!=ahead;hold=a->pitHold || f->pitHold;
 if(changed || b->own!=a->generation || b->front!=f->generation) {
  FI_TRACE(clock,"REFERENCE id=%d channel=%d ahead=%d old=%d changed=%d",id,channel,ahead,b->ahead,changed);
  FIClearBucket(b,0);b->pitPending=hold;
  b->ahead=ahead;b->own=a->generation;b->front=f->generation;
  FIRebase(id,ahead,channel,clock);
  if(b->display<0) {
   FIPairCache *p=&fiPairs[id][ahead];
   if((hold || p->pitPending) && p->valid && clock>=p->clock && p->own==a->generation && p->front==f->generation) {
    b->display=p->display;b->precise=p->precise;b->pitPending=1;
    FI_TRACE(clock,"PAIR_RESTORE id=%d channel=%d ahead=%d display=%ld",id,channel,ahead,b->display);
   }
  }
 }
 if(hold)b->pitPending=1;
 hold=hold || b->pitPending;
 if((!a->seen && !a->pitHold) || (!f->seen && !f->pitHold) ||
    clock<a->clock || clock-a->clock>500 || clock<f->clock || clock-f->clock>500) {
  FIClearBucket(b,0);b->pitPending=0;return;
 }
 /* Do not mix pre-stop samples into the post-stop mean. Keep only the
    display until actual common passages exist for this same pair. */
 if(a->pit){FIClearBucket(b,1);return;}
 for(i=0;i<a->count;i++) {
  key=a->events[i];
  if(key<=b->last)continue; /* Rebase may already include this tick's crossing. */
  own=&a->passage[key%FI_HISTORY];front=&f->passage[key%FI_HISTORY];
  if(!front->valid || front->key!=key || front->time>own->time) {
   FI_TRACE(clock,"MISSING_POINT id=%d channel=%d ahead=%d key=%ld pit_hold=%d",id,channel,ahead,key,hold);
   FIClearBucket(b,hold);
   if(hold)FIRebase(id,ahead,channel,clock);
   continue;
  }
  if(b->last>=0 && key!=b->last+1)FIClearBucket(b,hold);
  gap=(long)(own->time-front->time+0.5);partial=b->count<6;
  FIAddGap(b,key,gap);
  b->pitPending=0;
  FI_TRACE(clock,"MEASURE id=%d channel=%d ahead=%d key=%ld gap=%ld count=%d",id,channel,ahead,key,gap,b->count);
  if(partial || key%3==0)FIPublishBucket(id,channel,clock);
 }
}
static int FIPosition(int id,int pos,int lap,unsigned long clock,unsigned long duration)
{
 if(lap<2){fiPosition[id]=pos;fiDirection[id]=0;return 0;}
 if(fiPosition[id] && fiPosition[id]!=pos){fiDirection[id]=pos<fiPosition[id]?1:-1;fiChanged[id]=clock;}
 fiPosition[id]=pos;return clock-fiChanged[id]<duration?fiDirection[id]:0;
}
#endif
