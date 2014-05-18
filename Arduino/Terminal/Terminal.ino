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
#include <nRF905.h>
#include <SPI.h>
class MeasurementData
{
public:
  byte sensorID;
  unsigned long timestamp;
  unsigned int measurementID;
  int sensorValue;
  byte iteration;
};


#define DEBUG_OUT(x) Serial.println(x)
#define RXADDR {0x58, 0x6F, 0x2E, 0x10} // Address of this device (4 bytes)
#define TXADDR {0xFE, 0x4C, 0xA6, 0xE5} // Address of device to send to (4 bytes)


void setup(){
  Serial.begin(9600);
  
  nRF905_init();
  byte addr[] = RXADDR;
  nRF905_setRXAddress(addr);
  nRF905_setPayloadSize(sizeof(MeasurementData));	
  nRF905_receive();
}

void loop(){
  Serial.println(F("Waiting for data..."));
  MeasurementData measurementData;
  while(!nRF905_getData((byte*)&measurementData, sizeof(MeasurementData)));
  Serial.println(F("Got measurement"));
  Serial.println(measurementData.sensorValue);
  Serial.println(measurementData.measurementID);
  Serial.println(measurementData.iteration);
  Serial.println(measurementData.timestamp);
}
