#include <Arduino.h>
#line 1 "U:\\inmoov\\firmware\\nose_calibrator_uno\\nose_calibrator_uno.ino"
#include <Wire.h>
#include <EEPROM.h>

// Nose only, PCA9685 CH10. Uno A4=SDA, A5=SCL, common GND.
// External servo supply to V+. Uno 5V supplies PCA VCC logic only.
// Serial Monitor: 115200 baud, NEWLINE required.
// 0..180 = calibration position (1300..1700 us), NOT physical degrees.
// N/L/U save current neutral/down/up. P prints. X turns all outputs off.
// No automatic movement at startup. First number enables CH10 at that pulse.
const uint8_t PCA=0x40, CHANNEL=10;
const int MIN_US=1300, MAX_US=1700, EEPROM_BASE=192;
struct Calibration { uint16_t magic, lower, neutral, upper; };
Calibration saved={0x4E31,0,0,0};
bool ready=false, enabled=false, discard=false;
int currentUs=1500, targetUs=1500;
char line[12]; uint8_t length=0;
unsigned long frame=0;

#line 19 "U:\\inmoov\\firmware\\nose_calibrator_uno\\nose_calibrator_uno.ino"
bool regWrite(uint8_t reg,uint8_t value);
#line 23 "U:\\inmoov\\firmware\\nose_calibrator_uno\\nose_calibrator_uno.ino"
bool pwm(uint8_t channel,uint16_t ticks);
#line 29 "U:\\inmoov\\firmware\\nose_calibrator_uno\\nose_calibrator_uno.ino"
bool off();
#line 34 "U:\\inmoov\\firmware\\nose_calibrator_uno\\nose_calibrator_uno.ino"
void fault();
#line 35 "U:\\inmoov\\firmware\\nose_calibrator_uno\\nose_calibrator_uno.ino"
bool sendPulse();
#line 39 "U:\\inmoov\\firmware\\nose_calibrator_uno\\nose_calibrator_uno.ino"
bool valid(uint16_t v);
#line 40 "U:\\inmoov\\firmware\\nose_calibrator_uno\\nose_calibrator_uno.ino"
void printSaved();
#line 46 "U:\\inmoov\\firmware\\nose_calibrator_uno\\nose_calibrator_uno.ino"
void report();
#line 47 "U:\\inmoov\\firmware\\nose_calibrator_uno\\nose_calibrator_uno.ino"
void command();
#line 75 "U:\\inmoov\\firmware\\nose_calibrator_uno\\nose_calibrator_uno.ino"
void setup();
#line 87 "U:\\inmoov\\firmware\\nose_calibrator_uno\\nose_calibrator_uno.ino"
void loop();
#line 19 "U:\\inmoov\\firmware\\nose_calibrator_uno\\nose_calibrator_uno.ino"
bool regWrite(uint8_t reg,uint8_t value){
  Wire.beginTransmission(PCA);Wire.write(reg);Wire.write(value);
  return Wire.endTransmission()==0;
}
bool pwm(uint8_t channel,uint16_t ticks){
  Wire.beginTransmission(PCA);Wire.write(0x06+4*channel);
  Wire.write((uint8_t)0);Wire.write((uint8_t)0);
  Wire.write((uint8_t)(ticks&255));Wire.write((uint8_t)(ticks>>8));
  return Wire.endTransmission()==0;
}
bool off(){
  enabled=false;targetUs=currentUs;
  bool ok=true;for(uint8_t ch=0;ch<16;ch++)if(!pwm(ch,4096))ok=false;
  return ok;
}
void fault(){ready=false;off();Serial.println(F("I2C FAULT: cut servo power; check wiring. Reset to retry."));}
bool sendPulse(){
  if(!pwm(CHANNEL,((long)currentUs*4096+10000)/20000)){fault();return false;}
  return true;
}
bool valid(uint16_t v){return v==0||(v>=MIN_US&&v<=MAX_US);}
void printSaved(){
  Serial.println(F("servo,channel,lower_us,neutral_us,upper_us"));
  Serial.print(F("1,10,"));Serial.print(saved.lower);Serial.print(',');
  Serial.print(saved.neutral);Serial.print(',');Serial.println(saved.upper);
  Serial.println(F("0 in table = not saved. L=physical down, U=physical up."));
}
void report(){Serial.print(F("Nose CH10 pulse "));Serial.print(currentUs);Serial.println(F(" us. N/L/U to save."));}
void command(){
  if(discard){Serial.println(F("Command too long"));return;}
  if(!length)return;
  if(length==1){
    char c=line[0];if(c>='a'&&c<='z')c-=32;
    if(c=='X'){if(!off())fault();else Serial.println(F("OFF"));return;}
    if(c=='P'){printSaved();return;}
    if(c=='N'||c=='L'||c=='U'){
      if(!ready||!enabled||currentUs!=targetUs){Serial.println(F("Move first and wait until stopped before saving."));return;}
      if(c=='N')saved.neutral=currentUs;
      if(c=='L')saved.lower=currentUs;
      if(c=='U')saved.upper=currentUs;
      EEPROM.put(EEPROM_BASE,saved);printSaved();return;
    }
  }
  int position=0;
  for(uint8_t i=0;i<length;i++){
    if(line[i]<'0'||line[i]>'9'){Serial.println(F("Use 0..180, N, L, U, P or X"));return;}
    position=position*10+line[i]-'0';
    if(position>180){Serial.println(F("Position must be 0..180"));return;}
  }
  if(!ready)return;
  targetUs=MIN_US+(long)position*(MAX_US-MIN_US)/180;
  if(!enabled){
    // Actual unpowered shaft position is unknown; first enable may move abruptly.
    currentUs=targetUs;if(!sendPulse())return;enabled=true;report();
  }else if(targetUs==currentUs)report();
}
void setup(){
  Serial.begin(115200);Wire.begin();Wire.setClock(100000);Wire.setWireTimeout(25000,true);
  if(!off()||!regWrite(0x00,0x10)||!regWrite(0xFE,121)||!regWrite(0x00,0x20)){fault();return;}
  delay(5);if(!regWrite(0x00,0xA0)||!off()){fault();return;}
  Calibration loaded;EEPROM.get(EEPROM_BASE,loaded);
  if(loaded.magic==saved.magic&&valid(loaded.lower)&&valid(loaded.neutral)&&valid(loaded.upper))saved=loaded;
  ready=true;
  Serial.println(F("READY nose CH10 - all outputs OFF. Set Serial Monitor to Newline."));
  Serial.println(F("Move: 90 then 95 etc. Save: N neutral, L down, U up. P table. X OFF."));
  Serial.println(F("1300..1700 us is a test window, NOT established safe mechanical limits."));
  printSaved();
}
void loop(){
  while(Serial.available()){
    char c=Serial.read();
    if(c=='\n'||c=='\r'){command();length=0;discard=false;}
    else if(!discard){if(length<sizeof(line))line[length++]=c;else discard=true;}
  }
  if(!ready||!enabled||currentUs==targetUs||millis()-frame<20)return;
  frame=millis();
  currentUs+=constrain(targetUs-currentUs,-4,4);
  if(!sendPulse())return;
  if(currentUs==targetUs)report();
}

