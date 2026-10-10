import ctypes as C, subprocess,tempfile
from pathlib import Path
from PIL import Image
root=Path(__file__).resolve().parents[1]
class Card(C.Structure):
 _fields_=[(k,C.c_int) for k in ['id','pos','pit','outlap','post','pole','showGap','hasGap','leaderId']]+[(k,C.c_long) for k in ['running','last','gap','leaderBest']]+[('color',C.c_int*3)]
with tempfile.TemporaryDirectory() as tmp:
 tmp=Path(tmp);lib=tmp/'renderer.so'
 subprocess.run(['gcc','-std=c89','-Wall','-Wextra','-Werror','-Wno-missing-braces','-shared','-fPIC',*[str(root/'src'/n) for n in ['f1render.c','f1timing.c','f1config.c']],'-o',str(lib)],check=True)
 api=C.CDLL(str(lib));api.F1ConfigEnsure();theme=C.c_int.in_dll(api,'f1_theme')
 pal=[]
 for i in range(256):pal.extend([round(((i>>5)&7)*63/7),round(((i>>2)&7)*63/7),round((i&3)*63/3)])
 palette=(C.c_ubyte*768)(*pal);card=Card(id=34,pos=13,running=78400,leaderBest=95505,leaderId=16)
 card.color[:]=[3,1,2];colors=(C.c_ubyte*30)(*([3,1,2,0,1,3,2,1,0,1]*3))
 original=bytes([109])*(640*480)
 for mode in [0,1]:
  theme.value=mode
  for cockpit in [0,1]:
   guard=64;buf=(C.c_ubyte*(640*480+2*guard))(*([173]*guard+list(original)+[173]*guard));dst=C.cast(C.byref(buf,guard),C.POINTER(C.c_ubyte))
   api.F1RenderQualyCard(dst,palette,C.byref(card),cockpit);api.F1RenderMicro(dst,palette,colors,cockpit)
   assert bytes(buf[:guard])==bytes([173])*guard and bytes(buf[-guard:])==bytes([173])*guard
   result=bytes(buf[guard:-guard]);changed=[i for i,v in enumerate(result) if v!=original[i]]
   left,top,width,height=(180,8,280,50) if cockpit else (215,302,210,84)
   assert changed and all(left<=i%640<left+width and top<=i//640<top+height for i in changed)
   bar_y=top+(43 if cockpit else 77)
   assert any(bar_y<=i//640<bar_y+3 for i in changed)
   if True:
    im=Image.frombytes('P',(640,480),result);im.putpalette([v*4 for v in pal]);im.convert('RGB').resize((1280,960)).save(root/'validation'/('cards-'+('modern' if mode==0 else '90s')+('-onboard.png' if cockpit else '-tv.png')))
 # Exercise the actual toggle functions in a minimal host harness.
 s=(root/'src/f1tower.c').read_text()
 def fn(name,nextname):return s[s.index('void '+name+'('):s.index('void '+nextname+'(',s.index('void '+name+'('))]
 cardfn=fn('F1ToggleCard','F1ToggleMap');driverfn=fn('F1ToggleDriver','F1Tab')
 harness='''#include <assert.h>
#define PAGE_F1CARD 1
#define PAGE_F1DRIVER 2
#define PAGESETOFF(p) (activepage &= ~(p))
int activepage,F1MicroMode,f1_micro_enabled=1,F1CockpitView,driverDiff;
unsigned char mode; unsigned char *pSessionMode=&mode;
void F1AdvancedReport(void){} void F1DiagLog(const char *s){(void)s;}
void F1TogglePart(int part){activepage ^= part;}
'''+cardfn+driverfn+'''
int main(void){
mode=0x40;activepage=PAGE_F1DRIVER;F1ToggleCard();assert(activepage==PAGE_F1CARD);
F1ToggleCard();assert(F1MicroMode && activepage==PAGE_F1CARD);
F1ToggleDriver();assert(activepage==PAGE_F1DRIVER && !F1MicroMode);
F1ToggleDriver();assert(!activepage);
mode=0;activepage=PAGE_F1DRIVER;F1ToggleCard();assert(activepage==PAGE_F1CARD);
F1ToggleDriver();assert(activepage==PAGE_F1DRIVER);
mode=0x80;activepage=PAGE_F1CARD;F1ToggleDriver();assert(activepage==3);
F1ToggleDriver();assert(driverDiff && activepage==3);
return 0;}
'''
 (tmp/'toggle.c').write_text(harness);subprocess.run(['gcc','-std=c89','-Wall','-Wextra','-Werror',str(tmp/'toggle.c'),'-o',str(tmp/'toggle')],check=True);subprocess.run([str(tmp/'toggle')],check=True)
print('PASS: TV bottom-centre and compact onboard top-centre bounds, both themes and camera views, microsector alignment, key 4/5 exclusion and race cycle preserved.')
