"""InMoov voice chatbot backend.

Browser (orb.html) <-- WebSocket 8766 --> this server.
Pipeline: PCM from browser -> faster-whisper STT (ta/en autodetect)
       -> Ollama chat (streamed) -> TTS (Piper en / MMS-TTS ta) -> WAV back.

Everything runs locally. Jaw output is optional and off by default because
the browser calibration UI owns COM9; set JAW_SERIAL to enable later.
"""
import asyncio, io, json, re, sys, threading, time, urllib.request, wave
from pathlib import Path

import numpy as np

import memory
import skills
import languages

HOST, PORT = "127.0.0.1", 8766
OLLAMA = "http://127.0.0.1:11434"
LLM_MODEL = "qwen3:8b"
WHISPER_MODEL = "small"          # multilingual; handles Tamil + English
PIPER_DIR = Path(__file__).parent / "models" / "piper"
# (MMS language models are chosen dynamically in languages.py)
WEB_SEARCH = True                # enrich answers with DuckDuckGo when online

# Cloud brain (optional) — voice/api_keys.json holds the Gemini key and is
# gitignored so it can never leak into the repo. When the key is present and
# Google is reachable, Gemini answers; any failure falls straight back to
# local Ollama, so IRIS keeps talking with the internet down.
# (Hybrid pattern from the deestudio028/jarvis reference project.)
_keys = {}
try:
    _keys = json.loads((Path(__file__).parent / "api_keys.json").read_text("utf-8"))
except Exception:
    pass
GEMINI_KEY = (_keys.get("gemini_api_key") or "").strip()
GEMINI_MODEL = _keys.get("gemini_model", "gemini-flash-latest")
LLM_PROVIDER = _keys.get("llm_provider", "auto" if GEMINI_KEY else "ollama")
GEMINI_URL = "https://generativelanguage.googleapis.com/v1beta"

# English voices the user can pick in the UI's Voice tab. Tamil always uses
# MMS (the only decent free offline Tamil voice). First existing file wins
# as the default.
SPEECH_SPEED = 0.92              # <1 = calmer, more relaxed delivery

# engine "kokoro" = most natural (neural, 24 kHz); "piper" = fast fallback.
ENGLISH_VOICES = [
    {"id": "k-george", "label": "George — British male (JARVIS-style, default)", "engine": "kokoro", "voice": "bm_george"},
    {"id": "k-fenrir", "label": "Fenrir — deep, natural male", "engine": "kokoro", "voice": "am_fenrir"},
    {"id": "k-michael", "label": "Michael — calm, natural male", "engine": "kokoro", "voice": "am_michael"},
    {"id": "k-heart", "label": "Heart — calm, natural female", "engine": "kokoro", "voice": "af_heart"},
    {"id": "k-bella", "label": "Bella — warm, natural female", "engine": "kokoro", "voice": "af_bella"},
    {"id": "k-nicole", "label": "Nicole — soft, gentle female", "engine": "kokoro", "voice": "af_nicole"},
    {"id": "lessac-high", "label": "Lessac — natural, US female", "file": "en_US-lessac-high.onnx"},
    {"id": "ryan-high", "label": "Ryan — natural, US male", "file": "en_US-ryan-high.onnx"},
    {"id": "hfc-female", "label": "HFC — warm, US female", "file": "en_US-hfc_female-medium.onnx"},
    {"id": "amy", "label": "Amy — clear, US female", "file": "en_US-amy-medium.onnx"},
    {"id": "alba", "label": "Alba — British female", "file": "en_GB-alba-medium.onnx"},
    {"id": "lessac", "label": "Lessac classic (medium)", "file": "en_US-lessac-medium.onnx"},
]
JAW_SERIAL = None                # e.g. "COM9" - keep None while the browser UI owns the port
JAW_CLOSED_US, JAW_MAX_US = 1300, 1600

