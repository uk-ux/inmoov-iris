# Resume checkpoint — 2026-09-16

This is a saved conversation summary, not a verbatim transcript. It supersedes
older conflicting progress notes. Workspace: U:\inmoov.

## Where we stopped

User asked for a detailed digital twin on a separate webpage, then reported
that the parts were not correctly assembled and requested a perfect assembly.
Assembly corrections are PARTIAL, not finished. Last request: save conversation.

- Robot UI: http://127.0.0.1:8765/
- Digital twin: http://127.0.0.1:8765/twin/
- Local server was running at last test; these URLs require that server.
- Laptop + Arduino Uno + PCA9685. Latest recorded port COM9; rediscover before upload.

## Digital twin work completed

- 33 official i2Head/i2Eyes STLs retained unchanged, at original scale.
- Split print plates into 76 connected pieces, removing the incorrect duplication
  of whole paired plates. Each eye AllParts STL contains 16 separate pieces.
- Updated jaw/eye orientations, separate brows/forehead, initial skull poses,
  approximate eye gaze and separate upper/lower eyelid pivot motion.
- Source geometry: control_ui/twin/assets/manifest.json.
- Derived pieces: assets/components/ and assets/components.json (parent/source
  checksums plus derived checksums). No source triangles omitted or duplicated.
- Pose definitions: control_ui/twin/component-rig.js.
- Viewer: control_ui/twin/twin.js and index.html. Three.js bundled locally.
- Search, selection, isolation, skull visibility, wireframe, labels, explode,
  parts layout, placement editor, layout export and 16 channel sliders available.
- New layout storage key: inmoov-twin-layout-v2-components. Old v1 layouts remain
  in browser storage but are not applied to the new individual pieces.
- Viewer sends NO servo commands. Optional BroadcastChannel following shows
  commanded state from the control page, not measured servo position.

## Still incomplete — do not claim a perfect twin

Skull seams/gaps and smaller linkage alignment remain visibly incorrect or
unverified. Positions and axes are estimates, not CAD mates or a collision-checked
mechanism. Source-revision matching, hardware models and physical dimensions
have not been verified. No skin or measured position feedback is included.

Last message asked the user for clear front, side and back photos of the assembled
head to match their build. An assembled CAD file with transforms would be more
authoritative. Official assembly reference images are already downloaded into
hardware/assembly-references/; reference page https://inmoov.fr/headi2/.
Next: inspect new photos/CAD if supplied, correct shell mating surfaces and
linkages, then visually verify from multiple angles. Do not redo downloads.

## Current recorded servo configuration

All pulse values below are microseconds, NOT measured physical degrees.
A/B describe direction endpoints; they are not necessarily numeric min/max.

| CH | Joint | Endpoint A | Neutral/rest | Endpoint B |
| --- | --- | ---: | ---: | ---: |
| 0 | Left eye horizontal | 1300 | 1477 | 1700 |
| 1 | Left eye vertical | 1300 | 1500 | 1700 |
| 2 | Right eye horizontal | 1300 | 1477 | 1700 |
| 3 | Right eye vertical, reversed | 1700 | 1522 | 1300 |
| 4 | Left upper lid, open → closed | 1322 | 1322 | 1500 |
| 5 | Left lower lid, open → closed | 1588 | 1588 | 1411 |
| 6 | Right upper lid, open → closed | 1655 | 1655 | 1500 |
| 7 | Right lower lid, open → closed | 1322 | 1322 | 1500 |
| 8 | Left eyebrow | 1677 | 1500 | 1322 |
| 9 | Right eyebrow | 1322 | 1500 | 1700 |
| 10 | Nose (viewer uses upper-lip mesh; mapping unverified) | 1366 | 1411 | 1500 |
| 11 | Jaw, closed → fully open | 1300 | 1300 | 1700 |
| 12 | Left forehead | 1344 | 1455 | 1700 |
| 13 | Right forehead | 1611 | 1566 | 1322 |
| 14 | Left cheek | 1700 | 1500 | 1300 |
| 15 | Right cheek | 1300 | 1500 | 1700 |

Jaw halfway is 1500 us. Lids have no separate neutral; rest is open. Eyelid values
were updated on 2026-09-17. The updated combined firmware was uploaded and flash-
verified on Uno COM9; startup reported outputs OFF and no movement command was sent.

## Firmware / UI caution

- firmware/face_8_servo_uno/face_8_servo_uno.ino is historically named for eight
  servos but now handles 16. Working firmware previously uploaded includes
  independent eyelids, auto blink, jaw/cheek animation and direct channel control.
- A subsequent K<channel> <pulse> isolated-calibration addition compiled, but its
  upload FAILED with programmer-not-responding errors. Do not claim K is installed.
- Per-servo calibration panels exist in control_ui/calibrator.js; saved/exported
  values are drafts and are not automatically applied to firmware.
- Do not upload or send movement commands merely to resume this checkpoint.

## Verification and tooling

- node hardware/twin-tests/assets.mjs: PASS, 33 unchanged sources, 76 components,
  every source triangle retained exactly once; checksums checked.
- node hardware/twin-tests/check.mjs: PASS, 76 pieces, 16 controls, search,
  layouts/isolation, neutral/manual inputs, cross-tab command following,
  16 original-UI calibration buttons, mobile overflow and no page JS errors.
- Passing interaction tests do NOT establish correct mechanical assembly.
- Screenshots: hardware/twin-preview.png and twin-{parts,isolate,mobile}-preview.png.
- Rebuild pieces: py hardware/prepare_assembly_parts.py.
- Inspection helpers: control_ui/twin/inspect.html,
  hardware/twin-tests/inspect.mjs and hardware/mesh_moments.py.
- In-app browser backend unavailable; approved headless Edge/Playwright fallback
  used. rg is unavailable in this shell; use PowerShell searches. No Git repo here.

User prefers short instructions and simple controls. Resume with remaining
digital-twin assembly alignment, not an unrequested firmware change.
