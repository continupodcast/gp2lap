
 if(!StrategyPlayerEnabled || !PerCarTyreCompounds || !pCarSetups || !c || ((c->id&63)!=playerArmedId && (c->id&63)!=playerNativeId) || !pIsReplay || *pIsReplay)return;
 F1PlayerStrategyService();playerPitActive=0;playerPitOverride=-1;

