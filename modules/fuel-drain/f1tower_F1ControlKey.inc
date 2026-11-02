
    unsigned char targets[41];int i,id,n=0;
    if(!F1FuelKey(&fuelState,scan)) return;
    if(!F1FuelAllowed() || (pPaused && *pPaused)) {
        F1DiagLog("FUEL DRAIN: DISABLED OR SESSION NOT LIVE");return;
    }
    memset(targets,0,sizeof(targets));
    for(i=0;i<*pNumCars;i++) {
        id=pCarStructs[i].id&63;
        if(id>=1 && id<=40 && f1_fuel_targets[id] && !(pCarStructs[i].flags_90&32) && !targets[id]) { targets[id]=1;n++; }
    }
    fuelNoticeAt=*pCurTime;
    if(!n) { strcpy(fuelNotice,"FUEL DRAIN: NO ACTIVE TARGETS");F1DiagLog(fuelNotice);return; }
    fuelSession=*pSesStartTime;fuelTrack=*pTrackNr;
    F1FuelStart(&fuelState,targets,*pCurTime,f1_fuel_duration);
    sprintf(fuelNotice,"FUEL DRAIN: %d CAR%s",n,n==1?"":"S");F1DiagLog(fuelNotice);
    for(i=1;i<=40;i++) if(targets[i]) {char s[50];sprintf(s,"FUEL DRAIN: CarId=%d duration=%lu ms",i,f1_fuel_duration);F1DiagLog(s);}

