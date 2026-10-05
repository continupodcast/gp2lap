const fs=require('fs'),vm=require('vm'),assert=require('assert'),path=require('path');
const root=path.resolve(__dirname,'..');
const html=fs.readFileSync(path.join(root,'release/EDITOR.html'),'utf8');
const script=html.match(/<script>([\s\S]*?)<\/script>/)[1];
const ctx=vm.createContext({structuredClone,TextEncoder,TextDecoder,Blob,Uint8Array});
vm.runInContext(script.slice(0,script.indexOf("$('export').onclick")),ctx);
const run=s=>vm.runInContext(s,ctx);
const {fixture}=require('./editor_fixture.cjs');
for(const teams of [13,14])for(const flags of [0,64,128,192]) {
 const f=fixture(teams);ctx.bytes=f.bytes;const normal=run('JSON.stringify(readGP2(bytes))');
 for(let i=0;i<teams*2;i++)f.bytes[f.idsFile+i]|=flags;
 assert.equal(run('JSON.stringify(readGP2(bytes))'),normal);
 run('var result=readGP2(bytes);var imported=importedState(state,result);validate(imported)');
}
for(const raw of [0,41,63,64,105,127,128,169,191,192,233,255]) {
 const f=fixture(13);f.bytes[f.idsFile]=raw;ctx.bytes=f.bytes;
 assert.throws(()=>run('readGP2(bytes)'));
}
for(const flags of [0,64,128,192]) {
 const f=fixture(13);f.bytes[f.idsFile+1]=f.bytes[f.idsFile]|flags;ctx.bytes=f.bytes;
 assert.throws(()=>run('readGP2(bytes)'));
}
const exepath=process.argv[2];if(exepath){
 ctx.bytes=new Uint8Array(fs.readFileSync(exepath));
 run('var real=readGP2(bytes);var imported=importedState(state,real);validate(imported);state=imported;validate(parseCFG(cfg()))');
 assert.equal(run('real.length'),13);
 assert.equal(run('real[6].drivers[1].id'),34);
 assert.equal(run('real[6].drivers[1].name'),'Franco COLAPINTO [43]');
 assert.equal(run('imported.drivers.find(d=>d.id===34).number'),43);
 console.log('PASS actual GP2(6).EXE: 13 teams, 26 drivers, Colapinto CarId 34 / number 43, CFG roundtrip.');
}
console.log('PASS player flags 0x40/0x80/0xC0, 13/14 teams, invalid IDs and normalized duplicates rejected.');
