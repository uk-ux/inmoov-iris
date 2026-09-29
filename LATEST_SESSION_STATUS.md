# Latest conversation checkpoint

## Primary agent handoff

Read `INMOOV_AGENT_HANDOFF.md` first. It is the single authoritative project
handoff containing the goal, safety rules, wiring, current 16-channel pulse and
0-180 position table, firmware commands, calibration workflow, verified PCA
result, file locations, current status and exact next action.

## Authoritative checkpoint - 2026-09-28

Read `SESSION_CHECKPOINT_2026-09-28.md` first. A one-week completion plan, safe
calibration workflow/log and dedicated 16-servo browser calibrator were created.
PCA9685 diagnostic passed at address 0x40 with MODE1 0x20, MODE2 0x04 and
PRESCALE 0x79. Arduino AVR Boards 1.8.8 was installed. The saved combined face
firmware was restored to Uno COM9 and all 10,070 bytes verified; it replied OFF.
No movement was sent. Next step is CH0 left-eye horizontal at 1477 us with the
user physically observing the mechanism.

## Authoritative checkpoint - 2026-09-22

Read `SESSION_CHECKPOINT_2026-09-22.md` first. User created a private GitLab
project at `captainyami236/inmoov` and was given beginner PowerShell commands to
connect `U:\inmoov`. Command execution and authentication are not confirmed.
No Git initialization, commit or push was performed by the assistant. Inspect
Git status/remotes and prepare a `.gitignore` before staging the project.

## Authoritative checkpoint - 2026-09-21

Read `SESSION_CHECKPOINT_2026-09-21.md` first. Today the local UI server was
restarted and returned HTTP 200 at http://127.0.0.1:8765/. The saved calibrated
16-servo firmware was reuploaded to Uno COM9; all 10,070 flash bytes verified.
No movement commands were sent. User wants offline Tamil/English conversation;
voice implementation is pending laptop specifications and the current head check.
September 17 remains the detailed calibration/wiring reference. Older entries
below are historical.

## Authoritative saved checkpoint — 2026-09-17

Read `SESSION_CHECKPOINT_2026-09-17.md` first. It records the uploaded COM9
firmware, latest eyelid values, Uno/PCA9685 wiring, expression tab and current
file paths. Digital twin work is intentionally paused.

## Face expression tab — 2026-09-17

Added a top-level `Manual controls` / `Face expressions` tab switch to the local
control UI. The expression tab scrolls directly to the calibrated gesture panel;
manual controls returns to the hardware actions. Both tabs remain available while
the Uno is disconnected. Static JavaScript, gesture-range and duplicate-ID checks
pass; localhost serves the updated page.

## UI face-gesture loop — 2026-09-17

Added a calibrated gesture panel to `control_ui/index.html` and `app.js` with
Happy, Curious, Surprised, Concerned, Skeptical and Calm presets. Controls:
individual gesture buttons, Play loop, Next gesture, Stop + neutral and 2/3/5
second hold time. The loop controls 12 non-eyelid face channels, enables the
Uno's synchronized auto blink, updates sliders, and safely yields to manual
sliders, calibration, onboard animation, neutral/off and camera-follow modes.
All 72 preset pulses were checked against the active calibrated ranges. Local
HTTP serves the new UI; JavaScript/static UI checks pass. No firmware change or
upload was required for this UI-only addition, and no gesture was sent during
testing. The connected visual-browser backend was unavailable, so no automated
visual screenshot was produced.

## Eyelid recalibration — 2026-09-17

Latest open/closed pulses: CH4 1322/1500, CH5 1588/1411, CH6 1655/1500,
CH7 1322/1500 us. The middle values in the five-column input were ignored because
eyelids have no separate neutral; open is the rest position. `control_ui/app.js`,
`hardware/EYELID_CALIBRATION.md`, and combined face firmware source were updated.
Blink commands now use a separate per-lid closed-pulse array. Firmware must be
compiled and uploaded before the physical robot uses these new values.

