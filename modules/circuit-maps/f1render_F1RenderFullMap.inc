
    double minx,maxx,miny,maxy,scale,span,ox,oy,ax,ay;
    double focusX=0,focusY=0,bestDistance=-1;
    int i,j,k,x,y,di,col,hit,focusIndex=-1,order[26],px[26],py[26];
    if(!dst || !pal || !track || !cars || count<2 || count>2048 || n<1 || n>26 || pitcount<0 || pitcount>512) return;
    /* Keep the selected marker on the map centreline, avoiding lateral
       movement from the car's physical position and chassis motion. */
    for(i=0;i<n;i++) if(cars[i].id==focusId) {focusIndex=i;break;}
    if(focusIndex>=0) {
        focusX=cars[focusIndex].x;focusY=cars[focusIndex].y;
        for(i=0;i<count+(pits && pitcount>1?pitcount-1:0);i++) {
            double dx,dy,length,t,qx,qy,distance;
            const F1MapPoint *a,*b;
            if(i<count) {a=&track[i];b=&track[(i+1)%count];}
            else {if(!pits)break;a=&pits[i-count];b=a+1;}
            dx=b->x-a->x;dy=b->y-a->y;length=dx*dx+dy*dy;
            if(length<=0)continue;
            t=((cars[focusIndex].x-a->x)*dx+(cars[focusIndex].y-a->y)*dy)/length;
            if(t<0)t=0;
            if(t>1)t=1;
            qx=a->x+t*dx;qy=a->y+t*dy;
            dx=cars[focusIndex].x-qx;dy=cars[focusIndex].y-qy;distance=dx*dx+dy*dy;
            if(bestDistance<0 || distance<bestDistance) {bestDistance=distance;focusX=qx;focusY=qy;}
        }
    }
    hit=!mapZoom && fullMapValid && count==fullMapCount && pitcount==fullMapPitCount &&
        (pits!=NULL)==fullMapHasPits && !memcmp(fullMapPal,pal,768) &&
        !memcmp(fullMapTrack,track,count*sizeof(*track)) &&
        (!pits || !memcmp(fullMapPits,pits,pitcount*sizeof(*pits)));
    if(hit) {scale=fullMapScale;ox=fullMapOx;oy=fullMapOy;}
    else {
    minx=maxx=track[0].x; miny=maxy=track[0].y;
    for(i=0;i<count+pitcount;i++) {
        if(i>=count && !pits) break;
        ax=i<count?track[i].x:pits[i-count].x; ay=i<count?track[i].y:pits[i-count].y;
        if(ax<minx) minx=ax;
        if(ax>maxx) maxx=ax;
        if(ay<miny) miny=ay;
        if(ay>maxy) maxy=ay;
    }
    span=maxx-minx; if(maxy-miny>span) span=maxy-miny;
    if(span<=0) return;
    scale=112.0/span*mapZoomScale[mapZoom];
    ax=(maxx+minx)/2;ay=(maxy+miny)/2;
    if(mapZoom && focusIndex>=0) {ax=focusX;ay=focusY;}
    ox=F1_MAP_X+70+ax*scale; oy=F1_MAP_Y+70-ay*scale;
    }
    palette(pal); clipRight=F1_MAP_X+140; fullMapDrawing=1;
    for(y=F1_MAP_Y;y<F1_MAP_Y+140;y++) memcpy(mapImage+y*640+F1_MAP_X,dst+y*640+F1_MAP_X,140);
    if(hit) {
        for(y=0;y<140;y++) for(x=0;x<140;x++)
            if(fullMapMask[y*140+x]) mapImage[(y+F1_MAP_Y)*640+x+F1_MAP_X]=fullMapLayer[y*140+x];
    } else {
    fullMapCapture=!mapZoom;
    if(fullMapCapture) memset(fullMapMask,0,sizeof(fullMapMask));
    for(i=0;i<count;i++) {
        j=(i+1)%count;
        mapRoad(ox-track[i].x*scale,oy+track[i].y*scale,ox-track[j].x*scale,oy+track[j].y*scale,2,badgeWhite,0);
    }
    if(pits) for(i=1;i<pitcount;i++) mapRoad(ox-pits[i-1].x*scale,oy+pits[i-1].y*scale,ox-pits[i].x*scale,oy+pits[i].y*scale,1,nearest(pal,150,120,40),0);
    if(fullMapCapture) {
        for(y=0;y<140;y++) memcpy(fullMapLayer+y*140,mapImage+(y+F1_MAP_Y)*640+F1_MAP_X,140);
        memcpy(fullMapTrack,track,count*sizeof(*track));
        if(pits) memcpy(fullMapPits,pits,pitcount*sizeof(*pits));
        memcpy(fullMapPal,pal,768);fullMapCount=count;fullMapPitCount=pitcount;
        fullMapHasPits=pits!=NULL;fullMapScale=scale;fullMapOx=ox;fullMapOy=oy;fullMapValid=1;
    }
    fullMapCapture=0;
    }
    for(i=0;i<n;i++) { px[i]=(int)(ox-cars[i].x*scale); py[i]=(int)(oy+cars[i].y*scale); }
    if(focusIndex>=0) {px[focusIndex]=(int)(ox-focusX*scale);py[focusIndex]=(int)(oy+focusY*scale);}
    /* Backmarkers first; better positions remain above overlapping dots. */
    for(i=0;i<n;i++)order[i]=i;
    for(i=1;i<n;i++) {
        k=order[i];j=i;
        while(j>0 && (cars[order[j-1]].pos>0?cars[order[j-1]].pos:99)<(cars[k].pos>0?cars[k].pos:99)) {
            order[j]=order[j-1];j--;
        }
        order[j]=k;
    }
    for(k=0;k<n;k++) {
        i=order[k];
        if(cars[i].out && cars[i].id!=focusId) continue;
        di=driver(cars[i].id);col=di<0?badgeWhite:teamcol[f1_drivers[di].team];
        mapRoad(px[i],py[i],px[i],py[i],cars[i].id==focusId?3:2,col,0);
    }
    for(y=F1_MAP_Y;y<F1_MAP_Y+140;y++) memcpy(dst+y*640+F1_MAP_X,mapImage+y*640+F1_MAP_X,140);
    fullMapDrawing=0;

