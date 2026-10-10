/* f1r90.inc - late-1990s broadcast theme (HudTheme = 1).
   Included at the end of f1render.c: it reuses nearest(), palette(), the
   translucency tables (dark, white) and the bar/background helpers.
   Every text uses the Futura bitmaps in f1font90.h. */
#include "f1font90.h"

#define T90_X     8
#define T90_Y     8
#define T90_W     132
#define T90_QW    140
#define T90_HEAD  24
#define T90_ROW   12
#define T90_BOXW  14
#define T90_BOXH  11
#define D90_W     320
#define D90_H     30
#define D90_X     ((640-D90_W)/2)
#define D90_Y     (F1_TV_SLOT_BOTTOM-D90_H)
#define B90_W     196
#define B90_H     30

static unsigned char p90[256];
static unsigned char lift90[256];
static unsigned char c90yellow, c90gold, c90cyan, c90lime, c90red, c90green, c90ink, c90dimbox, c90navy;
static unsigned char r90box[4], r90cyan[4], r90green[4], r90red[4], grad90[32];
static unsigned char lastpal90[768];
static int init90;

static int mix90(int a,int b,int t,int n) { return (a*(n-t)+b*t)/n; }

static void palette90(const unsigned char *p)
{
    int i,r,g,b;
    palette(p);
    if(init90 && !memcmp(lastpal90,p,768)) return;
    memcpy(lastpal90,p,768); init90=1;
    c90yellow=nearest(p,244,194,13); c90gold=nearest(p,201,138,0);
    c90cyan=nearest(p,60,198,244);   c90lime=nearest(p,207,227,90);
    c90red=nearest(p,228,38,43);     c90green=nearest(p,0,168,84);
    c90ink=nearest(p,0,0,0);         c90dimbox=nearest(p,110,88,10);
    c90navy=nearest(p,6,32,58);
    for(i=0;i<4;i++) {
        r90box[i]=nearest(p,mix90(244,0,i,3),mix90(194,0,i,3),mix90(13,0,i,3));
        r90cyan[i]=nearest(p,mix90(60,6,i,3),mix90(198,32,i,3),mix90(244,58,i,3));
        r90green[i]=nearest(p,mix90(0,255,i,3),mix90(168,255,i,3),mix90(84,255,i,3));
        r90red[i]=nearest(p,mix90(228,255,i,3),mix90(38,255,i,3),mix90(43,255,i,3));
    }
    /* gold -> yellow -> white, used by the DIFFERENCE bars */
    for(i=0;i<32;i++) {
        if(i<10) grad90[i]=nearest(p,mix90(201,244,i,10),mix90(138,194,i,10),mix90(0,13,i,10));
        else grad90[i]=nearest(p,mix90(244,255,i-10,21),mix90(194,255,i-10,21),mix90(13,255,i-10,21));
    }
    for(i=0;i<256;i++) {
        r=p[i*3]*4; g=p[i*3+1]*4; b=p[i*3+2]*4;
        lift90[i]=nearest(p,mix90(r,255,2,11),mix90(g,255,2,11),mix90(b,255,2,11));
        p90[i]=dark[dark[i]];
    }
}

static int w90(const char *s,int f)
{
    int n=0,c; const F90Font *ft=&f90_fonts[f];
    while(*s) { c=(unsigned char)*s++; if(c>=32 && c<127) n+=ft->adv[c-32]; }
    return n;
}

/* color<0: white with coverage blending; ramp: exact colours for a known
   background; otherwise solid colour for coverage >= 2.
   maxw>0 condenses the text horizontally so it never exceeds maxw. */