Upload completed on 2026-09-17 to Arduino Uno COM9. Avrdude identified the
ATmega328P, wrote and verified 10070 bytes, and the board then reported
`READY calibrated 16-servo face - outputs OFF`. No movement command was sent.

## Authoritative continuation — saved 2026-09-16 after assembly corrections

Read `SESSION_CHECKPOINT_2026-09-16.md` first; it supersedes conflicting notes
below. Current digital twin: http://127.0.0.1:8765/twin/. It now loads 76 separate
pieces from 33 unchanged STL sources, rather than 36 whole-print-plate instances.
Eye pieces are separated and have approximate gaze/eyelid motion. Duplicate
paired plates and jaw/eye orientations were corrected. Skull mating surfaces and
smaller linkages remain visibly unverified/incorrect; the user's request for
perfect assembly is NOT complete. Await front/side/back photos or assembled CAD
to support further alignment. Hardware calibration was unchanged; no upload or
servo movement was performed for this twin correction. Asset-integrity and
browser interaction tests passed. Saving this conversation creates a summary,
not a verbatim transcript. Earlier sections remain as history.

## Latest digital-twin page — 2026-09-16

- User explicitly requested a new digital-twin webpage, superseding the earlier
  removal of the simplified head preview. New separate page:
  http://127.0.0.1:8765/twin/ under `control_ui/twin/`.
- Downloaded 33 official full-resolution i2Head/i2Eyes STL source files,
  673406 triangles, ~33.7 MB, excluding silicone casting molds. Local files
  and provenance manifest (URLs, bounds, checksums, license) in twin/assets.
  Three.js 0.180.0 bundled locally in twin/vendor/package.
- Features: 36 displayed instances including paired cheeks/brows/forehead,
  searchable part list, picking/inspector, source links, isolation, skull hide,
  wireframe, labels, orbit/zoom, exploded/parts layouts, editable placement,
  browser-local layout saving and JSON export, 16 pulse preview controls,
  approximate expression animation, and links to original calibration cards.
- Original UI now has twin link and servo card anchors. A read-only
  BroadcastChannel link reports successful serial writes to the twin when its
  Follow control-page commands checkbox is checked. The twin sends no hardware
  commands. No measured position feedback or onboard-animation telemetry.
- IMPORTANT: this is a detailed mesh/assembly workbench, NOT a mechanically
  validated complete twin. Mesh shapes are original, but placement/joint axes
  are editable estimates. Eye AllParts STLs are print sets, not separated/rigged;
  eye/eyelid values have readouts but no accurate eyeball/eyelid motion yet.
  No silicone skin, servo hardware model or neck channel is fabricated.
- Existing per-part calibrator is linked; its new K command firmware still has
  the earlier unresolved upload status. No firmware upload was attempted for
  the twin page, and the viewer itself needs none.
- Verified: 33 STL parses/triangle counts, 16 unique channel definitions, JS
  syntax, local HTTP, desktop and mobile browser rendering, search, layout,
  isolation, neutral/manual controls, cross-tab command following, 16 calibration
  buttons on original page, no twin page JS errors, no mobile horizontal overflow.
- In-app Browser skill/runtime was inspected; no browser backend was available.
  Tests used a separately launched headless Edge with explicit permission.
  Reproduce with `node hardware/twin-tests/check.mjs`. Screenshots in hardware/
  twin-preview.png, twin-parts-preview.png, twin-isolate-preview.png,
  twin-mobile-preview.png. A first CLI capture was blank; subsequent automated
  browser rendering with SwiftShader verified actual meshes visible.
- Next mechanical work: separate print sets and align/rig parts against actual
  assembly references and measurements before calling the model an accurate twin.

## Latest UI update: per-part calibration panels

