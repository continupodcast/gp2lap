#include "f1modules.h"
/* Keep GP2's entire message routine. Temporarily supply another native font
   and layout parameters, then restore the shared font and layout state. */
#include "stdinc.h"
#include "gp2lap.h"
#include "gp2glob.h"
#include "miscahf.h"
#include "f1native.h"
#include "f1config.h"
#include "f1nativefont.h"
static unsigned char **fontTable,*fontState,*coords,*center,*viewOffset;
static unsigned long *height;
static unsigned char *oldFont,oldState[72],oldCoords[4],oldCenter;
static unsigned long oldHeight;
static int applied;
void (*fpF1NativeOriginal)(void);
static void before(void)
{
    if(!pUseSVGA || !*pUseSVGA || applied) return;
    oldFont=fontTable[2];memcpy(oldState,fontState,sizeof(oldState));
    memcpy(oldCoords,coords,4);oldCenter=*center;oldHeight=*height;
    fontTable[2]=f1_native_font;
    /* Current font id plus cached header, glyph-table and kerning pointers.
       Install directly because GP2 may already have selected font 2. */
    *(unsigned long *)fontState=2;
    memcpy(fontState+4,f1_native_font,36);
    *(unsigned long *)(fontState+64)=(unsigned long)(f1_native_font+40);
    *(unsigned long *)(fontState+68)=0;
    coords[0]=0;coords[1]=(unsigned char)(3-(*viewOffset!=0));coords[2]=0;coords[3]=0;
    *center=255;*height=13;applied=1;
}
static void after(void)
{
    if(!applied) return;
    fontTable[2]=oldFont;memcpy(fontState,oldState,sizeof(oldState));
    memcpy(coords,oldCoords,4);*center=oldCenter;*height=oldHeight;applied=0;
}
void (*fpF1NativeBefore)(void)=before;
void (*fpF1NativeAfter)(void)=after;
int F1NativeInit(void)
{
#if F1_MODULE_NATIVE_CAPTIONS
#include "../modules/native-captions/f1native_F1NativeInit.inc"
#else
return 0;
#endif
}

/* Preserve composition and cache ownership; suppress only framebuffer copies. */
int f1NativeSuppressPixels;
void (*fpF1NativePixelsOriginal)(void);
static unsigned char *captionView;
static unsigned long pauseText;
static unsigned long captionReturns[5];
void (*fpF1NativeVisibilityOriginal)(void);
void (*fpF1NativeTextOriginal)(void);
static int __cdecl hiddenAt(unsigned long ret)
{
    if(ret==captionReturns[0]) {
        if(f1_hide_camera_caption)return 1;
        return captionView && *captionView==0x80?f1_hide_viewing_caption:f1_hide_riding_caption;
    }
    if(ret==captionReturns[1])return f1_hide_retirement_caption;
    if(ret==captionReturns[2])return 1; /* Native fastest text is always hidden. */
    if(ret==captionReturns[3])return f1_hide_winner_caption;
    if(ret==captionReturns[4])return f1_hide_pit_caption;
    return 0;
}
static int __cdecl hiddenText(unsigned long text)
{
    /* This generic function draws only. Pause/restart logic is in its caller.
       Match the validated text address, never arbitrary user-facing strings. */
    return f1_hide_pause_caption && text==pauseText;
}
int (__cdecl *fpF1NativeHiddenAt)(unsigned long)=hiddenAt;
int (__cdecl *fpF1NativeHiddenText)(unsigned long)=hiddenText;
int F1NativeFiltersInit(void)
{
#if F1_MODULE_NATIVE_CAPTIONS
#include "../modules/native-captions/f1native_F1NativeFiltersInit.inc"
#else
return 0;
#endif
}
