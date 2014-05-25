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


#define ADDRESS     	"tcp://95.85.36.115:1883"
#define CLIENTID    	"gardenSoilHumidityServer"
#define PUBLISH_TOPIC   "gardenSoilHumidityMeasure"
#define SUBSCRIBE_TOPIC "gardenSoilHumidityMeasurementResult"
#define QOS         1
#define TIMEOUT     10000L
#define INTERVAL 	20

volatile int connected = 0;
volatile MQTTClient_deliveryToken deliveredtoken;

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
			printf("%s\n", curl_easy_strerror(curlErr));
		}
		curl_global_cleanup();
	}
}

void delivered(void *context, MQTTClient_deliveryToken dt)
{
    printf("Message with token value %d delivery confirmed\n", dt);
    deliveredtoken = dt;
}

int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{
    int i;
    char* payloadptr;

    printf("Message arrived\n");
    printf("     topic: %s\n", topicName);
    printf("   message: ");	
    payloadptr = message->payload;
    for(i=0; i<message->payloadlen; i++)
    {
        putchar(*payloadptr++);
    }
    putchar('\n');
    char* data = malloc(message->payloadlen+1);
	memcpy(data, message->payload, message->payloadlen);
	data[message->payloadlen] = 0;    
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    sendData(data);
    free(data);
    printf("\n");
    return 1;
}

void connlost(void *context, char *cause)
{
    printf("\nConnection lost\n");
    printf("     cause: %s\n", cause);
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
        printf("Failed to connect, return code %d\n", rc);        
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
        printf("Waiting for publication of %s\n"
                "on topic %s for client with ClientID: %s\n",
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
	while(1){
		connectToServer();
	}
}
