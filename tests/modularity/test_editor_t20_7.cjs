const fs=require('fs'),vm=require('vm'),assert=require('assert'),path=require('path');
const html=fs.readFileSync(path.join(__dirname,'../../modules/hud-editor/EDITOR.html'),'utf8');
const code=html.match(/<script>([\s\S]*?)<\/script>/)[1],nodes=new Map();
class Node{
 constructor(){this.children=[];this.style={setProperty(k,v){this[k]=v}};this.dataset={};this.value='';this.checked=false;this.disabled=false;this.attributes={};this.classList={values:new Set(),add(x){this.values.add(x)},remove(x){this.values.delete(x)},toggle(x,on){if(on)this.values.add(x);else this.values.delete(x)}};}
 append(...x){this.children.push(...x)}replaceChildren(...x){this.children=x}add(x){this.children.push(x)}setAttribute(k,v){this.attributes[k]=v}addEventListener(){}focus(){}
 getContext(){return {clearRect(){},fillRect(){},fillText(){},drawImage(){}}}
}
const context={structuredClone,TextEncoder,TextDecoder,Uint8Array,DataView,ArrayBuffer,Blob,console,setTimeout,URL,Image:class {},Option:class{constructor(text,value){this.textContent=text;this.value=value}},document:{getElementById(id){if(!nodes.has(id))nodes.set(id,new Node());return nodes.get(id)},createElement(){return new Node()}},window:{addEventListener(){}}};
vm.createContext(context);vm.runInContext(code,context); // Actual startup render and all event bindings.
vm.runInContext(`
function check(v,msg){if(!v)throw Error(msg)}
validate(state);check(!state.gameMods.tyres,'default master off');
check($('strategyAuto').disabled,'dependencies off');
$('tyresEnabled').checked=true;$('tyresEnabled').onchange();
for(const [id,key] of Object.entries(STRATEGY_CONTROLS)){check($(id).checked,'initial '+id);check(!$(id).disabled,'enabled '+id)}
$('strategySeed').value='321';$('strategySeed').onchange();
const enabled=cfg();
for(const param of ['PerCarTyreCompounds','StrategyAutoEnabled','StrategyDistributionEnabled','StrategyPlayerEnabled','StrategyPlayerPitChoice','ShowTyreCompoundBadge'])check(enabled.includes(param+' = 1'),'export '+param);
for(const param of ['StrategyDiagnostics','TyreDiagnostics','StrategyTestEnabled','StrategyRepairTestEnabled','TyreTestFuelLaps','TyreTestCompound'])check(enabled.includes(param+' = 0'),'remove '+param);
check(enabled.includes('StrategyAutoSeed = 321'),'seed export');
const round=parseCFG(enabled);validate(round);for(const key of Object.keys(state.gameMods))check(round.gameMods[key]===state.gameMods[key],'roundtrip '+key);
for(const param of Object.values(STRATEGY_FIELDS))check((enabled.match(new RegExp('^'+param+' =','gm'))||[]).length===1,'duplicates '+param);
$('strategyAuto').checked=false;$('strategyAuto').onchange();check($('strategyDistribution').disabled,'AI dependency');check(cfg().includes('StrategyDistributionEnabled = 0'),'AI gated');
$('strategyPlayer').checked=false;$('strategyPlayer').onchange();check($('strategyPitChoice').disabled,'pit dependency');check(cfg().includes('StrategyPlayerPitChoice = 0'),'pit gated');
$('tyresEnabled').checked=false;$('tyresEnabled').onchange();
for(const p of ['PerCarTyreCompounds','StrategyAutoEnabled','StrategyDistributionEnabled','StrategyPlayerEnabled','StrategyPlayerPitChoice','ShowTyreCompoundBadge'])check(cfg().includes(p+' = 0'),'master gate '+p);
// Saved projects retain independent preferences even while master is off.
const project=JSON.parse(JSON.stringify(state));validate(project);check(project.gameMods.pitChoice,'retained preference');
const legacy=structuredClone(INITIAL);legacy.gameMods={tyres:true,tyreDiagnostics:true,testCompound:3,softQuali:false,cameras:true};validate(legacy);check(!('testCompound'in legacy.gameMods)&&!('tyreDiagnostics'in legacy.gameMods),'legacy clean');check(!legacy.gameMods.autoStrategies&&!legacy.gameMods.playerStrategies,'legacy opt in');
state=structuredClone(INITIAL);render();showEditorTab('grid');check(!$('gridPanel').classList.values.has('hidden'),'grid tab');check($('rosterMain').classList.values.has('hidden'),'roster hidden');
const original=state.drivers.map(d=>d.id);check($('qGridList').children.length===26,'grid 26');
let row=$('qGridList').children[0];const team=state.teams[state.drivers[0].team];check(row.style['--team']===team.color,'team colour');check(row.children[2].children[1].textContent===team.name,'team name');
row.children[3].children[1].onclick();check(state.quickGrid.order[1]===original[0],'arrow move');
row=$('qGridList').children[1];row.children[0].value='5';row.children[0].onchange();check(state.quickGrid.order[4]===original[0],'position move');
row=$('qGridList').children[4];row.children[0].value='0';row.children[0].onchange();check(state.quickGrid.order[4]===original[0],'reject invalid position');
const drag=state.quickGrid.order[2],data={setData(k,v){this.value=v},getData(){return this.value}};
$('qGridList').children[2].ondragstart({dataTransfer:data});$('qGridList').children[0].ondrop({dataTransfer:data,preventDefault(){}});check(state.quickGrid.order[0]===drag,'drag order');
$('qGridSearch').value='COLAPINTO';$('qGridSearch').oninput();check($('qGridList').children.length===1&&$('qGridList').children[0].children[2].children[0].textContent.includes('COLAPINTO'),'search driver');
$('qGridSearch').value='no-matches';renderQuickGrid();check($('qGridList').children[0].textContent==='No matching drivers.','empty state');
$('qGridSearch').value='';renderQuickGrid();$('qGridEnabled').checked=true;$('qGridEnabled').onchange();
const gridParsed=parseCFG(cfg());validate(gridParsed);check(gridParsed.quickGrid.enabled&&JSON.stringify(gridParsed.quickGrid.order)===JSON.stringify(state.quickGrid.order),'grid roundtrip');
$('qGridReset').onclick();check(JSON.stringify(original)===JSON.stringify(state.quickGrid.order),'reset order');
for(const tab of ['game','teams','drivers','grid'])showEditorTab(tab);
// 14 teams and 28 drivers remain supported.
const expanded=structuredClone(state);expanded.teams.push(emptyTeam());expanded.drivers.push({id:1,number:1,first:'A',last:'TEST',abbr:'TES',team:13},{id:2,number:2,first:'B',last:'TEST',abbr:'TST',team:13});validate(expanded);state=expanded;render();check($('qGridList').children.length===28,'grid 28');
`,context);
const fixture=fs.readFileSync(path.join(__dirname,'legacy-strategy-fixture.cfg'),'utf8');context.fixture=fixture;
vm.runInContext(`
state=parseCFG(fixture);validate(state);renderGameMods();
for(const id of ['tyresEnabled','strategyAuto','strategyDistribution','strategyPlayer','strategyPitChoice'])check($(id).checked,'import current flag '+id);
for(const param of ['TyreDiagnostics','StrategyDiagnostics','TyreTestCompound','TyreTestFuelLaps'])check(cfg().includes(param+' = 0'),'import removes tests '+param);
// Inspect the editor's actual generated ZIP, not a reconstructed serializer.
globalThis.archive=zip([['GP2LAP.CFG',enc.encode(cfg())]]);
const bad=structuredClone(state);bad.gameMods.autoSeed=-1;let rejected=false;try{validate(bad)}catch(e){rejected=true}check(rejected,'negative seed rejected');
`,context);
(async()=>{fs.writeFileSync('/tmp/t15-editor-export.zip',Buffer.from(await context.archive.arrayBuffer()));console.log('PASS T15 actual JS + DOM harness: full startup, switches/dependencies, strategy CFG roundtrip, legacy migration/tests disabled, project preferences, team colours, grid arrows/direct/invalid/drag/search/reset, 14-team/28-driver compatibility, native ZIP serializer, seed validation');})();
vm.runInContext(`
state.gameMods.tyres=true;render();
check(state.gameMods.requireCompoundChange===true,'legacy/default compound rule on');
$('strategyCompoundRule').checked=false;$('strategyCompoundRule').onchange();
check(state.gameMods.requireCompoundChange===false,'checkbox disables rule');
check(cfg().includes('StrategyRequireCompoundChange = 0'),'export off');
const ruleText=cfg();state=parseCFG(ruleText);render();
check(state.gameMods.requireCompoundChange===false,'CFG roundtrip off');
check(!$('strategyCompoundRule').checked,'import checkbox off');
$('strategyCompoundRule').checked=true;$('strategyCompoundRule').onchange();
check(cfg().includes('StrategyRequireCompoundChange = 1'),'export on');
`,context);
console.log('PASS compound-rule checkbox, default compatibility and CFG roundtrip');

