#include <JeeLib.h>
#include <SPI.h>
#include <Mirf.h>
#include <nRF24L01.h>
#include <MirfHardwareSpiDriver.h>
ISR(WDT_vect) { Sleepy::watchdogEvent(); }
//#define SLEEP0(x) delay(x)
//#define DEBUG_START Serial.begin(9600)
//#define DEBUG_OUT(x) Serial.println(x)
#define SLEEP0(x) Sleepy::loseSomeTime(x)
#define DEBUG_START
#define DEBUG_OUT(x)

byte payload[sizeof(unsigned long) + sizeof(int) + 5];
char* clientAddress = "clie1";
int sensorAnalogPin = A0;
int sensorPowerPin = 9;
int progressPin = 2;
int successPin = 3;
int failPin = 4;
void setup(){
  DEBUG_START;
  Mirf.spi = &MirfHardwareSpi;
  Mirf.init();
  Mirf.setRADDR((byte *)clientAddress);
  Mirf.payload = sizeof(int);
  Mirf.config();
  Mirf.configRegister(SETUP_RETR, 0xFF);
  Mirf.configRegister(RF_SETUP, 0x06);    
  
  pinMode(sensorPowerPin, OUTPUT); 
  digitalWrite(sensorPowerPin, LOW);
  
  pinMode(progressPin, OUTPUT); 
  digitalWrite(progressPin, LOW);
  
  pinMode(successPin, OUTPUT); 
  digitalWrite(successPin, LOW);
  
  pinMode(failPin, OUTPUT); 
  digitalWrite(failPin, LOW);
  
  pinMode(sensorAnalogPin, INPUT);
  //digitalWrite(sensorAnalogPin, HIGH);
  Mirf.powerDown();
}
void flash(int pin)
{
  for(int i = 0;i<5;i++){
    digitalWrite(pin, HIGH);
    SLEEP0(100);
    digitalWrite(pin, LOW);
    SLEEP0(100);
  }
}
void loop(){
  unsigned long time = millis();
  digitalWrite(sensorPowerPin, HIGH);
  digitalWrite(progressPin, HIGH);
  SLEEP0(100);
  int sensorValue = analogRead(sensorAnalogPin); 
  DEBUG_OUT(sensorValue);
  digitalWrite(sensorPowerPin, LOW);
  
  memcpy(payload, (byte *)&sensorValue, sizeof(int));
  
  Mirf.setTADDR((byte *)"serv1");
  
  Mirf.send(payload);
  SLEEP0(100);
  int success = 0;
  while (Mirf.PTX) {
    uint8_t st = Mirf.getStatus();
    if (st & (1 << TX_DS))
    {
      success = 1;
      break;
    }else if (st & (1 << MAX_RT)){
      break;
    }
  }      
  while(Mirf.isSending()){
    SLEEP0(10);
  }
  Mirf.powerDown();
  digitalWrite(progressPin, LOW);
  if (success)
    flash(successPin);
  else
    flash(failPin);
  SLEEP0(14000);
}