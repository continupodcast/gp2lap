#include <string.h>
#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include <stdlib.h>
#include "f1render.h"
#include "f1config.h"
#include "f1assets.h"
#include "f1qualy.h"
#define TEAM_LOGO(t) ((f1_custom_logo_loaded[t] || (t)>=13) ? f1_custom_logos[t] : f1_logos[t])
static unsigned char lastpal[768], dark[256], white[4][256];
static unsigned char teamcol[F1_TEAM_COUNT], logocol[F1_TEAM_COUNT][192];
static int initialized,clipRight=640;
static unsigned char sectorcol[4], losscol, platecol[F1_TEAM_COUNT][4];
static unsigned char badgeWhite,badgeBlack,tyrecol[4];
static void F1RenderRace90(unsigned char *dst,const unsigned char *pal,const F1Row *rows,int count,int lap,int total,int mode);
static void F1RenderDriver90(unsigned char *dst,const unsigned char *pal,int id,int position,int x,int y);
static void F1RenderQualy90(unsigned char *dst,const unsigned char *pal,const F1QRow *rows,int count,
                            long remaining,int practice,int gaps,int compact,int focus);
static void F1RenderQualyCard90(unsigned char *dst,const unsigned char *pal,const F1QCard *c,int cockpit);
static unsigned char nearest(const unsigned char *p,int r,int g,int b)
{
    int i,best=0,dr,dg,db; long d,min=2147483647L;
    for(i=0;i<256;i++) { dr=p[i*3]*4-r; dg=p[i*3+1]*4-g; db=p[i*3+2]*4-b;
        d=(long)dr*dr+(long)dg*dg+(long)db*db;
        if(d<min) { min=d; best=i; }
    }
    return (unsigned char)best;
}
static void palette(const unsigned char *p)
{
    int i,j,k,r,g,b;
    F1ConfigEnsure();
    if(initialized && !memcmp(lastpal,p,768)) return;
    memcpy(lastpal,p,768); initialized=1;
    for(i=0;i<256;i++) {
        r=p[i*3]*4; g=p[i*3+1]*4; b=p[i*3+2]*4;
        dark[i]=nearest(p,(r+15*4)/5,(g+17*4)/5,(b+23*4)/5);
        for(j=0;j<4;j++) white[j][i]=nearest(p,(r*(3-j)+255*j)/3,(g*(3-j)+255*j)/3,(b*(3-j)+255*j)/3);
    }
    tyrecol[0]=nearest(p,210,210,210);tyrecol[1]=nearest(p,245,224,0);
    tyrecol[2]=nearest(p,255,48,48);tyrecol[3]=nearest(p,168,85,247);
    losscol=nearest(p,255,64,64);
    badgeWhite=nearest(p,255,255,255);badgeBlack=nearest(p,0,0,0);
    sectorcol[0]=nearest(p,45,45,45); sectorcol[1]=nearest(p,0,224,96);
    sectorcol[2]=nearest(p,245,224,0); sectorcol[3]=nearest(p,168,85,247);
    for(i=0;i<F1_TEAM_COUNT;i++) {
        platecol[i][0]=nearest(p,17,16,25);
        platecol[i][1]=nearest(p,25,23,34);
        platecol[i][2]=nearest(p,(f1_colors[i][0]+30)/4,(f1_colors[i][1]+30)/4,(f1_colors[i][2]+45)/4);
        platecol[i][3]=nearest(p,f1_colors[i][0]/2,f1_colors[i][1]/2,f1_colors[i][2]/2);
        teamcol[i]=nearest(p,f1_colors[i][0],f1_colors[i][1],f1_colors[i][2]);
        for(k=0;k<192;k++) logocol[i][k]=nearest(p,TEAM_LOGO(i)[k][0],TEAM_LOGO(i)[k][1],TEAM_LOGO(i)[k][2]);
    }
}
static int textwidth(const char *s,int font)
{
    int n=0,c; const unsigned char *w;
    w=font==0?f1_width0:font==1?f1_width1:font==2?f1_width2:font==3?f1_width3:font==4?f1_width4:f1_width5;
    while(*s) { c=(unsigned char)*s++; if(c>=32 && c<127) n+=w[c-32]; }
    return n;
}
static void text(unsigned char *dst,int x,int y,const char *s,int font,int color,int dim)
{
    int c,xx,yy,a,w; const unsigned char *glyph,*width;
    width=font==0?f1_width0:font==1?f1_width1:font==2?f1_width2:font==3?f1_width3:font==4?f1_width4:f1_width5;
    while(*s) {
        c=(unsigned char)*s++; if(c<32 || c>=127) continue; c-=32;
        glyph=font==0?f1_font0[c]:font==1?f1_font1[c]:font==2?f1_font2[c]:font==3?f1_font3[c]:font==4?f1_font4[c]:f1_font5[c];
        w=width[c];
        for(yy=0;yy<16 && y+yy<480;yy++) for(xx=0;xx<w && x+xx<clipRight;xx++) {
            a=glyph[yy*20+xx]; if(!a || x+xx<0 || y+yy<0) continue;
            if(dim && a>1) a=1;
            if(color<0) dst[(y+yy)*640+x+xx]=white[a][dst[(y+yy)*640+x+xx]];
            else if(a>=2) dst[(y+yy)*640+x+xx]=(unsigned char)color;
        }
        x+=w;
    }
}
void F1RenderNotice(unsigned char *dst,const unsigned char *pal,const char *message)
{
    int x,y;
    palette(pal);clipRight=440;
    for(y=50;y<64;y++) for(x=200;x<440;x++) dst[y*640+x]=dark[dst[y*640+x]];
    text(dst,320-textwidth(message,0)/2,52,message,0,-1,0);
}
void F1RenderMicro(unsigned char *dst,const unsigned char *pal,const unsigned char *colors,int cockpit)
{
    int s,i,x,y,col,step=cockpit?9:6,width=cockpit?86:62;
    int left=cockpit?F1_CARD_COMPACT_X:F1_CARD_X;
    int top=(cockpit?F1_CARD_Y_COCKPIT:F1_CARD_Y_TV)+(cockpit?43:77);
    palette(pal);
    for(s=0;s<3;s++) {
        for(i=0;i<width;i++) for(y=top;y<top+3;y++) {
            col=i<step*10 && i%step<step-1?sectorcol[colors[s*10+i/step]<=3?colors[s*10+i/step]:0]:badgeBlack;
            x=left+8+s*(cockpit?90:66)+i;dst[y*640+x]=(unsigned char)col;
        }
    }
}
/* Keep exactly the two immediate neighbours on either side, where present. */
int F1Window(const F1Row *rows,int count,int focusId,int *start)
{
    int i,end;
    *start=0;
    for(i=0;i<count;i++) if(rows[i].id==focusId) {
        *start=i>2?i-2:0;
        end=i+3<count?i+3:count;
        return end-*start;
    }
    return 0;
}
void F1Gap(char *label,long milliseconds)
{
    long tenths;
    if(milliseconds<0) { strcpy(label,"-"); return; }
    tenths=(milliseconds+50)/100;
    if(tenths>=600) sprintf(label,"+%ld:%02ld.%ld",tenths/600,(tenths/10)%60,tenths%10);
    else sprintf(label,"+%ld.%ld",tenths/10,tenths%10);
}
void F1Render(unsigned char *dst,const unsigned char *pal,const F1Row *rows,int count,int lap,int total)
{
    F1RenderRace(dst,pal,rows,count,lap,total,0);
}
/* Compact badges outside the existing panel: no time-column width is lost. */
static void finishBadge(unsigned char *dst,int x,int y)
{
    int xx,yy;
    /* Same 11x11 indicator slot, bordered black/white checkerboard. */
    for(yy=0;yy<11;yy++) for(xx=0;xx<11;xx++)
        dst[(y+yy)*640+x+xx]=(xx==0 || yy==0 || xx==10 || yy==10 || ((xx-1)/3+(yy-1)/3)%2)?badgeBlack:badgeWhite;
}
static void badge(unsigned char *dst,int x,int y,int fastest)
{
    static const unsigned char clock[9]={28,8,62,65,73,73,77,65,62};
    static const unsigned char pit[9]={0,30,17,17,30,16,16,16,0};
    int xx,yy,on;
    for(yy=0;yy<11;yy++) for(xx=0;xx<11;xx++) {
        on=yy>0 && yy<10 && xx>1 && xx<9 &&
            ((fastest?clock[yy-1]:pit[yy-1])&(1<<(8-xx)));
        dst[(y+yy)*640+x+xx]=fastest?(on?badgeWhite:sectorcol[3]):(on?badgeBlack:badgeWhite);
    }
}
void F1RenderRace(unsigned char *dst,const unsigned char *pal,const F1Row *rows,int count,int lap,int total,int mode)
{
    int i,j,x,y,team,di,bottom,col; const F1Row *row; char label[40];
    if(!dst || !pal || !rows || count<1) return;
    if(count>F1_MAX_ROWS) count=F1_MAX_ROWS;
    if(f1_theme==F1_THEME_90S) { F1RenderRace90(dst,pal,rows,count,lap,total,mode); return; }
    clipRight=F1_PANEL_X+F1_RACE_W;
    palette(pal); bottom=F1_PANEL_Y+F1_HEADER_H+count*F1_ROW_H+3;
    for(y=F1_PANEL_Y;y<bottom;y++) for(x=F1_PANEL_X;x<F1_PANEL_X+F1_RACE_W;x++) dst[y*640+x]=dark[dst[y*640+x]];
    strcpy(label,mode==4?"TYRE AGE":mode==1?"POS +/-":mode==2?"PITS":mode==3?"LEADER GAP":"RACE");
    text(dst,F1_PANEL_X+(F1_RACE_W-textwidth(label,3))/2,F1_PANEL_Y+2,label,3,-1,0);
    if(total>0) sprintf(label,"LAP %d / %d",lap,total); else sprintf(label,"LAP %d",lap);
    text(dst,F1_PANEL_X+(F1_RACE_W-textwidth(label,1))/2,F1_PANEL_Y+20,label,1,-1,0);
    for(i=0;i<count;i++) {
        row=&rows[i]; y=F1_PANEL_Y+F1_HEADER_H+i*F1_ROW_H;
        di=-1; for(j=0;j<F1_DRIVER_COUNT;j++) if(f1_drivers[j].id==row->id) { di=j; break; }
        team=di<0?-1:f1_drivers[di].team;
        if(row->focused) for(j=0;j<F1_ROW_H-1;j++) {
            dst[(y+j)*640+F1_PANEL_X]=team<0?white[3][0]:teamcol[team];
            dst[(y+j)*640+F1_PANEL_X+1]=team<0?white[3][0]:teamcol[team];
        }
        if(row->positionChange) {
            col=row->positionChange>0?sectorcol[1]:losscol;
            for(j=0;j<4;j++)for(x=-j;x<=j;x++)
                dst[(y+3+(row->positionChange>0?j:3-j))*640+F1_PANEL_X+10+x]=col;
        } else {
            sprintf(label,"%d",row->pos); text(dst,F1_PANEL_X+5,y,label,2,-1,row->out);
        }
        if(team>=0) {
            if(f1_team_logo_enabled[team]) {
                for(j=0;j<192;j++) if(TEAM_LOGO(team)[j][3]>=128)
                    dst[(y+j/16)*640+F1_PANEL_X+18+j%16]=row->out?dark[logocol[team][j]]:logocol[team][j];
            } else for(j=0;j<192;j++) dst[(y+j/16)*640+F1_PANEL_X+18+j%16]=row->out?dark[teamcol[team]]:teamcol[team];
        }
        text(dst,F1_PANEL_X+35,y,di<0?"???":f1_drivers[di].abbr,0,-1,row->out);
        col=-1;
        if(mode==1) {
            if(row->gain==F1_GAIN_UNKNOWN) strcpy(label,"-");
            else if(!row->gain) strcpy(label,"-");
            else { sprintf(label,"%+d",row->gain);col=row->gain>0?sectorcol[1]:losscol; }
        } else if(mode==2) sprintf(label,"%d",row->stops);
        else if(mode==4){if(row->tyreLaps<0)strcpy(label,"-");else sprintf(label,"%d",row->tyreLaps);}
        else if(row->out) strcpy(label,"OUT");
        else if(row->pit) strcpy(label,"IN PIT");
        else if(row->pos==1) strcpy(label,mode==3?"Leader":"Interval");
        else if(mode==3 && row->lapsBehind>0) sprintf(label,"+%dL",row->lapsBehind);
        else if(mode==3) F1Gap(label,row->leaderGap);
        else F1Gap(label,row->gap);
        if(!mode && !row->out && (row->pit || row->pos==1) && team>=0) col=teamcol[team];
        text(dst,F1_PANEL_X+F1_PANEL_W-3-textwidth(label,1),y+1,label,1,col,mode?0:row->out);
        if(PerCarTyreCompounds){
        label[0]=row->compound>=0 && row->compound<4?"HMSS"[row->compound]:'?';label[1]=0;
        text(dst,F1_PANEL_X+F1_PANEL_W+4,y+1,label,1,row->compound>=0 && row->compound<4?tyrecol[row->compound]:-1,row->out);
        text(dst,F1_PANEL_X+F1_PANEL_W+5,y+1,label,1,row->compound>=0 && row->compound<4?tyrecol[row->compound]:-1,row->out);
        }
        if(row->finished) finishBadge(dst,F1_PANEL_X+F1_RACE_W+1,y);
        else if(row->fastest) badge(dst,F1_PANEL_X+F1_RACE_W+1,y,1);
    }
}

