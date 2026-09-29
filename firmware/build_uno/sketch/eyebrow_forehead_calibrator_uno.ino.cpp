#include <Arduino.h>
#line 1 "U:\\inmoov\\firmware\\eyebrow_forehead_calibrator_uno\\eyebrow_forehead_calibrator_uno.ino"
#include <Wire.h>
#include <EEPROM.h>

// Arduino Uno I2C connections:
//   A4 / SDA -> PCA9685 SDA
//   A5 / SCL -> PCA9685 SCL
// Wire.begin() configures these pins automatically.
constexpr uint8_t PCA9685_ADDRESS = 0x40;
constexpr float SERVO_FREQUENCY_HZ = 50.0f;
constexpr float PCA9685_OSCILLATOR_HZ = 25000000.0f;

// PCA9685 register addresses and MODE1 bits. This sketch talks to the driver
// directly, so no external PCA9685 library is required.
constexpr uint8_t MODE1_REGISTER = 0x00;
constexpr uint8_t LED0_ON_L_REGISTER = 0x06;
constexpr uint8_t PRESCALE_REGISTER = 0xFE;
constexpr uint8_t MODE1_RESTART = 0x80;
constexpr uint8_t MODE1_AUTO_INCREMENT = 0x20;
constexpr uint8_t MODE1_SLEEP = 0x10;

// Conservative calibration guard. Do not widen this until the mechanism has
// been inspected and the servo specification is known.
constexpr uint16_t HARD_MIN_US = 1300;
constexpr uint16_t HARD_MAX_US = 1700;
constexpr uint16_t START_US = 1500;

struct Joint {
  const char *name;
  uint8_t channel;
  uint16_t pulseUs;
  uint16_t lowerUs;
  uint16_t restUs;
  uint16_t upperUs;
};

Joint joints[] = {
  {"left eyebrow", 8, START_US, 0, 0, 0},
  {"right eyebrow", 9, START_US, 0, 0, 0},
  {"left forehead", 12, START_US, 0, 0, 0},
  {"right forehead", 13, START_US, 0, 0, 0},
};

constexpr uint8_t JOINT_COUNT = sizeof(joints) / sizeof(joints[0]);
uint8_t selected = 0;
bool outputEnabled = false;
char commandLine[16];
uint8_t commandLength = 0;
bool discardCommand = false;
unsigned long lastCommandByteMs = 0;

