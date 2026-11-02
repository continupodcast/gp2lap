#!/usr/bin/env python3
"""Host regression suite for extracted bodies. Does not emulate GP2."""
import importlib.util,json,pathlib,subprocess,tempfile
ROOT=pathlib.Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('modules',ROOT/'tools/modules.py');m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
def run():
    names=list(json.loads(m.CATALOG.read_text()));m.configure(names)
    expected={'tyre-compounds','race-strategy-engine','player-race-strategy','pit-compound-selector'}
    assert set(m.resolve(['pit-compound-selector']))==expected
    assert m.resolve(['hud-editor'])==['hud-editor']
    for bad in ['missing','../tyre-compounds']:
        try:m.resolve([bad]);raise AssertionError('invalid selection accepted')
        except ValueError:pass
    try:m.resolve(['a'],{'a':{'requires':['b']},'b':{'requires':['a']}});raise AssertionError('cycle accepted')
    except ValueError:pass
    tests=['test_compound_state_v08','test_compound_rule_t20_7','test_modal_t20','test_inlinepit_t19_3','test_strategy_nav_t20_4','test_strategy_edit_t20','test_quickgrid','test_player_palette_t14_1','test_player_shuffle_t14_2','test_distribution_t13','test_gameplay_modes']
    results=[]
    with tempfile.TemporaryDirectory() as t:
        for test in tests:
            exe=pathlib.Path(t)/test
            p=subprocess.run(['gcc','-std=c99','-O2','-w','-include',str(ROOT/'src/f1modules.h'),str(ROOT/'tests/modularity'/(test+'.c')),'-lm','-o',str(exe)],capture_output=True,text=True)
            if p.returncode:raise RuntimeError(test+' build failed\n'+p.stderr)
            p=subprocess.run([str(exe)],cwd=t,capture_output=True,text=True)
            if p.returncode:raise RuntimeError(test+' failed\n'+p.stdout+p.stderr)
            results.append({'test':test,'status':'PASS','output':p.stdout.strip()})
    p=subprocess.run(['node',str(ROOT/'tests/modularity/test_editor_t20_7.cjs')],capture_output=True,text=True)
    if p.returncode:raise RuntimeError(p.stdout+p.stderr)
    results.append({'test':'editor CFG round-trip','status':'PASS','output':p.stdout.strip()})
    results.append({'test':'dependency validation, closure and cycles','status':'PASS'})
    # Test actual renderer entry points under the empty profile.
    m.configure([])
    with tempfile.TemporaryDirectory() as t:
        exe=pathlib.Path(t)/'disabled-renderer'
        subprocess.run(['gcc','-std=c99','-O2','-w','-ffunction-sections','-fdata-sections',str(ROOT/'tests/modularity/test_disabled_renderer.c'),str(ROOT/'src/f1render.c'),'-Wl,--gc-sections','-lm','-o',str(exe)],check=True)
        subprocess.run([str(exe)],cwd=t,check=True)
    results.append({'test':'omitted render modules: null input and unchanged framebuffer','status':'PASS'})
    m.configure(names)
    print(json.dumps(results,indent=2))
if __name__=='__main__':run()
