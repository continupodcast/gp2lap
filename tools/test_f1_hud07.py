"""Host checks of the actual C race state and renderer; no emulator required."""
import ctypes as C
import json
import subprocess
import tempfile
from pathlib import Path
from PIL import Image

root = Path(__file__).resolve().parents[1]
class Row(C.Structure):
    _fields_ = [(k, C.c_int) for k in ['id','pos','lap','out','pit','focused']] + [
        ('gap',C.c_long),('gain',C.c_int),('stops',C.c_int),('leaderGap',C.c_long),('lapsBehind',C.c_int),('fastest',C.c_int),('positionChange',C.c_int)]

with tempfile.TemporaryDirectory() as temp:
    lib = Path(temp)/'hud.so'
    subprocess.run(['gcc','-std=c89','-Wall','-Wextra','-Werror','-Wno-missing-braces',
        '-shared','-fPIC',*[str(root/'src'/f) for f in ['f1render.c','f1timing.c','f1config.c','f1race.c']],
        '-o',str(lib)],check=True)
    api=C.CDLL(str(lib))
    api.F1RaceStats.argtypes=[C.POINTER(Row),C.c_int]
    rows=(Row*26)(*[Row(i+1,i+1,0,0,0,0,1234,1000,i%4) for i in range(26)])
    api.F1RaceReset(1);api.F1RaceStats(rows,26)
    assert all(r.gain==0 for r in rows)
    rows[0].pos=5;rows[4].pos=1
    api.F1RaceStats(rows,26)
    assert rows[0].gain==-4 and rows[4].gain==4
    # Baseline must not follow later overtakes.
    rows[4].pos=2;api.F1RaceStats(rows,26);assert rows[4].gain==3
    api.F1RaceReset(0);api.F1RaceStats(rows,26)
    assert all(r.gain==1000 for r in rows)  # Loaded game, grid unavailable.
    for r in rows:r.lap=6
    api.F1RaceReset(1);api.F1RaceStats(rows,26)
    assert all(r.gain==1000 for r in rows)  # Never invent a mid-race grid.
    for r in rows:r.lap=0
    for i,r in enumerate(rows):r.pos=i+1
    rows[1].id=1
    api.F1RaceReset(1);api.F1RaceStats(rows,26)
    assert all(r.gain==1000 for r in rows)  # Duplicate IDs invalidate capture.
    rows[1].id=2;api.F1RaceStats(rows,26)
    assert all(r.gain==0 for r in rows)
    # Disabled TAB, press vs repeat/release, and wraparound.
    api.F1RaceKey(15,0);assert api.F1RaceMode()==0
    api.F1RaceKey(143,0)
    for expected in [1,2,3,0,1]:
        api.F1RaceKey(15,1);assert api.F1RaceMode()==expected
        api.F1RaceKey(15,1);assert api.F1RaceMode()==expected
        api.F1RaceKey(143,0);assert api.F1RaceMode()==expected
    pal=[]
    for i in range(256):pal.extend([round(((i>>5)&7)*63/7),round(((i>>2)&7)*63/7),round((i&3)*63/3)])
    palette=(C.c_ubyte*768)(*pal)
    def frame(): return (C.c_ubyte*(640*480))(*([109]*(640*480)))
    api.F1RenderRace.argtypes=[C.POINTER(C.c_ubyte),C.POINTER(C.c_ubyte),C.POINTER(Row),*([C.c_int]*4)]
    api.F1RenderDriver.argtypes=[C.POINTER(C.c_ubyte),C.POINTER(C.c_ubyte),C.c_int,C.c_int]
    def bounds(buf,rect):
        x0,y0,x1,y1=rect
        for y in range(480):
            for x in range(640):
                if not (x0<=x<x1 and y0<=y<y1):assert buf[y*640+x]==109,(x,y)
    for mode in range(4):
        for n in [3,5,26]:
            buf=frame();api.F1RenderRace(buf,palette,rows,n,3,42,mode)
            bounds(buf,(8,8,112,8+34+n*13+3))
    data=json.loads((root/'f1-input/drivers.json').read_text())
    for d in data['drivers']:
        for pos in [0,26]:
            buf=frame();api.F1RenderDriver(buf,palette,d['car_id'],pos)
            bounds(buf,(220,8,420,46));assert bytes(buf)!=bytes([109])*(640*480)
    buf=frame();api.F1RenderDriver(buf,palette,0,0);assert bytes(buf)==bytes([109])*(640*480)
    # Mercedes must be visibly drawn rather than the old two isolated pixels.
    mercedes=next(d for d in data['drivers'] if d['team']=='mercedes')
    rows[0].id=mercedes['car_id'];rows[0].pos=1
    buf=frame();api.F1RenderRace(buf,palette,rows,1,3,42,0)
    visible=sum(buf[y*640+x]!=buf[y*640+10] for y in range(42,54) for x in range(26,42))
    assert visible>=20,visible
    # Review the real renderer with synthetic telemetry/palette.
    col=next(d for d in data['drivers'] if d['abbr']=='COL')
    ids=[mercedes['car_id'],13,col['car_id'],23,35]
    fixture=(Row*5)(*[Row(id,7+i,3,0,0,i==2,1234,[-2,1,4,0,-1][i],i%3) for i,id in enumerate(ids)])
    frames=[]
    for mode in range(4):
        buf=frame();api.F1RenderRace(buf,palette,fixture,5,3,42,mode)
        api.F1RenderDriver(buf,palette,col['car_id'],9)
        im=Image.frombytes('P',(640,480),bytes(buf));im.putpalette([v*4 for v in pal])
        frames.append(im.convert('RGB').crop((0,0,480,125)))
    preview=Image.new('RGB',(480,125*3))
    for i,im in enumerate(frames):preview.paste(im,(0,i*125))
    preview.save(root/'validation/hud-0.7-preview.png')
print('PASS: race grid baseline/reset/load; gains; TAB disabled/repeat/release/cycle; 3/5/26-row bounds; all driver cards; Mercedes visibility.')