static int driver(int id)
{
    int i;for(i=0;i<F1_DRIVER_COUNT;i++) if(f1_drivers[i].id==id) return i;return -1;
}
static void background(unsigned char *dst,int x,int y,int w,int h)
{
    int xx,yy;for(yy=y;yy<y+h;yy++) for(xx=x;xx<x+w;xx++) dst[yy*640+xx]=dark[dst[yy*640+xx]];
}
static void bar(unsigned char *dst,int x,int y,int w,int h,int col)
{
    int xx,yy;for(yy=y;yy<y+h;yy++) for(xx=x;xx<x+w;xx++) dst[yy*640+xx]=(unsigned char)col;
}
static void logo(unsigned char *dst,int x,int y,int team)
{
    int j;if(team<0) return;
    if(!f1_team_logo_enabled[team]) { for(j=0;j<192;j++) dst[(y+j/16)*640+x+j%16]=teamcol[team]; return; }
    for(j=0;j<192;j++) if(TEAM_LOGO(team)[j][3]>=128) dst[(y+j/16)*640+x+j%16]=logocol[team][j];
}
/* Centre the visible glyph bounds, not the font advance or baseline. */
static void centeredNumber(unsigned char *dst,int x,int y,int w,int h,const char *s,int scale,int col)
{
    int c,xx,yy,sx,sy,a,advance=0,left=1000,right=-1,top=16,bottom=-1,ox,oy;
    const char *q=s;
    while(*q) {
        c=*q++-32;
        for(yy=0;yy<16;yy++) for(xx=0;xx<f1_width0[c];xx++) if(f1_font0[c][yy*20+xx]) {
            if(advance+xx<left) left=advance+xx;
            if(advance+xx>right) right=advance+xx;
            if(yy<top) top=yy;
            if(yy>bottom) bottom=yy;
        }
        advance+=f1_width0[c];
    }
    if(right<left) return;
    ox=x+(w-(right-left+1)*scale)/2-left*scale;
    oy=y+(h-(bottom-top+1)*scale)/2-top*scale;
    while(*s) {
        c=*s++-32;
        for(yy=top;yy<=bottom;yy++) for(xx=0;xx<f1_width0[c];xx++) {
            a=f1_font0[c][yy*20+xx];if(!a) continue;
            for(sy=0;sy<scale;sy++) for(sx=0;sx<scale;sx++) {
                int px=ox+xx*scale+sx,py=oy+yy*scale+sy;
                if(px>=x && px<x+w && py>=y && py<y+h)
                    dst[py*640+px]=col<0?white[a][dst[py*640+px]]:(unsigned char)col;
            }
        }
        ox+=f1_width0[c]*scale;
    }
}
int F1DriverLeft(int cockpit)
{
    return f1_theme==F1_THEME_90S && !cockpit?F1_P90_TV_X:F1_DRIVER_X;
}
int F1DriverTop(int cockpit,int qualy,int rows)
{
    int bottom;
    if(cockpit) return F1_DRIVER_Y;
    if(f1_theme==F1_THEME_90S) return F1_P90_TV_Y;
    if(rows<1) return 480-8-F1_DRIVER_H;
    if(rows>F1_MAX_ROWS) rows=F1_MAX_ROWS;
    bottom=F1_PANEL_Y+(qualy?F1_Q_HEADER:F1_HEADER_H)+rows*F1_ROW_H+3;
    return bottom-F1_DRIVER_H;
}
void F1RenderDriver(unsigned char *dst,const unsigned char *pal,int id,int position)
{
    F1RenderDriverAt(dst,pal,id,position,F1_DRIVER_Y);
}
/* Larger surname face. Condense only when necessary to preserve the full
   surname inside the existing 200x38 card, without covering the number. */
