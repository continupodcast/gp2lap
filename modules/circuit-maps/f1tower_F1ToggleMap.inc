
    if(!(activepage&PAGE_F1MAP)) { fullMap=1; F1TogglePart(PAGE_F1MAP); }
    else if(fullMap) { fullMap=0; F1DiagLog("KEY 6: LOCAL MAP"); }
    else F1TogglePart(PAGE_F1MAP);

