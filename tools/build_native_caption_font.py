"""Encode KH Interference in GP2's native proportional bitmap font format."""
from pathlib import Path
import struct
from PIL import Image, ImageDraw, ImageFont
root=Path(__file__).resolve().parents[1]
font=ImageFont.truetype(str(root/'f1-input/fonts/KHINTERFERENCE-REGULAR.OTF'),10)
first,last=22,255
table=bytearray(2*(last-first+1));glyphs=bytearray()
for code in range(first,last+1):
    char=chr(code+26) if 22<=code<=31 else '-' if code==226 else bytes([code]).decode('cp850')
    width=max(1,min(12,round(font.getlength(char))))
    im=Image.new('L',(width,9));ImageDraw.Draw(im).text((0,-1),char,font=font,fill=255)
    pixels=[int(v>=90) for v in im.getdata()]
    # Native renderer consumes packed DWORDs MSB first, continuously across rows.
    packed=bytearray()
    for i in range(0,len(pixels),32):
        word=sum(v<<(31-j) for j,v in enumerate(pixels[i:i+32]))
        packed+=struct.pack('<I',word)
    packed+=b'\0'*4 # native reader prefetches at exact 32-bit boundaries
    struct.pack_into('<H',table,(code-first)*2,len(table)+len(glyphs))
    glyphs+=bytes([width,9,13,0])+packed
# first, last, reserved, line height, ascent, descent, space, spacing, leading,
# optional kerning pointer. All addresses are relative except kerning=none.
block=struct.pack('<10I',first,last,0,13,10,3,3,0,0,0)+table+glyphs
(root/'validation/native-caption-font.bin').write_bytes(block)
(root/'src/f1nativefont.h').write_text('/* KH Interference Regular, native GP2 bitmap format. Generated. */\nstatic unsigned char f1_native_font[] = {\n'+',\n'.join(','.join(str(v) for v in block[i:i+32]) for i in range(0,len(block),32))+'\n};\n')
print('Native caption font:',len(block),'bytes')