TAMIL_RE = re.compile(r"[஀-௿]")
SENTENCE_SPLIT = re.compile(r"(?<=[.!?…।])\s+")
SYSTEM_PROMPT = (
    "You are IRIS, a calm and gentle AI assistant created by Udhaya Kumar. "
    "You live in the InMoov robot head and control its face, and you can also "
    "help with the laptop. You speak in a relaxed, unhurried, warm way, like a "
    "thoughtful friend. Keep replies short: one to three spoken sentences. "
    "Never rush, never gush. Stay composed and kind even if the user is excited "
    "or upset. No emoji, no markdown, no lists, no stage directions, no "
    "exclamation marks. Always answer in the language the user used. You have "
    "moving eyes, eyelids, eyebrows, jaw and cheeks, and a camera to see faces."
)

def log(*a):
    """Never let a console encoding problem kill a conversation.

    The Windows console is cp1252; printing Tamil text raised
    UnicodeEncodeError and took the whole WebSocket connection down.
    """
    line = time.strftime("[%H:%M:%S] ") + " ".join(str(x) for x in a)
    try:
        print(line, flush=True)
    except UnicodeEncodeError:
        enc = getattr(sys.stdout, "encoding", None) or "ascii"
        print(line.encode(enc, "replace").decode(enc, "replace"), flush=True)

# ---------------------------------------------------------------- STT
log("loading faster-whisper", WHISPER_MODEL, "...")
from faster_whisper import WhisperModel
try:
    # GPU is ~11x faster than CPU here (0.2 s vs 2.5 s per utterance) and is
    # the single biggest latency win in the pipeline. Falls back automatically.
    stt_model = WhisperModel(WHISPER_MODEL, device="cuda", compute_type="int8_float16")
    import numpy as _np
    stt_model.transcribe(_np.zeros(16000, dtype=_np.float32))  # force CUDA init
    log("whisper ready on GPU")
except Exception as e:
    log("GPU whisper unavailable (%s); using CPU" % str(e)[:60])
    stt_model = WhisperModel(WHISPER_MODEL, device="cpu", compute_type="int8",
                             cpu_threads=8)
    log("whisper ready on CPU")

WAKE_RE = re.compile(r"\b(i\.?\s?r\.?\s?i\.?\s?s|iris|irish|ஐரிஸ்|ఐరిస్|आइरिस)\b[,.!\s]*", re.IGNORECASE)
FOLLOWUP_S = 45   # after an exchange, hands-free keeps listening this long unaddressed
NAME_FIX = re.compile(r"\bin[\s-]?moo?ve?\b", re.IGNORECASE)

def transcribe(pcm16: bytes):
    audio = np.frombuffer(pcm16, dtype=np.int16).astype(np.float32) / 32768.0
    if len(audio) < 1600:  # <0.1 s
        return "", "en"
    segments, info = stt_model.transcribe(
        audio, language=None,
        beam_size=5,                    # GPU is fast; buy accuracy back
        best_of=5, temperature=[0.0, 0.2, 0.4],
        condition_on_previous_text=False,   # stops one bad guess poisoning the next
        no_speech_threshold=0.5,
        vad_filter=True,
        vad_parameters=dict(min_silence_duration_ms=300,
                            speech_pad_ms=400),   # keep word edges
        without_timestamps=True,
        hotwords="Iris InMoov")
    text = " ".join(s.text.strip() for s in segments).strip()
    text = NAME_FIX.sub("InMoov", text)
    return text, (info.language or "en")

# ---------------------------------------------------------------- TTS english (Piper)
from piper import PiperVoice

try:
    from kokoro import KPipeline
    _kokoro = KPipeline(lang_code="a")
    log("kokoro ready (natural voices)")
except Exception as e:
    _kokoro = None
    log("kokoro unavailable (%s); piper only" % str(e)[:60])

