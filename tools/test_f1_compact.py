"""Host checks for the native renderer and cockpit window, without DOS emulation."""
import ctypes as C
import subprocess
import tempfile
from pathlib import Path
from PIL import Image
root=Path(__file__).resolve().parents[1]
class Row(C.Structure):
    _fields_=[(k,C.c_int) for k in ['id','pos','lap','out','pit','focused']]+[('gap',C.c_long),('gain',C.c_int),('stops',C.c_int),('leaderGap',C.c_long),('lapsBehind',C.c_int),('fastest',C.c_int),('positionChange',C.c_int)]
with tempfile.TemporaryDirectory() as temp:
    lib=Path(temp)/'f1.so'
    subprocess.run(['gcc','-std=c89','-Wall','-Wextra','-Werror','-Wno-missing-braces','-shared','-fPIC',str(root/'src/f1render.c'),str(root/'src/f1timing.c'),str(root/'src/f1config.c'),'-o',str(lib)],check=True)
    api=C.CDLL(str(lib)); api.F1Window.argtypes=[C.POINTER(Row),C.c_int,C.c_int,C.POINTER(C.c_int)]
    api.F1Gap.argtypes=[C.c_char_p,C.c_long]
    for ms,expected in [(-1,'-'),(0,'+0.0'),(49,'+0.0'),(50,'+0.1'),(1234,'+1.2'),(1250,'+1.3'),(59949,'+59.9'),(59950,'+1:00.0'),(3600000,'+60:00.0')]:
        label=C.create_string_buffer(40);api.F1Gap(label,ms);assert label.value.decode()==expected
    rows=(Row*26)(*[Row(i+1,i+1,3,0,0,0,1234) for i in range(26)])
    for count in range(1,27):
        for focus in range(count):
            start=C.c_int();n=api.F1Window(rows,count,focus+1,C.byref(start))
            assert list(range(start.value,start.value+n))==list(range(max(0,focus-2),min(count,focus+3)))
    start=C.c_int();assert api.F1Window(rows,26,40,C.byref(start))==0
    pal=[]
    for i in range(256):pal.extend([round(((i>>5)&7)*63/7),round(((i>>2)&7)*63/7),round((i&3)*63/3)])
    palette=(C.c_ubyte*768)(*pal)
    api.F1RenderRace.argtypes=[C.POINTER(C.c_ubyte),C.POINTER(C.c_ubyte),C.POINTER(Row),C.c_int,C.c_int,C.c_int,C.c_int]
    for direction in [-1,1]:
        rows[0].positionChange=direction
        screen=(C.c_ubyte*(640*480))(*([109]*(640*480)))
        api.F1RenderRace(screen,palette,rows,1,3,42,0)
        tip_y=45 if direction>0 else 48
        base_y=48 if direction>0 else 45
        color=screen[tip_y*640+18]
        assert all(screen[base_y*640+x]==color for x in range(15,22))
        assert screen[tip_y*640+17]!=color and screen[tip_y*640+19]!=color
    rows[0].positionChange=0
    api.F1Render.argtypes=[C.POINTER(C.c_ubyte),C.POINTER(C.c_ubyte),C.POINTER(Row),C.c_int,C.c_int,C.c_int]
    # Five rows, leader/last edge sizes and the full grid: no writes outside panel.
    for n in [1,3,4,5,26]:
        guard=64;buf=(C.c_ubyte*(640*480+guard*2))(*([109]*(640*480+guard*2)))
        screen=C.cast(C.byref(buf,guard),C.POINTER(C.c_ubyte))
        api.F1Render(screen,palette,rows,n,3,42)
        assert list(buf[:guard])==[109]*guard and list(buf[-guard:])==[109]*guard
        for y in range(480):
            for x in range(640):
                if not (8<=x<112 and 8<=y<8+34+n*13+3):assert screen[y*640+x]==109
    # Use the same five-row drawing code for a preview, with actual latest mappings.
    ids=[12,10,34,23,35]
    compact=(Row*5)(*[Row(id,7+i,3,0,0,i==2,1234+i*400) for i,id in enumerate(ids)])
    buf=(C.c_ubyte*(640*480))(*[109]*(640*480))
    api.F1Render(buf,palette,compact,5,3,42)
    im=Image.frombytes('P',(640,480),bytes(buf));im.putpalette([v*4 for v in pal]);im.convert('RGB').save(root/'validation/compact-preview.png')
print('PASS: rounding/carry; all focus positions in 1..26-car grids; unknown focus; panel bounds in 1/3/4/5/26-row modes.')
