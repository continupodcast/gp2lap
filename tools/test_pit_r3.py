from pathlib import Path
import ctypes as C,subprocess,tempfile
from PIL import Image
root=Path(__file__).resolve().parents[1];src=root/'src';out=root.parent/'validation'
class Pit(C.Structure):_fields_=[('id',C.c_int),('position',C.c_int),('elapsed',C.c_ulong)]
with tempfile.TemporaryDirectory() as tmp:
 tmp=Path(tmp);lib=tmp/'render.so'
 subprocess.run(['gcc','-std=c89','-Wall','-Wextra','-Werror','-Wno-missing-braces','-shared','-fPIC',*[str(src/n) for n in ['f1render.c','f1timing.c','f1config.c']],'-o',str(lib)],check=True)
 api=C.CDLL(str(lib));api.F1ConfigLoad(str(root.parent/'GP2LAP.CFG').encode());theme=C.c_int.in_dll(api,'f1_theme');pal=[]
 for i in range(256):pal.extend([round(((i>>5)&7)*63/7),round(((i>>2)&7)*63/7),round((i&3)*63/3)])
 palette=(C.c_ubyte*768)(*pal);cards=(Pit*26)(*[Pit(34 if i%2 else 26,i+1,24400+i*100) for i in range(26)])
 for mode in [0,1]:
  theme.value=mode
  for n in [0,2,26]:
   original=bytes([109])*(640*480);buf=(C.c_ubyte*(640*480+128))(*([173]*64+list(original)+[173]*64));dst=C.cast(C.byref(buf,64),C.POINTER(C.c_ubyte));api.F1RenderPitDisplays(dst,palette,cards,n)
   assert bytes(buf[:64])==bytes([173])*64 and bytes(buf[-64:])==bytes([173])*64
   data=bytes(buf[64:-64]);changed=[i for i in range(len(data)) if data[i]!=original[i]]
   if n==0:assert not changed
   else:assert changed and all(any(528-(j//10)*108<=i%640<632-(j//10)*108 and 346-(j%10)*37<=i//640<380-(j%10)*37 for j in range(n)) for i in changed)
   if n==2:
    im=Image.frombytes('P',(640,480),data);im.putpalette([v*4 for v in pal]);im.convert('RGB').crop((518,300,640,388)).resize((610,440)).save(out/('pit-cards-modern.png' if mode==0 else 'pit-cards-90s.png'))
 s=(src/'f1tower.c').read_text();fn=s[s.index('void F1ToggleCard('):s.index('void F1ToggleMap(')]
 harness='''#include <assert.h>
#include <string.h>
#define PAGE_F1CARD 1
#define PAGE_F1DRIVER 2
#define PAGESETOFF(p) (activepage &= ~(p))
int activepage,F1MicroMode,f1_micro_enabled=1;unsigned char mode,*pSessionMode=&mode;
char pitNotice[40];unsigned long pitNoticeAt,clock=5000,*pCurTime=&clock;
void F1AdvancedReport(void){} void F1DiagLog(const char *s){(void)s;}
void F1TogglePart(int p){activepage^=p;}
'''+fn+'''
int main(void){mode=128;F1ToggleCard();assert(activepage==1 && !strcmp(pitNotice,"Pit signal ON") && pitNoticeAt==5000);
F1ToggleCard();assert(activepage==0 && !strcmp(pitNotice,"Pit signal OFF"));mode=0;F1ToggleCard();assert(activepage==1);F1ToggleCard();assert(F1MicroMode);return 0;}
'''
 (tmp/'test.c').write_text(harness);subprocess.run(['gcc','-std=c89','-Wall','-Wextra','-Werror',str(tmp/'test.c'),'-o',str(tmp/'test')],check=True);subprocess.run([str(tmp/'test')],check=True)
 (out/'test-pit-r3.txt').write_text('PASS: actual renderer for modern/90s, 0/2/26 cards, buffer guards and card bounds.\nPASS: actual key4 function creates ON/OFF notices and retains qualifying cycle.\nRetroArch runtime validation pending.\n')
print('PASS: renderer bounds, Futura 90s preview and key4 notices')
