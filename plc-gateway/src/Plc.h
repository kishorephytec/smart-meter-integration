/*
 * Plc.h
 *
 *  Created on: 09-Dec-2024
 *      Author: Srikanth Jogi
 */

#ifndef PLC_H_
#define PLC_H_


enum state{
        PKT_ADM,
	PKT_TX,
	RX_DATA
};


/*setting Rx buffers*/
//extern unsigned char uart_rx_buffer[2250];


void Create_PLC_Packet_And_send(unsigned char* data, unsigned short  len, unsigned char* system, unsigned short system_len);
//void Create_PLC_Packet_And_send(unsigned char* data, unsigned short  len);

int write_2_Uart(unsigned char *write_bufer,int length);



#endif /* PLC_H_ */