static void driverSurname(unsigned char *dst,int x,int y,const char *name,int col)
{
    char upper[F1_NAME_LEN];int i,c,w,total,advance=0,xx,yy,a,px,target;
    for(i=0;i<F1_NAME_LEN-1 && name[i];i++)upper[i]=(char)toupper((unsigned char)name[i]);
    upper[i]=0;total=textwidth(upper,3);target=total>105?105:total;
    if(!total)return;
    for(i=0;upper[i];i++) {
        c=(unsigned char)upper[i];if(c<32 || c>=127)continue;c-=32;w=f1_width3[c];
        for(yy=0;yy<16;yy++)for(xx=0;xx<w;xx++) {
            a=f1_font3[c][yy*20+xx];px=x+(advance+xx)*target/total;
            if(a>=2 && px<clipRight && y+yy<480)dst[(y+yy)*640+px]=(unsigned char)col;
        }
        advance+=w;
    }
}
void F1RenderDriverAt(unsigned char *dst,const unsigned char *pal,int id,int position,int y)
{
    F1RenderDriverAtXY(dst,pal,id,position,F1_DRIVER_X,y);
}
void F1RenderDriverAtXY(unsigned char *dst,const unsigned char *pal,int id,int position,int x,int y)
{
    int di,team,xx,yy,k,lx,j,best,a,dx,dy;
    char label[20];const F1Driver *d;
    if(!dst || !pal || id<1 || id>40 || y<0 || y>480-F1_DRIVER_H || x<0 || x>640-F1_DRIVER_W) return;
    di=driver(id);if(di<0) return;d=&f1_drivers[di];team=d->team;
    if(f1_theme==F1_THEME_90S) { F1RenderDriver90(dst,pal,id,position,x,y); return; }
    palette(pal);clipRight=x+F1_DRIVER_W;
    background(dst,x,y,F1_DRIVER_W,F1_DRIVER_H);
    for(yy=0;yy<F1_DRIVER_H;yy++) for(xx=0;xx<F1_DRIVER_W;xx++) {
        k=xx+yy/2;
        if(xx>=145) dst[(y+yy)*640+x+xx]=platecol[team][2];
        else if((k>=96 && k<108) || (k>=117 && k<126))
            dst[(y+yy)*640+x+xx]=platecol[team][1];
    }
    if(position>0) sprintf(label,"%d",position);else strcpy(label,"-");
    centeredNumber(dst,x+2,y,30,38,label,2,-1);
    clipRight=x+143;
    text(dst,x+38,y+2,d->first,5,-1,0);
    driverSurname(dst,x+38,y+10,d->last,teamcol[team]);
    text(dst,x+38,y+26,f1_team_names[team],5,-1,0);
    lx=x+38+textwidth(f1_team_names[team],5)+3;
    if(lx+8<=x+143 && !f1_team_logo_enabled[team]) for(yy=0;yy<6;yy++) for(xx=0;xx<8;xx++) dst[(y+27+yy)*640+lx+xx]=teamcol[team];
    else if(lx+8<=x+143) for(yy=0;yy<6;yy++) for(xx=0;xx<8;xx++) {
        best=-1;a=0;
        for(dy=0;dy<2;dy++) for(dx=0;dx<2;dx++) {
            j=(yy*2+dy)*16+xx*2+dx;
            if(TEAM_LOGO(team)[j][3]>a) {a=TEAM_LOGO(team)[j][3];best=j;}
        }
        if(a>=128) dst[(y+27+yy)*640+lx+xx]=logocol[team][best];
    }
    sprintf(label,"%d",d->number);
    centeredNumber(dst,x+145,y,55,38,label,2,teamcol[team]);
}
void F1RenderQualy(unsigned char *dst,const unsigned char *pal,const F1QRow *rows,
                   int count,long remaining,int practice,int gaps,int compact,int focus)
{
    int i,y,di,team,start=0,n,col,font;long leader;char label[40];const F1QRow *r;
    if(!dst || !pal || !rows || count<1) return;
    if(count>F1_Q_MAX) count=F1_Q_MAX;
    leader=rows[0].best;
    if(f1_theme==F1_THEME_90S) { F1RenderQualy90(dst,pal,rows,count,remaining,practice,gaps,compact,focus); return; }
    n=compact?F1QWindow(rows,count,focus,&start):count;
    if(!n) return;
    palette(pal);clipRight=F1_PANEL_X+F1_Q_TOTAL_W;
    background(dst,F1_PANEL_X,F1_PANEL_Y,F1_Q_TOTAL_W,F1_Q_HEADER+n*F1_ROW_H+3);
    text(dst,F1_PANEL_X+5,F1_PANEL_Y+2,practice?"PRACTICE":"QUALY",practice?0:3,-1,0);
    if(remaining>=0) {
        sprintf(label,"%ld:%02ld",remaining/60000,(remaining/1000)%60);
        text(dst,F1_PANEL_X+F1_Q_W-3-textwidth(label,1),F1_PANEL_Y+3,label,1,remaining<120000?sectorcol[2]:-1,0);
    }
    text(dst,F1_PANEL_X+F1_Q_W-3-textwidth((F1QCombinedMode?"COMBINED":"BEST / GAP"),1),F1_PANEL_Y+21,(F1QCombinedMode?"COMBINED":"BEST / GAP"),1,-1,0);
    for(i=0;i<n;i++) {
        r=&rows[start+i];y=F1_PANEL_Y+F1_Q_HEADER+i*F1_ROW_H;di=driver(r->id);team=di<0?-1:f1_drivers[di].team;
        col=r->improved?sectorcol[1]:-1;
        if(compact && r->id==focus) bar(dst,F1_PANEL_X,y,2,F1_ROW_H-1,team<0?sectorcol[1]:teamcol[team]);
        if(r->best) sprintf(label,"%d",r->pos);else strcpy(label,"-");
        clipRight=F1_PANEL_X+18;text(dst,F1_PANEL_X+5,y,label,2,col,0);
        logo(dst,F1_PANEL_X+18,y,team);
        clipRight=F1_PANEL_X+62;text(dst,F1_PANEL_X+35,y,di<0?"???":f1_drivers[di].abbr,0,col,0);
        if(!r->best) strcpy(label,"NO TIME");
        else if(gaps && r->pos>1 && leader) F1QDelta(label,r->best-leader);
        else F1QTime(label,r->best,0);
        font=r->best?1:5;
        clipRight=F1_PANEL_X+F1_Q_W-3;text(dst,clipRight-textwidth(label,font),y+(r->best?1:3),label,font,col,0);
        if(PerCarTyreCompounds){
        label[0]=r->compound>=0 && r->compound<4?"HMSS"[r->compound]:'?';label[1]=0;
        clipRight=F1_PANEL_X+F1_Q_TOTAL_W;
        text(dst,F1_PANEL_X+F1_Q_W+4,y+1,label,1,r->compound>=0 && r->compound<4?tyrecol[r->compound]:-1,0);
        text(dst,F1_PANEL_X+F1_Q_W+5,y+1,label,1,r->compound>=0 && r->compound<4?tyrecol[r->compound]:-1,0);
        }
        clipRight=F1_PANEL_X+F1_Q_TOTAL_W;
        if(r->pit) badge(dst,F1_PANEL_X+F1_Q_TOTAL_W+1,y,0);
    }
}
void F1RenderQualyCard(unsigned char *dst,const unsigned char *pal,const F1QCard *c,int cockpit)
{
    int di,team,i,x,y,font;char label[40];const char *name;
    if(!dst || !pal || !c || !c->id) return;
    if(f1_theme==F1_THEME_90S) { F1RenderQualyCard90(dst,pal,c,cockpit); return; }
    if(cockpit) {
        palette(pal);x=F1_CARD_COMPACT_X;y=F1_CARD_Y_COCKPIT;
        di=driver(c->id);team=di<0?-1:f1_drivers[di].team;
        clipRight=x+F1_CARD_COMPACT_W;background(dst,x,y,F1_CARD_COMPACT_W,F1_CARD_COMPACT_H);
        bar(dst,x,y,26,23,badgeBlack);
        if(c->pos) sprintf(label,"%d",c->pos);else strcpy(label,"-");
        text(dst,x+4,y+3,label,3,-1,0);
        text(dst,x+30,y+3,di<0?"???":f1_drivers[di].abbr,3,-1,0);
        logo(dst,x+82,y+4,team);
        if(c->pit) strcpy(label,"IN PIT");
        else if(c->outlap) strcpy(label,"OUT LAP");
        else if(c->post) F1QTime(label,c->last,0);
        else if(c->showGap) F1QDelta(label,c->gap);
        else F1QLiveTime(label,c->running);
        clipRight=x+196;text(dst,x+104,y+5,label,3,c->showGap?sectorcol[c->gap<=0?1:2]:-1,0);
        clipRight=x+F1_CARD_COMPACT_W-4;
        di=driver(c->leaderId);name=di<0?"???":f1_drivers[di].last;
        text(dst,x+199,y+2,name,1,-1,0);
        if(c->pole) strcpy(label,"POLE");
        else if(c->post && c->hasGap) F1QDelta(label,c->gap);
        else F1QTime(label,c->leaderBest,0);
        text(dst,x+199,y+13,label,1,c->pole?sectorcol[3]:-1,0);
        clipRight=x+F1_CARD_COMPACT_W;
        bar(dst,x,y+24,F1_CARD_COMPACT_W,2,team<0?sectorcol[0]:teamcol[team]);
        for(i=0;i<3;i++) {
            sprintf(label,"S%d",i+1);text(dst,x+8+i*90+34,y+29,label,0,sectorcol[c->color[i]],0);
            bar(dst,x+8+i*90,y+43,86,3,sectorcol[c->color[i]]);
        }
        return;
    }
    palette(pal);x=F1_CARD_X;y=cockpit?F1_CARD_Y_COCKPIT:F1_CARD_Y_TV;clipRight=x+F1_CARD_W;
    di=driver(c->id);team=di<0?-1:f1_drivers[di].team;
    background(dst,x,y,F1_CARD_W,F1_CARD_H);
    bar(dst,x,y,26,22,nearest(pal,0,0,0));
    if(c->pos) sprintf(label,"%d",c->pos);else strcpy(label,"-");
    text(dst,x+4,y+3,label,3,-1,0);
    name=di<0?"UNKNOWN":f1_drivers[di].last;font=textwidth(name,3)>151?0:3;
    clipRight=x+F1_CARD_W-27;text(dst,x+30,y+3,name,font,-1,0);
    logo(dst,x+F1_CARD_W-22,y+4,team);clipRight=x+F1_CARD_W;
    bar(dst,x,y+23,F1_CARD_W,2,team<0?sectorcol[0]:teamcol[team]);
    if(c->pit) strcpy(label,"IN PIT");
    else if(c->outlap) strcpy(label,"OUT LAP");
    else if(c->post) F1QTime(label,c->last,0);
    else if(c->showGap) F1QDelta(label,c->gap);
    else F1QLiveTime(label,c->running);
    clipRight=x+107;text(dst,x+8,y+34,label,3,c->showGap?sectorcol[c->gap<=0?1:2]:-1,0);
    clipRight=x+F1_CARD_W-6;
    if(c->pole) strcpy(label,"POLE");
    else if(c->post && c->hasGap) F1QDelta(label,c->gap);
    else F1QTime(label,c->leaderBest,0);
    text(dst,x+112,y+32,label,0,c->pole?sectorcol[3]:-1,0);
    if(!c->pole && c->leaderId) {
        di=driver(c->leaderId);text(dst,x+112,y+47,di<0?"???":f1_drivers[di].last,1,-1,0);
    }
    clipRight=x+F1_CARD_W;
    for(i=0;i<3;i++) {
        sprintf(label,"S%d",i+1);
        text(dst,x+8+i*66+24,y+61,label,0,sectorcol[c->color[i]],0);
        bar(dst,x+8+i*66,y+77,62,3,sectorcol[c->color[i]]);
    }
}

