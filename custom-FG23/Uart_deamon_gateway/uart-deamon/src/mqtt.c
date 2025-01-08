/*
 * mqtt.c
 *
 *  Created on: 23-Aug-2024
 *      Author: madhulika
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <mosquitto.h>
#include <cjson/cJSON.h>
#include <mqtt.h>
#include <common.h>
#include <timestamp.h>
#include <system_ip.h>


//Global and extern variables
struct mosquitto *mosq = NULL; // Declare the global MQTT client instance
extern IpPacket packet;
extern SystemIP *head;


//Declaration of functions
void on_connect(struct mosquitto *mosq, void *userdata, int rc);
void on_message(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message);
void mqtt_initialise();
void mqtt_publish(char *buf,int len,char *ip);

// Utility function to convert a hex string to bytes
int hex_string_to_bytes(const char *hex_str, uint8_t **bytes) {
    size_t len = strlen(hex_str);
    if (len % 2 != 0) {
        return -1;  // Invalid hex string length
    }

    size_t byte_len = len / 2;
    *bytes = (uint8_t *)malloc(byte_len);
    if (*bytes == NULL) {
        return -2;  // Memory allocation failure
    }

    for (size_t i = 0; i < byte_len; i++) {
        sscanf(hex_str + 2 * i, "%2hhx", &(*bytes)[i]);
    }

    return byte_len;  // Return the length of the byte array
}

char* string_to_hex(const char* input, int length) {
    // Allocate memory for the hex string, each byte will need 2 characters + 1 for the null terminator
    char* hex_representation = (char*)malloc(2 * length + 1);
    if (hex_representation == NULL) {
        // Memory allocation failed
        return NULL;
    }

    // Loop through each byte of the input string and convert it to hex
    int hex_index = 0;
    for (int i = 0; i < length; i++) {
        // Format each byte as a 2-character hex value
        hex_index += sprintf(&hex_representation[hex_index], "%02x", (unsigned char)input[i]);
    }

    // Return the hex representation
    return hex_representation;
}

void polling_func(void* args) {
    char timestamp[30];
    get_timestamp(timestamp);

    while (1) {
        int system_count = 0;
        char* system_list = print_system_table(&system_count);
        if (system_list == NULL) {
            printf("%s [INFO] Error: Failed to generate system list.\n", timestamp);
            return;
        }

        // Construct the data string including the system list and the count of routes
        char data[2048];  // Adjust size as needed
        snprintf(data, sizeof(data), "rt list %s Listed %d routes", system_list, system_count);

        // Respond to "rt list"
        cJSON* response_message = cJSON_CreateObject();
        cJSON_AddNumberToObject(response_message, "network", 1);
        cJSON_AddStringToObject(response_message, "data", data);

        // Publish the response to PUBLISH_TOPIC
        char* response_payload = cJSON_PrintUnformatted(response_message);
        mosquitto_publish(mosq, NULL, DCU_PUB_TOPIC, strlen(response_payload), response_payload, 0, false);
        printf("%s [INFO] Published response: %s\n", timestamp, response_payload);

        cJSON_Delete(response_message);
        free(response_payload);
        free(system_list);
        sleep(900);  // Wait for 15 minutes
    }
    return NULL;
}

/******************************MQTT*****************************************/

void on_connect(struct mosquitto *mosq, void *userdata, int rc) {
	
	//timestamp
	char timestamp[30];
	get_timestamp(timestamp);
	
	if (rc == 0) {
		printf("\n%s [INFO][MQTT CONNECTION]:Connected to MQTT broker\n",timestamp);
		// Subscribes to topic
		mosquitto_subscribe(mosq, NULL, MQTT_SUB_TOPIC, 0);
        mosquitto_subscribe(mosq, NULL, DCU_SUB_TOPIC, 0);
	} else {
		printf("\n%s [ERROR][MQTT CONNECTION]:Failed to connect to MQTT broker: %s\n",timestamp,mosquitto_connack_string(rc));
	}
}

void mqtt_initialise() {
	int rc;

	mosquitto_lib_init(); // Initialize libmosquitto

	mosq = mosquitto_new(NULL, true, NULL);
	if (!mosq) {
		fprintf(stderr, "Error: Out of memory.\n");
		return;
	}

	mosquitto_connect_callback_set(mosq, on_connect);
	mosquitto_message_callback_set(mosq, on_message);

	rc = mosquitto_connect(mosq, MQTT_HOST, MQTT_PORT, 60);
	if (rc != MOSQ_ERR_SUCCESS) {
		fprintf(stderr, "Unable to connect to MQTT broker: %s\n",
				mosquitto_strerror(rc));
	}

	mosquitto_loop_start(mosq);

    //polling rt_list every minutes
    #if 1
	pthread_t polling_thread;
	if(pthread_create(&polling_thread,NULL,polling_func,NULL)){
		fprintf(stderr," [INFO][ERROR                  ]: Failed to craete polling thread\n\r");
	}
    #endif
}

