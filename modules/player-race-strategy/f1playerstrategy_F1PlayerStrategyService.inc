
 int i,id,set,changed,advanced,next,action;GP2Car *c;GP2CSx *s;
 if(!StrategyPlayerEnabled || (!playerArmedId && !playerNativeId) || !PerCarTyreCompounds ||
  !pSessionMode || *pSessionMode!=0x80 || !pIsReplay || *pIsReplay || !pNumCars ||
  !pCarStructs || !pCarSetups || !pCurTime || !GP2_LapsInThisRace)return;
 if(*pNumCars<1 || *pNumCars>26)return;F1TyreTablesInit();
 for(i=0;i<*pNumCars;i++){
  c=&pCarStructs[i];id=c->id&63;if((id!=playerArmedId && id!=playerNativeId) || !(c->id&128))continue;
  s=&pCarSetups[id-1];set=tyreTablesReady?(int)tyreSetIndex[id-1]:-1;
  if(id==playerNativeId){
   /* Original strategy owns all scheduling/fuel. Only the explicit pit choice
      is applied after native service, and again on exit if no event is seen. */
   if(playerPitActive && playerPitOverride>=0){
    s->compound=(unsigned char)playerPitOverride;
    if(pPlayerSetup)((GP2CSx *)pPlayerSetup)->compound=s->compound;
   }
   continue;
  }
  changed=set>=0 && set<=6 && autoSetSeen[id] && set!=autoSet[id];
  advanced=c->numPitStopsDone>autoStops[id];next=AutoNextLap(id);
  if(changed || advanced){
   action=1;
   if(next && advanced && (int)c->lapNr>=next-7){autoStint[id]++;action=2;}
   else if(!next)action=3;
   if(playerPitOverride>=0){
    autoCarPlan[id].compound[autoStint[id]]=(unsigned char)playerPitOverride;action=6;
   }
   s->compound=autoCarPlan[id].compound[autoStint[id]];
   playerUsedMask=(playerPitActive?playerPitUsedBefore:playerUsedMask)|(1U<<s->compound);
   if(advanced && c->numPitStops!=0)F1PlayerSchedule(c);
   if(pPlayerSetup)((GP2CSx *)pPlayerSetup)->compound=s->compound;
   AutoServiceLog(c,id,action,autoSetSeen[id]?autoSet[id]:-1,set);
  }
  if(playerPitActive && playerPitOverride>=0){
   s->compound=(unsigned char)playerPitOverride;
   autoCarPlan[id].compound[autoStint[id]]=(unsigned char)playerPitOverride;
   playerUsedMask=playerPitUsedBefore|(1U<<s->compound);
   if(pPlayerSetup)((GP2CSx *)pPlayerSetup)->compound=s->compound;
  }
  F1PlayerFinishWithoutStops(c);
  autoStops[id]=c->numPitStopsDone;
  if(set>=0 && set<=6){autoSet[id]=(unsigned char)set;autoSetSeen[id]=1;}
 }

