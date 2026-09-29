# InMoov i2Head Hardware Plan

The printed-part list for this project matches the official InMoov i2Head.

## Actuator count

The official bill of materials specifies:

- 15 JX PDI-1109MG servos for facial features
- 2 JX PDI-6221MG servos for jaw and head rotation
- 1 PCA9685 16-channel servo controller

An HS-805BB can be used for head rotation instead of a PDI-6221MG. The printed
`servoAdapter` and `servoHornAdapter` are used when the smaller PDI-6221MG-style
servo is installed in the rotation mechanism.

## Mechanical groups

### Fixed structure and access

- `FaceHolderLeft`, `FaceHolderRight`, `GearHolder`
- `Topskull`, `TopskullFront`, `TopBackskull`, `LowBack`
- `EarLock`, `Ring`, `SkullServoFix`

### Head rotation

- `MainGear`, `ServoGear`, `GearHolder`, `Ring`, `SkullServoFix`
- One high-torque servo: PDI-6221MG or HS-805BB

### Jaw and mouth

- `Jaw`, `JawSupport`, `JawHinge`, `JawPiston`
- `TopTeeth`, `BottomTeeth`, `TeethTopHolder`
- One high-torque PDI-6221MG for the jaw
- One PDI-1109MG for the upper lip

### Facial expression system

- Four eye-direction servos: left/right and up/down for each eye
- Four eyelid servos: upper/lower for each eye
- Two eyebrow servos
- Two cheek servos connected to the `CheekPuller` pieces
- Two forehead servos connected to `ForHeads`
- One upper-lip servo

Total facial servos: 15.

## Proposed controller assignment

This is a new, internally consistent map for the scratch rebuild. MyRobotLab
can later be configured to use these channel numbers.

| Controller | Channel/pin | Function | Expected servo |
| --- | ---: | --- | --- |
| PCA9685 | 0 | Left eye left/right | PDI-1109MG |
| PCA9685 | 1 | Left eye up/down | PDI-1109MG |
| PCA9685 | 2 | Right eye left/right | PDI-1109MG |
| PCA9685 | 3 | Right eye up/down | PDI-1109MG |
| PCA9685 | 4 | Left upper eyelid | PDI-1109MG |
| PCA9685 | 5 | Left lower eyelid | PDI-1109MG |
| PCA9685 | 6 | Right upper eyelid | PDI-1109MG |
| PCA9685 | 7 | Right lower eyelid | PDI-1109MG |
| PCA9685 | 8 | Left eyebrow | PDI-1109MG |
| PCA9685 | 9 | Right eyebrow | PDI-1109MG |
| PCA9685 | 10 | Left cheek | PDI-1109MG |
| PCA9685 | 11 | Right cheek | PDI-1109MG |
| PCA9685 | 12 | Left forehead | PDI-1109MG |
| PCA9685 | 13 | Right forehead | PDI-1109MG |
| PCA9685 | 14 | Upper lip | PDI-1109MG |
| PCA9685 | 15 | Reserved, output disabled | - |
| Mega | D44 | Jaw signal | PDI-6221MG |
| Mega | D45 | Head rotation signal | PDI-6221MG or HS-805BB |

The D44/D45 assignment is a project choice, not a factory InMoov default. Do
not attach the jaw or rotation mechanism until each servo has been centred and
its direction and safe pulse limits have been measured.

## Power layout

The PCA9685 and Mega generate control signals; they must not be used as the
source of servo current.

```text
                    +--> fused facial-servo branch --> PCA9685 V+
Regulated servo PSU |
                    +--> fused jaw branch ----------> jaw servo positive
                    +--> fused rotation branch -----> rotation servo positive

PSU negative ------+--> PCA9685 GND
                    +--> jaw/rotation servo grounds
                    +--> Mega GND
```

Use a supply voltage supported by every connected servo. Confirm the actual
servo labels/datasheets before choosing the supply voltage. Size supply, fuse,
wiring and connectors from measured or specified stall current, not normal
idle current.

## Build order

1. Verify that all printed parts are present and move freely without servos.
2. Bench-test the Mega and PCA9685 with no servos.
3. Bench-test one PDI-1109MG without horn or linkage.
4. Test and label all 15 facial servos individually.
5. Install facial servos one mechanism at a time.
6. Bench-test and centre the jaw PDI-6221MG, then install it.
7. Bench-test and centre the rotation servo, then install it last.
8. Calibrate narrow limits before applying the silicone skin.

