/* Keep GP2's entire message routine. Temporarily supply another native font
   and layout parameters, then restore the shared font and layout state. */
#include "stdinc.h"
#include "gp2lap.h"
#include "gp2glob.h"
#include "miscahf.h"
#include "f1native.h"
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
