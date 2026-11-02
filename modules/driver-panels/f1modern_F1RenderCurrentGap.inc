
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

