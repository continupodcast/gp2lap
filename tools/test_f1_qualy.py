"""Exercise native C timing state transitions and generate a synthetic preview."""
import ctypes as C,json,subprocess,tempfile
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
 lib=Path(temp)/'q.so'
 subprocess.run(['gcc','-std=c89','-Wall','-Wextra','-Werror','-Wno-missing-braces','-shared','-fPIC',str(root/'src/f1render.c'),str(root/'src/f1timing.c'),str(root/'src/f1config.c'),'-o',str(lib)],check=True)
 api=C.CDLL(str(lib));api.F1QTick.argtypes=[C.POINTER(Car),C.c_int,C.c_ulong]
 api.F1QRows.argtypes=[C.POINTER(C.c_int)];api.F1QRows.restype=C.POINTER(Row)
 api.F1QGetCard.argtypes=[C.c_int,C.c_ulong,C.POINTER(Card)]
 def tick(cars,clock):api.F1QTick((Car*len(cars))(*cars),len(cars),clock)
 def rows():
  n=C.c_int();r=api.F1QRows(C.byref(n));return [r[i] for i in range(n.value)]
 def card(id,clock):
  c=Card();assert api.F1QGetCard(id,clock,C.byref(c));return c
 # Bests survive an absent active car; invalid native time flags stay unavailable.
 api.F1QReset();a=Car(13,0,0,2,1,0,90000,30000,60000,0,0,0)
 b=Car(34,1,2,2,0,1000,91000,31000,61000,92000,31000,62000)
 no=Car(33,1,0,2,1,0,0xf0000000,0xf0000000,0xf0000000,0,0,0)
 tick([a,b,no],1100);assert [r.id for r in rows()]==[13,34,33];assert rows()[2].best==0
 assert card(34,1100).running==100
 # S1 improvement becomes purple; compare against leader and freeze for 8 s.
 b.split=0;b.s1=29000;tick([a,b,no],30000)
 c=card(34,30000);assert c.showGap and c.gap==-1000 and c.color[0]==3
 tick([a,b,no],39000);assert not card(34,39000).showGap
 # S2 slower than own reference: yellow (31s > 30s).
 b.split=1;b.s2=60000;tick([a,b,no],61000);assert card(34,61000).color[1]==2
 # Close a lap, new pole; freeze completed sectors, then resume running clock.
 b.lap=3;b.split=2;b.start=90000;b.last=89000;b.best=89000;b.best1=29000;b.best2=60000
 tick([a,b,no],90000);c=card(34,90000);assert c.post and c.pole and c.last==89000 and c.color[2]==3
 assert rows()[0].id==34
 tick([a,b,no],98001);assert not card(34,98001).post and card(34,98001).running==8001
 # Pit entry clears card sectors; exit remains out-lap until crossing the line.
 b.pit=1;tick([a,b,no],99000);assert card(34,99000).pit and list(card(34,99000).color)==[0,0,0]
 b.pit=0;tick([a,b,no],100000);assert card(34,100000).outlap
 b.lap=4;b.start=110000;tick([a,b,no],110000);assert not card(34,110000).outlap and not card(34,110000).post
 # No fabricated completed lap on the initial snapshot; full reset erases history.
 api.F1QReset();tick([b],110010);assert not card(34,110010).post
 api.F1QReset();assert rows()==[];c=Card();assert not api.F1QGetCard(34,0,C.byref(c))
 # Load supplied qualifying fixture as native snapshots, then renderer bounds.
 data=json.loads((root/'validation/timing_table.json').read_text());sd=json.loads((root/'validation/sector_times.json').read_text())
 cars=[]
 for r in data['table']:
  best=r['best_lap_ms'];s1=r['s1_ms'];s2=r['s2_ms']
  cars.append(Car(r['raw_id'],1,3,1,0,sd['lap_start_ms'],best,s1,s1+s2,best,r.get('live_s1_ms',0),r.get('live_s1_ms',0)+r.get('live_s2_ms',0)))
 tick(cars,sd['session_clock_ms']);rr=rows();qcard=card(sd['car_id'],sd['session_clock_ms'])
 pal=[]
 for i in range(256):pal.extend([round(((i>>5)&7)*63/7),round(((i>>2)&7)*63/7),round((i&3)*63/3)])
 palette=(C.c_ubyte*768)(*pal)
 api.F1RenderQualy.argtypes=[C.POINTER(C.c_ubyte),C.POINTER(C.c_ubyte),C.POINTER(Row),C.c_int,C.c_long,C.c_int,C.c_int,C.c_int,C.c_int]
 api.F1RenderQualyCard.argtypes=[C.POINTER(C.c_ubyte),C.POINTER(C.c_ubyte),C.POINTER(Card),C.c_int]
 original=bytes(109 if ((x//24+y//24)%2) else 146 for y in range(480) for x in range(640));guard=64
 buf=(C.c_ubyte*(640*480+guard*2))(*([173]*guard+list(original)+[173]*guard));dst=C.cast(C.byref(buf,guard),C.POINTER(C.c_ubyte))
 api.F1RenderQualy(dst,palette,(Row*len(rr))(*rr),len(rr),1200000,0,1,0,sd['car_id'])
 api.F1RenderQualyCard(dst,palette,C.byref(qcard),0)
 assert list(buf[:guard])==[173]*guard and list(buf[-guard:])==[173]*guard
 result=bytes(buf[guard:-guard])
 for y in range(480):
  for x in range(640):
   if not ((8<=x<124 and 8<=y<8+36+len(rr)*13+3) or (422<=x<632 and 302<=y<386)):assert result[y*640+x]==original[y*640+x]
 im=Image.frombytes('P',(640,480),result);im.putpalette([v*4 for v in pal]);im=im.convert('RGB')
 draw=ImageDraw.Draw(im);draw.rectangle((0,438,639,479),fill=(15,17,23));draw.text((12,450),'Qualy + sector card: native C / synthetic palette',fill='white');draw.text((12,465),'Preview only - not a game capture',fill='white');im.save(root/'validation/qualy-preview.png')
 api.F1QReset()
 field=[Car(i,0,0,2,1,0,90000+i*100,0,0,0,0,0) for i in range(1,29)]
 tick(field,1000);rr=rows();assert len(rr)==28 and rr[-1].id==28
 api.F1RenderQualy(dst,palette,(Row*28)(*rr),28,1200000,0,1,0,28)
 assert list(buf[:guard])==[173]*guard and list(buf[-guard:])==[173]*guard
 api.F1QReset();tick(field[:26],0);assert len(rows())==26
 print('PASS: invalid times, sorting, live timer, sector deltas/colors, pole, post-lap hold, pits/out-lap, reset, missing focus, renderer bounds.')
