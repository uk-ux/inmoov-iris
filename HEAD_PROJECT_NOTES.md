# InMoov v2 Head Project Notes

CONFIRMED 2026-09-15: eyes CH0-3 and eyebrows/forehead CH8/9/12/13 work
correctly together. CH3 right-eye vertical uses reversed direction. The local
UI and MediaPipe face tracking are working. Proceed next to eyelids CH4-7.

For the current resume point, read LATEST_SESSION_STATUS.md. The latest request
was to save progress after restoring the original channels; recalibration is
still pending and the restored source files have not been uploaded here.

The latest resume point includes eye neutrals and the compiled combined
eight-servo animation sketch. It is ready for upload and bench testing when the
user returns.

CURRENT OVERRIDE: restored original channels 8/9/12/13 for left eyebrow,
right eyebrow, left forehead, right forehead. User reports changed safe limits;
recheck calibration before animation. Reassignment below is historical.

Latest channel map overrides historical notes below: left eyebrow CH12,
right eyebrow CH13, left forehead CH14, right forehead CH15. Both Uno sketches
are updated; upload before powering the rewired servos. Upper-lip assignment
is now pending because its earlier CH14 assignment is occupied.

## Current situation

- The project is an InMoov v2 humanoid robot.
- The user's responsibility is the robot head, including human-like facial reactions and related behavior.
- A teammate has already assembled and worked on the head.
- The head is not fully functional because some servo joints are broken.
- A NUC contains the head calibration data and other details.
- The NUC currently has no dedicated monitor.
- The NUC now has a major fault and is not available as the active controller.
- The user's Windows laptop will become the new InMoov head computer.
- The rebuild will start from scratch, beginning with hardware.
- The selected controller architecture is Arduino Mega 2560 plus PCA9685.
- An Arduino Uno is temporarily being used because the Mega is not currently
  available.
- The head has now been dismantled for staged repair and reassembly.
- The eyebrow and forehead four-servo mechanism is the current first phase.
- PCA9685 assignments: left eyebrow CH8, right eyebrow CH9, left forehead CH12,
  and right forehead CH13.
- Left-eyebrow neutral is confirmed at 1500 microseconds, approximately 90
  degrees. Its lower and upper limits still need to be measured.

## Agreed approach

- Do not dismantle and rebuild the entire head immediately.
- First document and understand the existing assembly.
- Turn off robot and servo power before inspection.
- Photograph the head and label servos, wires, connectors, boards, and linkages.
- Identify whether each failed joint has a cracked printed part, loose/stripped servo horn, failed servo gears, or an electrical/control problem.
- Do not operate a servo against a broken joint.
- Back up the NUC calibration files, control software, and configuration before making changes.
- Repair or replace one affected joint at a time.
- Center each servo before attaching its horn/linkage, then test slowly and recalibrate after mechanical repairs.

## Immediate next steps

1. Preserve the old NUC and its storage without formatting it.
2. Keep the assembled head unpowered and make the emergency stop accessible.
3. Bench-wire the Mega and PCA9685 with no head servos connected.
4. Identify every servo model and calculate the required power capacity.
5. Test one mechanically disconnected servo on PCA9685 channel 0.
6. Make an inventory of the broken joints and repair them.
7. Calibrate and connect one head joint at a time.
8. Install and configure the laptop software only after the controller bench test.

## Current next action

- Simplified Uno controls compiled and uploaded to COM8 on 2026-09-10:
  `1 90`, `2 90`, `3 90`, `4 90`, and `0` for all outputs off.
  Startup and the off command were verified without commanding movement.
- Calibrate one servo at a time in small steps, beginning with the left eyebrow
  near its confirmed 90-degree neutral.

## Information to collect from teammate

- NUC operating system and login details
- Control software name and source code location
- Servo model and controller board
- Wiring diagram and servo-to-joint assignments
- Calibration procedure and backup location
- Known mechanical limits and current faults
# Current completion plan

The active one-week workflow is `PROJECT_WEEK_PLAN_2026-09-28.md`. It targets a
calibrated, offline Tamil/English conversational head by 4 October 2026. Follow
the daily exit checks and keep digital-twin, neck and full-body work outside the
weekly critical path.
