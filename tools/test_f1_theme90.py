"""90s theme: bounds of every element and HudTheme CFG parsing (host build of the real renderer)."""
import ctypes as C, json, subprocess, tempfile, os
from pathlib import Path
root=Path(__file__).resolve().parents[1]
ids=[d['car_id'] for d in json.loads((root/'f1-input/drivers.json').read_text())['drivers']]
class Row(C.Structure):
    _fields_=[(k,C.c_int) for k in ['id','pos','lap','out','pit','focused']]+[('gap',C.c_long),('gain',C.c_int),('stops',C.c_int),('leaderGap',C.c_long),('lapsBehind',C.c_int),('fastest',C.c_int),('positionChange',C.c_int)]
class QRow(C.Structure):
    _fields_=[(k,C.c_int) for k in ['id','pos','pit','outlap']]+[('best',C.c_long),('pb',C.c_long*3),('live',C.c_long*2),('color',C.c_int*2)]
with tempfile.TemporaryDirectory() as t:
    lib=Path(t)/'r.so'
    subprocess.run(['gcc','-std=c89','-Wall','-Wextra','-Werror','-Wno-missing-braces','-shared','-fPIC',str(root/'src/f1render.c'),str(root/'src/f1timing.c'),str(root/'src/f1config.c'),'-o',str(lib)],check=True)
    L=C.CDLL(str(lib)); L.F1ConfigLoad.argtypes=[C.c_char_p]
    theme=C.c_int.in_dll(L,'f1_theme')
    # CFG parsing
    cfg=Path(t)/'a.cfg'
    for text,want in [('[F1 Controls]\nHudTheme = 1\n',1),('[F1 Controls]\nHudTheme = 0\n',0),('[F1 Controls]\nHudTheme = 5\n',0),('[F1 HUD]\nF1UseCustomData = 0\n',0)]:
        cfg.write_text(text); L.F1ConfigLoad(str(cfg).encode()); assert theme.value==want,(text,theme.value)
    theme.value=1
    pal=(C.c_ubyte*768)(*[(i*37+j*11)%64 for i in range(256) for j in range(3)])
    SCR=C.c_ubyte*(640*480)
    def changed(buf):
        return [(i%640,i//640) for i in range(640*480) if buf[i]!=7]
    def check(buf,rects,label):
        for x,y in changed(buf):
            assert any(rx<=x<rx+rw and ry<=y<ry+rh for rx,ry,rw,rh in rects),(label,x,y)
    tests=0
    for n in (1,2,3,5,26):
        rows=(Row*n)()
        for i in range(n):
            r=rows[i];r.id=ids[i];r.pos=i+1;r.lap=5;r.gap=1234 if i else 0;r.gain=(i%3)-1;r.stops=i%2
            r.out=i==n-1 and n>3;r.pit=i==1;r.positionChange=(1,-1,0)[i%3];r.leaderGap=i*900
        for mode in range(4):
            buf=SCR(*([7]*(640*480)));L.F1RenderRace(buf,pal,rows,n,5,43,mode)
            check(buf,[(8,8,132+12,24+n*12+3)],'tower'); tests+=1
        for f in range(n):
            buf=SCR(*([7]*(640*480)));L.F1Render90Difference(buf,pal,rows,n,ids[f])
            check(buf,[(160,350,320,30)],'difference'); tests+=1
    # TV view: GP2 displays only lines 0..387, every TV element must end above them
    assert L.F1DriverTop(0,0,26)+38<=388 and L.F1DriverTop(0,1,26)+38<=388
    for cockpit in (0,1):
        for pos in (0,1,9,26):
            buf=SCR(*([7]*(640*480)));x=L.F1DriverLeft(cockpit);y=L.F1DriverTop(cockpit,0,26)
            L.F1RenderDriverAtXY(buf,pal,ids[3],pos,x,y);check(buf,[(x,y,200,38)],'plate'); tests+=1
    for y in (8,50):
        buf=SCR(*([7]*(640*480)));L.F1Render90Fastest(buf,pal,ids[0],C.c_long(82207),y)
        check(buf,[(222,y,196,30)],'fastest'); tests+=1
    q=(QRow*26)()
    for i in range(26): q[i].id=ids[i];q[i].pos=i+1;q[i].best=0 if i>22 else 80000+i*150;q[i].pit=i==4
    for compact in (0,1):
        buf=SCR(*([7]*(640*480)));L.F1RenderQualy(buf,pal,q,26,C.c_long(90000),0,1,compact,ids[10])
        rows_n=5 if compact else 26
        check(buf,[(8,8,140+12,24+rows_n*12+3)],'qualy'); tests+=1
print('PASS: HudTheme CFG parsing; TV elements inside the 388 visible lines; 90s tower (4 modes), DIFFERENCE for every focus, plate TV/onboard, FASTEST LAP and qualy tower stay inside their areas (%d renders).'%tests)
