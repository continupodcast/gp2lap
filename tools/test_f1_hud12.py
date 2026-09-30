"""Verify badge placement, removal and independent leader-gap rendering."""
import ctypes as C
import subprocess
import tempfile
from pathlib import Path
from PIL import Image

root=Path(__file__).resolve().parents[1]
class Row(C.Structure):
    _fields_=[(k,C.c_int) for k in ['id','pos','lap','out','pit','focused']]+[('gap',C.c_long),('gain',C.c_int),('stops',C.c_int),('leaderGap',C.c_long),('lapsBehind',C.c_int),('fastest',C.c_int),('positionChange',C.c_int)]
class QRow(C.Structure):
    _fields_=[(k,C.c_int) for k in ['id','pos','pit','outlap']]+[('best',C.c_long),('pb',C.c_long*3),('live',C.c_long*2),('color',C.c_int*2)]
with tempfile.TemporaryDirectory() as tmp:
    lib=Path(tmp)/'hud.so'
    subprocess.run(['gcc','-std=c89','-Wall','-Wextra','-Werror','-Wno-missing-braces','-shared','-fPIC',*[str(root/'src'/f) for f in ['f1render.c','f1timing.c','f1config.c']],'-lm','-o',str(lib)],check=True)
    api=C.CDLL(str(lib));api.F1ConfigLoad(None)
    pal=[v for i in range(256) for v in (round(((i>>5)&7)*63/7),round(((i>>2)&7)*63/7),round((i&3)*63/3))]
    palette=(C.c_ubyte*768)(*pal)
    def frame(): return (C.c_ubyte*(640*480))(*([109]*(640*480)))
    def region(b,x,y,w,h): return bytes(b[yy*640+xx] for yy in range(y,y+h) for xx in range(x,x+w))
    rows=(Row*3)(Row(36,1,4,0,0,0,0,0,0,0,0,0),Row(11,2,4,0,0,0,1000,1,0,5000,0,1),Row(28,3,3,0,0,0,2000,-1,1,95000,1,0))
    a=frame();api.F1RenderRace(a,palette,rows,3,4,42,3)
    assert len(set(region(a,113,55,11,11)))==2
    assert region(a,113,42,11,11)==bytes([109])*121
    rows[1].fastest=0;b=frame();api.F1RenderRace(b,palette,rows,3,4,42,3)
    assert region(b,113,55,11,11)==bytes([109])*121
    assert region(a,8,8,104,76)==region(b,8,8,104,76)
    rows[1].leaderGap=12000;c=frame();api.F1RenderRace(c,palette,rows,3,4,42,3)
    assert region(b,72,55,40,13)!=region(c,72,55,40,13)
    qr=(QRow*2)(QRow(36,1,1,0,90000),QRow(11,2,0,1,91000))
    q=frame();api.F1RenderQualy(q,palette,qr,2,300000,0,1,0,36)
    assert len(set(region(q,125,44,11,11)))==2
    assert region(q,125,57,11,11)==bytes([109])*121
    qr[0].pit=0;q2=frame();api.F1RenderQualy(q2,palette,qr,2,300000,1,1,0,36)
    assert region(q2,125,44,11,11)==bytes([109])*121
    for data,name in [(a,'race-012.png'),(q,'qualy-012.png')]:
        im=Image.frombytes('P',(640,480),bytes(data));im.putpalette([v*4 for v in pal]);im.convert('RGB').save(root/'validation'/name)
print('PASS: fastest-lap badge, unchanged tower width, independent leader gap, P badge and removal, out-lap without P.')