available_voices = [
    v for v in ENGLISH_VOICES
    if (v.get("engine") == "kokoro" and _kokoro) or
       (v.get("engine") != "kokoro" and (PIPER_DIR / v["file"]).exists())
]
if not available_voices:
    raise SystemExit(f"no piper voice files in {PIPER_DIR}")
active_voice = {"id": available_voices[0]["id"]}
_piper_cache = {}

def get_piper(voice_id):
    if voice_id not in _piper_cache:
        entry = next(v for v in available_voices if v["id"] == voice_id)
        p = str(PIPER_DIR / entry["file"])
        log("loading piper voice", entry["file"], "...")
        try:
            _piper_cache[voice_id] = PiperVoice.load(p)
        except TypeError:
            _piper_cache[voice_id] = PiperVoice.load(p, p + ".json")
    return _piper_cache[voice_id]

if not next(v for v in available_voices if v["id"] == active_voice["id"]).get("engine") == "kokoro":
    get_piper(active_voice["id"])
log("default voice:", active_voice["id"])

def tts_kokoro(text: str, voice_name: str) -> bytes:
    audio = np.concatenate([g.audio.numpy() for g in
                            _kokoro(text, voice=voice_name, speed=SPEECH_SPEED)])
    pcm = np.clip(audio * 32767, -32768, 32767).astype(np.int16)
    buf = io.BytesIO()
    with wave.open(buf, "wb") as w:
        w.setnchannels(1); w.setsampwidth(2); w.setframerate(24000)
        w.writeframes(pcm.tobytes())
    return buf.getvalue()

def tts_english(text: str, voice_id: str = None) -> bytes:
    voice_id = voice_id or active_voice["id"]
    entry = next(v for v in available_voices if v["id"] == voice_id)
    if entry.get("engine") == "kokoro":
        return tts_kokoro(text, entry["voice"])
    voice = get_piper(voice_id)
    # length_scale > 1 slows piper down to match kokoro's calm pacing
    try:
        voice.config.length_scale = 1 / SPEECH_SPEED
    except Exception:
        pass
    buf = io.BytesIO()
    with wave.open(buf, "wb") as w:
        if hasattr(voice, "synthesize_wav"):
            voice.synthesize_wav(text, w)
        else:
            w.setnchannels(1); w.setsampwidth(2)
            w.setframerate(voice.config.sample_rate)
            for chunk in voice.synthesize_stream_raw(text):
                w.writeframes(chunk)
    return buf.getvalue()

# ---------------------------------------------------------------- TTS tamil (MMS, lazy)
_mms_cache = {}     # mms_code -> (model, tokenizer, torch)
_mms_lock = threading.Lock()

def _get_mms(mms_code):
    with _mms_lock:
        if mms_code not in _mms_cache:
            log(f"loading facebook/mms-tts-{mms_code} (first use, ~15 s)...")
            import torch
            from transformers import VitsModel, AutoTokenizer
            model = VitsModel.from_pretrained(f"facebook/mms-tts-{mms_code}")
            tok = AutoTokenizer.from_pretrained(f"facebook/mms-tts-{mms_code}")
            _mms_cache[mms_code] = (model, tok, torch)
            log(f"mms-tts-{mms_code} ready")
    return _mms_cache[mms_code]

def tts_mms(text: str, mms_code: str) -> bytes:
    model, tok, torch = _get_mms(mms_code)
    with torch.no_grad():
        wav = model(**tok(text, return_tensors="pt")).waveform[0].numpy()
    pcm = np.clip(wav * 32767, -32768, 32767).astype(np.int16)
    buf = io.BytesIO()
    with wave.open(buf, "wb") as w:
        w.setnchannels(1); w.setsampwidth(2)
        w.setframerate(model.config.sampling_rate)
        w.writeframes(pcm.tobytes())
    return buf.getvalue()

def tts(text: str, voice_id: str = None) -> bytes:
    # Route the VOICE by the reply's script, so it speaks correctly whatever
    # language the model produced (Telugu letters -> Telugu voice, and so on).
    mc = languages.mms_code(languages.detect_script(text))
    return tts_mms(text, mc) if mc else tts_english(text, voice_id)