void mqtt_publish(char *buf,int len,char *ip){
	
	//timestamp
	char timestamp[30];
	get_timestamp(timestamp);

	SystemIP *current = head;
	char topic[100];
    char publish_topic[128];

    cJSON *response_message = cJSON_CreateObject();

    while (current != NULL) {
		if (memcmp(current->ip, ip,16) == 0) {
            snprintf(publish_topic, sizeof(publish_topic), "%s%s", DCU_PUB_TOPIC, current->system);
	    
       char* hex_string = string_to_hex(buf,len);            

            cJSON_AddStringToObject(response_message, "Macid", current->system);
            cJSON_AddStringToObject(response_message, "data",hex_string);
            char *response_payload = cJSON_PrintUnformatted(response_message);

            mosquitto_publish(mosq, NULL, publish_topic,strlen(response_payload), response_payload, 0, false);
            printf("\n%s [INFO][MQTT PUBLISH ]:Published to topic %s\n",publish_topic,timestamp);
            return;
        }
        current = current->next;
    }

    printf("\n%s [WARNING][MQTT PUBLISH ]:No matching system found for system name: %s\n", timestamp,topic);
}

void publish_response(struct mosquitto *mosq, const char *data) {
    
    char timestamp[30];
    get_timestamp(timestamp);

    cJSON *response_message = cJSON_CreateObject();
    cJSON_AddNumberToObject(response_message, "network", 1);
    cJSON_AddStringToObject(response_message, "data", data);

    char publish_topic[128];
    snprintf(publish_topic, sizeof(publish_topic), "%s%s", DCU_PUB_TOPIC, "rf_command");

    char *response_payload = cJSON_PrintUnformatted(response_message);
    mosquitto_publish(mosq, NULL, publish_topic, strlen(response_payload), response_payload, 0, false);
    printf("%s [INFO] Published response: %s\n", timestamp, response_payload);

    cJSON_Delete(response_message);
    free(response_payload);
}

static void handle_dcu_sub_topic_message(struct mosquitto *mosq, const struct mosquitto_message *msg) {
    // Get current timestamp
    char timestamp[30];
    get_timestamp(timestamp);

    // Parse the received JSON message
    char *payload = (char *)malloc(msg->payloadlen + 1);
    if (payload == NULL) {
        printf("%s [INFO][ERROR]: Failed to allocate memory for payload\n", timestamp);
        return;
    }
    memcpy(payload, msg->payload, msg->payloadlen);
    payload[msg->payloadlen] = '\0';

    // Parse the JSON message
    cJSON *message = cJSON_Parse(payload);
    if (message == NULL) {
        printf("%s [INFO][ERROR]: Failed to parse JSON\n", timestamp);
        free(payload);
        return;
    }

	printf("%s\n",__FUNCTION__);
    // Log the parsed JSON for debugging
    char *json_str = cJSON_PrintUnformatted(message);
    printf("%s [INFO] Parsed JSON: %s\n", timestamp, json_str);
    free(json_str);

    // Check if "rf_command" and "data" fields are present
    cJSON *rf_command = cJSON_GetObjectItem(message, "rf_command");
    if (rf_command == NULL) {
        printf("%s [INFO][ERROR]: rf_command field is missing\n", timestamp);
        free(payload);
        cJSON_Delete(message);
        return;
    }

    cJSON *data = cJSON_GetObjectItem(rf_command, "data");
    if (data == NULL) {
        printf("%s [INFO][ERROR]: data field is missing\n", timestamp);
        free(payload);
        cJSON_Delete(message);
        return;
    }

    const char *command_data = data->valuestring;

    // Handle different command data types
    if (strcmp(command_data, "rt list") == 0) {
        int system_count = 0;
        char *system_list = print_system_table(&system_count);
        if (system_list == NULL) {
            printf("%s [INFO] Error: Failed to generate system list.\n", timestamp);
            return;
        }

        // Construct the data string including the system list and the count of routes
        char data[2048];  // Adjust size as needed
        snprintf(data, sizeof(data), "rt list %s Listed %d routes", system_list, system_count);

        publish_response(mosq,data);
        free(system_list);
    }
    else if (strcmp(command_data, "show routes") == 0) {

        int system_count = 0;
        char *system_list = print_system_table(&system_count);
        if (system_list == NULL) {
            printf("%s [INFO] Error: Failed to generate system list.\n", timestamp);
            return;
        }

        // Construct the data string including the system list and the count of routes
        char data[2048];  // Adjust size as needed
        snprintf(data, sizeof(data), "show routes Routes - %d graph network_topology { %s }", system_count, system_list);
        
        publish_response(mosq,data);
        free(system_list);
    }
    else if (strcmp(command_data, "env") == 0) {
        // Respond to "env"
        char data_buffer[BUFFER_SIZE] = {0}; // Buffer to hold fetched details

        // Fetch and store system and network details in the buffer
        fetch_system_info(data_buffer, sizeof(data_buffer));
        fetch_network_info(data_buffer, sizeof(data_buffer), INTERFACE);
        
        publish_response(mosq,data);
     }
    else {
        printf("%s [INFO][ERROR]: Unknown command: %s\n", timestamp, command_data);
    }

    // Free dynamically allocated memory
    free(payload);
    cJSON_Delete(message);
}

