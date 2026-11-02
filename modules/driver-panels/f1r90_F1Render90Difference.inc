
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

