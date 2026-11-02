#include <assert.h>
#include <string.h>
#include <stdio.h>
/* Model DAC address/data ports, including a completely faded native palette. */
static unsigned char dac[768],mask=0,readIndex,writeIndex;
static int readComponent,writeComponent;
static unsigned int inp(unsigned int port)
{
 unsigned int value;
 if(port==0x3c6)return mask;
 assert(port==0x3c9);value=dac[readIndex*3+readComponent++];
 if(readComponent==3){readComponent=0;readIndex++;}return value;
}
static void outp(unsigned int port,unsigned int value)
{
 if(port==0x3c6){mask=(unsigned char)value;return;}
 if(port==0x3c7){readIndex=(unsigned char)value;readComponent=0;return;}
 if(port==0x3c8){writeIndex=(unsigned char)value;writeComponent=0;return;}
 assert(port==0x3c9);dac[writeIndex*3+writeComponent++]=(unsigned char)value;
 if(writeComponent==3){writeComponent=0;writeIndex++;}
}
#include "../../src/f1playerpalette.h"
int main(void)
{
 unsigned char pal[768],saved[768],original[768];unsigned int oldMask;int i;
 memset(dac,0,sizeof(dac));mask=0;oldMask=F1PlayerPaletteCapture(saved);
 F1PlayerPaletteBuild(pal);for(i=0;i<768;i++)assert(pal[i]<=63);
 assert(memcmp(pal,saved,768));F1PlayerPalettePresent(pal,255);
 assert(mask==255 && !memcmp(dac,pal,768));assert(dac[3*3]>dac[1*3]+40);
 F1PlayerPalettePresent(saved,oldMask);for(i=0;i<768;i++)assert(dac[i]==0);assert(mask==0);
 for(i=0;i<768;i++)dac[i]=(unsigned char)((i*19+5)%64);memcpy(original,dac,768);mask=0x7f;
 oldMask=F1PlayerPaletteCapture(saved);F1PlayerPalettePresent(pal,255);
 F1PlayerPalettePresent(saved,oldMask);assert(mask==0x7f && !memcmp(dac,original,768));
 puts("PASS T14.1 visible temporary palette from all-black DAC and zero mask; exact restoration of black and arbitrary native palette/mask");return 0;
}
