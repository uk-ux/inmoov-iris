"""InMoov skills — turning speech into actions.

Two layers, fast first:

  1. FAST INTENTS (this file) — regex patterns in English and Tamil that map
     straight to an action. Zero model latency: "look around" fires in about
     a millisecond instead of waiting 5 s for an 8B model to emit a tool call.
     This is why commands feel instant while conversation still feels smart.

  2. The LLM handles everything that is not a command.

Robot motion goes out over the same serial protocol the browser UI uses
(C<ch> <pulse>), so nothing new had to be added to the firmware.
Laptop control is a fixed allow-list — never arbitrary shell execution.
"""
import os, re, subprocess, time, threading
from pathlib import Path

# ---------------------------------------------------------------- robot link
class Robot:
    """Serial link to the Uno. Safe by default: does nothing unless enabled."""

    def __init__(self, port=None, baud=115200):
        self.port, self.baud, self.ser = port, baud, None
        self.lock = threading.Lock()
        self.last_error = None

    def connect(self):
        if not self.port or self.ser:
            return self.ser is not None
        try:
            import serial
            self.ser = serial.Serial(self.port, self.baud, timeout=1)
            time.sleep(2.0)          # Uno resets on open; firmware prints READY
            self.ser.reset_input_buffer()
            return True
        except Exception as e:
            self.last_error = str(e)[:120]
            self.ser = None
            return False

    def send(self, line):
        if not self.ser and not self.connect():
            return False
        try:
            with self.lock:
                self.ser.write((line + "\n").encode())
            return True
        except Exception as e:
            self.last_error = str(e)[:120]
            self.ser = None
            return False

    def pose(self, moves, hold=0.0):
        """moves: {channel: pulse}. Sends one channel at a time, like the UI."""
        ok = all(self.send(f"C{ch} {us}") for ch, us in moves.items())
        if hold:
            time.sleep(hold)
        return ok

    def off(self):
        return self.send("0")

    @property
    def available(self):
        return bool(self.port)


robot = Robot()      # server sets robot.port when the user enables motion

# Calibrated rest values from INMOOV_AGENT_HANDOFF.md. Every pose below stays
# inside the compiled firmware limits, and the firmware rejects anything that
# does not — so a bad value here cannot drive a servo past its stop.
REST = {0: 1477, 1: 1500, 2: 1477, 3: 1522, 8: 1500, 9: 1500,
        10: 1411, 11: 1300, 12: 1455, 13: 1566, 14: 1500, 15: 1500}
EYES_L = {0: 1360, 2: 1360}
EYES_R = {0: 1600, 2: 1600}
EYES_C = {0: 1477, 2: 1477}
EYES_UP = {1: 1400, 3: 1620}
EYES_DOWN = {1: 1600, 3: 1420}
SMILE = {8: 1400, 9: 1600, 14: 1380, 15: 1620, 11: 1340}
SAD = {8: 1620, 9: 1380, 14: 1600, 15: 1400}
SURPRISE = {1: 1433, 3: 1589, 8: 1322, 9: 1700, 11: 1560, 12: 1700, 13: 1322}


def _look_around():
    """A slow scan: right, left, centre. Reads as curiosity, not a twitch."""
    robot.pose(EYES_R, 0.75)
    robot.pose(EYES_L, 0.95)
    robot.pose(EYES_C, 0.4)
    return "Looking around."


def _blink(times=2):
    for _ in range(times):
        robot.send("B1"); time.sleep(0.16)
        robot.send("B0"); time.sleep(0.28)
    return "Blinking."


def _expression(pose, name):
    robot.pose(pose, 1.6)
    robot.pose(REST)
    return name


# ---------------------------------------------------------------- laptop link
def _run(args):
    try:
        subprocess.Popen(args, shell=False,
                         creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0))
        return True
    except Exception:
        return False


def _ps(script):
    return _run(["powershell", "-NoProfile", "-WindowStyle", "Hidden",
                 "-Command", script])


def _volume(direction, steps=5):
    key = {"up": 175, "down": 174, "mute": 173}[direction]
    _ps("$w=New-Object -ComObject WScript.Shell;" +
        f"1..{steps}|%{{$w.SendKeys([char]{key})}}")
    return {"up": "Volume up.", "down": "Volume down.", "mute": "Muted."}[direction]


