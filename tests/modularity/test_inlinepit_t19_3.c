unsigned long StrategyRequireCompoundChange=1;
#include <assert.h>
#include <stdio.h>
#include <string.h>
typedef struct {unsigned char compound,num_pitstops,pit_laps[3];} GP2CSx;
typedef struct {unsigned char flags_91,field_67,digCtrlInput,flags_169,id,racePos,lapNr,numPitStopsDone,flags_90,numPitStops,pitStop1,pitStop2,pitStop3;unsigned speed,fuelLoadLaps;unsigned long fuelLoad;} GP2Car;
static unsigned long StrategyDistributionEnabled=0,StrategyDiagnostics=0;
static unsigned long StrategyAutoEnabled=0,StrategyAutoSeed=12345;
static unsigned char quickFlag=0,quickOp[7]={198,5,1,0,0,0,128};
static unsigned long StrategyRepairTestEnabled=0,TyreDiagnostics=0;
static unsigned char setTable[40],*tyreSetIndex=setTable;
static int tyreTablesReady=1;
static void F1TyreTablesInit(void){}
static unsigned long StrategyTestEnabled=0,PerCarTyreCompounds=1,TyreTestFuelLaps=0,TyreTestCompound=0;
static unsigned long clockValue=41,start=0,unit=1746944,*pCurTime=&clockValue,*pSesStartTime=&start;
static unsigned char mode=128,replay=0,acc=0,*pSessionMode=&mode,*pIsReplay=&replay,*pIsAccTime=&acc;
static unsigned short laps=53,*GP2_LapsInThisRace=&laps;static int count=2,*pNumCars=&count;
static GP2Car cars[2],*pCarStructs=cars;static GP2CSx setups[40],player,*pCarSetups=setups;static void *pPlayerSetup=&player;
static unsigned char testPalette[768];
static unsigned char a[6]={247,37,216,87,1,0},b[6]={247,53,216,87,1,0};
static unsigned char *IDAtoFlat(unsigned long x){return x==0x6AE32UL?quickOp:x==0x2C4EFUL?a:b;}
static void *IDACodeReftoDataRef(unsigned long x){if(x==0x7f624UL)return testPalette;if(x==0x6AE34UL)return &quickFlag;assert(x==0x2C4F1UL);return &unit;}
#include "../../src/f1simulationmode.h"
#include "../../src/f1strategytest.h"
#include "../../src/f1autostrategy.h"
#define __near
#define __cdecl
#define _cdecl
void StrategyServiceHook(void){}
static unsigned long StrategyPlayerEnabled=0,StrategyPlayerPitChoice=0;
#include "../../src/f1playerstrategy.h"
#include "../../src/f1strategyservice.h"

static int chooseCalls,chooseResult=1,chooseRank=0;
static int pitPick=-1,pitCalls;
int F1PlayerPitChoose(int id,int recommended,int required,int initial,int *choice)
{(void)recommended;(void)required;(void)initial;assert(id==34);pitCalls++;*choice=pitPick;return 1;}

int F1PlayerStrategyChoose(const AutoPlan *p,int n,int id,int distance,int *choice)
{chooseCalls++;assert(n>0 && id==34 && distance==laps);*choice=chooseRank;return chooseResult;}
static void reset(void)
{
 memset(cars,0,sizeof(cars));memset(setups,0,sizeof(setups));memset(&player,0,sizeof(player));
 cars[0].id=33;cars[1].id=128|34;cars[1].racePos=2;
 count=2;clockValue=41;start=0;mode=128;replay=0;quickFlag=0;laps=53;
 playerMenuDone=0;playerArmedId=0;playerMode=-1;autoBuilt=0;autoTopCount=0;
 StrategyAutoEnabled=0;StrategyTestEnabled=0;StrategyPlayerEnabled=1;StrategyPlayerPitChoice=0;pitPick=-1;pitCalls=0;
 TyreDiagnostics=0;StrategyDiagnostics=1;chooseCalls=0;chooseResult=1;chooseRank=0;
 memset(autoSetSeen,0,sizeof(autoSetSeen));
}

static void begin(void)
{
 reset();setTable[33]=3;F1PlayerStrategyTick();F1PlayerStrategyTick();
 start=17054;clockValue=1000;cars[1].lapNr=2;cars[1].fuelLoad=5555555;
}

