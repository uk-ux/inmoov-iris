# Simple eyelid recalibration — Uno + PCA9685

Upload this sketch manually after closing the control UI serial connection and
Serial Monitor. Upload replaces the combined face program until you restore it.
No Adafruit library required. Servo power OFF during upload/reset. PCA address 0x40.
Uno A4 -> SDA, A5 -> SCL, 5V -> VCC, common GND. Servo V+ uses the correct external
servo supply, NOT Uno 5V. Keep an accessible servo-power switch.

Serial Monitor: **115200 baud, Newline**. Other mechanisms remain disabled.

| Servo number | Channel | Eyelid |
| --- | --- | --- |
| 1 | 4 | Left upper |
| 2 | 5 | Left lower |
| 3 | 6 | Right upper |
| 4 | 7 | Right lower |

Commands (one line at a time):

```
1 1500
1 1495
1 C
1 O
P
0
```

The two movement commands specify microseconds, not degrees. 1500 us was the
previously reported closed position, NOT guaranteed safe after mechanical changes.
Start at a known safe pulse for the current assembly. Initial activation/switching
servos cannot be smoothly ramped from an unknown physical position and may jump.
Once active, small numerical changes ramp slowly. Use 5 us steps near contact;
PCA9685 at 50 Hz resolves roughly 4.9 us, so not every numeric change is distinct.
Do not run a full-range sweep. The 1300–1700 guard is not a measured safe limit.

For each servo: adjust to just closed without pressing the lids together; wait for
AT, then `1 C`. Move to a comfortable open position, wait for AT, then `1 O`.
Replace 1 with 2/3/4 for the other lids. Leave clearance before mechanical stops;
stop immediately for strain, buzzing or binding. On I2C failure software cannot
guarantee power-off: physically cut servo power. `0` disables PWM, not supply power.

Match the visible eye opening/closure, not equal numbers: mirrored linkages may
need different pulses. No neutral is needed. This sketch deliberately tests one
lid at a time, not simultaneous blinking. Send the `P` CSV after all four are done
to update and test coordinated closure in the main application.

O/C save the last commanded pulse only after its ramp completes; they do not sense
the actual shaft or detect a stall. Values persist in Uno EEPROM at 256..279, using
a new format tag. The older calibrator data at 128 is not overwritten. Zero in CSV
means not yet recorded. Calibration does NOT automatically update UI/face firmware.
