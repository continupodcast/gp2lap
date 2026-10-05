"""Execute the fastest-caption hook from frankasm.inc with controlled globals."""
from pathlib import Path
import re, struct, subprocess, tempfile
from unicorn import Uc, UC_ARCH_X86, UC_MODE_32
from unicorn.x86_const import *
root=Path(__file__).resolve().parents[1]
s=(root/'src/frankasm.inc').read_text().split('Hook_PrnFastestLap proc  near')[1].split('Hook_PrnFastestLap endp')[0]
s=s.split('MyOrgPFLJmp:')[0]+'MyOrgPFLJmp: mov dword ptr [0x2030],1\n ret\n'
s=s.replace('retn','ret').replace('ds:_fpF1SuppressNativeFastest','[0x2000]').replace('ds:_SupressFastestLap','[0x2010]').replace('ds:_activepage','[0x2020]')
s=re.sub(r';[^\n]*','',s)
with tempfile.TemporaryDirectory() as t:
 p=Path(t);(p/'hook.s').write_text('.intel_syntax noprefix\n.code32\n.text\n'+s)
 subprocess.run(['as','--32',str(p/'hook.s'),'-o',str(p/'hook.o')],check=True)
 subprocess.run(['objcopy','-O','binary','-j','.text',str(p/'hook.o'),str(p/'hook.bin')],check=True)
 for hud,cfg,page in [(0,0,0),(1,1,0),(0,1,9),(0,0,9),(0,1,0)]:
  u=Uc(UC_ARCH_X86,UC_MODE_32);u.mem_map(0x1000,0x9000);u.mem_write(0x1000,(p/'hook.bin').read_bytes())
  def put(a,n):u.mem_write(a,struct.pack('<I',n))
  put(0x2000,0x3000);put(0x2010,cfg);put(0x2020,page)
  u.mem_write(0x3000,b'\xb8'+struct.pack('<I',hud)+b'\xc3')
  regs=[UC_X86_REG_EAX,UC_X86_REG_EBX,UC_X86_REG_ECX,UC_X86_REG_EDX,UC_X86_REG_ESI,UC_X86_REG_EDI,UC_X86_REG_EBP]
  for i,r in enumerate(regs):u.reg_write(r,100+i)
  u.reg_write(UC_X86_REG_EFLAGS,0x246);u.reg_write(UC_X86_REG_ESP,0x9000);put(0x9000,0x4000)
  u.emu_start(0x1000,0x4000,count=1000)
  assert [u.reg_read(r) for r in regs]==list(range(100,107))
  assert u.reg_read(UC_X86_REG_EFLAGS)==0x246 and u.reg_read(UC_X86_REG_ESP)==0x9004
  forwarded=struct.unpack('<I',u.mem_read(0x2030,4))[0]
  assert forwarded==int(not(hud or (cfg and page==9)))
print('PASS: original fastest-caption hook extension: HUD suppression, legacy At The Line, normal forwarding, registers/flags/stack preserved.')
