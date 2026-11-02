#include "f1modules.h"
/* Modern broadcast panels. Reuse the shared timing rows without recalculation. */
static void modernFit(char *s,int font,int width)
{
    int n=(int)strlen(s);
    while(n && textwidth(s,font)>width) s[--n]=0;
}
static void modernPosition(unsigned char *dst,int x,int y,int pos)
{
    char s[8];int k,c,xx,yy,a,dx,dy,w;
    sprintf(s,"%d",pos);
    for(k=0;s[k];k++) {
        c=s[k]-32;w=f1_width2[c];
        for(yy=0;yy<16;yy++) for(xx=0;xx<w;xx++) {
            a=f1_font2[c][yy*20+xx];if(!a) continue;
            for(dy=0;dy<2;dy++) for(dx=0;dx<2;dx++)
                dst[(y+yy*2+dy)*640+x+xx*2+dx]=white[a][dst[(y+yy*2+dy)*640+x+xx*2+dx]];
        }
        x+=w*2;
    }
}
static void modernSide(unsigned char *dst,int x,int y,int w,const F1Row *r,int right)
{
    int di=driver(r->id),team,info,end,font,lw;char last[F1_NAME_LEN],pos[8],first[F1_NAME_LEN],tn[F1_NAME_LEN];
    if(di<0) return;
    team=f1_drivers[di].team;
    sprintf(pos,"%d",r->pos);
    modernPosition(dst,right?x+w-textwidth(pos,2)*2-2:x+2,y+10,r->pos);
    info=right?x:x+35;end=right?x+w-35:x+w;
    clipRight=end;
    strcpy(first,f1_drivers[di].first);modernFit(first,1,end-info);
    text(dst,right?end-textwidth(first,1):info,y+4,first,1,-1,0);
    up90(last,f1_drivers[di].last,sizeof(last));
    font=textwidth(last,0)>end-info?5:0;
    modernFit(last,font,end-info);
    text(dst,right?end-textwidth(last,font):info,y+15,last,font,teamcol[team],0);
    strncpy(tn,f1_team_names[team],sizeof(tn)-1);tn[sizeof(tn)-1]=0;
    font=textwidth(tn,1)>end-info-19?4:1;modernFit(tn,font,end-info-19);
    lw=textwidth(tn,font);
    text(dst,right?end-lw:info,y+26,tn,font,-1,0);
    logo(dst,right?end-lw-19:info+lw+3,y+25,team);
}
void F1RenderCurrentGap(unsigned char *dst,const unsigned char *pal,const F1Row *rows,int count,int focus)
{
#if F1_MODULE_DRIVER_PANELS
#include "../modules/driver-panels/f1modern_F1RenderCurrentGap.inc"
#else

#endif
}
void F1RenderModernFastest(unsigned char *dst,const unsigned char *pal,int id,long ms,int y)
{
#if F1_MODULE_TIMING_TOWER
#include "../modules/timing-tower/f1modern_F1RenderModernFastest.inc"
#else

#endif
}
