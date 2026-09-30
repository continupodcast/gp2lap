/* Test-only substitute for DOS video hardware. The actual rectangle-transfer
   and restoration implementation in f1video.c is compiled unchanged below it. */
#include <string.h>
#include "../src/f1qualy.h"
#include "../src/f1render.h"
#include "../src/pages.h"
typedef struct { unsigned char id; } GP2Car;
unsigned long activepage,host_clock;
unsigned char host_svga=1,host_mode=0x40;
unsigned char *pUseSVGA=&host_svga,*pSessionMode=&host_mode;
unsigned long *pCurTime=&host_clock;
GP2Car host_car={34},*host_car_ptr=&host_car;
GP2Car **ppCockpitCS=&host_car_ptr,**ppSelectedCS=&host_car_ptr;
unsigned char host_video[640*480],host_palette[768];
unsigned long host_bank=1;
unsigned short host_gran=64;
int host_failure;
unsigned short GetSvgaGranularity(void) { return host_gran; }
unsigned char *IDACodeReftoDataRef(unsigned long p) { (void)p;return host_palette; }
unsigned long F1BankGet(void) { return host_failure?0xffffffffUL:host_bank; }
int F1BankSet(unsigned long bank) { host_bank=bank;return 1; }
unsigned char *host_window(unsigned long offset) { return host_video+host_bank*host_gran*1024+offset; }
int host_micro;
unsigned char host_micro_colors[30];
void F1AdvancedCard(unsigned char *dst,const unsigned char *pal,int focus,int cockpit)
{
    (void)focus;
    if(host_micro) F1RenderMicro(dst,pal,host_micro_colors,cockpit);
}
