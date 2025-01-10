/*
 * network-sync.h
 *
 *  Created on: 17-Dec-2024
 *      Author: Srikanth Jogi
 */

#ifndef NETWORK_SYNC_H_
#define NETWORK_SYNC_H_


extern int is_boot_up_phase;
enum Network_Sync_Cases {
        NC_DATABASE_ENTRY = 0,
        GET_RS_INFO
};

enum Network_Sync_Response {
       INIT = 0,
       HDR,
       RX_INFO
};       




int network_sync_NC();
void Create_PLC_Packet_And_send(unsigned char* data, unsigned short  len, unsigned char* system, unsigned short system_len);
int write_2_Uart(unsigned char *write_bufer,int length);



#endif /* NETWORK_SYNC_H_ */
