import ctypes as C,subprocess,tempfile,json
from pathlib import Path
from PIL import Image
root=Path(__file__).resolve().parents[1]
with tempfile.TemporaryDirectory() as tmp:
 stub=Path(tmp)/'stub.c';stub.write_text('int F1QCombinedMode;\n')
 lib=Path(tmp)/'grid.so'
 subprocess.run(['gcc','-std=c89','-Wall','-Wextra','-Werror','-Wno-missing-braces','-shared','-fPIC',str(root/'src/f1render.c'),str(root/'src/f1timing.c'),str(root/'src/f1config.c'),str(stub),'-lm','-o',str(lib)],check=True)
 api=C.CDLL(str(lib));ids0=[d['car_id'] for d in json.loads((root/'f1-input/drivers.json').read_text())['drivers']][:26]
 ids=(C.c_ubyte*26)(*ids0);times=(C.c_ulong*40)()
 for i,k in enumerate(ids0):times[k-1]=81505+i*147
 colors=[round(((i>>5)&7)*63/7) if c==0 else round(((i>>2)&7)*63/7) if c==1 else round((i&3)*63/3) for i in range(256) for c in range(3)]
 pal=(C.c_ubyte*768)(*colors)
 for scroll in (0,8,256,704,1404):
  data=(C.c_ubyte*(640*480+128))(*([109]*(640*480+128)));screen=C.cast(C.byref(data,64),C.POINTER(C.c_ubyte))
  api.F1RenderGridMenu(screen,pal,ids,times,26,C.c_long(scroll),C.c_long(1724),C.c_char_p(b"Japan"),C.c_char_p(b"Suzuka"),53,(C.c_int*4)(371,444,492,468))
  assert bytes(data[:64])==bytes([109])*64 and bytes(data[-64:])==bytes([109])*64
  b=bytes(data[64:-64])
  for y in range(480):
   row=b[y*640:(y+1)*640]
   assert all(v==109 for v in (row if not 58<=y<475 else row[:140]+row[496:])),y
  for yy in range(58,437):
   assert len(set(b[yy*640+474:yy*640+496]))==1, (scroll,yy)
  assert b[437*640:]==bytes([109])*((480-437)*640)
  if scroll==0:
   img=Image.frombytes('P',(640,480),b);img.putpalette([v*4 for v in colors]);img.convert('RGB').crop((135,53,501,480)).resize((732,854)).save(root.parent/'validation/grid-panel-preview.png')
 theme=C.c_int.in_dll(api,'f1_theme')
 snapshots=[]
 for mode in (0,1):
  theme.value=mode
  data=(C.c_ubyte*(640*480))(*([109]*(640*480)))
  api.F1RenderGridMenu(data,pal,ids,times,26,C.c_long(0),C.c_long(1724),C.c_char_p(b"Japan"),C.c_char_p(b"Suzuka"),53,(C.c_int*4)(0,0,0,0))
  snapshots.append(bytes(data))
 assert snapshots[0]!=snapshots[1], '90s theme must have its own typography and palette'
 for mode,b in enumerate(snapshots):
  assert b[437*640:]==bytes([109])*((480-437)*640)
  for y in range(480):
   assert b[y*640:y*640+140]==bytes([109])*140
   assert b[y*640+496:(y+1)*640]==bytes([109])*144
  img=Image.frombytes('P',(640,480),b);img.putpalette([v*4 for v in colors])
  img.convert('RGB').crop((135,53,501,480)).resize((732,854)).save(root.parent/('validation/grid-theme-%d.png'%mode))
  for y in range(58,437):
   assert len(set(b[y*640+474:y*640+496]))==1,(mode,y)
  controls=[(379,440,500,463,2),(259,440,378,463,1),(139,440,258,463,0),
            (475,110,494,129,3),(475,130,494,149,4),(479,153,488,386,7),
            (475,390,494,409,5),(475,410,494,429,6)]
  buf=(C.c_ubyte*(640*480+128))(*([109]*(640*480+128)))
  screen=C.cast(C.byref(buf,64),C.POINTER(C.c_ubyte))
  theme.value=mode
  for x,y,right,bottom,kind in controls:
   api.F1RenderGridControl(screen,pal,(C.c_int*4)(x,y,right,bottom),kind,0,C.c_long(704),C.c_long(1724))
  assert bytes(buf[:64])==bytes([109])*64 and bytes(buf[-64:])==bytes([109])*64
 print('Both themes: complete scrollbar backing and control buffer guards passed')
 print('Grid renderer guards and original controls bounds passed for five scroll positions')
