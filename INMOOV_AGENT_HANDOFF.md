# InMoov head - authoritative agent handoff

**Last updated:** 2026-09-28  
**Workspace:** `U:\inmoov`  
**Owner:** Udhaya Kumar  
**Current controller:** Arduino Uno on COM9 + one PCA9685 at I2C address `0x40`

Read this file before changing firmware, calibration, wiring or the browser UI.
It supersedes conflicting channel maps and calibration values in older notes.

## Project goal

Finish a demonstrable InMoov head by 4 October 2026. The finished milestone must:

1. Start safely with all servo outputs off.
2. Use validated limits/rest positions for all 16 facial servos.
3. Provide manual control, neutral, emergency Outputs off, blinking, expressions
   and camera face following.
4. Listen and converse offline in Tamil and English.
5. Speak offline with jaw movement synchronized to audio.
6. Complete two five-minute demonstrations after a cold restart.

Active weekly plan: `PROJECT_WEEK_PLAN_2026-09-28.md`.

Deferred until the milestone passes: digital-twin correction, neck rotation,
full-body integration, custom voice training and GitLab setup.

## Critical safety rules

- Keep external servo power off during wiring changes and firmware uploads.
- Uno, PCA9685 and the external servo supply must share ground.
- Never power the servos from Uno 5V or USB.
- Keep the physical servo-power switch reachable during movement tests.
- Test only one channel at a time with the firmware `K` command.
- Stop immediately for hard contact, strong buzzing, cable tension, bending or
  unexpected motion.
- Do not run All neutral, animation, blink, expressions or camera following
  until all individual channels are physically revalidated.
- Do not send movement commands automatically on reconnect.

## Authoritative hardware wiring

| Arduino Uno | PCA9685 |
|---|---|
| 5V | VCC logic power |
| GND | GND |
| A4 | SDA |
| A5 | SCL |
| GND | OE |

External regulated servo-supply positive goes to the PCA green `POWER+`
terminal. Supply negative goes to `POWER-` and common Uno/PCA ground. Servo plug
rows on the current PCA board are PWM signal, V+ and GND. Address jumpers remain
open for `0x40`.

## Meaning of angle/position values

The UI's `0-180` values are calibration positions used to map between two pulse
endpoints. They are **not measured mechanical degrees**. Pulse values in
microseconds are the authoritative controller values. Mechanically reversed
servos legitimately have decreasing pulse as position increases.

The table below is the baseline compiled into the restored face firmware and UI.
It is saved, but it still needs physical revalidation after storage. The new
calibrator's browser-local records are not a firmware update and have not yet
been exported in this session.

## Saved 16-servo calibration baseline

| CH | Mechanism | Position 0 / endpoint A | Rest/neutral | Position 180 / endpoint B | State |
|---:|---|---|---|---|---|
| 0 | Left eye horizontal | Left: 1300 us | pos 80 / 1477 us | Right: 1700 us | Needs recheck |
| 1 | Left eye vertical | Up: 1300 us | pos 90 / 1500 us | Down: 1700 us | Needs recheck |
| 2 | Right eye horizontal | Left: 1300 us | pos 80 / 1477 us | Right: 1700 us | Needs recheck |
| 3 | Right eye vertical | Up: 1700 us | pos 80 / 1522 us | Down: 1300 us | Reversed; needs recheck |
| 4 | Left upper eyelid | Open/rest: 1322 us | pos 0 / 1322 us | Closed: 1500 us | Needs recheck |
| 5 | Left lower eyelid | Open/rest: 1588 us | pos 0 / 1588 us | Closed: 1411 us | Reversed; needs recheck |
| 6 | Right upper eyelid | Open/rest: 1655 us | pos 0 / 1655 us | Closed: 1500 us | Reversed; needs recheck |
| 7 | Right lower eyelid | Open/rest: 1322 us | pos 0 / 1322 us | Closed: 1500 us | Needs recheck |
| 8 | Left eyebrow | Lower: 1677 us | pos 90 / 1500 us | Upper: 1322 us | Reversed; needs recheck |
| 9 | Right eyebrow | Lower: 1322 us | pos 85 / 1500 us | Upper: 1700 us | Needs recheck |
| 10 | Nose | Down: 1366 us | pos 60 / 1411 us | Up: 1500 us | Needs recheck |
| 11 | Jaw | Closed/rest: 1300 us | pos 0 / 1300 us | Fully open: 1700 us | Needs recheck |
| 12 | Left forehead | Lower: 1344 us | pos 56 / 1455 us | Upper: 1700 us | Needs recheck |
| 13 | Right forehead | Lower: 1611 us | pos 28 / 1566 us | Upper: 1322 us | Reversed; needs recheck |
| 14 | Left cheek | Down: 1700 us | pos 90 / 1500 us | Pulled: 1300 us | Reversed; needs recheck |
| 15 | Right cheek | Down: 1300 us | pos 90 / 1500 us | Pulled: 1700 us | Needs recheck |

