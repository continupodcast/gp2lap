import ctypes as C, subprocess, tempfile, math, json
from pathlib import Path
from PIL import Image
root=Path(__file__).resolve().parents[1]
class Point(C.Structure):_fields_=[('x',C.c_double),('y',C.c_double)]
class Car(C.Structure):_fields_=[('id',C.c_int),('pos',C.c_int),('out',C.c_int),('x',C.c_double),('y',C.c_double),('angle',C.c_double)]
with tempfile.TemporaryDirectory() as tmp:
 lib=Path(tmp)/'map.so'
 subprocess.run(['gcc','-std=c89','-Wall','-Wextra','-Werror','-Wno-missing-braces','-shared','-fPIC',str(root/'src/f1render.c'),str(root/'src/f1timing.c'),str(root/'src/f1config.c'),'-lm','-o',str(lib)],check=True)
 api=C.CDLL(str(lib));api.F1RenderMap.argtypes=[C.POINTER(C.c_ubyte),C.POINTER(C.c_ubyte),C.POINTER(Point),C.c_int,C.POINTER(Point),C.c_int,C.POINTER(Car),C.c_int,C.c_int]
 pal=[]
 for i in range(256):pal.extend([round(((i>>5)&7)*63/7),round(((i>>2)&7)*63/7),round((i&3)*63/3)])
 palette=(C.c_ubyte*768)(*pal)
 track=(Point*6)(Point(0,-2000),Point(0,2000),Point(1000,2500),Point(3000,2000),Point(3000,-2000),Point(1000,-2500))
 pits=(Point*3)(Point(320,-1000),Point(400,0),Point(320,1000))
 data=json.loads((root/'f1-input/drivers.json').read_text());ids=[d['car_id'] for d in data['drivers']]
 cars=(Car*5)(*[Car(id,i+1,0,(-60 if i%2 else 60), (i-2)*300,0) for i,id in enumerate(ids[:5])]);focus=ids[2]
 def render():
  guard=64;buf=(C.c_ubyte*(640*480+128))(*([109]*(640*480+128)));screen=C.cast(C.byref(buf,64),C.POINTER(C.c_ubyte))
  api.F1RenderMap(screen,palette,track,6,pits,3,cars,5,focus)
  assert bytes(buf[:64])==bytes([109])*64 and bytes(buf[-64:])==bytes([109])*64
  result=bytes(buf[64:-64])
  for y in range(480):
   for x in range(640):
    if (x-562)**2+(y-78)**2>68**2:assert result[y*640+x]==109
  return result
 images=[]
 for angle in [0,math.pi/2,math.pi,-math.pi/2]:
  for car in cars:car.angle=angle
  result=render();assert result!=bytes([109])*(640*480)
  im=Image.frombytes('P',(640,480),result);im.putpalette([v*4 for v in pal]);images.append(im.convert('RGB').crop((488,4,636,152)))
 # A retired rival disappears; no focus means no writes.
 focus=ids[1];moved=render();focus=ids[2];assert render()!=moved
 before=render();cars[0].out=1;assert render()!=before
 focus=40;assert render()==bytes([109])*(640*480)
 preview=Image.new('RGB',(296,296))
 for i,im in enumerate(images):preview.paste(im,((i%2)*148,(i//2)*148))
 preview.save(root/'validation/map-0.9-preview.png')
 # Validate tiny Aston and Mercedes assets retain visible strokes.
 from PIL import ImageOps
 for key in ['astonmartin','mercedes']:
  im=Image.open(root/'f1-input/logos'/data['teams'][key]['logo']).convert('RGBA');im=im.crop(im.getchannel('A').getbbox());im=ImageOps.contain(im,(16,12),Image.Resampling.LANCZOS)
  assert sum(min(255,a*3)>=128 for a in im.getchannel('A').getdata())>=10
print('PASS: map rotations, circular bounds/guards, moving focus, retirement, missing focus, Aston/Mercedes alpha coverage.')
