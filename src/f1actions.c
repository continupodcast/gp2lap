#include <string.h>
#include "f1actions.h"
void F1FuelCancel(F1FuelState *s) { s->active=0; memset(s->targets,0,sizeof(s->targets)); }
int F1FuelKey(F1FuelState *s,unsigned int scan)
{
    if(scan==(8|128)) { s->held=0; return 0; }
    if(scan!=8 || s->held) return 0;
    s->held=1; return 1;
}
void F1FuelStart(F1FuelState *s,const unsigned char *targets,unsigned long now,unsigned long duration)
{
    memcpy(s->targets,targets,sizeof(s->targets));
    s->start=s->last=now; s->duration=duration; s->active=duration>=100 && duration<=30000;
}
int F1FuelTick(F1FuelState *s,unsigned long now,int allowed)
{
    if(!allowed || now<s->last || now-s->start>=s->duration) F1FuelCancel(s);
    s->last=now; return s->active;
}
int F1FuelTarget(const F1FuelState *s,unsigned int id)
{
    id&=63;return s->active && id>=1 && id<=40 && s->targets[id];
}
/* Skip drawing only; the original timer/flag cleanup at 6e772 still runs.
   This signature has no relocated pointers. Never patch an unknown layout. */
int F1PatchCameraCaption(unsigned char *code)
{
    static const unsigned char expected[]={0x60,0xb8,0,0,0,0,0xe8,0x9d,0x2c,0,0,0x0f,0x85,0xc6,0,0,0};
    static const unsigned char replacement[]={0xe9,0xc7,0,0,0,0x90};
    if(memcmp(code,expected,sizeof(expected))) return 0;
    memcpy(code+11,replacement,sizeof(replacement));return 1;
}
/* Whole retirement banner: skip its name, suffix, background and copy.
   Preserve the visibility check and balanced pushad/popad. No relocated bytes. */
int F1PatchRetirementCaption(unsigned char *code)
{
    static const unsigned char expected[]={0x60,0xb8,3,0,0,0,0xe8,2,0x2b,0,0,0x75,0x69};
    if(memcmp(code,expected,sizeof(expected)) || code[0x76]!=0x61 || code[0x77]!=0xc3) return 0;
    code[11]=0xeb;return 1;
}