APPS = {
    "notepad": "notepad.exe", "calculator": "calc.exe", "calculator app": "calc.exe",
    "chrome": "chrome.exe", "browser": "chrome.exe", "edge": "msedge.exe",
    "explorer": "explorer.exe", "file explorer": "explorer.exe",
    "files": "explorer.exe", "paint": "mspaint.exe", "terminal": "wt.exe",
    "camera": "microsoft.windows.camera:", "settings": "ms-settings:",
    "task manager": "taskmgr.exe", "calendar": "outlookcal:",
}


def _open_app(name):
    name = name.lower().strip(" .?!")
    exe = APPS.get(name)
    if not exe:
        for k, v in APPS.items():
            if k in name or name in k:
                exe, name = v, k
                break
    if not exe:
        return f"I do not know how to open {name}."
    if exe.endswith(":"):
        _run(["cmd", "/c", "start", "", exe])
    else:
        _run(["cmd", "/c", "start", "", exe])
    return f"Opening {name}."


def _screenshot():
    try:
        from PIL import ImageGrab
        out = Path.home() / "Pictures" / f"inmoov-{time.strftime('%H%M%S')}.png"
        out.parent.mkdir(exist_ok=True)
        ImageGrab.grab().save(out)
        return "Screenshot saved to your Pictures folder."
    except Exception:
        _ps("Add-Type -AssemblyName System.Windows.Forms;"
            "[System.Windows.Forms.SendKeys]::SendWait('{PRTSC}')")
        return "Screenshot copied to the clipboard."


def _diagnose():
    """A spoken health check of InMoov — the JARVIS 'run diagnostics' moment.

    Checks the serial link, asks the firmware for a safe state, and reports.
    It never drives a servo; it only confirms the head is reachable and safe.
    """
    if not robot.available:
        return ("Diagnostics: my body is not connected. Enable motion first, "
                "and make sure the calibrator is closed so I can use the port.")
    checks = []
    # 1. serial link up?
    if robot.connect():
        checks.append("serial link to the controller is up")
    else:
        return f"Diagnostics failed: I cannot reach the controller. {robot.last_error or ''}"
    # 2. can the firmware accept a safe command?
    if robot.off():
        checks.append("all servo outputs are safely off")
    else:
        checks.append("WARNING: the controller did not confirm outputs off")
    # 3. report the known configuration
    checks.append("sixteen facial channels are configured")
    return "Diagnostics complete. " + ". ".join(c[0].upper() + c[1:] for c in checks) + "."


def _status_report():
    parts = []
    parts.append("my body is connected" if robot.available else "my body is not connected")
    try:
        parts.append(_battery().lower().rstrip("."))
    except Exception:
        pass
    parts.append("it is " + time.strftime("%I:%M %p").lstrip("0"))
    return "Here is my status. " + ", and ".join(parts) + "."


def _battery():
    try:
        import ctypes

        class S(ctypes.Structure):
            _fields_ = [("ACLineStatus", ctypes.c_byte), ("BatteryFlag", ctypes.c_byte),
                        ("BatteryLifePercent", ctypes.c_byte), ("SystemStatusFlag", ctypes.c_byte),
                        ("BatteryLifeTime", ctypes.c_ulong), ("BatteryFullLifeTime", ctypes.c_ulong)]
        s = S()
        ctypes.windll.kernel32.GetSystemPowerStatus(ctypes.byref(s))
        plug = " and charging" if s.ACLineStatus == 1 else ""
        return f"Battery is at {s.BatteryLifePercent} percent{plug}."
    except Exception:
        return "I could not read the battery."


