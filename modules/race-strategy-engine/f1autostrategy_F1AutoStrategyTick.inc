
 int i,id,laps,n,m,rank,fuel,quick,set,changed,advanced,next,action;
 unsigned long now,unit;unsigned char *a,*b,*q;GP2Car *c;GP2CSx *s;
 if(!StrategyAutoEnabled || !PerCarTyreCompounds || TyreTestFuelLaps || TyreTestCompound)return;
 if(!pSessionMode || !pCurTime || !pSesStartTime || !pNumCars || !pCarStructs || !pCarSetups ||
  !GP2_LapsInThisRace || !pIsReplay || *pIsReplay)return;
 now=*pCurTime;m=*pSessionMode;
 if(m!=autoMode || now<autoClock){autoBuilt=0;memset(autoArmed,0,sizeof(autoArmed));memset(autoSetSeen,0,sizeof(autoSetSeen));}
 autoMode=m;autoClock=now;if(m!=0x80)return;
 n=*pNumCars;laps=*GP2_LapsInThisRace;if(n<1 || n>26 || laps<6 || laps>255)return;
 if(!autoSignatureChecked){
  autoSignatureChecked=1;a=IDAtoFlat(0x2C4EFUL);b=IDAtoFlat(0x2BEF0UL);q=IDAtoFlat(0x6AE32UL);
  if(!a || !b || memcmp(a,"\xf7\x25",2) || memcmp(b,"\xf7\x35",2) || memcmp(a+2,b+2,4) ||
   !q || q[0]!=0xC6 || q[1]!=0x05 || q[6]!=0x80)return;
  autoFuelUnit=(unsigned long *)IDACodeReftoDataRef(0x2C4F1UL);
  autoQuickFlag=(unsigned char *)IDACodeReftoDataRef(0x6AE34UL);
 }
 if(!autoFuelUnit || !autoQuickFlag)return;unit=*autoFuelUnit;
 if(!unit || unit>0x7fffffffUL/99UL)return;
 if(!autoBuilt){
  if(*pSesStartTime || (pIsAccTime && *pIsAccTime))return;
  quick=*autoQuickFlag!=0;
  if(!(StrategyDistributionEnabled?AutoBuildDiverse(laps,quick):AutoBuild(laps,quick)))return;
  autoRandom=StrategyAutoSeed?StrategyAutoSeed:(unsigned long)time(NULL);
  if(!autoRandom)autoRandom=1;
  autoAllocationSeed=autoRandom;
  if(StrategyDistributionEnabled)AutoPrepareDistribution(n);
  for(i=0;i<autoTopCount;i++)AutoPlanLog(0,i+1,&autoTop[i],0);
  autoBuilt=1;
 }
 F1TyreTablesInit();
 for(i=0;i<n;i++){
  c=&pCarStructs[i];id=c->id&63;if(id<1 || id>40 || (c->id&128))continue;
  s=&pCarSetups[id-1];
  if(!autoArmed[id]){
   if(*pSesStartTime || c->lapNr || c->speed || c->numPitStopsDone || (c->flags_90&0x80))continue;
   if(StrategyDistributionEnabled){if(!autoChosen[id])continue;rank=autoPoolRank[id];}
   else{rank=(int)((AutoRand()>>8)%autoTopCount);autoCarPlan[id]=autoTop[rank];}
   autoStint[id]=0;autoStops[id]=0;
   fuel=(int)((autoCarPlan[id].length[0]+1)*1.0395305+0.999999);
   if(fuel>99)continue;
   c->pitStop1=c->pitStop2=c->pitStop3=0;
   s->pit_laps[0]=s->pit_laps[1]=s->pit_laps[2]=100;
   AutoSchedule(c,s,id,laps);
   c->fuelLoad=unit*(unsigned long)fuel;c->fuelLoadLaps=fuel*64;
   s->compound=autoCarPlan[id].compound[0];autoArmed[id]=1;
   AutoPlanLog(id,rank+1,&autoCarPlan[id],fuel);
  }
  set=tyreTablesReady?(int)tyreSetIndex[id-1]:-1;
  changed=set>=0 && set<=6 && autoSetSeen[id] && set!=autoSet[id];
  advanced=c->numPitStopsDone>autoStops[id];next=AutoNextLap(id);
  if(changed || advanced){
   action=1;
   if(next && advanced && (int)c->lapNr>=next-7){autoStint[id]++;action=2;}
   else if(!next)action=3;
   if(advanced && !AutoSchedule(c,s,id,laps))action=4;
   s->compound=autoCarPlan[id].compound[autoStint[id]];
   AutoServiceLog(c,id,action,autoSetSeen[id]?autoSet[id]:-1,set);
  }
  autoStops[id]=c->numPitStopsDone;
  if(set>=0 && set<=6){autoSet[id]=(unsigned char)set;autoSetSeen[id]=1;}
  s->compound=autoCarPlan[id].compound[autoStint[id]];
 }

