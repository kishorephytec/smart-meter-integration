/*
 * Plc.h
 *
 *  Created on: 13-Dec-2024
 *      Author: Srikanth Jogi
 */

#ifndef BOOT_UP_H_
#define BOOT_UP_H_


extern int is_boot_up_phase;
enum BootCases {
        SOFT_RESET = 0,
        GET_NETWORK_SIZE,
        GET_NC_DATABASE_SIZE,
        GET_SERIAL_NUMBER,
        GET_NETWORK_ID,
        GET_NODE_ID,
        BOOT_DONE
};

enum BootResponse {
	SOFT_RESET_RESPONSE1 = 0,
	SOFT_RESET_RESPONSE2,
	SOFT_RESET_RESPONSE3,
	NETWORK_SIZE_RESPONSE,
        NC_DATABASE_SIZE_RESPONSE,
        SERIAL_NUMBER_RESPONSE,
        NETWORK_ID_RESPONSE,
        NODE_ID_RESPONSE
};





int boot_up_NC();
int write_2_Uart(unsigned char *write_bufer,int length);



#endif /* BOOT_UP_H_ */
