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
#include <Wire.h>
#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

IPAddress ip(192,168,1,8);
byte server[] = { 95,85,36,115 };

EthernetClient ethClient;
PubSubClient client(server, 1883, callback, ethClient);
int resetPin = 3;
class MeasurementData
{
public:
  byte sensorID;
  unsigned long timestamp;
  unsigned int measurementID;
  int sensorValue;
  byte iteration;
};
boolean connectToServer()
{
  Serial.println("Connecting to server...");
  if (client.connect("soilHumidityTerminalMaster")) {
    client.subscribe("gardenSoilHumidityMeasure");
    Serial.println("Connected to server.");
    return true;
  }
  Serial.println("Failed to connect to server.");
  return false;
}
boolean dhcpSetupSuccessfull=false;
void setup(){
  digitalWrite(resetPin, HIGH);
  pinMode(resetPin, OUTPUT);
  
  Serial.begin(9600);
  Wire.begin();
  Serial.println("Configuring Ethernet using DHCP...");
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP.");
    delay(1000);
    digitalWrite(resetPin, LOW);
  }
}
long delayDuration = 500;
long failureCount = 0;
void loop(){
  if (!dhcpSetupSuccessfull){
    Serial.println("Configuring Ethernet using DHCP...");
    if (Ethernet.begin(mac) == 0) {
      Serial.println("Failed to configure Ethernet using DHCP.");
    }else{
      Serial.println("Configured Ethernet using DHCP.");
      dhcpSetupSuccessfull=true;
    }
    return;
  }
  if (!client.connected()){
    if (failureCount>15)
      digitalWrite(resetPin, LOW);
    Serial.print("Not connected will try to connect in ");
    Serial.print(delayDuration);
    Serial.println("ms");
    delay(delayDuration);
    delayDuration = delayDuration * 2;
    failureCount ++;
    if (delayDuration > 60000)
      delayDuration = 60000;
      if (failureCount % 4 == 0){
        Serial.println("Reconfiguring Ethernet using DHCP...");
        Ethernet.maintain();
      }
    connectToServer();
  }else{    
    delayDuration = 500;
    failureCount = 0;
  }
  client.loop();
}
byte* readMeasurement()
{
  Wire.requestFrom(2, sizeof(MeasurementData) + 1);
  if (Wire.available() >= sizeof(MeasurementData) + 1)
  {
    byte r = Wire.read();
    if (r==0x01)
    {
      MeasurementData* m = new MeasurementData();
      Wire.readBytes((char*)m, sizeof(MeasurementData));
      return (byte*)m;
    }
    else{
      return NULL;
    }
  }
  while(Wire.available())    // slave may send less than requested
  { 
    byte r = Wire.read();
  }
}
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.println(topic);
  MeasurementData* m = (MeasurementData*)readMeasurement();  
  if (m){
    char buffer[128];
    char h[16];
    dtostrf(m->sensorValue, 5, 2, h);
    sprintf(buffer, "{\"sensor1\":%s}", h);
    client.publish("gardenSoilHumidityMeasurementResult",buffer);
    Serial.println(buffer);
    delete m;
  }
  else{
    client.publish("gardenSoilHumidityMeasurementResult","{}");    
  }
}
