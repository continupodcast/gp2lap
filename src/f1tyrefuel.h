/* T5: one initial fill on a stationary pre-start grid, never a refill.
   Native 2C4EF: raw = fuelLoadLaps * DATA[157D8] >> 6.
   50 whole laps therefore equals 50 * DATA[157D8].
   Native physics recalculates mass at 2B3BF -> 2C500 each update. */
static unsigned char tyreFuelDone[41];
static unsigned long tyreFuelClock;
static int tyreFuelMode=-1;
static unsigned long *tyreFuelUnit;
static int tyreFuelChecked;
static void F1TyreFuelTick(void)
{
 int i,id,m;unsigned long now,unit;unsigned char *a,*b;GP2Car *c;
 if(!TyreTestFuelLaps || !TyreDiagnostics || !PerCarTyreCompounds ||
    TyreTestCompound<1 || TyreTestCompound>4)return;
 if(!pSessionMode || !pCurTime || !pSesStartTime || !pNumCars || !pCarStructs)return;
 if(!pIsReplay || *pIsReplay || (pIsAccTime && *pIsAccTime))return;
 now=*pCurTime;m=*pSessionMode;
 if(m!=tyreFuelMode || now<tyreFuelClock)memset(tyreFuelDone,0,sizeof(tyreFuelDone));
 tyreFuelMode=m;tyreFuelClock=now;
 if(m!=0x80 || *pSesStartTime || *pNumCars<1 || *pNumCars>26)return;
 if(!tyreFuelChecked){
  tyreFuelChecked=1;
  a=IDAtoFlat(0x2C4EFUL);b=IDAtoFlat(0x2BEF0UL);
  if(!a || !b || memcmp(a,"\xf7\x25",2) || memcmp(b,"\xf7\x35",2) ||
     memcmp(a+2,b+2,4))return;
  tyreFuelUnit=(unsigned long *)IDACodeReftoDataRef(0x2C4F1UL);
 }
 if(!tyreFuelUnit)return;
 unit=*tyreFuelUnit;if(!unit || unit>0x7fffffffUL/50UL)return;
 for(i=0;i<*pNumCars;i++){
  c=&pCarStructs[i];id=c->id&63;
  if(id<1 || id>40 || tyreFuelDone[id] || c->lapNr || c->speed ||
     c->numPitStopsDone || (c->flags_90&0x80))continue;
  c->fuelLoad=unit*50UL;c->fuelLoadLaps=50*64;
  c->numPitStops=0;c->pitStop1=c->pitStop2=c->pitStop3=255;
  tyreFuelDone[id]=1;
 }
}
