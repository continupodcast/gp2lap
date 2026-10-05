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
    int i,fi=-1,a,b,font,x=120,y=F1_TV_SLOT_BOTTOM-40;char label[32];
    if(!dst || !pal || !rows || count<2) return;
    for(i=0;i<count;i++) if(rows[i].id==focus) fi=i;
    if(fi<0) return;
    a=fi?fi-1:0;b=fi?fi:1;
    if(rows[a].out || rows[a].pit || rows[b].out || rows[b].pit || rows[b].gap<0) return;
    palette(pal);clipRight=x+400;
    bar(dst,x,y,400,40,platecol[0][0]);
    modernSide(dst,x+7,y,133,&rows[a],0);
    modernSide(dst,x+260,y,133,&rows[b],1);
    bar(dst,x+145,y,1,40,nearest(pal,100,100,105));
    bar(dst,x+254,y,1,40,nearest(pal,100,100,105));
    sprintf(label,"+%ld.%03lds",rows[b].gap/1000,rows[b].gap%1000);
    clipRight=x+252;
    font=textwidth(label,3)>104?0:3;
    if(textwidth(label,font)>104) font=5;
    text(dst,x+200-textwidth(label,font)/2,y+6,label,font,-1,0);
    text(dst,x+200-textwidth("CURRENT GAP",1)/2,y+23,"CURRENT GAP",1,-1,0);
    clipRight=640;
}
void F1RenderModernFastest(unsigned char *dst,const unsigned char *pal,int id,long ms,int y)
{
    int di=driver(id),team,x=150,i,font,begin,end,fw,nw;char label[24],name[F1_NAME_LEN],first[F1_NAME_LEN];
    if(!dst || !pal || di<0 || ms<0 || y<0 || y>454) return;
    team=f1_drivers[di].team;palette(pal);clipRight=x+340;
    bar(dst,x,y,340,26,platecol[0][0]);
    bar(dst,x,y,83,26,sectorcol[3]);
    for(i=0;i<26;i++) bar(dst,x+83,y+i,13-abs(i-12),1,sectorcol[3]);
    text(dst,x+5,y+7,"FASTEST LAP",2,-1,0);
    F1QTime(label,ms,0);
    end=x+334-textwidth(label,0);
    text(dst,end,y+7,label,0,sectorcol[3],0);
    strcpy(first,f1_drivers[di].first);
    up90(name,f1_drivers[di].last,sizeof(name));
    font=3;fw=0;
    if(textwidth(first,fw)+4+textwidth(name,font)>end-23-(x+100)) font=0;
    modernFit(first,fw,60);
    modernFit(name,font,end-23-(x+100)-textwidth(first,fw)-4);
    nw=textwidth(first,fw)+4+textwidth(name,font);
    begin=x+100+(end-23-(x+100)-nw)/2;
    clipRight=end-23;
    text(dst,begin,y+7,first,fw,-1,0);begin+=textwidth(first,fw)+4;
    text(dst,begin,y+(font==3?5:7),name,font,teamcol[team],0);
    logo(dst,end-20,y+7,team);clipRight=640;
}
