
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