- User says head parts are printed and assembled and wants a calibrator for each
  part on the page. Added Calibrate buttons to all 16 servo cards in the existing
  control UI; no detailed STL digital twin has been assembled yet.
- New `control_ui/calibrator.js`: pulse entry, +/-5 us, endpoint/neutral capture,
  browser-local draft storage and JSON export. Eyelids use open/closed only.
- Drafts explicitly do NOT change live tracking, animation or firmware limits.
  Ranges are restricted to the existing compiled bounds, including eyelids.
  Expanded ranges require review and new firmware calibration.
- Calibration stops automatic modes. New K<channel> <pulse> firmware command
  switches all outputs off, then drives only the requested channel at that pulse.
  First activation can jump; users must observe the mechanism before recording.
- Firmware compile passed: 10062 bytes flash, 899 bytes RAM. JS syntax checks
  passed and calibrator.js served HTTP 200. Upload retry is not yet confirmed;
  initial attempt reports programmer not responding on COM9.

## Latest animation update — 2026-09-16 (uploaded successfully)

- Start animation's five expression poses now include jaw and both cheeks:
  attentive smile, smiling glance, concerned glance, open-mouth surprise,
  and asymmetric skeptical expression. Each returns to neutral between poses.
- Jaw motion spans 1360–1620 us in expressions and returns to 1300 us closed.
  Both cheek directions respect their mirrored calibration. Existing blink
  and camera imitation functionality remains in the combined build.
- Compile passed (9898 bytes flash, 899 bytes RAM); all 5 poses verified to
  contain 16 entries within calibrated limits and move jaw and both cheeks.
- Initial upload was blocked by COM9 access denied. Retry at the user's request
  succeeded on COM9. The jaw/cheek animation is now installed. Connect the UI
  and press Start animation to test the physical movements.

## Latest blink update — 2026-09-16 (uploaded successfully)

- User requested faster blinking and independent one-eye winks, superseding
  the earlier requirement to close both eyes on either human eye's blink.
- UI now uses `blink-tracking.mjs`: separate left/right states, close threshold
  0.25, reopen threshold 0.12, minimum closure command 140 ms; sustained eye
  closure remains closed until that eye reopens. No two-frame delay.
- New firmware E0=both open, E1=left closed, E2=right closed, E3=both closed.
  Left pair CH4/5, right pair CH6/7. Each pair has its own interpolation timer;
  an unchanged eye's motion does not restart when the other eye changes.
- Eyelid travel 80 ms (previously 130); Auto/Test Blink reopen 180 ms after
  close command. Auto/Test Blink still use both eyes together.
- JS syntax and synthetic independent wink/hold/hysteresis/reset tests passed.
  Firmware compile: 9898 bytes flash, 899 bytes RAM.
- Upload initially blocked by COM9 access denied, then retried at the user's
  request and SUCCEEDED on COM9. New faster independent-wink firmware is on
  the Uno. Ctrl+F5 the UI, reconnect, Start camera, Follow my face to test.

## Latest update — 2026-09-16: jaw/cheek face imitation enabled

- 16-servo combined firmware UPLOADED successfully to COM9 on this turn,
  superseding the blocked upload status below.
- Follow my face now uses jawOpen to control jaw CH11; mouthSmile and
  cheekSquint lift each cheek independently, mouthFrown lowers each cheek.
  Neutral input closes jaw to 1300 us and returns both cheeks to 1500 us.
- `control_ui/mouth-tracking.mjs` contains bounded expression mappings with
  small deadbands. `app.js` smooths movement and sends CH11/14/15 along with
  existing tracking channels; nose CH10 remains manual.
- Syntax and synthetic mapping checks passed (neutral, jaw open, independent
  cheeks, invalid scores, bounded targets). Physical imitation still needs
  user observation; gains are initial defaults, not measured facial calibration.
- Refresh portal with Ctrl+F5, Connect Uno COM9, Start camera, Follow my face.

