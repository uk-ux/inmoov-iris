import * as THREE from 'three';
import {OrbitControls} from 'three/addons/controls/OrbitControls.js';
import {STLLoader} from 'three/addons/loaders/STLLoader.js';
import {joints,placement,nameOf,clamp} from './rig.js';
import {componentPose} from './component-rig.js';
const $=id=>document.getElementById(id),host=$('viewport');
const scene=new THREE.Scene(),camera=new THREE.PerspectiveCamera(38,1,.1,10000);
let renderer;
try{renderer=new THREE.WebGLRenderer({antialias:true,alpha:true});}
catch(error){$('load-status').textContent='3D unavailable: enable browser hardware acceleration, then reload.';throw error;}
renderer.setPixelRatio(Math.min(devicePixelRatio,2));renderer.setClearColor(0x0c1720,0);
renderer.outputColorSpace=THREE.SRGBColorSpace;host.prepend(renderer.domElement);
const orbit=new OrbitControls(camera,renderer.domElement);orbit.enableDamping=true;orbit.autoRotateSpeed=.6;
scene.add(new THREE.HemisphereLight(0xdbeeff,0x26313a,2.4));
for(const [pos,color,intensity] of [[[180,260,260],0xfff5dd,3],[[-220,60,150],0x86d9ff,2],[[0,170,-220],0xabc9ff,2.5]]){
 const light=new THREE.DirectionalLight(color,intensity);light.position.set(...pos);scene.add(light);
}
const grid=new THREE.GridHelper(700,28,0x3c6370,0x1b3440);grid.position.y=-139;scene.add(grid);
const root=new THREE.Group();scene.add(root);
const loader=new STLLoader(),raycaster=new THREE.Raycaster(),pointer=new THREE.Vector2();
let manifest,selected=null,mode='assembly',isolated=false,demo=false,explode=0,objects=[],last=0;
const layoutKey='inmoov-twin-layout-v2-components';
let savedLayout={};try{const v=JSON.parse(localStorage.getItem(layoutKey)||'{}');if(v&&typeof v==='object')savedLayout=v;}catch{}
const pulses=Object.fromEntries(joints.map(j=>[j.ch,j.neutral]));
const channelUI=new Map(),labels=new Map();
function validPose(p){return p&&['position','rotation'].every(k=>Array.isArray(p[k])&&p[k].length===3&&p[k].every(v=>Number.isFinite(v)&&Math.abs(v)<10000));}
function setPulse(ch,p){const j=joints.find(x=>x.ch===ch);if(!j||!Number.isFinite(p))return;pulses[ch]=Math.round(clamp(p,j.a,j.b));const ui=channelUI.get(ch);if(ui){ui.input.value=pulses[ch];ui.out.textContent=`CH${ch} · ${pulses[ch]} µs`;}}
function neutral(){joints.forEach(j=>setPulse(j.ch,j.neutral));}
function setDemo(on){demo=on;$('demo').textContent=on?'Pause preview':'Play expression preview';}
for(const j of joints){
 const box=document.createElement('div');box.className='channel';
 const label=document.createElement('label');label.textContent=j.name;label.htmlFor=`ch-${j.ch}`;
 const input=document.createElement('input');Object.assign(input,{type:'range',id:`ch-${j.ch}`,min:Math.min(j.a,j.b),max:Math.max(j.a,j.b),step:1,value:j.neutral});
 const out=document.createElement('span');out.className='readout';out.textContent=`CH${j.ch} · ${j.neutral} µs`;
 const a=document.createElement('a');a.href=`../#servo-${j.ch}`;a.target='_blank';a.textContent='Calibrate ↗';
 input.oninput=()=>{setDemo(false);$('live').checked=false;setPulse(j.ch,+input.value);$('motion-status').textContent='Manual simulation · no hardware commands sent';highlightChannels([j.ch]);};
 box.append(label,input,out,a);$('channels').appendChild(box);channelUI.set(j.ch,{input,out});
}
function bounds(){const b=new THREE.Box3();objects.filter(o=>o.mesh.visible).forEach(o=>b.expandByObject(o.mesh));return b;}
function fit(front=false){root.updateMatrixWorld(true);const b=bounds();if(b.isEmpty())return;const center=b.getCenter(new THREE.Vector3()),size=b.getSize(new THREE.Vector3());const halfFov=Math.min(THREE.MathUtils.degToRad(camera.fov/2),Math.atan(Math.tan(THREE.MathUtils.degToRad(camera.fov/2))*camera.aspect));const distance=size.length()*.5/Math.sin(halfFov)*1.12;const direction=front?new THREE.Vector3(0,.05,1).normalize():new THREE.Vector3(.65,.3,1).normalize();orbit.target.copy(center);camera.position.copy(center).addScaledVector(direction,Math.max(distance,80));camera.near=.1;camera.far=10000;camera.updateProjectionMatrix();orbit.update();}
function highlightChannels(chs){for(const o of objects)o.mesh.material.emissive.setHex(o.channels.some(ch=>chs.includes(ch))?0x244d33:0x000000);}
function select(o){selected=o;document.querySelectorAll('.part').forEach(b=>b.classList.toggle('selected',b.dataset.id===o.id));highlightChannels(o.channels);o.mesh.material.emissive.setHex(0x245448);
 $('part-name').textContent=o.name;$('part-file').textContent=o.meta.file;
 const d=o.meta.max.map((n,i)=>(n-o.meta.min[i]).toFixed(1));
 $('part-meta').textContent=`${o.meta.triangles.toLocaleString()} triangles · ${(o.meta.bytes/1e6).toFixed(2)} MB\nSource bounds ${d.join(' × ')} mm\n${o.meta.group} · original mesh, scale 1:1`;
 $('source-link').href=o.meta.url;
 $('mechanism').textContent=o.channels.length?o.channels.map(ch=>`CH${ch} ${joints.find(j=>j.ch===ch).name}`).join(' · '):'Structural / passive component. No calibrated channel assigned.';
 if(o.eye)$('mechanism').textContent+=' — Separated eye component; preview axes require physical verification.';
 $('calibrate').href=o.channels.length?`../#servo-${o.channels[0]}`:'../';
 for(const id of ['isolate','save-layout','reset-part'])$(id).disabled=false;
 for(const [key,values] of Object.entries(o.pose))values.forEach((v,i)=>{const e=$(`edit-${key}-${i}`);if(e)e.value=v;});
}
for(const key of ['position','rotation'])for(let i=0;i<3;i++){
 const label=document.createElement('label');label.textContent=`${key==='position'?'Move':'Rotate'} ${'XYZ'[i]} (${key==='position'?'mm':'°'})`;
 const input=document.createElement('input');Object.assign(input,{type:'number',step:key==='position'?1:5,id:`edit-${key}-${i}`,value:0});
 input.onchange=()=>{if(!selected||input.value===''||!Number.isFinite(+input.value)||Math.abs(+input.value)>=10000)return;selected.pose[key][i]=+input.value;};label.appendChild(input);$('placement').appendChild(label);
}
function basePose(meta){const def=componentPose(meta);return {position:[...def.position],rotation:[...def.rotation]};}
function addPart(meta,geometry,suffix='',mirror=false){
 const name=nameOf(meta.parent),id=meta.file+suffix,def=componentPose(meta);
 const pose=basePose(meta);if(mirror)pose.position[0]*=-1;
 const material=new THREE.MeshStandardMaterial({color:meta.group==='i2Eyes'?0x78bdc9:/(Teeth)/.test(name)?0xf2e5c9:/(Gear|Ring|PCA|Holder|Support|Hinge|Fix)/.test(name)?0x6e899b:0xdbe4e8,roughness:.58,metalness:.18,side:THREE.DoubleSide});
 const mesh=new THREE.Mesh(geometry,material);root.add(mesh);
 const channels=def.channels;
 const o={id,name:def.name||name.replace(/([a-z])([A-Z])/g,'$1 $2')+` · piece ${meta.component+1}`,meta,mesh,pose:validPose(savedLayout[id])?structuredClone(savedLayout[id]):structuredClone(pose),original:pose,channels,eye:def.eye,shell:/^(Topskull|TopBackskull|LowBack|EarLock)/.test(name)};
 if(def.eye?.component===0)material.color.setHex(0x4e779c);
 if(def.eye&&[1,3,4].includes(def.eye.component))material.color.setHex(0xe5edf0);
 mesh.userData.part=o;objects.push(o);
 const b=document.createElement('button');b.className='part';b.dataset.id=id;const title=document.createElement('span');title.textContent=o.name;const small=document.createElement('small');small.textContent=channels.length?'CH '+channels.join(' / '):'STRUCTURE';b.append(title,small);b.onclick=()=>select(o);$('parts').appendChild(b);o.button=b;
 const l=document.createElement('span');l.className='label';l.textContent=o.name;$('labels').appendChild(l);labels.set(o,l);
 return o;
}
function applyVisibility(){for(const o of objects){o.mesh.visible=(!isolated||o===selected)&&($('shell').checked||!o.shell);o.mesh.material.wireframe=$('wire').checked;}}
function animatePart(o,t){
 const p=o.pose;let pos=[...p.position],rot=[...p.rotation];
 if(mode==='catalog'){
   const i=objects.indexOf(o);pos=[(i%7-3)*165,Math.floor(i/7)*-160+320,0];rot=[-90,0,0];
 }else{
   const amount=explode/100;
   pos=pos.map((v,i)=>v+(i===0?Math.sign(v||1)*80:i===1?Math.sign(v||1)*70:Math.sign(v||1)*90)*amount);
   const name=nameOf(o.meta.parent),ch=o.channels[0];
   const delta=ch===undefined?0:(pulses[ch]-joints.find(j=>j.ch===ch).neutral)/400;
   if(['Jaw','BottomTeeth','JawSupport'].includes(name)){const a=(pulses[11]-1300)/400*.36;const pivot=[0,-30,0],y=pos[1]-pivot[1],z=pos[2];pos[1]=pivot[1]+y*Math.cos(a)-z*Math.sin(a);pos[2]=y*Math.sin(a)+z*Math.cos(a);rot[0]+=a*180/Math.PI;}
   if(['Eyebrow','ForHeads'].includes(name))rot[2]+=delta*32;
   if(name==='CheekPuller')rot[2]+=delta*40;
   if(name==='UpperLip')pos[1]+=delta*16;
   if(o.eye){
    const e=o.eye;
    if(e.component<3){const h=(pulses[e.channels[0]]-1477)/400*.55,v=(pulses[e.channels[1]]-joints[e.channels[1]].neutral)/400*.45*(e.side===1?1:-1);const offset=new THREE.Vector3(...pos).sub(new THREE.Vector3(...e.center));const q=new THREE.Quaternion().setFromEuler(new THREE.Euler(v,h,0));offset.applyQuaternion(q).add(new THREE.Vector3(...e.center));pos=offset.toArray();o.motionQuaternion=q;}
    else if(e.component===3||e.component===4){const j=joints[ch],closed=(pulses[ch]-j.neutral)/(1500-j.neutral);const angle=(e.component===3?1:-1)*(1-closed)*.9,pivot=new THREE.Vector3(...e.center),offset=new THREE.Vector3(...pos).sub(pivot);o.motionQuaternion=new THREE.Quaternion().setFromAxisAngle(new THREE.Vector3(1,0,0),angle);pos=offset.applyQuaternion(o.motionQuaternion).add(pivot).toArray();}
   }
 }
 o.mesh.position.set(...pos);o.mesh.rotation.set(...rot.map(THREE.MathUtils.degToRad));
 if(mode==='assembly'&&o.motionQuaternion){o.mesh.quaternion.premultiply(o.motionQuaternion);o.motionQuaternion=null;}
}
function setMode(next){mode=next;isolated=false;$('isolate').textContent='Isolate';$('assembly').setAttribute('aria-pressed',String(next==='assembly'));$('catalog').setAttribute('aria-pressed',String(next==='catalog'));$('explode').disabled=next==='catalog';applyVisibility();objects.forEach(o=>animatePart(o,0));fit();}
$('assembly').onclick=()=>setMode('assembly');$('catalog').onclick=()=>setMode('catalog');
$('fit').onclick=()=>fit();$('front').onclick=()=>fit(true);
$('rotate').onclick=()=>{orbit.autoRotate=!orbit.autoRotate;$('rotate').setAttribute('aria-pressed',String(orbit.autoRotate));$('rotate').textContent=orbit.autoRotate?'Pause rotation':'Rotate';};
$('explode').oninput=e=>explode=+e.target.value;
$('shell').onchange=$('wire').onchange=applyVisibility;
$('isolate').onclick=()=>{isolated=!isolated;$('isolate').textContent=isolated?'End isolation':'Isolate';applyVisibility();fit();};
$('show-all').onclick=()=>{isolated=false;$('shell').checked=true;$('search').value='';$('filter').value='all';filter();applyVisibility();fit();};
function filter(){const q=$('search').value.toLowerCase();for(const o of objects)o.button.hidden=!(o.name.toLowerCase().includes(q)||o.meta.file.toLowerCase().includes(q))||($('filter').value!=='all'&&o.meta.group!==$('filter').value);}
$('search').oninput=$('filter').onchange=filter;
$('save-layout').onclick=()=>{if(!selected)return;savedLayout[selected.id]=structuredClone(selected.pose);try{localStorage.setItem(layoutKey,JSON.stringify(savedLayout));$('motion-status').textContent='Assembly layout saved in this browser.';}catch{$('motion-status').textContent='Browser storage unavailable; export layout instead.';}};
$('reset-part').onclick=()=>{if(!selected)return;selected.pose=structuredClone(selected.original);select(selected);};
$('export-layout').onclick=()=>{const data={units:'mm',status:'unverified-assembly',parts:Object.fromEntries(objects.map(o=>[o.id,o.pose]))};const url=URL.createObjectURL(new Blob([JSON.stringify(data,null,2)],{type:'application/json'}));const a=document.createElement('a');a.href=url;a.download='inmoov-head-layout.json';a.click();setTimeout(()=>URL.revokeObjectURL(url),1000);};
$('demo').onclick=()=>{$('live').checked=false;setDemo(!demo);$('motion-status').textContent='Expression simulation · separated eye, brow and jaw mechanisms · estimated joint axes';};
$('rest').onclick=()=>{setDemo(false);$('live').checked=false;neutral();$('motion-status').textContent='Preview neutral · no commands sent to robot';};
$('live').onchange=()=>{setDemo(false);$('motion-status').textContent=$('live').checked?'Waiting for commands from the robot-control tab. Not measured motor feedback.':'Simulation only';};
if('BroadcastChannel' in window){const bus=new BroadcastChannel('inmoov-twin-commands-v1');bus.onmessage=({data})=>{
 if(!$('live').checked||typeof data?.command!=='string')return;
 const c=data.command,match=/^[CK](\d+) (\d+)$/.exec(c);
 if(match)setPulse(+match[1],+match[2]);
 else if(c==='2')neutral();
 else if(/^E[0-3]$/.test(c)||/^B[01]$/.test(c)){const mask=c[0]==='E'?+c[1]:(c[1]==='1'?3:0);for(const ch of [4,5,6,7])setPulse(ch,mask&(ch<6?1:2)?1500:joints[ch].neutral);}
 $('motion-status').textContent=`Last command: ${c} · ${new Date().toLocaleTimeString()} · commanded state only`;
 if(c==='1'||c==='A1')$('motion-status').textContent+=' · onboard motion has no position telemetry';
 };}
