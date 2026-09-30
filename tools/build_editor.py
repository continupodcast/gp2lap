import base64
import io
import json
import re
from pathlib import Path
from PIL import Image

root=Path(__file__).resolve().parents[1]
data=json.loads((root/'f1-input/drivers.json').read_text())
keys=list(data['teams'])
header=(root/'src/f1assets.h').read_text()
values=list(map(int,re.search(r'f1_logos\[.*?= \{(.*?)\};',header,re.S)[1].split(',')))
teams=[]
for i,key in enumerate(keys):
    t=data['teams'][key]
    im=Image.frombytes('RGBA',(16,12),bytes(values[i*768:(i+1)*768]))
    b=io.BytesIO();im.save(b,format='PNG')
    teams.append(dict(name=t['name'].upper(),color=t['color'],mode='builtin',file='',pixels=None,
                      builtin='data:image/png;base64,'+base64.b64encode(b.getvalue()).decode()))
drivers=[dict(id=d['car_id'],number=d['number'],first=d.get('first',''),last=d['last'],abbr=d['abbr'],team=keys.index(d['team'])) for d in data['drivers']]
initial=dict(controls=dict(hideCaption=False,hideRetirement=False,fuelEnabled=False,fuelMilliseconds=3000,fuelTargets=[],theme=0),season=data.get('season','CUSTOM'),base=(root/'release/GP2LAP.CFG').read_text(),teams=teams,drivers=drivers)
html=(root/'tools/editor.html').read_text().replace('__INITIAL__',json.dumps(initial).replace('</','<\\/'))
html=html.replace('__EXE_IMPORT__',(root/'tools/exe_import.js').read_text())
(root/'release/EDITOR.html').write_text(html)
print('Generated release/EDITOR.html')