#line 51 "U:\\inmoov\\firmware\\eyebrow_forehead_calibrator_uno\\eyebrow_forehead_calibrator_uno.ino"
bool pcaWrite8(uint8_t reg, uint8_t value);
#line 58 "U:\\inmoov\\firmware\\eyebrow_forehead_calibrator_uno\\eyebrow_forehead_calibrator_uno.ino"
bool pcaRead8(uint8_t reg, uint8_t &value);
#line 73 "U:\\inmoov\\firmware\\eyebrow_forehead_calibrator_uno\\eyebrow_forehead_calibrator_uno.ino"
bool pcaSetPWM(uint8_t channel, uint16_t on, uint16_t off);
#line 87 "U:\\inmoov\\firmware\\eyebrow_forehead_calibrator_uno\\eyebrow_forehead_calibrator_uno.ino"
bool pcaSetFrequency(float frequencyHz);
#line 123 "U:\\inmoov\\firmware\\eyebrow_forehead_calibrator_uno\\eyebrow_forehead_calibrator_uno.ino"
bool pcaWriteMicroseconds(uint8_t channel, uint16_t microseconds);
#line 136 "U:\\inmoov\\firmware\\eyebrow_forehead_calibrator_uno\\eyebrow_forehead_calibrator_uno.ino"
void disableAllOutputs();
#line 144 "U:\\inmoov\\firmware\\eyebrow_forehead_calibrator_uno\\eyebrow_forehead_calibrator_uno.ino"
void printSelected();
#line 162 "U:\\inmoov\\firmware\\eyebrow_forehead_calibrator_uno\\eyebrow_forehead_calibrator_uno.ino"
void printHelp();
#line 166 "U:\\inmoov\\firmware\\eyebrow_forehead_calibrator_uno\\eyebrow_forehead_calibrator_uno.ino"
void saveJointCalibration(uint8_t index);
#line 173 "U:\\inmoov\\firmware\\eyebrow_forehead_calibrator_uno\\eyebrow_forehead_calibrator_uno.ino"
void loadJointCalibration();
#line 185 "U:\\inmoov\\firmware\\eyebrow_forehead_calibrator_uno\\eyebrow_forehead_calibrator_uno.ino"
void applyPulse();
#line 197 "U:\\inmoov\\firmware\\eyebrow_forehead_calibrator_uno\\eyebrow_forehead_calibrator_uno.ino"
void selectJoint(uint8_t index);
#line 205 "U:\\inmoov\\firmware\\eyebrow_forehead_calibrator_uno\\eyebrow_forehead_calibrator_uno.ino"
void processShortCommand();
#line 280 "U:\\inmoov\\firmware\\eyebrow_forehead_calibrator_uno\\eyebrow_forehead_calibrator_uno.ino"
void printCalibration();
#line 296 "U:\\inmoov\\firmware\\eyebrow_forehead_calibrator_uno\\eyebrow_forehead_calibrator_uno.ino"
void setup();
#line 333 "U:\\inmoov\\firmware\\eyebrow_forehead_calibrator_uno\\eyebrow_forehead_calibrator_uno.ino"
void loop();
#line 51 "U:\\inmoov\\firmware\\eyebrow_forehead_calibrator_uno\\eyebrow_forehead_calibrator_uno.ino"
bool pcaWrite8(uint8_t reg, uint8_t value) {
  Wire.beginTransmission(PCA9685_ADDRESS);
  Wire.write(reg);
  Wire.write(value);
  return Wire.endTransmission() == 0;
}

bool pcaRead8(uint8_t reg, uint8_t &value) {
  Wire.beginTransmission(PCA9685_ADDRESS);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0) {
    return false;
  }

  if (Wire.requestFrom(PCA9685_ADDRESS, (uint8_t)1) != 1) {
    return false;
  }

  value = Wire.read();
  return true;
}

bool pcaSetPWM(uint8_t channel, uint16_t on, uint16_t off) {
  if (channel > 15) {
    return false;
  }

  Wire.beginTransmission(PCA9685_ADDRESS);
  Wire.write(LED0_ON_L_REGISTER + 4 * channel);
  Wire.write(on & 0xFF);
  Wire.write(on >> 8);
  Wire.write(off & 0xFF);
  Wire.write(off >> 8);
  return Wire.endTransmission() == 0;
}

bool pcaSetFrequency(float frequencyHz) {
  float prescaleValue = PCA9685_OSCILLATOR_HZ;
  prescaleValue /= 4096.0f;
  prescaleValue /= frequencyHz;
  prescaleValue -= 1.0f;

  uint16_t roundedPrescale = (uint16_t)(prescaleValue + 0.5f);
  if (roundedPrescale < 3) {
    roundedPrescale = 3;
  }
  if (roundedPrescale > 255) {
    roundedPrescale = 255;
  }

  uint8_t oldMode = 0;
  if (!pcaRead8(MODE1_REGISTER, oldMode)) {
    return false;
  }

  const uint8_t sleepMode = (oldMode & ~MODE1_RESTART) | MODE1_SLEEP;
  if (!pcaWrite8(MODE1_REGISTER, sleepMode)) {
    return false;
  }
  if (!pcaWrite8(PRESCALE_REGISTER, (uint8_t)roundedPrescale)) {
    return false;
  }
  if (!pcaWrite8(MODE1_REGISTER, oldMode)) {
    return false;
  }

  delay(5);
  return pcaWrite8(
      MODE1_REGISTER,
      oldMode | MODE1_RESTART | MODE1_AUTO_INCREMENT);
}

