#include "f1modules.h"
#ifndef F1INLINEPIT_H
#define F1INLINEPIT_H
/* T17: extend native race pit navigation to three fields and four display
   lines. Native fuel, stops, repeat timing and Fire handling are retained. */
static unsigned long *inlinePitMenu,*inlinePitRequests,*inlinePitStride;
static unsigned long *inlinePitFonts,*inlinePitHeights;
static unsigned char **inlinePitImageBase;
static unsigned char *inlinePitCoords,*inlinePitMap,*inlinePitDirty,*inlinePitTextMode;
static int inlinePitReady,inlinePitExtended,inlinePitNativeDepth,inlinePitRearmed;
static int inlinePitLastFocus=-1,inlinePitFocusPaint;
void (__cdecl *fpF1InlinePitNavigationOriginal)(void);
void __cdecl F1InlinePitNavigationWrapped(void);
static unsigned long *inlinePitRow,*inlinePitColumn,*inlinePitPromptRef,*inlinePitClearRef;
static unsigned long inlinePitOriginalPrompt,inlinePitOriginalClear;
static unsigned char inlinePitPrompt[]={22,20,0,6,'P','R','E','S','S',' ','F','I','R','E',' ','T','O',' ','G','O',0,255};
static unsigned char inlinePitClear[]={22,20,0,6,' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',0,255};
static unsigned char *inlinePitAction;
static unsigned char **inlinePitDescriptors,*inlinePitOriginalDescriptor;
/* Native navigation walks upwards through increasing row indices. */
static unsigned char inlinePitDescriptor[]={3,3,19,6,18,6,17,6,2,0,0,
 30,32,0,0,33,35,0,0,36,38,0,0,
 34,37,0,0,34,37,0,0,34,37,0,0,
 34,37,0,0,34,37,0,0,34,37,0,0};
void (__cdecl *fpF1InlinePitControlOriginal)(void);
void (__cdecl *fpF1InlinePitPollOriginal)(void);
void __cdecl F1InlinePitControlWrapped(void);
void __cdecl F1InlinePitRunControl(GP2Car *);
void __cdecl F1InlinePitRunPoll(GP2Car *);
void __cdecl F1InlinePitRunDraw(GP2Car *);
static int F1InlinePitAllowed(GP2Car *c);
static int inlinePitBackground=1,inlinePitSolid;
static unsigned char inlinePitGlyph[32*640];
void (__cdecl *fpF1InlinePitCursorOriginal)(void);
void __cdecl F1InlinePitCursorWrapped(void);
void __cdecl F1InlinePitRunCursor(GP2Car *);
static void __cdecl F1InlinePitCursor(GP2Car *c)
{
 if(!F1InlinePitAllowed(c))F1InlinePitRunCursor(c);
}
void (__cdecl *fpF1InlinePitCursor)(GP2Car *)=F1InlinePitCursor;
void (__cdecl *fpF1InlinePitOriginal)(void);
static void (*inlinePitChar)(unsigned int);
extern int F1CockpitView;
void __cdecl F1InlinePitWrapped(void);
static int F1InlinePitAllowed(GP2Car *c)
{
 int id;if(!inlinePitReady || !StrategyPlayerEnabled || !StrategyPlayerPitChoice ||
  !PerCarTyreCompounds || !pUseSVGA || !*pUseSVGA || !pSessionMode || *pSessionMode!=0x80 ||
  !pIsReplay || *pIsReplay || !F1CockpitView || !pCarSetups || !c || !(c->id&128))return 0;
 id=c->id&63;
 return (playerPitActive || ((c->flags_91&64) && c->field_67==3)) && id>=1 && id<=40 && (id==playerArmedId || id==playerNativeId) &&
  *inlinePitMenu==6;
}
/* Native glyphs are drawn through GP2 itself, so cockpit buffering/banking,
   palette and refresh behaviour match the fuel/stops rows. Recolour only a
   temporary copy of the active glyph and restore the font pointer/state. */
