#include <Wire.h>

// Uno A4=SDA, A5=SCL; PCA VCC=Uno 5V. Separate servo V+ supply, common GND.
// Serial Monitor: 115200 baud, Newline. Send 1 to hold all four at 1500 us;
// send 0 to disable all outputs. Outputs stay off at startup.
// 1500 us is position 90 on our calibration scale, not measured degrees.
const uint8_t ADDRESS = 0x40;
const uint8_t CHANNELS[4] = {8, 9, 12, 13};
bool ready = false;
char commandChar = 0;
uint8_t commandLength = 0;

bool writeRegister(uint8_t reg, uint8_t value) {
  Wire.beginTransmission(ADDRESS);
  Wire.write(reg); Wire.write(value);
  return Wire.endTransmission() == 0;
}

bool writePWM(uint8_t channel, uint16_t ticks) {
  Wire.beginTransmission(ADDRESS);
  Wire.write(0x06 + 4 * channel);
  Wire.write((uint8_t)0); Wire.write((uint8_t)0);
  Wire.write((uint8_t)(ticks & 255)); Wire.write((uint8_t)(ticks >> 8));
  return Wire.endTransmission() == 0;
}

bool allOff() {
  bool ok = true;
  for (uint8_t i = 0; i < 16; ++i) if (!writePWM(i, 4096)) ok = false;
  return ok;
}

void fault() {
  ready = false;
  allOff();
  Serial.println(F("I2C FAULT: cut servo power; check wiring and reset."));
}

void setup() {
  Serial.begin(115200);
  Wire.begin();
  Wire.setWireTimeout(25000, true);
  if (!allOff() || !writeRegister(0x00, 0x10) ||
      !writeRegister(0xFE, 121) || !writeRegister(0x00, 0x20)) {
    fault(); return;
  }
  delay(5);
  if (!writeRegister(0x00, 0xA0) || !allOff()) { fault(); return; }
  ready = true;
  Serial.println(F("READY hold position - outputs OFF. Use Newline."));
}

void loop() {
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\r' || c == '\n') {
      // Exact one-character lines only; echoed status cannot trigger motion.
      if (commandLength == 1) {
        if (commandChar == '0') {
          if (!allOff()) fault();
          else Serial.println(F("OFF"));
        } else if (ready && commandChar == '1') {
          // Same rounded 50 Hz conversion as calibration: 1500 us -> 307.
          bool ok = true;
          for (uint8_t i = 0; i < 4; ++i) {
            if (!writePWM(CHANNELS[i], 307)) { ok = false; break; }
          }
          if (!ok) fault();
          else Serial.println(F("HOLDING all four at 1500 us (position 90)"));
        }
      }
      commandLength = 0;
    } else {
      commandChar = c;
      if (commandLength < 2) ++commandLength;
    }
  }
}
