/*
 * boot-up-NC.c
 *
 *  Created on: 13-Dec-2024
 *      Author: Srikanth Jogi
 */

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <common.h>
#include <time.h>
#include <timestamp.h>
#include "boot-up-NC.h"
#include "database.h"

enum BootResponse boot_response_state = SOFT_RESET_RESPONSE1;
int boot_up_NC()
{

    char timestamp[30];
    get_timestamp(timestamp); // Assume this function provides a timestamp string

    unsigned char response[256];
    int case_status = SOFT_RESET;
    int res;

    while (case_status != BOOT_DONE) {
        int response_length = 0;

        switch (case_status) {
        	case SOFT_RESET: {
            		unsigned char command[] = {0xCA,0x02,0x00,0x00,0x20,0x22}; 

	    		printf("%s [INFO][BOOT-UP]: Sending Soft Reset Command\n\r", timestamp);

	    		res = write_2_Uart((uint8_t *)command, sizeof(command));
	    		sleep(2);

            		printf("%s [INFO][BOOT-UP]: Soft Reset Successful\n\r", timestamp);
            		case_status = GET_NETWORK_SIZE;
            		break;

		}

        	case GET_NETWORK_SIZE: {
            		unsigned char command[] = {0xCA,0x07,0x00,0x00,0x42,0x06,0x38,0x00,0x01,0x00,0x88}; 
            		printf("%s [INFO][BOOT-UP]: Sending Get Network Size Command\n\r", timestamp);

	    		res = write_2_Uart((uint8_t *)command, sizeof(command));
			sleep(2);
                    	printf("%s [INFO][BOOT-UP]: Network Size Received\n\r", timestamp);
                    	case_status = GET_NC_DATABASE_SIZE;
            		break;
        	}

		case GET_NC_DATABASE_SIZE: {
            		unsigned char command[] = {0xCA,0x07,0x00,0x00,0x42,0x06,0x5B,0x00,0x01,0x00,0xAB}; 

	    		printf("%s [INFO][BOOT-UP]: Sending Get NC Database size Command\n\r", timestamp);

	    		res = write_2_Uart((uint8_t *)command, sizeof(command));
	    		sleep(2);

            		printf("%s [INFO][BOOT-UP]: NC Database size Received\n\r", timestamp);
            		case_status = GET_SERIAL_NUMBER;
            		break;

		}

		case GET_SERIAL_NUMBER: {
            		unsigned char command[] = {0xCA,0x05,0x00,0x00,0x42,0x05,0xAB,0xBA,0xB1};

	    		printf("%s [INFO][BOOT-UP]: Sending Get Serial Number Command\n\r", timestamp);

	    		res = write_2_Uart((uint8_t *)command, sizeof(command));
	    		sleep(2);

            		printf("%s [INFO][BOOT-UP]: Serial Number Received\n\r", timestamp);
            		case_status = GET_NETWORK_ID;
            		break;
		}

		case GET_NETWORK_ID: {
            		unsigned char command[] = {0xCA,0x07,0x00,0x00,0x42,0x06,0x18,0x00,0x01,0x00,0x68};

	    		printf("%s [INFO][BOOT-UP]: Sending Get Network ID Command\n\r", timestamp);

	    		res = write_2_Uart((uint8_t *)command, sizeof(command));
	    		sleep(2);

            		printf("%s [INFO][BOOT-UP]: Network ID Received\n\r", timestamp);
            		case_status = GET_NODE_ID;
            		break;
		}

		case GET_NODE_ID: {
            		unsigned char command[] = {0xCA,0x07,0x00,0x00,0x42,0x06,0x19,0x00,0x01,0x00,0x69};

	    		printf("%s [INFO][BOOT-UP]: Sending Get Node ID Command\n\r", timestamp);

	    		res = write_2_Uart((uint8_t *)command, sizeof(command));
	    		sleep(2);

            		printf("%s [INFO][BOOT-UP]: Node ID Received\n\r", timestamp);
            		case_status = BOOT_DONE;
            		break;

		}

        	default:
            		fprintf(stderr, "%s [ERROR][BOOT-UP]: Unknown State\n\r", timestamp);
            		return EXIT_FAILURE;
        }
    }

    is_boot_up_phase = 2;
    printf("%s [INFO][BOOT-UP]: Boot-Up Sequence Completed Successfully\n\r", timestamp);
    return EXIT_SUCCESS;

}


