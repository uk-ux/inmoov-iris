#include <Wire.h>
#include <EEPROM.h>

// FAST EYEBROW + FOREHEAD CALIBRATOR
// Uno: A4=SDA, A5=SCL, 5V=PCA VCC, GND=PCA GND.
// External servo supply powers PCA V+; all grounds must be common.
// Serial Monitor: 115200 baud. Newline or No line ending both work.
//
// Move: 1 100    Save neutral: 1 N
// Save lower: 1 L    Save upper: 1 U
// P = print saved calibration    0 = all PCA outputs off
//
// Servo 1 = left eyebrow CH8
// Servo 2 = right eyebrow CH9
// Servo 3 = left forehead CH12
// Servo 4 = right forehead CH13
// Position 0..180 maps to 1300..1700 us; it is not a measured angle.

const uint8_t PCA = 0x40;
const uint8_t CHANNEL[4] = {8, 9, 12, 13};
const char *NAME[4] = {"left eyebrow", "right eyebrow",
                       "left forehead", "right forehead"};
const uint16_t MIN_US = 1300, MAX_US = 1700;
const int EEPROM_BASE = 80;

struct Calibration { uint16_t lower, neutral, upper; };
Calibration saved[4];
uint16_t currentUs[4] = {1522, 1477, 1411, 1633};
bool ready = false;
char line[16];
uint8_t length = 0;
bool discard = false;
unsigned long lastByte = 0;

bool writeReg(uint8_t reg, uint8_t value) {
  Wire.beginTransmission(PCA); Wire.write(reg); Wire.write(value);
  return Wire.endTransmission() == 0;
}

bool writePWM(uint8_t channel, uint16_t ticks) {
  Wire.beginTransmission(PCA); Wire.write(0x06 + 4 * channel);
  Wire.write((uint8_t)0); Wire.write((uint8_t)0);
  Wire.write((uint8_t)(ticks & 255)); Wire.write((uint8_t)(ticks >> 8));
  return Wire.endTransmission() == 0;
}

bool allOff() {
  bool ok = true;
  for (uint8_t ch = 0; ch < 16; ++ch) if (!writePWM(ch, 4096)) ok = false;
  return ok;
}

void fault() {
  ready = false;
  allOff();
  Serial.println(F("I2C FAULT: check VCC, GND, SDA=A4 and SCL=A5."));
}

void loadCalibration() {
  for (uint8_t i = 0; i < 4; ++i) {
    EEPROM.get(EEPROM_BASE + i * sizeof(Calibration), saved[i]);
    if (saved[i].lower < MIN_US || saved[i].lower > MAX_US) saved[i].lower = 0;
    if (saved[i].neutral < MIN_US || saved[i].neutral > MAX_US) saved[i].neutral = 0;
    if (saved[i].upper < MIN_US || saved[i].upper > MAX_US) saved[i].upper = 0;
  }
}

void storeCalibration(uint8_t servo) {
  EEPROM.put(EEPROM_BASE + servo * sizeof(Calibration), saved[servo]);
}

void printCalibration() {
  Serial.println(F("servo,channel,lower_us,neutral_us,upper_us"));
  for (uint8_t i = 0; i < 4; ++i) {
    Serial.print(i + 1); Serial.print(','); Serial.print(CHANNEL[i]);
    Serial.print(','); Serial.print(saved[i].lower);
    Serial.print(','); Serial.print(saved[i].neutral);
    Serial.print(','); Serial.println(saved[i].upper);
  }
}

void processCommand() {
  if (discard || length == 0) return;

  if (length == 1 && line[0] == '0') {
    if (!allOff()) fault(); else Serial.println(F("OFF"));
    return;
  }
  if (length == 1 && (line[0] == 'P' || line[0] == 'p')) {
    printCalibration(); return;
  }
  if (!ready || length < 3 || line[0] < '1' || line[0] > '4' ||
      line[1] != ' ') return;

  uint8_t servo = line[0] - '1';
  uint8_t p = 2;
  while (p < length && line[p] == ' ') ++p;

  if (p + 1 == length) {
    char mark = line[p];
    if (mark == 'N' || mark == 'n') saved[servo].neutral = currentUs[servo];
    else if (mark == 'L' || mark == 'l') saved[servo].lower = currentUs[servo];
    else if (mark == 'U' || mark == 'u') saved[servo].upper = currentUs[servo];
    else return;
    storeCalibration(servo);
    Serial.print(NAME[servo]); Serial.print(F(" saved at "));
    Serial.print(currentUs[servo]); Serial.println(F(" us"));
    return;
  }

  int position = 0; bool hasDigit = false;
  while (p < length && line[p] >= '0' && line[p] <= '9') {
    hasDigit = true; position = position * 10 + line[p++] - '0';
    if (position > 180) return;
  }
  while (p < length && line[p] == ' ') ++p;
  if (!hasDigit || p != length) return;

  currentUs[servo] = MIN_US + (long)position * (MAX_US - MIN_US) / 180;
  if (!allOff()) { fault(); return; }
  uint16_t ticks = ((unsigned long)currentUs[servo] * 4096 + 10000) / 20000;
  if (!writePWM(CHANNEL[servo], ticks)) { fault(); return; }
  Serial.print(NAME[servo]); Serial.print(F(" CH")); Serial.print(CHANNEL[servo]);
  Serial.print(F(" position ")); Serial.print(position);
  Serial.print(F(" pulse ")); Serial.print(currentUs[servo]); Serial.println(F(" us"));
}

void setup() {
  Serial.begin(115200);
  Wire.begin(); Wire.setClock(100000); Wire.setWireTimeout(25000, true);
  Wire.beginTransmission(PCA);
  if (Wire.endTransmission() != 0) { fault(); return; }
  if (!writeReg(0x00, 0x10) || !writeReg(0xFE, 121) ||
      !writeReg(0x00, 0x20)) { fault(); return; }
  delay(5);
  if (!writeReg(0x00, 0xA0) || !allOff()) { fault(); return; }
  loadCalibration(); ready = true;
  Serial.println(F("READY fast brow/forehead calibration - outputs OFF"));
  Serial.println(F("Move 1 100 | Save 1 N/L/U | Print P | Off 0"));
}

void loop() {
  while (Serial.available()) {
    char c = Serial.read(); lastByte = millis();
    if (c == '\r' || c == '\n') {
      if (length || discard) processCommand();
      length = 0; discard = false;
    } else if (!discard) {
      if (length < sizeof(line)) line[length++] = c;
      else discard = true;
    }
  }
  if ((length || discard) && millis() - lastByte >= 40) {
    processCommand(); length = 0; discard = false;
  }
}
