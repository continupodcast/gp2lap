import ctypes as C
import subprocess
import tempfile
from pathlib import Path
from PIL import Image

root=Path(__file__).resolve().parents[1]
class Driver(C.Structure):
    _fields_=[('id',C.c_int),('number',C.c_int),('team',C.c_int),('abbr',C.c_char*8),('last',C.c_char*24),('first',C.c_char*24)]
with tempfile.TemporaryDirectory() as tmp:
    lib=Path(tmp)/'driver.so'
    subprocess.run(['gcc','-std=c89','-Wall','-Wextra','-Werror','-Wno-missing-braces','-shared','-fPIC',*[str(root/'src'/f) for f in ['f1render.c','f1timing.c','f1config.c']],'-lm','-o',str(lib)],check=True)
    api=C.CDLL(str(lib));api.F1ConfigLoad(None)
    drivers=(Driver*28).in_dll(api,'f1_drivers');d=drivers[0]
    d.first=b'Damon';d.last=b'Hill';d.number=0
    # A fixed palette lets image bounds and uppercase rendering be compared.
    pal=[v for i in range(256) for v in (round(((i>>5)&7)*63/7),round(((i>>2)&7)*63/7),round((i&3)*63/3))]
    palette=(C.c_ubyte*768)(*pal)
    api.F1RenderDriverAt.argtypes=[C.POINTER(C.c_ubyte),C.POINTER(C.c_ubyte),C.c_int,C.c_int,C.c_int]
    def draw(pos=1,y=8):
        b=(C.c_ubyte*(640*480))(*([109]*(640*480)))
        api.F1RenderDriverAt(b,palette,d.id,pos,y)
        return bytes(b)
    mixed=draw();d.last=b'HILL';upper=draw();assert mixed==upper
    im=Image.frombytes('P',(640,480),upper);im.putpalette([v*4 for v in pal])
    im.crop((220,8,420,46)).convert('RGB').resize((800,152),Image.Resampling.NEAREST).save(root/'validation/driver-027.png')
    for name in [b'Verstappen',b'Hulkenberg',b'Longlastnamefortestonly']:
        d.last=name
        for pos in [1,12,26]:
            for y in [8,421,442]:
                b=draw(pos,y)
                assert all(b[yy*640+xx]==109 for yy in range(480) for xx in range(640) if not (220<=xx<420 and y<=yy<y+38))
    # White position glyph occupies >=12 vertical pixels in its isolated cell.
    ys=[yy for yy in range(8,46) if any(upper[yy*640+xx]==255 for xx in range(222,252))]
    assert max(ys)-min(ys)+1>=12
print('PASS: uppercase rendering, enlarged position, long surnames and 200x38 card bounds in TV/onboard locations')
