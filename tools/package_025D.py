from pathlib import Path
import zipfile
root=Path(__file__).resolve().parents[1]
target=root.parent/'GP2Lap-F1-HUD-v0.25D-Evaluation.zip'
with zipfile.ZipFile(target,'w',zipfile.ZIP_DEFLATED) as z:
    for src,dst in [('out/gp2lap.exe','GP2LAP.EXE'),('release/GP2LAP.CFG','GP2LAP.CFG'),('release/EDITOR.html','EDITOR.html'),('release/README-025D.txt','README.txt')]:
        z.write(root/src,dst)
    for file in root.rglob('*'):
        rel=file.relative_to(root)
        if file.is_file() and not any(p in {'.git','out','pub','release','__pycache__','validation'} for p in rel.parts):
            z.write(file,'source/'+rel.as_posix())
    for name in ['GP2LAP.CFG','README-025D.txt']:
        z.write(root/'release'/name,'source/release/'+name)
    for name in ['build-025D.log','test-025D.log']:
        z.write(root/'validation'/name,'validation/'+name)
with zipfile.ZipFile(target) as z:
    assert z.testzip() is None
    assert b'F1 HUD 0.25D' in z.read('GP2LAP.EXE')
    assert z.read('GP2LAP.EXE')==(root/'out/gp2lap.exe').read_bytes()
print(target)