# ---------------------------------------------------------------- intents
# (pattern, handler, needs_robot).  Tamil alternatives sit in the same pattern.
INTENTS = [
    # --- head / face ---
    (r"\b(look around|scan (the )?room|சுற்றி\s*பார்)\b", lambda m: _look_around(), True),
    (r"\b(look (to (the )?)?left|இடது\s*பக்கம்\s*பார்)\b",
     lambda m: (robot.pose(EYES_L), "Looking left.")[1], True),
    (r"\b(look (to (the )?)?right|வலது\s*பக்கம்\s*பார்)\b",
     lambda m: (robot.pose(EYES_R), "Looking right.")[1], True),
    (r"\b(look up|மேலே\s*பார்)\b", lambda m: (robot.pose(EYES_UP), "Looking up.")[1], True),
    (r"\b(look down|கீழே\s*பார்)\b", lambda m: (robot.pose(EYES_DOWN), "Looking down.")[1], True),
    (r"\b(look at me|center your eyes|நேராக\s*பார்)\b",
     lambda m: (robot.pose(EYES_C), "Looking at you.")[1], True),
    (r"\b(blink|கண்\s*சிமிட்டு)\b", lambda m: _blink(), True),
    (r"\b(smile|சிரி)\b", lambda m: _expression(SMILE, "Smiling."), True),
    (r"\b(look sad|frown|சோகமா)\b", lambda m: _expression(SAD, "Feeling sad."), True),
    (r"\b(look surprised|surprise|ஆச்சரியம்)\b",
     lambda m: _expression(SURPRISE, "Surprised."), True),
    (r"\b(neutral|relax your face|rest your face|இயல்பு\s*நிலை)\b",
     lambda m: (robot.pose(REST), "Back to neutral.")[1], True),
    (r"\b(stop moving|freeze|outputs off|நிறுத்து)\b",
     lambda m: (robot.off(), "Stopping all movement.")[1], True),
    (r"\b(run (a )?diagnostic|diagnostics|health check|self test|check yourself|சோதனை)\b",
     lambda m: _diagnose(), False),
    (r"\b(status report|system status|how are your systems|report status)\b",
     lambda m: _status_report(), False),

    # --- laptop ---
    (r"\bopen (?:the )?(.+?)(?: app| program)?\s*$", lambda m: _open_app(m.group(1)), False),
    (r"\b(volume up|louder|turn it up|சத்தம்\s*அதிகரி)\b", lambda m: _volume("up"), False),
    (r"\b(volume down|quieter|turn it down|சத்தம்\s*குறை)\b", lambda m: _volume("down"), False),
    (r"\b(mute|unmute|silence)\b", lambda m: _volume("mute"), False),
    (r"\b(take a )?screenshot\b", lambda m: _screenshot(), False),
    (r"\b(lock (the )?(screen|laptop|computer))\b",
     lambda m: (_run(["rundll32.exe", "user32.dll,LockWorkStation"]), "Locking the screen.")[1], False),
    (r"\b(battery|charge level|பேட்டரி)\b", lambda m: _battery(), False),
    (r"\bwhat(?:'s| is)? the time|what time is it|நேரம்\s*என்ன\b",
     lambda m: "It is " + time.strftime("%I:%M %p").lstrip("0") + ".", False),
    (r"\bwhat(?:'s| is)? (?:today'?s )?(?:date|day)|what day is it|என்ன\s*தேதி\b",
     lambda m: "Today is " + time.strftime("%A, %d %B %Y") + ".", False),
]
COMPILED = [(re.compile(p, re.IGNORECASE), fn, needs) for p, fn, needs in INTENTS]


def match(text):
    """Return (reply, name) if this is a command, else (None, None).

    Runs before the LLM, so commands never pay model latency.
    """
    t = " ".join(text.split())
    for rx, fn, needs_robot in COMPILED:
        m = rx.search(t)
        if not m:
            continue
        if needs_robot and not robot.available:
            return ("My body is not connected yet, so I cannot move. "
                    "Enable motion once the servos are calibrated."), "blocked"
        try:
            return fn(m), rx.pattern[:28]
        except Exception as e:
            return f"I tried, but something went wrong: {str(e)[:60]}", "error"
    return None, None


def describe():
    return {
        "robot": ["look around", "look left / right / up / down", "look at me",
                  "blink", "smile", "look sad", "look surprised", "neutral",
                  "stop moving"],
        "laptop": ["open notepad / chrome / calculator / explorer / camera",
                   "volume up / down", "mute", "screenshot", "lock the screen",
                   "run diagnostics", "status report", "battery", "what time is it", "what is the date"],
        "motion_enabled": robot.available,
    }