bool pcaWriteMicroseconds(uint8_t channel, uint16_t microseconds) {
  float ticksValue = microseconds;
  ticksValue *= SERVO_FREQUENCY_HZ;
  ticksValue *= 4096.0f;
  ticksValue /= 1000000.0f;

  uint16_t ticks = (uint16_t)(ticksValue + 0.5f);
  if (ticks > 4095) {
    ticks = 4095;
  }
  return pcaSetPWM(channel, 0, ticks);
}

void disableAllOutputs() {
  for (uint8_t channel = 0; channel < 16; ++channel) {
    // Bit 12 in the OFF register means fully off.
    pcaSetPWM(channel, 0, 4096);
  }
  outputEnabled = false;
}

void printSelected() {
  const long approximateAngle = map(
      joints[selected].pulseUs,
      HARD_MIN_US,
      HARD_MAX_US,
      0,
      180);
  Serial.print(F("Selected: "));
  Serial.print(joints[selected].name);
  Serial.print(F(" | channel "));
  Serial.print(joints[selected].channel);
  Serial.print(F(" | pulse "));
  Serial.print(joints[selected].pulseUs);
  Serial.print(F(" us | calibration position "));
  Serial.print(approximateAngle);
  Serial.println(F(" (not measured degrees)"));
}

void printHelp() {
  Serial.println(F("Move: 1 90 | Save: 1 N, 1 L, 1 U | Print: P | Off: 0"));
}

void saveJointCalibration(uint8_t index) {
  const int address = index * 6;
  EEPROM.put(address + 0, joints[index].lowerUs);
  EEPROM.put(address + 2, joints[index].restUs);
  EEPROM.put(address + 4, joints[index].upperUs);
}

void loadJointCalibration() {
  for (uint8_t i = 0; i < JOINT_COUNT; ++i) {
    const int address = i * 6;
    EEPROM.get(address + 0, joints[i].lowerUs);
    EEPROM.get(address + 2, joints[i].restUs);
    EEPROM.get(address + 4, joints[i].upperUs);
    if (joints[i].lowerUs < HARD_MIN_US || joints[i].lowerUs > HARD_MAX_US) joints[i].lowerUs = 0;
    if (joints[i].restUs < HARD_MIN_US || joints[i].restUs > HARD_MAX_US) joints[i].restUs = 0;
    if (joints[i].upperUs < HARD_MIN_US || joints[i].upperUs > HARD_MAX_US) joints[i].upperUs = 0;
  }
}

void applyPulse() {
  if (!pcaWriteMicroseconds(
          joints[selected].channel,
          joints[selected].pulseUs)) {
    Serial.println(F("I2C write failed. Output state is unknown."));
    outputEnabled = false;
    return;
  }
  outputEnabled = true;
  printSelected();
}

void selectJoint(uint8_t index) {
  disableAllOutputs();
  selected = index;
  Serial.println(F("Previous output disabled."));
  printSelected();
  Serial.println(F("Type c to enable this servo at 1500 us."));
}