/* Rotating, transparent local map. Road segments are clipped independently:
   disjoint visible portions are never joined across an off-screen bend. */
static unsigned char mapImage[640*180];
static int fullMapDrawing;
/* Transparent track-only cache. Never retain gameplay pixels or car dots. */
static unsigned char fullMapLayer[140*140],fullMapMask[140*140],fullMapPal[768];
static F1MapPoint fullMapTrack[2048],fullMapPits[512];
static int fullMapValid,fullMapCount,fullMapPitCount,fullMapHasPits,fullMapCapture;
static double fullMapScale,fullMapOx,fullMapOy;

static void mapPixel(int x,int y,int color)
{
    int dx=x-(F1_MAP_X+70),dy=y-(F1_MAP_Y+70);
    if((fullMapDrawing || dx*dx+dy*dy<=68*68) && x>=F1_MAP_X && x<F1_MAP_X+140 && y>=F1_MAP_Y && y<F1_MAP_Y+140)
    {
        mapImage[y*640+x]=(unsigned char)color;
        if(fullMapCapture) fullMapMask[(y-F1_MAP_Y)*140+x-F1_MAP_X]=1;
    }
}
static int mapZoom;
static const double mapZoomScale[3]={1.0,1.25,2.0};
void F1CycleMapZoom(void) { mapZoom=(mapZoom+1)%3; }
static void mapPoint(double x,double y,const F1MapCar *focus,double co,double si,double *sx,double *sy)
{
    double dx=-(x-focus->x),dy=y-focus->y;
    *sx=F1_MAP_X+70+(dx*co-dy*si)*0.0825;
    *sy=F1_MAP_Y+70+(dx*si+dy*co)*0.0825;
}
static void mapRoadExact(double ax,double ay,double bx,double by,int radius,int color,int stripe)
{
    int x,y,x0,x1,y0,y1;double vx=bx-ax,vy=by-ay,den=vx*vx+vy*vy,t,dx,dy;
    x0=(int)(ax<bx?ax:bx)-radius;x1=(int)(ax>bx?ax:bx)+radius;
    y0=(int)(ay<by?ay:by)-radius;y1=(int)(ay>by?ay:by)+radius;
    if(x0<F1_MAP_X) x0=F1_MAP_X;
    if(x1>F1_MAP_X+139) x1=F1_MAP_X+139;
    if(y0<F1_MAP_Y) y0=F1_MAP_Y;
    if(y1>F1_MAP_Y+139) y1=F1_MAP_Y+139;
    for(y=y0;y<=y1;y++) for(x=x0;x<=x1;x++) {
        t=den?((x-ax)*vx+(y-ay)*vy)/den:0;if(t<0) t=0;if(t>1) t=1;
        dx=x-ax-t*vx;dy=y-ay-t*vy;
        if(dx*dx+dy*dy<=radius*radius) mapPixel(x,y,stripe && ((x+y)/4)%2?stripe:color);
    }
}
/* Like the original key-8 map, rasterize with integer pixel stamps.
   Connect stamps with Bresenham so this HUD retains continuous roads. */
static int mapSpanReady,mapDiskSpan[17][33],mapCircleSpan[140];
static void mapSpans(void)
{
    int r,y,x,d;
    if(mapSpanReady) return;
    for(r=0;r<=16;r++) for(y=-r;y<=r;y++) {
        x=r;while(x*x+y*y>r*r)x--;
        mapDiskSpan[r][y+16]=x;
    }
    for(y=0;y<140;y++) {
        d=y-70;x=68;
        while(x>=0 && x*x+d*d>68*68)x--;
        mapCircleSpan[y]=x;
    }
    mapSpanReady=1;
}
static int mapOutcode(int x,int y,int xmin,int ymin,int xmax,int ymax)
{
    return (x<xmin?1:x>xmax?2:0)|(y<ymin?4:y>ymax?8:0);
}
static void mapStamp(int x,int y,int radius,int color,int stripe)
{
    int row,left,right,w,half,xx,yy;
    for(yy=-radius;yy<=radius;yy++) {
        row=y+yy;if(row<F1_MAP_Y || row>=F1_MAP_Y+140)continue;
        w=mapDiskSpan[radius][yy+16];left=x-w;right=x+w;
        if(left<F1_MAP_X)left=F1_MAP_X;
        if(right>F1_MAP_X+139)right=F1_MAP_X+139;
        if(!fullMapDrawing) {
            half=mapCircleSpan[row-F1_MAP_Y];if(half<0)continue;
            if(left<F1_MAP_X+70-half)left=F1_MAP_X+70-half;
            if(right>F1_MAP_X+70+half)right=F1_MAP_X+70+half;
        }
        if(left>right)continue;
        if(!stripe)memset(mapImage+row*640+left,color,right-left+1);
        else for(xx=left;xx<=right;xx++)mapImage[row*640+xx]=(unsigned char)(((xx+row)/4)%2?stripe:color);
    }
}
static void mapRoad(double ax,double ay,double bx,double by,int radius,int color,int stripe)
{
    int x0,y0,x1,y1,c0,c1,c,x,y,xmin,xmax,ymin,ymax,dx,dy,sx,sy,err,e2,guard;
    /* Preserve the already validated, cached 1x layer byte for byte. */
    if(fullMapDrawing && !mapZoom) {mapRoadExact(ax,ay,bx,by,radius,color,stripe);return;}
    mapSpans();x0=(int)ax;y0=(int)ay;x1=(int)bx;y1=(int)by;
    xmin=F1_MAP_X-radius;xmax=F1_MAP_X+139+radius;
    ymin=F1_MAP_Y-radius;ymax=F1_MAP_Y+139+radius;
    /* Clip the centre line before walking it; off-screen roads never
       generate thousands of invisible stamps or join unrelated bends. */
    for(guard=0;guard<8;guard++) {
        c0=mapOutcode(x0,y0,xmin,ymin,xmax,ymax);c1=mapOutcode(x1,y1,xmin,ymin,xmax,ymax);
        if(!(c0|c1))break;
        if(c0&c1)return;
        c=c0?c0:c1;
        if(c&8){y=ymax;x=x0+(int)((long)(x1-x0)*(y-y0)/(y1-y0));}
        else if(c&4){y=ymin;x=x0+(int)((long)(x1-x0)*(y-y0)/(y1-y0));}
        else if(c&2){x=xmax;y=y0+(int)((long)(y1-y0)*(x-x0)/(x1-x0));}
        else{x=xmin;y=y0+(int)((long)(y1-y0)*(x-x0)/(x1-x0));}
        if(c==c0){x0=x;y0=y;}else{x1=x;y1=y;}
    }
    if(guard==8)return;
    dx=abs(x1-x0);dy=-abs(y1-y0);sx=x0<x1?1:-1;sy=y0<y1?1:-1;err=dx+dy;
    for(;;) {
        mapStamp(x0,y0,radius,color,stripe);
        if(x0==x1 && y0==y1)break;
        e2=2*err;if(e2>=dy){err+=dy;x0+=sx;}if(e2<=dx){err+=dx;y0+=sy;}
    }
}
static void mapCar(const F1MapCar *car,const F1MapCar *focus,double co,double si,int selected)
{
    int di,team,x,y,xx,yy,bx,by,col,pass,dx,dy,wheel,body;double sx,sy,ca,sa;char label[16];
    mapPoint(car->x,car->y,focus,co,si,&sx,&sy);
    if(sx<F1_MAP_X-12 || sx>F1_MAP_X+152 || sy<F1_MAP_Y-12 || sy>F1_MAP_Y+152) return;
    x=(int)sx;y=(int)sy;di=driver(car->id);team=di<0?-1:f1_drivers[di].team;
    col=team<0?white[3][0]:teamcol[team];ca=cos(car->angle-focus->angle);sa=sin(car->angle-focus->angle);
    for(pass=selected?0:1;pass<2;pass++) for(yy=-8;yy<=8;yy++) for(xx=-4;xx<=4;xx++) {
        wheel=(xx<=-3 || xx>=3) && ((yy>=-6 && yy<=-3) || yy>=5);
        body=(xx>=-1 && xx<=1) || (yy>=-2 && yy<=4 && xx>=-3 && xx<=3) || (yy==7 && xx>=-2 && xx<=2);
        if(!wheel && !body) continue;
        bx=x+(int)(xx*ca-yy*sa);by=y+(int)(xx*sa+yy*ca);
        if(!pass) {for(dy=-1;dy<=1;dy++) for(dx=-1;dx<=1;dx++) mapPixel(bx+dx,by+dy,white[3][0]);}
        else mapPixel(bx,by,wheel?sectorcol[0]:col);
    }
    /* Labels remain upright while cars and track rotate. */
    if(y>=F1_MAP_Y+12 && y<F1_MAP_Y+128 && x>=F1_MAP_X+12 && x<F1_MAP_X+128) {
        text(mapImage,x-textwidth(di<0?"???":f1_drivers[di].abbr,5)/2,y-16,di<0?"???":f1_drivers[di].abbr,5,-1,0);
        if(car->pos>0) sprintf(label,"%d",car->pos);else strcpy(label,"-");
        for(yy=-3;yy<=3;yy++) for(xx=-4;xx<=4;xx++) if(xx*xx+yy*yy<=16) mapPixel(x+xx,y+yy,sectorcol[0]);
        text(mapImage,x-textwidth(label,4)/2,y-3,label,4,-1,0);
    }
}
void F1RenderMap(unsigned char *dst,const unsigned char *pal,const F1MapPoint *track,int count,
    const F1MapPoint *pits,int pitcount,const F1MapCar *cars,int n,int focusId)
{
    int i,j,fi=-1,x,y,road,edge,pit,gold;double co,si,ax,ay,bx,by;const F1MapCar *focus;
    /* 'focused' index and pointer are kept separate for old Watcom C. */
    if(!dst || !pal || !track || !cars || count<2 || count>2048 || n<1 || n>26 || pitcount<0 || pitcount>512) return;
    for(i=0;i<n;i++) if(cars[i].id==focusId) fi=i;
    if(fi<0) return;
    focus=&cars[fi];
    fullMapDrawing=0;
    palette(pal);clipRight=F1_MAP_X+140;
    road=nearest(pal,48,56,65);edge=nearest(pal,74,83,92);pit=nearest(pal,34,34,34);gold=nearest(pal,110,84,30);
    co=cos(3.141592653589793-focus->angle);si=sin(3.141592653589793-focus->angle);
    for(y=F1_MAP_Y;y<F1_MAP_Y+140;y++) memcpy(mapImage+y*640+F1_MAP_X,dst+y*640+F1_MAP_X,140);
    for(j=0;j<2;j++) for(i=0;i<count;i++) {
        mapPoint(track[i].x,track[i].y,focus,co,si,&ax,&ay);
        mapPoint(track[(i+1)%count].x,track[(i+1)%count].y,focus,co,si,&bx,&by);
        mapRoad(ax,ay,bx,by,j?1:16,j?edge:road,0);
    }
    if(pits) for(i=1;i<pitcount;i++) {
        mapPoint(pits[i-1].x,pits[i-1].y,focus,co,si,&ax,&ay);
        mapPoint(pits[i].x,pits[i].y,focus,co,si,&bx,&by);
        mapRoad(ax,ay,bx,by,4,pit,gold);
    }
    for(i=0;i<n;i++) if(i!=fi && !cars[i].out) mapCar(&cars[i],focus,co,si,0);
    mapCar(focus,focus,co,si,1);
    for(y=F1_MAP_Y;y<F1_MAP_Y+140;y++) for(x=F1_MAP_X;x<F1_MAP_X+140;x++) {
        int dx=x-(F1_MAP_X+70),dy=y-(F1_MAP_Y+70);
        if(dx*dx+dy*dy<=68*68) dst[y*640+x]=mapImage[y*640+x];
    }
}