Do not use `hardware/servo_inventory.csv` as the active channel map; it contains
an early superseded design. The table above, `control_ui/calibrator-page.js`,
`control_ui/app.js` and the combined firmware are the current sources.

## Current software and URLs

- Main control page: `U:\inmoov\control_ui\index.html`
- Main URL: http://127.0.0.1:8765/
- Dedicated calibrator: `U:\inmoov\control_ui\calibrator.html`
- Calibrator URL: http://127.0.0.1:8765/calibrator.html
- Calibrator logic: `U:\inmoov\control_ui\calibrator-page.js`
- Combined firmware source:
  `U:\inmoov\firmware\face_8_servo_uno\face_8_servo_uno.ino`
- Last restored firmware image:
  `U:\inmoov\firmware\face_8_servo_uno\face_8_servo_uno.updated.hex`
- PCA diagnostic:
  `U:\inmoov\firmware\pca9685_safe_test_uno\pca9685_safe_test_uno.ino`
- Calibration procedure: `U:\inmoov\hardware\HEAD_CALIBRATION_WORKFLOW.md`
- Calibration log: `U:\inmoov\hardware\HEAD_CALIBRATION_LOG.csv`

Start the local server from `U:\inmoov`:

```powershell
py -m http.server 8765 --bind 127.0.0.1 --directory control_ui
```

The server is not a permanent Windows service and may need restarting.

## Firmware serial protocol at 115200 baud

| Command | Behaviour |
|---|---|
| `0` | Disable all PCA outputs; safest stop command |
| `K<ch> <pulse>` | Disable other outputs and drive only one selected channel for calibration |
| `C<ch> <pulse>` | Move one channel within compiled limits while retaining the face pose |
| `2` | Move all controlled channels to saved neutral/rest values |
| `1` | Start the onboard expression animation |
| `B1` / `B0` | Close / open all four eyelids |
| `E0`-`E3` | Both open, left wink, right wink, or both closed |
| `A1` / `A0` | Enable / disable Uno-controlled automatic blink |

The current firmware rejects channels outside CH0-CH15 and pulses outside each
compiled range. `K` is mandatory for isolated physical recalibration.

## Dedicated calibration UI behaviour

- Contains all 16 current channels.
- Selecting a channel does not move it.
- Requires a safety checkbox before movement controls unlock.
- Shows both calibration position and pulse microseconds.
- Provides +/-5 us and +/-10 us movement.
- Records endpoint A, neutral/rest and endpoint B. Eyelids use open/rest and
  closed only.
- `Save this calibration in page` stores a draft in browser localStorage under
  `inmoov-full-calibration-v1`.
- `Save and select next` advances to the next unsaved channel.
- Firmware baseline values can be loaded only as unverified references. The UI
  requires physical movement/recording before it will save them as calibrated.