static void t90(unsigned char *dst,int x,int y,const char *s,int f,int color,
                const unsigned char *ramp,int dim,int maxw)
{
    const F90Font *ft=&f90_fonts[f]; const unsigned char *g;
    int total=w90(s,f),adv=0,c,xx,yy,a,px,py;
    if(!total) return;
    if(maxw<=0 || total<=maxw) maxw=total;
    while(*s) {
        c=(unsigned char)*s++; if(c<32 || c>=127) continue; c-=32;
        g=ft->pix+(long)c*ft->cw*ft->ch;
        for(yy=0;yy<ft->ch;yy++) {
            py=y+yy; if(py<0 || py>=480) continue;
            for(xx=0;xx<ft->cw;xx++) {
                a=g[yy*ft->cw+xx]; if(!a) continue;
                px=x+(adv+xx)*maxw/total; if(px<0 || px>=clipRight) continue;
                if(dim && a>1) a=1;
                if(ramp) dst[py*640+px]=ramp[a];
                else if(color<0) dst[py*640+px]=white[a][dst[py*640+px]];
                else if(a>=2) dst[py*640+px]=(unsigned char)color;
            }
        }
        adv+=ft->adv[c];
    }
}

/* Top coordinate that centres the cap height of font f inside [top,top+h). */
static int vc90(int top,int h,int f)
{
    const F90Font *ft=&f90_fonts[f];
    return top+(h-(ft->base-ft->top+1))/2-ft->top;
}
static void tc90(unsigned char *dst,int x,int y,int w,int h,const char *s,int f,int color,const unsigned char *ramp)
{
    int tw=w90(s,f); if(tw>w) tw=w;
    t90(dst,x+(w-tw)/2,vc90(y,h,f),s,f,color,ramp,0,w);
}
static void tr90(unsigned char *dst,int right,int y,const char *s,int f,int color,int dim)
{
    t90(dst,right-w90(s,f),y,s,f,color,NULL,dim,0);
}
static void up90(char *dst,const char *src,int n)
{
    int i; for(i=0;i<n-1 && src[i];i++) dst[i]=(char)toupper((unsigned char)src[i]); dst[i]=0;
}
static void panel90(unsigned char *dst,int x,int y,int w,int h)
{
    int xx,yy; for(yy=y;yy<y+h;yy++) for(xx=x;xx<x+w;xx++) dst[yy*640+xx]=p90[dst[yy*640+xx]];
}
static void liftrow90(unsigned char *dst,int x,int y,int w,int h)
{
    int xx,yy; for(yy=y;yy<y+h;yy++) for(xx=x;xx<x+w;xx++) dst[yy*640+xx]=lift90[dst[yy*640+xx]];
}
static void oval90(unsigned char *dst,int cx,int cy,int rx,int ry,int col)
{
    int x,y; for(y=-ry;y<=ry;y++) for(x=-rx;x<=rx;x++)
        if((long)x*x*ry*ry+(long)y*y*rx*rx<=(long)rx*rx*ry*ry) dst[(cy+y)*640+cx+x]=(unsigned char)col;
}
static void time90(char *label,long ms)
{
    if(ms<0) ms=0;
    sprintf(label,"%ld:%02ld.%03ld",ms/60000,(ms/1000)%60,ms%1000);
}
static void diff90(char *label,long ms)
{
    if(ms<60000) sprintf(label,"%ld.%03ld",ms/1000,ms%1000);
    else time90(label,ms);
}
static void header90(unsigned char *dst,int x,int y,int w,const char *title,const char *small,
                     const char *value,int valueColor)
{
    t90(dst,x+5,y+4,title,F90_HEAD,-1,NULL,0,w-60);
    if(small) tr90(dst,x+w-5,y+2,small,F90_SMALL,-1,0);
    if(value) tr90(dst,x+w-5,y+11,value,F90_HEAD,valueColor,0);
}

