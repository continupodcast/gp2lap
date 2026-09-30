const fs=require('fs'),vm=require('vm'),assert=require('assert'),path=require('path');
const root=path.resolve(__dirname,'..'),html=fs.readFileSync(path.join(root,'release/EDITOR.html'),'utf8'),script=html.match(/<script>([\s\S]*?)<\/script>/)[1];
new vm.Script(script);
const context=vm.createContext({structuredClone,TextEncoder,TextDecoder,Blob,Uint8Array});
vm.runInContext(script.slice(0,script.indexOf("$('export').onclick")),context);
const run=s=>vm.runInContext(s,context);
run('state.controls={hideCaption:false,fuelEnabled:true,fuelMilliseconds:1500,fuelTargets:[10,34]};validate(state);var roundtrip=parseCFG(cfg());validate(roundtrip)');
assert.equal(run('roundtrip.controls.fuelMilliseconds'),1500);
assert.equal(run('JSON.stringify(roundtrip.controls.fuelTargets)'),'[10,34]');
assert.equal(run('roundtrip.controls.hideCaption'),false);
assert.equal(run('roundtrip.controls.fuelEnabled'),true);
assert.equal(run('roundtrip.controls.hideRetirement'),false);
run('state.controls.hideRetirement=true;state.controls.hideCaption=true;roundtrip=parseCFG(cfg());validate(roundtrip)');
assert.equal(run('roundtrip.controls.hideCaption'),false);
assert.equal(run('roundtrip.controls.hideRetirement'),false);
run('delete state.controls;validate(state)');assert.equal(run('state.controls.fuelEnabled'),false);
for(const test of ['state.controls.fuelTargets=[0]','state.controls.fuelTargets=[41]','state.controls.fuelTargets=[10,10]','state.controls.fuelMilliseconds=99','state.controls.fuelMilliseconds=30001']){
 run('state.controls=structuredClone(INITIAL.controls)');run(test);assert.throws(()=>run('validate(state)'));
}
console.log('PASS: fuel controls CFG roundtrip, legacy projects default to disabled, malformed controls rejected.');