- Export produces `inmoov-16-servo-calibration.json` and CSV.
- Browser-saved values do not change the Uno or main animation until an agent
  reviews the export, updates both source files, compiles and uploads firmware.

## Calibration workflow

For each channel, in order CH0 through CH15:

1. Keep external servo power off while connecting/reconfiguring.
2. Open the calibration page in Chrome or Edge and connect Uno COM9.
3. Select the channel; verify its part name and allowed pulse range.
4. Confirm safety, keep the power cutoff reachable, then enable servo power.
5. Begin at the saved rest/neutral pulse and press Move. Only `K` should be sent.
6. Observe the real mechanism from front and side.
7. Approach endpoint A in 5 us steps. Stop before binding/buzzing; record a
   comfortably unloaded value.
8. Find and record rest/neutral. Eyelid open is its rest value.
9. Approach and record endpoint B with the same method.
10. Return to rest and confirm repeatability.
11. Save and select next, then press Outputs off before touching hardware.
12. After CH0-CH15, export both JSON and CSV and copy final values into
    `hardware/HEAD_CALIBRATION_LOG.csv`.

After reviewing the export:

1. Update calibration arrays in the combined `.ino` file.
2. Update the matching servo definitions in `control_ui/app.js` and
   `control_ui/calibrator-page.js`.
3. Check every neutral is between its two pulse endpoints.
4. Compile for Arduino Uno.
5. Upload with external servo power off.
6. Validate individual channels, eye pairs, eyelids/winks/blink,
   eyebrow/forehead, nose, jaw, cheeks, All neutral, expressions, auto blink and
   camera following—in that order.

## PCA9685 diagnostic result

Arduino AVR Boards `arduino:avr` 1.8.8 is installed. The safe diagnostic compiled
at 5,924 bytes flash and 412 bytes RAM. The user reported:

```text
MODE1=0x20  MODE2=0x04  PRESCALE=0x79
ALL 16 OUTPUTS FORCED OFF
RESULT: PASS - PCA9685 communication is working
```

This confirms I2C address `0x40`, an awake PCA with auto-increment, normal output
mode and approximately 50 Hz servo timing. The diagnostic intentionally disabled
all outputs. After the test, the saved combined face firmware was restored to
COM9; avrdude wrote and verified all 10,070 bytes. A safe `0` command returned
`OFF`. No servo movement was sent after restoration.

## Current state and next action

Completed:

- PCA communication is confirmed working.
- Arduino AVR Boards 1.8.8 is installed.
- Combined face firmware is restored and verified on COM9.
- Dedicated calibration page and detailed workflow/log exist.
- Main UI already contains manual controls, auto blink, named expressions and
  local MediaPipe face tracking.

Not yet completed:

- No channel has been physically revalidated during the current calibration run.
- No new calibrator JSON/CSV export exists yet.
- Offline Tamil/English conversation is planned but not implemented.
- GitLab is not connected to this workspace.

Immediate next step:

1. Open http://127.0.0.1:8765/calibrator.html.
2. Connect COM9.
3. Select CH0, Left eye horizontal.
4. With the mechanism clear and power switch reachable, enable servo power.
5. Move only CH0 to the saved rest value: position 80 / 1477 us.
6. Ask the user whether the eye is centered, left, right, buzzing or not moving.
7. Do not continue to CH1 until CH0 endpoint A, rest and endpoint B are recorded
   and saved.

## Evidence and older records

- Latest detailed checkpoint: `SESSION_CHECKPOINT_2026-09-28.md`
- Previous hardware checkpoint: `SESSION_CHECKPOINT_2026-09-17.md`
- Project summary: `CONVERSATION_SUMMARY.md`
- Latest pointer: `LATEST_SESSION_STATUS.md`

Older handwritten calibration sheets and early channel proposals are historical.
Never replace the active table without a new physical observation and exported
calibration record.
