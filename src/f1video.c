#include "f1advanced.h"
/* Bottom-right cockpit overlay: draw after GP2's cockpit copy completes.
   GP2 skips static cockpit pixels when copying its scene buffer, so those
   pixels cannot be overlaid by the scene-buffer hook alone. */
#ifdef F1_VIDEO_TEST
#include "../tools/video_test_shim.h"
#else
#include "stdinc.h"
#include "gp2lap.h"
#include "gp2glob.h"
#include "miscahf.h"
#include "pages.h"
#include "svga/vesa.h"
#include "f1qualy.h"
#endif
static unsigned char work[640*480];
static unsigned char under[F1_CARD_W*F1_CARD_H],painted[F1_CARD_W*F1_CARD_H];
static int saved;
#ifndef F1_VIDEO_TEST
unsigned long F1BankGet(void);
#pragma aux F1BankGet = \
    "mov eax,4f05h" \
    "mov ebx,0100h" \
    "int 10h" \
    "cmp ax,004fh" \
    "jne failed" \
    "movzx eax,dx" \
    "jmp done" \
    "failed: mov eax,0ffffffffh" \
    "done:" \
    value [eax] modify [ebx ecx edx];
int F1BankSet(unsigned long bank);
#pragma aux F1BankSet = \
    "mov eax,4f05h" \
    "xor ebx,ebx" \
    "int 10h" \
    "cmp ax,004fh" \
    "sete al" \
    "movzx eax,al" \
    parm [edx] value [eax] modify [ebx ecx edx];
#endif
/* Transfer only our rectangle. Original window A bank is restored; B untouched. */
static int transfer(int operation)
{
    unsigned long previous,bank=0xffffffffUL,offset,wanted;
    unsigned int gran,y,x,i=0;unsigned char *pixel;int ok=1;
    gran=GetSvgaGranularity();if(!gran || gran>64 || 64%gran) return 0;
    previous=F1BankGet();if(previous==0xffffffffUL) return 0;
    for(y=F1_CARD_Y_COCKPIT;y<F1_CARD_Y_COCKPIT+F1_CARD_H && ok;y++) {
        for(x=F1_CARD_X;x<F1_CARD_X+F1_CARD_W;x++,i++) {
            offset=(unsigned long)y*640+x;wanted=(offset>>16)*(64/gran);
            if(bank!=wanted) { if(!F1BankSet(wanted)) { ok=0;break; }bank=wanted; }
            #ifdef F1_VIDEO_TEST
            pixel=host_window(offset&0xffffUL);
#else
            pixel=(unsigned char *)(0xa0000UL+(offset&0xffffUL));
#endif
            if(operation==0) {
                /* Do not undo pixels independently updated by GP2 since our draw. */
                if(*pixel==painted[i]) *pixel=under[i];
            } else if(operation==1) { under[i]=*pixel;work[offset]=*pixel; }
            else { painted[i]=work[offset];*pixel=painted[i]; }
        }
    }
    if(!F1BankSet(previous)) ok=0;
    return ok;
}
void F1VideoReset(void) { saved=0; }
void F1CockpitBefore(void)
{
    if(saved && pUseSVGA && *pUseSVGA) transfer(0);
    saved=0;
}
void F1CockpitAfter(void)
{
    int focus=0;unsigned char *pal;F1QCard card;
    if(!(activepage&PAGE_F1CARD) || !pUseSVGA || !*pUseSVGA || !pSessionMode || (*pSessionMode&0x80) || !pCurTime) return;
    if(ppCockpitCS && *ppCockpitCS) focus=(*ppCockpitCS)->id&0x3f;
    else if(ppSelectedCS && *ppSelectedCS) focus=(*ppSelectedCS)->id&0x3f;
    if(!F1QGetCard(focus,*pCurTime,&card)) return;
    pal=IDACodeReftoDataRef(0x7f624);if(!pal || !transfer(1)) return;
    F1RenderQualyCard(work,pal,&card,1);
    F1AdvancedCard(work,pal,focus,1);
    saved=transfer(2);
}
void (*fpF1CockpitBefore)(void)=F1CockpitBefore;
void (*fpF1CockpitAfter)(void)=F1CockpitAfter;
void (*fpF1VideoReset)(void)=F1VideoReset;
