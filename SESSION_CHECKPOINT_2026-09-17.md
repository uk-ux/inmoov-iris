# InMoov conversation checkpoint — 2026-09-17

Read this file first when continuing. User asked to save the conversation.

## Current files and links

- Arduino source: `U:\inmoov\firmware\face_8_servo_uno\face_8_servo_uno.ino`
- Arduino folder: `U:\inmoov\firmware\face_8_servo_uno`
- Local control UI: `U:\inmoov\control_ui\index.html`
- UI URL: http://127.0.0.1:8765/
- Last uploaded firmware image: `face_8_servo_uno.updated.hex` in the Arduino folder.
- Digital twin is intentionally set aside; do not resume it unless the user asks.

## Hardware upload state

The combined 16-channel face firmware was uploaded and flash-verified on the
Arduino Uno at COM9 on 2026-09-17. Avrdude identified an ATmega328P, wrote and
verified 10,070 bytes, and startup printed:

`READY calibrated 16-servo face - outputs OFF`

No movement command was sent during verification. Keep external servo power off
while uploading or changing wiring. The uploaded program supports C/K direct
channel commands, B0/B1 blink, E0–E3 eyelid masks, A0/A1 auto blink, 1 animation,
2 neutral and 0 outputs off.

## Latest eyelid calibration

The latest table uses open/rest and closed only. Any middle/neutral column is
ignored because eyelids have no separate neutral:

| Servo | Channel | Open/rest | Closed |
|---:|---:|---:|---:|
| 1 left upper | 4 | 1322 us | 1500 us |
| 2 left lower | 5 | 1588 us | 1411 us |
| 3 right upper | 6 | 1655 us | 1500 us |
| 4 right lower | 7 | 1322 us | 1500 us |

These values are in the uploaded combined firmware and the UI. Calibration
record: `hardware/EYELID_CALIBRATION.md`.

## UI additions

The UI now has top tabs:

- **Manual controls** — camera, sliders, calibration and hardware actions.
- **Face expressions** — Happy, Curious, Surprised, Concerned, Skeptical and Calm.

The expression panel has individual gesture buttons, 2/3/5-second hold time,
Play loop, Next gesture and Stop + neutral. The browser loop commands all 12
non-eyelid face channels and enables synchronized auto blink. It uses the current
calibrated pulse limits. No firmware upload is needed for the tab/gesture UI.
Refresh the page with Ctrl+F5 if it appears stale.

## Uno to PCA9685 wiring

| Uno | PCA9685 |
|---|---|
| 5V | VCC |
| GND | GND |
| A4 | SDA |
| A5 | SCL |
| GND | OE |

External servo supply positive goes to PCA `POWER +`; supply negative goes to
`POWER -` and common Uno GND. Servo plugs: signal top row, red V+ middle row,
brown/black GND bottom row. Never power the servos from Uno 5V or USB. Leave
address jumpers open for address 0x40.

## Channel map in the uploaded face program

CH0–3 eyes; CH4–7 eyelids; CH8–9 eyebrows; CH10 nose; CH11 jaw; CH12–13
forehead; CH14–15 cheeks.

## Verification

The latest UI checks passed: JavaScript syntax, six gesture presets within their
calibrated ranges, required expression controls, no duplicate HTML IDs, and
localhost serving the updated page. A connected visual browser was unavailable,
so no automated screenshot was made. Do not send movement commands automatically
on reconnect; test one safe mechanism at a time with the power switch accessible.
