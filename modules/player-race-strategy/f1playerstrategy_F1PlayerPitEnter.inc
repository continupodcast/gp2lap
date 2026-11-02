
 int id;
 if(!StrategyPlayerEnabled || !PerCarTyreCompounds || !pCarSetups || !c || ((c->id&63)!=playerArmedId && (c->id&63)!=playerNativeId) || !(c->id&128) ||
   !pIsReplay || *pIsReplay || !pSessionMode || *pSessionMode!=0x80)return;
 id=c->id&63;playerPitActive=1;playerPitOverride=-1;playerPitUsedBefore=playerUsedMask;
 if(!StrategyPlayerPitChoice)return;
 /* T16: selection is non-modal, in GP2's live refuelling menu.
    The native draw hook shows the scheduled/current compound; no waiting. */


