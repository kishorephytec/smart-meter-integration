/*
 * mqtt.c
 *
 *  Created on: 06-Dec-2024
 *      Author: Srikanth Jogi
 */

#include "mqtt.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "timestamp.h"
#include "Plc.h"
#include "network-sync.h"
#include <sqlite3.h>
#include <json-c/json.h>

// Static global Mosquitto instance
static struct mosquitto *mosq = NULL;

void hexStringToBytes(const char *hexString, unsigned char *output, int *output_len);

/**
 * Default on_connect callback 
 */
void on_connect(struct mosquitto *mosq, void *obj, int rc)
{
    char timestamp[30];
    get_timestamp(timestamp);
    if (rc == 0) {
	printf("\n%s [INFO][MQTT CONNECTION]:Connected to MQTT broker\n",timestamp);
	mosquitto_subscribe(mosq, NULL, MQTT_SUB_TOPIC, 0); // Subscribes to topic to receive from DLMS SDK
	mosquitto_subscribe(mosq, NULL, CONNECTED_METERS_REQUEST, 0); // Subscribes to topic to receive connected meters list
	mosquitto_subscribe(mosq, NULL, METER_SERIAL_NUMBER, 0); // Subscribes to topic to receive connected meters list
    } else {
        fprintf(stderr, "Failed to connect to MQTT broker, return code: %d\n", rc);
    }
}


