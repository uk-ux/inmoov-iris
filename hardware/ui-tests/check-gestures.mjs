import fs from 'node:fs';
import assert from 'node:assert/strict';

const app=fs.readFileSync('control_ui/app.js','utf8');
const html=fs.readFileSync('control_ui/index.html','utf8');
const extract=(pattern,label)=>{
  const match=app.match(pattern);assert(match,`Missing ${label}`);
  return Function(`"use strict";return (${match[1]})`)();
};
const servos=extract(/const servos = (\[[\s\S]*?\n\]);/,'servo definitions');
const gestures=extract(/const gestures=(\[[\s\S]*?\n\]);/,'gesture definitions');
const byChannel=new Map(servos.map(servo=>[servo.ch,servo]));
const gestureChannels=[0,1,2,3,8,9,10,11,12,13,14,15];

assert.deepEqual(gestures.map(gesture=>gesture.name),
  ['Happy','Curious','Surprised','Concerned','Skeptical','Calm']);
for(const gesture of gestures){
  assert.deepEqual(Object.keys(gesture.pose).map(Number),gestureChannels,
    `${gesture.name} must command every gesture channel exactly once`);
  for(const [channelText,pulse] of Object.entries(gesture.pose)){
    const servo=byChannel.get(Number(channelText));assert(servo,`Unknown channel ${channelText}`);
    assert(Number.isInteger(pulse),`${gesture.name} CH${channelText} pulse must be an integer`);
    assert(pulse>=Math.min(servo.low,servo.high)&&pulse<=Math.max(servo.low,servo.high),
      `${gesture.name} CH${channelText} ${pulse} outside ${servo.low}..${servo.high}`);
  }
}
for(const id of ['gestureButtons','gestureSpeed','gesturePlay','gestureNext','gestureStop','gestureStatus']){
  assert.equal((html.match(new RegExp(`id="${id}"`,'g'))||[]).length,1,`${id} must exist once`);
}
assert.match(app,/setTimeout\(async\(\)=>/);
assert.match(app,/sendRaw\('A1'\)/);
console.log('PASS: 6 gesture presets, 12 calibrated channels each, loop controls present.');
