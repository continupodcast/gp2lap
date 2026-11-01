#ifndef F1PLAYEROPTIONS_H
#define F1PLAYEROPTIONS_H
/* Player options deliberately span pit windows; the NPC allocation and exact
   family rankings stay separate. Each fixed window evaluates all compound pairs. */
static int F1PlayerOptionExists(const AutoPlan *out,int n,const AutoPlan *p)
{
 int i;for(i=0;i<n;i++)if(AutoSamePlan(&out[i],p))return 1;return 0;
}
static int F1PlayerOneStopOptions(AutoPlan *out,int wanted,int laps,int quick)
{
 int targets[3],i,a,b,n=0,first,second,minFirst,maxFirst,found;AutoPlan candidate,best;
 targets[0]=(laps+1)/3;targets[1]=laps-targets[0];targets[2]=(laps*2+2)/5;
 minFirst=laps-AUTO_MAX_STINT;if(minFirst<3)minFirst=3;
 maxFirst=laps-3;if(maxFirst>AUTO_MAX_STINT)maxFirst=AUTO_MAX_STINT;
 if(minFirst>maxFirst)return 0;
 for(i=0;i<3 && n<wanted;i++){
  first=targets[i];if(first<minFirst)first=minFirst;if(first>maxFirst)first=maxFirst;second=laps-first;
  found=0;
  for(a=0;a<3;a++)for(b=0;b<3;b++)if((!StrategyRequireCompoundChange || a!=b) && (!quick || a==i)){
   memset(&candidate,0,sizeof(candidate));candidate.count=2;
   candidate.compound[0]=(unsigned char)a;candidate.compound[1]=(unsigned char)b;
   candidate.length[0]=(unsigned char)first;candidate.length[1]=(unsigned char)second;
   candidate.score=27.0+4.581+autoCost[a][first]+autoCost[b][second];
   if(F1PlayerOptionExists(out,n,&candidate))continue;
   if(!found || candidate.score<best.score){best=candidate;found=1;}
  }
  if(found)out[n++]=best;
 }
 /* The fourth option keeps the best unrestricted one-stop strategy. Fill
    additional feasible slots if a short race leaves only this family. */
 for(i=0;i<autoFamilyCount[2] && n<wanted;i++)
  if(!F1PlayerOptionExists(out,n,&autoFamily[2][i]))out[n++]=autoFamily[2][i];
 return n;
}
/* Different duration profiles, not reordered copies of the same lengths. */
static int F1PlayerMultiStopOptions(AutoPlan *out,int wanted,int laps,int count)
{
 static const int weights3[4][4]={{20,35,45,0},{25,50,25,0},{40,20,40,0},{33,33,34,0}};
 static const int weights4[2][4]={{15,20,30,35},{40,25,20,15}};
 int profile,j,k,n=0,left,total,code,limit,distinct,found,bestIndex;
 int lengths[4],weights[4];double deficit,bestDeficit;AutoPlan candidate,best;
 if(laps<count*3 || laps>count*AUTO_MAX_STINT)return 0;
 for(profile=0;profile<wanted;profile++){
  for(j=0;j<count;j++){lengths[j]=3;weights[j]=count==3?weights3[profile%4][j]:weights4[profile%2][j];}
  left=laps-count*3;
  while(left-- >0){
   bestIndex=-1;bestDeficit=-1e30;
   for(j=0;j<count;j++)if(lengths[j]<AUTO_MAX_STINT){
    deficit=(double)laps*weights[j]/100.0-lengths[j];
    if(bestIndex<0 || deficit>bestDeficit){bestIndex=j;bestDeficit=deficit;}
   }
   if(bestIndex<0)break;lengths[bestIndex]++;
  }
  limit=1;for(j=0;j<count;j++)limit*=3;found=0;
  for(code=0;code<limit;code++){
   memset(&candidate,0,sizeof(candidate));candidate.count=(unsigned char)count;
   candidate.score=(27.0+4.581)*(count-1);k=code;distinct=0;total=0;
   for(j=0;j<count;j++){
    candidate.compound[j]=(unsigned char)(k%3);k/=3;
    distinct|=1<<candidate.compound[j];candidate.length[j]=(unsigned char)lengths[j];
    candidate.score+=autoCost[candidate.compound[j]][lengths[j]];total+=lengths[j];
   }
   if(total!=laps || (StrategyRequireCompoundChange && !(distinct&(distinct-1))) || F1PlayerOptionExists(out,n,&candidate))continue;
   if(!found || candidate.score<best.score){best=candidate;found=1;}
  }
  if(found)out[n++]=best;
 }
 return n;
}
static int AutoBuildPlayerOptions(int laps,int quick)
{
 AutoPlan alternatives[10];int i,count,wanted,available,at;
 if(!AutoBuildDiverse(laps,quick))return 0;
 for(count=2;count<=4;count++){
  wanted=0;for(i=0;i<autoTopCount;i++)if(autoTop[i].count==count)wanted++;
  if(count==2)available=F1PlayerOneStopOptions(alternatives,wanted,laps,quick);
  else available=F1PlayerMultiStopOptions(alternatives,wanted,laps,count);
  at=0;for(i=0;i<autoTopCount;i++)if(autoTop[i].count==count && at<available)autoTop[i]=alternatives[at++];
 }
 return autoTopCount;
}
#endif
