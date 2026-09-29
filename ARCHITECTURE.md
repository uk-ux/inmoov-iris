# InMoov head - workflow architecture

**Created:** 2026-09-28
**Status:** Proposal. Nothing in this document has been implemented yet.
**Scope:** How work flows through this workspace. It does not change any
calibration value, wiring decision or safety rule from
`INMOOV_AGENT_HANDOFF.md`.

Read `INMOOV_AGENT_HANDOFF.md` for the current authoritative project state.
This file explains why that handoff file has to be so long, and what to build
so it can be short.

---

## 1. What is actually wrong

The hardware design is sound and the firmware safety behaviour is good: the
`K` command isolates one channel, out-of-range pulses are rejected, outputs
start off. The problem is not the robot. The problem is that **the robot's
physical facts are stored as hand-copied constants in five places, and the
rules that keep them honest are stored as English prose.**

### 1.1 Calibration data is copy-pasted into four live sources

| Source | Form |
|---|---|
| `firmware/face_8_servo_uno/face_8_servo_uno.ino` | `CH[]`, `NEUTRAL_US[]`, `MINIMUM_US[]`, `MAXIMUM_US[]`, `LID_CLOSED_US[]` |
| `control_ui/app.js` | `servos[]` with `low` / `high` / `neutral` |
| `control_ui/calibrator-page.js` | `SERVOS[]` with `aUs` / `restUs` / `bUs` |
| `hardware/HEAD_CALIBRATION_LOG.csv` | one row per channel |

Plus the table in `INMOOV_AGENT_HANDOFF.md`, plus the explicitly superseded
`hardware/servo_inventory.csv` still sitting on disk next to the good data.

All four live sources were cross-checked on 2026-09-28 and **currently agree**
on all 16 channels. That is the good news and it is also the warning: the
upcoming CH0-CH15 revalidation run replaces 48 numbers, which means about 190
hand edits across four files, with no checker.

### 1.2 The duplication has already caused a silent divergence

The five firmware expression poses and the six `app.js` gestures are the same
expressions, entered twice. Four of the five match exactly. One has drifted:

| Expression | Channel | UI value | Firmware value | Difference |
|---|---|---|---|---|
| Curious / "smiling glance right" | CH11 jaw | 1360 us | 1480 us | -120 us |
| | CH14 left cheek | 1420 us | 1400 us | +20 us |
| | CH15 right cheek | 1580 us | 1680 us | -100 us |

The firmware version of this expression additionally moves the CH4 and CH5
eyelids, which the UI version does not touch at all. Pressing "Curious" in the
browser and letting the onboard animation reach the same pose therefore
produce two visibly different faces. Nobody chose that. It is what copy-paste
does over four weeks.

### 1.3 The calibrator's output is a dead end

The measurement loop today is:

```
move servo -> record -> browser localStorage -> download JSON/CSV
  -> an agent reads the download -> hand-edits the .ino
  -> hand-edits app.js -> hand-edits calibrator-page.js
  -> hand-edits the CSV log -> hand-edits the handoff table
  -> eyeball "is every neutral between its endpoints"
  -> compile -> upload -> report the result in chat
```

Measured physical truth leaves the machine's data model and only re-enters it
through human transcription. `calibrator-page.js` even writes
`applied_to_firmware: false` into each saved record, and nothing in the
workspace can ever flip it to `true`, because nothing downstream reads the
file.

### 1.4 Correctness rules are prose, not code

Real invariants currently enforced only by an agent remembering to read them:

- rest must lie between the two endpoints
- expression poses must lie inside each channel's range - the firmware
  `constrain()` call silently clips a bad pose instead of reporting it
- do not run All neutral, animation, blink, expressions or camera following
  until every channel is physically revalidated
- eyelid closed values must be inside the eyelid's range

The third one is the important one. It is the difference between a safe
session and a stripped gear, and today it is a sentence in a markdown file.

### 1.5 The handoff file is five documents with five different lifetimes

`INMOOV_AGENT_HANDOFF.md` contains, in one hand-edited file: immutable safety
rules, stable wiring facts, a semi-stable protocol table, volatile calibration
data, and per-session state with a next action. It has to be rewritten every
session because of the last part, which means every session's edit passes
through the safety rules. A reader cannot tell which lines are load-bearing
and which are yesterday's status.

### 1.6 No version control, and no record of what is on the chip

