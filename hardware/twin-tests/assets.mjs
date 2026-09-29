import fs from 'node:fs';
import crypto from 'node:crypto';
import assert from 'node:assert/strict';
const base='control_ui/twin/assets/';
const sources=JSON.parse(fs.readFileSync(base+'manifest.json')).parts;
const pieces=JSON.parse(fs.readFileSync(base+'components.json'));
assert.equal(pieces.length,76);
for(const source of sources){
 const original=fs.readFileSync(base+source.file);
 assert.equal(crypto.createHash('sha256').update(original).digest('hex'),source.sha256);
 const group=pieces.filter(p=>p.parent===source.file);
 assert.equal(group.reduce((n,p)=>n+p.triangles,0),source.triangles);
 const triangles=new Map();
 for(let i=84;i<original.length;i+=50){const key=original.subarray(i,i+50).toString('hex');triangles.set(key,(triangles.get(key)||0)+1);}
 for(const p of group){
  const b=fs.readFileSync(base+p.file);
  assert.equal(crypto.createHash('sha256').update(b).digest('hex'),p.sha256);
  for(let i=84;i<b.length;i+=50){const key=b.subarray(i,i+50).toString('hex');assert(triangles.get(key)>0);triangles.set(key,triangles.get(key)-1);}
 }
 assert([...triangles.values()].every(n=>n===0));
}
console.log('PASS: 33 unchanged sources, 76 components, every source triangle retained exactly once.');
