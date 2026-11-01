/* Do not label missing display updates as proven acceleration. */
static unsigned long strategyFrameClock;
static int strategyFrameSeen;
static void F1StrategyFrameObserve(void)
{
 if(pCurTime){strategyFrameClock=*pCurTime;strategyFrameSeen=1;}
}
static int F1StrategyAccelerationMode(void)
{
 if(pIsAccTime && *pIsAccTime)return 1;
 if(!pCurTime || !strategyFrameSeen || *pCurTime<strategyFrameClock ||
    *pCurTime-strategyFrameClock>2000UL)return -1;
 return 0;
}
