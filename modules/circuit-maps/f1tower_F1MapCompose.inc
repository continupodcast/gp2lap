
    static F1MapPoint track[2048],pits[512];
    F1MapCar cars[26];int i,j,n,nt,np,qn=0;GP2Car *c;const F1QRow *qr=NULL;
    if(!pTrackSegs || !pNumTrackSegs || !pCarStructs || !pNumCars) return;
    nt=pNumTrackSegs[0];np=pNumTrackSegs[1];n=*pNumCars;
    if(nt<2 || nt>2048 || np<0 || np>512 || n<1 || n>28) return;
    if(n>26)n=26; /* Physical slots remain bounded; qualy roster is separate. */
    for(i=0;i<nt;i++) {track[i].x=pTrackSegs[i].xPos;track[i].y=pTrackSegs[i].yPos;}
    /* Same three separator segments as the original GP2Lap track map. */
    for(i=0;i<np;i++) {pits[i].x=pTrackSegs[nt+3+i].xPos;pits[i].y=pTrackSegs[nt+3+i].yPos;}
    if(pSessionMode && !(*pSessionMode&0x80)) qr=F1QRows(&qn);
    for(i=0;i<n;i++) {
        c=&pCarStructs[i];cars[i].id=c->id&63;cars[i].pos=(c->racePos>>1)+1;
        if(qr) {
            cars[i].pos=0;
            for(j=0;j<qn;j++) if(qr[j].id==cars[i].id && qr[j].best) cars[i].pos=qr[j].pos;
        }
        cars[i].out=(c->flags_90&32) && !(c->flags_5E&1);
        cars[i].x=(double)(long)c->xPos/2048.0;cars[i].y=(double)(long)c->yPos/2048.0;
        cars[i].angle=(double)c->zAngle*3.141592653589793/32768.0;
    }
    if(fullMap) F1RenderFullMap(dst,pal,track,nt,pits,np,cars,n,focus);
    else F1RenderMap(dst,pal,track,nt,pits,np,cars,n,focus);

