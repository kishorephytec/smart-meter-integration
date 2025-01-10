/*
 * network-sync.c
 *
 *  Created on: 17-Dec-2024
 *      Author: Srikanth Jogi
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <common.h>
#include <time.h>
#include <timestamp.h>
#include "database.h"
#include "network-sync.h"
#include "mqtt.h"

enum Network_Sync_Cases network_sync_state = NC_DATABASE_ENTRY;
enum Network_Sync_Response network_response_state = INIT;

// Declare globally
unsigned char lsb = 0;
unsigned char msb = 0;
int node_info_flag = 0;

unsigned short global_system_len = 0; // Global variable to store system_len
unsigned char global_system[256]; // Adjust the size based on your requirements



int network_sync_NC() {
    char timestamp[30];
    get_timestamp(timestamp);

    printf("[DEBUG]: Entering network_sync_NC(). Current state = %d\n", network_sync_state);
    sleep(1);

    int res;

    while (1) {
        switch (network_sync_state) {
            case NC_DATABASE_ENTRY: {
                unsigned char command[] = {0xCA, 0x03, 0x00, 0x00, 0x65, 0x01, 0x69};

                printf("%s [INFO][NC-NETWORK-DATABASE]: Sending NC Data base Entry size Command\n", timestamp);
                res = write_2_Uart(command, sizeof(command));
                if (res != EXIT_SUCCESS) {
                    printf("%s [ERROR][NC-NETWORK-DATABASE]: Failed to write to UART\n", timestamp);
                    return res;
                }

                sleep(2); // Ensure sufficient time for response
            	network_sync_state = GET_RS_INFO;
                break;
            }

            case GET_RS_INFO: {
                printf("%s [INFO][NC-NETWORK-DATABASE]: Starting GET_RS_INFO Command Sequence\n", timestamp);
		printf("%s [INFO][NC-DATABASE-ENTRY]: LSB = %02X, MSB = %02X\n", timestamp, lsb, msb);

                for (unsigned short i = 0; i < ((msb << 8) | lsb); i++) {
                    unsigned char packet[9] = {0xCA, 0x05, 0x00, 0x00, 0x69, 0x00, (unsigned char)(i & 0xFF), (unsigned char)((i >> 8) & 0xFF), 0x00};

                    // Calculate checksum as the sum of the first 8 bytes
                    unsigned char checksum = 0;
                    for (int j = 1; j < 8; j++) {
                        checksum += packet[j];
                    }
                    packet[8] = checksum;
		    printf("Get Node info Frame \n");
		    for(int i =0; i<9; i++){
			    printf("%02X ", packet[i]);
		    }
		    printf("\n");


                    printf("%s [INFO][NC-NETWORK-DATABASE]: Sending GET_RS_INFO Packet\n", timestamp);
                    res = write_2_Uart(packet, sizeof(packet));
                    if (res != EXIT_SUCCESS) {
                        printf("%s [ERROR][NC-NETWORK-DATABASE]: Failed to write GET_RS_INFO Packet\n", timestamp);
                        return res;
                    }

                    sleep(1); // Allow response time
                }
		sleep(3);
                network_sync_state = NC_DATABASE_ENTRY; // Reset state for the next iteration
                break;
            }

            default:
                printf("%s [ERROR][NC-NETWORK-DATABASE]: Unknown state\n", timestamp);
                return EXIT_FAILURE;
        }

        printf("[DEBUG]: Updated state = %d\n", network_sync_state);
        if (network_sync_state == NC_DATABASE_ENTRY) {
            break;
        }
    }

    printf("%s [INFO][NC-NETWORK-DATABASE]: Completed Successfully\n", timestamp);
    return EXIT_SUCCESS;
}

int process_network_sync_byte(unsigned char rx_char, char *timestamp) {
	static int len = 0;
        static int pkt_len = 0;
        static int pkt_len_temp = 0;
    	static unsigned char network_response_buffer[1024] = {'\0'};  // Buffer for UART boot-up commands data

	switch(network_response_state)
	{
		case INIT:

			if(rx_char == 0xCA)
			{        

					len = 0; // Reset buffer index
                			pkt_len = 0;
                			pkt_len_temp = 0;
                			network_response_buffer[len++] = rx_char;
                			network_response_state = HDR;

			}
			break;

		case HDR:
			
			network_response_buffer[len++] = rx_char;
			if(len == 3)
			{
				pkt_len = (network_response_buffer[2] << 8) | network_response_buffer[1];
				printf("The data length : %d\n",pkt_len);
				network_response_state = RX_INFO;
					
		
			}
			break;

		case RX_INFO:

			network_response_buffer[len++] = rx_char;
			pkt_len_temp++;
			if (pkt_len_temp == pkt_len + 1) { // +1  checksum.
				
				if(network_response_buffer[4] == 0x65){

					// Extract LSB and MSB from the response
                    			lsb = network_response_buffer[8];
                    			msb = network_response_buffer[9];

                    			printf("%s [INFO][NC-DATABASE-ENTRY]: LSB = %02X, MSB = %02X\n", timestamp, lsb, msb);

				}
				else if(network_response_buffer[4] == 0x69){
					
					// Extract and format data
                    			char node_id[5];  // 4 characters + null terminator
                    			snprintf(node_id, sizeof(node_id), "%02X%02X", network_response_buffer[6], network_response_buffer[7]);

                    			char parent_id[5];  // 4 characters + null terminator
                    			snprintf(parent_id, sizeof(parent_id), "%02X%02X", network_response_buffer[8], network_response_buffer[9]);

                    			char serial_number[33];  // 16 bytes * 2 characters per byte + 1 null terminator
                    			snprintf(serial_number, sizeof(serial_number),
                             			"%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X",
                             			network_response_buffer[10], network_response_buffer[11], network_response_buffer[12],
                             			network_response_buffer[13], network_response_buffer[14], network_response_buffer[15],
                             			network_response_buffer[16], network_response_buffer[17], network_response_buffer[18],
                             			network_response_buffer[19], network_response_buffer[20], network_response_buffer[21],
                             			network_response_buffer[22], network_response_buffer[23], network_response_buffer[24],
                             			network_response_buffer[25]);

                    			char connectivity_status[3];  // 2 characters + null terminator
                    			snprintf(connectivity_status, sizeof(connectivity_status), "%02X", network_response_buffer[26]);

                    			store_network_sync_info(node_id, parent_id, serial_number, connectivity_status);


                    			printf("[INFO][NC-NODE-INFO]: Data stored in Network_sync_info database.\n");



				}

				else if(network_response_buffer[4] == 0x60){

					if(network_response_buffer[1] == 0x07){
						printf("Response1\n");


					}
					else if(network_response_buffer[1] == 0x09){

						printf("Response2\n");

				}
				}

				else if(network_response_buffer[4] == 0x68){

					printf("%s [INFO][Response from meter]: \n", timestamp);

					if(global_system_len == 2){
						char topic[5]; // 2 bytes * 2 characters per byte + 1 null terminator
						for (int i = 0; i < 2; i++) {
    							snprintf(&topic[i * 2], 3, "%02X", network_response_buffer[16 + i]);
						}
                			// Print the topic
                			printf("Topic: %s\n", topic);

					int data_len = pkt_len_temp - 23 - 1; // Length of data excluding the last byte (check)
                			printf("length is %d\n", data_len);

                			// Publish the data
                			mqtt_publish(&network_response_buffer[26], topic, data_len);

					}
					if (global_system_len > 2) {

    						char *meter_id = fetch_meter_id(global_system, global_system_len);
    						if (!meter_id) {
        						fprintf(stderr, "[ERROR]: Meter ID not found for the provided system. Exiting condition.\n");

        						// Handle the absence of meter_id, for example, skip publishing
        						len = 0;
        						pkt_len_temp = 0;
        						pkt_len = 0;
        						memset(network_response_buffer, '\0', sizeof(network_response_buffer));
        						return -1; // Or any appropriate error handling
    						}

    						printf("[INFO][METER ID]: %s \n", meter_id);

    						int data_len = pkt_len_temp - 23 - 1; // Length of data excluding the last byte (check)
    						printf("length is %d\n", data_len);

    						// Publish the data
    						mqtt_publish(&network_response_buffer[26], meter_id, data_len);

    						// Free the allocated memory for meter_id
    						free(meter_id);

					}



				}



				printf("%s [INFO][RECEIVED DATA        ]: ",timestamp);
                                int i;
                                for(i=0;i< (pkt_len+4);i++)
                                	printf("%02X ",network_response_buffer[i]);
                                printf("\n\r");
                                printf("****************************************************************************************\n\n");
				
				len=0;
                                pkt_len=0;
                                pkt_len_temp=0;
                                memset(network_response_buffer,'\0',sizeof(network_response_buffer));
                                network_response_state = INIT;

			}
			break;
			default:
            			network_response_state = INIT; // Reset on invalid state
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
//void Create_PLC_Packet_And_send(unsigned char* data, unsigned short  len)

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


