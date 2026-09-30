from pathlib import Path
import zipfile

root=Path(__file__).resolve().parents[1]
target=root.parent/'GP2Lap-F1-HUD-v0.14.zip'
with zipfile.ZipFile(target,'w',zipfile.ZIP_DEFLATED) as z:
    for src,dst in [('out/gp2lap.exe','GP2LAP.EXE'),('release/EDITOR.html','EDITOR.html'),('release/GP2LAP.CFG','GP2LAP.CFG'),('release/README-014.txt','README.txt')]:
        z.write(root/src,dst)
    for file in root.rglob('*'):
        rel=file.relative_to(root)
        if not file.is_file() or any(p in {'.git','out','pub','release','__pycache__','validation'} for p in rel.parts): continue
        z.write(file,'source/'+rel.as_posix())
    z.write(root/'release/GP2LAP.CFG','source/release/GP2LAP.CFG')
    z.write(root/'release/README-014.txt','source/release/README-014.txt')
with zipfile.ZipFile(target) as z:
    assert z.testzip() is None
    assert z.read('GP2LAP.EXE')[:2]==b'MZ'
    assert b'F1 HUD 0.14' in z.read('GP2LAP.EXE')
print(target)
