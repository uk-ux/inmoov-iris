# IRIS — the InMoov voice assistant

**IRIS** (the eye's iris, and Iris the Greek messenger goddess) is the
laptop app. **InMoov** is the robot it drives.

Offline voice chatbot: speak Tamil or English, InMoov thinks locally and
replies aloud in your language. UI is the orb page with a chat panel.

## Start everything

Double-click `U:\inmoov\START_VOICE.bat`
(or manually: start Ollama, then `voice\venv\Scripts\python.exe voice\server.py`,
then open http://127.0.0.1:8765/orb.html in Chrome/Edge).

Hold the button (or Space) → talk → release. Or type in the chat box.

**Voice tab** (top of the chat panel): pick between 6 English voices and
test them. Tamil always uses its own voice. Your choice is remembered.

**Web search**: when internet is available, questions are enriched with live
DuckDuckGo results ("searching the web" shows under the orb), so InMoov can
answer about current events. Offline, it answers from the model alone —
nothing breaks. Turn off with WEB_SEARCH = False in voice/server.py.

## Stack (all free, all local)

| Job | Component |
|---|---|
| Speech to text | faster-whisper `small` on **GPU** (0.3 s), Tamil/English autodetect |
| Thinking | Ollama + `qwen3:8b` with thinking disabled — far better Tamil grammar than qwen2.5 |
| Voice out, English | **Kokoro** neural TTS (6 natural voices, default Heart) + Piper as fallback |
| Voice out, Tamil | Meta MMS-TTS `facebook/mms-tts-tam` (Piper has no Tamil voice) |
| UI | `control_ui/orb.html` — animated orb + chat panel, WebSocket to 8766 |

Reply language follows the reply text: Tamil script → MMS Tamil voice,
otherwise Piper English. The LLM is instructed to answer in the user's
language.

## Languages

Speak or type any of these; IRIS detects the language and replies in it,
each with its own voice:

- **English** — Kokoro neural voice
- **Tamil, Telugu, Hindi** — ready (MMS voices, downloaded)
- Kannada, Malayalam, Bengali, Marathi, Gujarati, Punjabi, Urdu — download on
  first use

Whisper detects the spoken language; the reply is routed to a voice by its
script (Telugu letters -> Telugu voice, and so on).

## Skills (say these — they run instantly, before the model)

- **Head** (needs motion enabled): look around / left / right / up / down /
  at me, blink, smile, look sad, look surprised, neutral, stop moving
- **Diagnostics**: run diagnostics, status report
- **Laptop**: open notepad / chrome / calculator / camera, volume up / down,
  mute, screenshot, lock the screen, battery, what time is it, date

Enable head motion in the **Skills** tab (COM9). The browser calibrator must
be disconnected first — one program owns the port.

## Model cache location

All AI models are cached on **U:** (`voice/models/hf`) because C: was full.
Set by `HF_HOME` at the top of server.py; do not remove it.

## Ports

- 8765 — static UI (http.server)
- 8766 — voice WebSocket (`voice/server.py`)
- 11434 — Ollama

## Jaw sync (later)

`server.py` has a built-in jaw driver (`JAW_SERIAL = None` at the top).
After CH11 is calibrated AND the browser calibrator is disconnected from
COM9, set `JAW_SERIAL = "COM9"` and update `JAW_CLOSED_US` / `JAW_MAX_US`
from the real calibration. The server then streams `C11` pulses that follow
the speech envelope while the browser plays the audio. It sends `0`
(outputs off) on exit.

## Offline check before demo day

Turn Wi-Fi off and do one full round-trip in each language. Models are
cached after first use: whisper in `%USERPROFILE%\.cache\huggingface`,
MMS likewise, Piper in `voice/models/`, qwen in `%USERPROFILE%\.ollama`.

## If something breaks

- Orb page says "voice server offline" → the `InMoov voice server` window
  is closed or crashed; rerun START_VOICE.bat and read its window.
- Replies never come → Ollama not running or model missing:
  `ollama list` should show `qwen3:8b`, else `ollama pull qwen3:8b`.
- Mic does nothing → Chrome site permissions → Microphone → Allow for
  127.0.0.1; and check Windows Settings > Privacy > Microphone.
- Tamil reply sounds robotic/fails → first Tamil reply loads MMS (~20 s);
  watch the server window.

## Tuning knobs (top of voice/server.py)

- `LLM_MODEL` — `qwen3:8b` (best Tamil) · `qwen2.5:7b` (faster, weak Tamil) · `qwen2.5:3b` (fastest)
- `SPEECH_SPEED` — 0.92 = calm. Lower = slower/calmer, 1.0 = normal.
- `WEB_SEARCH` — False turns off internet enrichment (faster, fully offline).
- `SYSTEM_PROMPT` — InMoov's personality; currently calm, gentle, unhurried.

Replies stream sentence-by-sentence, so speech starts before the full answer
is written. Voice choice is per browser tab (Voice tab), not global.
