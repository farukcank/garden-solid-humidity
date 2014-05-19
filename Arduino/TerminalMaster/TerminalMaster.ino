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
#define WEBSITE  "api.carriots.com"
#define API_KEY "90bde81f3406173a38c7e6b93ed380e66702ada196f0b3e9bd0b3037cfa63b5e"
#define DEVICE  "SoilHumidity@farukcank.farukcank"
byte mac[] = { 
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
EthernetUDP Udp;

unsigned int localPort = 8888;      // local port to listen for UDP packets

IPAddress timeServer(132, 163, 4, 101); // time-a.timefreq.bldrdoc.gov NTP server
// IPAddress timeServer(132, 163, 4, 102); // time-b.timefreq.bldrdoc.gov NTP server
// IPAddress timeServer(132, 163, 4, 103); // time-c.timefreq.bldrdoc.gov NTP server

const int NTP_PACKET_SIZE= 48; // NTP time stamp is in the first 48 bytes of the message

byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets 


IPAddress ip(192,168,1,8);
class MeasurementData
{
public:
  byte sensorID;
  unsigned long timestamp;
  unsigned int measurementID;
  int sensorValue;
  byte iteration;
};
void setup(){
  Serial.begin(9600);
  Wire.begin();
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // no point in carrying on, so do nothing forevermore:
    // try to congifure using IP address instead of DHCP:
    Ethernet.begin(mac, ip);
  }
  Udp.begin(localPort);
}
void postData(const char* server, const char* path, const char* data)
{
  EthernetClient client;
  if (client.connect(server,80))
  {    
    char buffer[128];
    sprintf(buffer, "POST %s HTTP/1.1",path);
    client.println(buffer);
    sprintf(buffer, "Host: %s",server);
    client.println(buffer);
    client.println("Connection: Close");
    client.println("User-Agent: Arduino");    
    sprintf(buffer, "carriots.apiKey:%s", API_KEY);    
    client.println(buffer);
    sprintf(buffer, "Content-Length: %d", strlen(data));    
    client.println(buffer);
    client.println();
    client.print(data);
    client.println();
    for(int i = 0;i<50;i++)
    {
      if (client.available()>0)
        break;
      delay(100);
    }
    do
    {
      char c = client.read();
      Serial.print(c);
    }
    while(client.available()>0);
    client.stop();
  }
  else
  {
    Serial.println("Cannot connect to Server");
  }
}
void sendData(double soilHumidity)
{
  char buffer[128];
  char h[16];
  dtostrf(soilHumidity, 5, 2, h);
  sprintf(buffer, "{\"protocol\":\"v1\",\"at\":%ld,\"device\":\"%s\",\"data\":{\"soilHumidity\":%s},\"checksum\":\"\"}", ntpTime(), DEVICE, h);
  postData(WEBSITE, "/streams", buffer);
}
void loop(){
  Wire.requestFrom(2, sizeof(MeasurementData) + 1);
  if (Wire.available() >= sizeof(MeasurementData) + 1)
  {
    byte r = Wire.read();
    if (r==0x01)
    {
      MeasurementData m;
      Wire.readBytes((char*)&m, sizeof(m));
      Serial.println(m.sensorValue);
      sendData(m.sensorValue);
    }
  }
  while(Wire.available())    // slave may send less than requested
  { 
    byte r = Wire.read();
  }
  delay(500);
}
unsigned long ntpTime(){
  sendNTPpacket(timeServer); // send an NTP packet to a time server

    // wait to see if a reply is available
  delay(1000);  
  if ( Udp.parsePacket() ) {  
    // We've received a packet, read the data from it
    Udp.read(packetBuffer,NTP_PACKET_SIZE);  // read the packet into the buffer

    //the timestamp starts at byte 40 of the received packet and is four bytes,
    // or two words, long. First, esxtract the two words:

    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);  
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    unsigned long secsSince1900 = highWord << 16 | lowWord;             

    // now convert NTP time into everyday time:
    // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
    const unsigned long seventyYears = 2208988800UL;     
    // subtract seventy years:
    unsigned long epoch = secsSince1900 - seventyYears;                         
    return epoch;
  }
  return 0;

}
// send an NTP request to the time server at the given address 
void sendNTPpacket(IPAddress& address)
{
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE); 
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49; 
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp: 	
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer,NTP_PACKET_SIZE);
  Udp.endPacket();
}
