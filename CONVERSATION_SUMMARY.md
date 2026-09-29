# InMoov i2Head Conversation Summary

Latest saved checkpoint: `SESSION_CHECKPOINT_2026-09-28.md`. The PCA9685 safe
diagnostic passed, Arduino AVR Boards 1.8.8 was installed, and the saved combined
face firmware was restored and verified on Uno COM9. A dedicated calibration-only
UI now records all 16 channel positions/pulses in browser storage and exports
JSON/CSV. No servo movement was sent after restoration. Begin CH0 at 1477 us
with physical observation. The one-week completion plan is
`PROJECT_WEEK_PLAN_2026-09-28.md`.

Latest saved checkpoint: `SESSION_CHECKPOINT_2026-09-22.md`. The user created a
private GitLab project at `captainyami236/inmoov` and received beginner commands
to connect the existing `U:\inmoov` folder. Execution is not confirmed; no commit
or push was performed. Inspect status/remotes and create a `.gitignore` before
staging files.

Previous checkpoint: `SESSION_CHECKPOINT_2026-09-21.md`. On September 21,
restarted the local UI server (HTTP 200) and reuploaded the saved 16-servo firmware
to Uno COM9 (10,070 bytes verified). No movement commands were sent. Offline
Tamil/English conversation is planned but not implemented; hardware checking
comes first. Laptop specifications are still unknown.

Detailed hardware checkpoint: `SESSION_CHECKPOINT_2026-09-17.md`. It supersedes older
notes for current uploaded firmware, eyelid calibration, UI expression tabs and
Uno–PCA9685 wiring. Digital twin work is paused by user request.

2026-09-17 UI update: added six named face gestures and a browser-controlled
gesture loop with 2/3/5-second timing, Next and Stop + neutral. Loop uses all 12
non-eyelid facial channels and synchronized auto blink. Static tests passed; no
firmware change/upload and no real movement command were needed for this update.
Added top-level Manual controls and Face expressions tabs; both remain usable
while disconnected and the expression tab jumps to the gesture panel.

## Latest saved checkpoint — 2026-09-16

Read `SESSION_CHECKPOINT_2026-09-16.md` first. It contains the current 16-channel
calibration, firmware upload caveat and digital-twin work. The twin now loads 76
separated pieces from 33 unchanged STLs. Assembly correction is partial: skull
seams and linkages still need alignment. Front/side/back photos were requested.
Older entries below are historical and may be superseded.

2026-09-17 eyelids (latest): open/closed pulses are CH4=1322/1500,
CH5=1588/1411, CH6=1655/1500, CH7=1322/1500 us. The supplied middle column
is ignored: open is rest and no separate eyelid neutral is used. Main UI and
combined firmware source were updated; the updated firmware is not yet uploaded.
The updated 10070-byte firmware was subsequently uploaded and verified on the
Uno at COM9 on 2026-09-17. Startup reported outputs OFF; no movement was sent.

2026-09-15 confirmed checkpoint: all eight eye/eyebrow/forehead servos work
correctly together, including live MediaPipe camera tracking. Right-eye
vertical CH3 was inverted in UI and animation code, keeping neutral at 1522 us.
Next step is eyelid calibration on planned channels CH4, CH5, CH6 and CH7.

2026-09-15: new exact eyebrow/forehead EEPROM calibration supplied by user:
CH8 1677/1500/1322 us, CH9 1322/1500/1700 us, CH12 1344/1455/1700 us,
CH13 1611/1566/1322 us (lower/neutral/upper). This supersedes earlier sheets.

2026-09-11 update: user resupplied the limit sheet as current for CH8/9/12/13.
See hardware/EYEBROW_FOREHEAD_REPORTED_LIMITS.md for the recorded endpoints.
The previous pending-limits status is superseded; neutral changes have not
been clarified. No firmware change/upload was made for this latest report.

2026-09-12 eye update: user supplied neutrals servo 0=80, 1=90, 2=80, 3=100;
all eye limits are 0–180. See hardware/EYE_CALIBRATION.md.

2026-09-12 combined-face update: created and compiled
firmware/face_8_servo_uno/face_8_servo_uno.ino for all eight eye,
eyebrow, and forehead servos. It uses CH0–3 and CH8/9/12/13; commands are
`1` animate, `2` neutral, `0` all off. It has not been uploaded yet.

Latest saved checkpoint: see LATEST_SESSION_STATUS.md for the active channel
map, firmware/upload status, and pending recalibration after changed limits.

CURRENT OVERRIDE: user restored original channels: left eyebrow 8, right
eyebrow 9, left forehead 12, right forehead 13. All three Uno sketches
(calibrator, animation, channel check) match. User reports safe limits changed;
recalibration is pending, and neutral validity remains unconfirmed. Do not run
the old animation until calibration is revalidated. The reassignment below is
historical and no longer active.

## Latest channel reassignment (supersedes older maps below)

