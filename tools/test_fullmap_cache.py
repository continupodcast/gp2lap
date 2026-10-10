import ctypes as C, math, subprocess, tempfile, time
from pathlib import Path
root=Path(__file__).resolve().parents[1]
class Point(C.Structure):_fields_=[('x',C.c_double),('y',C.c_double)]
class Car(C.Structure):_fields_=[('id',C.c_int),('pos',C.c_int),('out',C.c_int),('x',C.c_double),('y',C.c_double),('angle',C.c_double)]
with tempfile.TemporaryDirectory() as td:
 apis=[]
 for name,src in [('old',root/'tools/fixtures/f1render-before-map-cache.c'),('new',root/'src/f1render.c')]:
  lib=Path(td)/(name+'.so')
  subprocess.run(['gcc','-O2','-std=c89','-Wall','-Wextra','-Werror','-Wno-missing-braces','-Wno-format-overflow','-shared','-fPIC','-I'+str(root/'src'),str(src),str(root/'src/f1config.c'),str(root/'src/f1timing.c'),'-lm','-o',str(lib)],check=True)
  api=C.CDLL(str(lib));api.F1RenderFullMap.argtypes=[C.POINTER(C.c_ubyte),C.POINTER(C.c_ubyte),C.POINTER(Point),C.c_int,C.POINTER(Point),C.c_int,C.POINTER(Car),C.c_int,C.c_int];apis.append(api)
 pal=(C.c_ubyte*768)(*[v for i in range(256) for v in (i%64,(i//4)%64,(i//16)%64)])
 track=(Point*512)(*[Point(3000*math.cos(i*math.tau/512),2000*math.sin(i*math.tau/512)) for i in range(512)])
 pits=(Point*16)(*[Point(3200,2000-i*250) for i in range(16)])
 cars=(Car*26)(*[Car(i+1,i+1,0,2800*math.cos(i*math.tau/26),1800*math.sin(i*math.tau/26),0) for i in range(26)])
 def render(api,bg=73,focus=3):
  buf=(C.c_ubyte*(640*480+128))(*([bg]*(640*480+128)))
  dst=C.cast(C.byref(buf,64),C.POINTER(C.c_ubyte))
  api.F1RenderFullMap(dst,pal,track,512,pits,16,cars,26,focus)
  assert bytes(buf[:64])==bytes([bg])*64 and bytes(buf[-64:])==bytes([bg])*64
  out=bytes(buf[64:-64])
  for y in range(480):
   assert out[y*640:y*640+492]==bytes([bg])*492
   assert out[y*640+632:(y+1)*640]==bytes([bg])*8
   if not 8<=y<148:assert out[y*640:(y+1)*640]==bytes([bg])*640
  return out
 for step in range(12):
  cars[2].x+=31;cars[4].out=step%2
  if step==3:track[4].x+=400 # same pointer/count but changed geometry
  if step==5:pits[7].y-=300
  if step==7:pal[100]=62 # palette invalidation
  assert render(apis[0],step+60)==render(apis[1],step+60)
 for zoom in range(3):
  for api in apis:api.F1CycleMapZoom()
  for step in range(3):
   cars[2].y+=100
   old=render(apis[0],81+step);new=render(apis[1],81+step)
   diff=sum(a!=b for a,b in zip(old,new))
   if zoom==2:assert diff==0
   else:assert diff<2000
 # C-render cost only: buffer allocation, Python loop and palette warmup excluded.
 dst=(C.c_ubyte*(640*480))();loops=400;timings=[]
 for api in apis:
  api.F1RenderFullMap(dst,pal,track,512,pits,16,cars,26,3)
  t=time.perf_counter()
  for _ in range(loops):api.F1RenderFullMap(dst,pal,track,512,pits,16,cars,26,3)
  timings.append((time.perf_counter()-t)/loops)
 print('PASS: identical cached 1x; geometry/palette invalidation; mobile zoom edge differences bounded; screen guards.')
 print('Linux host C renderer benchmark (not DOS/RetroArch occupancy): old %.3f ms/frame, cache %.3f ms/frame, %.1fx faster.'%(timings[0]*1000,timings[1]*1000,timings[0]/timings[1]))

 for zoom in (1,2):
  for api in apis:api.F1CycleMapZoom()
  timings=[]
  for api in apis:
   t=time.perf_counter()
   for _ in range(loops):api.F1RenderFullMap(dst,pal,track,512,pits,16,cars,26,3)
   timings.append((time.perf_counter()-t)/loops)
  print('Mobile zoom %d host: old %.3f ms/frame, integer %.3f ms/frame, %.1fx faster.'%(zoom,timings[0]*1000,timings[1]*1000,timings[0]/timings[1]))
 for api in apis:api.F1RenderMap.argtypes=api.F1RenderFullMap.argtypes
 for step in range(8):
  cars[2].angle=step*0.6;cars[2].x=step*150;cars[2].y=step*100
  bg=74;buf=(C.c_ubyte*(640*480+128))(*([bg]*(640*480+128)))
  screen=C.cast(C.byref(buf,64),C.POINTER(C.c_ubyte))
  apis[1].F1RenderMap(screen,pal,track,512,pits,16,cars,26,3)
  assert bytes(buf[:64])==bytes([bg])*64 and bytes(buf[-64:])==bytes([bg])*64
  for y in range(480):
   for x in range(640):
    if not (492<=x<632 and 8<=y<148) or (x-562)**2+(y-78)**2>68**2:assert buf[64+y*640+x]==bg
 timings=[]
 for api in apis:
  api.F1RenderMap(dst,pal,track,512,pits,16,cars,26,3)
  t=time.perf_counter()
  for _ in range(loops):api.F1RenderMap(dst,pal,track,512,pits,16,cars,26,3)
  timings.append((time.perf_counter()-t)/loops)
 print('Local map host: old %.3f ms/frame, integer %.3f ms/frame, %.1fx faster.'%(timings[0]*1000,timings[1]*1000,timings[0]/timings[1]))
 print('PASS: rotating/moving local map preserves circular clipping and screen guards.')