`U:\inmoov` is not a git repository. Consequences:

- "This file supersedes conflicting values in older notes" is doing the job
  that commit history does for free.
- Eleven root-level markdown files are serving as a hand-maintained changelog.
- The only record of what is flashed on the Uno is the sentence
  "last restored firmware image: `face_8_servo_uno.updated.hex`". There is no
  link from that hex to the source or the calibration values it was built from.
- A bad calibration upload cannot be rolled back to the last good state.
- 17 sketches and 15 `build_*` directories (about 12,000 lines of generated
  `compile_commands.json`) sit beside the sources with no marker for which
  sketch is live.

---

## 2. The architecture

One principle:

> **The robot's physical facts live in exactly one machine-readable file.
> Every other representation is generated from it or validated against it.**

Everything below follows from that.

```
                      config/head.json
              (single source of truth, git-tracked)
                             |
        +--------------------+--------------------+
        |                    |                    |
   generated            fetched at            read by
        |                 runtime                 |
        v                    v                    v
 firmware/head_face/    control_ui/          tools/validate.py
   head_config.h        app.js +              (gate: no upload
   (DO NOT EDIT)        calibrator-page.js     unless clean)
        |                    |
        v                    ^
   compile + upload          |  POST /api/channel writes measured
        |                    |  values straight back into head.json
        +--------------------+
           tools/serve.py  (local HTTP + write-back API)
```

### 2.1 `config/head.json` - the single source of truth

Replaces: the handoff calibration table, `HEAD_CALIBRATION_LOG.csv`,
`servo_inventory.csv`, and the three hard-coded arrays.

```json
{
  "schema": 1,
  "controller": { "board": "arduino:avr:uno", "port": "COM9", "baud": 115200 },
  "driver": { "kind": "PCA9685", "i2c": "0x40", "pwm_hz": 50 },
  "protocol": 1,
  "safety": { "absolute_min_us": 1000, "absolute_max_us": 2000,
              "min_travel_us": 40 },

  "channels": [
    {
      "ch": 0,
      "servo": 1,
      "part": "Left eye horizontal",
      "group": "eyes",
      "endpoint_a": { "label": "Left",  "us": 1300 },
      "endpoint_b": { "label": "Right", "us": 1700 },
      "rest_us": 1477,
      "rest_is_endpoint": null,
      "verified": { "state": "needs_recheck", "at": null, "notes": "" }
    },
    {
      "ch": 4,
      "servo": 5,
      "part": "Left upper eyelid",
      "group": "eyelids",
      "endpoint_a": { "label": "Open",   "us": 1322 },
      "endpoint_b": { "label": "Closed", "us": 1500 },
      "rest_us": 1322,
      "rest_is_endpoint": "a",
      "verified": { "state": "needs_recheck", "at": null, "notes": "" }
    }
  ],

  "expressions": [
    { "name": "Curious", "pose": { "0": 1555, "1": 1495, "11": 1480 } }
  ]
}
```

Four schema decisions that each remove a whole class of bug:

**Pulses in microseconds only.** The `0-180` value is computed for display and
never stored. The handoff file currently needs a paragraph explaining that
`0-180` is not degrees; that paragraph exists because the number got stored.
Delete the stored number and the confusion has nowhere to live.

**`rest_is_endpoint`.** Eyelids and the jaw legitimately rest *on* an
endpoint. Declaring that lets the validator enforce "rest strictly between
endpoints" for the other twelve channels instead of weakening the rule for all
sixteen.

**Direction is derived, never stored.** `reversed` is
`endpoint_b.us < endpoint_a.us`. A servo cannot be marked reversed and have
ascending endpoints at the same time, because there is only one place to say
it.

**Expressions live here too**, as `channel -> microseconds`. Both the firmware
poses and the UI gestures come from this one list, which is the fix for the
Curious divergence in section 1.2.

### 2.2 Generate the firmware header; let the browser fetch the JSON

`tools/generate.py` emits **one** artifact:

`firmware/head_face/head_config.h` - a generated header with a
`// GENERATED FROM config/head.json - DO NOT EDIT` banner and the config's
content hash in a comment, containing `NEUTRAL_US[]`, `MINIMUM_US[]`,
`MAXIMUM_US[]`, `LID_CLOSED_US[]` and the expression pose table.

