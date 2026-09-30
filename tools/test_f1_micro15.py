import ctypes as C
import subprocess, tempfile
from pathlib import Path

root=Path(__file__).resolve().parents[1]
class Car(C.Structure):
    _fields_=[('seen',C.c_int),('lap',C.c_int),('haveCross',C.c_int),('newLap',C.c_int),('position',C.c_double),('cross',C.c_double),('clock',C.c_ulong),('personal',C.c_long*30),('colors',C.c_ubyte*30)]
class State(C.Structure):
    _fields_=[('cars',Car*41),('best',C.c_long*30)]
with tempfile.TemporaryDirectory() as tmp:
    tmp=Path(tmp);lib=tmp/'micro.so'
    subprocess.run(['gcc','-std=c89','-Wall','-Wextra','-Werror','-shared','-fPIC',str(root/'src/f1micro.c'),str(root/'src/f1actions.c'),'-o',str(lib)],check=True)
    api=C.CDLL(str(lib));api.F1MicroSample.argtypes=[C.POINTER(State),C.c_int,C.c_int,C.c_double,C.c_ulong,C.c_int]
    s=State()
    def sample(id,pos,t,lap=0,valid=1):api.F1MicroSample(C.byref(s),id,lap,pos,t,valid)
    def segment(id,base,third,fourth):
        for pos,t in [(.8,0),(1.2,100),(1.8,third),(2.2,fourth)]:sample(id,pos,base+t)
    segment(1,0,300,500)
    assert s.best[1]==350 and s.cars[1].colors[1]==3
    assert not s.cars[1].personal[0] # Partial first micro is never scored.
    segment(2,0,400,600)
    assert s.cars[2].personal[1]==450 and s.cars[2].colors[1]==1
    segment(2,1000,350,550)
    assert s.cars[2].personal[1]==400 and s.cars[2].colors[1]==1
    segment(2,2000,450,650)
    assert s.cars[2].colors[1]==2 and s.cars[2].personal[1]==400
    segment(2,3000,200,400)
    assert s.best[1]==250 and s.cars[2].colors[1]==3
    assert s.cars[1].colors[1]==1 # Previous record is green immediately.
    assert s.cars[1].personal[1]==350 # Preserve its actual personal best.
    previous=bytes(s)
    sample(0,1,0);sample(41,1,0)
    assert bytes(s)==previous
    sample(2,3,3700,valid=0)
    assert s.cars[2].colors[1]==3 and not s.cars[2].haveCross
    assert s.cars[2].personal[1]==250
    sample(1,3,5000) # Missing sampling interval must not create a record.
    assert not s.cars[1].haveCross and s.best[1]==250
    api.F1MicroReset(C.byref(s))
    # Two complete laps with frame-sized samples exercise boundary 29 -> 0.
    for step in range(1,311):
        absolute=step/5.0
        sample(3,absolute%30,step*100,int(absolute//30))
    assert all(v==500 for v in s.cars[3].personal)
    sample(3,3,0,lap=0) # rewind invalidates the current traversal
    assert not s.cars[3].haveCross and not any(s.cars[3].colors)
    api.F1MicroReset(C.byref(s));assert not any(bytes(s))
    # Records from non-selected drivers update all stored cards, including
    # tied records. Equal times share purple until strictly beaten.
    segment(1,0,300,500);segment(2,0,300,500)
    assert s.cars[1].colors[1]==s.cars[2].colors[1]==3
    s.cars[4].colors[1]=2;s.cars[5].colors[1]=0
    s.cars[1].colors[7]=3
    segment(3,0,200,400)
    assert s.cars[1].colors[1]==s.cars[2].colors[1]==1
    assert s.cars[3].colors[1]==3 and s.best[1]==250
    assert s.cars[4].colors[1]==2 and s.cars[5].colors[1]==0
    assert s.cars[1].colors[7]==3
    segment(3,1000,100,300) # Same driver beats its own record.
    assert s.cars[3].colors[1]==3 and s.best[1]==150
    api.F1MicroReset(C.byref(s));assert not any(bytes(s))
    # Patch validation uses the provided executable, including return bytes.
    exe=(root.parent/'upload/GP2.EXE').read_bytes();offset=0x88254+0x6e836-0x10000
    original=exe[offset:offset+0x78]
    buf=(C.c_ubyte*len(original)).from_buffer_copy(original)
    assert api.F1PatchRetirementCaption(buf)==1
    assert bytes(buf[:11])==original[:11] and bytes(buf[12:])==original[12:]
    assert buf[11]==0xeb and 0x6e836+13+buf[12]==0x6e8ac
    for index in (1,11,0x76,0x77):
        bad=bytearray(original);bad[index]^=1
        buf=(C.c_ubyte*len(bad)).from_buffer_copy(bad)
        assert api.F1PatchRetirementCaption(buf)==0 and bytes(buf)==bad
    # Exercise the actual key-cycle function with a minimal page fixture.
    text=(root/'src/f1tower.c').read_text()
    toggle=text[text.index('void F1ToggleCard('):text.index('void F1ToggleMap(')]
    fixture=tmp/'toggle.c'
    fixture.write_text('''#include <assert.h>
#define PAGE_F1CARD 4
static int activepage,F1MicroMode,f1_micro_enabled=1;
static void F1AdvancedReport(void) {}
static void F1DiagLog(const char *s) {(void)s;}
static void F1TogglePart(int p) {activepage^=p;}
'''+toggle+'''
int main(void) {
 F1ToggleCard();assert(activepage==4 && !F1MicroMode);
 F1ToggleCard();assert(activepage==4 && F1MicroMode);
 F1ToggleCard();assert(!activepage && !F1MicroMode);
 F1ToggleCard();F1ToggleCard();activepage=0;
 F1ToggleCard();assert(activepage==4 && !F1MicroMode);
 f1_micro_enabled=0;F1ToggleCard();assert(!activepage);
 return 0;
}''')
    subprocess.run(['gcc','-std=c89','-Wall','-Wextra','-Werror',str(fixture),'-o',str(tmp/'toggle')],check=True)
    subprocess.run([str(tmp/'toggle')],check=True)
print('PASS: interpolated micro times, personal/session comparisons, full-lap wrapping, missing data, rewind, real retirement patch and key-4 cycle.')
