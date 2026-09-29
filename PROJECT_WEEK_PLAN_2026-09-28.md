# InMoov head - one-week completion workflow

**Week:** Monday 28 September through Sunday 4 October 2026  
**Goal:** Finish a demonstrable InMoov head that can listen and converse offline
in Tamil and English while using safe, calibrated facial movement.

## What "finished this week" means

The project is complete for this milestone when all of these checks pass:

- [ ] Uno and PCA9685 start reliably with outputs off.
- [ ] All 16 face servos have confirmed safe limits and rest positions.
- [ ] Manual UI control, Outputs off and All neutral work reliably.
- [ ] Eyes, eyelids, eyebrows, forehead, nose, jaw and cheeks pass group tests.
- [ ] Synchronized blink, auto blink and at least three expressions work.
- [ ] Camera face following works without servo binding or I2C faults.
- [ ] Offline speech recognition understands short Tamil and English sentences.
- [ ] A local conversation model produces short replies without internet.
- [ ] Offline speech output speaks both Tamil and English understandably.
- [ ] Jaw moves with speech audio; blinking and mild expressions continue safely.
- [ ] A five-minute demonstration runs twice without restarting the system.
- [ ] Final calibration, firmware, UI and startup instructions are backed up.

## Project flow

```mermaid
flowchart LR
    A[Inspect wiring and mechanics] --> B[Connect Uno and PCA]
    B --> C[Calibrate 16 servos individually]
    C --> D[Validate groups and expressions]
    D --> E[Test camera tracking]
    E --> F[Offline Tamil and English speech recognition]
    F --> G[Local conversation model]
    G --> H[Offline Tamil and English voice]
    H --> I[Jaw and expression synchronization]
    I --> J[Full demonstration and backup]
```

Do not move to a later block while the previous block has an unresolved fault.

## Monday 28 September - restore and inspect

**Target:** Know that the electronics and every mechanism are safe to test.

- [ ] Photograph the front, left, right and rear wiring before changing anything.
- [ ] Inspect servo horns, screws, linkages, cables and printed parts with power off.
- [ ] Verify Uno-to-PCA wiring and external supply common ground.
- [ ] Connect the Uno USB cable and identify its new COM port.
- [ ] Open http://127.0.0.1:8765/ in Chrome or Edge.
- [ ] Confirm firmware startup: `READY calibrated 16-servo face - outputs OFF`.
- [ ] Confirm **Outputs off** works before enabling servo power.
- [ ] Record any broken, loose, reversed or noisy mechanism.

**Exit check:** Controller connects, PCA responds, outputs remain off at startup,
and no mechanical repair remains unknown.

## Tuesday 29 September - calibrate all 16 channels

**Target:** Produce one reviewed calibration record for the entire face.

- [ ] Follow `hardware/HEAD_CALIBRATION_WORKFLOW.md` exactly.
- [ ] Calibrate CH0-CH3: both eyes.
- [ ] Calibrate CH4-CH7: four eyelids.
- [ ] Calibrate CH8-CH9: eyebrows.
- [ ] Calibrate CH10: nose.
- [ ] Calibrate CH11: jaw.
- [ ] Calibrate CH12-CH13: forehead.
- [ ] Calibrate CH14-CH15: cheeks.
- [ ] Export `inmoov-calibration-drafts.json` from the UI.
- [ ] Update `hardware/HEAD_CALIBRATION_LOG.csv` with date, status and notes.

**Exit check:** Every channel reaches two safe endpoints and returns to a quiet,
repeatable rest position without touching a hard stop.

## Wednesday 30 September - integrate and validate movement

**Target:** One stable firmware/UI calibration and reliable facial behaviour.

- [ ] Review the exported values before changing code.
- [ ] Update both `firmware/face_8_servo_uno/face_8_servo_uno.ino` and
  `control_ui/app.js`.
- [ ] Compile and upload with external servo power off.
- [ ] Run the validation sequence from the calibration workflow.
- [ ] Correct eyelid timing and synchronized blink if required.
- [ ] Verify All neutral, Outputs off, three expressions and auto blink.
- [ ] Test camera face following for two continuous minutes.
- [ ] Save the final working firmware image and calibration record.

**Exit check:** Five consecutive neutral-expression-neutral cycles and ten blinks
complete without binding, resets, I2C errors or visibly incorrect direction.

## Thursday 1 October - offline listening and conversation

**Target:** The laptop understands speech and creates a local text reply.

- [ ] Record laptop CPU, RAM, GPU and available disk space.
- [ ] Install or select a multilingual offline speech recognizer.
- [ ] Test five English and five Tamil sentences from the actual microphone.
- [ ] Record recognition accuracy and response time.
- [ ] Install or select a local conversation model sized for the laptop.
- [ ] Add a short system prompt for friendly, concise Tamil/English replies.
- [ ] Test English, Tamil and mixed-language text conversations offline.
- [ ] Add an emergency Stop command that sends `0` to the Uno.

**Exit check:** With internet disabled, one program converts microphone speech to
text and produces an appropriate text reply in the detected language.

## Friday 2 October - offline voice and jaw movement

**Target:** InMoov speaks understandable Tamil and English and moves its jaw.

