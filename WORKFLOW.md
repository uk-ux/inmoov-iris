# InMoov head - operating workflow (no git)

**Created:** 2026-09-28
**Companion to:** `ARCHITECTURE.md` (why) - this file is the *how*.
Ignore ARCHITECTURE.md section 2.7 (git); this workflow replaces it with
dated backups and an append-only history log.
**Safety rules in `INMOOV_AGENT_HANDOFF.md` override everything here.**

Instead of version control, three habits carry the history:

1. `backups/` - a dated copy of any file about to be changed. Never edit a
   backup. Never delete one this week.
2. `config/calibration-history.jsonl` - every measurement appended, never
   rewritten. The current value lives in `config/head.json`; the history of
   how it got there lives here.
3. `STATE.md` - three lines per session: DONE / NEXT / BLOCKED.

Five loops. Run them in order the first time; after that, each session is
Loop 1 -> (Loop 2 or 3 or 4) -> Loop 5.

---

## Loop 0 - one-time setup (~45 min, do before touching CH0)

```
[ ] mkdir backups
[ ] Copy today's state as the baseline (never touched again):
      backups/2026-09-28-baseline-face_8_servo_uno.ino
      backups/2026-09-28-baseline-app.js
      backups/2026-09-28-baseline-calibrator-page.js
      backups/2026-09-28-baseline-face_8_servo_uno.updated.hex
[ ] Extract config/head.json from the current app.js servos[] table
    (all four sources agree as of 2026-09-28 - mechanical copy, no judgement)
[ ] Write tools/validate.py (checks in ARCHITECTURE.md section 2.3)
[ ] python tools/validate.py   -> must pass on the extracted config
[ ] Create empty config/calibration-history.jsonl
[ ] Create STATE.md with today's DONE / NEXT / BLOCKED
```

Optional same day (2-3 h, makes Loop 2 much faster): `tools/serve.py` with
`POST /api/channel`, and point `calibrator-page.js` at it. If you skip this,
Loop 2 still works - you just keep the export-and-transcribe step.

---

## Loop 1 - session start (every session, ~5 min)

```
[ ] Servo power OFF before touching anything
[ ] Read STATE.md - what was done, what is next, what is blocked
[ ] Start the server:  py tools/serve.py     (or the http.server command)
[ ] Open http://127.0.0.1:8765/calibrator.html
[ ] Connect COM9 - expect "READY calibrated 16-servo face - outputs OFF"
[ ] Send 0 once - expect "OFF" (confirms protocol + safe state)
[ ] Physical check: power switch reachable, mechanism clear, wiring seated
```

Only now may servo power go ON, and only when a movement test is about to run.

---

## Loop 2 - calibrate one channel (repeat CH0 -> CH15)

One channel per iteration. Never two. `K` commands only.

```
[ ] Select channel in calibrator - verify part name + allowed range on screen
[ ] Tick the safety checkbox; power switch within reach; servo power ON
[ ] Move to saved rest pulse - ask observer: centered / off / buzzing / dead?
[ ] Walk to endpoint A in 5 us steps - stop BEFORE binding; record unloaded value
[ ] Return to rest; confirm it repeats
[ ] Walk to endpoint B the same way; record
[ ] Back to rest; confirm repeatability once more
[ ] Save:
      with serve.py:  "Save" -> writes config/head.json + appends history line
      without:        "Save in page" -> localStorage (export at end of day)
[ ] Send 0 (Outputs off) BEFORE touching hardware
[ ] One line in STATE.md:  chNN verified: <A> / <B>, rest <R>
```

Stop conditions (any one -> power off, note it in STATE.md, do not continue):
hard contact, strong buzzing, cable tension, bending, unexpected motion,
a value that will not repeat.

End of a calibration day, if still on localStorage:

```
[ ] Export JSON + CSV from the calibrator
[ ] Copy config/head.json -> backups/<date>-head.json  BEFORE editing it
[ ] Update config/head.json from the export
[ ] python tools/validate.py  -> green before the session ends
[ ] Append the day's measurements to config/calibration-history.jsonl
```

