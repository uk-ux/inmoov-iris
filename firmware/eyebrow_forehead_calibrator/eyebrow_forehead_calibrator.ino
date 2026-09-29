#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

constexpr uint8_t PCA9685_ADDRESS = 0x40;
constexpr float SERVO_FREQUENCY_HZ = 50.0f;

// Conservative calibration guard. Do not widen this until the mechanism has
// been inspected and its servo specification is known.
constexpr uint16_t HARD_MIN_US = 1300;
constexpr uint16_t HARD_MAX_US = 1700;
constexpr uint16_t START_US = 1500;
constexpr uint16_t STEP_US = 5;

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
Adafruit_PWMServoDriver pwm(PCA9685_ADDRESS);
uint8_t selected = 0;
bool outputEnabled = false;

void disableAllOutputs() {
  for (uint8_t channel = 0; channel < 16; ++channel) {
    pwm.setPWM(channel, 0, 4096);  // PCA9685 full-off bit
  }
  outputEnabled = false;
}

void printSelected() {
  Serial.print(F("Selected: "));
  Serial.print(joints[selected].name);
  Serial.print(F(" | channel "));
  Serial.print(joints[selected].channel);
  Serial.print(F(" | pulse "));
  Serial.print(joints[selected].pulseUs);
  Serial.println(F(" us"));
}

void printHelp() {
  Serial.println(F("Commands:"));
  Serial.println(F("  1 = select left eyebrow (CH8)"));
  Serial.println(F("  2 = select right eyebrow (CH9)"));
  Serial.println(F("  3 = select left forehead (CH12)"));
  Serial.println(F("  4 = select right forehead (CH13)"));
  Serial.println(F("  c = enable selected servo at 1500 us"));
  Serial.println(F("  + / - = move by 5 us"));
  Serial.println(F("  l = record current pulse as lower limit"));
  Serial.println(F("  r = record current pulse as neutral/rest"));
  Serial.println(F("  u = record current pulse as upper limit"));
  Serial.println(F("  p = print calibration table"));
  Serial.println(F("  x = disable all outputs"));
  Serial.println(F("  ? = show help"));
}

void applyPulse() {
  pwm.writeMicroseconds(joints[selected].channel, joints[selected].pulseUs);
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
  while (!Serial) {
    ;
  }

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

  if (!pwm.begin()) {
    Serial.println(F("PCA9685 initialization failed."));
    while (true) {
      delay(1000);
    }
  }

  pwm.setPWMFreq(SERVO_FREQUENCY_HZ);
  delay(10);
  disableAllOutputs();

  Serial.println(F("Eyebrow and forehead calibrator ready."));
  Serial.println(F("All PWM outputs are OFF."));
  printHelp();
}

void loop() {
  if (!Serial.available()) {
    return;
  }

  const char command = Serial.read();
  switch (command) {
    case '1':
      selectJoint(0);
      break;
    case '2':
      selectJoint(1);
      break;
    case '3':
      selectJoint(2);
      break;
    case '4':
      selectJoint(3);
      break;

    case 'c':
    case 'C':
      joints[selected].pulseUs = START_US;
      applyPulse();
      break;

    case '+':
      if (!outputEnabled) {
        Serial.println(F("Output is OFF. Type c first."));
      } else if (joints[selected].pulseUs < HARD_MAX_US) {
        joints[selected].pulseUs += STEP_US;
        applyPulse();
      } else {
        Serial.println(F("1700 us guard reached."));
      }
      break;

    case '-':
      if (!outputEnabled) {
        Serial.println(F("Output is OFF. Type c first."));
      } else if (joints[selected].pulseUs > HARD_MIN_US) {
        joints[selected].pulseUs -= STEP_US;
        applyPulse();
      } else {
        Serial.println(F("1300 us guard reached."));
      }
      break;

    case 'l':
    case 'L':
      joints[selected].lowerUs = joints[selected].pulseUs;
      Serial.println(F("Lower pulse recorded in RAM."));
      break;

    case 'r':
    case 'R':
      joints[selected].restUs = joints[selected].pulseUs;
      Serial.println(F("Neutral/rest pulse recorded in RAM."));
      break;

    case 'u':
    case 'U':
      joints[selected].upperUs = joints[selected].pulseUs;
      Serial.println(F("Upper pulse recorded in RAM."));
      break;

    case 'p':
    case 'P':
      printCalibration();
      break;

    case 'x':
    case 'X':
      disableAllOutputs();
      Serial.println(F("All PWM outputs are OFF."));
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