/* ══════════════ Race tower ══════════════ */
static void F1RenderRace90(unsigned char *dst,const unsigned char *pal,const F1Row *rows,int count,int lap,int total,int mode)
{
    int i,j,x=T90_X,y,di,box,col,dim,bx; const F1Row *row; char label[40],name[F1_NAME_LEN];
    palette90(pal); clipRight=x+T90_W;
    background(dst,x,T90_Y,T90_W,T90_HEAD+count*T90_ROW+3);
    if(total>0) sprintf(label,"%d / %d",lap,total); else sprintf(label,"%d",lap);
    header90(dst,x,T90_Y,T90_W,mode==1?"POSITIONS +/-":mode==2?"PIT STOPS":mode==3?"GAP TO LEADER":"RACE POSITIONS",
             "CURRENT LAP",label,-1);
    for(i=0;i<count;i++) {
        row=&rows[i]; y=T90_Y+T90_HEAD+i*T90_ROW; di=driver(row->id); dim=row->out;
        if(row->focused) liftrow90(dst,x,y-1,T90_W,T90_ROW);
        bx=x+4;
        box=row->out?c90dimbox:row->positionChange>0?c90green:row->positionChange<0?c90red:c90yellow;
        bar(dst,bx,y,T90_BOXW,T90_BOXH,box);
        if(row->positionChange && !row->out) {
            for(j=0;j<4;j++) for(col=-j;col<=j;col++)
                dst[(y+3+(row->positionChange>0?j:3-j))*640+bx+T90_BOXW/2+col]=badgeWhite;
        } else {
            sprintf(label,"%d",row->pos);
            tc90(dst,bx,y,T90_BOXW,T90_BOXH,label,F90_TOWER,-1,r90box);
        }
        up90(name,di<0?"???":f1_drivers[di].last,sizeof(name));
        t90(dst,x+22,vc90(y,T90_BOXH,F90_TOWER),name,F90_TOWER,-1,NULL,dim,T90_W-22-42);
        col=c90lime;
        if(mode==1) {
            if(row->gain==F1_GAIN_UNKNOWN || !row->gain) { strcpy(label,"-"); col=-1; }
            else { sprintf(label,"%+d",row->gain); col=row->gain>0?c90green:c90red; }
        } else if(mode==2) { sprintf(label,"%d",row->stops); col=-1; }
        else if(row->out) { strcpy(label,"OUT"); col=-1; }
        else if(row->pit) { strcpy(label,"IN PIT"); col=c90cyan; }
        else if(row->pos==1) strcpy(label,mode==3?"Leader":"Gap");
        else if(mode==3 && row->lapsBehind>0) sprintf(label,"+%dL",row->lapsBehind);
        else if(mode==3) F1Gap(label,row->leaderGap);
        else F1Gap(label,row->gap);
        tr90(dst,x+T90_W-4,vc90(y,T90_BOXH,F90_TOWER),label,F90_TOWER,col,dim);
        if(row->finished) finishBadge(dst,x+T90_W+1,y);
    }
}

/* ══════════════ Driver plate (same 200x38 slot as the modern card) ══════════════ */
static void F1RenderDriver90(unsigned char *dst,const unsigned char *pal,int id,int position,int x,int y)
{
    int di,team,nx; char label[20],name[F1_NAME_LEN*2+2],first[F1_NAME_LEN],last[F1_NAME_LEN]; const F1Driver *d;
    di=driver(id); if(di<0) return; d=&f1_drivers[di]; team=d->team;
    palette90(pal); clipRight=x+F1_DRIVER_W;
    panel90(dst,x,y,F1_DRIVER_W,F1_DRIVER_H);
    bar(dst,x,y,F1_DRIVER_H,F1_DRIVER_H,c90yellow);
    if(position>0) sprintf(label,"%d",position); else strcpy(label,"-");
    tc90(dst,x,y,F1_DRIVER_H,F1_DRIVER_H,label,F90_BIG,-1,r90box);
    up90(first,d->first,sizeof(first)); up90(last,d->last,sizeof(last));
    if(first[0]) sprintf(name,"%s %s",first,last); else strcpy(name,last);
    nx=x+F1_DRIVER_H+8;
    t90(dst,nx,y+4,name,F90_NAME,-1,NULL,0,F1_DRIVER_W-(F1_DRIVER_H+8)-46);
    up90(name,f1_team_names[team],sizeof(name));
    t90(dst,nx,y+23,name,F90_HEAD,c90cyan,NULL,0,F1_DRIVER_W-(F1_DRIVER_H+8)-46);
    oval90(dst,x+F1_DRIVER_W-22,y+19,17,10,c90cyan);
    sprintf(label,"%d",d->number);
    tc90(dst,x+F1_DRIVER_W-39,y+9,34,21,label,F90_NAME,-1,r90cyan);
}

