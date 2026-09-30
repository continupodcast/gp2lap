#include <string.h>
#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include "f1render.h"
#include "f1config.h"
#include "f1assets.h"
#include "f1qualy.h"
#define TEAM_LOGO(t) ((f1_custom_logo_loaded[t] || (t)>=13) ? f1_custom_logos[t] : f1_logos[t])
static unsigned char lastpal[768], dark[256], white[4][256];
static unsigned char teamcol[F1_TEAM_COUNT], logocol[F1_TEAM_COUNT][192];
static int initialized,clipRight=640;
static unsigned char sectorcol[4], losscol, platecol[F1_TEAM_COUNT][4];
static unsigned char badgeWhite,badgeBlack;
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
    int s,i,x,y,col,top=cockpit?F1_CARD_Y_COCKPIT:F1_CARD_Y_TV;
    palette(pal);
    for(s=0;s<3;s++) {
        for(i=0;i<62;i++) for(y=top+77;y<top+80;y++) {
            col=i<60 && i%6<5?sectorcol[colors[s*10+i/6]<=3?colors[s*10+i/6]:0]:badgeBlack;
            x=F1_CARD_X+8+s*66+i;dst[y*640+x]=(unsigned char)col;
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
    clipRight=F1_PANEL_X+F1_PANEL_W;
    palette(pal); bottom=F1_PANEL_Y+F1_HEADER_H+count*F1_ROW_H+3;
    for(y=F1_PANEL_Y;y<bottom;y++) for(x=F1_PANEL_X;x<F1_PANEL_X+F1_PANEL_W;x++) dst[y*640+x]=dark[dst[y*640+x]];
    strcpy(label,mode==1?"POS +/-":mode==2?"PITS":mode==3?"LEADER GAP":"RACE");
    text(dst,F1_PANEL_X+(F1_PANEL_W-textwidth(label,3))/2,F1_PANEL_Y+2,label,3,-1,0);
    if(total>0) sprintf(label,"LAP %d / %d",lap,total); else sprintf(label,"LAP %d",lap);
    text(dst,F1_PANEL_X+(F1_PANEL_W-textwidth(label,1))/2,F1_PANEL_Y+20,label,1,-1,0);
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
        else if(row->out) strcpy(label,"OUT");
        else if(row->pit) strcpy(label,"IN PIT");
        else if(row->pos==1) strcpy(label,"Leader");
        else if(mode==3 && row->lapsBehind>0) sprintf(label,"+%dL",row->lapsBehind);
        else if(mode==3) F1Gap(label,row->leaderGap);
        else F1Gap(label,row->gap);
        if(!mode && !row->out && (row->pit || row->pos==1) && team>=0) col=teamcol[team];
        text(dst,F1_PANEL_X+64,y+1,label,1,col,mode?0:row->out);
        if(row->fastest) badge(dst,F1_PANEL_X+F1_PANEL_W+1,y,1);
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
    int i,y,di,team,start=0,n;long leader;char label[40];const F1QRow *r;
    if(!dst || !pal || !rows || count<1) return;
    if(count>F1_Q_MAX) count=F1_Q_MAX;
    leader=rows[0].best;
    if(f1_theme==F1_THEME_90S) { F1RenderQualy90(dst,pal,rows,count,remaining,practice,gaps,compact,focus); return; }
    n=compact?F1QWindow(rows,count,focus,&start):count;
    if(!n) return;
    palette(pal);clipRight=F1_PANEL_X+F1_Q_W;
    background(dst,F1_PANEL_X,F1_PANEL_Y,F1_Q_W,F1_Q_HEADER+n*F1_ROW_H+3);
    text(dst,F1_PANEL_X+5,F1_PANEL_Y+2,practice?"PRACTICE":"QUALY",practice?0:3,-1,0);
    if(remaining>=0) {
        sprintf(label,"%ld:%02ld",remaining/60000,(remaining/1000)%60);
        text(dst,F1_PANEL_X+F1_Q_W-3-textwidth(label,1),F1_PANEL_Y+3,label,1,remaining<120000?sectorcol[2]:-1,0);
    }
    text(dst,F1_PANEL_X+64,F1_PANEL_Y+21,"BEST / GAP",1,-1,0);
    for(i=0;i<n;i++) {
        r=&rows[start+i];y=F1_PANEL_Y+F1_Q_HEADER+i*F1_ROW_H;di=driver(r->id);team=di<0?-1:f1_drivers[di].team;
        if(compact && r->id==focus) bar(dst,F1_PANEL_X,y,2,F1_ROW_H-1,team<0?sectorcol[1]:teamcol[team]);
        if(r->best) sprintf(label,"%d",r->pos);else strcpy(label,"-");
        clipRight=F1_PANEL_X+18;text(dst,F1_PANEL_X+5,y,label,2,-1,0);
        logo(dst,F1_PANEL_X+18,y,team);
        clipRight=F1_PANEL_X+62;text(dst,F1_PANEL_X+35,y,di<0?"???":f1_drivers[di].abbr,0,-1,0);
        if(!r->best) strcpy(label,"NO TIME");
        else if(compact && r->id==focus) F1QTime(label,r->best,0);
        else if(gaps && r->pos>1 && leader) F1QDelta(label,r->best-leader);
        else F1QTime(label,r->best,0);
        clipRight=F1_PANEL_X+F1_Q_W-2;text(dst,F1_PANEL_X+64,y+1,label,1,-1,0);
        if(r->pit) badge(dst,F1_PANEL_X+F1_Q_W+1,y,0);
    }
}
void F1RenderQualyCard(unsigned char *dst,const unsigned char *pal,const F1QCard *c,int cockpit)
{
    int di,team,i,x,y,font;char label[40];const char *name;
    if(!dst || !pal || !c || !c->id) return;
    if(f1_theme==F1_THEME_90S) { F1RenderQualyCard90(dst,pal,c,cockpit); return; }
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
static void mapPixel(int x,int y,int color)
{
    int dx=x-(F1_MAP_X+70),dy=y-(F1_MAP_Y+70);
    if((fullMapDrawing || dx*dx+dy*dy<=68*68) && x>=F1_MAP_X && x<F1_MAP_X+140 && y>=F1_MAP_Y && y<F1_MAP_Y+140)
        mapImage[y*640+x]=(unsigned char)color;
}
static void mapPoint(double x,double y,const F1MapCar *focus,double co,double si,double *sx,double *sy)
{
    double dx=-(x-focus->x),dy=y-focus->y;
    *sx=F1_MAP_X+70+(dx*co-dy*si)*0.0825;
    *sy=F1_MAP_Y+70+(dx*si+dy*co)*0.0825;
}
static void mapRoad(double ax,double ay,double bx,double by,int radius,int color,int stripe)
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
    int i,j,pass,y,di,col,px[26],py[26];
    if(!dst || !pal || !track || !cars || count<2 || count>2048 || n<1 || n>26 || pitcount<0 || pitcount>512) return;
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
    scale=112.0/span; ox=F1_MAP_X+70+(maxx+minx)*scale/2; oy=F1_MAP_Y+70-(maxy+miny)*scale/2;
    palette(pal); clipRight=F1_MAP_X+140; fullMapDrawing=1;
    for(y=F1_MAP_Y;y<F1_MAP_Y+140;y++) memcpy(mapImage+y*640+F1_MAP_X,dst+y*640+F1_MAP_X,140);
    for(i=0;i<count;i++) {
        j=(i+1)%count;
        mapRoad(ox-track[i].x*scale,oy+track[i].y*scale,ox-track[j].x*scale,oy+track[j].y*scale,2,badgeWhite,0);
    }
    if(pits) for(i=1;i<pitcount;i++) mapRoad(ox-pits[i-1].x*scale,oy+pits[i-1].y*scale,ox-pits[i].x*scale,oy+pits[i].y*scale,1,nearest(pal,150,120,40),0);
    for(i=0;i<n;i++) { px[i]=(int)(ox-cars[i].x*scale); py[i]=(int)(oy+cars[i].y*scale); }
    /* Viewed car last so it remains visible in a pack. */
    for(pass=0;pass<2;pass++) for(i=0;i<n;i++) {
        if((cars[i].id==focusId)!=(pass==1) || (cars[i].out && cars[i].id!=focusId)) continue;
        di=driver(cars[i].id);col=di<0?badgeWhite:teamcol[f1_drivers[di].team];
        if(pass==1) mapRoad(px[i],py[i],px[i],py[i],3,badgeWhite,0);
        mapRoad(px[i],py[i],px[i],py[i],2,col,0);
    }
    for(y=F1_MAP_Y;y<F1_MAP_Y+140;y++) memcpy(dst+y*640+F1_MAP_X,mapImage+y*640+F1_MAP_X,140);
    fullMapDrawing=0;
}

#include "f1r90.inc"
