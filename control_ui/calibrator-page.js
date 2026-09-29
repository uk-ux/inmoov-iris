const SERVOS=[
  {group:'Eyes',name:'Left eye horizontal',ch:0,servo:1,aUs:1300,restUs:1477,bUs:1700,a:'Left',b:'Right'},
  {group:'Eyes',name:'Left eye vertical',ch:1,servo:2,aUs:1300,restUs:1500,bUs:1700,a:'Up',b:'Down'},
  {group:'Eyes',name:'Right eye horizontal',ch:2,servo:3,aUs:1300,restUs:1477,bUs:1700,a:'Left',b:'Right'},
  {group:'Eyes',name:'Right eye vertical',ch:3,servo:4,aUs:1700,restUs:1522,bUs:1300,a:'Up',b:'Down'},
  {group:'Eyelids',name:'Left upper eyelid',ch:4,servo:5,aUs:1322,restUs:1322,bUs:1500,a:'Open',b:'Closed',restIsA:true},
  {group:'Eyelids',name:'Left lower eyelid',ch:5,servo:6,aUs:1588,restUs:1588,bUs:1411,a:'Open',b:'Closed',restIsA:true},
  {group:'Eyelids',name:'Right upper eyelid',ch:6,servo:7,aUs:1655,restUs:1655,bUs:1500,a:'Open',b:'Closed',restIsA:true},
  {group:'Eyelids',name:'Right lower eyelid',ch:7,servo:8,aUs:1322,restUs:1322,bUs:1500,a:'Open',b:'Closed',restIsA:true},
  {group:'Eyebrows',name:'Left eyebrow',ch:8,servo:9,aUs:1677,restUs:1500,bUs:1322,a:'Lower',b:'Upper'},
  {group:'Eyebrows',name:'Right eyebrow',ch:9,servo:10,aUs:1322,restUs:1500,bUs:1700,a:'Lower',b:'Upper'},
  {group:'Nose',name:'Nose up / down',ch:10,servo:13,aUs:1366,restUs:1411,bUs:1500,a:'Down',b:'Up'},
  {group:'Jaw',name:'Jaw',ch:11,servo:14,aUs:1300,restUs:1300,bUs:1700,a:'Closed',b:'Fully open'},
  {group:'Forehead',name:'Left forehead',ch:12,servo:11,aUs:1344,restUs:1455,bUs:1700,a:'Lower',b:'Upper'},
  {group:'Forehead',name:'Right forehead',ch:13,servo:12,aUs:1611,restUs:1566,bUs:1322,a:'Lower',b:'Upper'},
  {group:'Cheeks',name:'Left cheek',ch:14,servo:15,aUs:1700,restUs:1500,bUs:1300,a:'Down',b:'Pulled'},
  {group:'Cheeks',name:'Right cheek',ch:15,servo:16,aUs:1300,restUs:1500,bUs:1700,a:'Down',b:'Pulled'}
];

const STORAGE_KEY='inmoov-full-calibration-v1';
const $=id=>document.getElementById(id);
const clamp=(value,min,max)=>Math.max(min,Math.min(max,value));
const bounds=s=>[Math.min(s.aUs,s.bUs),Math.max(s.aUs,s.bUs)];
const positionToPulse=(s,position)=>Math.round(s.aUs+(s.bUs-s.aUs)*(position/180));
const pulseToPosition=(s,pulse)=>Math.round((pulse-s.aUs)*180/(s.bUs-s.aUs));
const formatPoint=point=>point?`${point.position} deg / ${point.pulse} us${point.verified===false?' (baseline)':''}`:'Not recorded';

let port=null,writer=null,reader=null,reading=false,queue=Promise.resolve();
let active=SERVOS[0],activePulse=active.restUs,lastMovedPulse=null,lastMoveAt=0;
let drafts={},saved=loadSaved();

function loadSaved(){
  try{
    const value=JSON.parse(localStorage.getItem(STORAGE_KEY)||'{}');
    return value&&typeof value==='object'&&!Array.isArray(value)?value:{};
  }catch{return {}}
}

function setMessage(text,tone=''){
  $('moveStatus').textContent=text;
  $('moveStatus').className=`message ${tone}`.trim();
}