# Warm the two most-used Indic voices so the first reply doesn't stall.
for _wc, _hi in (("tam", "வணக்கம்"), ("tel", "నమస్కారం")):
    threading.Thread(target=lambda c=_wc, h=_hi: tts_mms(h, c), daemon=True).start()

# ---------------------------------------------------------------- web search
SMALLTALK = re.compile(
    r"^\s*(hi|hello|hey|bye|goodbye|thanks|thank you|ok(ay)?|yes|no|"
    r"வணக்கம்|நன்றி|சரி|ஆம்|இல்லை)\b[\s!,.?]*$", re.IGNORECASE)

# Questions about the moving world always deserve a live search — even short
# ones ("latest news") and ones phrased at the robot ("do you know today's...").
FRESH_RE = re.compile(
    r"\b(news|latest|today|tonight|current(ly)?|right now|weather|temperature|"
    r"price|stock|score|match|election|release[ds]?|202\d|yesterday|"
    r"this (week|month|year))\b", re.IGNORECASE)

def web_search(query: str) -> str:
    """Top DuckDuckGo snippets, or '' if offline/failed/not worth searching."""
    if not WEB_SEARCH or SMALLTALK.match(query) or (
            len(query.split()) < 3 and not FRESH_RE.search(query)):
        return ""
    try:
        from ddgs import DDGS
        hits = list(DDGS().text(query, max_results=3))
        out = "\n".join(f"- {h['title']}: {h['body']}" for h in hits)
        return out[:1500]
    except Exception as e:
        log("web search skipped:", str(e)[:80])
        return ""

# ---------------------------------------------------------------- LLM (Ollama, streaming)
def extract_facts(user_text, reply, lang):
    """Ask the model what is worth remembering. Runs AFTER the reply is sent,
    so it never adds latency to the conversation."""
    prompt = (
        "From this exchange, list any durable facts about the USER worth "
        "remembering later (their name, job, likes, plans, relationships, "
        "or anything they asked you to remember). "
        "One short fact per line, in English, no bullets, no commentary. "
        "If there is nothing durable, reply exactly: NONE\n\n"
        f"User: {user_text}\nInMoov: {reply}")
    payload = {"model": LLM_MODEL, "stream": False, "keep_alive": "30m",
               "messages": [{"role": "user", "content": prompt}],
               "options": {"num_predict": 120, "temperature": 0.1}}
    if LLM_MODEL.startswith("qwen3"):
        payload["think"] = False
    try:
        req = urllib.request.Request(OLLAMA + "/api/chat", data=json.dumps(payload).encode(),
                                     headers={"Content-Type": "application/json"})
        with urllib.request.urlopen(req, timeout=60) as resp:
            out = json.load(resp)["message"]["content"].strip()
    except Exception as e:
        log("fact extraction failed:", str(e)[:60]); return
    if out.upper().startswith("NONE"):
        return
    for line in out.splitlines():
        line = line.strip(" -*.	")
        if 4 < len(line) < 200 and not line.upper().startswith("NONE"):
            if memory.add_fact(line, source="learned"):
                log("learned:", line[:70])


