#!/usr/bin/env python3
"""Compose additive GP2Lap build modules, resolve dependencies and clean-build DOS."""
import argparse, hashlib, json, os, pathlib, shutil, subprocess, sys
ROOT=pathlib.Path(__file__).resolve().parents[1]
CATALOG=ROOT/'modules/catalog.json'
def resolve(requested,catalog=None):
    catalog=catalog or json.loads(CATALOG.read_text());done=set();visiting=set()
    def visit(name):
        if name not in catalog:raise ValueError('Unknown module: '+name)
        if name in visiting:raise ValueError('Cyclic module dependency: '+name)
        if name in done:return
        visiting.add(name)
        for dep in catalog[name]['requires']:visit(dep)
        visiting.remove(name);done.add(name)
    for name in requested:visit(name)
    return sorted(done)
def selected():return sorted(p.stem for p in (ROOT/'modules/enabled').glob('*.module'))
def configure(requested=None):
    catalog=json.loads(CATALOG.read_text());requested=selected() if requested is None else requested
    enabled=resolve(requested,catalog)
    for name in enabled:
        if not (ROOT/'modules'/name/'module.json').exists():
            raise ValueError('Module sources unavailable: '+name+'. Merge its branch and dependencies first.')
    lines=['/* Generated; do not edit. */','#ifndef F1MODULES_GENERATED_H','#define F1MODULES_GENERATED_H']
    lines += ['#define F1_MODULE_'+name.upper().replace('-','_')+' '+str(int(name in enabled)) for name in sorted(catalog)]
    lines+=['#endif','']
    (ROOT/'src/f1modules.generated.h').write_text('\n'.join(lines))
    manifest={'requested':sorted(requested),'resolved':enabled,'base':'v0.8','format':1}
    (ROOT/'out').mkdir(exist_ok=True)
    (ROOT/'out/modules.json').write_text(json.dumps(manifest,indent=2)+'\n')
    return manifest
def build(requested=None,destination=None):
    m=configure(requested)
    # Watcom's historical makefiles do not track included headers. Never reuse objects.
    for p in list((ROOT/'out').iterdir()):
        if p.suffix.lower() in ('.obj','.err','.exe'):p.unlink()
    env=os.environ.copy()
    if 'WATCOM' in env:
        base=pathlib.Path(env['WATCOM']);env['PATH']=os.pathsep.join(str(base/name) for name in (['binnt64','binnt'] if os.name=='nt' else ['binl64','binl']))+os.pathsep+env['PATH'];env.setdefault('INCLUDE',str(base/'h'))
    if not shutil.which('wmake',path=env['PATH']):raise ValueError('Open Watcom wmake not found; set WATCOM to the compiler directory.')
    subprocess.run(['wmake','-a','-f','makefile.lin','out/gp2lap.exe'],cwd=ROOT,env=env,check=True)
    for response in ROOT.glob('wm*.tmp'):
        response.unlink()
    exe=ROOT/'out/gp2lap.exe'
    m.update(executable_sha256=hashlib.sha256(exe.read_bytes()).hexdigest(),executable_bytes=exe.stat().st_size)
    (ROOT/'out/modules.json').write_text(json.dumps(m,indent=2)+'\n')
    if destination:
        d=pathlib.Path(destination).resolve();d.mkdir(parents=True,exist_ok=True)
        shutil.copy2(exe,d/'GP2LAP.EXE');shutil.copy2(ROOT/'out/modules.json',d/'modules.json')
        for name in ['GP2LAP.CFG']:
            if (ROOT/'release'/name).exists():shutil.copy2(ROOT/'release'/name,d/name)
        # Never silently ship a configuration which enables absent gameplay modules.
        cfg=d/'GP2LAP.CFG'
        if cfg.exists():
            import re
            text=cfg.read_text(encoding='latin1')
            controls={'tyre-compounds':['PerCarTyreCompounds','SoftTyresInQuali','TyreDiagnostics','TyreTestCompound','TyreTestFuelLaps'],
              'race-strategy-engine':['StrategyAutoEnabled','StrategyDistributionEnabled','StrategyTestEnabled','StrategyRepairTestEnabled'],
              'player-race-strategy':['StrategyPlayerEnabled'],'pit-compound-selector':['StrategyPlayerPitChoice'],'camera-controls':['EnhancedCameras']}
            for module,keys in controls.items():
                if module not in m['resolved']:
                    for key in keys:text=re.sub(r'(?im)^(\s*'+key+r'\s*=\s*)[^;\r\n]*',r'\g<1>0 ',text)
            cfg.write_text(text,encoding='latin1')
        if 'hud-editor' in m['resolved']:shutil.copy2(ROOT/'modules/hud-editor/EDITOR.html',d/'EDITOR.html')
        for name in ['LICENSE.md','CREDITS-v0.8.md']:
            if (ROOT/name).exists():shutil.copy2(ROOT/name,d/name)
    return m
def main():
    a=argparse.ArgumentParser(description=__doc__);a.add_argument('command',choices=['list','configure','build']);a.add_argument('--modules',help='Explicit comma-separated override; empty string means core');a.add_argument('--output');args=a.parse_args()
    try:
        if args.command=='list':print(json.dumps(json.loads(CATALOG.read_text()),indent=2));return
        names=None if args.modules is None else [s for s in args.modules.split(',') if s]
        m=build(names,args.output) if args.command=='build' else configure(names)
        print(json.dumps(m,indent=2))
    except (ValueError,subprocess.CalledProcessError) as e:print(str(e),file=sys.stderr);sys.exit(1)
if __name__=='__main__':main()