/* Fixed orientation and aspect ratio; only team-coloured dots, no labels. */
void F1RenderFullMap(unsigned char *dst,const unsigned char *pal,const F1MapPoint *track,int count,
    const F1MapPoint *pits,int pitcount,const F1MapCar *cars,int n,int focusId)
{
    double minx,maxx,miny,maxy,scale,span,ox,oy,ax,ay;
    double focusX=0,focusY=0,bestDistance=-1;
    int i,j,k,x,y,di,col,hit,focusIndex=-1,order[26],px[26],py[26];
    if(!dst || !pal || !track || !cars || count<2 || count>2048 || n<1 || n>26 || pitcount<0 || pitcount>512) return;
    /* Keep the selected marker on the map centreline, avoiding lateral
       movement from the car's physical position and chassis motion. */
    for(i=0;i<n;i++) if(cars[i].id==focusId) {focusIndex=i;break;}
    if(focusIndex>=0) {
        focusX=cars[focusIndex].x;focusY=cars[focusIndex].y;
        for(i=0;i<count+(pits && pitcount>1?pitcount-1:0);i++) {
            double dx,dy,length,t,qx,qy,distance;
            const F1MapPoint *a,*b;
            if(i<count) {a=&track[i];b=&track[(i+1)%count];}
            else {if(!pits)break;a=&pits[i-count];b=a+1;}
            dx=b->x-a->x;dy=b->y-a->y;length=dx*dx+dy*dy;
            if(length<=0)continue;
            t=((cars[focusIndex].x-a->x)*dx+(cars[focusIndex].y-a->y)*dy)/length;
            if(t<0)t=0;
            if(t>1)t=1;
            qx=a->x+t*dx;qy=a->y+t*dy;
            dx=cars[focusIndex].x-qx;dy=cars[focusIndex].y-qy;distance=dx*dx+dy*dy;
            if(bestDistance<0 || distance<bestDistance) {bestDistance=distance;focusX=qx;focusY=qy;}
        }
    }
    hit=!mapZoom && fullMapValid && count==fullMapCount && pitcount==fullMapPitCount &&
        (pits!=NULL)==fullMapHasPits && !memcmp(fullMapPal,pal,768) &&
        !memcmp(fullMapTrack,track,count*sizeof(*track)) &&
        (!pits || !memcmp(fullMapPits,pits,pitcount*sizeof(*pits)));
    if(hit) {scale=fullMapScale;ox=fullMapOx;oy=fullMapOy;}
    else {
    minx=maxx=track[0].x; miny=maxy=track[0].y;
    for(i=0;i<count+pitcount;i++) {
        if(i>=count && !pits) break;
        ax=i<count?track[i].x:pits[i-count].x; ay=i<count?track[i].y:pits[i-count].y;
        if(ax<minx) minx=ax;
        if(ax>maxx) maxx=ax;
        if(ay<miny) miny=ay;
        if(ay>maxy) maxy=ay;
    }
    span=maxx-minx; if(maxy-miny>span) span=maxy-miny;
    if(span<=0) return;
    scale=112.0/span*mapZoomScale[mapZoom];
    ax=(maxx+minx)/2;ay=(maxy+miny)/2;
    if(mapZoom && focusIndex>=0) {ax=focusX;ay=focusY;}
    ox=F1_MAP_X+70+ax*scale; oy=F1_MAP_Y+70-ay*scale;
    }
    palette(pal); clipRight=F1_MAP_X+140; fullMapDrawing=1;
    for(y=F1_MAP_Y;y<F1_MAP_Y+140;y++) memcpy(mapImage+y*640+F1_MAP_X,dst+y*640+F1_MAP_X,140);
    if(hit) {
        for(y=0;y<140;y++) for(x=0;x<140;x++)
            if(fullMapMask[y*140+x]) mapImage[(y+F1_MAP_Y)*640+x+F1_MAP_X]=fullMapLayer[y*140+x];
    } else {
    fullMapCapture=!mapZoom;
    if(fullMapCapture) memset(fullMapMask,0,sizeof(fullMapMask));
    for(i=0;i<count;i++) {
        j=(i+1)%count;
        mapRoad(ox-track[i].x*scale,oy+track[i].y*scale,ox-track[j].x*scale,oy+track[j].y*scale,2,badgeWhite,0);
    }
    if(pits) for(i=1;i<pitcount;i++) mapRoad(ox-pits[i-1].x*scale,oy+pits[i-1].y*scale,ox-pits[i].x*scale,oy+pits[i].y*scale,1,nearest(pal,150,120,40),0);
    if(fullMapCapture) {
        for(y=0;y<140;y++) memcpy(fullMapLayer+y*140,mapImage+(y+F1_MAP_Y)*640+F1_MAP_X,140);
        memcpy(fullMapTrack,track,count*sizeof(*track));
        if(pits) memcpy(fullMapPits,pits,pitcount*sizeof(*pits));
        memcpy(fullMapPal,pal,768);fullMapCount=count;fullMapPitCount=pitcount;
        fullMapHasPits=pits!=NULL;fullMapScale=scale;fullMapOx=ox;fullMapOy=oy;fullMapValid=1;
    }
    fullMapCapture=0;
    }
    for(i=0;i<n;i++) { px[i]=(int)(ox-cars[i].x*scale); py[i]=(int)(oy+cars[i].y*scale); }
    if(focusIndex>=0) {px[focusIndex]=(int)(ox-focusX*scale);py[focusIndex]=(int)(oy+focusY*scale);}
    /* Backmarkers first; better positions remain above overlapping dots. */
    for(i=0;i<n;i++)order[i]=i;
    for(i=1;i<n;i++) {
        k=order[i];j=i;
        while(j>0 && (cars[order[j-1]].pos>0?cars[order[j-1]].pos:99)<(cars[k].pos>0?cars[k].pos:99)) {
            order[j]=order[j-1];j--;
        }
        order[j]=k;
    }
    for(k=0;k<n;k++) {
        i=order[k];
        if(cars[i].out && cars[i].id!=focusId) continue;
        di=driver(cars[i].id);col=di<0?badgeWhite:teamcol[f1_drivers[di].team];
        mapRoad(px[i],py[i],px[i],py[i],cars[i].id==focusId?3:2,col,0);
    }
    for(y=F1_MAP_Y;y<F1_MAP_Y+140;y++) memcpy(dst+y*640+F1_MAP_X,mapImage+y*640+F1_MAP_X,140);
    fullMapDrawing=0;
}

