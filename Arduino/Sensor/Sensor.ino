/*
The MIT License (MIT)

Copyright (c) 2014 Faruk Can KAYA

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/wdt.h>
#include <nRF905.h>
#include <SPI.h>
//#define DEBUG_START Serial.begin(9600)
//#define DEBUG_OUT(x) Serial.println(x)
#define DEBUG_START
#define DEBUG_OUT(x)
#define NUMBER_OF_ITERATIONS_PER_MEASUREMENT 5
#define WAIT_ITERATIONS 3
ISR(WDT_vect)
{
}
void enterSleep(void)
{
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  sleep_mode();
  sleep_disable();
  power_all_enable();
}
class MeasurementData
{
public:
  byte sensorID;
  unsigned long timestamp;
  unsigned int measurementID;
  int sensorValue;
  byte iteration;
};

byte sensorId = 0xE5;
byte RXADDR[] = {0xFE, 0x4C, 0xA6, sensorId}; // Address of this device (4 bytes)
byte TXADDR[] = {0x58, 0x6F, 0x2E, 0x10}; // Address of device to send to (4 bytes)
unsigned int measurementID = 0;
int sensorAnalogPin = A5;
int sensorPowerPin = 5;
int progressPin = 4;

void flash(int pin)
{
  for(int i = 0;i<5;i++){
    digitalWrite(pin, HIGH);
    delay(100);
    digitalWrite(pin, LOW);
    delay(100);
  }
}
void setup(){
  DEBUG_START;  
  
  pinMode(sensorPowerPin, OUTPUT); 
  digitalWrite(sensorPowerPin, LOW);
  
  pinMode(progressPin, OUTPUT); 
  digitalWrite(progressPin, LOW);
  
  pinMode(sensorAnalogPin, INPUT);
  //digitalWrite(sensorAnalogPin, HIGH);
  
  nRF905_init();
  nRF905_setPayloadSize(sizeof(MeasurementData));
  nRF905_setRXAddress(RXADDR);
  nRF905_powerDown();

  MCUSR &= ~(1<<WDRF);
  WDTCSR |= (1<<WDCE) | (1<<WDE);
  WDTCSR = 1<<WDP0 | 1<<WDP3;
  WDTCSR |= _BV(WDIE);
  
  flash(progressPin);
  DEBUG_OUT("Startup");
}
void loop(){
  digitalWrite(sensorPowerPin, HIGH);
  digitalWrite(progressPin, HIGH);
  delay(100);
  int sensorValue = analogRead(sensorAnalogPin); 
  unsigned long timestamp = millis();
  DEBUG_OUT(sensorValue);
  digitalWrite(sensorPowerPin, LOW);
  measurementID++;
  
  nRF905_powerUp();
  nRF905_setTXAddress(TXADDR);
  MeasurementData measurementData;
  measurementData.sensorID = sensorId;
  measurementData.timestamp = timestamp;
  measurementData.sensorValue = sensorValue;
  measurementData.measurementID = measurementID;
  for(byte i = 0;i<NUMBER_OF_ITERATIONS_PER_MEASUREMENT;i++){    
    measurementData.iteration = i;
    while(!nRF905_setData((byte*)&measurementData, sizeof(MeasurementData))){
      DEBUG_OUT("nRF905_setData");
      delay(10);
    }
    while(!nRF905_send()){
      DEBUG_OUT("nRF905_send");
      delay(10);
    }
    while(NRF905_RADIO_STATE_TX == nRF905_getState()){
      DEBUG_OUT("nRF905_getState");
      delay(10);
    }
    if (i<NUMBER_OF_ITERATIONS_PER_MEASUREMENT)
      delay(50);
  }
  
  nRF905_powerDown();
  digitalWrite(progressPin, LOW);

  for(int i = 0;i<WAIT_ITERATIONS;i++)
    enterSleep();
}
