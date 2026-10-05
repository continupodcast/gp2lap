"""Exercise the real renderer: panel bounds, millisecond gaps and zoom clipping."""
import ctypes as C
import json
import math
import subprocess
import tempfile
from pathlib import Path
from PIL import Image

root = Path(__file__).resolve().parents[1]
class Row(C.Structure):
    _fields_ = [(k,C.c_int) for k in ['id','pos','lap','out','pit','focused']] + [('gap',C.c_long),('gain',C.c_int),('stops',C.c_int),('leaderGap',C.c_long),('lapsBehind',C.c_int),('fastest',C.c_int),('positionChange',C.c_int)]
class Point(C.Structure):
    _fields_ = [('x',C.c_double),('y',C.c_double)]
class Car(C.Structure):
    _fields_ = [('id',C.c_int),('pos',C.c_int),('out',C.c_int),('x',C.c_double),('y',C.c_double),('angle',C.c_double)]

ids = [d['car_id'] for d in json.loads((root/'f1-input/drivers.json').read_text())['drivers']]
colors = [round(((i>>5)&7)*63/7) if c==0 else round(((i>>2)&7)*63/7) if c==1 else round((i&3)*63/3) for i in range(256) for c in range(3)]
pal = (C.c_ubyte*768)(*colors)
with tempfile.TemporaryDirectory() as tmp:
    lib = Path(tmp)/'r.so'
    subprocess.run(['gcc','-std=c89','-Wall','-Wextra','-Werror','-Wno-missing-braces','-shared','-fPIC',str(root/'src/f1render.c'),str(root/'src/f1timing.c'),str(root/'src/f1config.c'),'-lm','-o',str(lib)],check=True)
    api = C.CDLL(str(lib))
    rows = (Row*26)()
    for i,r in enumerate(rows): r.id=ids[i];r.pos=i+1;r.gap=1234;r.lap=3
    def draw(fn,rect,*args):
        buf = (C.c_ubyte*(640*480+128))(*([109]*(640*480+128)))
        screen = C.cast(C.byref(buf,64),C.POINTER(C.c_ubyte))
        fn(screen,pal,*args)
        assert bytes(buf[:64])==bytes([109])*64 and bytes(buf[-64:])==bytes([109])*64
        data=bytes(buf[64:-64]);x,y,w,h=rect
        for yy in range(480):
            row=data[yy*640:(yy+1)*640]
            assert all(p==109 for p in (row if not y<=yy<y+h else row[:x]+row[x+w:])), (fn.__name__,yy)
        return data
    shots=[]
    for i in range(26):
        data=draw(api.F1RenderCurrentGap,(120,340,400,40),rows,26,ids[i])
        assert data != bytes([109])*(640*480)
        if i in (1,7,13,15):shots.append((data,(120,340,520,380)))
        data=draw(api.F1RenderModernFastest,(140,50,360,26),ids[i],C.c_long(81505),50)
        if i in (1,7,13,15):shots.append((data,(140,50,500,76)))
    first=draw(api.F1RenderCurrentGap,(120,340,400,40),rows,26,ids[1])
    rows[1].gap=1235
    assert draw(api.F1RenderCurrentGap,(120,340,400,40),rows,26,ids[1])!=first, 'milliseconds lost'
    for attr,value in [('gap',-1),('pit',1),('out',1)]:
        old=getattr(rows[1],attr);setattr(rows[1],attr,value)
        assert draw(api.F1RenderCurrentGap,(120,340,400,40),rows,26,ids[1])==bytes([109])*(640*480)
        setattr(rows[1],attr,old)
    track=(Point*60)(*[Point(2000*math.cos(i*math.pi/30),1500*math.sin(i*math.pi/30)) for i in range(60)])
    cars=(Car*3)(Car(ids[0],1,0,1800,0,0),Car(ids[1],2,0,1600,500,0),Car(ids[2],3,0,1800,-700,0))
    for fn in (api.F1RenderFullMap,api.F1RenderMap):
        images=[]
        for zoom in range(4):
            data=draw(fn,(492,8,140,140),track,60,None,0,cars,3,ids[0]);images.append(data)
            if zoom<3:api.F1CycleMapZoom()
        assert images[0]==images[3]
        assert len(set(images[:3]))==(3 if fn==api.F1RenderFullMap else 1)
    canvas=Image.new('RGB',(800,len(shots)*88),(24,24,24))
    for i,(data,crop) in enumerate(shots):
        im=Image.frombytes('P',(640,480),data);im.putpalette([v*4 for v in colors]);im=im.convert('RGB').crop(crop)
        canvas.paste(im.resize((im.width*2,im.height*2),Image.Resampling.NEAREST),(0,i*88))
    canvas.save(root/'validation/modern-034.png')
print('PASS: 52 modern panels, guarded bounds, true millisecond gap, unavailable/pit/out handling; both maps at 3 zoom levels and zoom wraparound.')