#include "f1r90.inc"
#include "f1modern.inc"

/* Centre actual visible bounds inside the pit-card header. */
static void pitHeaderText(unsigned char *dst,int x,int y,int w,int h,const char *s,int font,int leftAligned)
{
    int c,xx,yy,advance=0,left=1000,right=-1,top=16,bottom=-1;
    const unsigned char *glyph,*width=font==1?f1_width1:f1_width2;
    const char *q=s;
    while(*q) {
        c=(unsigned char)*q++;if(c<32 || c>=127)continue;c-=32;
        glyph=font==1?f1_font1[c]:f1_font2[c];
        for(yy=0;yy<16;yy++)for(xx=0;xx<width[c];xx++)if(glyph[yy*20+xx]) {
            if(advance+xx<left)left=advance+xx;
            if(advance+xx>right)right=advance+xx;
            if(yy<top)top=yy;
            if(yy>bottom)bottom=yy;
        }
        advance+=width[c];
    }
    if(right<left)return;
    text(dst,x+(leftAligned?0:(w-(right-left+1))/2)-left,y+(h-(bottom-top+1))/2-top,s,font,-1,0);
}

/* Bottom-right pit cards, stacked upwards. Overflow continues to the left. */
void F1RenderPitDisplays(unsigned char *dst,const unsigned char *pal,const F1PitDisplay *cards,int count)
{
    int i,di,team,x,y,j,k,font,modern,px,available;
    char name[F1_NAME_LEN],number[16],elapsed[24];
    palette(pal);modern=f1_theme!=F1_THEME_90S;
    if(!modern)palette90(pal);
    for(i=0;i<count;i++) {
        x=640-8-104-(i/10)*108;y=F1_TV_SLOT_BOTTOM-34-(i%10)*37;
        if(x<F1_PANEL_X+F1_PANEL_W+4)break;
        clipRight=x+100;di=driver(cards[i].id);team=di<0?-1:f1_drivers[di].team;
        if(team<0 || team>=F1_TEAM_COUNT)team=-1;
        background(dst,x,y,104,34);
        bar(dst,x,y,16,15,modern?badgeBlack:c90yellow);
        sprintf(number,"%d",cards[i].position);
        if(modern)pitHeaderText(dst,x,y,16,15,number,1,0);
        else tc90(dst,x,y,16,15,number,F90_NAME,c90ink,NULL);
        px=x+18;
        if(modern && team>=0) {logo(dst,px,y+1,team);px+=18;}
        strcpy(name,di<0?"UNKNOWN":f1_drivers[di].last);
        for(j=0;name[j];j++)name[j]=(char)toupper((unsigned char)name[j]);
        font=1;available=x+100-px;
        if(modern && textwidth(name,font)>available)font=2;
        k=(int)strlen(name);while(modern && k>0 && textwidth(name,font)>available)name[--k]=0;
        if(modern)pitHeaderText(dst,px,y,available,15,name,font,1);
        else t90(dst,px,vc90(y,15,F90_NAME),name,F90_NAME,-1,NULL,0,available);
        bar(dst,x,y+15,104,2,modern?(team>=0?teamcol[team]:badgeWhite):c90yellow);
        if(modern)text(dst,x+4,y+19,"IN PIT",1,-1,0);
        else t90(dst,x+4,vc90(y+17,17,F90_NAME),"IN PIT",F90_NAME,-1,NULL,0,40);
        sprintf(elapsed,"%lu.%lu",cards[i].elapsed/1000,(cards[i].elapsed%1000)/100);
        if(modern)text(dst,x+100-textwidth(elapsed,0),y+18,elapsed,0,-1,0);
        else {int tw=w90(elapsed,F90_NAME);if(tw>52)tw=52;
            t90(dst,x+100-tw,vc90(y+17,17,F90_NAME),elapsed,F90_NAME,-1,NULL,0,52);}
    }
    clipRight=640;
}

static long F1CombinedValidGrid(unsigned long t) {return t && !(t&0xf0000000UL) && t<=3600000UL?(long)t:0;}
/* Full panel reskin; native hit boxes and event handling remain untouched. */
static void F1GridButton(unsigned char *dst,int x,int y,int w,int h,const char *label,int fill,int edge,int ink)
{
    bar(dst,x,y,w,h,fill);bar(dst,x,y,w,1,edge);bar(dst,x,y,1,h,edge);
    bar(dst,x,y+h-1,w,1,edge);bar(dst,x+w-1,y,1,h,edge);
    clipRight=x+w-3;text(dst,x+(w-textwidth(label,0))/2,y+(h-9)/2,label,0,ink,0);
}
static void F1GridArrow(unsigned char *dst,int x,int y,int up,int twice,int color)
{
    int a,k,w;for(a=0;a<(twice?2:1);a++)for(k=0;k<5;k++) {
        w=1+k*2;bar(dst,x+7-k,y+(up?k:4-k)+a*6,w,1,color);
    }
}
#include "f1grid90.inc"

void F1RenderGridMenu(unsigned char *dst,const unsigned char *pal,const unsigned char *ids,
                      const unsigned long *times,int count,long scroll,long extent,
                      const char *country,const char *circuit,int laps,const int *focus)
{
    int i,j,start,n,y,di,team,col,bg,light,ink,muted,rail,accent,font,thumb,offset;long range,pole,t;
    char label[128],surname[F1_NAME_LEN];
    if(!dst || !pal || !ids || !times || count<1 || count>40)return;
    if(f1_theme==F1_THEME_90S){F1RenderGridMenu90(dst,pal,ids,times,count,scroll,extent,country,circuit,laps);return;}
    palette(pal);clipRight=494;bg=nearest(pal,17,18,25);light=nearest(pal,210,210,210);ink=nearest(pal,15,15,15);
    muted=nearest(pal,30,32,42);rail=nearest(pal,50,53,64);accent=nearest(pal,230,28,45);
    range=extent-320;if(range<1)range=1;
    if(scroll<0)scroll=0;
    if(scroll>range)scroll=range;
    start=count>10?(int)(scroll*(count-10)/range):0;n=count-start;if(n>10)n=10;
    pole=0;di=ids[0]&63;if(di>=1 && di<=40)pole=F1CombinedValidGrid(times[di-1]);
    /* Clear list/header only; actual native controls draw separately. */
    /* Whole native grid body, including scrollbar backing; controls follow. */
    bar(dst,140,58,356,379,bg);
    sprintf(label,"GRAND PRIX OF %.40s",country?country:"UNKNOWN");
    for(i=0;label[i];i++)label[i]=(char)toupper((unsigned char)label[i]);
    font=textwidth(label,3)>340?0:3;
    text(dst,318-textwidth(label,font)/2,66,label,font,-1,0);
    if(laps>0)sprintf(label,"%.40s (%d Laps)",circuit?circuit:"Unknown",laps);
    else sprintf(label,"%.40s",circuit?circuit:"Unknown");
    text(dst,318-textwidth(label,0)/2,87,label,0,-1,0);
    bar(dst,148,105,319,2,accent);
    text(dst,154,110,"STARTING GRID",0,-1,0);
    clipRight=467;
    text(dst,153,128,"POS",1,-1,0);text(dst,174,128,"DRIVER",1,-1,0);
    text(dst,315,128,"TEAM",1,-1,0);text(dst,443,128,"TIME",1,-1,0);
    for(i=0;i<n;i++) {
        j=start+i;y=147+i*27;di=driver(ids[j]&63);team=di<0?-1:f1_drivers[di].team;col=j==0?ink:-1;
        if(j==0)bar(dst,148,y-2,319,26,light);
        else if(i&1)bar(dst,148,y-2,319,26,muted);
        sprintf(label,"%d",j+1);clipRight=171;text(dst,153,y+4,label,2,col,0);
        clipRight=295;
        if(di>=0) {
            text(dst,174,y,f1_drivers[di].first,5,col,0);strcpy(surname,f1_drivers[di].last);
            for(j=0;surname[j];j++)surname[j]=(char)toupper((unsigned char)surname[j]);
            text(dst,174,y+12,surname,0,col,0);
        } else text(dst,174,y+5,"UNKNOWN",0,col,0);
        clipRight=398;
        if(f1_theme!=F1_THEME_90S)logo(dst,298,y+6,team);
        text(dst,315,y+5,team>=0?f1_team_names[team]:"UNKNOWN",0,col,0);
        j=start+i;di=ids[j]&63;t=di>=1 && di<=40?F1CombinedValidGrid(times[di-1]):0;
        if(!t)strcpy(label,"NO TIME");else if(j==0 || !pole)F1QTime(label,t,0);else F1QDelta(label,t-pole);
        clipRight=465;text(dst,465-textwidth(label,1),y+5,label,1,col,0);
    }
    (void)focus;(void)rail;(void)thumb;(void)offset;
    clipRight=640;
}

