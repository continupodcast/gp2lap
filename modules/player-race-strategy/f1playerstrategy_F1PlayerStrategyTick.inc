
 int i,n,humans=0,id=0,choice,quick,savedCount;unsigned char *a,*b,*q;GP2Car *c=0;
 AutoPlan savedTop[10];
 if(!StrategyPlayerEnabled || !PerCarTyreCompounds || StrategyTestEnabled || TyreTestFuelLaps || TyreTestCompound ||
  !pCurTime || !pSessionMode || !pIsReplay || *pIsReplay || !pSesStartTime || !pNumCars ||
  !pCarStructs || !pCarSetups || !pPlayerSetup || !GP2_LapsInThisRace)return;
 if(playerMode!=*pSessionMode || *pCurTime<playerClock){F1PlayerStrategyReset();}
 playerMode=*pSessionMode;playerClock=*pCurTime;
 if(playerMode!=0x80)return;
 F1PlayerStrategyService();
 if(playerMenuDone || *pSesStartTime || *GP2_LapsInThisRace<6)return;
 n=*pNumCars;if(n<1 || n>26)return;
 for(i=0;i<n;i++)if(pCarStructs[i].id&128){humans++;c=&pCarStructs[i];}
 if(!humans)return;
 /* Multiple player cars share native pPlayerSetup. Do not promise independent
    per-player compounds until that path is implemented and verified. */
 if(humans!=1){playerMenuDone=1;return;}
 id=c->id&63;if(id<1 || id>40 || c->lapNr || c->speed || c->numPitStopsDone)return;
 a=IDAtoFlat(0x2C4EFUL);b=IDAtoFlat(0x2BEF0UL);q=IDAtoFlat(0x6AE32UL);
 if(!a || !b || !q || memcmp(a,"\xf7\x25",2) || memcmp(b,"\xf7\x35",2) || memcmp(a+2,b+2,4) ||
  q[0]!=0xC6 || q[1]!=0x05 || q[6]!=0x80){playerMenuDone=1;return;}
 autoFuelUnit=(unsigned long *)IDACodeReftoDataRef(0x2C4F1UL);
 autoQuickFlag=(unsigned char *)IDACodeReftoDataRef(0x6AE34UL);if(!autoFuelUnit || !autoQuickFlag)return;
 quick=*autoQuickFlag!=0;
 savedCount=autoTopCount;memcpy(savedTop,autoTop,sizeof(savedTop));
 if(!AutoBuildPlayerOptions(*GP2_LapsInThisRace,quick)){playerMenuDone=1;autoTopCount=savedCount;memcpy(autoTop,savedTop,sizeof(savedTop));return;}
 /* NPC plans are already copied into autoCarPlan. The chooser never mutates pool. */
 playerMenuDone=1;choice=-1;
 if(F1PlayerStrategyChoose(autoTop,autoTopCount,id,*GP2_LapsInThisRace,&choice) && (choice>=0 || choice==-2)){
  if(choice==-2){autoTop[0]=playerEditedPlan;choice=0;}
  if(!F1PlayerApply(c,choice))playerNativeId=id;
 }else playerNativeId=id;
 autoTopCount=savedCount;memcpy(autoTop,savedTop,sizeof(savedTop));

