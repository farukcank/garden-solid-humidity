CC=gcc
pahodir=/home/cank/org.eclipse.paho.mqtt.c
libdir = $(pahodir)/build/output/lib
includedir = $(pahodir)/build/output/include
MQTTLIB_C = paho-mqtt3c
MQTTLIB_CS = paho-mqtt3cs
MQTTLIB_A = paho-mqtt3a
MQTTLIB_AS = paho-mqtt3as

all: main.c curltest.c
	$(CC) main.c -L $(libdir) -L /usr/lib/x86_64-linux-gnu/ -I $(includedir) -I /usr/include/ -lcurl -l$(MQTTLIB_C) -o hello
