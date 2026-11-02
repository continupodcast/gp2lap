
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

