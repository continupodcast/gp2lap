"""Compile the actual C renderer on the host; validate bounds and render a fixture.
Usage: python tools/preview_f1.py race.json focused_car.json output.png
The background/palette are synthetic, NOT a GP2 game capture.
"""
import ctypes as C,json,subprocess,sys,tempfile
from pathlib import Path
from PIL import Image,ImageDraw
root=Path(__file__).resolve().parents[1]
race=json.loads(Path(sys.argv[1]).read_text()); focus=json.loads(Path(sys.argv[2]).read_text())
class Row(C.Structure):
 _fields_=[(k,C.c_int) for k in ['id','pos','lap','out','pit','focused']]+[('gap',C.c_long),('gain',C.c_int),('stops',C.c_int),('leaderGap',C.c_long),('lapsBehind',C.c_int),('fastest',C.c_int),('positionChange',C.c_int)]
with tempfile.TemporaryDirectory() as temp:
 lib=Path(temp)/'render.so'
 subprocess.run(['gcc','-std=c89','-Wall','-Wextra','-Werror','-Wno-missing-braces','-shared','-fPIC',str(root/'src/f1render.c'),str(root/'src/f1timing.c'),str(root/'src/f1config.c'),'-o',str(lib)],check=True)
 render=C.CDLL(str(lib)).F1Render
 render.argtypes=[C.POINTER(C.c_ubyte),C.POINTER(C.c_ubyte),C.POINTER(Row),C.c_int,C.c_int,C.c_int]
 pal=[]
 for i in range(256): pal.extend([round(((i>>5)&7)*63/7),round(((i>>2)&7)*63/7),round((i&3)*63/3)])
 base=Image.new('RGB',(640,480)); draw=ImageDraw.Draw(base)
 for y in range(0,480,24):
  for x in range(0,640,24): draw.rectangle((x,y,x+23,y+23),fill=(70,100,110) if ((x+y)//24)%2 else (120,145,145))
 # A neutral grid demonstrates actual background-dependent palette blending.
 palette_image=Image.new('P',(1,1)); palette_image.putpalette([v*4 for v in pal])
 base=base.quantize(palette=palette_image,dither=Image.Dither.NONE)
 original=bytes(base.tobytes()); guard=64
 storage=(C.c_ubyte*(640*480+2*guard))(*([173]*guard+list(original)+[173]*guard))
 screen=C.cast(C.byref(storage,guard),C.POINTER(C.c_ubyte)); palette=(C.c_ubyte*768)(*pal)
 focused=int(focus.get('car_id',0))&63
 rows=(Row*26)()
 for i,d in enumerate(race['cars']):
  rows[i]=Row(d['car_id'],d['pos'],d['lap'],d['is_out'],d['in_pits'],d['car_id']==focused,d['gap_ms'] if d['gap_str'] else -1)
 render(screen,palette,rows,len(race['cars']),race['leader_lap'],race['total_laps'])
 assert bytes(storage[:guard])==bytes([173]*guard) and bytes(storage[-guard:])==bytes([173]*guard)
 result=bytes(storage[guard:-guard]); bottom=8+34+len(race['cars'])*13+3
 for y in range(480):
  for x in range(640):
   if not (8<=x<112 and 8<=y<bottom): assert result[y*640+x]==original[y*640+x],(x,y)
 assert result!=original
 # Empty table must perform no writes.
 before=bytes(storage); render(screen,palette,rows,0,0,0); assert bytes(storage)==before
 im=Image.frombytes('P',(640,480),result); im.putpalette([v*4 for v in pal]); im=im.convert('RGB')
 draw=ImageDraw.Draw(im); draw.rectangle((0,442,639,479),fill=(15,17,23));draw.text((170,450),'PREVIEW: native C renderer / synthetic palette',fill='white'); draw.text((170,464),'Not a game capture. In-game validation pending.',fill='white')
 im.save(sys.argv[3])
 print('PASS: 26-row fixture; buffer guards; writes confined to panel; empty table no-op.')
 print('Fixture: lap',race['leader_lap'],'/',race['total_laps'],'focused ID',focused)
