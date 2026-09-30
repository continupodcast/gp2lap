import ctypes as C
import math, subprocess, tempfile, zipfile
from pathlib import Path
from PIL import Image
root=Path(__file__).resolve().parents[1]
class Point(C.Structure): _fields_=[('x',C.c_double),('y',C.c_double)]
class Car(C.Structure): _fields_=[('id',C.c_int),('pos',C.c_int),('out',C.c_int),('x',C.c_double),('y',C.c_double),('angle',C.c_double)]
with tempfile.TemporaryDirectory() as tmp:
 tmp=Path(tmp);lib=tmp/'map.so'
 subprocess.run(['gcc','-std=c89','-Wall','-Wextra','-Werror','-Wno-missing-braces','-shared','-fPIC',str(root/'src/f1render.c'),str(root/'src/f1timing.c'),str(root/'src/f1config.c'),'-lm','-o',str(lib)],check=True)
 api=C.CDLL(str(lib));api.F1RenderFullMap.argtypes=[C.POINTER(C.c_ubyte),C.POINTER(C.c_ubyte),C.POINTER(Point),C.c_int,C.POINTER(Point),C.c_int,C.POINTER(Car),C.c_int,C.c_int]
 api.F1ConfigLoad.argtypes=[C.c_char_p]
 with zipfile.ZipFile(root/'validation/editor-013-export.zip') as z:
  assert z.testzip() is None;z.extractall(tmp)
 assert api.F1ConfigLoad(str(tmp/'GP2LAP.CFG').encode())==1
 assert (C.c_ubyte*13).in_dll(api,'f1_custom_logo_loaded')[0]==1
 # Root-level legacy paths still load; missing sprites fall back to colour.
 cfg=tmp/'GP2LAP.CFG';original=cfg.read_text();sprite=tmp/'SEASONS/LOGOS/2026/TEAM01.F1L'
 (tmp/'TEAM01.F1L').write_bytes(sprite.read_bytes());cfg.write_text(original.replace('SEASONS/LOGOS/2026/TEAM01.F1L','TEAM01.F1L'))
 assert api.F1ConfigLoad(str(cfg).encode())==1 and (C.c_ubyte*13).in_dll(api,'f1_custom_logo_loaded')[0]==1
 (tmp/'TEAM01.F1L').unlink();api.F1ConfigLoad(str(cfg).encode());assert (C.c_ubyte*13).in_dll(api,'f1_team_logo_enabled')[0]==0
 points=[(0,-2000),(0,2000),(1000,2500),(3000,2000),(3000,-2000),(1000,-2500)]
 track=(Point*6)(*[Point(*p) for p in points]);cars=(Car*26)(*[Car(i+10,i+1,0,0,-1900+i*145,0) for i in range(26)])
 pal=[]
 for i in range(256):pal.extend([round(((i>>5)&7)*63/7),round(((i>>2)&7)*63/7),round((i&3)*63/3)])
 palette=(C.c_ubyte*768)(*pal)
 def render(focus=12):
  buf=(C.c_ubyte*(640*480+128))(*([109]*(640*480+128)));screen=C.cast(C.byref(buf,64),C.POINTER(C.c_ubyte))
  api.F1RenderFullMap(screen,palette,track,6,None,0,cars,26,focus)
  assert bytes(buf[:64])==bytes([109])*64 and bytes(buf[-64:])==bytes([109])*64
  result=bytes(buf[64:-64])
  for y in range(480):
   assert result[y*640:y*640+492]==bytes([109])*492
   assert result[y*640+632:(y+1)*640]==bytes([109])*8
   if not 8<=y<148:assert result[y*640:(y+1)*640]==bytes([109])*640
  return result
 before=render();assert before!=bytes([109])*(640*480)
 for car in cars:car.angle=math.pi
 assert render()==before # full map does not rotate with the camera
 cars[0].out=1;assert render()!=before
 im=Image.frombytes('P',(640,480),before);im.putpalette([v*4 for v in pal]);im.crop((492,8,632,148)).resize((560,560),Image.Resampling.NEAREST).save(root/'validation/map-013.png')
 # Isolate the selected car in the centre, away from the track line.
 for c in cars:c.out=1
 cars[2].out=0;cars[2].x=1500;cars[2].y=0
 plain=render(0);highlighted=render(12);cx,cy=562,78
 assert highlighted[cy*640+cx]==plain[cy*640+cx]
 assert highlighted[cy*640+cx+3]==255 and plain[cy*640+cx+3]!=255
 assert highlighted[(cy+3)*640+cx]==255
print('PASS: full map guards, orientation, retired cars, nested and legacy logos, missing-logo fallback.')
