from pathlib import Path
from PIL import Image, ImageFont, ImageDraw
root=Path(__file__).resolve().parents[1]
fontpath=root/'f1-input/fonts/PIXEARG_.TTF'
rows=[];widths=[]
for size in (8,12):
 font=ImageFont.truetype(str(fontpath),size)
 glyphs=[];adv=[]
 for code in range(32,127):
  im=Image.new('L',(24,20));ImageDraw.Draw(im).text((0,-1),chr(code),font=font,fill=255)
  glyphs.append([sum((1<<x) for x in range(24) if im.getpixel((x,y))>=128) for y in range(20)])
  adv.append(round(font.getlength(chr(code))))
 rows.append(glyphs);widths.append(adv)
out='/* Rasterized from supplied Pixel Arial 11; menu only. */\n'
out+='static const unsigned char strategyFontWidth[2][95]={'+','.join('{'+','.join(map(str,x))+'}' for x in widths)+'};\n'
out+='static const unsigned long strategyFontBits[2][95][20]={'+','.join('{'+','.join('{'+','.join(str(r)+'UL' for r in g)+'}' for g in x)+'}' for x in rows)+'};\n'
(root/'src/f1strategyfont.h').write_text(out)
