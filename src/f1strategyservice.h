extern void F1PlayerStrategyService(void);
/* T10: native refuelling service returns after tyre-set reset and stop-count
   update. Run strategy here in both normal and accelerated simulation. */
extern void StrategyServiceHook(void);
unsigned long StrategyServiceOriginal=0;
static int strategyServiceHookReady;
static int F1StrategyServiceInit(void)
{
 unsigned char *p,*original;
 if(strategyServiceHookReady)return 1;
 if(!StrategyAutoEnabled && !StrategyTestEnabled && !StrategyPlayerEnabled)return 0;
 p=IDAtoFlat(0x2A54DUL);original=IDAtoFlat(0x2C58BUL);
 if(!p || !original || p[0]!=0xE8 ||
    (unsigned long)(p+5)+*(long *)(p+1)!=(unsigned long)original ||
    memcmp(original,"\xc7\x05",2))return 0;
 StrategyServiceOriginal=(unsigned long)original;
 *(long *)(p+1)=(long)((unsigned long)StrategyServiceHook-(unsigned long)(p+5));
 strategyServiceHookReady=1;return 1;
}
void __near _cdecl F1StrategyServiceAfter(void)
{
 /* Existing guards reject replays and inactive modes. Never construct a new
    plan in the service callback: only a plan armed on a fresh grid can run. */
 if(!pSessionMode || *pSessionMode!=0x80 || !pIsReplay || *pIsReplay)return;
 if(!pSesStartTime || !*pSesStartTime)return;
 if(StrategyAutoEnabled && autoBuilt)F1AutoStrategyTick();
 if(StrategyTestEnabled)F1StrategyTestTick();
 if(StrategyPlayerEnabled)F1PlayerStrategyService();
}
