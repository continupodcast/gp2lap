import ctypes as C
import tempfile
import subprocess
from pathlib import Path

root=Path(__file__).resolve().parents[1]
class Driver(C.Structure):
    _fields_=[('id',C.c_int),('number',C.c_int),('team',C.c_int),('abbr',C.c_char*8),('last',C.c_char*24),('first',C.c_char*24)]
with tempfile.TemporaryDirectory() as temp:
    temp=Path(temp); lib=temp/'f1.so'; cfg=temp/'GP2LAP.CFG'
    subprocess.run(['gcc','-std=c89','-Wall','-Wextra','-Werror','-Wno-missing-braces','-shared','-fPIC',str(root/'src/f1config.c'),'-o',str(lib)],check=True)
    api=C.CDLL(str(lib)); api.F1ConfigLoad.argtypes=[C.c_char_p]; api.F1ConfigLoad.restype=C.c_int
    cfg.write_text('[F1 HUD]\nF1UseCustomData = 1\n[F1 Teams]\nTeam01Name = "TEST"\nTeam01Color = "#112233"\nTeam01Logo = 0\n[F1 Drivers]\nDriver01CarId = 40\nDriver01Abbr = "TST"\nDriver01Number = 99\nDriver01Team = 1\n')
    assert api.F1ConfigLoad(str(cfg).encode())==1
    drivers=(Driver*26).in_dll(api,'f1_drivers'); colors=((C.c_ubyte*3)*13).in_dll(api,'f1_colors'); logos=(C.c_ubyte*13).in_dll(api,'f1_team_logo_enabled')
    assert (drivers[0].id,drivers[0].number,drivers[0].abbr)==(40,99,b'TST') and tuple(colors[0])==(17,34,51) and logos[0]==0
    cfg.write_text('[F1 HUD]\nF1UseCustomData = 1\n[F1 Drivers]\nDriver01CarId = 2\nDriver02CarId = 2\n')
    assert api.F1ConfigLoad(str(cfg).encode())==-1
    cfg.write_text('[F1 HUD]\nF1UseCustomData = 1\n[F1 Teams]\nTeam14Name = "PACIFIC"\nTeam14Color = "#123456"\nTeam14Logo = 1\nTeam14LogoFile = "TEAM14.F1L"\n[F1 Drivers]\nDriver27CarId = 1\nDriver27Team = 14\nDriver28CarId = 2\nDriver28Team = 14\n')
    (temp/'TEAM14.F1L').write_bytes(b'F1L1'+bytes([255])*768)
    assert api.F1ConfigLoad(str(cfg).encode())==1
    drivers28=(Driver*28).in_dll(api,'f1_drivers')
    assert drivers28[26].id==1 and drivers28[27].team==13
    assert (C.c_ubyte*14).in_dll(api,'f1_custom_logo_loaded')[13]==1
    cfg.write_text(cfg.read_text().replace('Driver28CarId = 2','Driver28CarId = 1'))
    assert api.F1ConfigLoad(str(cfg).encode())==-1
    cfg.write_text('[F1 Race Tower]\nGapStartProgress=50\nGapUpdateTime=5000\nPositionChangeTime=1000\n')
    assert api.F1ConfigLoad(str(cfg).encode())==0
    assert C.c_ulong.in_dll(api,'f1_gap_update').value==4000 # Legacy key ignored.
    assert C.c_int.in_dll(api,'f1_gap_start').value==50
    cfg.write_text('[F1 Race Tower]\nGapStartProgress=200\n')
    assert api.F1ConfigLoad(str(cfg).encode())==-1
    assert C.c_ulong.in_dll(api,'f1_gap_update').value==4000
print('PASS: CFG overrides, colours, duplicate CarId rejection, tower options and range checks.')