static unsigned char svga=1,*pUseSVGA=&svga;
static GP2Car *cockpit=&cars[1],**ppCockpitCS=&cockpit;
int F1CockpitView=1;
void F1InlinePitWrapped(void){}
void F1InlinePitControlWrapped(void){}
void F1InlinePitCursorWrapped(void){}
void F1InlinePitNavigationWrapped(void){}
#include "../../src/f1inlinepit.h"
static unsigned long menuValue=6,requestsValue=0x1c,strideValue=640,fontOffsets[8],heights[8];
static unsigned char atlas[8*16*640],*atlasBase=atlas,coords[11],map[256],dirtyFlag,textMode;
static char drawn[65536];static int drawnCount;
static void nativeChar(unsigned int ch){
 unsigned glyph=map[ch]-32,group=glyph>>5,index=glyph&31;unsigned char *src=atlasBase+fontOffsets[group]+index*16;
 assert(src==inlinePitGlyph);assert(src[1]==inlinePitBackground);
 {unsigned int r,x;for(r=0;r<inlinePitHeights[group];r++)for(x=0;x<10;x++)assert(src[r*640+x]!=0);}assert(src[0]>0);
 drawn[drawnCount++]=(char)ch;coords[0]++;
}
static void prepare(void){int i;
 inlinePitReady=1;inlinePitMenu=&menuValue;inlinePitRequests=&requestsValue;inlinePitStride=&strideValue;
 inlinePitFonts=fontOffsets;inlinePitHeights=heights;inlinePitImageBase=&atlasBase;inlinePitCoords=coords;inlinePitMap=map;inlinePitDirty=&dirtyFlag;inlinePitTextMode=&textMode;inlinePitChar=nativeChar;
 menuValue=6;requestsValue=0x1c;svga=1;replay=0;F1CockpitView=1;dirtyFlag=0;textMode=0;drawnCount=0;memset(drawn,0,sizeof(drawn));
 for(i=0;i<256;i++)map[i]=(unsigned char)i;
 for(i=0;i<8;i++){fontOffsets[i]=i*16*640;heights[i]=10;}
 memset(atlas,0,sizeof(atlas));for(i=0;i<8*16*640;i+=16)atlas[i]=40;
 memset(coords,0xa5,11);coords[7]=coords[8]=0;
 testPalette[3]=60;testPalette[4]=60;testPalette[5]=60;
 testPalette[6]=63;testPalette[7]=56;testPalette[8]=0;
 testPalette[9]=63;testPalette[10]=10;testPalette[11]=10;
 StrategyPlayerPitChoice=1;
}

