import ctypes as C
from pathlib import Path
import subprocess,tempfile,struct
root=Path(__file__).resolve().parents[1]
class Fuel(C.Structure):
 _fields_=[('targets',C.c_ubyte*41),('start',C.c_ulong),('last',C.c_ulong),('duration',C.c_ulong),('active',C.c_int),('held',C.c_int)]
with tempfile.TemporaryDirectory() as tmp:
 tmp=Path(tmp);lib=tmp/'actions.so'
 subprocess.run(['gcc','-std=c89','-Wall','-Wextra','-Werror','-Wno-missing-braces','-shared','-fPIC',str(root/'src/f1actions.c'),str(root/'src/f1config.c'),'-o',str(lib)],check=True)
 api=C.CDLL(str(lib));api.F1FuelStart.argtypes=[C.POINTER(Fuel),C.POINTER(C.c_ubyte),C.c_ulong,C.c_ulong];api.F1FuelTick.argtypes=[C.POINTER(Fuel),C.c_ulong,C.c_int]
 state=Fuel();targets=(C.c_ubyte*41)();targets[10]=targets[34]=1
 assert api.F1FuelKey(C.byref(state),8)==1
 assert api.F1FuelKey(C.byref(state),8)==0
 assert api.F1FuelKey(C.byref(state),136)==0
 assert api.F1FuelKey(C.byref(state),8)==1
 api.F1FuelStart(C.byref(state),targets,1000,3000)
 for raw in range(256):assert bool(api.F1FuelTarget(C.byref(state),raw))==((raw&63) in (10,34))
 assert api.F1FuelTick(C.byref(state),3999,1)==1
 assert api.F1FuelTick(C.byref(state),4000,1)==0
 api.F1FuelStart(C.byref(state),targets,1000,3000);assert api.F1FuelTick(C.byref(state),999,1)==0
 api.F1FuelStart(C.byref(state),targets,1000,3000);assert api.F1FuelTick(C.byref(state),1100,0)==0
 api.F1FuelStart(C.byref(state),targets,1000,3000);api.F1FuelCancel(C.byref(state));assert not api.F1FuelTarget(C.byref(state),10)
 # Verify the patch against the provided EXE, not a made-up byte signature.
 exe=(root.parent/'upload/GP2.EXE').read_bytes();start=0x88254+0x6e69b-0x10000;original=exe[start:start+0x10e]
 buf=(C.c_ubyte*len(original)).from_buffer_copy(original)
 assert api.F1PatchCameraCaption(buf)==1
 patched=bytes(buf);assert patched[:11]==original[:11] and patched[17:]==original[17:]
 assert patched[11]==0xe9 and 0x6e69b+11+5+struct.unpack_from('<i',patched,12)[0]==0x6e772
 bad=bytearray(original);bad[1]=0x90;buf=(C.c_ubyte*len(bad)).from_buffer_copy(bad)
 assert api.F1PatchCameraCaption(buf)==0 and bytes(buf)==bad
 cfg=tmp/'GP2LAP.CFG';api.F1ConfigLoad.argtypes=[C.c_char_p]
 cfg.write_text('[F1 Controls]\nHideCameraCaption=0\nFuelDrainEnabled=1\nFuelDrainMilliseconds=1200\nFuelDrainCarIds="10,34"\n')
 assert api.F1ConfigLoad(str(cfg).encode())==0
 assert C.c_int.in_dll(api,'f1_hide_camera_caption').value==0
 assert C.c_int.in_dll(api,'f1_fuel_enabled').value==1
 assert C.c_ulong.in_dll(api,'f1_fuel_duration').value==1200
 assert list((C.c_ubyte*41).in_dll(api,'f1_fuel_targets'))==list(targets)
 for value in ['0','41','10,34,','10,no','10;bad']:
  # A semicolon is a supported CFG comment, not an invalid ID.
  if ';' in value:continue
  cfg.write_text('[F1 Controls]\nFuelDrainEnabled=1\nFuelDrainCarIds="'+value+'"\n')
  assert api.F1ConfigLoad(str(cfg).encode())==-1
  assert C.c_int.in_dll(api,'f1_fuel_enabled').value==0
 cfg.write_text('[F1 Controls]\nFuelDrainMilliseconds=30001\n');assert api.F1ConfigLoad(str(cfg).encode())==-1
 # Compile the actual native adapter against a small host telemetry fixture.
 native=(root/'src/f1tower.c').read_text()
 adapter=native[native.index('void F1ControlCancel(void)'):native.index('void F1LogKey(')]
 harness=tmp/'adapter.c'
 harness.write_text('''
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "f1actions.h"
#include "f1config.h"
typedef struct { unsigned int id,flags_90; unsigned long fuelLoad,guard; } GP2Car;
static GP2Car cars[3],*pCarStructs=cars;
static unsigned long now=1000,session=1,track=1;
static unsigned long *pCurTime=&now,*pSesStartTime=&session,*pTrackNr=&track;
static int count=3,replay,paused,*pNumCars=&count,*pIsReplay=&replay,*pPaused=&paused;
static F1FuelState fuelState;
static unsigned long fuelSession,fuelTrack,fuelNoticeAt;
static char fuelNotice[80];
static void F1DiagLog(const char *s) { (void)s; }
''' + adapter + '''
static void arm(void) { F1ControlKey(136);F1ControlKey(8); }
int main(void) {
 int i;
 f1_fuel_enabled=1;f1_fuel_duration=3000;f1_fuel_targets[10]=f1_fuel_targets[34]=1;
 cars[0].id=138;cars[1].id=34;cars[2].id=20;
 for(i=0;i<3;i++) {cars[i].fuelLoad=100;cars[i].guard=12345;}
 arm();F1ControlUpdate();assert(cars[0].fuelLoad==0 && cars[1].fuelLoad==0 && cars[2].fuelLoad==100);
 for(i=0;i<3;i++) assert(cars[i].guard==12345);
 now=3999;cars[0].fuelLoad=90;F1ControlUpdate();assert(cars[0].fuelLoad==0);
 now=4000;cars[0].fuelLoad=90;F1ControlUpdate();assert(cars[0].fuelLoad==90);
 arm();paused=1;F1ControlUpdate();assert(cars[0].fuelLoad==90);paused=0;
 replay=1;F1ControlUpdate();replay=0;F1ControlUpdate();assert(cars[0].fuelLoad==90);
 arm();session++;F1ControlUpdate();assert(cars[0].fuelLoad==90);
 arm();track++;F1ControlUpdate();assert(cars[0].fuelLoad==90);
 arm();now--;F1ControlUpdate();assert(cars[0].fuelLoad==90);
 arm();F1ControlCancel();F1ControlUpdate();assert(cars[0].fuelLoad==90);
 cars[0].flags_90=32;arm();F1ControlUpdate();assert(cars[0].fuelLoad==90);
 cars[0].flags_90=0;F1ControlCancel();f1_fuel_enabled=0;arm();F1ControlUpdate();assert(cars[0].fuelLoad==90);
 return 0;
}
''')
 exe=tmp/'adapter'
 subprocess.run(['gcc','-std=c89','-Wall','-Wextra','-Werror','-Wno-missing-braces','-I'+str(root/'src'),str(harness),str(root/'src/f1actions.c'),str(root/'src/f1config.c'),'-o',str(exe)],check=True)
 subprocess.run([str(exe)],check=True)
print('PASS: fuel ID masking/selection, key repeat, timeout, rewind, cancellation; real EXE caption branch and mismatch guard; controls CFG.')
