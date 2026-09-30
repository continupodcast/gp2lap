/* GP2's LE objects and data references, as used by WWPatchDataHooks.
   No name searching: CarId is the 1-based index in the 40-entry name table. */
function readGP2(bytes) {
 const v=new DataView(bytes.buffer,bytes.byteOffset,bytes.byteLength);
 const fail=()=>{throw Error('Unsupported or damaged GP2.EXE. No changes were applied.');};
 function u32(p){if(p<0||p+4>bytes.length)fail();return v.getUint32(p,true);}
 let mz=-1,h=-1;
 for(let p=0;p<Math.min(bytes.length-64,1048576);p++) {
  if(bytes[p]!==77||bytes[p+1]!==90)continue;
  const q=p+u32(p+60);
  if(q+196<=bytes.length&&bytes[q]===76&&bytes[q+1]===69&&bytes[q+2]===0&&bytes[q+3]===0){mz=p;h=q;break;}
 }
 if(h<0||u32(h+40)!==4096||u32(h+68)!==3)fail();
 const obj=h+u32(h+64),pages=h+u32(h+72),data=mz+u32(h+128);
 function objectByte(index,offset){
  const o=obj+index*24,size=u32(o),first=u32(o+12),pageCount=u32(o+16);
  if(offset<0||offset>=size||Math.floor(offset/4096)>=pageCount)fail();
  const p=pages+(first-1+Math.floor(offset/4096))*4;
  if(p<0||p+4>bytes.length||bytes[p+3]!==0)fail();
  const page=bytes[p]*65536+bytes[p+1]*256+bytes[p+2];
  const file=data+(page-1)*4096+offset%4096;
  if(page<1||file<0||file>=bytes.length)fail();return bytes[file];
 }
 function reference(address,opcode){
  const offset=address-u32(obj+4);
  if(objectByte(0,offset-1)!==opcode)fail();
  let n=0;for(let i=0;i<4;i++)n+=objectByte(0,offset+i)*2**(8*i);return n;
 }
 const names=reference(0x14fd3,0xbe),teams=reference(0x7be4b,0xb8),ids=reference(0x14ec3,0xbf);
 // These are offsets in GP2's data object, not flat file offsets.
 function stringAt(offset,len){
  let s='';const high='ÇüéâäàåçêëèïîìÄÅÉæÆôöòûùÿÖÜ¢£¥₧ƒáíóúñÑªº¿⌐¬½¼¡«»';
  for(let i=0;i<len;i++){const c=objectByte(2,offset+i);if(!c)return s.trim();if(c<32)fail();s+=c<127?String.fromCharCode(c):(high[c-128]||'?');}
  fail();
 }
 const seen=new Set(),result=[];
 for(let team=0;team<14;team++) {
  // The optional fourteenth pair is zero-filled in the 13-team executable.
  if(team===13 && objectByte(2,ids+26)===0 && objectByte(2,ids+27)===0)break;
  const name=stringAt(teams+team*13,13);if(!name)fail();const drivers=[];
  for(let k=0;k<2;k++) {
   const id=objectByte(2,ids+team*2+k);if(id<1||id>40||seen.has(id))fail();seen.add(id);
   const name=stringAt(names+(id-1)*24,24);if(!name)fail();drivers.push({id,name});
  }
  result.push({name,drivers});
 }
 return result;
}
function emptyTeam(){return {name:'TEAM 14',color:'#ffffff',mode:'color',file:'',pixels:null,builtin:''};}
function importedState(current,roster){
 const next=structuredClone(current),used=new Set(),drivers=[];
 while(next.teams.length<roster.length)next.teams.push(emptyTeam());
 for(const team of roster){
  // Keep artwork with the team identified by its existing drivers.
  let slot=-1,score=-1;
  next.teams.forEach((t,i)=>{if(used.has(i))return;const n=current.drivers.filter(d=>d.team===i&&team.drivers.some(r=>r.id===d.id)).length;if(n>score){score=n;slot=i;}});
  used.add(slot);next.teams[slot].name=ascii(team.name);
  for(const r of team.drivers){
   const old=current.drivers.find(d=>d.id===r.id),m=r.name.match(/\s*[\[(](\d{1,2})[\])]\s*$/),clean=r.name.replace(/\s*[\[(]\d{1,2}[\])]\s*$/,'').trim();
   const words=clean.split(/\s+/);let split=words.findIndex((s,i)=>i>0&&s===s.toUpperCase());if(split<0)split=Math.max(0,words.length-1);
   const first=ascii(words.slice(0,split).join(' ')),last=ascii(words.slice(split).join(' '));
   drivers.push({id:r.id,first,last,abbr:old&&old.last.toUpperCase()===last.toUpperCase()?old.abbr:last.replace(/[^a-z]/gi,'').slice(0,3).toUpperCase(),number:m?+m[1]:(old?.number??r.id),team:slot});
  }
 }
 next.drivers=drivers;return next;
}
