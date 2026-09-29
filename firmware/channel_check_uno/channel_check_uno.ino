#include <Wire.h>

// Uno A4 -> SDA, A5 -> SCL, 5V -> VCC, shared GND.
// External servo supply -> V+ and GND. Serial Monitor: 115200, Newline.
// Send 1, 2, 3 or 4 to test channel 8, 9, 12 or 13 respectively.
// Only that channel gently rocks around 1450 us.
// Send 0: disable all PWM. No movement at startup.
// 1410-1490 us is inside the overlap of the four recorded travel ranges.
// This is an identification position, NOT any servo's calibrated neutral.
const uint8_t ADDRESS = 0x40;
const uint8_t CHANNELS[4] = {8, 9, 12, 13};
int activeChannel = -1;
bool ready = false;
unsigned long started = 0, lastFrame = 0;
uint16_t previousTicks = 0;
char line[8];
uint8_t used = 0;
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
  activeChannel = -1;
  previousTicks = 0;
  bool ok = true;
  for (uint8_t i = 0; i < 16; ++i) if (!pwmWrite(i, 4096)) ok = false;
  return ok;
}

void fault() {
  ready = false;
  stopAll();
  Serial.println(F("I2C FAULT: switch off servo power and check wiring."));
}

void command() {
  if (discard) return;
  if (used == 1 && line[0] == '0') {
    if (!stopAll()) fault();
    else Serial.println(F("OFF"));
    return;
  }
  if (!ready || used != 1 || line[0] < '1' || line[0] > '4') return;
  if (!stopAll()) { fault(); return; }
  activeChannel = CHANNELS[line[0] - '1'];
  started = millis();
  Serial.print(F("Testing channel ")); Serial.println(activeChannel);
}

void setup() {
  Serial.begin(115200);
  Wire.begin();
  Wire.setWireTimeout(25000, true);
  if (!stopAll() || !regWrite(0x00, 0x10) ||
      !regWrite(0xFE, 121) || !regWrite(0x00, 0x20)) {
    fault(); return;
  }
  delay(5);
  if (!regWrite(0x00, 0xA0) || !stopAll()) { fault(); return; }
  ready = true;
  Serial.println(F("READY simple channel check - outputs OFF. Use Newline."));
}

void loop() {
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\r' || c == '\n') {
      command(); used = 0; discard = false;
    } else if (!discard) {
      if (used < sizeof(line)) line[used++] = c;
      else discard = true;
    }
  }
  if (!ready || activeChannel < 0) return;
  unsigned long now = millis();
  // Automatically release the channel after two gentle cycles.
  if (now - started >= 6000) {
    if (!stopAll()) fault();
    else Serial.println(F("Test finished - OFF"));
    return;
  }
  if (now - lastFrame < 20) return;
  lastFrame = now;
  float phase = ((now - started) % 3000) * (6.2831853f / 3000.0f);
  int pulse = 1450 + (int)(40.0f * sin(phase));
  uint16_t ticks = ((unsigned long)pulse * 4096 + 10000) / 20000;
  if (ticks != previousTicks) {
    if (!pwmWrite(activeChannel, ticks)) { fault(); return; }
    previousTicks = ticks;
  }
}
