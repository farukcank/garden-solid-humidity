/*******************************************************************************
 * The MIT License (MIT)
 * 
 * Copyright (c) 2014 Faruk Can KAYA
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "MQTTClient.h"
#include <time.h>
#include <curl/curl.h>
#include <log4c.h>

#define ADDRESS     	"tcp://95.85.36.115:1883"
#define CLIENTID    	"gardenSoilHumidityServer"
#define PUBLISH_TOPIC   "gardenSoilHumidityMeasure"
#define SUBSCRIBE_TOPIC "gardenSoilHumidityMeasurementResult"
#define QOS         1
#define TIMEOUT     10000L
#define INTERVAL 	60

volatile int connected = 0;
volatile MQTTClient_deliveryToken deliveredtoken;
log4c_category_t* mycat = NULL;
void sendData(char* data)
{
	char url[2048];
	sprintf(url, "http://95.85.36.115/garden-soil-humidity/insert.php?value=%s", data);

	curl_global_init(CURL_GLOBAL_ALL);
	char* curlErrStr = (char*)malloc(CURL_ERROR_SIZE);
	CURL* curlHandle = curl_easy_init(); 
	if(curlHandle) {
		curl_easy_setopt(curlHandle, CURLOPT_ERRORBUFFER, curlErrStr);
		curl_easy_setopt(curlHandle, CURLOPT_URL, url);
		CURLcode curlErr = curl_easy_perform(curlHandle);
		if(curlErr) {
			log4c_category_log(mycat, LOG4C_PRIORITY_ERROR, "curl error %s", curl_easy_strerror(curlErr));
		}
		curl_global_cleanup();
	}
}

void delivered(void *context, MQTTClient_deliveryToken dt)
{
    log4c_category_log(mycat, LOG4C_PRIORITY_NOTICE, "Message with token value %d delivery confirmed", dt);
    deliveredtoken = dt;
}

int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{
    int i;
    char* data = malloc(message->payloadlen+1);
	memcpy(data, message->payload, message->payloadlen);
	data[message->payloadlen] = 0;    
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    log4c_category_log(mycat, LOG4C_PRIORITY_NOTICE, "Message arrived topic: %s message: %s",topicName,data);
    sendData(data);
    free(data);
    return 1;
}

void connlost(void *context, char *cause)
{
	log4c_category_log(mycat, LOG4C_PRIORITY_ERROR, "Connection lost cause: %s", cause);
    connected=0;
}

int connectToServer()
{
    MQTTClient client;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    MQTTClient_deliveryToken token;
    int rc;

    MQTTClient_create(&client, ADDRESS, CLIENTID,
        MQTTCLIENT_PERSISTENCE_NONE, NULL);
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;

    MQTTClient_setCallbacks(client, NULL, connlost, msgarrvd, delivered);

    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
    {
        log4c_category_log(mycat, LOG4C_PRIORITY_ERROR, "Failed to connect, return code %d", rc);        
    }
    else{
    MQTTClient_subscribe(client, SUBSCRIBE_TOPIC, QOS);
    connected = 1;
    char buffer[1024];
    while(1){
        time_t rawTime;
        time(&rawTime);
        time_t waitDuration = INTERVAL - (rawTime % INTERVAL);
        time_t targetTime = rawTime + waitDuration;
        sleep(waitDuration);
        sprintf(buffer, "%zu", targetTime);
        pubmsg.payload = buffer;
        pubmsg.payloadlen = strlen(buffer);
        pubmsg.qos = QOS;
        pubmsg.retained = 0;
        deliveredtoken = 0;
        MQTTClient_publishMessage(client, PUBLISH_TOPIC, &pubmsg, &token);
        log4c_category_log(mycat, LOG4C_PRIORITY_NOTICE, "Waiting for publication of %s on topic %s for client with ClientID: %s",
                buffer, PUBLISH_TOPIC, CLIENTID);
        while(deliveredtoken != token);
    }
    MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);
    }
    return rc;
}

int main(int argc, char* argv[])
{
	if (log4c_init()){
   		printf("log4c_init() failed\n");
		exit(-1);
  	}
	mycat = log4c_category_get("com.eclipseuzmani.gardensoilhumidity");
	log4c_category_log(mycat, LOG4C_PRIORITY_NOTICE, "Starting...");
	while(1){
		connectToServer();
	}
}
