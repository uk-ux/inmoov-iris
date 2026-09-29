#include <Wire.h>

// CALIBRATED EYEBROW + FOREHEAD ANIMATION
// Uno: A4=SDA, A5=SCL, 5V=PCA VCC, common GND.
// External supply powers PCA V+. Serial Monitor: 115200 baud.
// Commands: 1=animate, 2=exact neutral, 0=all outputs off.

const uint8_t PCA = 0x40;
const uint8_t CHANNEL[4] = {8, 9, 12, 13};

// Exact calibration supplied by user on 2026-09-15.
const int LOWER_US[4]   = {1677, 1322, 1344, 1611};
const int NEUTRAL_US[4] = {1500, 1500, 1455, 1566};
const int UPPER_US[4]   = {1322, 1700, 1700, 1322};

int currentUs[4] = {1500, 1500, 1455, 1566};
int startUs[4], targetUs[4];
uint16_t previousTicks[4] = {0, 0, 0, 0};
bool ready=false, enabled=false, automatic=false, moving=false;
bool returnNext=false;
unsigned long moveStarted=0, moveTime=400, holdTime=700, nextMove=0, lastFrame=0;
char commandChar=0;
uint8_t commandLength=0;
unsigned long lastCommandByte=0;

bool writeRegister(uint8_t reg, uint8_t value) {
  Wire.beginTransmission(PCA); Wire.write(reg); Wire.write(value);
  return Wire.endTransmission()==0;
}

bool writePWM(uint8_t channel, uint16_t ticks) {
  Wire.beginTransmission(PCA); Wire.write(0x06+4*channel);
  Wire.write((uint8_t)0); Wire.write((uint8_t)0);
  Wire.write((uint8_t)(ticks&255)); Wire.write((uint8_t)(ticks>>8));
  return Wire.endTransmission()==0;
}

bool allOff() {
  automatic=false; moving=false; enabled=false;
  bool ok=true;
  for(uint8_t channel=0;channel<16;++channel) if(!writePWM(channel,4096)) ok=false;
  for(uint8_t i=0;i<4;++i) previousTicks[i]=0;
  return ok;
}

void fault() {
  ready=false; allOff();
  Serial.println(F("I2C FAULT: cut servo power and check wiring."));
}

bool sendCurrentPose() {
  for(uint8_t i=0;i<4;++i) {
    int minimum=min(LOWER_US[i],UPPER_US[i]);
    int maximum=max(LOWER_US[i],UPPER_US[i]);
    currentUs[i]=constrain(currentUs[i],minimum,maximum);
    uint16_t ticks=((unsigned long)currentUs[i]*4096+10000)/20000;
    if(ticks!=previousTicks[i]) {
      if(!writePWM(CHANNEL[i],ticks)){fault();return false;}
      previousTicks[i]=ticks;
    }
  }
  return true;
}

void beginMove(const int pose[4], unsigned long duration, unsigned long pause) {
  for(uint8_t i=0;i<4;++i){startUs[i]=currentUs[i];targetUs[i]=pose[i];}
  moveStarted=millis(); moveTime=duration; holdTime=pause; moving=true;
}

void moveNeutral(unsigned long pause) {
  beginMove(NEUTRAL_US,random(300,451),pause);
}

void moveExpression() {
  // All poses use only exact endpoints or neutral values from the new table.
  const int poses[4][4] = {
    {1322,1700,1700,1322}, // all upper / raised expression
    {1677,1322,1344,1611}, // all lower / focused expression
    {1322,1322,1700,1611}, // left raised, right lowered
    {1677,1700,1344,1322}  // left lowered, right raised
  };
  beginMove(poses[random(0,4)],random(300,501),random(500,1001));
}

void enableAtNeutral() {
  for(uint8_t i=0;i<4;++i) currentUs[i]=NEUTRAL_US[i];
  if(sendCurrentPose()) enabled=true;
}

void processCommand() {
  if(commandLength!=1)return;
  if(commandChar=='0') {
    if(!allOff())fault(); else Serial.println(F("OFF"));
    return;
  }
  if(!ready||(commandChar!='1'&&commandChar!='2'))return;
  if(!enabled)enableAtNeutral();
  if(!enabled)return;
  if(commandChar=='2') {
    automatic=false; returnNext=false; moveNeutral(0);
    Serial.println(F("NEUTRAL"));
  } else {
    automatic=true; returnNext=false; moveNeutral(500);
    Serial.println(F("ANIMATION STARTED"));
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin(); Wire.setClock(100000); Wire.setWireTimeout(25000,true);
  Wire.beginTransmission(PCA);
  if(Wire.endTransmission()!=0){fault();return;}
  if(!writeRegister(0x00,0x10)||!writeRegister(0xFE,121)||
     !writeRegister(0x00,0x20)){fault();return;}
  delay(5);
  if(!writeRegister(0x00,0xA0)||!allOff()){fault();return;}
  randomSeed(analogRead(A0)); ready=true;
  Serial.println(F("READY calibrated brow/forehead - outputs OFF"));
  Serial.println(F("1 animate | 2 neutral | 0 off"));
}

void loop() {
  while(Serial.available()) {
    char c=Serial.read(); lastCommandByte=millis();
    if(c=='\r'||c=='\n') {
      if(commandLength)processCommand();
      commandLength=0;
    } else {
      commandChar=c;
      if(commandLength<2)++commandLength;
    }
  }
  if(commandLength&&millis()-lastCommandByte>=40){processCommand();commandLength=0;}
  if(!ready||!enabled)return;

  unsigned long now=millis();
  if(now-lastFrame<20)return;
  lastFrame=now;
  if(moving) {
    unsigned long elapsed=now-moveStarted;
    float t=min(1.0f,(float)elapsed/moveTime);
    float smooth=t*t*t*(10.0f+t*(-15.0f+6.0f*t));
    for(uint8_t i=0;i<4;++i)
      currentUs[i]=startUs[i]+(int)((targetUs[i]-startUs[i])*smooth);
    if(!sendCurrentPose())return;
    if(elapsed>=moveTime){moving=false;nextMove=now+holdTime;}
  } else if(automatic&&(long)(now-nextMove)>=0) {
    if(returnNext){moveNeutral(random(700,1601));returnNext=false;}
    else{moveExpression();returnNext=true;}
  }
}