void on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg)
{
    char timestamp[30];
    get_timestamp(timestamp);

    // Check for the specific topic

    if (strcmp(msg->topic, "Yitrandata_cloud/plc_command") == 0) {
    printf("\n%s [INFO] Received message from topic: %s.\n", timestamp, msg->topic);

    // Parse the received JSON payload
    struct json_object *parsed_json, *plc_command, *network, *data;
    parsed_json = json_tokener_parse((const char *)msg->payload);
    if (!parsed_json) {
        printf("\n%s [INFO][ERROR    ]: Invalid JSON format received!\n", timestamp);
        return;
    }

    if (json_object_object_get_ex(parsed_json, "plc_command", &plc_command) &&
        json_object_object_get_ex(plc_command, "network", &network) &&
        json_object_object_get_ex(plc_command, "data", &data) &&
        strcmp(json_object_get_string(data), "RS list") == 0) {

        const char *network_value = json_object_get_string(network);

        // Open the database
        sqlite3 *db;
        char *err_msg = NULL;
        int rc = sqlite3_open("Network_sync_data.db", &db);

        if (rc != SQLITE_OK) {
            printf("\n%s [INFO][ERROR    ]: Cannot open database: %s\n", timestamp, sqlite3_errmsg(db));
            sqlite3_close(db);
            json_object_put(parsed_json); // Free the JSON object
            return;
        }

        // Query the database
        const char *query = "SELECT node_id, connectivity_status FROM Network_sync_info";
        sqlite3_stmt *stmt;
        rc = sqlite3_prepare_v2(db, query, -1, &stmt, NULL);

        if (rc != SQLITE_OK) {
            printf("\n%s [INFO][ERROR    ]: Failed to execute query: %s\n", timestamp, sqlite3_errmsg(db));
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            json_object_put(parsed_json); // Free the JSON object
            return;
        }

        // Create the final JSON object
        struct json_object *result_object = json_object_new_object();
        json_object_object_add(result_object, "network", json_object_new_string(network_value));

        // Create a JSON array for "RS_List"
        struct json_object *rs_list_array = json_object_new_array();

        // Iterate over the results
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const char *node_id = (const char *)sqlite3_column_text(stmt, 0);
            const char *connectivity_status = (const char *)sqlite3_column_text(stmt, 1);

            // Create a JSON object for the current row
            struct json_object *row = json_object_new_object();
            json_object_object_add(row, "node_id", json_object_new_string(node_id));
            json_object_object_add(row, "connectivity_status", json_object_new_string(connectivity_status));

            // Add the row to the "RS_List" array
            json_object_array_add(rs_list_array, row);
        }

        // Add the "RS_List" array to the final JSON object
        json_object_object_add(result_object, "RS_List", rs_list_array);

        // Print the resulting JSON
        const char *json_str = json_object_to_json_string_ext(result_object, JSON_C_TO_STRING_PRETTY);
        printf("\n%s [INFO] Node list:\n%s\n", timestamp, json_str);

        // Publish the available RS list
        mqtt_publish_nodelist(mosq, json_str);

        // Free resources
        json_object_put(result_object);
        sqlite3_finalize(stmt);
        sqlite3_close(db);
    }

    json_object_put(parsed_json); // Free the JSON object
}

    
    else if (strcmp(msg->topic, "meter-serial") == 0) {

    	printf("Received payload\n");
    	printf("%s\n", msg->payload);

    	// Parse the received JSON payload
    	struct json_object *parsed_json, *node_id_obj, *meter_serial_number_obj;
    	parsed_json = json_tokener_parse((const char *)msg->payload);
    	if (!parsed_json) {
        	printf("\n%s [INFO][ERROR    ]: Invalid JSON format received!\n", timestamp);
        	return;
    	}

    	// Extract the node_id and Meter Serial Number from the JSON payload
    	if (json_object_object_get_ex(parsed_json, "node_id", &node_id_obj) &&
            json_object_object_get_ex(parsed_json, "Meter Serial Number", &meter_serial_number_obj)) {

            const char *node_id = json_object_get_string(node_id_obj);
            const char *meter_serial_number = json_object_get_string(meter_serial_number_obj);

            printf("Node ID: %s, Meter Serial Number: %s\n", node_id, meter_serial_number);

            // Open the database
            sqlite3 *db;
            char *err_msg = NULL;
            int rc = sqlite3_open("Network_sync_data.db", &db);

            if (rc != SQLITE_OK) {
            	printf("\n%s [INFO][ERROR    ]: Cannot open database: %s\n", timestamp, sqlite3_errmsg(db));
            	sqlite3_close(db);
            	json_object_put(parsed_json); // Free the JSON object
            	return;
            }

            // Query the database to check if node_id exists
            const char *check_query = "SELECT id FROM Network_sync_info WHERE node_id = ?";
            sqlite3_stmt *stmt;
            rc = sqlite3_prepare_v2(db, check_query, -1, &stmt, NULL);

            if (rc != SQLITE_OK) {
            	printf("\n%s [INFO][ERROR    ]: Failed to prepare query: %s\n", timestamp, sqlite3_errmsg(db));
            	sqlite3_finalize(stmt);
            	sqlite3_close(db);
            	json_object_put(parsed_json); // Free the JSON object
            	return;
            }

            sqlite3_bind_text(stmt, 1, node_id, -1, SQLITE_STATIC);

            // Check if the node_id exists in the table
            if (sqlite3_step(stmt) == SQLITE_ROW) {
            	// Node ID exists, update the meter_serial_number
            	const char *update_query = "UPDATE Network_sync_info SET meter_serial_number = ? WHERE node_id = ?";
            	sqlite3_stmt *update_stmt;
            	rc = sqlite3_prepare_v2(db, update_query, -1, &update_stmt, NULL);

                if (rc != SQLITE_OK) {
                    printf("\n%s [INFO][ERROR    ]: Failed to prepare update query: %s\n", timestamp, sqlite3_errmsg(db));
                    sqlite3_finalize(update_stmt);
                    sqlite3_close(db);
                    json_object_put(parsed_json); // Free the JSON object
                    return;
                }

            	sqlite3_bind_text(update_stmt, 1, meter_serial_number, -1, SQLITE_STATIC);
            	sqlite3_bind_text(update_stmt, 2, node_id, -1, SQLITE_STATIC);

            	rc = sqlite3_step(update_stmt);
                if (rc != SQLITE_DONE) {
                    printf("\n%s [INFO][ERROR    ]: Failed to update meter serial number: %s\n", timestamp, sqlite3_errmsg(db));
            	} else {
                    printf("Meter serial number updated for node_id: %s\n", node_id);
            	}

                sqlite3_finalize(update_stmt);
            } else {
            	printf("\n%s [INFO] No matching node_id found in the database.\n", timestamp);
            }

            sqlite3_finalize(stmt);
            sqlite3_close(db);
        } else {
            printf("\n%s [INFO][ERROR    ]: Required fields not found in the payload!\n", timestamp);
        }

    	json_object_put(parsed_json); // Free the JSON object
}

    else {
        // Original task for other topics
        if (msg->payloadlen < 1) {
            printf("\n%s [INFO][ERROR    ]: Received empty message!\n", timestamp);
            return;
        }

        // Allocate and copy the payload, ensuring null-termination for safe string operations
        char *payload = (char *)malloc(msg->payloadlen + 1);
        if (payload == NULL) {
            printf("\n%s [INFO][ERROR    ]: Failed to allocate memory for payload\n", timestamp);
            return;
        }
        memcpy(payload, msg->payload, msg->payloadlen);
        payload[msg->payloadlen] = '\0';  // Null-terminate the payload

        // Find the first space
        char *space_ptr = strchr(payload, ' ');
        if (space_ptr == NULL) {
            printf("\n%s [INFO][ERROR    ]: Cannot extract system name!\n", timestamp);
            free(payload);
            return;
        }

        // Calculate the length of the system name
        int system_name_length = space_ptr - payload;
	printf("system name length: %d\n",system_name_length);

        // Extract the system name
        char *system = (char *)malloc(system_name_length + 1);
        if (system == NULL) {
            printf("\n%s [INFO][ERROR    ]: Failed to allocate memory for system name\n", timestamp);
            free(payload);
            return;
        }
        strncpy(system, payload, system_name_length);
        system[system_name_length] = '\0';  // Null-terminate the system name

        // Calculate the length of the remaining content
        int content_length = msg->payloadlen - (system_name_length + 1);
        if (content_length < 0) {
            printf("\n%s [INFO][ERROR    ]: Invalid content length\n", timestamp);
            free(system);
            free(payload);
            return;
        }

        // Allocate memory for content
        unsigned char *content = (unsigned char *)malloc(content_length);
        if (content == NULL) {
            printf("\n%s [INFO][ERROR    ]: Failed to allocate memory for content\n", timestamp);
            free(system);
            free(payload);
            return;
        }

        // Copy the content
        memcpy(content, space_ptr + 1, content_length);

        // Print the system name and remaining content in hex
        printf("Received message for system %s:\n", system);
        for (int i = 0; i < content_length; i++) {
            printf("%02X ", content[i]);
        }
        printf("\n");

        // Convert the system name from hex string to actual bytes
	if(system_name_length == 4){
            unsigned char system_bytes[system_name_length / 3];  // For each pair of hex digits
            int system_bytes_length = 0;
            hexStringToBytes(system, system_bytes, &system_bytes_length);

            // Print the system name as hex
            printf("Converted system bytes: ");
            for (int i = 0; i < system_bytes_length; i++) {
            	printf("%02X ", system_bytes[i]);
            }

	    printf("system_bytes_length : %d\n", system_bytes_length);
            printf("\n");
    	    Create_PLC_Packet_And_send(content, content_length,system_bytes, system_bytes_length);

	}
	else{


    	    Create_PLC_Packet_And_send(content, content_length,system, system_name_length);

	}


        // Free the allocated memory
        free(content);
        free(system);
        free(payload);
    }
}

