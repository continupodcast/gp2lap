from pathlib import Path
import zipfile

root = Path(__file__).resolve().parents[1]
target = root.parent / 'GP2Lap-F1-HUD-v0.24-Complete.zip'
with zipfile.ZipFile(target, 'w', zipfile.ZIP_DEFLATED) as z:
    z.write(root / 'out/gp2lap.exe', 'GP2LAP.EXE')
    z.write(root / 'release/GP2LAP.CFG', 'GP2LAP.CFG')
    z.write(root / 'release/EDITOR.html', 'EDITOR.html')
    z.write(root / 'release/README-024-Tower.txt', 'README.txt')
    for file in root.rglob('*'):
        rel = file.relative_to(root)
        if not file.is_file() or any(p in {'.git', 'out', 'pub', 'release', '__pycache__', 'validation'} for p in rel.parts):
            continue
        z.write(file, 'source/' + rel.as_posix())
    z.write(root / 'release/README-024-Tower.txt', 'source/release/README-024-Tower.txt')
    for name in ['build-tower024.log', 'test-tower024.log']:
        z.write(root / 'validation' / name, 'validation/' + name)
with zipfile.ZipFile(target) as z:
    assert z.testzip() is None
    assert z.read('GP2LAP.EXE')[:2] == b'MZ'
    assert b'F1 HUD 0.24 Tower' in z.read('GP2LAP.EXE')
print(target)