static void handle_generic_topic_message(struct mosquitto *mosq, const struct mosquitto_message *msg){
  // Get current timestamp
    char timestamp[30];
    get_timestamp(timestamp);

    // Parse the received JSON message
    char *payload = (char *)malloc(msg->payloadlen + 1);
    if (payload == NULL) {
        printf("%s [INFO][ERROR]: Failed to allocate memory for payload\n", timestamp);
        return;
    }
    memcpy(payload, msg->payload, msg->payloadlen);
    payload[msg->payloadlen] = '\0';

    // Parse the JSON message
    cJSON *message = cJSON_Parse(payload);
    if (message == NULL) {
        printf("%s [INFO][ERROR]: Failed to parse JSON\n", timestamp);
        free(payload);
        return;
    }

    // Check if "rf_command" and "data" fields are present
    cJSON *rf_dlms = cJSON_GetObjectItem(message, "rf_dlms");
    if (rf_dlms == NULL) {
        printf("%s [INFO][ERROR]: rf_dlms field is missing\n", timestamp);
        free(payload);
        cJSON_Delete(message);
        return;
    }

    cJSON *macid = cJSON_GetObjectItem(rf_dlms, "Macid");
    if (macid == NULL) {
        printf("%s [INFO][ERROR]: data field is missing\n", timestamp);
        free(payload);
        cJSON_Delete(message);
        return;
    }

    cJSON *data = cJSON_GetObjectItem(rf_dlms, "data");
    if (data == NULL) {
        printf("%s [INFO][ERROR]: data field is missing\n", timestamp);
        free(payload);
        cJSON_Delete(message);
        return;
    }  

   /* const char *dlms_data = data->valuestring;
    int data_length = strlen(data->valuestring);
    printf("%s [INFO]: Data length is %d\n", timestamp, data_length);*/
const char *hex_data = data->valuestring;
    printf("%s [INFO]: Received hex data: %s\n", timestamp, hex_data);

    // Convert the hex string to a byte array
    uint8_t *byte_data = NULL;
    int data_length = hex_string_to_bytes(hex_data, &byte_data);
    if (data_length < 0) {
        printf("%s [INFO][ERROR]: Failed to convert hex string to bytes\n", timestamp);
        cJSON_Delete(message);
        return;
    }
    
    SystemIP *current = head;
    while (current != NULL) {
        if (strcmp(current->system, macid->valuestring) == 0) {
            // Do something with the matching system IP (you can add logic for your processing)
            memcpy(packet.dst_address, &current->ip, 16);
            Create_HIF_Packet_And_send(byte_data, data_length, 0x48, 0x02);

            // Free memory after processing
            free(payload);
            cJSON_Delete(message);
            return;
        }
        current = current->next;
}
}

// Modified on_message callback
void on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg) {
    char timestamp[30];
    get_timestamp(timestamp);

    if (strncmp(msg->topic, "RB_DCU/Ineshdata_cloud/", 23) == 0) {
        const char *sub_topic = msg->topic + 23;  // Extract the sub-topic part
	printf("%s\n",sub_topic);
        if (strcmp(sub_topic, "rf_command") == 0) {
            printf("%s [INFO] Received message from DCU_SUB_TOPIC\n", timestamp);
            handle_dcu_sub_topic_message(mosq, msg);
        } else {
            handle_generic_topic_message(mosq, msg);
        }
    } else {
        printf("%s [INFO][ERROR]: Received message on an unexpected topic: %s\n", timestamp, msg->topic);
    }
}


