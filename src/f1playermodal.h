#include "f1modules.h"
#ifndef F1PLAYERMODAL_H
#define F1PLAYERMODAL_H
#include "f1playerui.h"
#include "f1playershuffle.h"
#include "f1playerpalette.h"
#include "keyqueue.h"
#include "svga/vesa.h"
/* Modal at the fresh grid: simulation is suspended by staying within this
   single frame, rather than changing a native pause flag or rewinding clocks. */
volatile unsigned long F1StrategyMenuActive=0;
static int playerModalPit,playerModalRecommended,playerModalRequired;
unsigned int F1StrategyBankGet(void);
#pragma aux F1StrategyBankGet = "mov eax,4f05h" "mov ebx,100h" "int 10h" "cmp ax,4fh" "je okbank" "mov edx,0ffffh" "okbank: and edx,0ffffh" value [edx] modify [eax ebx ecx edx];
unsigned int F1StrategyBankSet(unsigned int bank);
#pragma aux F1StrategyBankSet = "mov eax,4f05h" "xor ebx,ebx" "int 10h" "cmp ax,4fh" "sete al" "movzx eax,al" parm [edx] value [eax] modify [eax ebx ecx edx];
static int F1StrategyScreen(unsigned char *buffer,int present)
{
 unsigned long offset=0,size;unsigned int bank=0,oldbank=0,gran=GetSvgaGranularity();
 if(pLFBuf){if(present)memcpy(pLFBuf,buffer,307200);else memcpy(buffer,pLFBuf,307200);return 1;}
 if(!gran)return 0;oldbank=F1StrategyBankGet();if(oldbank==0xffff)return 0;
 while(offset<307200UL){
  size=307200UL-offset;if(size>65536UL)size=65536UL;
  if(!F1StrategyBankSet(bank*64/gran)){F1StrategyBankSet(oldbank);return 0;}
  if(present)memcpy((void *)0xa0000UL,buffer+offset,size);
  else memcpy(buffer+offset,(const void *)0xa0000UL,size);
  offset+=size;bank++;
 }
 F1StrategyBankSet(oldbank);return 1;
}
#include "f1strategyedit.h"
#include "f1strategynav.h"
static int F1PlayerEditStrategy(const AutoPlan *plans,int n,int id,int laps,int *choice)
{
 unsigned char *saved,*screen,pal[768],oldpal[768];unsigned int mask;
 static unsigned long serial;unsigned long seed;int order[10],preset=0,focus=0,side=0,held=0,activationKey=0,nativeFuel=0;
 int dirty=1,done=0,result=0,pending=0,key,i,j,quick,action,edited=0,defaultPreset=0;
 AutoPlan draft,original;F1StrategyOption option;KeyEvent event;char error[100];
 if(!pUseSVGA || !*pUseSVGA || n<1 || n>10)return 0;
 quick=autoQuickFlag && *autoQuickFlag;memset(&original,0,sizeof(original));
 if(pNumCars && pCarStructs && pCarSetups)for(i=0;i<*pNumCars;i++)if((pCarStructs[i].id&63)==id){
  GP2Car *c=&pCarStructs[i];int previous=0,end,pits[3];
  pits[0]=c->pitStop1;pits[1]=c->pitStop2;pits[2]=c->pitStop3;
  if(c->numPitStops>3 || pCarSetups[id-1].compound>2)break;
  original.count=c->numPitStops+1;nativeFuel=(c->fuelLoadLaps+32)/64;
  for(j=0;j<original.count;j++){
   end=j<original.count-1?pits[j]:laps;
   if(end<=previous || end>laps){original.count=0;break;}
   original.length[j]=(unsigned char)(end-previous);
   original.compound[j]=pCarSetups[id-1].compound;previous=end;
  }break;
 }
 seed=(unsigned long)time(NULL)^(++serial*2654435761UL)^((unsigned long)id*97UL);
 F1PlayerShuffle(order,n,&seed);
 if(!original.count)preset=defaultPreset=1;
 draft=preset?plans[order[preset-1]]:original;error[0]=0;
 saved=(unsigned char *)malloc(307200);screen=(unsigned char *)malloc(307200);
 if(!saved || !screen){free(saved);free(screen);return 0;}
 if(!F1StrategyScreen(saved,0)){free(saved);free(screen);return 0;}
 F1PlayerCircuitPrepare();
 F1PlayerPaletteBuild(pal);mask=F1PlayerPaletteCapture(oldpal);
 while(KeyEventDequeue(&event));F1StrategyMenuActive=1;
 while(!done){
  action=2*draft.count+1;
  if(dirty){
   memset(&option,0,sizeof(option));option.count=draft.count;option.stops=draft.count-1;
   memcpy(option.compound,draft.compound,4);memcpy(option.length,draft.length,4);
   option.fuel=!preset && !edited?nativeFuel:(int)((draft.length[0]+1)*1.0395305+0.999999);
   F1RenderStrategyEditor(screen,pal,&option,focus*4+side+(held?2:0),id,laps,preset,n,edited,quick,error);
   if(!F1StrategyScreen(screen,1))break;F1PlayerPalettePresent(pal,255);dirty=0;
  }
  if(!KeyEventDequeue(&event)){delay(10);continue;}
  key=event.scancode;
  if(pending){if(key==(pending|0x80)){done=1;result=1;}continue;}
  if(held){if(key==(activationKey|0x80)){held=0;dirty=1;}continue;}
  if(key&0x80)continue;
  if(key==1){*choice=-1;pending=1;continue;}
  if(key==0x48 || key==0x50 || key==0x4b || key==0x4d){
   F1StrategyNavigate(draft.count,key,&focus,&side);dirty=1;continue;
  }
  if(key==0x1c || key==0x39)activationKey=key;
  if((key==0x1c || key==0x39) && focus<action){
   int delta=side?1:-1;held=1;
   error[0]=0;
   if(focus==0){
    preset=(preset+delta+n+1)%(n+1);
    if(!preset && !original.count)preset=delta>0?1:n;
    draft=preset?plans[order[preset-1]]:original;edited=0;
   }else if(focus==1){
    if(F1DraftStops(&draft,draft.count+delta,laps,quick))edited=1;
   }else if(focus<action){
    i=(focus-2)/2;
    if((focus-2)%2==0){draft.compound[i]=(unsigned char)((draft.compound[i]+delta+3)%3);edited=1;}
    else if(F1DraftMovePit(&draft,i,delta))edited=1;
   }
   dirty=1;continue;
  }
  if(key==0x1c || key==0x39){
   if(focus==action){preset=defaultPreset;draft=preset?plans[order[preset-1]]:original;edited=0;focus=2*draft.count+1;held=1;error[0]=0;dirty=1;}
   else if(focus==action+2){*choice=-1;pending=key;held=1;dirty=1;}
   else if(focus==action+1){
    if(!preset && !edited){*choice=-1;pending=key;held=1;dirty=1;}
    else if(!F1DraftValid(&draft,laps,quick)){
     strcpy(error,StrategyRequireCompoundChange?"Use two compounds and at least one stop; stint limit: 90 laps.":"At least one race stop required; stint limit: 90 laps.");dirty=1;
    }else{playerEditedPlan=draft;*choice=-2;pending=key;held=1;dirty=1;}
   }
  }
 }
 F1StrategyMenuActive=0;if(!F1StrategyScreen(saved,1))result=0;
 F1PlayerPalettePresent(oldpal,mask);free(saved);free(screen);return result;
}
int F1PlayerStrategyChoose(const AutoPlan *plans,int n,int id,int laps,int *choice)
{
#if F1_MODULE_PLAYER_RACE_STRATEGY
#include "../modules/player-race-strategy/f1playermodal_F1PlayerStrategyChoose.inc"
#else
return 0;
#endif
}
int F1PlayerPitChoose(int id,int recommended,int required,int initial,int *choice)
{
 AutoPlan options[3];int c,n=0,index=-1,result;
 memset(options,0,sizeof(options));
 for(c=0;c<3;c++)if(!required || c!=initial){
  options[n].count=1;options[n].compound[0]=(unsigned char)c;options[n].length[0]=1;n++;
 }
 playerModalPit=1;playerModalRecommended=recommended;playerModalRequired=required;
 result=F1PlayerStrategyChoose(options,n,id,1,&index);playerModalPit=0;
 *choice=result && index>=0?options[index].compound[0]:-1;return result;
}
#endif