- [ ] Install or select offline Tamil/English text-to-speech.
- [ ] Choose one consistent voice and comfortable speaking speed.
- [ ] Test five Tamil and five English replies through the final speaker.
- [ ] Measure audio level or envelope during playback.
- [ ] Map audio level to the calibrated CH11 jaw range.
- [ ] Smooth jaw motion and return it to the closed/rest pulse after speech.
- [ ] Keep jaw pulses inside the reviewed calibration limits.
- [ ] Stop microphone listening while the robot speaks to prevent feedback.

**Exit check:** Ten sentences play without clipping; jaw motion follows syllables,
closes after speech and never reaches a mechanical hard stop.

## Saturday 3 October - full behaviour integration

**Target:** One application coordinates listening, thinking, speaking and movement.

- [ ] Use these states: Idle, Listening, Thinking, Speaking and Emergency stop.
- [ ] Idle: neutral face and occasional auto blink.
- [ ] Listening: look toward the person and pause robot speech.
- [ ] Thinking: small safe eye movement or thoughtful expression.
- [ ] Speaking: play TTS, move jaw and use one mild expression.
- [ ] Emergency stop: stop audio and send `0` immediately.
- [ ] Add visible status text to the UI for the current state and transcript.
- [ ] Run English-only, Tamil-only and mixed-language conversations.
- [ ] Test microphone feedback, silence, interruption and missing-face cases.

**Exit check:** Complete ten conversation turns offline with no unsafe servo
movement, feedback loop, application crash or manual serial reconnection.

## Sunday 4 October - acceptance test and handoff

**Target:** Demonstrate, document and preserve the finished milestone.

- [ ] Cold start the laptop, Uno, PCA9685 supply and speaker from powered off.
- [ ] Follow the startup instructions without using development tools.
- [ ] Run the five-minute demonstration twice.
- [ ] Demonstrate English, Tamil, blinking, expressions and emergency stop.
- [ ] Record a short final video showing the full system.
- [ ] Write exact start/stop commands and model locations.
- [ ] Save the final firmware, UI, calibration JSON and conversation software.
- [ ] Create `.gitignore`, review files for secrets and large generated content,
  then connect and back up the project to GitLab if time remains.

**Exit check:** A fresh restart succeeds twice and another person can follow the
written startup steps.

## Daily working method

Use the same small loop for every task:

1. **Prepare:** write the single result expected from the session.
2. **Change one thing:** one servo, one model or one integration point.
3. **Test immediately:** use a short repeatable test.
4. **Record evidence:** value, result, error message, screenshot or short video.
5. **Save a checkpoint:** update the log before starting the next task.

If a task remains blocked for more than 60 minutes, record the exact blocker,
use the simplest safe fallback that still satisfies the weekly demonstration,
and continue along the critical path.

## Scope control for this week

These items belong to a later milestone unless all acceptance checks above are
already complete:

- Digital-twin assembly correction.
- Neck rotation or new servos.
- Full-body integration.
- Custom voice training or voice cloning.
- Cloud conversation services.
- Cosmetic redesign and additional printed mechanisms.
- Advanced long-term memory or person identification.

## Feature ideas and priority

### Implement this week

1. **Conversation state display**  
   Show `Idle`, `Listening`, `Thinking`, `Speaking` and `Stopped` in the control
   UI. This makes faults easy to diagnose and makes the demonstration clearer.

2. **Emergency stop at two levels**  
   Keep the UI Stop button that sends `0`, and keep a physical switch that cuts
   external servo power. The physical switch must remain reachable during every
   test.

3. **Simple mood-to-expression mapping**  
   Map a reply to one of a few reviewed expressions: Happy, Curious, Concerned,
   Surprised or Calm. Use only one mild expression per reply and always return to
   neutral.

4. **Push-to-talk mode**  
   Add a large microphone button. Press to listen, release to process. This is
   easier to finish reliably than always-listening speech and prevents the robot
   from hearing its own speaker.

5. **System health panel**  
   Display Uno connection, PCA status, microphone status, speech model status,
   conversation model status and voice model status. A failed component should
   show a useful error instead of leaving the robot silent.

### Add after the weekly demonstration

- **Local memory:** remember a person's name and a few preferences in a local
  file, with a Clear memory button.
- **Offline knowledge folder:** answer questions from selected local PDF or text
  documents without internet.
- **Eye LEDs:** use color or brightness to show Listening, Thinking and Speaking.
- **Servo protection:** add current monitoring so a stalled servo can trigger
  Outputs off automatically.
- **Microphone array:** estimate which person is speaking and turn attention in
  that direction.
- **Touch interaction:** a capacitive sensor on the hand or head can trigger a
  greeting or pause.
- **Neck rotation:** add it only after mechanical limits, power requirements and
  an emergency stop are independently validated.
- **Local face recognition:** optional named-person greeting with explicit local
  enrollment and a Delete faces control.

The recommended demonstration feature set is push-to-talk, bilingual offline
conversation, audio-driven jaw movement, natural blinking, one mild expression
per response, the state display and emergency stop.

## Current starting point

- [x] Combined 16-channel Uno/PCA firmware exists.
- [x] Manual control UI exists.
- [x] Camera face tracking, expressions and auto blink exist in the UI.
- [x] A safe channel-by-channel calibration workflow and log exist.
- [ ] Uno is currently detected and connected for this week's test.
- [ ] Current physical calibration is revalidated after storage.
- [ ] Offline conversation software exists in this workspace.
- [ ] GitLab backup is connected.

The first active task is **Monday: connect the Uno, identify its COM port and
confirm outputs-off startup**.
