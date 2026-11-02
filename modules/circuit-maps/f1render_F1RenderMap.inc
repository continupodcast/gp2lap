
    int i,j,fi=-1,x,y,road,edge,pit,gold;double co,si,ax,ay,bx,by;const F1MapCar *focus;
    /* 'focused' index and pointer are kept separate for old Watcom C. */
    if(!dst || !pal || !track || !cars || count<2 || count>2048 || n<1 || n>26 || pitcount<0 || pitcount>512) return;
    for(i=0;i<n;i++) if(cars[i].id==focusId) fi=i;
    if(fi<0) return;
    focus=&cars[fi];
    fullMapDrawing=0;
    palette(pal);clipRight=F1_MAP_X+140;
    road=nearest(pal,48,56,65);edge=nearest(pal,74,83,92);pit=nearest(pal,34,34,34);gold=nearest(pal,110,84,30);
    co=cos(3.141592653589793-focus->angle);si=sin(3.141592653589793-focus->angle);
    for(y=F1_MAP_Y;y<F1_MAP_Y+140;y++) memcpy(mapImage+y*640+F1_MAP_X,dst+y*640+F1_MAP_X,140);
    for(j=0;j<2;j++) for(i=0;i<count;i++) {
        mapPoint(track[i].x,track[i].y,focus,co,si,&ax,&ay);
        mapPoint(track[(i+1)%count].x,track[(i+1)%count].y,focus,co,si,&bx,&by);
        mapRoad(ax,ay,bx,by,j?1:16,j?edge:road,0);
    }
    if(pits) for(i=1;i<pitcount;i++) {
        mapPoint(pits[i-1].x,pits[i-1].y,focus,co,si,&ax,&ay);
        mapPoint(pits[i].x,pits[i].y,focus,co,si,&bx,&by);
        mapRoad(ax,ay,bx,by,4,pit,gold);
    }
    for(i=0;i<n;i++) if(i!=fi && !cars[i].out) mapCar(&cars[i],focus,co,si,0);
    mapCar(focus,focus,co,si,1);
    for(y=F1_MAP_Y;y<F1_MAP_Y+140;y++) for(x=F1_MAP_X;x<F1_MAP_X+140;x++) {
        int dx=x-(F1_MAP_X+70),dy=y-(F1_MAP_Y+70);
        if(dx*dx+dy*dy<=68*68) dst[y*640+x]=mapImage[y*640+x];
    }