static void F1InlinePitLetter(int ch,int colour)
{
 unsigned int glyph,group,index,r,x,h,stride;unsigned long saved;
 unsigned char *src;
 glyph=(unsigned int)inlinePitMap[ch]-32U;if(glyph>=256U)return;
 group=glyph>>5;index=glyph&31;h=(unsigned int)inlinePitHeights[group];stride=(unsigned int)*inlinePitStride;
 if(h<1 || h>32 || stride!=640)return;
 saved=inlinePitFonts[group];src=*inlinePitImageBase+saved+index*16;
 for(r=0;r<h;r++)for(x=0;x<10;x++)inlinePitGlyph[r*stride+x]=(inlinePitSolid || src[r*stride+x])?(unsigned char)colour:(unsigned char)inlinePitBackground;
 inlinePitFonts[group]=(unsigned long)inlinePitGlyph-(unsigned long)*inlinePitImageBase-index*16;
 inlinePitChar((unsigned int)ch);inlinePitFonts[group]=saved;
}
static void F1InlinePitText(const char *s,int colour)
{
 while(*s)F1InlinePitLetter((unsigned char)*s++,colour);
}
static int F1InlinePitColour(const unsigned char *pal,int r,int g,int b)
{
 int i,best=0,d,dr,dg,db,min=0x7fffffff;
 for(i=0;i<256;i++){dr=(int)pal[i*3]*4-r;dg=(int)pal[i*3+1]*4-g;db=(int)pal[i*3+2]*4-b;
  d=dr*dr+dg*dg+db*db;if(d<min){min=d;best=i;}}
 return best;
}
static void F1InlinePitRestoreLayout(void)
{
 if(!inlinePitExtended)return;
 inlinePitDescriptors[6]=inlinePitOriginalDescriptor;
 if(inlinePitPromptRef)*inlinePitPromptRef=inlinePitOriginalPrompt;
 if(inlinePitClearRef)*inlinePitClearRef=inlinePitOriginalClear;
 if(*inlinePitMenu==6){*inlinePitRow=*inlinePitRow==2?1:0;*inlinePitColumn=0;}
 inlinePitExtended=0;
}
static void F1InlinePitSetLayout(void)
{
 if(inlinePitExtended)return;
 *inlinePitRow=*inlinePitRow==1?2:1;*inlinePitColumn=0;
 inlinePitDescriptors[6]=inlinePitDescriptor;
 if(inlinePitPromptRef)*inlinePitPromptRef=(unsigned long)inlinePitPrompt;
 if(inlinePitClearRef)*inlinePitClearRef=(unsigned long)inlinePitClear;
 inlinePitExtended=1;*inlinePitDirty=255;
}
static void F1InlinePitTick(void)
{
 GP2Car *c=ppCockpitCS?*ppCockpitCS:NULL;
 if(!F1InlinePitAllowed(c)){
  F1InlinePitRestoreLayout();inlinePitLastFocus=-1;
  if(inlinePitRearmed){F1PlayerStrategyService();playerPitActive=0;playerPitOverride=-1;inlinePitRearmed=0;}
 }else {
  if(!playerPitActive){playerPitActive=1;inlinePitRearmed=1;}
  F1InlinePitSetLayout();
  /* Request GP2's usual repaint after a vertical focus change. Do not draw
     inside the native input/repeat poll: that path must remain T19-identical. */
  if(inlinePitLastFocus!=(int)*inlinePitRow){inlinePitLastFocus=(int)*inlinePitRow;*inlinePitDirty=255;}
 }
}
static void F1InlinePitLine(int row,const char *text,int colour)
{
 char padded[22];int n=(int)strlen(text);
 if(n>20)n=20;memset(padded,' ',20);memcpy(padded,text,n);padded[20]=0;
 inlinePitCoords[0]=22;inlinePitCoords[1]=(unsigned char)row;inlinePitCoords[2]=0;inlinePitCoords[3]=6;
 F1InlinePitText(padded,colour);
}
static void __cdecl F1InlinePitAfter(GP2Car *c)
{
 unsigned char saved[11],*pal;unsigned int group;unsigned long oldHeight;int id,selected,white,colours[3],i,stops,fuel;char line[40];
 /* Repaint masks and text-command capture are transient drawing states,
    not evidence that the native pit menu has closed. Never restore the old
    two-row renderer from a nested/capture pass. */
 if(inlinePitNativeDepth || (inlinePitTextMode && *inlinePitTextMode==1))return;
 if(!F1InlinePitAllowed(c)){F1InlinePitRestoreLayout();F1InlinePitRunDraw(c);return;}
 if(!playerPitActive){playerPitActive=1;inlinePitRearmed=1;}
 F1InlinePitSetLayout();id=c->id&63;if(!inlinePitFocusPaint)F1PlayerStrategyService();
 selected=pCarSetups[id-1].compound;if(selected>2)return;
 pal=IDACodeReftoDataRef(0x7f624);if(!pal)return;
 white=F1InlinePitColour(pal,240,240,240);colours[0]=white;
 colours[1]=F1InlinePitColour(pal,255,225,0);colours[2]=F1InlinePitColour(pal,255,40,40);
 /* Native transparency restores the original cockpit bitmap, which still
    contains the old FUEL/STOPS labels. Use an opaque non-zero black index. */
 inlinePitBackground=1;
 for(i=2;i<256;i++)if((int)pal[i*3]+pal[i*3+1]+pal[i*3+2] <
  (int)pal[inlinePitBackground*3]+pal[inlinePitBackground*3+1]+pal[inlinePitBackground*3+2])inlinePitBackground=i;
 group=((unsigned int)inlinePitMap[' ']-32U)>>5;if(group>=8)return;
 memcpy(saved,inlinePitCoords,11);inlinePitCoords[7]=inlinePitCoords[8]=0;
 /* Opaque strips overlap vertically, clearing stale text in the inter-row
    gaps too. GP2 still performs all native buffer/bank writes. */
 oldHeight=inlinePitHeights[group];inlinePitHeights[group]=32;inlinePitSolid=1;
 for(i=18;i<=20;i++)F1InlinePitLine(i,"",inlinePitBackground);
 inlinePitSolid=0;inlinePitHeights[group]=oldHeight;
 fuel=c->fuelLoadLaps>>6;stops=(int)c->numPitStops-(int)c->numPitStopsDone;if(stops<0)stops=0;
 sprintf(line,*inlinePitRow==2?"FUEL LAPS: -[%2d]+":"FUEL LAPS: - %2d +",fuel);F1InlinePitLine(17,line,white);
 sprintf(line,*inlinePitRow==1?"STOPS TO GO:-[%2d]+":"STOPS TO GO:- %2d +",stops);F1InlinePitLine(18,line,white);
 F1InlinePitLine(19,"",white);inlinePitCoords[0]=22;
 F1InlinePitText("TYRE    ",white);
 for(i=0;i<3;i++){
  F1InlinePitLetter(i==selected?'[':' ',white);F1InlinePitLetter("HMS"[i],colours[i]);F1InlinePitLetter(i==selected?']':' ',white);
 }
 F1InlinePitLine(20,"PRESS FIRE TO GO",white);
 if(!inlinePitFocusPaint && *inlinePitRow==0)*inlinePitColumn=(unsigned long)selected;
 memcpy(inlinePitCoords,saved,11);
}
/* Called after GP2 has committed the vertical row change, before its caller
   branches on Carry. Painting here cannot rewrite the navigation result,
   digital input, repeat timer or service state. The ASM wrapper preserves all
   registers and the original Carry result across this read-only paint. */
