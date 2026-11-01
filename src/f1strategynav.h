#ifndef F1STRATEGYNAV_H
#define F1STRATEGYNAV_H
/* Button centres match the pre-race renderer. Each arrow follows screen space. */
static void F1StrategyNavigate(int count,int key,int *focus,int *side)
{
 int f,s,n=0,i,cur=-1,best=-1,dx,dy,distance,bestDistance=100000;
 int fields[21],sides[21],xs[21],ys[21],action=2*count+1;
 for(f=0;f<action+3;f++)for(s=0;s<(f<action?2:1);s++){
  fields[n]=f;sides[n]=s;
  if(f==0){xs[n]=s?382:232;ys[n]=104;}
  else if(f==1){xs[n]=s?312:232;ys[n]=135;}
  else if(f<action){xs[n]=(f%2)?(s?459:379):(s?330:186);ys[n]=196+30*((f-2)/2);}
  else{xs[n]=f==action?173:f==action+1?320:467;ys[n]=413;}
  if(f==*focus && (f>=action || s==*side))cur=n;
  n++;
 }
 if(cur<0)return;
 for(i=0;i<n;i++){
  dx=xs[i]-xs[cur];dy=ys[i]-ys[cur];
  if(key==0x4b || key==0x4d){
   if(dy || (key==0x4b?dx>=0:dx<=0))continue;
   distance=dx<0?-dx:dx;
  }else{
   if(key==0x48?dy>=0:dy<=0)continue;
   distance=(dy<0?-dy:dy)*100+(dx<0?-dx:dx);
  }
  if(distance<bestDistance){best=i;bestDistance=distance;}
 }
 if(best>=0){*focus=fields[best];*side=sides[best];}
}
#endif
