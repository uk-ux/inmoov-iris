# InMoov head - voice subsystem plan

**Created:** 2026-09-28
**Target:** milestone requirements 4 and 5 - listen and converse offline in
Tamil and English; speak offline with jaw movement synchronized to audio.
**Descope for the demo (per WORKFLOW_3DAY.md):** scripted/keyword
conversation, not open-ended chat. Full conversation is phase V3,
after the milestone.

Machine facts this plan is built on (checked 2026-09-28):

- Windows 11, Python 3.14.3 via `py`, 16 GB RAM
- NVIDIA RTX 3050 6GB laptop GPU (CUDA capable)
- `pyserial` and `sounddevice` already installed; no torch yet
- Mics: laptop mic array + realme Buds T200 headset
- Outputs: Realtek speakers + realme Buds (Bluetooth)
- Uno on COM9; jaw = CH11, calibrated 1300 us closed .. 1700 us open,
  rest 1300; `C11 <pulse>` moves the jaw and keeps the rest of the pose

---

## 1. Architecture

One Python app, `voice/app.py`, owning everything:

```
mic (push-to-talk) --> VAD trim --> STT faster-whisper (ta/en autodetect)
                                        |
                                        v
                              dialog table (keyword -> reply)
                                        |
                                        v
                    TTS -> WAV   (Piper for English, MMS-TTS for Tamil)
                          |
            +-------------+--------------+
            v                            v
     audio playback              RMS envelope, 50 ms frames
     (sounddevice)                       |
                                         v
                              jaw pulse map + smoothing
                                         |
                                         v
                              pyserial COM9: C11 <pulse> @ 20 Hz
```

Two hard rules inherited from the head workspace:

- **COM9 has one owner.** The browser UI and the voice app cannot hold the
  port at once. Disconnect (or close) the browser page before starting the
  voice app, and vice versa. The app must fail with a clear message if the
  port is busy, not retry.
- **Safe exit.** The app sends `0` (outputs off) in a `finally:` block on any
  exit, including Ctrl+C and crashes. It never moves anything at startup.

## 2. Component choices, with fallbacks

| Job | First choice | Why | Fallback |
|---|---|---|---|
| STT (both languages) | `faster-whisper`, model `small`, int8 on GPU | one model does Tamil and English with autodetect; RTX 3050 makes it ~1 s per short utterance | model `base` on CPU; `medium` if Tamil accuracy disappoints and GPU handles it |
| TTS English | Piper, `en_US-lessac-medium` | fast, light, entirely offline, good quality | Windows SAPI via pyttsx3 rendered to WAV |
| TTS Tamil | Meta MMS-TTS Tamil (`facebook/mms-tts-tam`, transformers + torch) | offline once downloaded, intelligible Tamil | eSpeak NG Tamil (robotic but guaranteed to work) |
| Dialog | hand-written keyword table, ta + en | deterministic for the demo, zero latency, zero risk | - |
| Conversation (V3, post-milestone) | Ollama + a small multilingual model (e.g. qwen2.5:3b) | runs in 6 GB VRAM, handles Tamil | keep keyword table |
| Jaw sync | RMS envelope of the rendered WAV | simple, robust, no phoneme alignment needed | - |

**Known risk - Python 3.14.** torch / ctranslate2 / onnxruntime wheels may
not exist yet for 3.14. Plan: create the voice venv with Python 3.12
(`py -3.12 -m venv voice\venv`); install 3.12 first if it is not on the
machine. Do not fight wheel availability on 3.14 - it is not worth an hour.

**Known trap - Bluetooth latency.** The realme Buds add 100-300 ms of audio
delay, which makes the jaw visibly lead the sound. For the demo, play
through the Realtek speakers (wired path), and use the laptop mic array or
the Buds only as the *input*. Select devices explicitly by index in config,
never rely on Windows defaults.

## 3. Jaw synchronization design

1. TTS renders the full sentence to a WAV file first (no streaming).
2. Compute RMS per 50 ms frame; normalize to the 95th percentile of the
   utterance so quiet voices still move the jaw.
3. Map envelope 0..1 to pulse **1300..1600 us** (not 1700 - full mechanical
   open looks like shouting and stresses the linkage; widen later if it
   looks too subtle).
4. Smoothing: fast attack (new frame wins immediately when louder), slow
   decay (~150 ms) so consonant gaps do not chatter the servo.