## Latest integration — 2026-09-16: 16-servo UI prepared

- User requested jaw/cheek UI integration; implemented manual sliders and neutral
  buttons for jaw CH11, left cheek CH14 and right cheek CH15.
- Jaw UI 0=closed/neutral 1300 us, 90=half-open 1500 us, 180=open 1700 us.
  Left cheek UI down/up = 1700/1300; right = 1300/1700; both neutral 1500.
- Combined `face_8_servo_uno` sketch now supports all 16 PCA channels. Existing
  automatic expression poses keep jaw closed and cheeks neutral. Camera tracking
  still controls eyes/brows/forehead/eyelids; jaw and cheeks are manual controls.
- Compiled successfully: 9600 bytes flash, 894 bytes RAM. JS syntax passed;
  checked UI/firmware calibration agreement and all animation poses within limits.
- Upload attempt was BLOCKED: COM9 access denied. User must release the port
  (close Serial Monitor / disconnect web UI) before the new firmware can upload.
  Do not claim the 16-servo version is on the Uno yet.
- Refresh http://127.0.0.1:8765/ with Ctrl+F5 after upload.

## Latest update — 2026-09-16: jaw and cheeks calibrated

This update supersedes the pending jaw/cheek arrangement below.
- Jaw CH11: lower/open 1700 us; neutral/closed 1300 us; upper/closed 1300 us.
  User confirmed position 0 closed and neutral, 90 half open, 180 fully open.
- Left cheek CH14: lower 1700, neutral 1500, upper 1300 us.
- Right cheek CH15: lower 1300, neutral 1500, upper 1700 us.
- Full record: `hardware/JAW_CHEEK_CALIBRATION.md`.
- Simple calibration sketch compiled: 6676 bytes flash, 485 bytes RAM.
  `firmware/jaw_cheek_calibrator_uno/jaw_cheek_calibrator_uno.ino`.
  Commands: `1 90` jaw, `2 90` left cheek, `3 90` right cheek;
  number plus N/D/U saves neutral/down/up; P prints; X off.
- User experienced upload errors earlier; assistant did not upload this calibrator.
  User subsequently reported movement and supplied cheek calibration rows.
- Combined firmware/UI still control 13 servos. Jaw and cheek integration is
  the next available step; it has not yet been requested or implemented.

## Authoritative resume checkpoint — 2026-09-15

This checkpoint supersedes the older status sections below (retained as history).

- Platform: laptop, Arduino Uno on COM9, PCA9685 at 0x40; external servo power.
- Portal: http://127.0.0.1:8765/ (Chrome/Edge). Ctrl+F5 after UI edits.
- Combined sketch: `firmware/face_8_servo_uno/face_8_servo_uno.ino`.
  Despite its historical name, it now controls 13 servos.
- Latest combined firmware compiled successfully: 9548 bytes flash, 818 bytes RAM.
  Upload to COM9 succeeded after the user's latest reupload request.
- UI: `control_ui/index.html`, `styles.css`, `app.js`; local MediaPipe assets.
- User previously confirmed eye, eyebrow and forehead operation. Physical testing
  of the latest nose/auto-blink build has not yet been confirmed by the user.
- Rotating head / exploded assembly preview was added and then removed at the
  user's request. Do not restore it unless requested.

### Active mapping and calibration (microseconds)

| Part | CH | Calibration |
| --- | --- | --- |
| Left eye horizontal | 0 | 1300–1700, neutral 1477 |
| Left eye vertical | 1 | 1300–1700, neutral 1500 |
| Right eye horizontal | 2 | 1300–1700, neutral 1477 |
| Right eye vertical | 3 | 1300–1700, neutral 1522; inverted orientation |
| Left upper eyelid | 4 | Open 1322, closed 1500 |
| Left lower eyelid | 5 | Open 1700, closed 1500 |
| Right upper eyelid | 6 | Open 1700, closed 1500 |
| Right lower eyelid | 7 | Open 1322, closed 1500 |
| Left eyebrow | 8 | Lower 1677, neutral 1500, upper 1322 |
| Right eyebrow | 9 | Lower 1322, neutral 1500, upper 1700 |
| Nose up/down | 10 | Lower 1366, neutral 1411, upper 1500 |
| Left forehead | 12 | Lower 1344, neutral 1455, upper 1700 |
| Right forehead | 13 | Lower 1611, neutral 1566, upper 1322 |