/* ══════════════ Qualifying tower ══════════════ */
static void F1RenderQualy90(unsigned char *dst,const unsigned char *pal,const F1QRow *rows,int count,
                            long remaining,int practice,int gaps,int compact,int focus)
{
    int i,y,di,start=0,n,x=T90_X,col; long leader; char label[40],name[F1_NAME_LEN]; const F1QRow *r;
    leader=rows[0].best;
    n=compact?F1QWindow(rows,count,focus,&start):count;
    if(!n) return;
    palette90(pal); clipRight=x+T90_QW;
    background(dst,x,T90_Y,T90_QW,T90_HEAD+n*T90_ROW+3);
    if(remaining>=0) sprintf(label,"%ld:%02ld",remaining/60000,(remaining/1000)%60); else label[0]=0;
    t90(dst,x+5,T90_Y+4,practice?"PRACTICE":"QUALIFYING",F90_HEAD,-1,NULL,0,T90_QW-60);
    if(label[0]) tr90(dst,x+T90_QW-5,T90_Y+4,label,F90_HEAD,remaining<120000?c90red:-1,0);
    tr90(dst,x+T90_QW-5,T90_Y+14,"BEST / GAP",F90_SMALL,-1,0);
    for(i=0;i<n;i++) {
        r=&rows[start+i]; y=T90_Y+T90_HEAD+i*T90_ROW; di=driver(r->id);
        if(compact && r->id==focus) liftrow90(dst,x,y-1,T90_QW,T90_ROW);
        bar(dst,x+4,y,T90_BOXW,T90_BOXH,r->best?c90yellow:c90dimbox);
        if(r->best) sprintf(label,"%d",r->pos); else strcpy(label,"-");
        tc90(dst,x+4,y,T90_BOXW,T90_BOXH,label,F90_TOWER,-1,r90box);
        up90(name,di<0?"???":f1_drivers[di].last,sizeof(name));
        t90(dst,x+22,vc90(y,T90_BOXH,F90_TOWER),name,F90_TOWER,-1,NULL,!r->best,T90_QW-22-46);
        col=-1;
        if(!r->best) strcpy(label,"NO TIME");
        else if(compact && r->id==focus) F1QTime(label,r->best,0);
        else if(gaps && r->pos>1 && leader) { F1QDelta(label,r->best-leader); col=c90lime; }
        else F1QTime(label,r->best,0);
        tr90(dst,x+T90_QW-4,vc90(y,T90_BOXH,F90_TOWER),label,F90_TOWER,col,!r->best);
        if(r->pit) badge(dst,x+T90_QW+1,y,0);
    }
}

