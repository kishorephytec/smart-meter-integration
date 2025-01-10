/*
 * mqtt.h
 *
 *  Created on: 08-dec-2024
 *      Author: Srikanth Jogi
 */


#ifndef MQTT_H
#define MQTT_H

#include <mosquitto.h>

// MQTT Configuration Macros
#define MQTT_HOST "192.168.11.23"
#define MQTT_PORT 1883
#define MQTT_CLIENT_ID "mqtt_client"
#define MQTT_SUB_TOPIC "gurux/to_mqtt/test"
#define CONNECTED_METERS_REQUEST "Yitrandata_cloud/plc_command"
#define CONNECTED_METERS_RESPONSE "Yitrandata_dcu/plc_command"
#define METER_SERIAL_NUMBER "meter-serial"
#define MQTT_PUB_TOPIC "gurux/to_mqtt/test"
#define MQTT_QOS 0
#define MQTT_RETAIN false

// Function prototypes
void mqtt_initialise();
void mqtt_publish(const char *message, const char *topic, int len);
void mqtt_cleanup();

void mqtt_publish_nodelist(struct mosquitto *mosq, const char *json_data);


// Callback prototypes (can be overridden by the user)
void on_connect(struct mosquitto *mosq, void *obj, int rc);
void on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg);

#endif // MQTT_H