def llm_stream(history, lang, on_delta, search_context="", memories=""):
    # Small models ignore language rules in the system prompt; forcing the
    # language inline on the last user message is what actually works.
    system = SYSTEM_PROMPT
    if memories:
        system += (
            "\n\nMEMORY — facts you have learned about THE PERSON YOU ARE TALKING TO. "
            "These describe the user, not you. When they ask about themselves "
            "(their name, what they like, what they are doing), answer from these "
            "facts, and do not confuse them with your own identity. Use them "
            "naturally in conversation; never recite the list mechanically.\n"
            + memories)
    messages = [{"role": "system", "content": system}] + [dict(m) for m in history]
    if search_context:
        messages[-1]["content"] = (
            "Web search results (use them if relevant, ignore if not):\n"
            + search_context + "\n\nUser said: " + messages[-1]["content"])
    messages[-1]["content"] += languages.instruction(lang)
    # Cloud first, local fallback. If Gemini dies AFTER streaming words out,
    # we must not restart on Ollama — the room already heard the beginning —
    # so the fallback only fires when nothing was emitted yet.
    emitted = [False]
    def guarded(piece):
        emitted[0] = True
        on_delta(piece)
    if GEMINI_KEY and LLM_PROVIDER in ("auto", "gemini"):
        try:
            out = _gemini_stream(messages, guarded)
            if out:
                return out
        except Exception as e:
            log("gemini failed, using ollama:", str(e)[:80])
        if emitted[0]:
            return ""
    return _ollama_stream(messages, on_delta)


def _gemini_stream(messages, on_delta):
    system, contents = "", []
    for m in messages:
        if m["role"] == "system":
            system = m["content"]
        else:
            contents.append({"role": "user" if m["role"] == "user" else "model",
                             "parts": [{"text": m["content"]}]})
    payload = {"contents": contents,
               "generationConfig": {"maxOutputTokens": 400, "temperature": 0.6}}
    if system:
        payload["system_instruction"] = {"parts": [{"text": system}]}
    req = urllib.request.Request(
        f"{GEMINI_URL}/models/{GEMINI_MODEL}:streamGenerateContent?alt=sse",
        data=json.dumps(payload).encode(),
        headers={"Content-Type": "application/json", "x-goog-api-key": GEMINI_KEY})
    full = []
    with urllib.request.urlopen(req, timeout=60) as resp:
        for raw in resp:
            raw = raw.decode("utf-8", "ignore").strip()
            if not raw.startswith("data:"):
                continue
            data = raw[5:].strip()
            if data == "[DONE]":
                break
            try:
                piece = "".join(p.get("text", "") for p in
                                json.loads(data)["candidates"][0]["content"]["parts"])
            except Exception:
                continue
            if piece:
                full.append(piece)
                on_delta(piece)
    return "".join(full).strip()


def _ollama_stream(messages, on_delta):
    payload = {
        "model": LLM_MODEL,
        "messages": messages,
        "stream": True,
        "keep_alive": "30m",
        "options": {"num_predict": 220, "temperature": 0.6},
    }
    if LLM_MODEL.startswith("qwen3"):
        # qwen3 reasons before answering, which costs 20-40 s per reply.
        # Conversation does not need it.
        payload["think"] = False
    body = json.dumps(payload).encode()
    req = urllib.request.Request(OLLAMA + "/api/chat", data=body,
                                 headers={"Content-Type": "application/json"})
    full = []
    with urllib.request.urlopen(req, timeout=120) as resp:
        for line in resp:
            if not line.strip():
                continue
            piece = json.loads(line).get("message", {}).get("content", "")
            if piece:
                full.append(piece)
                on_delta(piece)
    return "".join(full).strip()

# ---------------------------------------------------------------- optional jaw
_jaw = None
if JAW_SERIAL:
    import serial
    _jaw = serial.Serial(JAW_SERIAL, 115200, timeout=1)
    time.sleep(2)  # Uno resets on open; firmware sends READY, no movement
    log("jaw serial open on", JAW_SERIAL)

def jaw_play(wav_bytes: bytes):
    """Stream C11 pulses matching the WAV envelope. Called only if enabled."""
    if not skills.robot.available:
        return
    with wave.open(io.BytesIO(wav_bytes)) as w:
        rate, n = w.getframerate(), w.getnframes()
        pcm = np.frombuffer(w.readframes(n), dtype=np.int16).astype(np.float32)
    frame = int(rate * 0.05)
    env = np.array([np.sqrt(np.mean(pcm[i:i+frame]**2)) for i in range(0, len(pcm), frame)])
    peak = np.percentile(env, 95) or 1.0
    env = np.clip(env / peak, 0, 1)
    level = 0.0
    for e in env:
        level = max(e, level * 0.75)          # fast attack, slow decay
        pulse = int(JAW_CLOSED_US + (JAW_MAX_US - JAW_CLOSED_US) * level)
        _jaw.write(f"C11 {pulse}\n".encode())
        time.sleep(0.05)
    _jaw.write(f"C11 {JAW_CLOSED_US}\n".encode())

