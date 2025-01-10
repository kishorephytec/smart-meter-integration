/*
 * Plc.c
 *
 *  Created on: 09-Dec-2024
 *      Author: Srikanth Jogi
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <common.h>
#include <time.h>
#include <timestamp.h>
#include "mqtt.h"
#include "Plc.h"
#include "database.h"

enum state uart_read_state = PKT_ADM;

unsigned short global_system_len = 0; // Global variable to store system_len
unsigned char global_system[256]; // Adjust the size based on your requirements


int process_uart_byte(unsigned char rx_char, char *timestamp) {
    static unsigned char uart_rx_buffer[2048] = {'\0'}; // Buffer size for UART data
    static int len = 0;               // Current buffer length
    static int pkt_len_temp = 0;      // Temporary length during packet processing
    static int pkt_len = 0;

    switch (uart_read_state) {
        case PKT_ADM:
            // Fill buffer only in this state
            uart_rx_buffer[len++] = rx_char;

            // Process packet admission (first 11 bytes)
            if (len == 11) {
                if (uart_rx_buffer[1] == 0x07 && uart_rx_buffer[4] == 0x60) {
                    printf("%s [INFO][Response1]: for packet admission.\n", timestamp);
                    for (int i = 0; i < 11; i++) {
                        printf("%02X ", uart_rx_buffer[i]);
                    }
                    printf("\n");

                    uart_read_state = PKT_TX;  // Transition to the next state
                    len = 0;  // Reset buffer for the next packet
                } else {
                    // Reset if conditions are not met
		    len = 0;
		    memset(uart_rx_buffer,'\0',sizeof(uart_rx_buffer));
                    uart_read_state = PKT_ADM;

                }
            }
            break;

        case PKT_TX:
            // Fill buffer only in this state
            uart_rx_buffer[len++] = rx_char;

            // Process packet transmission (13 bytes)
            if (len == 13) {
                if (uart_rx_buffer[1] == 0x09 && uart_rx_buffer[4] == 0x60) {
                    printf("%s [INFO][Response2]: for packet transmission.\n", timestamp);
                    for (int i = 0; i < 13; i++) {
                        printf("%02X ", uart_rx_buffer[i]);
                    }
                    printf("\n");

                    uart_read_state = RX_DATA;  // Transition to the next state
                    len = 0;  // Reset buffer for the next packet
                } else {
                    // Reset if conditions are not met
		    memset(uart_rx_buffer,'\0',sizeof(uart_rx_buffer));
                    uart_read_state = PKT_ADM;

                    len = 0;
                }
            }
            break;

        case RX_DATA:
            // Fill buffer only in this state
            uart_rx_buffer[len++] = rx_char;
            pkt_len_temp++;
	    pkt_len = (uart_rx_buffer[2] << 8) | uart_rx_buffer[1];

            // Process data once expected packet length is received
            if (pkt_len_temp == pkt_len + 4) { // +4 is for Header,length 2 bytes and checksum. 
                printf("%s [INFO][Response from meter]: \n", timestamp);
                for (int i = 0; i < pkt_len_temp; i++) {
                    printf("%02X ", uart_rx_buffer[i]);
                }
                printf("\n");
                
                if(global_system_len == 2){ 
			char topic[5]; // 2 bytes * 2 characters per byte + 1 null terminator
			for (int i = 0; i < 2; i++) {
    				snprintf(&topic[i * 2], 3, "%02X", uart_rx_buffer[16 + i]);
			}
                	// Print the topic
                	printf("Topic: %s\n", topic);

			int data_len = pkt_len_temp - 26 - 1; // Length of data excluding the last byte (check)
                	printf("length is %d\n", data_len);

                	// Publish the data
                	mqtt_publish(&uart_rx_buffer[26], topic, data_len);

                	// Reset after processing the packet
                	len = 0;
                	pkt_len_temp = 0;
                	pkt_len = 0;
                	uart_read_state = PKT_ADM; // Transition back to the starting state
                	memset(uart_rx_buffer,'\0',sizeof(uart_rx_buffer));

		}
		if (global_system_len > 2) {
    			printf("Came Inside of meter-id fetching condition\n");

    			char *meter_id = fetch_meter_id(global_system, global_system_len);
    			if (!meter_id) {
        			fprintf(stderr, "[ERROR]: Meter ID not found for the provided system. Exiting condition.\n");

        			// Handle the absence of meter_id, for example, skip publishing
        			len = 0;
        			pkt_len_temp = 0;
        			pkt_len = 0;
        			uart_read_state = PKT_ADM; // Transition back to the starting state
        			memset(uart_rx_buffer, '\0', sizeof(uart_rx_buffer));
        			return -1; // Or any appropriate error handling
    			}		

    			printf("[INFO][METER ID]: %s \n", meter_id);

    			int data_len = pkt_len_temp - 26 - 1; // Length of data excluding the last byte (check)
    			printf("length is %d\n", data_len);

    			// Publish the data
    			mqtt_publish(&uart_rx_buffer[26], meter_id, data_len);

    			// Free the allocated memory for meter_id
    			free(meter_id);

    			// Reset after processing the packet
    			len = 0;
    			pkt_len_temp = 0;
    			pkt_len = 0;
    			uart_read_state = PKT_ADM; // Transition back to the starting state
    			memset(uart_rx_buffer, '\0', sizeof(uart_rx_buffer));
		}

            }
            break;

        default:
            // Reset state and buffer on unexpected state
            len = 0;
            pkt_len_temp = 0;
            uart_read_state = PKT_ADM;
            memset(uart_rx_buffer, 0, sizeof(uart_rx_buffer));
            break;
    }
    return 0;
}



// Function to calculate checksum (sum of all bytes except the first byte)
unsigned char Generate_Checksum_payload(unsigned char* data, size_t length) {
    unsigned int sum = 0;

    printf("Received Data for calculating checksum \n");
    for(int i =0; i<length; i++)
    {
	printf("%02X ", data[i]);

    }

    printf("\r\n");    

    // Start from the second byte (skip the first byte, which is 0xCA)
    for (size_t i = 0; i < length; ++i) {
        sum += data[i];
    }

    // Take the sum modulo 256
    return (unsigned char)(sum % 256);

}


// Convert a hexadecimal string to raw bytes
void hex_string_to_bytes(const char *hex_string, unsigned char *byte_array, size_t byte_array_length) {
    for (size_t i = 0; i < byte_array_length; ++i) {
        sscanf(&hex_string[i * 2], "%2hhx", &byte_array[i]);
    }
}



void Create_PLC_Packet_And_send(unsigned char* data, unsigned short  len, unsigned char* system, unsigned short system_len)
{

	char timestamp[30];
        get_timestamp(timestamp);
	global_system_len = system_len; // Store system_len in the global variable
        int res;
        int ii;
        unsigned char *orig_dataPtr = NULL ;
        unsigned char *dataPtr = NULL ;
        unsigned short total_buff_len = (len+ 16 + 1 ) ; // +1 is for Payload Check sum 
         
	strncpy(global_system, system, system_len);	

	orig_dataPtr= (unsigned char *)malloc(total_buff_len + 1);

	if (!orig_dataPtr) {
        	printf("%s [ERROR]: Memory allocation failed!\n", timestamp);
        	return;
    	}
        printf ("%s [INFO][DATA Receiving           ]: at PLC Function [%d]: ",timestamp, len);
        for(int i =0; i<len;i++)
        {

                printf("%02X ",data[i]);
        }

        printf("\n");

	printf ("%s [INFO][System ID Receiving           ]: at PLC Function [%d]: ",timestamp, system_len);
        for(int i =0; i<system_len;i++)
        {

                printf("%02X ",system[i]);
        }

        printf("\n");



        if (orig_dataPtr == NULL)
        {
                exit(EXIT_FAILURE);
        }

	memset (orig_dataPtr , 0x00, total_buff_len);
        dataPtr= orig_dataPtr;

	*dataPtr++ = 0xCA;
	*(unsigned short*)dataPtr = len + 2 + 11;
	dataPtr += 2;
	*dataPtr++ = 0x00;
	*dataPtr++ = 0x60;
	*dataPtr++ = 0x01;
	*dataPtr++ = 0x00;
	*dataPtr++ = 0x01;
	*dataPtr++ = 0x08;
	*dataPtr++ = 0x07;
	*dataPtr++ = 0x01;
	*dataPtr++ = 0x00;
	*dataPtr++ = 0x00;
	*dataPtr++ = 0x02;
	// Now copy the Modem serial number dynamically into the buffer
	// Fetch the modem serial number
	
	/*char *serial_number = fetch_modem_number();
	printf("Modem Serial Number: %s\n", serial_number);

	if (serial_number) {
    		size_t serial_number_length = strlen(serial_number);
    		if (serial_number_length == 32) { // Ensure the serial number is valid
        		unsigned char serial_bytes[16]; // 16 bytes for the raw representation
        		hex_string_to_bytes(serial_number, serial_bytes, 16);

        		// Copy the raw bytes into the buffer
        		memcpy(dataPtr, serial_bytes, 16);
        		dataPtr += 16;
    		} else {
        		fprintf(stderr, "[ERROR]: Invalid modem serial number length: %zu\n", serial_number_length);
    		}

    		// Free the allocated memory for the serial number
    		free(serial_number);
	} else {
    		fprintf(stderr, "[ERROR]: Failed to fetch modem serial number.\n");
	}*/


	char *node_id = fetch_node_id(system, system_len);
	printf("Node ID: %s\n", node_id);

	if (node_id) {
    		size_t node_id_length = strlen(node_id);
    		if (node_id_length == 4) { // Ensure the serial number is valid
        		unsigned char serial_bytes[2]; // 2 bytes for the raw representation
        		hex_string_to_bytes(node_id, serial_bytes, 2);

        		// Copy the raw bytes into the buffer
        		memcpy(dataPtr, serial_bytes, 2);
        		dataPtr += 2;
    		} else {
        		fprintf(stderr, "[ERROR]: Invalid Node ID number length: %zu\n", node_id_length);
    		}

    		// Free the allocated memory for the serial number
    		free(node_id);
	} else {
    		fprintf(stderr, "[ERROR]: Failed to fetch Node ID.\n");
	}
	
	memcpy(dataPtr, data, len);
	dataPtr +=len;
	
	unsigned char *checksum_start = orig_dataPtr + 1; // Start after the first byte (0xCA)
	*dataPtr++ = Generate_Checksum_payload(checksum_start, total_buff_len - 2);


	printf ("%s [INFO][DATA COUNT           ]: SENDING PLC packet [%d]: ",timestamp, total_buff_len);
     	dataPtr = orig_dataPtr;
    	for (ii = 0; ii < total_buff_len; ii++){

        	printf ("%02X ", dataPtr[ii]);

	}
    	printf ("\r\n");

        printf("%s [INFO][UART Communication   ]: UART Deamon Sending Data to NC Board.\n",timestamp);
        res = write_2_Uart(dataPtr, total_buff_len);

        free (orig_dataPtr);


}
