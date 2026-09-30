/* Synthetic LE data container, not a GP2 executable. No game bytes needed. */
exports.fixture=function(teamCount) {
 const bytes=new Uint8Array(4096+145*4096),v=new DataView(bytes.buffer);
 const u=(p,n)=>v.setUint32(p,n,true),h=64,obj=256,pages=512,data=4096;
 bytes.set([77,90],0);u(60,h);bytes.set([76,69],h);
 u(h+40,4096);u(h+68,3);u(h+64,obj-h);u(h+72,pages-h);u(h+128,data);
 for(const [index,size,first,count] of [[0,128*4096,1,128],[1,4096,129,1],[2,16*4096,130,16]]) {
  u(obj+index*24,size);u(obj+index*24+4,0);u(obj+index*24+12,first);u(obj+index*24+16,count);
 }
 for(let i=0;i<145;i++){const n=i+1;bytes[pages+i*4]=(n>>16)&255;bytes[pages+i*4+1]=(n>>8)&255;bytes[pages+i*4+2]=n&255;}
 for(const [addr,opcode,offset] of [[0x14fd3,0xbe,0x1000],[0x7be4b,0xb8,0x2000],[0x14ec3,0xbf,0x3000]]) {
  bytes[data+addr-1]=opcode;u(data+addr,offset);
 }
 const base=data+129*4096;
 const str=(offset,s)=>bytes.set(Buffer.from(s+'\0','ascii'),base+offset);
 for(let id=1;id<=40;id++)str(0x1000+(id-1)*24,'Test DRIVER'+id+' ['+id+']');
 const ids=Array.from({length:28},(_,i)=>i+1).filter(x=>x!==18&&x!==21).concat([18,21]);
 for(let team=0;team<teamCount;team++) {
  str(0x2000+team*13,team===13?'Pacific':'Team '+(team+1));
  bytes[base+0x3000+team*2]=ids[team*2];bytes[base+0x3001+team*2]=ids[team*2+1];
 }
 return {bytes,idsFile:base+0x3000};
};