---

## Loop 3 - build and upload firmware (only after >=1 channel changed)

Never edit calibration numbers directly in the .ino once head.json exists.

```
[ ] Servo power OFF (stays off through the whole loop)
[ ] python tools/validate.py            -> must be green, no exceptions
[ ] Backup first:
      copy the current .ino  -> backups/<date>-face_8_servo_uno.ino
      copy the current .hex  -> backups/<date>-last-known-good.hex
[ ] python tools/generate.py            -> regenerates head_config.h
    (until generate.py exists: hand-edit .ino + app.js + calibrator-page.js
     from head.json, then re-run validate.py to catch transcription errors)
[ ] Compile for arduino:avr:uno - note flash/RAM sizes
[ ] Upload to COM9 - avrdude must report verified
[ ] Reconnect; expect READY banner; send 0 -> OFF
[ ] STATE.md:  fw uploaded <date>, <N> bytes verified, from head.json as of <date>
```

Roll-back path (why the backups exist): if the new firmware misbehaves,
re-upload `backups/<date>-last-known-good.hex` with avrdude, confirm READY
and OFF, then debug on the bench - never on a moving face.

Then re-validate ONLY what the change touched, one channel at a time with
`K`, servo power on, before using any grouped command.

---

## Loop 4 - climb the verification ladder (after all 16 channels are green)

Strict order. Each step only after the previous passes. Record pass/fail
per step in STATE.md (or the ladder block in head.json once it exists).

```
1. [ ] Individual channels     - each CH via K, already done in Loop 2
2. [ ] Eye pairs               - C commands, horizontal then vertical together
3. [ ] Eyelids / winks / blink - B1/B0, E0-E3
4. [ ] Eyebrow + forehead      - paired C moves
5. [ ] Nose, jaw, cheeks       - individual C moves
6. [ ] All neutral             - command 2
7. [ ] Expressions             - command 1, watch full cycle x3
8. [ ] Auto blink              - A1, observe 2 min, A0
9. [ ] Camera following        - main UI, follow mode, 2 min
```

Any failure: send 0, drop back to Loop 2 for the offending channel, then
re-climb from the failed step (earlier green steps stay green unless the
channel's values changed).

**Demo-ready** = steps 1-9 green, timestamped AFTER the last firmware upload,
then two 5-minute demos from cold restart (power everything off, start from
Loop 1, run the demo script twice).

---

## Loop 5 - session end (every session, ~5 min, never skip)

```
[ ] Send 0 - outputs off; servo power OFF at the switch
[ ] If head.json changed today: copy it to backups/<date>-head.json
[ ] Update STATE.md: 3 lines - DONE today / NEXT action / BLOCKED on
[ ] If calibration values changed but firmware was NOT re-uploaded:
    say so explicitly in STATE.md - "head.json ahead of chip since <date>"
```

That last line is the one that prevents the next session from trusting a
board that is running yesterday's numbers.

---

## Which loop am I in? (quick dispatch)

| Situation | Loop |
|---|---|
| Just sat down | 1 |
| A channel still says needs_recheck | 2 |
| head.json changed since last upload | 3 |
| All 16 verified, ladder not green | 4 |
| About to stand up | 5 |

## This week against 4 October

| Day | Target |
|---|---|
| Sep 28 | Loop 0, then Loop 2 for CH0-CH3 (eyes) |
| Sep 29 | Loop 2 for CH4-CH9 (eyelids, brows), first Loop 3 upload |
| Sep 30 | Loop 2 for CH10-CH15, Loop 3, start Loop 4 |
| Oct 1  | Loop 4 complete; freeze calibration |
| Oct 2-3 | Voice/jaw work (separate design - the open risk) + demo rehearsal |
| Oct 4  | Two cold-restart demos |

Calibration slipping past Sep 30 eats the voice days. That is the trade to
watch, and the reason no non-essential tooling gets built this week.
