
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
        if(F1_MODULE_TYRE_COMPOUND_HUD && PerCarTyreCompounds){
#if F1_MODULE_TYRE_COMPOUND_HUD
#include "../tyre-compound-hud/f1render_F1RenderQualy_badges.inc"
#endif
}
        clipRight=F1_PANEL_X+F1_Q_TOTAL_W;
        if(r->pit) badge(dst,F1_PANEL_X+F1_Q_TOTAL_W+1,y,0);
    }