/* ══════════════ Qualifying card (keeps the modern geometry: 210x84) ══════════════ */
static void F1RenderQualyCard90(unsigned char *dst,const unsigned char *pal,const F1QCard *c,int cockpit)
{
    int di,i,x,y,col; char label[40],name[F1_NAME_LEN];
    if(cockpit) {
        palette90(pal);x=F1_CARD_COMPACT_X;y=F1_CARD_Y_COCKPIT;clipRight=x+F1_CARD_COMPACT_W;
        di=driver(c->id);panel90(dst,x,y,F1_CARD_COMPACT_W,F1_CARD_COMPACT_H);
        bar(dst,x,y,26,23,c90yellow);
        if(c->pos) sprintf(label,"%d",c->pos);else strcpy(label,"-");
        tc90(dst,x,y,26,23,label,F90_NAME,-1,r90box);
        t90(dst,x+30,vc90(y,23,F90_NAME),di<0?"???":f1_drivers[di].abbr,F90_NAME,-1,NULL,0,48);
        if(c->pit) strcpy(label,"IN PIT");
        else if(c->outlap) strcpy(label,"OUT LAP");
        else if(c->post) F1QTime(label,c->last,0);
        else if(c->showGap) F1QDelta(label,c->gap);
        else F1QLiveTime(label,c->running);
        t90(dst,x+104,y+4,label,F90_NAME,c->showGap?sectorcol[c->gap<=0?1:2]:-1,NULL,0,90);
        di=driver(c->leaderId);up90(name,di<0?"???":f1_drivers[di].last,sizeof(name));
        t90(dst,x+199,y+2,name,F90_SMALL,-1,NULL,0,77);
        if(c->pole) strcpy(label,"POLE");
        else if(c->post && c->hasGap) F1QDelta(label,c->gap);
        else F1QTime(label,c->leaderBest,0);
        t90(dst,x+199,y+13,label,F90_SMALL,c->pole?c90yellow:-1,NULL,0,77);
        for(i=0;i<F1_CARD_COMPACT_W;i++) {
            dst[(y+24)*640+x+i]=grad90[i*31/(F1_CARD_COMPACT_W-1)];
            dst[(y+25)*640+x+i]=grad90[i*31/(F1_CARD_COMPACT_W-1)];
        }
        clipRight=x+F1_CARD_COMPACT_W;
        for(i=0;i<3;i++) {
            sprintf(label,"S%d",i+1);
            t90(dst,x+8+i*90+34,y+30,label,F90_HEAD,c->color[i]?sectorcol[c->color[i]]:-1,NULL,!c->color[i],0);
            bar(dst,x+8+i*90,y+43,86,3,sectorcol[c->color[i]]);
        }
        return;
    }
    palette90(pal); x=F1_CARD_X; y=cockpit?F1_CARD_Y_COCKPIT:F1_CARD_Y_TV; clipRight=x+F1_CARD_W;
    di=driver(c->id);
    panel90(dst,x,y,F1_CARD_W,F1_CARD_H);
    bar(dst,x,y,26,23,c90yellow);
    if(c->pos) sprintf(label,"%d",c->pos); else strcpy(label,"-");
    tc90(dst,x,y,26,23,label,F90_NAME,-1,r90box);
    up90(name,di<0?"UNKNOWN":f1_drivers[di].last,sizeof(name));
    t90(dst,x+32,vc90(y,23,F90_NAME),name,F90_NAME,-1,NULL,0,F1_CARD_W-38);
    for(i=0;i<F1_CARD_W;i++) { dst[(y+23)*640+x+i]=grad90[i*31/(F1_CARD_W-1)]; dst[(y+24)*640+x+i]=grad90[i*31/(F1_CARD_W-1)]; }
    col=-1;
    if(c->pit) { strcpy(label,"IN PIT"); col=c90cyan; }
    else if(c->outlap) strcpy(label,"OUT LAP");
    else if(c->post) F1QTime(label,c->last,0);
    else if(c->showGap) { F1QDelta(label,c->gap); col=sectorcol[c->gap<=0?1:2]; }
    else F1QLiveTime(label,c->running);
    clipRight=x+107; t90(dst,x+8,y+30,label,F90_VALUE,col,NULL,0,97);
    clipRight=x+F1_CARD_W-6;
    if(c->pole) strcpy(label,"POLE");
    else if(c->post && c->hasGap) F1QDelta(label,c->gap);
    else F1QTime(label,c->leaderBest,0);
    t90(dst,x+112,y+31,label,F90_NAME,c->pole?c90yellow:-1,NULL,0,92);
    if(!c->pole && c->leaderId) {
        di=driver(c->leaderId); up90(name,di<0?"???":f1_drivers[di].last,sizeof(name));
        t90(dst,x+112,y+48,name,F90_SMALL,-1,NULL,0,92);
    }
    clipRight=x+F1_CARD_W;
    for(i=0;i<3;i++) {
        sprintf(label,"S%d",i+1);
        t90(dst,x+8+i*66+26,y+62,label,F90_HEAD,c->color[i]?sectorcol[c->color[i]]:-1,NULL,!c->color[i],0);
        bar(dst,x+8+i*66,y+77,62,3,sectorcol[c->color[i]]);
    }
}

