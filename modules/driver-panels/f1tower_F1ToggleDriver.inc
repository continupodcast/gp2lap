
    if(!(activepage&PAGE_F1DRIVER) && pSessionMode && !(*pSessionMode&0x80)) {
        PAGESETOFF(PAGE_F1CARD);F1MicroMode=0;
    }
    if(!F1CockpitView && pSessionMode && (*pSessionMode&0x80)
       && (activepage&PAGE_F1DRIVER) && !driverDiff) {
        driverDiff=1;F1DiagLog("KEY 5: DIFFERENCE");return;
    }
    driverDiff=0;F1TogglePart(PAGE_F1DRIVER);