Eyelids have no separate neutral; open is rest. Calibration values are pulse
widths, not measured physical angles. Nose data also saved in
`hardware/NOSE_CALIBRATION.md`. Nose-only calibration sketch:
`firmware/nose_calibrator_uno/nose_calibrator_uno.ino`.

### Blink and UI behavior

- B1/B0 close/open all four eyelids with a shared 130 ms movement timer,
  independent of other face movement commands.
- Auto Blink button uses A1/A0; timing runs on the Uno without a camera.
  Random open interval 2500–5500 ms; closure phase 470 ms including 130 ms
  travel and approximately 340 ms at closed target.
- Follow my face switches auto blink off and imitates detected human blinks.
  Either eye can trigger both robot eyes (threshold 0.18, two camera frames).
- Test Blink is a single coordinated blink. Outputs off stops tracking and auto
  blink and cancels the pending test timer. Disconnect sends outputs off.
- Nose UI slider and neutral button use CH10 and the supplied calibration.
- Serial: 115200 baud. Commands 1 animate, 2 neutral, 0 off, C10 1411 nose
  neutral, B1/B0 close/open eyelids, A1/A0 auto blink on/off.

### Next step: jaw and cheeks — awaiting arrangement confirmation

User wants to move to jaw and cheeks. Proposed (NOT yet confirmed or implemented):
CH11 jaw, CH14 left cheek, CH15 right cheek. Asked whether there is one jaw servo
and two separate cheek servos. Await that answer before calibration work.
No jaw or cheek limits, neutrals, code, or wiring have been confirmed.

---

## Older checkpoints (historical)

Last updated: 2026-09-12

## Confirmed working checkpoint — 2026-09-15

The user confirmed that the four eye servos plus the two eyebrow and two
forehead servos are now working correctly together. Active channels are eyes
CH0/1/2/3 and eyebrow/forehead CH8/9/12/13. Right-eye vertical CH3 is mounted
in the opposite direction; its mapping was inverted while preserving the
1522 us neutral. The updated combined firmware compiles successfully.

The local control UI at control_ui includes eight calibrated sliders, serial
connect, animation/neutral/off controls, and local MediaPipe live-camera face
tracking. MediaPipe package 1.0.1 and the face-landmarker model are stored under
control_ui/vendor. The user confirmed eye, eyebrow and forehead operation after
the CH3 inversion fix.

Next module: four eyelid servos. Planned PCA9685 mapping is left upper CH4,
left lower CH5, right upper CH6, right lower CH7. Eyelid neutral positions and
safe open/closed limits have not been calibrated yet.

Eyelid update 2026-09-15: user mechanically aligned all four eyelid servos so
position 90 / 1500 us is safely closed. See hardware/EYELID_CALIBRATION.md.
Individual open and normal rest positions are still pending.

Confirmed open pulses: CH4 left upper 1322 us, CH5 left lower 1700 us,
CH6 right upper 1700 us, CH7 right lower 1322 us. Closed is 1500 us on all
four. User confirmed that no separate neutral is needed; open is the rest state.

2026-09-15 authoritative eyebrow/forehead calibration from the Uno EEPROM:
CH8 lower/neutral/upper 1677/1500/1322 us; CH9 1322/1500/1700 us;
CH12 1344/1455/1700 us; CH13 1611/1566/1322 us. These values supersede the
earlier handwritten position sheets and still need copying into the combined
animation firmware.

