"""0.6 precision, layout and real rectangle transfer with mocked video banks."""
import ctypes as C,subprocess,tempfile
from pathlib import Path
from PIL import Image,ImageDraw
root=Path(__file__).resolve().parents[1]
class Car(C.Structure):
 _fields_=[(k,C.c_int) for k in ['id','active','lap','split','pit']]+[(k,C.c_ulong) for k in ['start','best','best1','best2','last','s1','s2']]
class Row(C.Structure):
 _fields_=[(k,C.c_int) for k in ['id','pos','pit','outlap']]+[('best',C.c_long),('pb',C.c_long*3),('live',C.c_long*2),('color',C.c_int*2)]
class Card(C.Structure):
 _fields_=[(k,C.c_int) for k in ['id','pos','pit','outlap','post','pole','showGap','hasGap','leaderId']]+[(k,C.c_long) for k in ['running','last','gap','leaderBest']]+[('color',C.c_int*3)]
with tempfile.TemporaryDirectory() as temp:
 lib=Path(temp)/'layout.so'
 subprocess.run(['gcc','-std=c89','-Wall','-Wextra','-Werror','-Wno-missing-braces','-DF1_VIDEO_TEST','-shared','-fPIC',*[str(root/'src'/f) for f in ['f1render.c','f1timing.c','f1config.c','f1video.c']],'-o',str(lib)],check=True)
 api=C.CDLL(str(lib));api.F1QLiveTime.argtypes=[C.c_char_p,C.c_long];api.F1QTime.argtypes=[C.c_char_p,C.c_long,C.c_int]
 for ms,want in [(1,'0:00.0'),(12345,'0:12.3'),(59999,'0:59.9'),(60000,'1:00.0'),(95789,'1:35.7')]:
  s=C.create_string_buffer(40);api.F1QLiveTime(s,ms);assert s.value.decode()==want
 s=C.create_string_buffer(40);api.F1QTime(s,95789,0);assert s.value==b'1:35.789'
 rr=(Row*26)()
 for i in range(26):rr[i].id=i+1;rr[i].pos=i+1;rr[i].best=90000+i*100
 api.F1QWindow.argtypes=[C.POINTER(Row),C.c_int,C.c_int,C.POINTER(C.c_int)]
 for n in range(1,27):
  for focus in range(n):
   start=C.c_int();size=api.F1QWindow(rr,n,focus+1,C.byref(start))
   assert (start.value,size)==(max(0,focus-2),min(n,focus+3)-max(0,focus-2))
 start=C.c_int();assert not api.F1QWindow(rr,26,40,C.byref(start))
 pal=[]
 for i in range(256):pal.extend([round(((i>>5)&7)*63/7),round(((i>>2)&7)*63/7),round((i&3)*63/3)])
 palette=(C.c_ubyte*768)(*pal)
 api.F1RenderQualy.argtypes=[C.POINTER(C.c_ubyte),C.POINTER(C.c_ubyte),C.POINTER(Row),C.c_int,C.c_long,C.c_int,C.c_int,C.c_int,C.c_int]
 api.F1RenderQualyCard.argtypes=[C.POINTER(C.c_ubyte),C.POINTER(C.c_ubyte),C.POINTER(Card),C.c_int]
 q=Card();q.id=34;q.pos=13;q.running=95789;q.leaderId=16;q.leaderBest=95005;q.color[0]=3;q.color[1]=1
 for cockpit in [0,1]:
  guard=64;buf=(C.c_ubyte*(640*480+2*guard))(*([109]*(640*480+2*guard)));dst=C.cast(C.byref(buf,guard),C.POINTER(C.c_ubyte))
  api.F1RenderQualy(dst,palette,rr,26,999000,0,1,cockpit,13);api.F1RenderQualyCard(dst,palette,C.byref(q),cockpit)
  colors=(C.c_ubyte*30)(*[i%4 for i in range(30)])
  api.F1RenderMicro(dst,palette,colors,cockpit)
  assert bytes(buf[:guard])==bytes([109]*guard) and bytes(buf[-guard:])==bytes([109]*guard)
  n=5 if cockpit else 26;cy=388 if cockpit else 302
  for y in range(480):
   for x in range(640):
    if not ((8<=x<124 and 8<=y<8+36+n*13+3) or (422<=x<632 and cy<=y<cy+84)):assert dst[y*640+x]==109
  im=Image.frombytes('P',(640,480),bytes(buf[guard:-guard]));im.putpalette([v*4 for v in pal]);im=im.convert('RGB')
  ImageDraw.Draw(im).text((145,18),'Synthetic preview - not GP2 capture',fill='white')
  im.save(root/'validation'/('cockpit-layout-preview.png' if cockpit else 'tv-layout-preview.png'))
 video=(C.c_ubyte*(640*480)).in_dll(api,'host_video');vp=(C.c_ubyte*768).in_dll(api,'host_palette');vp[:]=pal
 active=C.c_ulong.in_dll(api,'activepage');clock=C.c_ulong.in_dll(api,'host_clock');bank=C.c_ulong.in_dll(api,'host_bank');gran=C.c_ushort.in_dll(api,'host_gran');failure=C.c_int.in_dll(api,'host_failure')
 api.F1QTick.argtypes=[C.POINTER(Car),C.c_int,C.c_ulong]
 car=Car(34,1,2,2,0,1000,95000,30000,61000,95000,30000,61000);clock.value=10000
 api.F1QReset();api.F1QTick(C.byref(car),1,clock.value);original=bytes([109]*(640*480))
 for g,m in [(64,0),(16,0),(64,1),(16,1)]:
  C.c_int.in_dll(api,'host_micro').value=m
  (C.c_ubyte*30).in_dll(api,'host_micro_colors')[:]=[i%4 for i in range(30)]
  video[:]=original;gran.value=g;bank.value=1;active.value=0x20;api.F1VideoReset();api.F1CockpitAfter();first=bytes(video)
  assert first!=original and bank.value==1
  api.F1CockpitBefore();assert bytes(video)==original and bank.value==1
  api.F1CockpitAfter();assert bytes(video)==first
  offset=410*640+450;changed=(video[offset]+1)%256;video[offset]=changed
  active.value=0;api.F1CockpitBefore();expect=bytearray(original);expect[offset]=changed
  assert bytes(video)==bytes(expect) and bank.value==1
  active.value=0x20;api.F1CockpitAfter();api.F1VideoReset();video[:]=original
  api.F1CockpitBefore();assert bytes(video)==original
 failure.value=1;api.F1CockpitAfter();assert bytes(video)==original
 print('PASS: precision; focused qualy windows; drawing bounds; 64K/16K banks; bank restoration; hide; no repeated darkening; native updates; view reset; unavailable video.')
