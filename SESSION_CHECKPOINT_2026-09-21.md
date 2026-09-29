# InMoov conversation checkpoint - 2026-09-21

This is a saved summary, not a verbatim conversation transcript.

## Current hardware and UI

- Laptop controls Arduino Uno and PCA9685; 16 face servos.
- UI: `U:\inmoov\control_ui\index.html`
- Local URL: http://127.0.0.1:8765/
- Firmware source: `U:\inmoov\firmware\face_8_servo_uno\face_8_servo_uno.ino`
- Firmware image: `U:\inmoov\firmware\face_8_servo_uno\face_8_servo_uno.updated.hex`
- Detailed calibration and wiring: `SESSION_CHECKPOINT_2026-09-17.md`.
- Digital twin remains paused by user request.

## Today's checks and upload

User returned after three days and wanted to check the head before voice work.
The browser showed a firewall/connection warning. No local listener was found
on port 8765. Started the server using:

```powershell
py -m http.server 8765 --bind 127.0.0.1 --directory control_ui
```

Run this from `U:\inmoov` if the server needs restarting. HTTP returned 200.
Server availability must be checked again in a future session; it is not a
permanent startup service. No firewall settings were changed.

At the user's request, uploaded the saved September 17 firmware image to COM9
using avrdude on September 21. Upload succeeded with exit code 0 and all 10,070
flash bytes read back and verified. No firmware source changes or new compilation
were performed. No servo movement commands were sent and physical movement has
not yet been confirmed by the user. The saved firmware starts with outputs off.
Next: open the UI, Connect Uno, select COM9, and test the head with the user.

## Offline conversation plan - not implemented

User wants InMoov to converse naturally with people in both Tamil and English,
preferably completely offline. Initial software/model downloads may require
internet; runtime should be local on the laptop. Proposed components:

- Multilingual Whisper via whisper.cpp for speech recognition.
- A local conversation model such as Gemma 3 via Ollama.
- AI4Bharat Indic Parler-TTS as a Tamil/English speech-output candidate.
- Later integration with the existing UI/Uno for audio-driven jaw movement,
  blinking and occasional facial expressions.

These are candidates, not installed or benchmarked components. Mixed Tamil and
English accuracy and response latency require testing. Laptop CPU, RAM and GPU
specifications were requested but have not been supplied or checked. User then
prioritized testing the head. No voice code, model downloads or installation
were performed. Resume hardware checks before voice integration.

Primary references checked:
- https://github.com/ggml-org/whisper.cpp
- https://ollama.com/library/gemma3
- https://huggingface.co/ai4bharat/indic-parler-tts
