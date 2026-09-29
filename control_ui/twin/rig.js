// Draft assembly coordinates in mm. Source meshes remain unscaled.
// Pivots and travel are visualization estimates, not engineering measurements.
export const joints=[
 [0,'Left eye horizontal',1300,1477,1700],[1,'Left eye vertical',1300,1500,1700],
 [2,'Right eye horizontal',1300,1477,1700],[3,'Right eye vertical',1700,1522,1300],
 [4,'Left upper eyelid',1322,1322,1500],[5,'Left lower eyelid',1700,1700,1500],
 [6,'Right upper eyelid',1700,1700,1500],[7,'Right lower eyelid',1322,1322,1500],
 [8,'Left eyebrow',1677,1500,1322],[9,'Right eyebrow',1322,1500,1700],
 [10,'Nose / upper lip',1366,1411,1500],[11,'Jaw',1300,1300,1700],
 [12,'Left forehead',1344,1455,1700],[13,'Right forehead',1611,1566,1322],
 [14,'Left cheek',1700,1500,1300],[15,'Right cheek',1300,1500,1700]
].map(([ch,name,a,neutral,b])=>({ch,name,a,neutral,b}));
export const placement={
 BottomTeeth:[[0,-49,62],[0,0,0],[11]],CheekPuller:[[50,-12,37],[0,0,0],[14,15]],
 EarLock:[[0,32,-10],[90,0,0],[]],EyebrowSupport:[[0,42,10],[0,0,0],[8,9]],
 Eyebrow:[[29,38,66],[0,0,0],[8,9]],FaceHolderLeft:[[32,-8,2],[90,0,90],[]],
 FaceHolderRight:[[-32,-8,2],[90,0,-90],[]],ForHeadSupport:[[0,65,14],[0,0,0],[12,13]],
 ForHeads:[[27,66,50],[0,0,0],[12,13]],GearHolder:[[0,-55,-13],[-90,0,0],[]],
 JawHinge:[[0,-29,2],[0,0,0],[11]],JawPiston:[[0,-45,10],[90,0,0],[11]],
 JawSupport:[[0,-44,3],[90,0,0],[11]],Jaw:[[0,-65,32],[-90,0,0],[11]],
 LowBack:[[0,-48,-39],[-90,0,0],[]],MainGear:[[0,-100,-13],[-90,0,0],[]],
 PCA9685Connect:[[0,0,-27],[0,0,0],[]],PCA9685support:[[0,-18,-42],[0,0,0],[]],
 Ring:[[0,-93,-13],[-90,0,0],[]],ServoGear:[[0,-110,-13],[-90,0,0],[]],
 SkullServoFix:[[0,3,-5],[0,0,0],[]],TeethTopHolder:[[0,-29,40],[0,0,0],[]],
 TopBackskull:[[0,49,-45],[-90,0,180],[]],TopTeeth:[[0,-36,65],[0,0,0],[]],
 TopskullFront:[[0,72,35],[-90,0,0],[]],Topskull:[[0,110,-5],[-90,0,0],[]],
 UpperLip:[[0,-17,64],[0,0,0],[10]],servoAdapter:[[0,-106,-20],[0,0,0],[]],
 servoHornAdapter:[[0,-112,-13],[-90,0,0],[]],
 'Eye-L-Base':[[32,15,31],[-90,0,0],[0,1]],'Eye-R-Base':[[-32,15,31],[-90,0,0],[2,3]],
 'Eye-L-AllParts':[[38,14,50],[-90,0,0],[0,1,4,5]],'Eye-R-AllParts':[[-38,14,50],[-90,0,0],[2,3,6,7]]
};
export const nameOf=file=>file.replace(/V\d.*\.stl$/i,'');
export const clamp=(v,a,b)=>Math.max(Math.min(a,b),Math.min(Math.max(a,b),v));