**Emitted in channel order 0-15.** The current firmware carries
`CH[] = {0,1,2,3,4,5,6,7,8,9,12,13,10,11,14,15}`, so array index 10 is channel
12 and index 12 is channel 10. The values happen to be correct today. Any
human editing "channel 10's neutral" by counting to the tenth slot writes the
nose value into the forehead. Generating in channel order deletes the `CH[]`
indirection and the hazard with it.

The browser does **not** get a generated file. `app.js` and
`calibrator-page.js` `fetch('/config/head.json')` at startup and build their
`servos` arrays from it. Generated JavaScript would be a fourth copy that can
go stale; a runtime fetch cannot. Only C++ needs codegen, because it needs
compile-time constants.

Net effect: four hand-maintained copies become one file plus one generated
header.

### 2.3 `tools/validate.py` - the gate

Runs before every compile. Non-zero exit blocks the build. Checks:

1. exactly 16 channels, ids unique and within 0-15
2. `rest_us` inside `[min(a,b), max(a,b)]`, and strictly inside unless
   `rest_is_endpoint` says otherwise
3. `abs(a - b) >= safety.min_travel_us` - catches a mistyped endpoint
4. every pulse inside `safety.absolute_min_us .. absolute_max_us`
5. **every expression pose value inside its channel's endpoints** - the check
   the firmware currently hides behind a runtime `constrain()`
6. eyelid closed values inside their channel's range
7. regenerate `head_config.h` in memory and diff it against the file on disk,
   so a stale header can never be compiled
8. report how many channels have `verified.state == "verified"`, and stamp the
   build `unverified` if any do not

Check 7 is what makes `applied_to_firmware` a computed fact instead of a field
nobody updates.

### 2.4 `tools/serve.py` - close the measurement loop

Replaces `py -m http.server 8765`. About a hundred lines of standard-library
Python:

| Route | Behaviour |
|---|---|
| `GET /` and static | serves `control_ui/` exactly as today |
| `GET /config/head.json` | the source of truth |
| `POST /api/channel/<ch>` | validates one channel record, writes it into `config/head.json` atomically, appends the measurement to `config/calibration-history.jsonl` |

localStorage stays, as a crash-safe draft. But "Save this calibration" now
means *written into the repository file*, not *saved in this browser*. The
download buttons stay for sharing; they stop being the only way out.

The transcription step - the single largest source of error in this workspace -
disappears.

**Deliberately out of scope for now:** moving the serial port into Python. It
would give one protocol implementation and one command log instead of Web
Serial in two pages, but Web Serial works today and the milestone is six days
out. Revisit after 4 October.

### 2.5 The verification ladder as enforced state

The twelve-step validation order in the handoff file is correct. Make it data:

```json
"ladder": [
  { "step": "channels_individual", "requires": [],
    "state": "pending", "at": null },
  { "step": "eye_pairs", "requires": ["channels_individual"],
    "state": "pending", "at": null },
  { "step": "eyelids_winks_blink", "requires": ["channels_individual"],
    "state": "pending", "at": null },
  { "step": "all_neutral", "requires": ["channels_individual"],
    "state": "pending", "at": null },
  { "step": "expressions", "requires": ["all_neutral"],
    "state": "pending", "at": null },
  { "step": "auto_blink", "requires": ["eyelids_winks_blink"],
    "state": "pending", "at": null },
  { "step": "camera_following", "requires": ["eye_pairs", "all_neutral"],
    "state": "pending", "at": null }
]
```

`app.js` reads this on load and **disables** the All neutral, animation, blink,
expression and camera-follow controls until their prerequisites are green. The
rule from section 1.4 stops depending on an agent's memory and starts
depending on the config file.

It also gives milestone requirement 6 - two five-minute demos after a cold
restart - a definition: every ladder step green, timestamped after the last
firmware upload.

### 2.6 Split the documents by lifetime

| File | Lifetime | Contents |
|---|---|---|
| `README.md` | stable | what this is, how to start, where things live |
| `docs/SAFETY.md` | immutable | the safety rules, unchanged |
| `docs/HARDWARE.md` | stable | wiring, power, I2C, grounds |
| `docs/PROTOCOL.md` | semi-stable | the serial command table + protocol number |
| `docs/CALIBRATION.md` | semi-stable | the per-channel procedure |
| `STATE.md` | every session | done / next / blocked. Short. No data tables. |
| `docs/archive/` | frozen | the six session checkpoints, `servo_inventory.csv` |

