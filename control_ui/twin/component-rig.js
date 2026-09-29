// Source triangles stay at 1:1 scale. Poses are for individually centered pieces.
// Robot axes: +X left, +Y up, +Z face. Reference: inmoov.fr/headi2/.
import {placement,nameOf} from './rig.js';
const pose=(position,rotation=[0,0,0],channels=[],name)=>({position,rotation,channels,name});
const configs={
 Topskull:[pose([26.8,82,-12],[0,90,0]),pose([-26.8,79.57,-11.8],[0,-90,33.756]),pose([0,58,-64],[-90,0,0])],
 TopBackskull:[pose([41.5,46,-16],[-90,0,-90]),pose([-42.05,46,-15.85],[-90,0,88.876])],
 TopskullFront:[pose([28.46,70,25],[0,90,90]),pose([-28.46,70,25],[0,-90,-90]),pose([25.53,43,33],[0,90,90]),pose([-25.53,43,33],[0,-90,-90])],
 Jaw:[pose([0,-60,33],[0,0,0],[11])],
 LowBack:[pose([0,-51,-33],[-90,0,0])],
 Eyebrow:[pose([30,42,59],[-90,0,0],[8],'Left eyebrow'),pose([-30,42,59],[-90,0,180],[9],'Right eyebrow')],
 ForHeads:[pose([-12,54,54],[90,0,0],[13],'Right forehead lever'),pose([12,54,54],[90,0,0],[12],'Left forehead lever')],
 CheekPuller:[pose([41,-33,43],[0,0,90],[14],'Left cheek puller'),pose([-41,-33,43],[0,0,-90],[15],'Right cheek puller')],
 JawHinge:[pose([35,-30,-20],[90,0,90]),pose([-35,-30,-20],[90,0,-90])],
 JawSupport:[pose([48,-47,9],[0,90,30],[11]),pose([-48,-47,9],[0,-90,-30],[11])],
 JawPiston:[pose([0,-31,9],[90,0,0],[11]),pose([0,-45,15],[0,90,0],[11])],
 EarLock:[pose([58,4,-7],[0,90,90]),pose([-58,4,-7],[0,-90,-90])],
 BottomTeeth:[pose([0,-53,58],[-90,0,0],[11])],
 TopTeeth:[pose([0,-38,58],[90,0,0])],
 TeethTopHolder:[pose([0,-24,35],[-90,0,0])],
 EyebrowSupport:[pose([0,25,30],[90,0,0])],
 ForHeadSupport:[pose([0,65,34],[-90,0,0])],
 UpperLip:[pose([0,-20,53],[90,0,0],[10])],
 'Eye-L-Base':[pose([31,10,5],[-90,0,0])],
 'Eye-R-Base':[pose([-31,10,5],[-90,0,0])],
};
const eyeNames=['Iris','Eyeball shell','Eye gimbal','Lower eyelid','Upper eyelid','Upper lid rod','Lower lid rod','Upper rod spacer','Lower rod spacer','Horizontal link','Vertical link','Servo horn','Link clip','Link bracket','Servo mount','Eye support'];
export function componentPose(meta){
 const key=nameOf(meta.parent),i=meta.component;
 if(key.endsWith('AllParts')){
  const left=key.includes('-L-'),side=left?1:-1,x=31*side,base=left?0:2;
  const entries=[
   pose([x,13,69],[0,0,0],[base,base+1]),
   pose([x,13,60],[0,0,0],[base,base+1]),
   pose([x,13,42],[90,0,0],[base,base+1]),
   pose([x,5,64.3],[0,0,0],[left?5:7]),
   pose([x,21,64.3],[0,0,0],[left?4:6]),
   pose([x+side*19,24,28],[90,0,0]),pose([x+side*19,3,28],[90,0,0]),
   pose([x+side*19,24,51]),pose([x+side*19,3,51]),
   pose([x+side*12,8,10],[90,0,0]),pose([x-side*9,8,10],[90,0,0]),
   pose([x,0,-8],[0,0,0]),pose([x,10,30]),pose([x,0,12]),
   pose([x,12,-17],[-90,0,0]),pose([x,0,12],[-90,0,0])
  ];
  return {...entries[i],name:`${left?'Left':'Right'} ${eyeNames[i]}`,eye:{side,component:i,center:[x,13,55],channels:[base,base+1]}};
 }
 const def=configs[key]?.[i];if(def)return def;
 const fallback=placement[key]||[[0,0,0],[0,0,0],[]];return pose([...fallback[0]],[...fallback[1]],[...fallback[2]]);
}
