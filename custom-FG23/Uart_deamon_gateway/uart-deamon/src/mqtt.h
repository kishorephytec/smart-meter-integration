/*
 * mqtt.h
 *
 *  Created on: 23-Aug-2024
 *      Author: madhulika
 */

#ifndef MQTT_H_
#define MQTT_H_

#define MQTT_SUB_TOPIC "gurux/to_mqtt"
#define MQTT_PUB_TOPIC "mqtt/to_gurux"

#define DCU_SUB_TOPIC "RB_DCU/Ineshdata_cloud/#"
#define DCU_PUB_TOPIC "RB_DCU/Ineshdata_dcu/"

#define MQTT_HOST "118.91.232.233"   // MQTT broker address
//#define MQTT_HOST "mqtt.eclipseprojects.io"
#define MQTT_PORT 1883

#define TEST 0
#define METER 1

// Constants
#define BUFFER_SIZE 2048
#define INTERFACE "eth0" // Replace with your network interface

extern struct mosquitto *mosq; // Global MQTT client instance

void on_connect(struct mosquitto *mosq, void *userdata, int rc);
void on_message(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message);
void mqtt_initialise();
#if TEST
void mqtt_publish(char *buf,int len);
#endif

#if METER
void mqtt_publish(char *buf,int len,char *ip);
#endif

#endif /* MQTT_H_ */
