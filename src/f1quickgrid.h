#ifndef F1QUICKGRID_H
#define F1QUICKGRID_H
/* Host-testable, no native pointers or game writes. */
static int QGridParse(const char *s,unsigned char *out)
{
 int n=0,v,d;unsigned char seen[41]={0};
 if(!s)return -1;
 while(*s){
  while(*s==' ' || *s=='\t')s++;
  if(!*s)return n?n:-1;
  v=0;d=0;
  while(*s>='0' && *s<='9'){v=v*10+(*s++-'0');if(v>40)return -1;d=1;}
  if(!d || v<1 || seen[v] || n>=40)return -1;
  seen[v]=1;out[n++]=(unsigned char)v;
  while(*s==' ' || *s=='\t')s++;
  if(!*s)return n;
  if(*s++!=',')return -1;
  if(!*s)return -1;
 }
 return -1;
}
/* Reorder the existing selected entrants only. Preserve raw human/control bits.
   Build result separately so an invalid native roster leaves all bytes untouched. */
static int QGridReorder(const unsigned char *src,const unsigned char *pref,int np,unsigned char *dst)
{
 unsigned char seen[41]={0},used[26]={0};int i,j,n=0,id;
 if(np<1 || np>40)return 0;
 for(i=0;i<26;i++){
  id=src[i]&63;
  if(id<1 || id>40 || seen[id])return 0;
  seen[id]=1;
 }
 for(i=0;i<np;i++)for(j=0;j<26;j++)
  if(!used[j] && (src[j]&63)==pref[i]){dst[n++]=src[j];used[j]=1;break;}
 for(j=0;j<26;j++)if(!used[j])dst[n++]=src[j];
 return n==26;
}
#endif
