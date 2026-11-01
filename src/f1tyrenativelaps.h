/* Native start-of-lap event; independent of end-of-frame rendering. */
static void F1TyreBoundarySession(unsigned long now,int mode)
{
 if(tyreBoundaryMode!=-1 && (mode!=tyreBoundaryMode || now<tyreBoundaryClock))
  memset(tyreLaps,0,sizeof(tyreLaps));
 tyreBoundaryMode=mode;tyreBoundaryClock=now;
}
static void F1TyreNativeBoundary(GP2Car *c,int source)
{
 int id,set,compound;unsigned long wear=0,now;
 if(!TyreDiagnostics || !c || !pCurTime || !pSessionMode || *pSessionMode!=0x80 ||
    (pIsReplay && *pIsReplay))return;
 id=c->id&63;if(id<1 || id>40)return;
 now=*pCurTime;F1TyreBoundarySession(now,*pSessionMode);F1TyreTablesInit();
 set=tyreTablesReady?(int)tyreSetIndex[id-1]:-1;
 if(tyreTablesReady && set>=0 && set<=6)wear=tyreSetWear[(id-1)*8+set];
 compound=F1AssignedCompound(PerCarTyreCompounds?c->id:0x81,pCarSetups,(const GP2CSx *)pPlayerSetup);
 F1TyreLapBoundary(c,now,set,compound,wear,tyreTablesReady,source);
}

void F1TyreNativeLap(GP2Car *c){F1TyreNativeBoundary(c,0);}
void F1TyreNativePitLap(GP2Car *c)
{
 /* T12.1: observed Hook_FES receives the already advanced native lapNr.
    Use it unchanged; adding one suppresses the following real crossing. */
 F1TyreNativeBoundary(c,1);
}
