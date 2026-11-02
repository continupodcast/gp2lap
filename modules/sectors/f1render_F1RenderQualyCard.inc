
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