/* ══════════════ DIFFERENCE: focused car against the car ahead ══════════════ */
static void side90(unsigned char *dst,int x,int y,int w,const F1Row *r,int right)
{
    int di,i,bx,info,iw; char first[F1_NAME_LEN],last[F1_NAME_LEN],label[8];
    di=driver(r->id);
    bx=right?x+w-26:x;
    bar(dst,bx,y,26,D90_H,c90yellow);
    sprintf(label,"%d",r->pos);
    tc90(dst,bx,y,26,D90_H,label,F90_VALUE,-1,r90box);
    info=right?x+4:x+26+6; iw=w-26-10;
    for(i=0;i<iw;i++) {
        int g=right?(iw-1-i)*31/(iw-1):i*31/(iw-1);
        dst[(y+2)*640+info+i]=grad90[g]; dst[(y+3)*640+info+i]=grad90[g];
    }
    up90(first,di<0?"":f1_drivers[di].first,sizeof(first));
    up90(last,di<0?"???":f1_drivers[di].last,sizeof(last));
    clipRight=info+iw;
    if(right) {
        t90(dst,info+iw-w90(first,F90_SMALL),y+5,first,F90_SMALL,-1,NULL,0,0);
        i=w90(last,F90_NAME); if(i>iw) i=iw;
        t90(dst,info+iw-i,y+15,last,F90_NAME,-1,NULL,0,iw);
    } else {
        t90(dst,info,y+5,first,F90_SMALL,-1,NULL,0,iw);
        t90(dst,info,y+15,last,F90_NAME,-1,NULL,0,iw);
    }
    clipRight=640;
}
void F1Render90Difference(unsigned char *dst,const unsigned char *pal,const F1Row *rows,int count,int focus)
{
    int i,fi=-1,a,b,sw; char label[20];
    if(!dst || !pal || !rows || count<2) return;
    for(i=0;i<count;i++) if(rows[i].id==focus) fi=i;
    if(fi<0) return;
    a=fi>0?fi-1:0; b=fi>0?fi:1;
    if(rows[a].out || rows[a].pit || rows[b].out || rows[b].pit || rows[b].gap<0) return;
    palette90(pal); clipRight=640;
    panel90(dst,D90_X,D90_Y,D90_W,D90_H);
    sw=(D90_W-80)/2;
    side90(dst,D90_X,D90_Y,sw,&rows[a],0);
    side90(dst,D90_X+D90_W-sw,D90_Y,sw,&rows[b],1);
    clipRight=D90_X+sw+80;
    tc90(dst,D90_X+sw,D90_Y+2,80,10,"DIFFERENCE",F90_HEAD,c90cyan,NULL);
    diff90(label,rows[b].gap);
    tc90(dst,D90_X+sw,D90_Y+12,80,17,label,F90_VALUE,c90yellow,NULL);
    clipRight=640;
}

/* ══════════════ FASTEST LAP banner ══════════════ */
void F1Render90Fastest(unsigned char *dst,const unsigned char *pal,int id,long ms,int y)
{
    int di,x=(640-B90_W)/2,fw; char first[F1_NAME_LEN],last[F1_NAME_LEN],label[20];
    if(!dst || !pal || id<1 || id>40 || y<0 || y>480-B90_H) return;
    di=driver(id); if(di<0) return;
    palette90(pal); clipRight=x+B90_W;
    panel90(dst,x,y,B90_W,B90_H);
    t90(dst,x+6,y+3,"FASTEST LAP",F90_HEAD,c90cyan,NULL,0,0);
    up90(first,f1_drivers[di].first,sizeof(first)); up90(last,f1_drivers[di].last,sizeof(last));
    time90(label,ms);
    tr90(dst,x+B90_W-6,y+14,label,F90_NAME,-1,0);
    fw=first[0]?w90(first,F90_SMALL)+4:0;
    if(first[0]) t90(dst,x+6,y+20,first,F90_SMALL,-1,NULL,0,0);
    t90(dst,x+6+fw,y+14,last,F90_NAME,-1,NULL,0,B90_W-12-fw-w90(label,F90_NAME)-6);
}