`INMOOV_AGENT_HANDOFF.md` shrinks to roughly twenty-five lines: read
`docs/SAFETY.md`, then `STATE.md`, then `config/head.json`. It gets short
because the calibration table and the session state are no longer prose.

The protocol number matters more than it looks: `app.js` can send a version
handshake on connect and refuse to drive a board whose firmware predates the
config it was built from.

### 2.7 Git, and a build record

```bash
cd /u/inmoov && git init
```

Track: `config/`, `tools/`, `docs/`, `firmware/*/*.ino`, `firmware/*/*.h`,
`control_ui/*.js`, `control_ui/*.mjs`, `control_ui/*.html`,
`control_ui/*.css`.

`.gitignore`:

```gitignore
firmware/build_*/
control_ui/vendor/
control_ui/twin/vendor/
control_ui/twin/assets/
hardware/twin-browser-test/
hardware/twin-tests/node_modules/
*.tgz
*.hex
```

Two commit conventions that make history answer the questions the eleven
markdown files are answering today:

```
cal(ch07): verified 1322 open / 1500 closed, rest 1322
fw: uploaded to COM9, config a91f3c2, 10070 bytes verified
```

One commit per verified channel means the calibration run becomes sixteen
reviewable commits, and one bad channel reverts on its own without touching
the fifteen good ones.

Firmware layout: one live sketch at `firmware/head_face/`, diagnostics at
`firmware/tools/` (`pca9685_safe_test_uno`, `pca_i2c_diagnostic_uno`), the
other fifteen sketches moved to `firmware/archive/`. Which one is live becomes
a directory name instead of a claim in a document.

---

## 3. Before and after

**Per channel, today:** select, move, record, localStorage. Then after all
sixteen: export, read the export, hand-edit the `.ino`, hand-edit `app.js`,
hand-edit `calibrator-page.js`, hand-edit the CSV log, hand-edit the handoff
table, eyeball the invariants, compile, upload, report in chat.

**Per channel, after:** select, move, record. The POST writes `head.json`,
appends to the history log, and the channel gets its own commit.

**Once, at upload time:**

```bash
python tools/build.py        # validate -> generate -> compile -> upload -> record
```

No transcription anywhere in the loop.

---

## 4. Sequencing against 4 October

Six days out, with the calibration run not yet started and offline voice not
yet implemented. Nothing here is worth doing if it costs demo days, so the
order is strictly "what unblocks the run that starts now".

**Before touching CH0 - about one hour.** `git init`, commit the current
state, add `.gitignore`. Write `config/head.json` by extracting the existing
values; they were verified consistent across all four sources on 2026-09-28,
so the extraction is mechanical and safe. Write `tools/validate.py`. This
alone protects the sixteen-channel run.

**Day 0 to 1 - two to three hours.** `tools/serve.py` with `/api/config` and
`/api/channel`, and point `calibrator-page.js` at it. From here the run writes
to the file instead of to a download.

**During the run.** `tools/generate.py` and `tools/build.py`. Needed only at
the first upload, which is after several channels are recorded.

**After the run, before the demo.** Ladder gating in `app.js`. Move the
expression poses into `head.json`, which fixes the Curious divergence. Split
the documents.

**Not before 4 October.** Serial ownership in Python. Digital twin
correction. Neck rotation. GitLab.

### The unscoped risk

Milestone requirements 4 and 5 - offline Tamil and English conversation, and
speech with jaw synchronisation - are not implemented and are not addressed by
this document. They are a separate subsystem, and with six days left they are
a larger schedule risk than calibration is.

Where they plug into this architecture: a `voice/` service owning offline STT
and TTS locally, reading the jaw channel's endpoints from `config/head.json`
so jaw travel during speech is bounded by the same measured values as
everything else, and driving the jaw through the same command path as the UI.
That service needs its own design pass, which this document does not attempt.

---

## 5. What this does not change

- Every calibration value in `INMOOV_AGENT_HANDOFF.md` stays exactly as it is.
  This architecture moves those numbers into one file; it does not alter them.
- Every safety rule stays in force, unchanged.
- The wiring stays as documented.
- The firmware's existing safety behaviour - outputs off at boot, `K`
  isolation, range rejection - is kept. Generation changes where the constants
  come from, not what the firmware refuses to do.
- `hardware/servo_inventory.csv` is archived, not corrected. It describes an
  early superseded design and should never be used as a channel map.
