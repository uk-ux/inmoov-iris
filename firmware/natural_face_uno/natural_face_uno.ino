#include <Wire.h>

// Uno A4 -> SDA, A5 -> SCL; separate servo supply and shared ground.
// Commands: 1 animate, 2 neutral, 0 outputs off. Serial: 115200 baud.
// Calibration positions are NOT physical degrees.
const uint8_t ADDRESS = 0x40;
const uint8_t CHANNELS[4] = {8, 9, 12, 13};
// User reports changed mechanical limits: revalidate before running animation.
// Current user sheet: neutral positions are 100, 80, 50, 150.
const int NEUTRAL[4] = {1522, 1477, 1411, 1633};
// User supplied endpoints are 0 and 180 on the calibration scale.
const int MIN_US[4] = {1300, 1300, 1300, 1300};
const int MAX_US[4] = {1700, 1700, 1700, 1700};
int currentUs[4] = {1522, 1477, 1411, 1633};
int fromUs[4], targetUs[4];
uint16_t lastTicks[4] = {0, 0, 0, 0};
bool ready = false, enabled = false, automatic = false, moving = false;
bool nextIsNeutral = false;
unsigned long began = 0, duration = 1000, holdMs = 0, nextAt = 0, frameAt = 0;
const uint8_t delaysMs[4] = {0, 10, 15, 20};
char input[8];
uint8_t length = 0;
bool discard = false;
unsigned long receivedAt = 0;

bool writeRegister(uint8_t reg, uint8_t value) {
  Wire.beginTransmission(ADDRESS);
  Wire.write(reg); Wire.write(value);
  return Wire.endTransmission() == 0;
}

bool writeChannel(uint8_t ch, uint16_t ticks) {
  Wire.beginTransmission(ADDRESS);
  Wire.write(0x06 + 4 * ch);
  Wire.write((uint8_t)0); Wire.write((uint8_t)0);
  Wire.write((uint8_t)(ticks & 255)); Wire.write((uint8_t)(ticks >> 8));
  return Wire.endTransmission() == 0;
}

bool allOff() {
  automatic = false; moving = false; enabled = false;
  bool ok = true;
  for (uint8_t ch = 0; ch < 16; ++ch) {
    if (!writeChannel(ch, 4096)) ok = false;
  }
  for (uint8_t i = 0; i < 4; ++i) lastTicks[i] = 0;
  return ok;
}

void fault() {
  ready = false;
  allOff();
  Serial.println(F("FAULT: I2C error. Cut servo power and check wiring; reset to retry."));
}

bool sendPose() {
  for (uint8_t i = 0; i < 4; ++i) {
    currentUs[i] = constrain(currentUs[i], MIN_US[i], MAX_US[i]);
    // Same nominal 50 Hz conversion as the calibration sketch.
    uint16_t ticks = ((unsigned long)currentUs[i] * 4096 + 10000) / 20000;
    if (ticks != lastTicks[i]) {
      if (!writeChannel(CHANNELS[i], ticks)) { fault(); return false; }
      lastTicks[i] = ticks;
    }
  }
  return true;
}

void transition(const int *pose, unsigned long travel, unsigned long hold) {
  for (uint8_t i = 0; i < 4; ++i) {
    fromUs[i] = currentUs[i];
    targetUs[i] = constrain(pose[i], MIN_US[i], MAX_US[i]);
  }
  began = millis(); duration = travel; holdMs = hold; moving = true;
}

void expression() {
  int pose[4];
  // These are full user-reported limits, expressed as calibration positions.
  // The alternating poses move all four mechanisms together despite their
  // opposite physical directions.
  const int poses[5][4] = {
    {180, 0, 180, 0},    // broad raised/open expression
    {0, 180, 0, 180},    // opposite sweep
    {160, 20, 170, 20},  // strong left-led expression
    {20, 160, 20, 160},  // strong right-led expression
    {180, 0, 20, 160}    // asymmetric surprise
  };
  uint8_t choice = random(0, 5);
  for (uint8_t i = 0; i < 4; ++i) {
    pose[i] = 1300 + (long)poses[choice][i] * 400 / 180;
  }
  transition(pose, random(450, 701), random(500, 1201));
  nextIsNeutral = true;
}

void command() {
  // Exact single-character commands only: echoed status and old calibration
  // commands cannot accidentally start motion.
  if (discard || length != 1) return;
  if (input[0] == '0') {
    if (!allOff()) fault();
    else Serial.println(F("OFF"));
    return;
  }
  if (!ready || (input[0] != '1' && input[0] != '2')) return;
  if (!enabled) {
    // Position cannot be sensed after power-off: first engagement may move to
    // neutral immediately. All later movements are smoothly interpolated.
    for (uint8_t i = 0; i < 4; ++i) currentUs[i] = NEUTRAL[i];
    if (!sendPose()) return;
    enabled = true;
  }
  automatic = input[0] == '1';
  nextIsNeutral = false;
  transition(NEUTRAL, 300, 600);
  Serial.println(automatic ? F("RUNNING") : F("NEUTRAL"));
}

void readCommands() {
  while (Serial.available()) {
    char c = Serial.read(); receivedAt = millis();
    if (c == '\n' || c == '\r') {
      command(); length = 0; discard = false;
    } else if (!discard) {
      if (length < sizeof(input)) input[length++] = c;
      else discard = true;
    }
  }
  if ((length || discard) && millis() - receivedAt >= 75) {
    command(); length = 0; discard = false;
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin();
  Wire.setWireTimeout(25000, true);
  // Disable old PWM before changing frequency; outputs stay off on boot.
  if (!allOff() || !writeRegister(0x00, 0x10) ||
      !writeRegister(0xFE, 121) || !writeRegister(0x00, 0x20)) {
    fault(); return;
  }
  delay(5);
  if (!writeRegister(0x00, 0xA0) || !allOff()) { fault(); return; }
  randomSeed(analogRead(A0));
  ready = true;
  Serial.println(F("READY natural face FULL LIMIT SWEEP - outputs OFF"));
}

void loop() {
  readCommands();
  if (!ready || !enabled) return;
  unsigned long now = millis();
  if (now - frameAt < 20) return;
  frameAt = now;
  if (moving) {
    unsigned long elapsed = now - began;
    for (uint8_t i = 0; i < 4; ++i) {
      float t = elapsed <= delaysMs[i] ? 0.0f :
          min(1.0f, (float)(elapsed - delaysMs[i]) / duration);
      // Quintic easing: zero speed and acceleration at both ends.
      float ease = t*t*t*(10.0f + t*(-15.0f + 6.0f*t));
      currentUs[i] = fromUs[i] + (int)((targetUs[i] - fromUs[i]) * ease);
    }
    if (!sendPose()) return;
    if (elapsed >= duration + delaysMs[3]) {
      moving = false; nextAt = now + holdMs;
    }
  } else if (automatic && (long)(now - nextAt) >= 0) {
    if (nextIsNeutral) {
      transition(NEUTRAL, random(220, 401), random(700, 1601));
      nextIsNeutral = false;
    } else expression();
  }
}