The user moved left eyebrow, right eyebrow, left forehead, right forehead to
PCA9685 channels 12, 13, 14, 15 respectively. Both Uno calibration and natural
face sketches and the confirmed calibration tables now use this map. Reupload
the selected sketch before powering the rewired servos. Neutral pulses and
movement limits stay associated with their original mechanisms. Channel 14 is
no longer available for the proposed upper lip; its future assignment is pending.
Older Mega maps and the servo inventory still describe the previous layout.

Last updated: 2026-09-10

## Project situation

- The project is an InMoov v2 humanoid robot head.
- The head was previously assembled and worked on by a teammate.
- Some servo joints or mechanisms are broken or not fully functional.
- The original NUC has a major fault and cannot currently provide its old
  calibration or configuration.
- The old NUC and its storage must be preserved and must not be formatted,
  because its files might be recoverable later.
- The user's Windows laptop will now be the main computer for the head.
- The control and calibration system will be rebuilt from scratch.
- Hardware will be completed and tested before the final laptop software.

## Identified head version

The supplied printed-part list exactly matches the official InMoov `i2Head`:

- BottomTeeth
- CheekPuller
- EarLock
- EyebrowSupport
- Eyebrow
- FaceHolderLeft and FaceHolderRight
- ForHeadSupport and ForHeads
- GearHolder
- JawHinge, JawPiston, JawSupport and Jaw
- LowBack
- MainGear, ServoGear and Ring
- UpperLip
- SkullServoFix
- TeethTopHolder, TopTeeth
- TopBackskull, TopskullFront and Topskull
- Optional servoAdapter and servoHornAdapter for a head-rotation servo smaller
  than an HS-805BB

Some `1x` entries refer to one STL containing or representing paired left/right
parts; they do not imply that only one facial side moves.

## Selected electronics

- Arduino Mega 2560
- One PCA9685 16-channel servo controller, default I2C address `0x40`
- Separate regulated servo power supply
- Common ground between the servo supply, PCA9685 and Mega
- Fuse and accessible emergency servo-power switch
- Laptop connected to the Mega by USB

The Mega is not currently available, so an Arduino Uno is being used for the
first-stage eyebrow and forehead calibration. The final system can move back to
the Mega later without changing the PCA9685 channel assignments.

The Mega supplies PCA9685 logic power only. Servo power must not be taken from
the Mega 5 V pin.

## Expected servos

- 15 JX PDI-1109MG small metal-geared servos for facial features
- 2 JX PDI-6221MG high-torque servos:
  - one for the jaw
  - one for head rotation
- An HS-805BB may be used instead for head rotation.

Total full i2Head actuator count: 17 servos.

## Agreed module split

| Module | Movement | Servo count |
| --- | --- | ---: |
| Eyes | Independent left/right and up/down | 4 |
| Eyelids | Upper and lower lid for each eye | 4 |
| Eyebrows and forehead | Left/right eyebrows and forehead pullers | 4 |
| Cheeks | Left and right cheek pullers | 2 |
| Mouth | Upper-lip movement | 1 |
| Jaw | Open and close | 1 high-torque |
| Nose | Passive in the standard i2Head | 0 |
| Head/neck rotation | Turn head left and right | 1 high-torque |

The nose does not have an independent servo in the standard i2Head. A moving
nose would be a later custom mechanism. Neck pitch or roll would also be a
separate mechanism not represented by the supplied head-parts list.

## Proposed channel map

| Controller | Channel/pin | Joint |
| --- | ---: | --- |
| PCA9685 | 0 | eyeLeftLR |
| PCA9685 | 1 | eyeLeftUD |
| PCA9685 | 2 | eyeRightLR |
| PCA9685 | 3 | eyeRightUD |
| PCA9685 | 4 | eyelidLeftUpper |
| PCA9685 | 5 | eyelidLeftLower |
| PCA9685 | 6 | eyelidRightUpper |
| PCA9685 | 7 | eyelidRightLower |
| PCA9685 | 8 | eyebrowLeft |
| PCA9685 | 9 | eyebrowRight |
| PCA9685 | 10 | cheekLeft |
| PCA9685 | 11 | cheekRight |
| PCA9685 | 12 | foreheadLeft |
| PCA9685 | 13 | foreheadRight |
| PCA9685 | 14 | upperLip |
| PCA9685 | 15 | reserved and disabled |
| Mega | D44 | jaw |
| Mega | D45 | headRotate |

The D44/D45 assignments are project choices for the scratch rebuild, not
factory InMoov defaults. They can be changed during integration if necessary.

## Agreed build and test order

1. Verify the Mega and PCA9685 on the bench with no servo attached.
2. Test one loose PDI-1109MG without its horn or linkage.
3. Build and calibrate the four-servo eye module.
4. Add and calibrate the four eyelid servos.
5. Add the eyebrow and forehead module.
6. Add the two cheek servos.
7. Add the upper-lip servo.
8. Bench-test, centre and install the jaw servo.
9. Bench-test, centre and install head rotation last.
10. Add facial-expression software after every mechanism works safely.

## Current hardware progress

