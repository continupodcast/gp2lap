
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
        if(F1_MODULE_TYRE_COMPOUND_HUD && PerCarTyreCompounds){
#if F1_MODULE_TYRE_COMPOUND_HUD
#include "../tyre-compound-hud/f1render_F1RenderRace_badges.inc"
#endif
}
        if(row->finished) finishBadge(dst,F1_PANEL_X+F1_RACE_W+1,y);
        else if(row->fastest) badge(dst,F1_PANEL_X+F1_RACE_W+1,y,1);
    }