5. During playback, send `C11 <pulse>` at 20 Hz (every 50 ms). The firmware
   already applies ~90 ms easing per command, which suits this rate.
6. On utterance end: send `C11 1300` (closed), wait 200 ms.
7. Below a silence threshold send nothing - do not spam identical pulses
   (the firmware already skips identical ticks, but the serial line does not
   need the traffic).

## 4. Build phases and gates

### V0 - speak English with jaw sync (~2-3 h)

```
[ ] Install Python 3.12 if absent; create voice\venv with it
[ ] pip install faster-whisper piper-tts sounddevice soundfile numpy pyserial
[ ] Download Piper en_US voice; verify a WAV renders OFFLINE (airplane mode test)
[ ] jaw_sync.py: WAV -> envelope -> pulse list; DRY RUN mode prints pulses,
    no serial - verify numbers stay in 1300..1600
[ ] With servo power OFF: run against COM9, confirm READY + commands accepted
[ ] With servo power ON (calibrated CH11 only): speak one sentence,
    watch the jaw. Tune envelope gain + decay until it looks like talking.
```

**Gate:** a sentence plays through Realtek speakers and the jaw visibly
tracks the syllables. Do not proceed until this looks right.

### V1 - Tamil speech (~1-2 h)

```
[ ] pip install torch (CUDA wheel) + transformers; download facebook/mms-tts-tam
[ ] Render a Tamil test sentence to WAV offline; listen for intelligibility
[ ] If unusable: fall back to eSpeak NG Tamil and accept robotic voice for demo
[ ] Same jaw path - no changes needed (envelope is language-neutral)
```

**Gate:** one Tamil sentence, offline, jaw synced.

### V2 - listen and respond (~2-3 h)

```
[ ] Push-to-talk capture (hold SPACE / Enter to talk) - no wake word, no VAD
    complexity for the demo
[ ] faster-whisper transcribe with language autodetect; log detected language
[ ] dialog.py: keyword table, ~10 entries per language
      "hello" / "hi"            -> English greeting reply
      "vanakkam" / "வணக்கம்"     -> Tamil greeting reply
      "your name" / "peyar"     -> name reply in matching language
      "bye" / "poi varen"       -> farewell + jaw close
      no match                  -> polite fallback in detected language
[ ] Reply through the matching-language TTS + jaw sync
[ ] finally: outputs off + port close on every exit path
```

**Gate:** cold start -> push-to-talk "vanakkam" -> Tamil reply with jaw
movement -> "hello" -> English reply. Twice in a row.

### V3 - post-milestone (NOT before 4 October)

Ollama-based free conversation, wake word, VAD instead of push-to-talk,
barge-in, emotion-matched expressions during speech.

## 5. Download checklist (do while online, before demo day)

| Item | Approx size |
|---|---|
| Python 3.12 installer (if needed) | 25 MB |
| faster-whisper `small` model | ~460 MB |
| Piper + en_US-lessac-medium voice | ~80 MB |
| torch CUDA + transformers | ~3 GB |
| facebook/mms-tts-tam | ~400 MB |
| eSpeak NG (fallback) | 10 MB |

After downloading, run every component once with Wi-Fi off. "Offline" that
has never been tested offline is not offline.

## 6. File layout

```
voice/
  venv/            (Python 3.12)
  config.py        COM port, device indices, jaw pulse range, model paths
  jaw_sync.py      WAV -> envelope -> C11 pulse stream (+ DRY RUN mode)
  tts.py           speak(text, lang) -> WAV path   (piper | mms | espeak)
  stt.py           record while key held -> (text, language)
  dialog.py        keyword table -> reply text + language
  app.py           main loop, serial ownership, safe shutdown
  demo_script.md   the exact segments spoken in the two 5-minute demos
```

`config.py` holds the jaw range as `JAW_CLOSED_US=1300, JAW_MAX_US=1600` -
**update these from the real CH11 calibration after the run**, do not trust
the baseline blindly.

## 7. Order of operations against the head schedule

Voice work must not touch the head until CH11 is calibrated. But V0's
install + dry-run needs **no hardware at all** - it can happen in parallel
with calibration, on battery, anywhere. Only the final V0 gate (jaw moving)
waits for a calibrated CH11 and the verification ladder's individual-channel
step.

Suggested interleave: start model downloads now (they are the slowest,
least-attended step), do V0 dry-run while calibration is in progress,
V0 gate + V1 + V2 after the ladder's step 5 (jaw verified).