vm.runInContext(`
const legacyDefaults='[Textures]\\nJamTextureExtraMB = 0\\n[Track]\\nExtraTrackSegments = 0\\nExtraTrackFileKB = 0\\n[Pit]\\nPitSpeedKmh = 0\\n[AI]\\nAILowHpLaunchFix = 0\\nAILaunchFadeKmh = 128\\nAIBankingRoll = 0\\n[Season]\\n;SeasonOverrides = "override.cfg"\\n[Rcr]\\n;SvgaRcr1 = 9999000\\n;SvgaRcr2 = 9999000\\n;SvgaRcr3 = 9999000\\n';
state.base += '\\n'+legacyDefaults;const exportedClean=cfg();
for(const key of ['JamTextureExtraMB','ExtraTrackSegments','ExtraTrackFileKB','PitSpeedKmh','AILowHpLaunchFix','AILaunchFadeKmh','AIBankingRoll','SeasonOverrides'])check(!exportedClean.includes(key),'removed '+key);
for(let n=1;n<=3;n++)check(exportedClean.includes('SvgaRcr'+n+' = 9999000'),'active RCR '+n);
check(cfg()===exportedClean,'stable repeated export');
`,context);
console.log('PASS actual editor export cleans imported legacy defaults and enables SVGA RCR');