void F1RenderGridControl(unsigned char *dst,const unsigned char *pal,const int *rect,int kind,int focused,long scroll,long extent)
{
    int x,y,w,h,base,edge,fill,accent,thumb,offset;long range;
    if(!dst || !pal || !rect)return;
    x=rect[0];y=rect[1];w=rect[2]-x+1;h=rect[3]-y+1;
    if(x<0 || y<0 || w<1 || h<1 || x+w>640 || y+h>480)return;
    palette(pal);base=nearest(pal,42,45,55);edge=nearest(pal,190,190,190);accent=nearest(pal,230,28,45);
    if(f1_theme==F1_THEME_90S){palette90(pal);base=c90navy;edge=c90gold;accent=c90yellow;}
    fill=focused?accent:base;clipRight=x+w;
    if(kind<3 && f1_theme==F1_THEME_90S){
        bar(dst,x,y,w,h,fill);bar(dst,x,y,w,1,edge);bar(dst,x,y+h-1,w,1,edge);
        tc90(dst,x+2,y,w-4,h,kind==0?"PRINT SETUP":kind==1?"PRINT":"OK",F90_HEAD,focused?c90ink:-1,NULL);
        clipRight=640;return;
    }
    if(kind<3)F1GridButton(dst,x,y,w,h,kind==0?"PRINT SETUP":kind==1?"PRINT":"OK",fill,edge,-1);
    else if(kind<7) {
        F1GridButton(dst,x,y,w,h,"",fill,edge,-1);
        F1GridArrow(dst,x+(w-15)/2,y+(h-(kind==3 || kind==6?11:5))/2,kind<5,kind==3 || kind==6,edge);
    } else {
        /* Native rail has a narrow hit rectangle inside a wider painted track. */
        if(x==479 && w==10){x=475;y=150;w=20;h=240;clipRight=x+w;}
        bar(dst,x,y,w,h,base);range=extent-320;if(range<1)range=1;
        if(scroll<0)scroll=0;
        if(scroll>range)scroll=range;
        thumb=extent>0?(int)((long)h*320/extent):h;if(thumb<8)thumb=8;if(thumb>h)thumb=h;
        offset=(int)(scroll*(h-thumb)/range);bar(dst,x+1,y+offset,w>2?w-2:w,thumb,accent);
    }
    clipRight=640;
}

#include "f1playerui.h"
void F1RenderPlayerStrategy(unsigned char *dst,const unsigned char *pal,const F1StrategyOption *options,int n,int selected,int id,int laps,const F1StrategyOption *original)
{
 int i,j,y,bg,ink,muted,accent,row,tyres[3],start,end,total,labelcol;char label[128];
 const F1StrategyOption *o;
 palette(pal);clipRight=640;bg=nearest(pal,17,20,28);ink=nearest(pal,240,243,248);
 muted=nearest(pal,155,168,188);accent=nearest(pal,225,35,50);row=nearest(pal,33,40,54);
 tyres[0]=nearest(pal,215,215,215);tyres[1]=nearest(pal,245,224,0);tyres[2]=nearest(pal,255,48,48);
 memset(dst,bg,307200);bar(dst,20,15,600,3,accent);
 sprintf(label,"PLAYER STRATEGY  |  CAR %d  |  %d LAPS",id,laps);text(dst,25,30,label,3,ink,0);
 text(dst,30,101,"STOPS",0,muted,0);text(dst,85,101,"STRATEGY / PIT LAPS",0,muted,0);text(dst,550,101,"FUEL",0,muted,0);
 for(i=0;i<=n;i++){
  y=i?125+(i-1)*24:58;bar(dst,20,y-3,600,i?23:39,i==selected?accent:row);
  if(!i){
   text(dst,30,y,"KEEP ORIGINAL GAME STRATEGY",3,ink,0);
   if(original && original->count)sprintf(label,"%d STOPS  |  %s  |  FUEL %d L",original->stops,original->sequence,original->fuel);
   else strcpy(label,"NATIVE PLAN DETAILS UNAVAILABLE");
   text(dst,30,76,label,0,ink,0);continue;
  }
  o=&options[i-1];sprintf(label,"%d",o->stops);text(dst,40,y,label,3,ink,0);
  total=0;start=85;
  for(j=0;j<o->count && j<4 && laps>0;j++){
   total+=o->length[j];end=85+(int)((long)total*420/laps);
   if(end>505)end=505;
   if(o->compound[j]>2 || end<=start)continue;
   bar(dst,start,y+6,end-start,13,tyres[o->compound[j]]);
   labelcol=o->compound[j]==2?ink:bg;
   sprintf(label,"%c%u","HMS"[o->compound[j]],(unsigned)o->length[j]);
   if(textwidth(label,0)<end-start-2)text(dst,start+(end-start-textwidth(label,0))/2,y+6,label,0,labelcol,0);
   else if(end-start>6){label[1]=0;text(dst,start+1,y+6,label,0,labelcol,0);}
   if(j<o->count-1){bar(dst,end-1,y+4,1,16,bg);sprintf(label,"%d",total);text(dst,end-textwidth(label,0)/2,y-5,label,0,ink,0);}
   start=end;
  }
  sprintf(label,"%d L",o->fuel);text(dst,550,y,label,3,ink,0);
 }
 if(selected==0 && original && original->count){sprintf(label,"PIT LAPS: %s",original->pits);text(dst,25,372,label,0,ink,0);}
 if(selected>0){sprintf(label,"PIT LAPS: %s",options[selected-1].pits);text(dst,25,372,label,0,ink,0);}
 text(dst,25,391,"H HARD",0,tyres[0],0);text(dst,95,391,"M MEDIUM",0,tyres[1],0);text(dst,185,391,"S SOFT",0,tyres[2],0);
 text(dst,270,391,"FUEL = INITIAL LOAD IN GAME LAPS",0,muted,0);
 text(dst,25,418,"UP / DOWN: SELECT    ENTER: CONFIRM    ESC: ORIGINAL",0,ink,0);
 text(dst,25,446,"Race waits until you confirm your strategy.",0,muted,0);clipRight=640;
}

void F1RenderPlayerPitStrategy(unsigned char *dst,const unsigned char *pal,const F1StrategyOption *options,int n,int selected,int id,int recommended,int required)
{
 int i,y,bg,ink,muted,accent,row,tyres[3],c,col;char label[128];
 palette(pal);clipRight=640;bg=nearest(pal,17,20,28);ink=nearest(pal,240,243,248);
 muted=nearest(pal,155,168,188);accent=nearest(pal,225,35,50);row=nearest(pal,33,40,54);
 tyres[0]=nearest(pal,215,215,215);tyres[1]=nearest(pal,245,224,0);tyres[2]=nearest(pal,255,48,48);
 memset(dst,bg,307200);bar(dst,20,15,600,3,accent);
 sprintf(label,"PIT COMPOUND  |  CAR %d",id);text(dst,25,30,label,3,ink,0);
 bar(dst,20,55,600,23,selected==0?accent:row);
 sprintf(label,"KEEP PLANNED COMPOUND: %c","HMS"[recommended>=0 && recommended<3?recommended:0]);text(dst,30,58,label,3,ink,0);
 text(dst,30,88,"CHOOSE THE COMPOUND FOR THIS TYRE SET",0,muted,0);
 for(i=0;i<n;i++){
  y=112+i*48;c=options[i].compound[0];if(c<0 || c>2)continue;
  bar(dst,20,y-3,600,40,selected==i+1?accent:row);bar(dst,85,y+5,420,25,tyres[c]);
  col=c==2?ink:bg;sprintf(label,"%c  %s","HMS"[c],c==0?"HARD":c==1?"MEDIUM":"SOFT");text(dst,105,y+10,label,3,col,0);
 }
 text(dst,25,300,"The native pit menu controls fuel and remaining stops.",0,ink,0);
 text(dst,25,325,required?"A different compound is required before finishing the race.":"Zero more stops requires a second compound if not yet used.",0,muted,0);
 text(dst,25,418,"UP / DOWN: SELECT    ENTER: CONFIRM    ESC: KEEP PLAN",0,ink,0);
 text(dst,25,446,"The race waits while you select the tyres.",0,muted,0);clipRight=640;
}