function renderList(){
  $('servoList').innerHTML='';
  for(const servo of SERVOS){
    const button=document.createElement('button');
    button.className=`servo-choice${servo.ch===active.ch?' active':''}${saved[servo.ch]?' saved':''}`;
    button.innerHTML=`<span class="channel">CH${servo.ch}</span><span class="servo-copy"><strong>${servo.name}</strong><span>${servo.group} / servo ${servo.servo}</span></span><span class="saved-dot" aria-label="${saved[servo.ch]?'Saved':'Not saved'}"></span>`;
    button.onclick=()=>selectServo(servo);
    $('servoList').appendChild(button);
  }
  const count=Object.keys(saved).filter(ch=>SERVOS.some(s=>s.ch===Number(ch))).length;
  $('progressStatus').textContent=`${count} / ${SERVOS.length} saved`;
  $('savedBadge').textContent=`${count} saved`;
}

function selectServo(servo){
  active=servo;activePulse=servo.restUs;lastMovedPulse=null;
  $('armed').checked=false;
  const record=saved[servo.ch];
  drafts[servo.ch]=record?{a:record.endpoint_a,neutral:record.neutral_or_rest,b:record.endpoint_b}:{};
  $('activeGroup').textContent=servo.group.toUpperCase();
  $('activeName').textContent=servo.name;
  $('activeMeta').textContent=`Servo ${servo.servo} / PCA channel ${servo.ch} / allowed ${bounds(servo)[0]}-${bounds(servo)[1]} us`;
  $('selectedStatus').textContent=`CH${servo.ch}`;
  $('labelA').textContent=servo.a;$('labelB').textContent=servo.b;
  $('recordALabel').textContent=`${servo.a} endpoint`;$('recordBLabel').textContent=`${servo.b} endpoint`;
  $('neutralCard').hidden=Boolean(servo.restIsA);
  $('activeState').textContent=record?'Saved':'Not saved';$('activeState').className=`state ${record?'saved':'unsaved'}`;
  updateInputs(servo.restUs);renderDraft();renderList();updateMoveButtons();
  setMessage('Selected only. No movement sent. Confirm the safety checkbox before moving.','warn');
}

function updateInputs(pulse){
  const [min,max]=bounds(active);
  activePulse=clamp(Math.round(Number(pulse)||min),min,max);
  const position=clamp(pulseToPosition(active,activePulse),0,180);
  $('pulseInput').min=min;$('pulseInput').max=max;$('pulseInput').value=activePulse;
  $('positionInput').value=position;$('positionSlider').value=position;
}

function updateFromPosition(value){
  const position=clamp(Math.round(Number(value)||0),0,180);
  updateInputs(positionToPulse(active,position));
}

function updateMoveButtons(){
  const enabled=Boolean(writer)&&$('armed').checked;
  ['minus10','minus5','move','plus5','plus10','recordA','recordNeutral','recordB'].forEach(id=>$(id).disabled=!enabled);
  $('outputsOff').disabled=!writer;
}

async function sendRaw(command){
  if(!writer)return false;
  queue=queue.then(()=>writer.write(new TextEncoder().encode(command+'\n')))
    .then(()=>true).catch(error=>{setMessage(error.message,'bad');return false});
  return queue;
}

async function moveSelected(delta=0){
  if(!writer)return setMessage('Connect the Uno first.','bad');
  if(!$('armed').checked)return setMessage('Confirm the safety checkbox before moving.','warn');
  updateInputs(activePulse+delta);
  const ok=await sendRaw(`K${active.ch} ${activePulse}`);if(!ok)return;
  lastMovedPulse=activePulse;lastMoveAt=performance.now();
  $('controllerStatus').textContent=`CH${active.ch}: ${activePulse} us`;
  setMessage(`Moved only CH${active.ch} to ${pulseToPosition(active,activePulse)} deg / ${activePulse} us. Inspect before recording.`,'good');
}

function currentPoint(){
  if(lastMovedPulse===null||lastMovedPulse!==activePulse||performance.now()-lastMoveAt<500){
    setMessage('Move to this value, wait half a second, inspect it, then record.','warn');return null;
  }
  return {position:clamp(pulseToPosition(active,activePulse),0,180),pulse:activePulse,verified:true};
}

