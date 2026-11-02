#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../../src/f1playershuffle.h"
int main(void){int a[10],b[10],seen[10],front[10]={0},i,j,different=0;unsigned long x,y;
 for(i=1;i<=200;i++){
  x=y=(unsigned long)i;F1PlayerShuffle(a,10,&x);F1PlayerShuffle(b,10,&y);assert(!memcmp(a,b,sizeof(a)));
  memset(seen,0,sizeof(seen));for(j=0;j<10;j++){assert(a[j]>=0 && a[j]<10);seen[a[j]]++;if(a[j]!=j)different=1;}
  for(j=0;j<10;j++)assert(seen[j]==1);front[a[0]]++;
 }
 for(i=0;i<10;i++)assert(front[i]>0);assert(different);
 x=1;F1PlayerShuffle(a,1,&x);assert(a[0]==0);
 puts("PASS T14.2 randomized permutation of every selectable plan, original outside shuffle, valid selection mapping, independent random state");return 0;}