//publish plc node list
void mqtt_publish_nodelist(struct mosquitto *mosq, const char *json_data) {
    if (mosq == NULL || json_data == NULL) {
        fprintf(stderr, "[ERROR] Invalid input to mqtt_publish_nodelist\n");
        return;
    }

    int ret = mosquitto_publish(mosq, NULL, CONNECTED_METERS_RESPONSE, strlen(json_data), json_data, MQTT_QOS, MQTT_RETAIN);
    if (ret != MOSQ_ERR_SUCCESS) {
        fprintf(stderr, "[ERROR] Failed to publish node list: %s\n", mosquitto_strerror(ret));
    } else {
        printf("[INFO] Successfully published node list to topic '%s'\n", CONNECTED_METERS_RESPONSE);
    }
}




// Function to convert a hex string to an array of bytes
void hexStringToBytes(const char *hexString, unsigned char *output, int *output_len) {
    size_t len = strlen(hexString);
    int index = 0;

    // Convert each pair of hex characters to a byte
    for (size_t i = 0; i < len; i += 3) {  // We have a space after each byte
        unsigned char byte;
        sscanf(hexString + i, "%2hhx", &byte);
        output[index++] = byte;
    }
    *output_len = index;  // Set the output length to the number of bytes converted
}




/**
 * Initialize MQTT connection using predefined macros.
 */
void mqtt_initialise()
{
    int rc;

    // Initialize the Mosquitto library
    mosquitto_lib_init();

    // Create a new Mosquitto client instance
    mosq = mosquitto_new(MQTT_CLIENT_ID, true, NULL);
    if (!mosq) {
        fprintf(stderr, "Error: Out of memory.\n");
        return;
    }

    // Set callback functions
    mosquitto_connect_callback_set(mosq, on_connect);
    mosquitto_message_callback_set(mosq, on_message);

    // Connect to the MQTT broker
    rc = mosquitto_connect(mosq, MQTT_HOST, MQTT_PORT, 60);
    if (rc != MOSQ_ERR_SUCCESS) {
        fprintf(stderr, "Unable to connect to MQTT broker: %s\n",
                mosquitto_strerror(rc));
        return;
    }

    // Start the Mosquitto network loop in a new thread
    mosquitto_loop_start(mosq);
}

/**
 * Publish a message to the predefined MQTT topic.
 * 
 * @param message Message payload.
 */
void mqtt_publish(const char *message, const char *topic, int len)
{
    int rc;

    if (!mosq) {
        fprintf(stderr, "MQTT client not initialized.\n");
        return;
    }
    for(int i=0; i<len;i++)
    {
	    printf("%02X ",message[i]);
    }
    printf("\n");
    rc = mosquitto_publish(mosq, NULL, topic, len, message, MQTT_QOS, MQTT_RETAIN);
    if (rc != MOSQ_ERR_SUCCESS) {
        fprintf(stderr, "Failed to publish message: %s\n", mosquitto_strerror(rc));
    }
}

/**
 * Cleanup MQTT resources.
 */
void mqtt_cleanup()
{
    if (mosq) {
        mosquitto_loop_stop(mosq, true); // Stop the network loop
        mosquitto_disconnect(mosq);     // Disconnect from the broker
        mosquitto_destroy(mosq);        // Destroy the Mosquitto instance
    }

    mosquitto_lib_cleanup(); // Cleanup the Mosquitto library
}