static int originalCursorCalls;
void F1InlinePitRunCursor(GP2Car *c){(void)c;originalCursorCalls++;}
static unsigned long selectedRow,selectedColumn,promptRefValue=100,clearRefValue=200;static unsigned char actionReady=255,originalDescriptor[16]={1,2},*descriptors[10];
static int nativeControls,nativePolls,nativeDraws;static unsigned long nativeControlRow;
void F1InlinePitRunDraw(GP2Car *c){(void)c;nativeDraws++;}
void F1InlinePitRunPoll(GP2Car *c){(void)c;nativePolls++;}
void F1InlinePitRunControl(GP2Car *c){
 /* A redraw nested in native controls must not render with temporary row
    mappings or fall back to old fuel/stops positions. */
 {int oldDraws=nativeDraws,oldCount=drawnCount;F1InlinePitAfter(c);assert(nativeDraws==oldDraws&&drawnCount==oldCount);}
 nativeControls++;nativeControlRow=*inlinePitRow;assert(inlinePitDescriptors[6]==originalDescriptor);
 if(c->digCtrlInput&16)c->flags_169&=127;
 else if(c->digCtrlInput&8){if(nativeControlRow==1)c->fuelLoadLaps+=64;else c->numPitStops++;}
 else if(c->digCtrlInput&4){if(nativeControlRow==1)c->fuelLoadLaps-=64;else c->numPitStops--;}
}
static void layout(void){
 inlinePitPromptRef=&promptRefValue;inlinePitClearRef=&clearRefValue;inlinePitOriginalPrompt=100;inlinePitOriginalClear=200;
 inlinePitRow=&selectedRow;inlinePitColumn=&selectedColumn;inlinePitAction=&actionReady;
 inlinePitDescriptors=descriptors;inlinePitOriginalDescriptor=originalDescriptor;descriptors[6]=originalDescriptor;inlinePitExtended=0;
 selectedRow=1;selectedColumn=0;actionReady=255;nativeControls=nativePolls=nativeDraws=0;
}
int main(void){GP2Car before;unsigned char saved[11];unsigned long fonts[8],clock;
 reset();chooseRank=-1;F1PlayerStrategyTick();prepare();layout();start=1;cars[1].lapNr=2;cars[1].numPitStops=2;cars[1].fuelLoadLaps=22*64;
 F1PlayerPitEnter(&cars[1]);assert(pitCalls==0);F1InlinePitTick();assert(selectedRow==2 && descriptors[6]==inlinePitDescriptor);assert(promptRefValue==(unsigned long)inlinePitPrompt&&clearRefValue==(unsigned long)inlinePitClear);
 // Validate the native descriptor exactly as native parser 34bad/34cbf reads it.
 assert(inlinePitDescriptor[0]==3 && inlinePitDescriptor[1]==3);
 assert(inlinePitDescriptor[2]==19 && inlinePitDescriptor[4]==18 && inlinePitDescriptor[6]==17);
 assert(inlinePitDescriptor[8]==2 && inlinePitDescriptor[9]==0 && inlinePitDescriptor[10]==0);
 // Native UP increments row, native DOWN decrements it. Enter TYRE with two DOWNs.
 selectedRow--;selectedRow--;assert(selectedRow==0);
 before=cars[1];clock=clockValue;cars[1].digCtrlInput=8;setups[33].compound=player.compound=0;
 F1InlinePitControl(&cars[1]);assert(player.compound==1 && selectedColumn==1 && nativePolls==1 && nativeControls==0);
 assert(cars[1].fuelLoad==before.fuelLoad&&cars[1].fuelLoadLaps==before.fuelLoadLaps&&cars[1].numPitStops==before.numPitStops&&clockValue==clock);
 assert(cars[1].flags_169&128);
 memcpy(saved,coords,sizeof(saved));memcpy(fonts,fontOffsets,sizeof(fonts));F1InlinePitAfter(&cars[1]);
 assert(strstr(drawn,"FUEL LAPS:")&&strstr(drawn,"STOPS TO GO:")&&strstr(drawn,"TYRE")&&strstr(drawn,"[M]")&&strstr(drawn,"PRESS FIRE TO GO"));
 assert(!memcmp(coords,saved,sizeof(saved))&&!memcmp(fontOffsets,fonts,sizeof(fonts)));
 F1InlinePitControl(&cars[1]);assert(player.compound==2);F1InlinePitControl(&cars[1]);assert(player.compound==2);
 cars[1].digCtrlInput=4;F1InlinePitControl(&cars[1]);assert(player.compound==1);F1InlinePitControl(&cars[1]);assert(player.compound==0);F1InlinePitControl(&cars[1]);assert(player.compound==0);
 actionReady=0;cars[1].digCtrlInput=8;F1InlinePitControl(&cars[1]);assert(player.compound==0);actionReady=255;
 // Fuel and stop rows delegate to the original native indices and restore layout.
 selectedRow=2;F1InlinePitControl(&cars[1]);assert(nativeControlRow==1&&cars[1].fuelLoadLaps==23*64&&selectedRow==2&&descriptors[6]==inlinePitDescriptor);
 selectedRow=1;F1InlinePitControl(&cars[1]);assert(nativeControlRow==0&&cars[1].numPitStops==3&&selectedRow==1);
 selectedRow=0;cars[1].digCtrlInput=16;F1InlinePitControl(&cars[1]);assert(!(cars[1].flags_169&128)&&nativeControlRow==0);
 // Direct H/M/S keys are no longer a selector.
 assert(!F1PlayerInlinePitKey(0x23,0)&&!F1PlayerInlinePitKey(0x32,0)&&!F1PlayerInlinePitKey(0x1f,0));
 // Restore the original descriptor when the menu is closed or optional system off.
 menuValue=0;F1InlinePitTick();assert(!inlinePitExtended&&descriptors[6]==originalDescriptor);assert(promptRefValue==100&&clearRefValue==200);
 menuValue=6;F1InlinePitTick();StrategyPlayerPitChoice=0;F1InlinePitTick();assert(!inlinePitExtended&&descriptors[6]==originalDescriptor);assert(promptRefValue==100&&clearRefValue==200);
 // Mandatory change still applies to TYRE navigation after zero further stops.
 begin();prepare();layout();F1PlayerPitEnter(&cars[1]);F1InlinePitTick();selectedRow=0;
 playerInitialCompound=0;playerUsedMask=playerPitUsedBefore=1;autoCarPlan[34].compound[autoStint[34]]=0;
 cars[1].numPitStops=0;cars[1].digCtrlInput=4;F1InlinePitControl(&cars[1]);assert(player.compound!=0&&selectedColumn==player.compound);
 assert(!AutoNextLap(34));replay=1;F1InlinePitTick();assert(!inlinePitExtended&&descriptors[6]==originalDescriptor);assert(promptRefValue==100&&clearRefValue==200);
 F1PlayerStrategyReset();remove("F1AUTO.CSV");remove("F1PLANS.CSV");
 // Menu remains usable after off-jacks while the native pit menu is visible.
 replay=0;StrategyPlayerPitChoice=1;menuValue=6;cars[1].flags_91=64;cars[1].field_67=3;
 playerPitActive=0;playerNativeId=34;prepare();layout();F1InlinePitTick();assert(inlinePitExtended&&playerPitActive&&inlinePitRearmed);
 requestsValue=0;F1InlinePitTick();assert(inlinePitExtended);requestsValue=0x1c;
 textMode=1;{int old=drawnCount;F1InlinePitAfter(&cars[1]);assert(drawnCount==old&&inlinePitExtended);}textMode=0;
 selectedRow=0;cars[1].digCtrlInput=8;setups[33].compound=player.compound=0;F1InlinePitControl(&cars[1]);assert(player.compound==1);
 cars[1].field_67=0;cars[1].flags_91=0;playerPitActive=0;F1InlinePitTick();assert(!inlinePitExtended&&!inlinePitRearmed&&!playerPitActive);
 assert(inlinePitDescriptor[3]==6&&inlinePitDescriptor[5]==6&&inlinePitDescriptor[7]==6&&inlinePitPrompt[3]==6);
 // Initial M: a planned H must not trap navigation at H -> forbidden M.
 begin();prepare();layout();F1PlayerPitEnter(&cars[1]);F1InlinePitTick();selectedRow=0;
 playerInitialCompound=1;playerPitUsedBefore=playerUsedMask=2;playerCompoundRule=1;
 autoCarPlan[34].compound[autoStint[34]]=0;player.compound=setups[33].compound=0;
 cars[1].numPitStops=0;cars[1].digCtrlInput=8;F1InlinePitControl(&cars[1]);assert(player.compound==2&&selectedColumn==2);
 cars[1].digCtrlInput=4;F1InlinePitControl(&cars[1]);assert(player.compound==0&&selectedColumn==0);
 // Initial H: select unplanned S instead of planned M, retaining zero stops.
 playerInitialCompound=0;playerPitUsedBefore=playerUsedMask=1;player.compound=setups[33].compound=1;
 cars[1].digCtrlInput=8;F1InlinePitControl(&cars[1]);assert(player.compound==2&&cars[1].numPitStops==0);
 F1PlayerStrategyService();assert(player.compound==2);F1PlayerPitLeave(&cars[1]);assert(player.compound==2);
// The old native cursor may restore old labels; suppress it only in our menu.
 cars[1].flags_91=64;cars[1].field_67=3;originalCursorCalls=0;F1InlinePitCursor(&cars[1]);assert(originalCursorCalls==0);
 StrategyPlayerPitChoice=0;F1InlinePitCursor(&cars[1]);assert(originalCursorCalls==1);StrategyPlayerPitChoice=1;
 {unsigned long old[8];memcpy(old,heights,sizeof(old));drawnCount=0;F1InlinePitAfter(&cars[1]);assert(!memcmp(old,heights,sizeof(old))&&!inlinePitSolid);}
// Complete native DOWN navigation, then its new post-return hook. No frame
 // tick, horizontal adjustment or clock progression is allowed in this test.
 selectedRow=1;inlinePitLastFocus=1;drawnCount=0;F1InlinePitAfter(&cars[1]);assert(strstr(drawn,"STOPS TO GO:-["));
 {GP2Car unchanged=cars[1];GP2CSx unchangedSetup=setups[33];unsigned long unchangedClock=clockValue;
  selectedRow=0;selectedColumn=2;drawnCount=0;
  F1InlinePitNavigationAfter(&cars[1]);
  assert(strstr(drawn,"TYRE")&&!strstr(drawn,"STOPS TO GO:-["));
  assert(selectedRow==0&&selectedColumn==2&&!inlinePitFocusPaint);
  assert(!memcmp(&unchanged,&cars[1],sizeof(unchanged))&&!memcmp(&unchangedSetup,&setups[33],sizeof(unchangedSetup))&&clockValue==unchangedClock);
  // UP to stops must show only that numerical field's brackets, then UP fuel.
  selectedRow=1;drawnCount=0;F1InlinePitNavigationAfter(&cars[1]);assert(strstr(drawn,"STOPS TO GO:-["));
  selectedRow=2;drawnCount=0;F1InlinePitNavigationAfter(&cars[1]);assert(strstr(drawn,"FUEL LAPS: -[")&&!strstr(drawn,"STOPS TO GO:-["));
  drawnCount=0;F1InlinePitNavigationAfter(&cars[1]);assert(drawnCount==0);
 }
 puts("PASS T19.3 post-navigation immediate focus paint without frame ticks or horizontal input; unchanged car/setup/clock/column; T19 opaque nonzero pixels across all glyph backgrounds/clear strips, native cursor suppression and opt-out, height restoration; T18 free alternative with zero stops, forbidden-middle skip; post-jacks native menu lifetime, nested redraw suppression, panel inset; T17 native three-field descriptor, arrow compound selection and brackets, four lines, native fuel/stops/Fire delegation, repeat gating, no clock/fuel side effects on TYRE, font restoration, optional layout restore, mandatory compound rule");return 0;}
