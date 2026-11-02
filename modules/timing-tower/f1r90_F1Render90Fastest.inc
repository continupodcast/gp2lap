
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

