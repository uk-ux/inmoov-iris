#include <Wire.h>

// Uno: A4=SDA, A5=SCL, 5V=VCC, shared GND. External supply powers V+.
// Serial: 115200 baud, Newline. Example: "1 90". "0" disables all outputs.
// Position numbers use the ORIGINAL 1300-1700 us calibration scale,
// NOT physical degrees. Sheet values are interpreted as entered commands.
const uint8_t ADDRESS = 0x40;
const uint8_t CHANNEL[4] = {8, 9, 12, 13};
const int MIN_POSITION[4] = {20, 0, 20, 20};
const int MAX_POSITION[4] = {140, 120, 120, 120};
const int NEUTRAL_US[4] = {1500, 1388, 1500, 1477};
int active = -1, currentUs = 1500, targetUs = 1500;
bool ready = false, moving = false;
unsigned long lastFrame = 0;
uint16_t lastTicks = 0;
char buffer[16];
uint8_t length = 0;
bool discard = false;

bool regWrite(uint8_t reg, uint8_t value) {
  Wire.beginTransmission(ADDRESS);
  Wire.write(reg); Wire.write(value);
  return Wire.endTransmission() == 0;
}

bool pwmWrite(uint8_t channel, uint16_t ticks) {
  Wire.beginTransmission(ADDRESS);
  Wire.write(0x06 + 4 * channel);
  Wire.write((uint8_t)0); Wire.write((uint8_t)0);
  Wire.write((uint8_t)(ticks & 255)); Wire.write((uint8_t)(ticks >> 8));
  return Wire.endTransmission() == 0;
}

bool stopAll() {
  active = -1; moving = false; lastTicks = 0;
  bool ok = true;
  for (uint8_t i = 0; i < 16; ++i) if (!pwmWrite(i, 4096)) ok = false;
  return ok;
}

void fault() {
  ready = false;
  stopAll();
  Serial.println(F("I2C FAULT: cut servo power and check wiring; reset to retry."));
}

bool sendPulse() {
  uint16_t ticks = ((unsigned long)currentUs * 4096 + 10000) / 20000;
  if (ticks != lastTicks) {
    if (!pwmWrite(CHANNEL[active], ticks)) { fault(); return false; }
    lastTicks = ticks;
  }
  return true;
}

void command() {
  if (discard || length == 0) return;
  if (length == 1 && buffer[0] == '0') {
    if (!stopAll()) fault();
    else Serial.println(F("OFF"));
    return;
  }
  // Validate the WHOLE line before any motion. Ignore echoed output.
  if (!ready || length < 3 || buffer[0] < '1' || buffer[0] > '4' ||
      buffer[1] != ' ') return;
  uint8_t joint = buffer[0] - '1';
  uint8_t p = 2;
  while (p < length && buffer[p] == ' ') ++p;
  bool digit = false;
  int position = 0;
  while (p < length && buffer[p] >= '0' && buffer[p] <= '9') {
    digit = true;
    position = position * 10 + buffer[p++] - '0';
    if (position > 180) { Serial.println(F("Rejected: position too large.")); return; }
  }
  while (p < length && buffer[p] == ' ') ++p;
  if (!digit || p != length) return;
  if (position < MIN_POSITION[joint] || position > MAX_POSITION[joint]) {
    Serial.println(F("Rejected: outside your recorded limits."));
    return;
  }
  if (active != joint) {
    if (!stopAll()) { fault(); return; }
    active = joint;
    currentUs = NEUTRAL_US[joint];
    // No position feedback: initial engagement can jump to stored neutral.
    if (!sendPulse()) return;
  }
  targetUs = 1300 + (long)position * 400 / 180;
  moving = true;
  Serial.print(F("Channel ")); Serial.print(CHANNEL[joint]);
  Serial.print(F(" target position ")); Serial.print(position);
  Serial.print(F(" pulse ")); Serial.print(targetUs); Serial.println(F(" us"));
}

void setup() {
  Serial.begin(115200);
  Wire.begin();
  Wire.setWireTimeout(25000, true);
  if (!stopAll() || !regWrite(0x00, 0x10) || !regWrite(0xFE, 121) ||
      !regWrite(0x00, 0x20)) { fault(); return; }
  delay(5);
  if (!regWrite(0x00, 0xA0) || !stopAll()) { fault(); return; }
  ready = true;
  Serial.println(F("READY limit test - outputs OFF. Use Newline."));
}

void loop() {
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\r' || c == '\n') {
      command(); length = 0; discard = false;
    } else if (!discard) {
      if (length < sizeof(buffer)) buffer[length++] = c;
      else discard = true;
    }
  }
  if (!ready || active < 0 || !moving) return;
  unsigned long now = millis();
  if (now - lastFrame < 20) return;
  lastFrame = now;
  // Slow approach: at most 2 us per frame (100 us/second).
  if (currentUs < targetUs) currentUs = min(currentUs + 2, targetUs);
  else if (currentUs > targetUs) currentUs = max(currentUs - 2, targetUs);
  if (!sendPulse()) return;
  if (currentUs == targetUs) {
    moving = false;
    Serial.println(F("Target signal reached; holding. Send zero to release."));
  }
}
