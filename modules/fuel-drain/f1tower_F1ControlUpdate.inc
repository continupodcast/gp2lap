
    int i;
    if(!fuelState.active) return;
    if(!F1FuelAllowed() || *pSesStartTime!=fuelSession || *pTrackNr!=fuelTrack) { F1FuelCancel(&fuelState);return; }
    if(!F1FuelTick(&fuelState,*pCurTime,1) || (pPaused && *pPaused)) return;
    for(i=0;i<*pNumCars;i++) if(F1FuelTarget(&fuelState,pCarStructs[i].id) && !(pCarStructs[i].flags_90&32))
        pCarStructs[i].fuelLoad=0;