let pointerStart;
renderer.domElement.addEventListener('pointerdown',e=>pointerStart=[e.clientX,e.clientY]);
renderer.domElement.addEventListener('pointerup',e=>{if(!pointerStart||Math.hypot(e.clientX-pointerStart[0],e.clientY-pointerStart[1])>5)return;const r=renderer.domElement.getBoundingClientRect();pointer.set((e.clientX-r.left)/r.width*2-1,-(e.clientY-r.top)/r.height*2+1);raycaster.setFromCamera(pointer,camera);const hit=raycaster.intersectObjects(objects.filter(o=>o.mesh.visible).map(o=>o.mesh))[0];if(hit)select(hit.object.userData.part);});
new ResizeObserver(()=>{const w=host.clientWidth,h=host.clientHeight;renderer.setSize(w,h);camera.aspect=w/h;camera.updateProjectionMatrix();}).observe(host);
function frame(time){requestAnimationFrame(frame);if(document.hidden)return;const dt=Math.min((time-last)/1000,.05);last=time;
 if(demo){const a=(Math.sin(time/850)+1)/2;setPulse(11,1300+a*300);setPulse(14,1500-a*160);setPulse(15,1500+a*160);setPulse(8,1500-a*140);setPulse(9,1500+a*140);setPulse(12,1455+a*130);setPulse(13,1566-a*150);setPulse(10,1411+a*65);}
 objects.forEach(o=>animatePart(o,time));orbit.update(dt);renderer.render(scene,camera);
 for(const o of objects){const label=labels.get(o);label.hidden=!$('names').checked||!o.mesh.visible;if(!label.hidden){const v=o.mesh.getWorldPosition(new THREE.Vector3()).project(camera);label.hidden=v.z>1||v.z< -1;label.style.left=`${(v.x+1)*host.clientWidth/2}px`;label.style.top=`${(-v.y+1)*host.clientHeight/2}px`;}}
}
requestAnimationFrame(frame);
async function load(){
 const res=await fetch('assets/manifest.json');if(!res.ok)throw Error('Asset manifest missing');manifest=await res.json();$('part-count').textContent=manifest.parts.length;
 let done=0;const failed=[];
 // Bounded parallel loading keeps browser memory pressure and requests manageable.
 const componentResponse=await fetch('assets/components.json');if(!componentResponse.ok)throw Error('Component manifest missing');const components=await componentResponse.json();
 const queue=[...components];async function worker(){while(queue.length){const meta=queue.shift();try{const geo=await loader.loadAsync('assets/'+meta.file.split('/').map(encodeURIComponent).join('/'));geo.computeBoundingBox();geo.center();geo.computeVertexNormals();addPart(meta,geo);}catch(e){failed.push(meta.file);console.error(meta.file,e);}done++;$('load-status').textContent=`Loading ${done}/${components.length} assembly pieces`;}}
 await Promise.all([worker(),worker(),worker()]);$('loaded-count').textContent=`${done-failed.length}/${done} files`;objects.sort((a,b)=>a.name.localeCompare(b.name));objects.forEach(o=>$('parts').appendChild(o.button));
 $('load-status').textContent=failed.length?`Failed: ${failed.join(', ')}. Reload to retry.`:`${manifest.parts.length} source files · ${objects.length} separate assembly pieces loaded`;
 applyVisibility();objects.forEach(o=>animatePart(o,0));fit();if(objects.length)select(objects.find(o=>nameOf(o.meta.parent)==='Jaw')||objects[0]);
}
load().catch(error=>{$('load-status').textContent=`Unable to load models: ${error.message}. Open through localhost, not file://.`;console.error(error);});