int process_boot_up_byte(unsigned char rx_char, char *timestamp)
{
	static unsigned char boot_response_buffer[1024] = {'\0'}; // Buffer size for UART boot up commands data
    	static int len = 0;               // Current buffer length
	
	switch (boot_response_state) {

		case SOFT_RESET_RESPONSE1:
			boot_response_buffer[len++] = rx_char;

			// Process soft-reset-response1 (first 07 bytes)
            		if (len == 7) {
                		if (boot_response_buffer[0] == 0xCC ) {
                    		printf("%s [INFO][Response1]: for soft RESET.\n", timestamp);
                    		for (int i = 0; i < len; i++) {
                        		printf("%02X ", boot_response_buffer[i]);
                    		}
                    		printf("\n");

                    		boot_response_state = SOFT_RESET_RESPONSE2;  // Transition to the next state
                    		len = 0;  // Reset buffer for the next packet
                		} 	
				else {
                    		// Reset if conditions are not met
                    			len = 0;
                    			memset(boot_response_buffer,'\0',sizeof(boot_response_buffer));
                    			boot_response_state= SOFT_RESET_RESPONSE1;

                		}

            		}
            		break;
		case SOFT_RESET_RESPONSE2:
			boot_response_buffer[len++] = rx_char;

			// Process soft-reset-response2 (first 07 bytes)
            		if (len == 7) {
                		if (boot_response_buffer[0] == 0xCA ) {
                    		printf("%s [INFO][Response2]: for soft RESET.\n", timestamp);
                    		for (int i = 0; i < len; i++) {
                        		printf("%02X ", boot_response_buffer[i]);
                    		}
                    		printf("\n");

                    		boot_response_state = SOFT_RESET_RESPONSE3;  // Transition to the next state
                    		len = 0;  // Reset buffer for the next packet
                		} 
				else {
                    		// Reset if conditions are not met
                    			len = 0;
                    			memset(boot_response_buffer,'\0',sizeof(boot_response_buffer));
                    			boot_response_state = SOFT_RESET_RESPONSE1;

                			}
            		}
            		break;

		case SOFT_RESET_RESPONSE3:
			boot_response_buffer[len++] = rx_char;

			// Process packet admission (first 08 bytes)
            		if (len == 8) {
                		if (boot_response_buffer[0] == 0xCA ) {
                    			printf("%s [INFO][Response3]: for soft RESET.\n", timestamp);
                    			for (int i = 0; i < len; i++) {
                        			printf("%02X ", boot_response_buffer[i]);
                    			}
                    			printf("\n");

                    			boot_response_state = NETWORK_SIZE_RESPONSE;  // Transition to the next state
                    			len = 0;  // Reset buffer for the next packet
                		} 
				else {
                    			// Reset if conditions are not met
                    			len = 0;
                    			memset(boot_response_buffer,'\0',sizeof(boot_response_buffer));
                    			boot_response_state = SOFT_RESET_RESPONSE1;

                		}
            		}
            		break;

		case NETWORK_SIZE_RESPONSE:
			boot_response_buffer[len++] = rx_char;

			// Process Network size  (first 09 bytes)
            		if (len == 9) {
                		if (boot_response_buffer[0] == 0xCA ) {
                    			printf("%s [INFO][NETWORK SIZE]: for size of the Network.\n", timestamp);
                    			for (int i = 0; i < len; i++) {
                        			printf("%02X ", boot_response_buffer[i]);
                    			}
                    			printf("\n");

					// Extract and store the network size from the response
            				int network_size = (boot_response_buffer[6]) | (boot_response_buffer[7] << 8); // Combine LSB and MSB
            				store_network_size(network_size);

                    			boot_response_state = NC_DATABASE_SIZE_RESPONSE;  // Transition to the next state
                    			len = 0;  // Reset buffer for the next packet
                		} 
				else {
                    			// Reset if conditions are not met
                    			len = 0;
                    			memset(boot_response_buffer,'\0',sizeof(boot_response_buffer));
                    			boot_response_state = SOFT_RESET_RESPONSE1;

                		}	
            		}
            		break;

		case NC_DATABASE_SIZE_RESPONSE:
			boot_response_buffer[len++] = rx_char;

			// Process NC database size  (first 9 bytes)
            		if (len == 9) {
                		if (boot_response_buffer[0] == 0xCA ) {
                    			printf("%s [INFO][NC DATABASE SIZE]: for size of the NC Database.\n", timestamp);
                    			for (int i = 0; i < len; i++) {
                        			printf("%02X ", boot_response_buffer[i]);
                    			}
                    			printf("\n");

					// Extract and store the NC database from the response
            				int NC_database_size = (boot_response_buffer[6]) | (boot_response_buffer[7] << 8); // Combine LSB and MSB
            				store_nc_database_size(NC_database_size);

                    			boot_response_state = SERIAL_NUMBER_RESPONSE;  // Transition to the next state
                    			len = 0;  // Reset buffer for the next packet
                		}
				else {
                    			// Reset if conditions are not met
                    			len = 0;
                    			memset(boot_response_buffer,'\0',sizeof(boot_response_buffer));
                    			boot_response_state = SOFT_RESET_RESPONSE1;

                		}
            		}
            		break;

		case SERIAL_NUMBER_RESPONSE:
			boot_response_buffer[len++] = rx_char;

			// Process serial number (first 23 bytes)
        		if (len == 23) {
        			if (boot_response_buffer[0] == 0xCA ) {
                			printf("%s [INFO][NC SERIAL NUMBER]: for NC serial number.\n", timestamp);
                    			for (int i = 0; i < len; i++) {
                        			printf("%02X ", boot_response_buffer[i]);
                    			}
                    			printf("\n");

					//Extract NC serial number
					char serial_number[16];
					memcpy(serial_number,&boot_response_buffer[6],16);
					store_serial_number(serial_number);

                    			boot_response_state = NETWORK_ID_RESPONSE;  // Transition to the next state
                    			len = 0;  // Reset buffer for the next packet
                		}
                		else {
                    			// Reset if conditions are not met
                    			len = 0;
                    			memset(boot_response_buffer,'\0',sizeof(boot_response_buffer));
                    			boot_response_state = SOFT_RESET_RESPONSE1;

                		}	
            		}
            		break;

		case NETWORK_ID_RESPONSE:
			boot_response_buffer[len++] = rx_char;

			// Process network ID (first 0 bytes)
        		if (len == 9) {
        			if (boot_response_buffer[0] == 0xCA ) {
                			printf("%s [INFO][NETWORK ID]: for NC Network ID.\n", timestamp);
                    			for (int i = 0; i < len; i++) {
                        			printf("%02X ", boot_response_buffer[i]);
                    			}
                    			printf("\n");

					// Extract and store the network id from the response

					// Copy the 6th and 7th bytes into a buffer as a string
            				char network_id_str[5] = {0};  // 4 chars + null terminator
            				memcpy(network_id_str, &boot_response_buffer[6], 2);  // Copy 2 bytes
            				snprintf(network_id_str, sizeof(network_id_str), "%02X%02X",boot_response_buffer[6], boot_response_buffer[7]);

            				store_network_id(network_id_str);

                    			boot_response_state = NODE_ID_RESPONSE;  // Transition to the next state
                    			len = 0;  // Reset buffer for the next packet
                		}
                		else {
                    			// Reset if conditions are not met
                    			len = 0;
                    			memset(boot_response_buffer,'\0',sizeof(boot_response_buffer));
                    			boot_response_state = SOFT_RESET_RESPONSE1;

                		}
            		}
            		break;

		case NODE_ID_RESPONSE:
			boot_response_buffer[len++] = rx_char;

			// Process Node ID (first 9 bytes)
        		if (len == 9) {
        			if (boot_response_buffer[0] == 0xCA ) {
                			printf("%s [INFO][Node ID]: for NC Node ID.\n", timestamp);
                    			for (int i = 0; i < len; i++) {
                        			printf("%02X ", boot_response_buffer[i]);
                    			}
                    			printf("\n");

					// Extract and store the node id from the response
					// Copy the 6th and 7th bytes into a buffer as a string
            				char node_id_str[5] = {0};  // 4 chars + null terminator
            				memcpy(node_id_str, &boot_response_buffer[6], 2);  // Copy 2 bytes
            				snprintf(node_id_str, sizeof(node_id_str), "%02X%02X",boot_response_buffer[6], boot_response_buffer[7]);

            				store_node_id(node_id_str);

                    			len = 0;  // Reset buffer for the next packet
                		}
                		else {
                    			// Reset if conditions are not met
                    			len = 0;
                    			memset(boot_response_buffer,'\0',sizeof(boot_response_buffer));
                    			boot_response_state = SOFT_RESET_RESPONSE1;

                		}
            		}
            		break;

		default:
    			// Default state: handle unexpected states
    			printf("%s [ERROR]: Unexpected state encountered: %d\n", timestamp, boot_response_state);
    			// Reset to a known state
    			boot_response_state = SOFT_RESET_RESPONSE1;
    			len = 0;
    			memset(boot_response_buffer, '\0', sizeof(boot_response_buffer));
    			break;

	}
	return 0;

}

