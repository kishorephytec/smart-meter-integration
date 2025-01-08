/*
 * Hif.h
 *
 *  Created on: 06-Jul-2018
 *      Author: pro3
 */

#ifndef HIF_H_
#define HIF_H_


enum state{
	INIT,
	HDR,
	RX_DATA
};

enum SS_state{
	STOP,
	START
};

#define STV_LAYER_ID	0x02
//#define STV_LAYER_ID	0x07
#define REST_LAYER_ID	0x05
//#define DHCP_LAYER_ID	0x07
#define DHCP_LAYER_ID	0x09

/*setting Rx buffers*/
unsigned char uart_rx_buffer[2250]={'\0'};
unsigned char mqtt_buf[2250]={'\0'};
unsigned char HostAPD_pipe_rx_buffer[2250]={'\0'};
unsigned char UDP_rx_buffer[2250]={'\0'};

#if _AT_CMD
unsigned char AT_CMD_pipe_rx_buffer[2250]={'\0'};
#endif

#if _RestServer
unsigned char Rest_Server_pipe_rx_buffer[2250]={'\0'};
#endif

unsigned char cmd_id,udp_cmd_id,RestS_cmd_id,layer_id,host_cmd_id;
int rx_count,tx_count;

unsigned char Generate_Checksum_payload( unsigned char* data ,int payld_len );
int write_2_HostAPD_PIPE(unsigned char *write_bufer,int lenght);
int write_2_AT_CMD_PIPE(unsigned char *write_bufer,int lenght);
int write_2_Uart(unsigned char *write_bufer,int length);
int send_2_udp_client(unsigned char *write_bufer,int length);
int write_2_Rest_Server_PIPE(unsigned char *write_bufer,int length);
void check_hostAPD_state(unsigned char current_state);
int send_to_dhcp_server(unsigned char *write_bufer,int length);

#endif /* HIF_H_ */
