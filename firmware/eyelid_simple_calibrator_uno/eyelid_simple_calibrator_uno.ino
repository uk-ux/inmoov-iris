#include <Wire.h>
#include <EEPROM.h>
#include <stdlib.h>
#include <string.h>

// Uno: A4 SDA, A5 SCL. PCA VCC=5V; external servo V+; common GND.
// Serial: 115200 baud, NEWLINE. No Adafruit library required.
// 1 1500 = servo 1 to 1500 MICROSECONDS, NOT DEGREES.
// 1 O / 1 C = record open / closed. P = CSV. 0 = all outputs OFF.
// Only one output is enabled. First activation can jump: position is not sensed.
const uint8_t PCA=0x40, CHANNEL[4]={4,5,6,7};
const uint16_t MIN_US=1300, MAX_US=1700;
const int EEPROM_BASE=256; // Separate from the old eyelid calibrator.
struct Record { uint16_t tag,openUs,closedUs; };
Record saved[4];
const uint16_t TAG=0xEC17;
char line[24]; uint8_t length=0; bool overflow=false,ready=false;
int8_t active=-1;
uint16_t currentUs=1500,targetUs=1500;
unsigned long lastStep=0;

bool reg(uint8_t address,uint8_t value){
  Wire.beginTransmission(PCA);Wire.write(address);Wire.write(value);
  return Wire.endTransmission()==0;
}
bool pwm(uint8_t ch,uint16_t ticks){
  Wire.beginTransmission(PCA);Wire.write(0x06+4*ch);
  Wire.write((uint8_t)0);Wire.write((uint8_t)0);
  Wire.write((uint8_t)(ticks&255));Wire.write((uint8_t)(ticks>>8));
  return Wire.endTransmission()==0;
}
bool off(){
  active=-1;bool ok=true;
  for(uint8_t ch=0;ch<16;ch++)if(!pwm(ch,4096))ok=false;
  return ok;
}
void fault(){
  ready=false;off();
  Serial.println(F("I2C FAULT: outputs may remain powered. Cut servo power; check wiring and reset."));
}
bool sendPulse(){
  if(!pwm(CHANNEL[active],((uint32_t)currentUs*4096+10000)/20000)){
    fault();return false;
  }
  return true;
}
bool valid(uint16_t us){return us>=MIN_US&&us<=MAX_US;}
void printTable(){
  Serial.println(F("servo,channel,open_us,closed_us"));
  for(uint8_t i=0;i<4;i++){
    Serial.print(i+1);Serial.print(',');Serial.print(CHANNEL[i]);Serial.print(',');
    Serial.print(saved[i].openUs);Serial.print(',');Serial.println(saved[i].closedUs);
  }
  Serial.println(F("0 in table = not recorded. Save a copy; UI is NOT updated automatically."));
}
void help(){
  Serial.println(F("1=left upper CH4; 2=left lower CH5; 3=right upper CH6; 4=right lower CH7"));
  Serial.println(F("Move: 1 1500 | Save open: 1 O | Save closed: 1 C | Table: P | OFF: 0"));
  Serial.println(F("Use 1300..1700 us, NOT degrees. This guard range is NOT a verified mechanical limit."));
}
void process(){
  if(overflow){Serial.println(F("Too long; ignored."));return;}
  line[length]='\0';
  char *a=strtok(line," \t"),*b=strtok(NULL," \t"),*extra=strtok(NULL," \t");
  if(!a)return;
  if(!b&&strcmp(a,"0")==0){if(!off())fault();else Serial.println(F("OFF"));return;}
  if(!b&&(!strcmp(a,"P")||!strcmp(a,"p"))){printTable();return;}
  if(!b&&!strcmp(a,"?")){help();return;}
  if(!ready){Serial.println(F("OFFLINE: cut servo power, check I2C and reset."));return;}
  if(strlen(a)!=1||a[0]<'1'||a[0]>'4'||!b||extra){help();return;}
  const uint8_t servo=a[0]-'1';
  if(strlen(b)==1&&(b[0]=='O'||b[0]=='o'||b[0]=='C'||b[0]=='c')){
    if(active!=servo||currentUs!=targetUs){
      Serial.println(F("Move this servo first; wait for AT before saving."));return;
    }
    if(b[0]=='O'||b[0]=='o')saved[servo].openUs=currentUs;
    else saved[servo].closedUs=currentUs;
    saved[servo].tag=TAG;
    EEPROM.put(EEPROM_BASE+servo*sizeof(Record),saved[servo]);
    Serial.println(F("Saved in Uno EEPROM."));printTable();return;
  }
  // Strict short decimal parsing: reject signed, huge, or partially valid inputs.
  if(strlen(b)!=4){Serial.println(F("Use pulse e.g. 1 1500, not an angle."));return;}
  for(uint8_t i=0;i<4;i++)if(b[i]<'0'||b[i]>'9'){help();return;}
  const uint16_t us=atoi(b);
  if(!valid(us)){Serial.println(F("Rejected: allowed guard range 1300..1700 us."));return;}
  if(active!=servo){
    if(!off()){fault();return;}
    active=servo;currentUs=targetUs=us;
    if(!sendPulse())return;
    Serial.println(F("First activation: actual position unknown; movement may jump."));
  }else targetUs=us;
  Serial.print(F("Servo "));Serial.print(servo+1);Serial.print(F(" target "));Serial.println(us);
  if(currentUs==targetUs){Serial.print(F("AT "));Serial.println(currentUs);}
}
void setup(){
  Serial.begin(115200);
  for(uint8_t i=0;i<4;i++){
    EEPROM.get(EEPROM_BASE+i*sizeof(Record),saved[i]);
    if(saved[i].tag!=TAG){saved[i].tag=TAG;saved[i].openUs=saved[i].closedUs=0;}
    if(!valid(saved[i].openUs))saved[i].openUs=0;
    if(!valid(saved[i].closedUs))saved[i].closedUs=0;
  }
  Wire.begin();Wire.setClock(100000);Wire.setWireTimeout(25000,true);
  // Sleep before setting frequency; clear old ALL_LED overrides and all channels
  // while asleep, so stale outputs are not resumed when the oscillator starts.
  if(!reg(0x00,0x10)||!reg(0x01,0x04)){fault();return;}
  for(uint8_t r=0xFA;r<=0xFD;r++)if(!reg(r,0)){fault();return;}
  if(!reg(0x00,0x30)||!off()||!reg(0xFE,121)||!reg(0x00,0x20)){fault();return;}
  delay(5);
  if(!reg(0x00,0xA0)){fault();return;}
  ready=true;Serial.println(F("READY: all outputs OFF. Newline, 115200 baud."));help();
}
void loop(){
  while(Serial.available()){
    const char c=Serial.read();
    if(c=='\n'||c=='\r'){
      if(length||overflow)process();length=0;overflow=false;
    }else if(!overflow){
      if(length<sizeof(line)-1)line[length++]=c;else overflow=true;
    }
  }
  // Non-blocking ramp: stop command can be processed during motion.
  if(ready&&active>=0&&currentUs!=targetUs&&millis()-lastStep>=20){
    lastStep=millis();
    if(currentUs<targetUs)currentUs=min((int)targetUs,(int)currentUs+2);
    else currentUs=max((int)targetUs,(int)currentUs-2);
    if(sendPulse()&&currentUs==targetUs){Serial.print(F("AT "));Serial.println(currentUs);}
  }
}
