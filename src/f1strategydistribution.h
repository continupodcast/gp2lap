#ifndef F1STRATEGYDISTRIBUTION_H
#define F1STRATEGYDISTRIBUTION_H
/* T13 allocation heuristics are experimental, not measured traffic losses.
   Race-position weights only bias a draw; family quotas guarantee presence. */
static unsigned char autoChosen[41],autoPoolRank[41],autoGrid[41],autoGridValid;
static unsigned long autoAllocationSeed;
static int AutoGridBand(int position,int total)
{
 if(!position)return 1;
 return (position-1)*3/total;
}
static double AutoPlanCost(AutoPlan *p)
{
 double score=27.0*(p->count-1)+4.581;int i;
 for(i=0;i<p->count;i++)score+=autoCost[p->compound[i]][p->length[i]];
 return score;
}
static double AutoOverlap(AutoPlan *p,int id)
{
 int other,i,j,a,b;double penalty=0;
 for(other=1;other<=40;other++)if(autoChosen[other] && other!=id){
  a=0;for(i=0;i<p->count-1;i++){
   a+=p->length[i];b=0;
   for(j=0;j<autoCarPlan[other].count-1;j++){
    b+=autoCarPlan[other].length[j];
    if(a==b)penalty+=0.5;else if(a==b-1 || a==b+1)penalty+=0.25;
   }
  }
 }
 return penalty;
}
static void AutoChooseVariant(int id,int family)
{
 int rank,attempt,j,shift,x,y,bestRank=-1;double objective,best=1e30;
 AutoPlan candidate,bestPlan;
 for(rank=0;rank<autoTopCount;rank++)if(autoTop[rank].count==family){
  for(attempt=0;attempt<13;attempt++){
   candidate=autoTop[rank];
   if(attempt)for(j=0;j<family-1;j++){
    shift=(int)((AutoRand()>>8)%5)-2;
    x=(int)candidate.length[j]+shift;y=(int)candidate.length[j+1]-shift;
    if(x>=3 && x<=AUTO_MAX_STINT && y>=3 && y<=AUTO_MAX_STINT){
     candidate.length[j]=(unsigned char)x;candidate.length[j+1]=(unsigned char)y;
    }
   }
   candidate.score=AutoPlanCost(&candidate);
   /* Reject variants costing >2 estimated seconds versus the selected base. */
   if(candidate.score>autoTop[rank].score+2.0)continue;
   objective=candidate.score+AutoOverlap(&candidate,id)+(AutoRand()>>8)%1000*0.003;
   if(objective<best){best=objective;bestPlan=candidate;bestRank=rank;}
  }
 }
 if(bestRank>=0){autoCarPlan[id]=bestPlan;autoPoolRank[id]=(unsigned char)bestRank;autoChosen[id]=1;}
}
static void AutoPrepareDistribution(int total)
{
 int ids[26],positions[26],n=0,i,j,id,raw,k,quota[5]={0,0,0,0,0};
 int pool[5]={0,0,0,0,0},order[26],available=0,left,best,band,sum,draw,weight[5];
 /* Unique, even native racePos values validate the actual grid. Unknown grids
    use neutral weights and log position zero, never infer position from CarId. */
 autoGridValid=1;memset(autoChosen,0,sizeof(autoChosen));memset(autoGrid,0,sizeof(autoGrid));
 for(i=0;i<total;i++){
  raw=pCarStructs[i].racePos;
  if((raw&1) || raw/2>=total)autoGridValid=0;
  for(j=0;j<i;j++)if(raw==pCarStructs[j].racePos)autoGridValid=0;
 }
 for(i=0;i<total;i++){
  GP2Car *c=&pCarStructs[i];id=c->id&63;
  if(id<1 || id>40 || (c->id&128) || c->lapNr || c->speed || c->numPitStopsDone || (c->flags_90&0x80))continue;
  ids[n]=id;positions[n]=autoGridValid?(c->racePos/2+1):0;order[n]=n;n++;
 }
 if(!n)return;
 for(i=0;i<autoTopCount;i++)pool[autoTop[i].count]++;
 for(k=1;k<=4;k++)if(pool[k])available++;
 left=n;
 /* One representative per feasible family when field size permits. Then
    allocate remaining slots by pool proportions (normal pool 4:4:2). */
 if(n>=available)for(k=1;k<=4;k++)if(pool[k]){quota[k]=1;left--;}
 while(left--){
  best=0;
  for(k=1;k<=4;k++)if(pool[k] && (!best ||
    (double)pool[k]/(quota[k]+1)>(double)pool[best]/(quota[best]+1)))best=k;
  quota[best]++;
 }
 /* Stable ordering makes allocation independent of the physics array order. */
 for(i=0;i<n;i++)for(j=i+1;j<n;j++)if(
   (positions[order[j]] && positions[order[j]]<positions[order[i]]) ||
   (positions[order[j]]==positions[order[i]] && ids[order[j]]<ids[order[i]])){
  k=order[i];order[i]=order[j];order[j]=k;
 }
 for(i=0;i<n;i++){
  id=ids[order[i]];autoGrid[id]=(unsigned char)positions[order[i]];
  band=AutoGridBand(autoGrid[id],total);
  /* Front 4:5:1, middle 3:5:2, rear 5:3:2 for 1/2/3 stops.
     These are transparent trial weights, not universal optimal F1 rules. */
  sum=0;
  for(k=1;k<=4;k++){
   weight[k]=k==1?(band==2?6:3):k==2?(band==0?4:band==1?3:5):k==3?(band==2?3:5):(band==0?1:2);
   weight[k]*=quota[k];sum+=weight[k];
  }
  draw=(int)((AutoRand()>>8)%sum);
  for(k=1;k<=4;k++){if(draw<weight[k])break;draw-=weight[k];}
  AutoChooseVariant(id,k);quota[k]--;
 }
}
#endif