#include "f1strategyfont.h"
static int F1StrategyTextWidth(const char *s,int font)
{
 int n=0,c,k=font==3?1:0;while(*s){c=(unsigned char)*s++;if(c>=32 && c<127)n+=strategyFontWidth[k][c-32];}return n;
}
static void F1StrategyText(unsigned char *dst,int x,int y,const char *s,int font,int color,int dim)
{
 int c,k=font==3?1:0,xx,yy;unsigned long bits;
 (void)dim;
 while(*s){c=(unsigned char)*s++;if(c<32 || c>=127)continue;c-=32;
  for(yy=0;yy<20 && y+yy<480;yy++){bits=strategyFontBits[k][c][yy];
   for(xx=0;xx<24 && x+xx<clipRight;xx++)if((bits&(1UL<<xx)) && x+xx>=0 && y+yy>=0)dst[(y+yy)*640+x+xx]=(unsigned char)color;
  }x+=strategyFontWidth[k][c];
 }
}
/* T20: GP2-inspired bevelled setup panel, rendered in the existing SVGA modal. */
static char strategyCircuitName[100]="Circuit";
static double strategyCircuitXY[4096];static int strategyCircuitCount;
void F1StrategyCircuitSet(const char *name,const double *xy,int count)
{
 strncpy(strategyCircuitName,name?name:"Circuit",99);strategyCircuitName[99]=0;
 strategyCircuitCount=xy && count>=2 && count<=2048?count:0;
 if(strategyCircuitCount)memcpy(strategyCircuitXY,xy,count*2*sizeof(double));
}
static void F1StrategyCircuitDraw(unsigned char *dst,int ink)
{
 double minx=1e30,maxx=-1e30,miny=1e30,maxy=-1e30,x,y,scale,dx,dy;
 int i,j,x0,y0,x1,y1,ax,ay,sx,sy,err,e2;char name[100];
 strcpy(name,strategyCircuitName);while(*name && F1StrategyTextWidth(name,0)>150)name[strlen(name)-1]=0;
 F1StrategyText(dst,422,72,name,0,ink,0);
 if(strategyCircuitCount<2){F1StrategyText(dst,422,114,"Map unavailable",0,ink,0);return;}
 for(i=0;i<strategyCircuitCount;i++){
  x=-strategyCircuitXY[i*2];y=strategyCircuitXY[i*2+1];
  if(x<minx)minx=x;if(x>maxx)maxx=x;if(y<miny)miny=y;if(y>maxy)maxy=y;
 }
 dx=maxx-minx;dy=maxy-miny;if(dx<1)dx=1;if(dy<1)dy=1;
 scale=140/dx;if(scale>58/dy)scale=58/dy;
 for(i=0;i<strategyCircuitCount;i++){
  j=(i+1)%strategyCircuitCount;
  x0=(int)(497+(-strategyCircuitXY[i*2]-(minx+maxx)/2)*scale);
  y0=(int)(121+(strategyCircuitXY[i*2+1]-(miny+maxy)/2)*scale);
  x1=(int)(497+(-strategyCircuitXY[j*2]-(minx+maxx)/2)*scale);
  y1=(int)(121+(strategyCircuitXY[j*2+1]-(miny+maxy)/2)*scale);
  ax=abs(x1-x0);ay=-abs(y1-y0);sx=x0<x1?1:-1;sy=y0<y1?1:-1;err=ax+ay;
  for(;;){bar(dst,x0,y0,2,2,ink);if(x0==x1 && y0==y1)break;e2=2*err;if(e2>=ay){err+=ay;x0+=sx;}if(e2<=ax){err+=ax;y0+=sy;}}
 }
}
static int F1StrategyInset(int y,int h)
{
 int edge=y<h-1-y?y:h-1-y;
 return edge==0?3:edge<3?1:0;
}
static void F1StrategyRound(unsigned char *dst,int x,int y,int w,int h,int fill)
{
 int row,inset;for(row=0;row<h;row++){
  inset=F1StrategyInset(row,h);if(w>2*inset)bar(dst,x+inset,y+row,w-2*inset,1,fill);
 }
}
static void F1StrategyBevel(unsigned char *dst,int x,int y,int w,int h,int fill,int light,int shadow)
{
 int row,col,inset,color;
 F1StrategyRound(dst,x+2,y+2,w,h,shadow);
 for(row=0;row<h;row++){
  inset=F1StrategyInset(row,h);
  for(col=inset;col<w-inset;col++){
   color=row<2 || col<inset+2?light:row>=h-2 || col>=w-inset-2?shadow:fill;
   bar(dst,x+col,y+row,1,1,color);
  }
 }
}
static void F1StrategyField(unsigned char *dst,int x,int y,int w,const char *value,
 int focused,int side,int pressed,int fill,int light,int shadow,int ink,int red)
{
 int button,cx,cy,down,symbol=nearest(lastpal,250,250,244);
 for(button=0;button<2;button++){
  cx=button?x+w-20:x;down=focused && side==button && pressed;
  F1StrategyBevel(dst,cx,y+2,20,20,focused && side==button?red:fill,down?shadow:light,down?light:shadow);
  cx+=10+(down?1:0);cy=y+12+(down?1:0);
  bar(dst,cx-4,cy-1,9,3,symbol);
  if(button)bar(dst,cx-1,cy-4,3,9,symbol);
 }
 F1StrategyBevel(dst,x+26,y,w-52,24,fill,shadow,light);
 F1StrategyText(dst,x+w/2-F1StrategyTextWidth(value,0)/2,y+6,value,0,ink,0);
}
void F1RenderStrategyEditor(unsigned char *dst,const unsigned char *pal,const F1StrategyOption *o,
 int focus,int id,int laps,int preset,int n,int edited,int quick,const char *error)
{
 int bg,face,light,shadow,ink,yellow,red,tyres[3],i,y,end,total=0,start=62,action;
 int side=focus&1,pressed=(focus&2)!=0;char label[128];
 (void)n;(void)edited;
 focus/=4;
 if(!dst || !pal || !o || o->count<1 || o->count>4)return;
 palette(pal);clipRight=640;
 bg=nearest(pal,45,55,88);face=nearest(pal,85,97,134);light=nearest(pal,124,164,204);
 shadow=nearest(pal,32,40,76);ink=nearest(pal,250,250,244);yellow=nearest(pal,255,235,60);
 red=nearest(pal,190,30,5);tyres[0]=nearest(pal,220,220,220);
 tyres[1]=nearest(pal,255,235,0);tyres[2]=nearest(pal,255,35,10);
 memset(dst,bg,307200);
 /* A self-contained menu backdrop avoids retaining a black startup fade. */
 for(y=48;y<480;y+=8)for(i=0;i<640;i+=8)if((i+y)&8)bar(dst,i,y,3,3,shadow);
 F1StrategyBevel(dst,8,8,624,34,light,ink,shadow);
 F1StrategyText(dst,25,19,"Grand Prix 2",3,shadow,0);F1StrategyText(dst,245,19,"Race strategy",3,shadow,0);
 F1StrategyBevel(dst,40,58,560,322,face,light,shadow);
 sprintf(label,"CAR %d   /   %d LAPS   /   %s",id,laps,quick?"QUICKRACE":"RACE");
 F1StrategyText(dst,62,72,label,0,ink,0);
 F1StrategyText(dst,62,99,"Strategy",0,yellow,0);
 if(!preset)strcpy(label,"Default");else sprintf(label,"%d",preset);
 F1StrategyField(dst,222,92,170,label,focus==0,side,pressed,face,light,shadow,ink,red);
 F1StrategyText(dst,62,130,"Pit stops",0,yellow,0);sprintf(label,"%d",o->stops);
 F1StrategyField(dst,222,123,100,label,focus==1,side,pressed,face,light,shadow,ink,red);
 sprintf(label,"Initial fuel: %d laps",o->fuel);F1StrategyText(dst,62,148,label,0,ink,0);
 F1StrategyCircuitDraw(dst,ink);
 F1StrategyText(dst,62,163,"Stint",0,yellow,0);F1StrategyText(dst,208,163,"Tyre",0,yellow,0);
 F1StrategyText(dst,370,163,"Pit at lap",0,yellow,0);F1StrategyText(dst,502,163,"Laps",0,yellow,0);
 for(i=0;i<o->count;i++){
  y=184+i*30;total+=o->length[i];sprintf(label,"%d",i+1);F1StrategyText(dst,82,y+6,label,0,ink,0);
  strcpy(label,o->compound[i]==0?"Hard":o->compound[i]==1?"Medium":"Soft");
  F1StrategyField(dst,176,y,164,label,focus==2+2*i,side,pressed,face,light,shadow,tyres[o->compound[i]],red);
  if(i<o->count-1){
   sprintf(label,"%d",total);
   F1StrategyField(dst,369,y,100,label,focus==3+2*i,side,pressed,face,light,shadow,ink,red);
  }else F1StrategyText(dst,385,y+6,"Finish",0,ink,0);
  sprintf(label,"%d",o->length[i]);F1StrategyText(dst,514,y+6,label,0,ink,0);
 }
 total=0;
 for(i=0;i<o->count;i++){
  total+=o->length[i];end=62+(int)((long)total*510/laps);
  bar(dst,start,322,end-start,20,tyres[o->compound[i]]);
  sprintf(label,"%c","HMS"[o->compound[i]]);
  if(F1StrategyTextWidth(label,0)<end-start-4)F1StrategyText(dst,start+(end-start-F1StrategyTextWidth(label,0))/2,327,label,0,shadow,0);
  if(i<o->count-1){bar(dst,end-1,320,2,24,shadow);sprintf(label,"%d",total);F1StrategyText(dst,end-F1StrategyTextWidth(label,0)/2,349,label,0,ink,0);}
  start=end;
 }
 action=2*o->count+1;
 for(i=0;i<3;i++){
  int active=focus==action+i,down=active && pressed;
  const char *caption=i==0?"Restore Default":i==1?"Ok":"Cancel";
  F1StrategyBevel(dst,100+i*147,400,146,26,active?red:face,down?shadow:light,down?light:shadow);
  F1StrategyText(dst,173+i*147-F1StrategyTextWidth(caption,3)/2+(down?1:0),405+(down?1:0),caption,3,ink,0);
 }
 if(error && *error)F1StrategyText(dst,20,463,error,0,yellow,0);
 else F1StrategyText(dst,62,463,"Arrows: move   Enter / Space: press",0,ink,0);
 clipRight=640;
}
