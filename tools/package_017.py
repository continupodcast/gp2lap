from pathlib import Path
import zipfile

root=Path(__file__).resolve().parents[1]
target=root.parent/'GP2Lap-F1-HUD-v0.17.zip'
with zipfile.ZipFile(target,'w',zipfile.ZIP_DEFLATED) as z:
    for src,dst in [('out/gp2lap.exe','GP2LAP.EXE'),('release/EDITOR.html','EDITOR.html'),('release/GP2LAP.CFG','GP2LAP.CFG'),('release/README-017.txt','README.txt')]:
        z.write(root/src,dst)
    for file in root.rglob('*'):
        rel=file.relative_to(root)
        if file.name in {'f1neutral.c','f1neutral.h'} or (file.name.startswith('wm') and file.suffix=='.tmp'): continue
        if not file.is_file() or any(p in {'.git','out','pub','release','__pycache__','validation'} for p in rel.parts): continue
        z.write(file,'source/'+rel.as_posix())
    z.write(root/'release/GP2LAP.CFG','source/release/GP2LAP.CFG')
    z.write(root/'release/README-017.txt','source/release/README-017.txt')
with zipfile.ZipFile(target) as z:
    assert z.testzip() is None
    assert z.read('GP2LAP.EXE')[:2]==b'MZ'
    assert b'F1 HUD 0.17' in z.read('GP2LAP.EXE')
print(target)
