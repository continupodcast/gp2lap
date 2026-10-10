from pathlib import Path
import struct, subprocess,tempfile
root=Path(__file__).resolve().parents[1]
s=(root/'src/f1native.c').read_text()
helpers=s[s.index('static unsigned char *captionView;'):s.index('int F1NativeFiltersInit(void)')]
shim='''#include <assert.h>
#include <stdio.h>
#define __cdecl
#include "f1config.h"
'''
main='''
int main(void){unsigned char view=0;FILE *f;int i;
captionView=&view;pauseText=0x12345678UL;
for(i=0;i<5;i++)captionReturns[i]=0x60000000UL+i;
assert(hiddenAt(captionReturns[2])==1);
assert(hiddenAt(0)==0 && hiddenText(pauseText)==0);
f1_hide_riding_caption=1;assert(hiddenAt(captionReturns[0])==1);
view=0x80;assert(hiddenAt(captionReturns[0])==0);
f1_hide_viewing_caption=1;assert(hiddenAt(captionReturns[0])==1);
f1_hide_camera_caption=1;f1_hide_riding_caption=f1_hide_viewing_caption=0;
view=0;assert(hiddenAt(captionReturns[0])==1);view=0x80;assert(hiddenAt(captionReturns[0])==1);
f1_hide_retirement_caption=f1_hide_winner_caption=f1_hide_pit_caption=1;
assert(hiddenAt(captionReturns[1]) && hiddenAt(captionReturns[3]) && hiddenAt(captionReturns[4]));
f1_hide_pause_caption=1;assert(hiddenText(pauseText) && !hiddenText(pauseText+1));
f=fopen("controls.cfg","w");assert(f);
fputs("[F1 Controls]\\nHideCameraCaption = 0\\nHideRidingCaption = 1\\nHideViewingCaption = 0\\nHideRetirementCaption = 1\\nHideRaceWinnerCaption = 1\\nHidePitCaption = 1\\nHidePauseCaption = 1\\n",f);fclose(f);
assert(F1ConfigLoad("controls.cfg")>=0);
assert(!f1_hide_camera_caption && f1_hide_riding_caption && !f1_hide_viewing_caption);
assert(f1_hide_retirement_caption && f1_hide_winner_caption && f1_hide_pit_caption && f1_hide_pause_caption);
assert(hiddenAt(captionReturns[2])==1);
return 0;}
'''
with tempfile.TemporaryDirectory() as td:
 td=Path(td);(td/'test.c').write_text(shim+helpers+main)
 subprocess.run(['gcc','-std=c89','-Wall','-Wextra','-Werror','-I'+str(root/'src'),str(td/'test.c'),str(root/'src/f1config.c'),'-o',str(td/'test')],check=True)
 subprocess.run([str(td/'test')],cwd=td,check=True)
# Executable signatures / targets / skip destinations, before relocation.
exe=(root.parents[1]/'upload/GP2.EXE').read_bytes()
def raw(addr,size):
 offset=0x88254+addr-0x10000
 return exe[offset:offset+size]
def target(addr):return addr+5+struct.unpack('<i',raw(addr+1,4))[0]
for addr,end in [(0x6e6a1,0x6e772),(0x6e83c,0x6e8ac),(0x6e8b4,0x6e969),(0x6e971,0x6e9e1),(0x6e9e9,0x6eac3)]:
 assert raw(addr,1)==b'\xe8' and target(addr)==0x71343
 b=raw(addr+5,6)
 skip=addr+7+struct.unpack('b',b[1:2])[0] if b[0]==0x75 else addr+11+struct.unpack('<i',b[2:6])[0]
 assert skip==end
for addr in [int(v,16) for v in (root/'tools/caption_text_calls.txt').read_text().split(',')]:
 assert raw(addr,1)==b'\xe8' and target(addr)==0x6e1c2
assert raw(0x3487a,1)==b'\xb8' and raw(0x3487b,4)==struct.pack('<I',0x39e35)
assert exe[0x123254+0x39e35:0x123254+0x39e35+11]==b'\x13\x07\x06\x02?PAUSED'
print('PASS: real C filter decisions, legacy camera switch, all CFG controls, fastest unconditional, exact pause identification and GP2 call/cleanup signatures.')