static void __cdecl F1InlinePitNavigationAfter(GP2Car *c)
{
 if(!F1InlinePitAllowed(c) || !inlinePitExtended || inlinePitNativeDepth ||
  (inlinePitTextMode && *inlinePitTextMode==1))return;
 if(inlinePitLastFocus==(int)*inlinePitRow)return;
 inlinePitLastFocus=(int)*inlinePitRow;
 inlinePitFocusPaint=1;F1InlinePitAfter(c);inlinePitFocusPaint=0;
}
void (__cdecl *fpF1InlinePitNavigationAfter)(GP2Car *)=F1InlinePitNavigationAfter;
/* Native digital menu controls, including its existing repeat timer and Fire.
   H/M/S shortcuts are removed. The two existing rows delegate to GP2 unchanged. */
static void F1InlinePitNativeControl(GP2Car *c,unsigned long row)
{
 unsigned long old=*inlinePitRow;
 inlinePitDescriptors[6]=inlinePitOriginalDescriptor;*inlinePitRow=row;*inlinePitColumn=0;
 inlinePitNativeDepth++;F1InlinePitRunControl(c);inlinePitNativeDepth--;
 inlinePitDescriptors[6]=inlinePitDescriptor;*inlinePitRow=old;*inlinePitColumn=0;
}
static void __cdecl F1InlinePitControl(GP2Car *c)
{
 int compound,direction,current,required;
 if(!F1InlinePitAllowed(c)){F1InlinePitRestoreLayout();F1InlinePitRunControl(c);return;}
 F1InlinePitSetLayout();
 if(!playerPitActive){playerPitActive=1;inlinePitRearmed=1;}
 if(*inlinePitRow!=0){F1InlinePitNativeControl(c,*inlinePitRow==2?1:0);F1InlinePitAfter(c);return;}
 F1InlinePitRunPoll(c);
 if(*inlinePitAction!=255)return;
 if(c->digCtrlInput&16){F1InlinePitNativeControl(c,0);F1InlinePitAfter(c);return;}
 current=pCarSetups[(c->id&63)-1].compound;
 if(c->digCtrlInput&4)direction=-1;else if(c->digCtrlInput&8)direction=1;else return;
 compound=current+direction;
 /* Zero further stops only excludes the initial compound; the player's
    other alternative need not be the one scheduled by the original plan.
    Skip a forbidden middle compound so both legal alternatives are reachable. */
 required=playerCompoundRule && c->numPitStops==0 && playerPitUsedBefore &&
  !(playerPitUsedBefore&(playerPitUsedBefore-1));
 if(required && compound==playerInitialCompound)compound+=direction;
 if(compound<0 || compound>2)compound=current;
 if(required && compound==playerInitialCompound)compound=F1PlayerNextDifferent(c->id&63,compound);
 playerPitOverride=compound;F1PlayerStrategyService();c->flags_169|=128;
 *inlinePitColumn=pCarSetups[(c->id&63)-1].compound;*inlinePitDirty=255;F1InlinePitAfter(c);
}
void (__cdecl *fpF1InlinePitControl)(GP2Car *)=F1InlinePitControl;
void (__cdecl *fpF1InlinePitAfter)(GP2Car *)=F1InlinePitAfter;
static int F1InlinePitInit(void)
{
#if F1_MODULE_PIT_COMPOUND_SELECTOR
#include "../modules/pit-compound-selector/f1inlinepit_F1InlinePitInit.inc"
#else
return 0;
#endif
}
/* All navigation now comes from the native menu, not GP2Lap key events. */
int F1PlayerInlinePitKey(unsigned int scan,int control){(void)scan;(void)control;return 0;}
#endif
