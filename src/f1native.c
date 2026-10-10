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
    static const unsigned long calls[]={0x6e234,0x6e768,0x6e8a2,0x6e95f,0x6e9d7,0x6eab9};
    unsigned int i;unsigned char *p,*original=IDAtoFlat(0x6d792);
    /* All-or-nothing validation before any instruction is modified. */
    if(memcmp(original,"\x60\xb0\x20\xaa\xb0\x00\xaa",7)) return 0;
    for(i=0;i<sizeof(calls)/sizeof(calls[0]);i++) {
        p=IDAtoFlat(calls[i]);
        if(p[0]!=0xe8 || p+5+*(long *)(p+1)!=original) return 0;
    }
    fontTable=(unsigned char **)IDACodeReftoDataRef(0x7bc6b);
    fontState=IDACodeReftoDataRef(0x7bc2f);
    coords=IDACodeReftoDataRef(0x6e1e3);
    center=IDACodeReftoDataRef(0x7118d);
    viewOffset=IDACodeReftoDataRef(0x711a2);
    height=(unsigned long *)IDACodeReftoDataRef(0x7d7d9);
    if(!fontTable || !fontState || !coords || !center || !viewOffset || !height) return 0;
    fpF1NativeOriginal=(void (*)(void))original;
    for(i=0;i<sizeof(calls)/sizeof(calls[0]);i++) {
        p=IDAtoFlat(calls[i]);*(long *)(p+1)=(unsigned char *)F1NativeCaptionWrapped-(p+5);
    }
    return 1;
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
    static const unsigned long checks[]={0x6e6a1,0x6e83c,0x6e8b4,0x6e971,0x6e9e9};
    static const unsigned long textCalls[]={0x3487f,0x34ab1,0x6e65b,0x6e665,0x6e7c3,0x6e7ed,0x6e805,0x6e826,0x6e830};
    static const unsigned long pixelCalls[]={0x71358,0x6d7a8};
    unsigned int i;unsigned char *p,*pixels=IDAtoFlat(0x7120c),*visibility=IDAtoFlat(0x71343),*generic=IDAtoFlat(0x6e1c2);
    unsigned char *pause=IDACodeReftoDataRef(0x3487b);
    if(IDAtoFlat(0x3487a)[0]!=0xb8 || !pause || memcmp(pause,"\x13\x07\x06\x02?PAUSED",11))return 0;
    if(memcmp(visibility,"\xf6\x05",2) || memcmp(generic,"\xff\x35",2))return 0;
    for(i=0;i<sizeof(checks)/sizeof(checks[0]);i++) {
        p=IDAtoFlat(checks[i]);if(p[0]!=0xe8 || p+5+*(long *)(p+1)!=visibility)return 0;
        if(!(p[5]==0x75 || (p[5]==0x0f && p[6]==0x85)))return 0;
    }
    for(i=0;i<sizeof(textCalls)/sizeof(textCalls[0]);i++) {
        p=IDAtoFlat(textCalls[i]);if(p[0]!=0xe8 || p+5+*(long *)(p+1)!=generic)return 0;
    }
    if(memcmp(pixels,"\x80\x3d",2))return 0;
    for(i=0;i<2;i++){p=IDAtoFlat(pixelCalls[i]);if(p[0]!=0xe8 || p+5+*(long *)(p+1)!=pixels)return 0;}
    captionView=IDACodeReftoDataRef(0x6e6dd);
    if(!captionView)return 0;
    pauseText=(unsigned long)pause;
    f1NativeSuppressPixels=0;fpF1NativePixelsOriginal=(void (*)(void))pixels;
    for(i=0;i<2;i++){p=IDAtoFlat(pixelCalls[i]);*(long *)(p+1)=(unsigned char *)F1NativePixelsWrapped-(p+5);}
    fpF1NativeVisibilityOriginal=(void (*)(void))visibility;
    fpF1NativeTextOriginal=(void (*)(void))generic;
    for(i=0;i<sizeof(checks)/sizeof(checks[0]);i++) {
        p=IDAtoFlat(checks[i]);captionReturns[i]=(unsigned long)(p+5);
        *(long *)(p+1)=(unsigned char *)F1NativeVisibilityWrapped-(p+5);
    }
    for(i=0;i<sizeof(textCalls)/sizeof(textCalls[0]);i++) {
        p=IDAtoFlat(textCalls[i]);*(long *)(p+1)=(unsigned char *)F1NativeTextWrapped-(p+5);
    }
    return 1;
}
