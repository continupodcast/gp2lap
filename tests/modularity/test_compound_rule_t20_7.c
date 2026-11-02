#define main previous_tests
#include "test_player_apply_t20.c"
#undef main
#include "../../src/f1strategyedit.h"
int main(void){int runs;AutoPlan draft;previous_tests();
 memset(&draft,0,sizeof(draft));draft.count=2;draft.length[0]=20;draft.length[1]=33;
 StrategyRequireCompoundChange=1;assert(!F1DraftValid(&draft,53,0));
 AutoBuild(53,0);runs=autoSolveRuns;
 StrategyRequireCompoundChange=0;assert(F1DraftValid(&draft,53,0));
 assert(AutoBuild(53,0));assert(autoSolveRuns>runs);
 autoFamilyCount[2]=0;autoSequence[0]=autoSequence[1]=1;AutoSequences(2,2);assert(autoFamilyCount[2]>0);
 StrategyRequireCompoundChange=1;autoFamilyCount[2]=0;AutoSequences(2,2);assert(autoFamilyCount[2]==0);
 StrategyRequireCompoundChange=0;reset();chooseRank=-2;playerEditedPlan=draft;F1PlayerStrategyTick();
 assert(playerArmedId==34 && !playerCompoundRule);playerPitActive=1;cars[1].numPitStops=0;
 F1PlayerFinishWithoutStops(&cars[1]);assert(setups[33].compound==0 && !F1PlayerNeedsDifferent());
 StrategyRequireCompoundChange=1;reset();chooseRank=-2;draft.compound[1]=1;playerEditedPlan=draft;F1PlayerStrategyTick();
 assert(playerCompoundRule);playerPitActive=1;cars[1].numPitStops=0;F1PlayerFinishWithoutStops(&cars[1]);assert(setups[33].compound==1);
 StrategyRequireCompoundChange=0;draft.count=1;draft.length[0]=53;assert(!F1DraftValid(&draft,53,0));assert(F1DraftValid(&draft,53,1));
 puts("PASS compound rule ON/OFF: draft, solver same-compound sequences, cache invalidation, pit enforcement, mandatory stop independent, Quickrace exception");return 0;}