function recordPoint(field){
  const point=currentPoint();if(!point)return;
  drafts[active.ch]??={};drafts[active.ch][field]=point;
  if(active.restIsA&&field==='a')drafts[active.ch].neutral={...point};
  renderDraft();setMessage(`Recorded ${formatPoint(point)} as ${field==='a'?active.a:field==='b'?active.b:'neutral/rest'}.`,'good');
}

function renderDraft(){
  const draft=drafts[active.ch]||{};
  $('recordAValue').textContent=formatPoint(draft.a);
  $('recordNeutralValue').textContent=formatPoint(draft.neutral);
  $('recordBValue').textContent=formatPoint(draft.b);
}

function baselineDraft(){
  drafts[active.ch]={a:{position:0,pulse:active.aUs,verified:false},neutral:{position:clamp(pulseToPosition(active,active.restUs),0,180),pulse:active.restUs,verified:false},b:{position:180,pulse:active.bUs,verified:false}};
  renderDraft();setMessage('Loaded current firmware values as a reference. Move to and record every position before saving.','warn');
}

function saveCurrent(selectNext=false){
  const draft=drafts[active.ch]||{};
  if(!draft.a||!draft.b||(!active.restIsA&&!draft.neutral))return setMessage('Record both endpoints and neutral/rest before saving.','bad');
  const required=active.restIsA?[draft.a,draft.b]:[draft.a,draft.neutral,draft.b];
  if(required.some(point=>point.verified!==true))return setMessage('Baseline values are references only. Move to and record every required position before saving.','bad');
  if(draft.a.pulse===draft.b.pulse)return setMessage('The two endpoints must be different.','bad');
  const neutral=active.restIsA?draft.a:draft.neutral;
  const low=Math.min(draft.a.pulse,draft.b.pulse),high=Math.max(draft.a.pulse,draft.b.pulse);
  if(neutral.pulse<low||neutral.pulse>high)return setMessage('Neutral/rest must be between the two endpoint pulses.','bad');
  saved[active.ch]={channel:active.ch,servo:active.servo,part:active.name,group:active.group,endpoint_a_label:active.a,endpoint_a:draft.a,neutral_or_rest:neutral,endpoint_b_label:active.b,endpoint_b:draft.b,updated:new Date().toISOString(),applied_to_firmware:false};
  try{localStorage.setItem(STORAGE_KEY,JSON.stringify(saved))}catch{return setMessage('Browser storage failed. Export the values and check browser permissions.','bad')}
  renderResults();renderList();$('activeState').textContent='Saved';$('activeState').className='state saved';
  setMessage(`Saved CH${active.ch} in this page. Firmware has not been changed.`,'good');
  if(selectNext){
    const start=SERVOS.findIndex(servo=>servo.ch===active.ch);
    const next=[...SERVOS.slice(start+1),...SERVOS.slice(0,start)].find(servo=>!saved[servo.ch]);
    if(next)selectServo(next);else setMessage('All 16 calibrations are saved. Export JSON and CSV for review.','good');
  }
}

function renderResults(){
  $('resultsBody').innerHTML='';
  for(const servo of SERVOS){
    const record=saved[servo.ch],row=document.createElement('tr');
    if(record)row.innerHTML=`<td>CH${servo.ch}</td><td>${servo.name}</td><td>${record.endpoint_a_label}: ${formatPoint(record.endpoint_a)}</td><td>${formatPoint(record.neutral_or_rest)}</td><td>${record.endpoint_b_label}: ${formatPoint(record.endpoint_b)}</td><td>${new Date(record.updated).toLocaleString()}</td>`;
    else row.innerHTML=`<td>CH${servo.ch}</td><td>${servo.name}</td><td class="empty">Not saved</td><td class="empty">Not saved</td><td class="empty">Not saved</td><td class="empty">-</td>`;
    $('resultsBody').appendChild(row);
  }
}

function download(name,type,text){
  const url=URL.createObjectURL(new Blob([text],{type})),a=document.createElement('a');
  a.href=url;a.download=name;a.click();setTimeout(()=>URL.revokeObjectURL(url),1000);
}

function exportJson(){
  const data={version:1,kind:'inmoov-calibration-drafts-not-applied',exported:new Date().toISOString(),saved_count:Object.keys(saved).length,servos:SERVOS.map(s=>saved[s.ch]||{channel:s.ch,part:s.name,status:'not_saved'})};
  download('inmoov-16-servo-calibration.json','application/json',JSON.stringify(data,null,2));
}

