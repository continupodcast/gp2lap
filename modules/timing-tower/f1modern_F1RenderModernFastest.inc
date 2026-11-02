
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

