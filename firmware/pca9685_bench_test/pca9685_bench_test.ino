#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

// Default PCA9685 address. Change only if the board's address jumpers differ.
constexpr uint8_t PCA9685_ADDRESS = 0x40;
constexpr uint8_t TEST_CHANNEL = 0;
constexpr float SERVO_FREQUENCY_HZ = 50.0f;

// Intentionally narrow first-test window. These are not final mechanism limits.
constexpr uint16_t INITIAL_PULSE_US = 1500;
constexpr uint16_t SAFE_MIN_PULSE_US = 1400;
constexpr uint16_t SAFE_MAX_PULSE_US = 1600;
constexpr uint16_t STEP_US = 10;

Adafruit_PWMServoDriver pwm(PCA9685_ADDRESS);
uint16_t pulseUs = INITIAL_PULSE_US;
bool outputEnabled = false;

void disableAllOutputs() {
  for (uint8_t channel = 0; channel < 16; ++channel) {
    // Full-off bit: no control pulse is sent to the servo.
    pwm.setPWM(channel, 0, 4096);
  }
  outputEnabled = false;
}

void printHelp() {
  Serial.println(F("Commands:"));
  Serial.println(F("  n  enable channel 0 at 1500 us"));
  Serial.println(F("  +  increase pulse by 10 us (maximum 1600 us)"));
  Serial.println(F("  -  decrease pulse by 10 us (minimum 1400 us)"));
  Serial.println(F("  x  disable all PCA9685 outputs"));
  Serial.println(F("  ?  show this help"));
}

void applyPulse() {
  pwm.writeMicroseconds(TEST_CHANNEL, pulseUs);
  outputEnabled = true;
  Serial.print(F("Channel 0 pulse: "));
  Serial.print(pulseUs);
  Serial.println(F(" us"));
}

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ;
  }

  Wire.begin();

  Serial.println(F("InMoov PCA9685 bench test"));
  Serial.print(F("Checking I2C address 0x"));
  Serial.println(PCA9685_ADDRESS, HEX);

  Wire.beginTransmission(PCA9685_ADDRESS);
  const uint8_t i2cError = Wire.endTransmission();
  if (i2cError != 0) {
    Serial.print(F("PCA9685 not detected; I2C error "));
    Serial.println(i2cError);
    Serial.println(F("Check VCC, GND, SDA and SCL. Outputs remain unused."));
    while (true) {
      delay(1000);
    }
  }

  if (!pwm.begin()) {
    Serial.println(F("PCA9685 initialization failed."));
    while (true) {
      delay(1000);
    }
  }

  pwm.setPWMFreq(SERVO_FREQUENCY_HZ);
  delay(10);
  disableAllOutputs();

  Serial.println(F("PCA9685 detected. All outputs are OFF."));
  Serial.println(F("Keep the head disconnected; use one loose servo on channel 0."));
  printHelp();
}

void loop() {
  if (!Serial.available()) {
    return;
  }

  const char command = Serial.read();
  switch (command) {
    case 'n':
    case 'N':
      pulseUs = INITIAL_PULSE_US;
      applyPulse();
      break;

    case '+':
      if (!outputEnabled) {
        Serial.println(F("Output is OFF. Type n first."));
        break;
      }
      if (pulseUs < SAFE_MAX_PULSE_US) {
        pulseUs += STEP_US;
      }
      applyPulse();
      break;

    case '-':
      if (!outputEnabled) {
        Serial.println(F("Output is OFF. Type n first."));
        break;
      }
      if (pulseUs > SAFE_MIN_PULSE_US) {
        pulseUs -= STEP_US;
      }
      applyPulse();
      break;

    case 'x':
    case 'X':
      disableAllOutputs();
      Serial.println(F("All outputs are OFF."));
      break;

    case '?':
      printHelp();
      break;

    case '\r':
    case '\n':
      break;

    default:
      Serial.println(F("Unknown command. Type ? for help."));
      break;
  }
}