- The head was dismantled so that loose or damaged parts can be repaired and
  each mechanism can be rebuilt safely.
- The first active phase is the four-servo eyebrow and forehead mechanism.
- The eyebrow/forehead assembly shown in the supplied photograph is assembled.
- Current four-servo PCA9685 assignments are:
  - channel 8: robot-left eyebrow
  - channel 9: robot-right eyebrow
  - channel 12: robot-left forehead
  - channel 13: robot-right forehead
- Robot-left means the robot's own left side, not the viewer's left side.
- The left-eyebrow neutral position on channel 8 was tested and confirmed at
  1500 microseconds, approximately 90 degrees.
- The remaining lower and upper limits have not yet been measured.

## Temporary Uno and PCA9685 wiring

| Arduino Uno | PCA9685 |
| --- | --- |
| 5V | VCC logic input |
| GND | GND |
| A4/SDA | SDA |
| A5/SCL | SCL |

- Leave OE unconnected for now.
- Leave Uno pins 0/RX and 1/TX unused.
- The servo supply connects to the PCA9685 green V+ and GND terminal.
- Do not connect the Uno 5 V pin to PCA9685 V+ or use it to power servos.
- The external servo supply, PCA9685, and Uno must share ground.
- On the photographed PCA9685, the servo columns run left-to-right as
  channels 0 through 15 in four groups of four.
- Each servo header uses yellow for signal, red for V+, and black for GND.

## Uno calibration firmware

- The Uno sketch is
  `firmware/eyebrow_forehead_calibrator_uno/eyebrow_forehead_calibrator_uno.ino`.
- It controls the PCA9685 directly through `Wire`, so it does not require the
  missing `Adafruit_PWMServoDriver.h` library.
- The sketch has compiled and uploaded to the Uno on COM8.
- Occasional bootloader warnings such as `not in sync: resp=0x55` occurred, but
  retrying completed the upload.
- All outputs start disabled for safety.
- The Uno firmware was simplified to number-only commands. Type the servo
  number, a space, and the angle: `1 90`, `2 90`, `3 90`, or `4 90`.
- Servo numbers 1 through 4 correspond to PCA9685 channels 8, 9, 12, and 13.
- The single command `0` immediately disables all PCA9685 outputs.
- The short commands work with Newline or No line ending in Serial Monitor.
- The simplified sketch compiled and uploaded successfully to COM8 on
  2026-09-10. Serial verification showed READY with outputs off, and command
  `0` returned OFF. No movement command was sent during verification.
- The earlier compiler issue was an access restriction, not a missing compiler.
- Input overflow and overlong-command handling were corrected before upload.
- The 0-180 command value is a calibration scale spanning 1300-1700 us,
  not a measured physical angle. Mechanical limits are still unconfirmed.

## Calibration and safety rules

- Keep the assembled head unpowered during initial controller tests.
- Test only one mechanically disconnected servo at first.
- Never force a servo shaft or mechanism by hand.
- Centre a servo before fitting its horn or linkage.
- Begin with a narrow pulse range around 1500 microseconds.
- Determine each joint's direction and limits separately.
- Do not copy broad default angles into an uncalibrated mechanism.
- Stop immediately if a servo chatters, stalls, overheats or drives against a
  mechanical stop.
- Choose power-supply voltage from the actual servo labels/datasheets.
- Size supply current, fuses, connectors and wiring from servo stall current.
- Repair broken printed joints before powering their servos.

## Prepared project files

- `HEAD_PROJECT_NOTES.md` - general project notes
- `hardware/MEGA_PCA9685_WIRING.md` - controller wiring and safe bring-up
- `hardware/I2_HEAD_HARDWARE_PLAN.md` - i2Head mechanics and channel plan
- `hardware/servo_inventory.csv` - servo identification and calibration table
- `firmware/pca9685_bench_test/pca9685_bench_test.ino` - safe channel-0 test
  firmware; all PCA9685 outputs start disabled
- `firmware/eyebrow_forehead_calibrator_uno/eyebrow_forehead_calibrator_uno.ino`
  - library-free Uno eyebrow/forehead calibration firmware

## Next step

The simplified number-only Uno firmware is uploaded. Next test only the
left eyebrow on channel 8 using commands such as `1 80`, `1 90`, and `1 100`
to find safe lower and upper mechanical limits.
After recording those limits, repeat the same process separately for channels
9, 12, and 13.
# Latest saved continuation — 2026-09-15

Resume from `LATEST_SESSION_STATUS.md`, section "Authoritative resume checkpoint".
The Uno now has the successfully uploaded 13-servo combined firmware including
CH10 nose (1366 / 1411 / 1500 us lower / neutral / upper) and synchronized Auto
Blink. The UI includes nose controls and Auto Blink. The head rotation/exploded
preview was removed at the user's request. Next task: jaw and cheeks; proposed
channels 11 / 14 / 15, awaiting confirmation of one jaw and two cheek servos.
Earlier notes below are historical and may describe superseded calibration.
