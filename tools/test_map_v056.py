from pathlib import Path
import ctypes as C, subprocess, tempfile
root=Path(__file__).resolve().parents[1]
class Point(C.Structure): _fields_=[('x',C.c_double),('y',C.c_double)]
class Car(C.Structure): _fields_=[('id',C.c_int),('pos',C.c_int),('out',C.c_int),('x',C.c_double),('y',C.c_double),('angle',C.c_double)]
with tempfile.TemporaryDirectory() as tmp:
 lib=Path(tmp)/'render.so'
 subprocess.run(['gcc','-std=c89','-Wall','-Wextra','-Werror','-Wno-missing-braces','-shared','-fPIC',*[str(root/'src'/n) for n in ['f1render.c','f1timing.c','f1config.c']],'-o',str(lib)],check=True)
 api=C.CDLL(str(lib));api.F1ConfigLoad(str(root.parent/'GP2LAP.CFG').encode())
 pal=(C.c_ubyte*768)(*[v for i in range(256) for v in (round(((i>>5)&7)*63/7),round(((i>>2)&7)*63/7),round((i&3)*63/3))])
 track=(Point*4)(Point(-50,-50),Point(50,-50),Point(50,50),Point(-50,50))
 cars=(Car*1)(Car(26,1,0,0,-50,0))
 for zoom in range(3):
  results=[]
  for repeat in range(2):
   buf=(C.c_ubyte*(640*480+128))(*([173]*64+[109]*(640*480)+[173]*64))
   api.F1RenderFullMap(C.cast(C.byref(buf,64),C.POINTER(C.c_ubyte)),pal,track,4,None,0,cars,1,26)
   assert bytes(buf[:64])==bytes([173])*64 and bytes(buf[-64:])==bytes([173])*64
   data=bytes(buf[64:-64]);center=78*640+562
   center=(22 if zoom==0 else 78)*640+562
   assert data[center]!=0, 'Selected car must retain team colour'
   assert data[center+2]==data[center], 'Selected marker must be a solid enlarged team-colour dot'
   assert all(492<=i%640<632 and 8<=i//640<148 for i,v in enumerate(data) if v!=109)
   results.append(data)
  assert results[0]==results[1], 'Cached and fresh map must agree'
  api.F1CycleMapZoom()
 print('PASS: three zooms; solid enlarged selected dot, bounds and cache consistency.')

 # Leader must cover an overlapping selected backmarker, independent of array order.
 cars2=(Car*2)(Car(26,2,0,0,-50,0),Car(34,1,0,0,-50,0))
 def render(cs):
  buf=(C.c_ubyte*(640*480))(*([109]*(640*480)))
  api.F1RenderFullMap(buf,pal,track,4,None,0,cs,len(cs),26)
  return bytes(buf)
 a=render(cars2);b=render((Car*2)(cars2[1],cars2[0]))
 assert a==b, 'Position layering must not depend on car array order'
 center=22*640+562
 assert a[center]!=a[center+3], 'Leader covers the selected backmarker at the overlap centre'
 # Lateral car motion must not move the selected dot away from track centreline.
 assert render((Car*1)(Car(26,1,0,0,-48,0)))==render((Car*1)(Car(26,1,0,0,-52,0)))
 print('PASS: leader layering and selected marker centreline stability.')