# ---------------------------------------------------------------- websocket session
import websockets

def sys_stats():
    import psutil, shutil
    b = psutil.sensors_battery()
    du_c = shutil.disk_usage("C:\\")
    du_u = shutil.disk_usage("U:\\")
    return {"type": "sys",
            "cpu": round(psutil.cpu_percent()), "ram": round(psutil.virtual_memory().percent),
            "batt": round(b.percent) if b else None,
            "plugged": bool(b.power_plugged) if b else None,
            "disk_c": round(du_c.used / du_c.total * 100),
            "disk_u": round(du_u.used / du_u.total * 100),
            "gpu": None}


async def handle(ws):
    log("client connected")
    loop = asyncio.get_running_loop()
    history, chunks, capturing = [], [], False
    my_voice = {"id": active_voice["id"]}
    last_exchange = [0.0]          # hands-free follow-up window

    async def stats_loop():
        while True:
            try:
                await ws.send(json.dumps(await loop.run_in_executor(None, sys_stats)))
            except websockets.ConnectionClosed:
                return
            except Exception:
                pass
            await asyncio.sleep(3)
    stats_task = asyncio.create_task(stats_loop())

    async def send(obj):
        await ws.send(json.dumps(obj))

    async def respond(user_text, lang_hint):
        history.append({"role": "user", "content": user_text})
        await send({"type": "user", "text": user_text, "lang": lang_hint})
        await send({"type": "status", "state": "thinking"})
        memory.learn_phrase(user_text, lang_hint)
        # "remember that ..." / "forget ..." / "what do you know" are answered
        # directly so teaching is instant and reliable.
        direct, _skill = skills.match(user_text)
        if direct:
            log("skill:", _skill, "->", direct[:50])
        if not direct:
            direct = memory.handle_command(user_text)
        if direct:
            history.append({"role": "assistant", "content": direct})
            await send({"type": "reply", "text": direct})
            await send({"type": "status", "state": "speaking"})
            wav = await loop.run_in_executor(None, tts, direct, my_voice["id"])
            await send({"type": "audio"})
            await ws.send(wav)
            await send({"type": "speech_done"})
            await send({"type": "memory", "stats": memory.stats()})
            last_exchange[0] = time.time()   # skills open the follow-up window too
            return
        memories = "\n".join("- " + m for m in memory.recall(user_text))
        search_context = ""
        # Search only English world-questions: Tamil search results are poor and
        # triple the reply time, and questions about the robot itself
        # ("who are you", "what can you do") gain nothing from the web.
        robot_directed = re.search(r"(you|your|yours)", user_text, re.IGNORECASE)
        fresh = bool(FRESH_RE.search(user_text))
        if (WEB_SEARCH and lang_hint == "en" and
                (fresh or (not robot_directed and not SMALLTALK.match(user_text)
                           and len(user_text.split()) >= 3))):
            await send({"type": "status", "state": "thinking", "detail": "searching the web"})
            # Instant acknowledgment (Mark-LV pattern): a short spoken filler
            # covers the search delay so the room never gets dead air.
            ack = await loop.run_in_executor(None, tts, "One moment.", my_voice["id"])
            await send({"type": "audio"})
            await ws.send(ack)
            try:
                search_context = await asyncio.wait_for(
                    loop.run_in_executor(None, web_search, user_text), timeout=8)
            except asyncio.TimeoutError:
                search_context = ""
            await send({"type": "status", "state": "thinking"})
        deltas = asyncio.Queue()
        def on_delta(piece):
            loop.call_soon_threadsafe(deltas.put_nowait, piece)
        task = loop.run_in_executor(None, llm_stream, list(history), lang_hint,
                                    on_delta, search_context, memories)
        # Speak sentence-by-sentence while the LLM is still generating, so the
        # first words play ~2 s in instead of after the whole reply is done.
        sentence_q = asyncio.Queue()
        async def speaker():
            while True:
                sent = await sentence_q.get()
                if sent is None:
                    break
                wav = await loop.run_in_executor(None, tts, sent, my_voice["id"])
                await send({"type": "audio"})
                await ws.send(wav)
                # Jaw moves with THIS sentence while the next one is still
                # being written, so speech and motion stay in step.
                if skills.robot.available:
                    loop.run_in_executor(None, jaw_play, wav)
        speak_task = asyncio.create_task(speaker())
        pending = ""
        first_sent = False
        async def feed(piece):
            nonlocal pending, first_sent
            pending += piece
            parts = SENTENCE_SPLIT.split(pending)
            for complete in parts[:-1]:
                if complete.strip():
                    if not first_sent:
                        first_sent = True
                        await send({"type": "status", "state": "speaking"})
                    await sentence_q.put(complete.strip())
            pending = parts[-1]
        while True:
            get = asyncio.create_task(deltas.get())
            done, _ = await asyncio.wait({get, task}, return_when=asyncio.FIRST_COMPLETED)
            if get in done:
                piece = get.result()
                await send({"type": "delta", "text": piece})
                await feed(piece)
            else:
                get.cancel()
                break
        while not deltas.empty():
            piece = deltas.get_nowait()
            await send({"type": "delta", "text": piece})
            await feed(piece)
        reply = task.result()
        if not reply:
            reply = "மன்னிக்கவும், மீண்டும் சொல்லுங்கள்." if lang_hint == "ta" else "Sorry, say that again?"
            await sentence_q.put(reply)
        if pending.strip():
            await sentence_q.put(pending.strip())
        history.append({"role": "assistant", "content": reply})
        if len(history) > 20:
            del history[:2]
        await send({"type": "reply", "text": reply})
        await sentence_q.put(None)
        await speak_task
        await send({"type": "speech_done"})
        last_exchange[0] = time.time()
        memory.log_exchange(user_text, reply, lang_hint)
        loop.run_in_executor(None, extract_facts, user_text, reply, lang_hint)
        await send({"type": "memory", "stats": memory.stats()})

    try:
        await send({"type": "status", "state": "idle", "detail": "ready"})
        await send({"type": "memory", "stats": memory.stats()})
        await send({"type": "skills", **skills.describe()})
        await send({"type": "voices",
                    "list": [{"id": v["id"], "label": v["label"]} for v in available_voices],
                    "active": my_voice["id"]})
        async for message in ws:
            if isinstance(message, bytes):
                if capturing:
                    chunks.append(message)
                continue
            msg = json.loads(message)
            if msg["type"] == "start":
                chunks, capturing = [], True
                await send({"type": "status", "state": "listening"})
            elif msg["type"] == "stop":
                capturing = False
                hands_free = bool(msg.get("handsfree"))
                await send({"type": "status", "state": "thinking"})
                pcm = b"".join(chunks)
                text, lang = await loop.run_in_executor(None, transcribe, pcm)
                if not text:
                    await send({"type": "status", "state": "idle", "detail": "heard nothing"})
                    continue
                # Hands-free wake gate (Mark-LV pattern): answer only when
                # addressed by name, or within the follow-up window after an
                # exchange — so room chatter never triggers a reply.
                if hands_free:
                    addressed = WAKE_RE.search(text)
                    recent = time.time() - last_exchange[0] < FOLLOWUP_S
                    if not addressed and not recent:
                        log("standby, ignored:", text[:50])
                        await send({"type": "status", "state": "idle",
                                    "detail": "standby — say 'Iris'"})
                        continue
                    if addressed:
                        stripped = WAKE_RE.sub("", text, count=1).strip()
                        if stripped:
                            text = stripped
                try:
                    await respond(text, lang)
                except websockets.ConnectionClosed:
                    raise
                except Exception as e:
                    log("turn failed:", repr(e)[:120])
                    await send({"type": "error", "text": "Something went wrong on that one."})
                    await send({"type": "status", "state": "idle"})
            elif msg["type"] == "text":
                user_text = msg.get("text", "").strip()
                if user_text:
                    try:
                        await respond(user_text, languages.detect_script(user_text))
                    except websockets.ConnectionClosed:
                        raise
                    except Exception as e:
                        log("turn failed:", repr(e)[:120])
                        await send({"type": "error", "text": "Something went wrong on that one."})
                        await send({"type": "status", "state": "idle"})
            elif msg["type"] == "motion":
                want = bool(msg.get("on"))
                if want:
                    skills.robot.port = msg.get("port") or "COM9"
                    ok = skills.robot.connect()
                    if not ok:
                        skills.robot.port = None
                    await send({"type": "motion", "on": ok,
                                "detail": skills.robot.last_error if not ok else "connected"})
                else:
                    skills.robot.off()
                    try: skills.robot.ser and skills.robot.ser.close()
                    except Exception: pass
                    skills.robot.ser = None; skills.robot.port = None
                    await send({"type": "motion", "on": False, "detail": "outputs off"})
            elif msg["type"] == "skills":
                await send({"type": "skills", **skills.describe()})
            elif msg["type"] == "memory_list":
                await send({"type": "memory_list",
                            "facts": memory.all_facts()[-60:],
                            "phrases": memory.common_phrases(),
                            "stats": memory.stats()})
            elif msg["type"] == "memory_forget":
                memory.forget(msg.get("text") or None)
                await send({"type": "memory_list",
                            "facts": memory.all_facts()[-60:],
                            "phrases": memory.common_phrases(),
                            "stats": memory.stats()})
            elif msg["type"] == "set_voice":
                vid = msg.get("id")
                if any(v["id"] == vid for v in available_voices):
                    my_voice["id"] = vid
                    if not next(v for v in available_voices if v["id"] == vid).get("engine") == "kokoro":
                        await loop.run_in_executor(None, get_piper, vid)
                    await send({"type": "voice_set", "id": vid})
            elif msg["type"] == "say":
                sample = msg.get("text", "").strip()
                if sample:
                    wav = await loop.run_in_executor(None, tts, sample, my_voice["id"])
                    await send({"type": "audio"})
                    await ws.send(wav)
                    await send({"type": "speech_done"})
    except websockets.ConnectionClosed:
        pass
    finally:
        stats_task.cancel()
        log("client disconnected")

async def main():
    # quick ollama check so failures are loud at startup
    try:
        with urllib.request.urlopen(OLLAMA + "/api/tags", timeout=5) as r:
            models = [m["name"] for m in json.load(r).get("models", [])]
        log("ollama ok, models:", models or "NONE - run: ollama pull " + LLM_MODEL)
    except Exception as e:
        log("WARNING: ollama not reachable:", e)
        log("start it, then: ollama pull", LLM_MODEL)
    if GEMINI_KEY:
        log(f"brain: {GEMINI_MODEL} (cloud) with {LLM_MODEL} fallback (local)")
    else:
        log(f"brain: {LLM_MODEL} (local only — add voice/api_keys.json for Gemini)")
    log(f"listening on ws://{HOST}:{PORT} - open http://127.0.0.1:8765/orb.html")
    async with websockets.serve(handle, HOST, PORT, max_size=32 * 1024 * 1024):
        await asyncio.Future()

if __name__ == "__main__":
    try:
        asyncio.run(main())
    finally:
        if _jaw:
            _jaw.write(b"0\n"); _jaw.close()
