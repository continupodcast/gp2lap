/* Portable race timing core. All clocks are simulation milliseconds. */
#ifndef F1INTERVAL_H
#define F1INTERVAL_H
#include <string.h>
#define FI_HISTORY 2048
#define FI_WINDOW 64
typedef struct { long distance; unsigned long clock; } FIPoint;
typedef struct { FIPoint point[FI_HISTORY]; int head,count,lap; unsigned long generation; } FIHistory;
typedef struct { int ahead,count; unsigned long own,front; long values[FI_WINDOW],display; } FIBucket;
static FIHistory fiHistory[41];
static FIBucket fiBucket[41][2];
static int fiPosition[41],fiDirection[41],fiEnabled,fiStarted;
static unsigned long fiChanged[41],fiPublish;
static void FIReset(void)
{
    int i,j;
    memset(fiHistory,0,sizeof(fiHistory));memset(fiBucket,0,sizeof(fiBucket));
    memset(fiPosition,0,sizeof(fiPosition));memset(fiDirection,0,sizeof(fiDirection));
    fiEnabled=fiStarted=0;fiPublish=0;
    for(i=0;i<41;i++)for(j=0;j<2;j++)fiBucket[i][j].display=-1;
}
static void FISample(int id,int lap,long distance,unsigned long clock,int valid)
{
    FIHistory *h=&fiHistory[id];FIPoint *p;
    if(h->count) {
        p=&h->point[(h->head+FI_HISTORY-1)%FI_HISTORY];
        if(clock<p->clock || clock-p->clock>500 || distance<p->distance || lap<h->lap || lap>h->lap+1) {
            h->count=h->head=0;h->generation++;
        } else if(clock==p->clock && valid)return;
    }
    if(!valid || distance<0) {if(h->count)h->generation++;h->count=h->head=0;return;}
    p=&h->point[h->head];p->distance=distance;p->clock=clock;
    h->head=(h->head+1)%FI_HISTORY;if(h->count<FI_HISTORY)h->count++;h->lap=lap;
}
static long FIRaw(int id,int ahead,unsigned long clock)
{
    FIHistory *a=&fiHistory[id],*b=&fiHistory[ahead];FIPoint *p,*lo,*hi;
    int first,left,right,mid;long target;double crossing,raw;
    if(!a->count || b->count<2)return -1;
    p=&a->point[(a->head+FI_HISTORY-1)%FI_HISTORY];
    hi=&b->point[(b->head+FI_HISTORY-1)%FI_HISTORY];
    if(clock<p->clock || clock-p->clock>500 || clock<hi->clock || clock-hi->clock>500)return -1;
    target=p->distance;first=(b->head+FI_HISTORY-b->count)%FI_HISTORY;
    if(target<b->point[first].distance || target>hi->distance)return -1;
    left=0;right=b->count;
    while(left<right) {mid=(left+right)/2;if(b->point[(first+mid)%FI_HISTORY].distance<=target)left=mid+1;else right=mid;}
    lo=&b->point[(first+left-1)%FI_HISTORY];crossing=lo->clock;
    if(left<b->count && lo->distance!=target) {
        hi=&b->point[(first+left)%FI_HISTORY];
        crossing+=(double)(target-lo->distance)*(hi->clock-lo->clock)/(hi->distance-lo->distance);
    }
    raw=p->clock-crossing;return raw>=0 && raw<=3600000?(long)(raw+0.5):-1;
}
static void FICollect(int id,int ahead,int channel,unsigned long clock)
{
    FIBucket *b=&fiBucket[id][channel];long raw;
    if(ahead<1 || id==ahead) {b->count=0;b->display=-1;b->ahead=0;return;}
    if(b->ahead!=ahead || b->own!=fiHistory[id].generation || b->front!=fiHistory[ahead].generation) {
        b->count=0;b->display=-1;
    }
    b->ahead=ahead;b->own=fiHistory[id].generation;b->front=fiHistory[ahead].generation;
    raw=FIRaw(id,ahead,clock);
    if(raw<0) {b->count=0;b->display=-1;return;}
    if(b->count<FI_WINDOW)b->values[b->count++]=raw;
}
static void FISort(long *v,int n)
{
    int i,j;long x;for(i=1;i<n;i++){x=v[i];j=i;while(j && v[j-1]>x){v[j]=v[j-1];j--;}v[j]=x;}
}
static long FIAverage(FIBucket *b)
{
    long v[FI_WINDOW],d[FI_WINDOW],median,limit,delta,sum=0;int i,n=0;
    if(b->count<10)return -1;
    memcpy(v,b->values,b->count*sizeof(long));FISort(v,b->count);median=v[b->count/2];
    for(i=0;i<b->count;i++){delta=v[i]-median;d[i]=delta<0?-delta:delta;}
    FISort(d,b->count);limit=3*d[b->count/2];if(limit<100)limit=100;
    for(i=0;i<b->count;i++){delta=v[i]-median;if(delta>=-limit && delta<=limit){sum+=v[i];n++;}}
    return ((sum/n+50)/100)*100;
}
static void FIPublish(unsigned long clock,int eligible,unsigned long period)
{
    int i,j;
    if(!fiStarted){fiStarted=1;fiPublish=clock;}
    if(eligible)fiEnabled=1;
    if(clock-fiPublish<period)return;
    fiPublish=clock;
    for(i=1;i<=40;i++)for(j=0;j<2;j++) {
        fiBucket[i][j].display=fiEnabled?FIAverage(&fiBucket[i][j]):-1;
        fiBucket[i][j].count=0;
    }
}
static int FIPosition(int id,int pos,unsigned long clock,unsigned long duration)
{
    if(fiPosition[id] && fiPosition[id]!=pos) {
        fiDirection[id]=pos<fiPosition[id]?1:-1;fiChanged[id]=clock;
    }
    fiPosition[id]=pos;
    return clock-fiChanged[id]<duration?fiDirection[id]:0;
}
#endif
