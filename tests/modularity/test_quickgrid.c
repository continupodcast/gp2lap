#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "../../src/f1quickgrid.h"
int main(void){unsigned char p[40],s[26],d[26],backup[26];int i,n;
for(i=0;i<26;i++)s[i]=(unsigned char)(i+1);s[12]|=128;
n=QGridParse("13, 26, 1, 40",p);assert(n==4);assert(QGridReorder(s,p,n,d));
assert(d[0]==(13|128)&&d[1]==26&&d[2]==1&&d[3]==2);assert(d[25]==25);
/* Human flag remains associated with the correct car, all entrants retained. */
for(i=1;i<=26;i++){int j,count=0;for(j=0;j<26;j++)if((d[j]&63)==i)count++;assert(count==1);}
assert(QGridParse("1,1",p)==-1);assert(QGridParse("0",p)==-1);assert(QGridParse("41",p)==-1);assert(QGridParse("1,",p)==-1);assert(QGridParse("1,,2",p)==-1);assert(QGridParse("1x",p)==-1);assert(QGridParse("",p)==-1);
n=QGridParse("26,25",p);memcpy(backup,s,26);s[25]=s[0];assert(!QGridReorder(s,p,n,d));assert(s[25]==s[0]);memcpy(s,backup,26);s[25]=0;assert(!QGridReorder(s,p,n,d));
puts("PASS quick grid: desired order, skipped non-starter, all 26 retained, human flag preserved, invalid config/native roster rejected");return 0;}
