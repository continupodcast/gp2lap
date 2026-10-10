/* Portable timing model, fed only with native GP2 snapshots. */
#include <string.h>
#include <stdio.h>
#include "f1qualy.h"
#include "f1splitcolor.h"
typedef struct {
    int seen,lap,split,pit,outlap,active,postColor[3],liveColor[3],pole;
    unsigned long start,postUntil,gapUntil;
    long best,ref[3],pb[3],lapRef[3],live[3],postValue[3],last,gap;
    int hasGap,gapLeader;
    long gapReferenceBest;
} State;
static State state[41];
static F1QRow rows[F1_Q_MAX];
static int nrows;
static long global[3];
static long valid(unsigned long t) { return t && !(t&0xf0000000UL) && t<=3600000UL?(long)t:0; }
static void minimum(long *a,long b) { if(b>0 && (!*a || b<*a)) *a=b; }
void F1QTime(char *s,long ms,int sector)
{
    if(ms<=0) { strcpy(s,"-"); return; }
    if(sector) sprintf(s,"%ld.%03ld",ms/1000,ms%1000);
    else sprintf(s,"%ld:%02ld.%03ld",ms/60000,(ms/1000)%60,ms%1000);
}
/* Floor the running clock to a tenth: never display time not yet elapsed. */
void F1QLiveTime(char *s,long ms)
{
    if(ms<=0) { strcpy(s,"-");return; }
    sprintf(s,"%ld:%02ld.%ld",ms/60000,(ms/1000)%60,(ms%1000)/100);
}
int F1QWindow(const F1QRow *rows,int count,int focus,int *start)
{
    int i,end;*start=0;
    for(i=0;i<count;i++) if(rows[i].id==focus) {
        *start=i>2?i-2:0;end=i+3<count?i+3:count;return end-*start;
    }
    return 0;
}
void F1QDelta(char *s,long ms)
{
    char sign=ms<0?'-':'+'; if(ms<0) ms=-ms;
    sprintf(s,"%c%ld.%03ld",sign,ms/1000,ms%1000);
}
void F1QReset(void) { memset(state,0,sizeof(state)); memset(global,0,sizeof(global)); nrows=0; }
long F1QGlobal(int i) { return i>=0 && i<3?global[i]:0; }
void F1QTick(const F1QCar *cars,int count,unsigned long clock)
{
    int i,j,k,id,cross,complete,oldout,leader=0,ids[41];
    long leaderBest=0,leaderRef[3]={0,0,0},best,b1,b2,s1,s2,last,parts[3];
    const F1QCar *c; State *s; F1QRow tmp;
    memset(ids,0,sizeof(ids));
    /* Freeze the reference leader before applying this frame's improvements. */
    for(i=1;i<=40;i++) if(state[i].best && (!leaderBest || state[i].best<leaderBest)) {
        leader=i;leaderBest=state[i].best;
    }
    if(leader) memcpy(leaderRef,state[leader].ref,sizeof(leaderRef));
    nrows=0;
    for(i=0;i<count && nrows<F1_Q_MAX;i++) {
        c=&cars[i];id=c->id;if(id<1 || id>40 || ids[id]) continue;ids[id]=1;s=&state[id];
        best=valid(c->best);b1=valid(c->best1);b2=valid(c->best2);
        s1=valid(c->s1);s2=valid(c->s2);last=valid(c->last);
        oldout=s->outlap;
        cross=s->seen && c->active && !c->pit && (c->lap!=s->lap || c->start!=s->start);
        complete=cross && c->split==2 && !oldout && last && s1 && s2>s1 && last>s2;
        if(!s->seen) { s->outlap=c->pit || (!last && c->lap<=1); memcpy(s->lapRef,s->pb,sizeof(s->pb)); }
        if(c->pit) { s->outlap=1;s->postUntil=0;s->gapUntil=0; }
        else if(s->pit && c->active) s->outlap=1;
        if(complete) {
            parts[0]=s1;parts[1]=s2-s1;parts[2]=last-s2;
            for(k=0;k<3;k++) {
                s->postValue[k]=parts[k];
                s->postColor[k]=F1SplitColor(parts[k],s->lapRef[k],global[k]);
                minimum(&s->pb[k],parts[k]);minimum(&global[k],parts[k]);
            }
            s->last=last;s->postUntil=clock+8000;s->pole=(!leaderBest || last<leaderBest);
            s->hasGap=leaderBest>0;s->gap=last-leaderBest;
            s->gapLeader=leader;s->gapReferenceBest=leaderBest;
        }
        if(cross) {
            s->outlap=0;s->gapUntil=0;memset(s->live,0,sizeof(s->live));
            memset(s->liveColor,0,sizeof(s->liveColor));memcpy(s->lapRef,s->pb,sizeof(s->pb));
        }
        /* Best-lap times come from GP2's persistent tables, including cars in pits. */
        if(best && (!s->best || best<=s->best)) {
            s->best=best;
            if(b1 && b2>b1 && best>b2) {
                s->ref[0]=b1;s->ref[1]=b2-b1;s->ref[2]=best-b2;
                for(k=0;k<3;k++) { minimum(&s->pb[k],s->ref[k]);minimum(&global[k],s->ref[k]); }
            }
        }
        if(!s->seen) memcpy(s->lapRef,s->pb,sizeof(s->pb));
        memset(s->live,0,sizeof(s->live));
        if(c->active && !c->pit && !s->outlap) {
            if(c->split==0 || c->split==1) s->live[0]=s1;
            if(c->split==1 && s1 && s2>s1) s->live[1]=s2-s1;
            for(k=0;k<2;k++) {
                s->liveColor[k]=F1SplitColor(s->live[k],s->lapRef[k],global[k]);
                minimum(&s->pb[k],s->live[k]);minimum(&global[k],s->live[k]);
            }
            if(s->seen && c->split!=s->split && (c->split==0 || c->split==1)) {
                s->hasGap=leaderRef[0]>0 && (c->split==0 || leaderRef[1]>0);
                s->gap=c->split==0?s1-leaderRef[0]:s2-leaderRef[0]-leaderRef[1];
                s->gapUntil=clock+8000;s->gapLeader=leader;s->gapReferenceBest=leaderBest;
            }
        }
        s->seen=1;s->active=c->active;s->pit=c->pit;s->lap=c->lap;s->split=c->split;s->start=c->start;
        rows[nrows].id=id;rows[nrows].pit=c->pit;rows[nrows].outlap=s->outlap;
        rows[nrows].best=s->best;memcpy(rows[nrows].pb,s->pb,sizeof(s->pb));
        memcpy(rows[nrows].live,s->live,sizeof(rows[nrows].live));
        memcpy(rows[nrows].color,s->liveColor,sizeof(rows[nrows].color));nrows++;
    }
    /* All cars must see this frame's final session records, independent
       of snapshot order. Retire historical purple marks immediately. */
    for(i=1;i<=40;i++) if(state[i].seen) {
        s=&state[i];
        for(k=0;k<3;k++) {
            if(s->postColor[k]==3 && global[k]>0 && s->postValue[k]>global[k])
                s->postColor[k]=1;
            if(k<2 && s->live[k]>0) {
                if(s->liveColor[k]==3 && global[k]>0 && s->live[k]>global[k])
                    s->liveColor[k]=1;
            }
        }
    }
    for(i=0;i<nrows;i++)
        memcpy(rows[i].color,state[rows[i].id].liveColor,sizeof(rows[i].color));
    for(i=1;i<nrows;i++) {
        tmp=rows[i];j=i;
        while(j>0 && tmp.best && (!rows[j-1].best || tmp.best<rows[j-1].best)) { rows[j]=rows[j-1];j--; }
        rows[j]=tmp;
    }
    for(i=0;i<nrows;i++) rows[i].pos=i+1;
}
const F1QRow *F1QRows(int *count) { *count=nrows;return rows; }
int F1QGetCard(int id,unsigned long clock,F1QCard *out)
{
    int i;State *s;
    memset(out,0,sizeof(*out));if(id<1 || id>40 || !state[id].seen) return 0;s=&state[id];
    for(i=0;i<nrows;i++) if(rows[i].id==id) break;
    if(i==nrows) return 0;
    out->id=id;out->pit=s->pit;out->outlap=s->outlap;
    for(i=0;i<nrows;i++) if(rows[i].id==id) out->pos=rows[i].best?i+1:0;
    if(nrows && rows[0].best) { out->leaderId=rows[0].id;out->leaderBest=rows[0].best; }
    out->post=s->postUntil>clock && !s->pit && s->active;
    out->pole=out->post && s->pole;out->last=s->last;out->gap=s->gap;out->hasGap=s->hasGap;
    out->showGap=!out->post && s->gapUntil>clock && s->hasGap;
    if((out->post || out->showGap) && s->hasGap) { out->leaderId=s->gapLeader;out->leaderBest=s->gapReferenceBest; }
    if(s->active && !s->pit && !s->outlap && clock>=s->start && s->start>0 && clock-s->start<=3600000UL) out->running=clock-s->start;
    memcpy(out->color,out->post?s->postColor:s->liveColor,sizeof(out->color));
    if(s->pit || s->outlap || !s->active) { memset(out->color,0,sizeof(out->color));out->showGap=0; }
    return 1;
}
