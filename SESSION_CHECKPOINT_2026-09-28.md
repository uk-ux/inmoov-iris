# InMoov conversation checkpoint - 2026-09-28

This is a saved summary, not a verbatim transcript.

## Weekly completion goal

User wants to finish the InMoov head milestone during the week of 28 September
through 4 October 2026. The active plan is:

- `PROJECT_WEEK_PLAN_2026-09-28.md`

The milestone includes a validated 16-servo head, expressions and tracking,
offline Tamil/English conversation, offline voice, audio-driven jaw movement,
and a repeatable final demonstration. Digital-twin correction, neck rotation,
full-body work and GitLab setup are outside this week's critical path.

## Calibration workflow and UI

Created:

- `hardware/HEAD_CALIBRATION_WORKFLOW.md`
- `hardware/HEAD_CALIBRATION_LOG.csv`
- `control_ui/calibrator.html`
- `control_ui/calibrator.css`
- `control_ui/calibrator-page.js`

Calibration URL: http://127.0.0.1:8765/calibrator.html

The dedicated page contains all 16 current channels, selects one mechanism at a
time, uses firmware command `K<channel> <pulse>` to disable other outputs, shows
both 0-180 calibration position and pulse microseconds, records endpoints and
neutral/rest, stores reviewed drafts in browser localStorage, tracks progress,
offers Save and select next, and exports JSON/CSV. Eyelids use open/rest plus
closed. A safety checkbox is required before movement. Loading the firmware
baseline only creates unverified reference values; every position must be moved
to and recorded before the page allows saving. Saved page values do not update
firmware automatically.

The main face-control page links to the new calibration workspace. Verification
passed: JavaScript syntax, 16 unique channels CH0-CH15, no duplicate HTML IDs,
and HTTP 200 from the localhost page.

## PCA9685 diagnosis

The face firmware first replied `OFF`, later reported an intermittent I2C fault.
A safe diagnostic was created at:

- `firmware/pca9685_safe_test_uno/pca9685_safe_test_uno.ino`

It uses only `Wire`, tests PCA address `0x40` every two seconds, reads MODE1,
MODE2 and PRESCALE, and forces the ALL_LED output off. It never sends a servo
position. Arduino AVR Boards was missing from the active Arduino environment;
installed `arduino:avr` version 1.8.8. The diagnostic then compiled successfully:
5,924 bytes flash and 412 bytes RAM.

User uploaded/runs the test and reported:

```text
MODE1=0x20  MODE2=0x04  PRESCALE=0x79
ALL 16 OUTPUTS FORCED OFF
RESULT: PASS - PCA9685 communication is working
```

Interpretation: the PCA communicates correctly, is awake with auto-increment,
uses normal totem-pole output mode, and is configured for approximately 50 Hz.
The Outputs forced off line is expected diagnostic safety behaviour.

## Restored face firmware

After the PCA test, restored the saved combined firmware image to Arduino Uno
COM9:

- `firmware/face_8_servo_uno/face_8_servo_uno.updated.hex`
- 10,070 flash bytes written and read-back verified by avrdude.
- The restored firmware replied `OFF` to the safe command.
- No servo movement command was sent by the assistant.

The localhost calibration server was restarted and returned HTTP 200. Server
processes are not permanent; restart it from `U:\inmoov` when needed with:

```powershell
py -m http.server 8765 --bind 127.0.0.1 --directory control_ui
```

## Current next action

Begin physical recalibration at CH0:

1. Open http://127.0.0.1:8765/calibrator.html in Chrome or Edge.
2. Connect Uno and choose COM9.
3. Select CH0 - Left eye horizontal.
4. Confirm the safety checkbox and keep the physical power switch reachable.
5. Switch external servo power on.
6. Start at the existing neutral 1477 us and press Move selected servo.
7. Confirm the page reports `CAL CH0 PULSE 1477`.
8. User must report whether the left eye is centered, left, right, buzzing, or
   not moving before changing the pulse or testing another channel.

Do not run All neutral, animation, auto blink, expressions or camera tracking
until all individual channels are physically revalidated.
