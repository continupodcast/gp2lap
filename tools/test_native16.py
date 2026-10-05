"""Run the real GP2 native caption renderer with the replacement bitmap font."""
from pathlib import Path
import struct
from unicorn import Uc, UC_ARCH_X86, UC_MODE_32, UC_HOOK_MEM_WRITE
from unicorn.x86_const import *
from PIL import Image
root=Path(__file__).resolve().parents[1]
exe=(root.parent/'upload/GP2(1).EXE').read_bytes()
code=exe[0x88254:0x88254+0x99730]
font=(root/'validation/native-caption-font.bin').read_bytes()
def render(text,view=0,hidden=False):
 u=Uc(UC_ARCH_X86,UC_MODE_32);u.mem_map(0,0x900000);u.mem_write(0x10000,code)
 def put(a,n):u.mem_write(a,struct.pack('<I',n))
 def get(a):return struct.unpack('<I',u.mem_read(a,4))[0]
 # Resolve only the native code table references used here. Data stays at
 # its original segment-relative addresses in this controlled fixture.
 put(0x8a65a,0x8a668);put(0x8a668+0x44*4,0x8b48e)
 put(0x7c12b,0x7c138)
 for i in range(22):put(0x7c138+i*4,get(0x7c138+i*4)+0x10000)
 put(0x7d774,0x7d838);put(0x7d788,0x7c433)
 for i in range(22):put(0x40749a+i*4,2 if i in (2,6,7) else 1)
 u.mem_write(0x600000,font)
 for i in range(3):put(0x4075f5+i*4,0x600000)
 put(0x407452,0xffffffff)
 u.mem_write(0xb70dc,b'\xff');put(0x16c24,round(2.4*16384));put(0xa6a0,640)
 put(0xa07c,0x500000);u.mem_write(0x500000,b'\x30'*(640*480))
 u.mem_write(0xb1080,bytes(range(256))*16)
 u.mem_write(0xca98,bytes([view]));u.mem_write(0xca58,bytes([0,3-int(view!=0),0,0]));u.mem_write(0x39cff,b'\xff')
 def run(addr):
  u.reg_write(UC_X86_REG_ESP,0x800000);put(0x800000,0x8ff000)
  u.emu_start(addr,0x8ff000,count=3000000)
  assert u.reg_read(UC_X86_REG_EIP)==0x8ff000,hex(u.reg_read(UC_X86_REG_EIP))
 run(0x7d75d)
 message=b' \x02\x07\x07\x01'+bytes(c-26 if 48<=c<=57 else c for c in text.encode('cp850'))
 u.mem_write(0x39d04,message+b'\0'*3);u.reg_write(UC_X86_REG_EDI,0x39d04+len(message))
 writes=[]
 def watch(uc,access,address,size,value,data):
  if 0x5c95c-640<=address<0x60a5c+640:writes.append(address)
 u.hook_add(UC_HOOK_MEM_WRITE,watch)
 if hidden:
  # Same destination-pointer substitution as beforeFastest/after. Execute
  # the entire original caption routine, including its bookkeeping.
  put(0xa07c,0x650000)
 run(0x6d792)
 if hidden:
  put(0xa07c,0x500000)
  assert bytes(u.mem_read(0x500000,640*480))==b'\x30'*(640*480)
  assert any(u.mem_read(0x650000,640*480)), 'replacement destination not drawn'
 width=get(0xb4590);height=get(0xb4594);offset=get(0x39cf4)
 assert 0<width<640 and height==13,(width,height)
 assert offset==57*640+320-width//2,offset
 assert min(writes)>=0x5c95c and max(writes)<0x60a5c,(hex(min(writes)),hex(max(writes)))
 banner=bytes(u.mem_read(0x5c95c,640*26))
 assert all(v==255 for v in banner[13*640:]),'glyph exceeds caption height'
 assert 7 in banner,'no foreground glyphs rendered'
 frame=bytes(u.mem_read(0x500000,640*480))
 left=320-width//2
 for y in range(480):
  row=frame[y*640:(y+1)*640]
  assert all(v==48 for v in (row if not 57<=y<70 else row[:left]+row[left+width:])), 'framebuffer write outside banner'

 im=Image.frombytes('P',(640,480),bytes(u.mem_read(0x500000,640*480)))
 pal=[(20,24,30)]*256;pal[48]=(80,100,90);pal[7]=(235,231,142);pal[0]=(0,0,0)
 im.putpalette([v for rgb in pal for v in rgb])
 return im.convert('RGB').crop((0,50,640,76)),width
if __name__=='__main__':
 labels=['Viewing Franco COLAPINTO [43]','Riding with Nico HULKENBERG [27]','Race won by LEWIS HAMILTON','PIERRE GASLY is out of the race','Fastest Lap: MAX VERSTAPPEN 1:35.505','FERNANDO ALONSO is in the pits','PAUSED']
 canvas=Image.new('RGB',(640,len(labels)*26))
 for i,label in enumerate(labels):
  im,width=render(label);canvas.paste(im,(0,i*26));print(label,width)
 for view in (1,2):render(labels[0],view)
 for view in (0,1,2):render(labels[4],view,hidden=True)
 canvas.resize((1280,len(labels)*52)).save(root/'validation/native-captions-016.png')
 print('PASS: original x86 renderer, encoded digits, message categories, centered placement, bitmap and framebuffer bounds.')
