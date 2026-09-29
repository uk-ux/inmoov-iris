# InMoov head - 3-day finish workflow

**Days:** Sep 28 (calibrate) / Sep 29 (firmware + ladder + voice skeleton) /
Sep 30 (integrate + rehearse). Done by Sep 30 evening.
**Supersedes the week plan in `WORKFLOW.md` for pacing; every safety rule in
`INMOOV_AGENT_HANDOFF.md` still overrides everything here.**

Rules for the whole 3 days:

- Build nothing that is not on the critical path. No serve.py, no
  generate.py, no doc split, no ladder gating. Only `config/head.json` +
  `tools/validate.py` survive from the tooling plan.
- Voice is descoped to demo-grade: offline TTS (Piper - has Tamil and
  English voices) speaking scripted lines with jaw sync from audio
  amplitude. Keyword triggers only if the script already works. Full
  offline STT conversation is NOT in this plan.
- Every hardware step: power off to touch, `0` before hands near the face,
  one channel at a time with `K`.

---

## DAY 1 (Sep 28) - calibrate all 16 channels

### Morning block 1 (~45 min): minimal setup

```
[ ] mkdir backups; copy as never-edited baseline:
      backups/2026-09-28-baseline-face_8_servo_uno.ino
      backups/2026-09-28-baseline-app.js
      backups/2026-09-28-baseline-calibrator-page.js
      backups/2026-09-28-baseline-face_8_servo_uno.updated.hex
[ ] Extract config/head.json from app.js servos[] (sources agree 2026-09-28;
    mechanical copy)
[ ] Write tools/validate.py: rest inside endpoints (strict unless eyelid/jaw),
    travel >= 40us, all pulses 1000-2000, expression poses inside ranges
[ ] python tools/validate.py -> green
[ ] Create STATE.md
```

### Rest of day (~5 h): Loop 2 x 16, budget 15 min/channel

Start the server, connect COM9, expect READY then `0` -> OFF.
Order (slowest, most careful on eyelids and jaw - they strip gears):

```
[ ] CH0-CH3   eyes          [ ] CH8-CH9   eyebrows
[ ] CH4-CH7   eyelids       [ ] CH10-CH11 nose, jaw
[ ] CH12-CH13 foreheads     [ ] CH14-CH15 cheeks
```

Per channel: rest -> observer check -> endpoint A in 5us steps (stop before
binding) -> rest repeats -> endpoint B -> rest -> save in page -> `0` ->
one line in STATE.md.

**Midday gate:** fewer than 8 channels done by midday -> drop the second
repeatability check (single check only) for the remainder.
**Stuck-servo rule:** a channel that fights you gets safe conservative
endpoints recorded and you move on. Perfect later, if ever.

### Evening (~1.5 h): transcribe once

```
[ ] Export JSON + CSV from the calibrator
[ ] backups/2026-09-28-head.json, then update config/head.json from export
[ ] python tools/validate.py -> green
[ ] Hand-edit face_8_servo_uno.ino, app.js, calibrator-page.js from head.json
    (mind the .ino CH[] order {0..9,12,13,10,11,14,15} - indexes 10-13 are
    NOT channels 10-13)
[ ] python tools/validate.py again -> green
```

**Hard gate: all 16 recorded tonight, or Day 2 morning finishes the tail
before anything else.**

## DAY 2 (Sep 29) - upload, ladder, voice skeleton

### Morning: firmware + full ladder

```
[ ] Servo power OFF; backups/<date>-last-known-good.hex
[ ] Compile arduino:avr:uno, upload COM9, avrdude verified, READY, 0 -> OFF
[ ] Ladder in strict order, servo power on only per step:
    1 individual channels (K spot-checks)  2 eye pairs  3 lids/winks/blink
    4 brow+forehead  5 nose/jaw/cheeks  6 All neutral  7 expressions x3
    8 auto blink 2 min  9 camera following 2 min
[ ] Any failure: 0, fix that channel, re-climb from the failed step
[ ] FREEZE calibration by midday - after this, no value changes without
    redoing the ladder
```

Rollback if new firmware misbehaves: re-flash last-known-good.hex, READY,
`0`, debug on the bench.

### Afternoon: voice skeleton on the PC (no firmware change)

```
[ ] Install Piper + one Tamil voice + one English voice; render test WAVs
[ ] Python script: play WAV while streaming jaw over the EXISTING serial
    protocol - RMS envelope -> C11 <pulse>, mapped into the CALIBRATED jaw
    range from head.json, ~20 Hz, always ending on jaw-closed rest
[ ] Test with servo power on, power switch in reach; 0 when done
```

## DAY 3 (Sep 30) - demo build + cold rehearsals

### Morning: the demo script

```
[ ] 4-5 spoken segments, Tamil + English, interleaved with expressions (1),
    neutral (2), blink (B1/B0), auto blink (A1/A0), camera following
[ ] One runner script or checklist that a cold operator can follow
[ ] Keyword triggers ONLY if the scripted run already works end to end
```

### Afternoon: two full cold-restart rehearsals

```
[ ] Power EVERYTHING off. From nothing: server, COM9, READY, 0 -> OFF,
    run the 5-minute demo. Twice.
[ ] Fix only what breaks the demo; everything else -> STATE.md
[ ] Evening is the buffer - the rehearsals WILL surface something
```

**Done =** two clean cold-restart runs + STATE.md saying so + final
backups/<date>-head.json and hex copies.

---

## The two honest risks

1. Day 1 is the longest calibration day physically possible. One servo
   needing mechanical repair blows the schedule - the stuck-servo rule
   exists so it only costs 15 minutes, not the day.
2. Requirement 4 ("listen and converse offline") is only partially met by
   scripted speech + optional keywords. To an audience it looks nearly
   identical; against the milestone text it is a known gap that buys the
   two days this plan saves.
