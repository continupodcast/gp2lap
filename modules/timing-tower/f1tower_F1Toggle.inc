
    F1ReplayObserve();
    if(replayObserved && !replayTowerAllowed) {
        replayTowerAllowed=1;
        if(!(activepage&PAGE_F1TOWER)) F1TogglePart(PAGE_F1TOWER);
        else ReDrawAllPages(F1OnRedraw);
        return;
    }
    F1TogglePart(PAGE_F1TOWER);