void processShortCommand() {
  commandLine[commandLength] = '\0';

  // A single zero is the emergency all-off command.
  if (commandLength == 1 && commandLine[0] == '0') {
    disableAllOutputs();
    Serial.println(F("OFF"));
    return;
  }

  if (commandLength == 1 && (commandLine[0] == 'P' || commandLine[0] == 'p')) {
    printCalibration();
    return;
  }

  // Only accept: servo number (1-4), a separator, then angle (0-180).
  // Invalid or echoed serial text is ignored silently for safety.
  if (commandLength < 3 || commandLine[0] < '1' || commandLine[0] > '4') {
    return;
  }

  uint8_t position = 1;
  if (commandLine[position] != ' ' && commandLine[position] != ',') {
    return;
  }
  while (position < commandLength &&
         (commandLine[position] == ' ' || commandLine[position] == ',')) {
    ++position;
  }
  if (position >= commandLength) {
    return;
  }

  selected = commandLine[0] - '1';
  if (position + 1 == commandLength &&
      (commandLine[position] == 'N' || commandLine[position] == 'n' ||
       commandLine[position] == 'L' || commandLine[position] == 'l' ||
       commandLine[position] == 'U' || commandLine[position] == 'u')) {
    const char mark = commandLine[position];
    if (mark == 'N' || mark == 'n') joints[selected].restUs = joints[selected].pulseUs;
    if (mark == 'L' || mark == 'l') joints[selected].lowerUs = joints[selected].pulseUs;
    if (mark == 'U' || mark == 'u') joints[selected].upperUs = joints[selected].pulseUs;
    saveJointCalibration(selected);
    Serial.println(F("Saved to EEPROM."));
    return;
  }

  uint16_t angle = 0;
  bool foundDigit = false;
  while (position < commandLength &&
         commandLine[position] >= '0' && commandLine[position] <= '9') {
    foundDigit = true;
    angle = angle * 10 + (commandLine[position] - '0');
    if (angle > 180) {
      return;
    }
    ++position;
  }
  while (position < commandLength && commandLine[position] == ' ') {
    ++position;
  }
  if (!foundDigit || position != commandLength || angle > 180) {
    return;
  }

  disableAllOutputs();
  joints[selected].pulseUs = (uint16_t)map(
      angle,
      0,
      180,
      HARD_MIN_US,
      HARD_MAX_US);
  applyPulse();
}

void printCalibration() {
  Serial.println(F("joint,channel,lower_us,rest_us,upper_us"));
  for (uint8_t i = 0; i < JOINT_COUNT; ++i) {
    Serial.print(joints[i].name);
    Serial.print(',');
    Serial.print(joints[i].channel);
    Serial.print(',');
    Serial.print(joints[i].lowerUs);
    Serial.print(',');
    Serial.print(joints[i].restUs);
    Serial.print(',');
    Serial.println(joints[i].upperUs);
  }
  Serial.println(F("A zero means that value has not been recorded."));
}

void setup() {
  Serial.begin(115200);
  Serial.setTimeout(150);
  Wire.begin();

  Wire.beginTransmission(PCA9685_ADDRESS);
  const uint8_t i2cError = Wire.endTransmission();
  if (i2cError != 0) {
    Serial.print(F("PCA9685 not detected at 0x40; I2C error "));
    Serial.println(i2cError);
    while (true) {
      delay(1000);
    }
  }

  if (!pcaWrite8(MODE1_REGISTER, 0x00)) {
    Serial.println(F("PCA9685 reset failed."));
    while (true) {
      delay(1000);
    }
  }

  delay(10);
  if (!pcaSetFrequency(SERVO_FREQUENCY_HZ)) {
    Serial.println(F("Could not set PCA9685 frequency."));
    while (true) {
      delay(1000);
    }
  }
  delay(10);
  disableAllOutputs();
  loadJointCalibration();

  Serial.println(F("READY - all outputs OFF"));
  printHelp();
}

void loop() {
  while (Serial.available()) {
    const char incoming = Serial.read();
    lastCommandByteMs = millis();

    if (incoming == '\r' || incoming == '\n') {
      if (!discardCommand && commandLength > 0) {
        processShortCommand();
      }
      commandLength = 0;
      discardCommand = false;
    } else if (discardCommand) {
      // Ignore the entire remainder of an overlong command.
    } else if (commandLength < sizeof(commandLine) - 1) {
      commandLine[commandLength++] = incoming;
    } else {
      // Discard an overlong line instead of acting on a partial command.
      commandLength = 0;
      discardCommand = true;
    }
  }

  // Also accept commands when Serial Monitor is set to "No line ending".
  if ((commandLength > 0 || discardCommand) && millis() - lastCommandByteMs >= 75) {
    if (!discardCommand) {
      processShortCommand();
    }
    commandLength = 0;
    discardCommand = false;
  }
}