## Current task

Working on the four eyebrow/forehead servos with Arduino Uno and PCA9685.
User requested saving the conversation after restoring the original channels.

## Active channel mapping

| Joint | PCA9685 channel |
| --- | --- |
| Left eyebrow | 8 |
| Right eyebrow | 9 |
| Left forehead | 12 |
| Right forehead | 13 |

The temporary 12/13/14/15 map is superseded. Left and right refer to the robot.
The calibrator, natural-face animation and channel checker source files now
use 8/9/12/13. Latest channel-check build succeeded (6150 bytes flash,
431 bytes RAM). The restored mapping has NOT been uploaded by the assistant;
the firmware currently on the physical Uno has not been verified.

## Calibration state

Updated 2026-09-11 sheet neutrals: left eyebrow position 100, right eyebrow
80, left forehead 50, right forehead 150. Updated endpoint notes are saved in
hardware/CONFIRMED_NEUTRALS.md and hardware/EYEBROW_FOREHEAD_REPORTED_LIMITS.md.

On 2026-09-11 the user supplied a newer photographed sheet. It lists left
eyebrow neutral 100, right eyebrow 80, left forehead 50, right forehead 150;
endpoints are recorded in the limits file. These values have not been applied
to firmware or uploaded. They still need a cautious in-place test.

## Firmware and controls

- firmware/eyebrow_forehead_calibrator_uno/eyebrow_forehead_calibrator_uno.ino:
  commands such as `1 90`, `2 40`, `3 90`, `4 80`; `0` disables outputs.
  Position 0-180 maps to 1300-1700 us, not physical degrees. Those example
  commands reproduce historical neutrals, not newly validated positions.
- firmware/channel_check_uno/channel_check_uno.ino: at 115200 baud with
  Newline, `1`, `2`, `3`, `4` select channels 8, 9, 12, 13 respectively.
  Each test rocks around 1450 us (1410-1490 us) for six seconds; `0` disables
  outputs. This range was based on old limits and needs rechecking too.
- firmware/natural_face_uno/natural_face_uno.ino: `1` animates, `2` returns
  to stored neutral, `0` disables outputs. User requested larger swings and
  faster movement; current version uses broad expressions, 180-300 ms
  outward transitions and 220-400 ms returns, with shorter pauses.
- firmware/pca_i2c_diagnostic_uno/pca_i2c_diagnostic_uno.ino: diagnostic scan.
  User reported the I2C fault resolved, but did not identify its cause.

## Other discussed modules

Eye movement channels were proposed as 0/1 for robot-left horizontal/vertical
and 2/3 for robot-right horizontal/vertical. Eyelid channels were proposed as
4 left upper, 5 left lower, 6 right upper, 7 right lower. User then returned
to eyebrows/forehead; eye and eyelid calibration has not been completed.

Eye calibration values supplied on 2026-09-12 are now recorded in
hardware/EYE_CALIBRATION.md: servo 0/CH0 neutral 80, servo 1/CH1 neutral 90,
servo 2/CH2 neutral 80, servo 3/CH3 neutral 100. All four reported ranges are
0–180 on the calibration scale.

The combined eight-servo sketch is
firmware/face_8_servo_uno/face_8_servo_uno.ino. It uses eye channels 0–3 and
eyebrow/forehead channels 8, 9, 12, 13, with the latest neutrals for all eight.
It compiled successfully on 2026-09-12 but has not been uploaded. Commands at
115200 baud are `1` to animate, `2` to return to neutral, and `0` to disable
all outputs. The sketch uses coordinated eye, eyebrow, and forehead poses and
the recorded 0–180 endpoint scale.

## Next session

Keep servo power off while restoring plugs to 8/9/12/13. Upload the matching
calibration firmware, establish current neutral and safe travel for each joint
one at a time, then update the animation's calibrated bounds before running it.
Do not automatically run movement commands on reconnect.