function csvCell(value){return `"${String(value??'').replaceAll('"','""')}"`}
function exportCsv(){
  const header=['channel','part','endpoint_a_label','endpoint_a_position','endpoint_a_us','neutral_position','neutral_us','endpoint_b_label','endpoint_b_position','endpoint_b_us','updated'];
  const rows=[header.join(',')];
  for(const servo of SERVOS){
    const r=saved[servo.ch];
    rows.push([servo.ch,servo.name,r?.endpoint_a_label,r?.endpoint_a?.position,r?.endpoint_a?.pulse,r?.neutral_or_rest?.position,r?.neutral_or_rest?.pulse,r?.endpoint_b_label,r?.endpoint_b?.position,r?.endpoint_b?.pulse,r?.updated].map(csvCell).join(','));
  }
  download('inmoov-16-servo-calibration.csv','text/csv',rows.join('\r\n'));
}

async function readSerial(){
  reading=true;const decoder=new TextDecoder();
  while(port?.readable&&reading){
    reader=port.readable.getReader();
    try{
      while(true){
        const {value,done}=await reader.read();if(done)break;
        if(value){const reply=decoder.decode(value,{stream:true}).trim().split(/\r?\n/).at(-1);if(reply){$('controllerStatus').textContent=reply;if(reply.includes('FAULT'))setMessage(reply,'bad')}}
      }
    }catch(error){if(reading)setMessage(error.message,'bad')}
    finally{reader.releaseLock();reader=null}
  }
}

async function connect(){
  if(!('serial' in navigator))return setMessage('Use Chrome or Edge. This browser does not support Web Serial.','bad');
  try{
    port=await navigator.serial.requestPort();await port.open({baudRate:115200});writer=port.writable.getWriter();
    $('connectionStatus').textContent='Uno connected';$('connect').textContent='Disconnect';$('controllerStatus').textContent='Connected / no movement sent';
    updateMoveButtons();readSerial();setMessage('Connected. Select one servo, confirm safety, then press Move.','good');
  }catch(error){setMessage(`Connection failed: ${error.message}`,'bad')}
}

async function disconnect(){
  if(writer)await sendRaw('0');reading=false;if(reader)await reader.cancel();
  if(writer){writer.releaseLock();writer=null}if(port){await port.close();port=null}
  $('connectionStatus').textContent='Disconnected';$('connect').textContent='Connect Uno';$('controllerStatus').textContent='Outputs off';
  $('armed').checked=false;lastMovedPulse=null;updateMoveButtons();setMessage('Disconnected.','warn');
}

async function outputsOff(){
  if(!writer)return;await sendRaw('0');lastMovedPulse=null;$('armed').checked=false;updateMoveButtons();
  $('controllerStatus').textContent='Outputs off';setMessage('All PCA9685 outputs are off.','good');
}

$('positionInput').addEventListener('input',event=>updateFromPosition(event.target.value));
$('positionSlider').addEventListener('input',event=>updateFromPosition(event.target.value));
$('pulseInput').addEventListener('input',event=>updateInputs(event.target.value));
$('armed').addEventListener('change',()=>{lastMovedPulse=null;updateMoveButtons();setMessage($('armed').checked?'Safety confirmed. Press Move when ready.':'Movement controls locked.',$('armed').checked?'warn':'')});
$('minus10').onclick=()=>moveSelected(-10);$('minus5').onclick=()=>moveSelected(-5);$('move').onclick=()=>moveSelected();$('plus5').onclick=()=>moveSelected(5);$('plus10').onclick=()=>moveSelected(10);
$('recordA').onclick=()=>recordPoint('a');$('recordNeutral').onclick=()=>recordPoint('neutral');$('recordB').onclick=()=>recordPoint('b');
$('saveCurrent').onclick=()=>saveCurrent(false);$('saveNext').onclick=()=>saveCurrent(true);$('loadBaseline').onclick=baselineDraft;$('exportJson').onclick=exportJson;$('exportCsv').onclick=exportCsv;
$('connect').onclick=()=>port?disconnect():connect();$('outputsOff').onclick=outputsOff;
navigator.serial?.addEventListener('disconnect',()=>disconnect().catch(()=>{}));

selectServo(active);renderResults();
