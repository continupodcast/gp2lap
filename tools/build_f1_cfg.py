"""Create an extended GP2LAP.CFG from the supplied base configuration."""
import json
from pathlib import Path

root=Path(__file__).resolve().parents[1]
data=json.loads((root/'f1-input/drivers.json').read_text())
base=Path('/workspace/scratch/756a2f984016/upload/GP2LAP.CFG').read_text()
marks={'redbull':'RB','mclaren':'MC','ferrari':'FE','mercedes':'ME','astonmartin':'AM',
       'racingbulls':'VC','kicksauber':'SA','alpine':'AL','williams':'WI','haas':'HA',
       'cadillac':'CA','hondagulfnissan':'GN','porschepeugeot':'PP'}
lines=[base.rstrip(),'','; -----------------------------------------------------------------------------',
       '; F1 HUD custom data. It is read once when GP2LAP starts.',
       '; Set F1UseCustomData to 0 to use the compiled default data.',
       '; TeamLogo 1 uses the built-in emblem. TeamLogo 0 shows only the team colour.',
       '[F1 HUD]','F1UseCustomData = 1','','[F1 Teams]']
teams=list(data['teams'])
for i,key in enumerate(teams,1):
    t=data['teams'][key]
    lines += [f'Team{i:02d}Name = "{t["name"].upper()}"',f'Team{i:02d}Color = "{t["color"]}"',
              f'Team{i:02d}Mark = "{marks.get(key,"TM")}"',f'Team{i:02d}Logo = 1','']
lines += ['[F1 Drivers]','; CarId is GP2 internal car id (1 to 40). It must be unique. Team is 1 to 13.']
for i,d in enumerate(data['drivers'],1):
    lines += [f'Driver{i:02d}CarId = {d["car_id"]}',f'Driver{i:02d}First = "{d.get("first", "")}"',
              f'Driver{i:02d}Last = "{d["last"]}"',f'Driver{i:02d}Abbr = "{d["abbr"]}"',
              f'Driver{i:02d}Number = {d["number"]}',f'Driver{i:02d}Team = {teams.index(d["team"])+1}','']
(root/'release').mkdir(exist_ok=True)
(root/'release/GP2LAP.CFG').write_text('\n'.join(lines)+'\n')
print('Generated release/GP2LAP.CFG')
