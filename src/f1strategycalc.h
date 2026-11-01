#include "f1compoundrule.h"
#ifndef F1STRATEGYCALC_H
#define F1STRATEGYCALC_H
#include "f1strategymodel.h"
#define AUTO_MAX_LAPS 255
#define AUTO_MAX_STINT 90
typedef struct {double score;unsigned char count,compound[4],length[4];} AutoPlan;
static AutoPlan autoTop[10],autoFamily[5][10];
static int autoFamilyCount[5];
static int autoTopCount,autoCalcLaps,autoCalcQuick;
static int autoCacheLaps=-1,autoCacheQuick,autoSolveRuns,autoCacheCompoundRule=-1;
static int autoPrefixCount;
static unsigned char autoSolvedPrefix[4];
static double autoCost[3][AUTO_MAX_STINT+1];
static double autoDP[5][AUTO_MAX_LAPS+1][10];
static unsigned char autoPrev[5][AUTO_MAX_LAPS+1][10];
static unsigned char autoSequence[4];
static unsigned char autoPrevRank[5][AUTO_MAX_LAPS+1][10];
static int AutoSamePlan(const AutoPlan *a,const AutoPlan *b)
{
 int j;if(a->count!=b->count)return 0;
 for(j=0;j<a->count;j++)if(a->compound[j]!=b->compound[j] || a->length[j]!=b->length[j])return 0;
 return 1;
}
static void AutoKeepFamily(AutoPlan *p)
{
 int i,j,k=p->count,n=autoFamilyCount[k];
 for(i=0;i<n;i++)if(AutoSamePlan(p,&autoFamily[k][i]))return;
 for(i=0;i<n && autoFamily[k][i].score<=p->score;i++);
 if(i>=10)return;if(n<10)n++;
 for(j=n-1;j>i;j--)autoFamily[k][j]=autoFamily[k][j-1];
 autoFamily[k][i]=*p;autoFamilyCount[k]=n;
}
static void AutoKeep(AutoPlan *p)
{
 int i,j,same;
 for(i=0;i<autoTopCount;i++){
  same=p->count==autoTop[i].count;
  for(j=0;j<p->count && same;j++)if(p->compound[j]!=autoTop[i].compound[j] || p->length[j]!=autoTop[i].length[j])same=0;
  if(same)return;
 }
 for(i=0;i<autoTopCount && autoTop[i].score<=p->score;i++);
 if(i>=10)return;
 if(autoTopCount<10)autoTopCount++;
 for(j=autoTopCount-1;j>i;j--)autoTop[j]=autoTop[j-1];
 autoTop[i]=*p;
}
static void AutoEvaluate(AutoPlan *p)
{
 int i,total=0;
 p->score=27.0*(p->count-1)+4.581;
 for(i=0;i<p->count;i++){
  if(p->length[i]<3 || p->length[i]>AUTO_MAX_STINT)return;
  total+=p->length[i];p->score+=autoCost[p->compound[i]][p->length[i]];
 }
 if(total!=autoCalcLaps)return;
 AutoKeepFamily(p);AutoKeep(p);
}
static void AutoSolveSequence(int count)
{
 int i,t,n,r,pos,j,rank,left,bn,common=0,lo,hi,first,last;double v;AutoPlan p;
 /* Adjacent compound sequences share prefixes. Keep completed DP stages for
    those prefixes instead of recomputing them for each last compound. */
 if(autoPrefixCount==count){
  while(common<count && autoSequence[common]==autoSolvedPrefix[common])common++;
 }else{
  for(t=0;t<=autoCalcLaps;t++)for(r=0;r<10;r++)autoDP[0][t][r]=1e30;
  autoDP[0][0][0]=0;
 }
 for(i=common+1;i<=count;i++){
  for(t=0;t<=autoCalcLaps;t++)for(r=0;r<10;r++){
   autoDP[i][t][r]=1e30;autoPrev[i][t][r]=0;autoPrevRank[i][t][r]=0;
  }
  /* Only totals that can still reach the race distance are useful. At the
     final stage this is exactly one total: the complete race distance. */
  lo=autoCalcLaps-AUTO_MAX_STINT*(count-i);if(lo<3*i)lo=3*i;
  hi=autoCalcLaps-3*(count-i);if(hi>AUTO_MAX_STINT*i)hi=AUTO_MAX_STINT*i;
  for(t=lo;t<=hi;t++){
   first=t-AUTO_MAX_STINT*(i-1);if(first<3)first=3;
   last=t-3*(i-1);if(last>AUTO_MAX_STINT)last=AUTO_MAX_STINT;
   for(n=first;n<=last;n++)for(r=0;r<10;r++){
    if(autoDP[i-1][t-n][r]>=1e29)break;
    v=autoDP[i-1][t-n][r]+autoCost[autoSequence[i-1]][n];
    for(pos=0;pos<10 && autoDP[i][t][pos]<=v;pos++);
    if(pos==10)continue;
    for(j=9;j>pos;j--){autoDP[i][t][j]=autoDP[i][t][j-1];autoPrev[i][t][j]=autoPrev[i][t][j-1];autoPrevRank[i][t][j]=autoPrevRank[i][t][j-1];}
    autoDP[i][t][pos]=v;autoPrev[i][t][pos]=(unsigned char)n;autoPrevRank[i][t][pos]=(unsigned char)r;
   }
  }
 }
 autoPrefixCount=count;memcpy(autoSolvedPrefix,autoSequence,sizeof(autoSolvedPrefix));
 for(r=0;r<10;r++){
  if(autoDP[count][autoCalcLaps][r]>=1e29)break;
  memset(&p,0,sizeof(p));p.count=(unsigned char)count;left=autoCalcLaps;rank=r;
  for(i=count;i>0;i--){bn=autoPrev[i][left][rank];j=autoPrevRank[i][left][rank];p.length[i-1]=(unsigned char)bn;p.compound[i-1]=autoSequence[i-1];left-=bn;rank=j;}
  AutoEvaluate(&p);
 }
}
static void AutoSequences(int at,int count)
{
 int c,i,different=0;
 if(at==count){
  for(i=1;i<count;i++)if(autoSequence[i]!=autoSequence[0])different=1;
  if(!StrategyRequireCompoundChange || count==1 || different)AutoSolveSequence(count);return;
 }
 for(c=0;c<3;c++){autoSequence[at]=(unsigned char)c;AutoSequences(at+1,count);}
}
static int AutoBuild(int laps,int quick)
{
 int c,n,a,k,j;double sum,floor;
 autoTopCount=0;if(laps<6 || laps>AUTO_MAX_LAPS)return 0;
 autoCalcLaps=laps;autoCalcQuick=quick;
 if(autoCacheLaps==laps && autoCacheQuick==quick && autoCacheCompoundRule==(int)StrategyRequireCompoundChange){
  for(k=1;k<=4;k++)for(j=0;j<autoFamilyCount[k];j++)AutoKeep(&autoFamily[k][j]);
  return autoTopCount;
 }
 memset(autoFamilyCount,0,sizeof(autoFamilyCount));autoPrefixCount=0;autoSolveRuns++;
 for(c=0;c<3;c++){
  floor=0;for(a=35;a<=46;a++)floor+=autoZeroFuel[c][a];floor/=12;
  sum=0;autoCost[c][0]=0;
  for(n=1;n<=AUTO_MAX_STINT;n++){
   sum+=n<=46?autoZeroFuel[c][n]:floor;
   /* Measured burn 1.815 million raw/lap, one real reserve lap.
      Sum average fuel through stint = n*(n/2+1) laps. */
   autoCost[c][n]=sum+0.06831732804937128*n*(n*0.5+1.0);
  }
 }
 for(k=quick?1:2;k<=4;k++)if(3*k<=laps)AutoSequences(0,k);
 autoCacheLaps=laps;autoCacheQuick=quick;autoCacheCompoundRule=(int)StrategyRequireCompoundChange;
 return autoTopCount;
}
/* Ten best within explicit families, rather than ten absolute near-duplicates.
   Normal quota: four one-stop, four two-stop, two three-stop.
   Quickrace: two zero-stop, three one-stop, three two-stop, two three-stop. */
static int AutoBuildDiverse(int laps,int quick)
{
 int k,j,q[5]={0,0,4,4,2},used[5]={0,0,0,0,0},best,at;
 AutoPlan selected[10];
 if(!AutoBuild(laps,quick))return 0;
 if(quick){q[1]=2;q[2]=3;q[3]=3;}
 at=0;
 for(k=1;k<=4;k++)for(j=0;j<q[k] && j<autoFamilyCount[k];j++){
  selected[at++]=autoFamily[k][j];used[k]++;
 }
 while(at<10){
  best=0;
  for(k=1;k<=4;k++)if(used[k]<autoFamilyCount[k] &&
    (!best || autoFamily[k][used[k]].score<autoFamily[best][used[best]].score))best=k;
  if(!best)break;selected[at++]=autoFamily[best][used[best]++];
 }
 memcpy(autoTop,selected,at*sizeof(AutoPlan));autoTopCount=at;return at;
}

#endif
