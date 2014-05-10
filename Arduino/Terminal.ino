#include <SPI.h>
#include <Mirf.h>
#include <nRF24L01.h>
#include <MirfHardwareSpiDriver.h>


const uint8_t ESCAPE = 0x7D;
const uint8_t ESCAPE1 = 0x7E;
const uint8_t ESCAPE_S[] = {ESCAPE, ESCAPE1};
const uint8_t START = 0xAA;
const uint8_t END = 0xBB;
const uint8_t START_S[] = {ESCAPE, START};
const uint8_t END_S[] = {ESCAPE, END};

//#define SERIAL_SEND_ARRAY(arr, len) Serial.write(arr, len)
//#define SERIAL_SEND_BYTE(b) Serial.write((uint8_t)b)
//#define DEBUG_OUT(x)
#define SERIAL_SEND_ARRAY(arr, len)
#define SERIAL_SEND_BYTE(b)
#define DEBUG_OUT(x) Serial.println(x)

unsigned int writeRaw(const uint8_t* data, unsigned int len, unsigned int crc)
{  
  for(int i = 0;i<len;i++)
  {
    uint8_t d = data[i];
    crc += (d & 0xFF);
    crc = crc & 0xFF;
    if (d == ESCAPE)
      SERIAL_SEND_ARRAY(ESCAPE_S, 2);
    else
      SERIAL_SEND_BYTE(d);
  }
  return crc;
}
void writeStart()
{
  SERIAL_SEND_ARRAY(START_S, 2);
}
void writeEnd()
{
  SERIAL_SEND_ARRAY(END_S, 2);
}
void sendFrame(const uint8_t* data, unsigned int len)
{
  writeStart();
  int crc = writeRaw(data, len, 0);
  SERIAL_SEND_BYTE(crc);
  writeEnd();
}

void setup(){
  Serial.begin(9600);
  Mirf.spi = &MirfHardwareSpi;
  Mirf.init();
  Mirf.setRADDR((byte *)"serv1");
  Mirf.payload = sizeof(int);
  Mirf.config();
  Mirf.configRegister(SETUP_RETR, 0xFF);
  Mirf.configRegister(RF_SETUP, 0x06);  
}

void loop(){
  byte data[Mirf.payload];
  uint8_t payload[sizeof(int)*4];
  int zero = 0;
  for(int i = 0;i<4;i++)
  {
      memcpy(&payload[i*sizeof(int)], &zero, sizeof(int));
  }
  if(!Mirf.isSending() && Mirf.dataReady()){
    int sensorValue;    
    char clientAddress[6];
    clientAddress[5] = 0;
    unsigned long time;
    Mirf.getData(data);
    memcpy((byte *)&sensorValue, data, sizeof(int));
    DEBUG_OUT("Sensor Value:");
    DEBUG_OUT(sensorValue);
    memcpy(&payload[0], &sensorValue, sizeof(int));
    sendFrame(payload, sizeof(int)*4);
  }
}