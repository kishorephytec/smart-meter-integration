/*
 * Hif.c
 *
 *  Created on: 06-Jul-2018
 *      Author: pro3
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <common.h>
#include <Hif.h>
#include <time.h>
#include <command_id.h>
#include <string.h>
#include <timestamp.h>
#include <mqtt.h>
#include <system_ip.h>

enum state uart_read_state = INIT;
enum state HostAPD_pipe_read_state = INIT;
enum state AT_CMD_pipe_read_state = INIT;
enum state UDP_read_state = INIT;
enum state Rest_Server_pipe_read_state = INIT;
enum SS_state Prev_state = STOP;

char* meter_id;
extern IpPacket packet;
// Global structure initialization (no designated initializer)
IpPacket packet = {
    .counter = {0x00,0x01},
    .type = 1
};

static void print_command_info(unsigned char cmdid, char *time)
{
	int i = 0;
	while(command_id[i].cmdid != 0x00)
	{
		if (cmdid == command_id[i].cmdid)
		{
			printf("%s [INFO][WI-SUN STACK COMMAND ]: CMD ID: %x : %s\n",time, command_id[i].cmdid,command_id[i].cmd_info);
			break;
		}
		i++;
	}
}

int process_uart_byte(unsigned char rx_char, char *timestamp)
{
	static int len = 0;
	static int pkt_len = 0;
	static int pkt_len_temp = 0;
	unsigned char Hdr_chksum = 0;
	unsigned char Payload_chksum = 0;

	switch(uart_read_state)
	{
		case INIT:
			if(rx_char == 0xA5)/*for rasb pi prob.*/
			{
				uart_rx_buffer[len++]=rx_char;
				if(len == 3)
				{
					if((uart_rx_buffer[0] == 0xA5) && (uart_rx_buffer[1] == 0xA5) && (uart_rx_buffer[2] == 0xA5))
					{
						uart_read_state = HDR;
					#if DEBUG
						//Printing Start of Frame
						int i=0;
						for(i=0;i<3;i++)
						printf("%X ",uart_rx_buffer[i]);
					#endif
					}	
					else
					{
						len=0;
						memset(uart_rx_buffer,'\0',sizeof(uart_rx_buffer));
						uart_read_state = INIT;
					}
				}
			}
			break;
		case HDR:
				uart_rx_buffer[len++]=rx_char;
				if(len == 9)
				{
					layer_id = uart_rx_buffer[4];
					cmd_id = uart_rx_buffer[5];

					Hdr_chksum = ~(uart_rx_buffer[3]+uart_rx_buffer[4]+uart_rx_buffer[5]+uart_rx_buffer[6]+uart_rx_buffer[7]);
					Hdr_chksum &= (~((~0)<<8));
				#if DEBUG
					//Printing Checksum
					//printf("\n\n\rCalculated header checksum = %X\n\r",Hdr_chksum);
					//printf("Received header checksum= %X\n\r",uart_rx_buffer[8]);					
				#endif

					/*Comparing the Checksum and Printing */
					if(uart_rx_buffer[8] == Hdr_chksum)
					{
						pkt_len = ((uart_rx_buffer[7] << 8) | (uart_rx_buffer[6]));
						#if DEBUG
						printf("%s [INFO][Header Checksum      ]: Verification Success.\n",timestamp);
						//printf("\n\rLength is  %d",pkt_len);
						#endif
						uart_read_state = RX_DATA;
					}
					else
					{
						//printf("\n\rHeader checksum fail\n\r");
						printf("%s [INFO][Header Checksum      ]: Verification Failure.\n",timestamp);
						len=0;
						pkt_len=0;
						cmd_id=0;
						//pkt_len_temp=0;
						memset(uart_rx_buffer,'\0',sizeof(uart_rx_buffer));
						uart_read_state = INIT;
					}
				}
			break;
		case RX_DATA:
					uart_rx_buffer[len++]=rx_char;
					pkt_len_temp++;
					if(pkt_len_temp == (pkt_len+2))
					{
						#if DEBUG
						//printf("\n\rpkt_len_temp = %d\n\r",pkt_len_temp);
						#endif
						Payload_chksum = Generate_Checksum_payload(&uart_rx_buffer[9],pkt_len);
						#if DEBUG
						//printf("\n\rCalculated payload checksum = %X\n\r",Payload_chksum);
						//printf("Received payload checksum = %X\n\n\r",uart_rx_buffer[(pkt_len+11)-2]);					
						printf("layer id:%02x \n",layer_id);
						#endif

						if(Payload_chksum == uart_rx_buffer[(pkt_len+11)-2])
						{
							printf("%s [INFO][Payload Checksum     ]: Verification Success.\n",timestamp);
							
							/*put data on PIPE*/
							printf("%s [INFO][UART Communication   ]: Uart Deamon Received Data from EFM32 Board \n",timestamp);
							if(cmd_id >= 0x10 && cmd_id <= 0x27)
							{
								print_command_info(cmd_id,timestamp);
								printf("%s [INFO][PIPE Communication   ]: Uart Deamon Sending Data to HostAPD Pipe.\n",timestamp);
								write_2_HostAPD_PIPE(&uart_rx_buffer[0],(pkt_len+11));
							}
							/*need to modified */
							else{
								switch (cmd_id) {
									case 0x10 ... 0x27: // Handle range 0x10 to 0x27
										print_command_info(cmd_id, timestamp);
										printf("%s [INFO][PIPE Communication   ]: Uart Deamon Sending Data to HostAPD Pipe.\n", timestamp);
										write_2_HostAPD_PIPE(&uart_rx_buffer[0], (pkt_len + 11));
										break;

									case 0x51:
										//printf("pkt_len:%d \n", pkt_len);
										//ip_parsing(&uart_rx_buffer[9], pkt_len);
										memcpy(packet.src_address,&uart_rx_buffer[9],16);
										memcpy(packet.dst_address,&uart_rx_buffer[9+16],16);
										break;

									case 0x4D:
										if(uart_rx_buffer[9] == 0x11){
										//meter_id=decode_message(&uart_rx_buffer[9+16],pkt_len - 16);
										meter_id=call_decode_message(&uart_rx_buffer[10+16],pkt_len - 17);// 16(ip)+1(TYPE)
										printf("Value: %s\n",meter_id);
										insert_system(&uart_rx_buffer[10],meter_id, timestamp);
										}
										else{
										memcpy(packet.src_address,&uart_rx_buffer[10],16);
										#if 1
										printf("LBR:");
										for(int i=0;i<16;i++){
											printf("%02X ",packet.src_address[i]);
										}
										printf("\n");
										#endif
										}
										break;

									case 0x07:
										/*****CMD ID: 0x07 data received from the METER */
											#if _AT_CMD
											print_command_info(cmd_id, timestamp);
											printf("%s [INFO][PIPE Communication   ]: Uart Deamon Sending Data to AT CMD Pipe.\n", timestamp);
											write_2_AT_CMD_PIPE(&uart_rx_buffer[0], (pkt_len + 11));
											#endif

													/* send pkt on udp */
											if (layer_id == STV_LAYER_ID) {
												#if TEST
												printf("%d \n", __LINE__);
												print_command_info(cmd_id, timestamp);
												printf("%s [INFO][SOCKET Communication	]: Uart Deamon Sending Data to Procubed Stack Validation Tool.\n", timestamp);
												mqtt_publish(&uart_rx_buffer[9], pkt_len);
												#endif

												#if METER
												printf("%d \n", __LINE__);
												print_command_info(cmd_id, timestamp);
												printf("%s [INFO][SOCKET Communication	]: Uart Deamon Sending Data to Procubed Stack Validation Tool.\n", timestamp);
												char ip[16];
												memcpy(ip, &uart_rx_buffer[9], 16);
												mqtt_publish(&uart_rx_buffer[9 + 16], pkt_len - 16, ip);
												#endif
													} 
											
											#if _DHCPv6
											else if (layer_id == DHCP_LAYER_ID) {
												print_command_info(cmd_id, timestamp);
												printf("%s [INFO][SOCKET Communication	]: Uart Deamon Sending Data to DHCPv6 Server.\n", timestamp);
												send_to_dhcp_server(&uart_rx_buffer[9], pkt_len);
													}
											#endif

											#if _RestServer
											else if (layer_id == REST_LAYER_ID) {
											
														print_command_info(cmd_id, timestamp);
														printf("%s [INFO][PIPE Communication   ]: Uart Deamon Sending Data to Rest Server Pipe.\n", timestamp);
														write_2_Rest_Server_PIPE(&uart_rx_buffer[0], (pkt_len + 11));
													} 
											#endif

											else {
														printf("%s [INFO][ERROR                ]: Layer ID \"%x\" Not Recognized\n\r!!Packet Discard!!\n\r", timestamp, layer_id);
													}
										break;

									default:
										printf("%s [INFO][ERROR                ]: CMD ID \"%x\" Not Recognized\n\r!!Packet Discard!!\n\r", timestamp, cmd_id);
										break;
								}
							}
							
							printf("%s [INFO][RECEIVED DATA        ]: ",timestamp);
							int i;
							for(i=0;i< (pkt_len+11);i++)
								printf("%x ",uart_rx_buffer[i]);
							printf("\n\r");
							printf("****************************************************************************************\n\n");
							//mqtt_publish(&uart_rx_buffer,(pkt_len+11));
							/*write on pipe end here*/
#if DEBUG_1
						printf("\n\rcmd_id is %x\n\r",cmd_id);

						for(int i=9;i<(pkt_len+9);i++){
							printf("%x ",uart_rx_buffer[i]);
						}
						printf("\n\r");
						printf("----------------------------------------------------------------------------------------\n\r");
#endif
							//printf("read Pkt from uart count = %d\n",++rx_count);
							len=0;
							pkt_len=0;
							pkt_len_temp=0;
							cmd_id=0;
							memset(uart_rx_buffer,'\0',sizeof(uart_rx_buffer));
							uart_read_state = INIT;
						}
						else
						{
							printf("%s [INFO][Payload Checksum     ]: Verification Failure.\n",timestamp);
							//printf("\n\rPayload checksum fail\n\r");
							len=0;
							pkt_len=0;
							pkt_len_temp=0;
							cmd_id=0;
							memset(uart_rx_buffer,'\0',sizeof(uart_rx_buffer));
							uart_read_state = INIT;
						}
					}
			break;
	default:
			printf("%s [INFO][ERROR                ]: Something went wrong\n\r",timestamp);
			uart_read_state = INIT;
		break;
	}
	return 1;
}


unsigned char Generate_Checksum_payload( unsigned char* data ,int payld_len )
{
	unsigned char calc_chksum  = 0 ;
	int iCnt  =0;


  for (iCnt = 0; iCnt<payld_len;iCnt++)
  {
      calc_chksum += *data++;
  }

	return ~(calc_chksum);
}

int process_HostAPD_pipe_byte(unsigned char h_rx_char)
{
	char timestamp[30];
		get_timestamp(timestamp);
	static int p_len = 0;
		static int p_pkt_len = 0;
		static int p_pkt_len_temp = 0;
		unsigned char Hdr_chksum = 0;
		unsigned char Payload_chksum = 0;

		
		switch(HostAPD_pipe_read_state)
		{
		case	INIT:
			if(h_rx_char == 0xA5)/*for rasb pi prob.*/
			{
					HostAPD_pipe_rx_buffer[p_len++]=h_rx_char;
					if(p_len == 3)
					{
						if((HostAPD_pipe_rx_buffer[0] == 0xA5) && (HostAPD_pipe_rx_buffer[1] == 0xA5) && (HostAPD_pipe_rx_buffer[2] == 0xA5))
						{
							HostAPD_pipe_read_state = HDR;
#if DEBUG
							//int i=0;
							//for(i=0;i<3;i++)
							//	printf("%X ",HostAPD_pipe_rx_buffer[i]);
#endif
						}
						else
						{
							p_len=0;
							memset(HostAPD_pipe_rx_buffer,'\0',sizeof(HostAPD_pipe_rx_buffer));
							HostAPD_pipe_read_state = INIT;
						}
					}
				}
			break;
		case	HDR:
					HostAPD_pipe_rx_buffer[p_len++]=h_rx_char;
					if(p_len == 9)
					{
						Hdr_chksum = ~(HostAPD_pipe_rx_buffer[3]+HostAPD_pipe_rx_buffer[4]+HostAPD_pipe_rx_buffer[5]+HostAPD_pipe_rx_buffer[6]+HostAPD_pipe_rx_buffer[7]);
						Hdr_chksum &= (~((~0)<<8));
#if DEBUG
						//printf("\n\n\rCalculated header checksum = %X\n\r",Hdr_chksum);
						//printf("Received header checksum= %X\n\r",HostAPD_pipe_rx_buffer[8]);
#endif
						if(HostAPD_pipe_rx_buffer[8] == Hdr_chksum)
						{
							p_pkt_len = ((HostAPD_pipe_rx_buffer[7] << 8) | (HostAPD_pipe_rx_buffer[6]));/*reverse byte calculation of length*/
#if DEBUG
							//printf("\n\rLength is  %d",p_pkt_len);
							printf("%s [INFO][Header Checksum      ]: Verification Success.\n",timestamp);
#endif
							HostAPD_pipe_read_state = RX_DATA;
						}
						else
						{
							printf("%s [INFO][Header Checksum      ]: Verification Failure.\n",timestamp);
							//printf("\n\rHader checksum fail\n\r");
							p_len=0;
							p_pkt_len=0;
							//p_pkt_len_temp=0;
							memset(HostAPD_pipe_rx_buffer,'\0',sizeof(HostAPD_pipe_rx_buffer));
							HostAPD_pipe_read_state = INIT;
						}
					}
			break;
		case	RX_DATA:
						HostAPD_pipe_rx_buffer[p_len++]=h_rx_char;
						p_pkt_len_temp++;
						if(p_pkt_len_temp == (p_pkt_len+2))
						{
#if DEBUG
							//printf("\n\rpkt_len_temp = %d\n\r",p_pkt_len_temp);
#endif
							Payload_chksum = Generate_Checksum_payload(&HostAPD_pipe_rx_buffer[9],p_pkt_len);
#if DEBUG
							//printf("\n\rCalculated payload checksum = %X\n\r",Payload_chksum);
							//printf("Received payload checksum = %X\n\n\r",HostAPD_pipe_rx_buffer[(p_pkt_len+11)-2]);
#endif
							if(Payload_chksum == HostAPD_pipe_rx_buffer[(p_pkt_len+11)-2])
							{
								printf("%s [INFO][Payload Checksum     ]: Verification Success.\n",timestamp);
								printf("%s [INFO][PIPE Communication   ]: UART Deamon Received Data from HostAPD Pipe.\n",timestamp);
								/*put data on Uart*/
								
								printf("%s [INFO][UART Communication   ]: UART Deamon Sending Data to EFM32 Board.\n",timestamp);
								write_2_Uart(&HostAPD_pipe_rx_buffer[0],(p_pkt_len+11));
								/*write on Uart end here*/
#if DEBUG_1
								
								//printf("----------------------------------------------------------------------------------------\n\n\r");
								printf("%s [INFO][RECEIVED DATA        ]: ",timestamp);
								int i=0;
								for(i=0;i<(p_pkt_len+11);i++)
									printf("%x ",HostAPD_pipe_rx_buffer[i]);
								printf("\n\r");
								printf("****************************************************************************************\n\n");
#endif
								//printf("write Pkt to uart count = %d\n",++tx_count);
								p_len=0;
								p_pkt_len=0;
								p_pkt_len_temp=0;
								memset(HostAPD_pipe_rx_buffer,'\0',sizeof(HostAPD_pipe_rx_buffer));
								HostAPD_pipe_read_state = INIT;
							}
							else
							{
								//printf("\n\rPayload checksum fail\n\r");
								printf("%s [INFO][Payload Checksum     ]: Verification Failure.\n",timestamp);
								p_len=0;
								p_pkt_len=0;
								p_pkt_len_temp=0;
								memset(HostAPD_pipe_rx_buffer,'\0',sizeof(HostAPD_pipe_rx_buffer));
								HostAPD_pipe_read_state = INIT;
							}
						}
			break;
		default:
				printf("%s [INFO][ERROR                ]:Something went wrong\n\r",timestamp);
				HostAPD_pipe_read_state = INIT;
			break;
		}
		return 1;
}

#if _AT_CMD
int process_AT_CMD_pipe_byte(unsigned char at_rx_char)
{
		static int at_len = 0;
		static int at_pkt_len = 0;
		static int at_pkt_len_temp = 0;
		unsigned char Hdr_chksum = 0;
		unsigned char Payload_chksum = 0;
		
		

		switch(AT_CMD_pipe_read_state)
		{
		case	INIT:
			if(at_rx_char == 0xA5)/*for rasb pi prob.*/
			{
					AT_CMD_pipe_rx_buffer[at_len++]=at_rx_char;
					if(at_len == 3)
					{
						if((AT_CMD_pipe_rx_buffer[0] == 0xA5) && (AT_CMD_pipe_rx_buffer[1] == 0xA5) && (AT_CMD_pipe_rx_buffer[2] == 0xA5))
						{
							AT_CMD_pipe_read_state = HDR;
#if DEBUG
							//int i=0;
							//for(i=0;i<3;i++)
							//	printf("%X ",AT_CMD_pipe_rx_buffer[i]);
#endif
						}
						else
						{
							at_len=0;
							memset(AT_CMD_pipe_rx_buffer,'\0',sizeof(AT_CMD_pipe_rx_buffer));
							AT_CMD_pipe_read_state = INIT;
						}
					}
				}
			break;
		case	HDR:
					AT_CMD_pipe_rx_buffer[at_len++]=at_rx_char;
					if(at_len == 9)
					{
						Hdr_chksum = ~(AT_CMD_pipe_rx_buffer[3]+AT_CMD_pipe_rx_buffer[4]+AT_CMD_pipe_rx_buffer[5]+AT_CMD_pipe_rx_buffer[6]+AT_CMD_pipe_rx_buffer[7]);
						Hdr_chksum &= (~((~0)<<8));
#if DEBUG
						//printf("\n\n\rCalculated header checksum = %X\n\r",Hdr_chksum);
						//printf("Received header checksum= %X\n\r",AT_CMD_pipe_rx_buffer[8]);
#endif
						if(AT_CMD_pipe_rx_buffer[8] == Hdr_chksum)
						{
							at_pkt_len = ((AT_CMD_pipe_rx_buffer[6] << 8) | (AT_CMD_pipe_rx_buffer[7]));/*reverse byte calculation of length*/
#if DEBUG
							printf("%s [INFO][Header Checksum      ]: Verification Success.\n",timestamp);
							//printf("\n\rLength is  %d",at_pkt_len);
#endif
							AT_CMD_pipe_read_state = RX_DATA;
						}
						else
						{
							//printf("\n\rHader checksum fail\n\r");
							printf("%s [INFO][Header Checksum      ]: Verification Failure.\n",timestamp);
							at_len=0;
							at_pkt_len=0;
							//p_pkt_len_temp=0;
							memset(AT_CMD_pipe_rx_buffer,'\0',sizeof(AT_CMD_pipe_rx_buffer));
							AT_CMD_pipe_read_state = INIT;
						}
					}
			break;
		case	RX_DATA:
						AT_CMD_pipe_rx_buffer[at_len++]=at_rx_char;
						at_pkt_len_temp++;
						if(at_pkt_len_temp == (at_pkt_len+2))
						{
#if DEBUG
							//printf("\n\rpkt_len_temp = %d\n\r",at_pkt_len_temp);
#endif
							Payload_chksum = Generate_Checksum_payload(&AT_CMD_pipe_rx_buffer[9],at_pkt_len);
#if DEBUG
							//printf("\n\rCalculated payload checksum = %X\n\r",Payload_chksum);
							//printf("Received payload checksum = %X\n\n\r",AT_CMD_pipe_rx_buffer[(at_pkt_len+11)-2]);
#endif
							if(Payload_chksum == AT_CMD_pipe_rx_buffer[(at_pkt_len+11)-2])
							{
								printf("%s [INFO][Payload Checksum     ]: Verification Success.\n",timestamp);
								printf("%s [INFO][PIPE Communication   ]: UART Deamon Received Data from AT CMD Pipe.\n",timestamp);
							/*put data on Uart*/
								printf("[INFO][UART Communication   ]: UART Deamon Sending Data to EFM32 Board.\n",timestamp);
								write_2_Uart(&AT_CMD_pipe_rx_buffer[0],(at_pkt_len+11));
							/*write on Uart end here*/
#if DEBUG_1
								//printf("----------------------------------------------------------------------------------------\n\r");
								printf("%s [INFO][RECEIVED DATA        ]: ",timestamp);
								int i=0;
								for(i=0;i<(at_pkt_len+11);i++)
									printf("%x ",AT_CMD_pipe_rx_buffer[i]);
								printf("\n\r");
								printf("****************************************************************************************\n\n");
#endif
								//printf("write Pkt to uart count = %d\n",++tx_count);
								at_len=0;
								at_pkt_len=0;
								at_pkt_len_temp=0;
								memset(AT_CMD_pipe_rx_buffer,'\0',sizeof(AT_CMD_pipe_rx_buffer));
								AT_CMD_pipe_read_state = INIT;
							}
							else
							{
								printf("%s [INFO][Payload Checksum     ]: Verification Failure.\n",timestamp);
								//printf("\n\rPayload checksum fail\n\r");
								at_len=0;
								at_pkt_len=0;
								at_pkt_len_temp=0;
								memset(AT_CMD_pipe_rx_buffer,'\0',sizeof(AT_CMD_pipe_rx_buffer));
								AT_CMD_pipe_read_state = INIT;
							}
						}
			break;
		default:
				printf("%s [INFO][ERROR                ]: Something went wrong\n\r",timestamp);
				AT_CMD_pipe_read_state = INIT;
			break;
		}
		return 1;
}
#endif


#if _Stack_Validation
int process_UDP_rx_byte(unsigned char udp_rx_char,char *timestamp)
{
	static int u_len = 0;
	static int u_pkt_len = 0;
	static int u_pkt_len_temp = 0;
	unsigned char Hdr_chksum = 0;
	unsigned char Payload_chksum = 0;

	

	switch(UDP_read_state)
	{
	case	INIT:
		if(udp_rx_char == 0xA5)/*for rasb pi prob.*/
		{
				UDP_rx_buffer[u_len++]=udp_rx_char;
				if(u_len == 3)
				{
					if((UDP_rx_buffer[0] == 0xA5) && (UDP_rx_buffer[1] == 0xA5) && (UDP_rx_buffer[2] == 0xA5))
					{
						UDP_read_state = HDR;
#if DEBUG
						//int i=0;
						//for(i=0;i<3;i++)
						//	printf("%X ",UDP_rx_buffer[i]);
#endif
					}
					else
					{
						u_len=0;
						memset(UDP_rx_buffer,'\0',sizeof(UDP_rx_buffer));
						UDP_read_state = INIT;
					}
				}
			}
		break;
	case	HDR:
				UDP_rx_buffer[u_len++]=udp_rx_char;
				if(u_len == 9)
				{
					udp_cmd_id = UDP_rx_buffer[5];
					Hdr_chksum = ~(UDP_rx_buffer[3]+UDP_rx_buffer[4]+UDP_rx_buffer[5]+UDP_rx_buffer[6]+UDP_rx_buffer[7]);
					Hdr_chksum &= (~((~0)<<8));
#if DEBUG
					//printf("\n\n\rCalculated header checksum = %X\n\r",Hdr_chksum);
					//printf("Received header checksum= %X\n\r",UDP_rx_buffer[8]);
#endif
					if(UDP_rx_buffer[8] == Hdr_chksum)
					{
						u_pkt_len = ((UDP_rx_buffer[7] << 8) | (UDP_rx_buffer[6]));/*reverse byte calculation of length*/
#if DEBUG
						//printf("\n\rLength is  %d",u_pkt_len);
						printf("%s [INFO][Header Checksum      ]: Verification Success.\n",timestamp);
#endif
						UDP_read_state = RX_DATA;
					}
					else
					{
						printf("%s [INFO][Header Checksum      ]: Verification Failure.\n",timestamp);
						//printf("\n\rHader checksum fail\n\r");
						u_len=0;
						u_pkt_len=0;
						//p_pkt_len_temp=0;
						memset(UDP_rx_buffer,'\0',sizeof(UDP_rx_buffer));
						UDP_read_state = INIT;
					}
				}
		break;
	case	RX_DATA:
					UDP_rx_buffer[u_len++]=udp_rx_char;
					u_pkt_len_temp++;
					if(u_pkt_len_temp == (u_pkt_len+2))
					{
#if DEBUG
						//printf("\n\rpkt_len_temp = %d\n\r",u_pkt_len_temp);
#endif
						Payload_chksum = Generate_Checksum_payload(&UDP_rx_buffer[9],u_pkt_len);
#if DEBUG
						//printf("\n\rCalculated payload checksum = %X\n\r",Payload_chksum);
						//printf("Received payload checksum = %X\n\n\r",UDP_rx_buffer[(u_pkt_len+11)-2]);
#endif
						if(Payload_chksum == UDP_rx_buffer[(u_pkt_len+11)-2])
						{
							printf("%s [INFO][Payload Checksum     ]: Verification Success.\n",timestamp);
							printf("%s [INFO][SOCKET Communication	]: UART Deamon Received Data from Procubed Stack Validation Tool.\n",timestamp);
							if(udp_cmd_id == 0x30) /*for TY-Realese*/
							{
								//printf("cmd id matched\n\r");
								print_command_info(udp_cmd_id, timestamp);
								check_hostAPD_state(UDP_rx_buffer[9]);
							}
						/*put data on Uart*/
							printf("%s [INFO][UART Communication   ]: UART Deamon Sending Data to EFM32 Board.\n",timestamp);
							write_2_Uart(&UDP_rx_buffer[0],(u_pkt_len+11));
						/*write on Uart end here*/
#if DEBUG_1
							//printf("----------------------------------------------------------------------------------------\n\r");
							printf("%s [INFO][RECEIVED DATA        ]: ",timestamp);
							int i=0;
							for(i=0;i<(u_pkt_len+11);i++)
								printf("%x ",UDP_rx_buffer[i]);
							printf("\n\r");
							printf("****************************************************************************************\n\n");
#endif
							//printf("write Pkt to uart count = %d\n",++tx_count);
							u_len=0;
							u_pkt_len=0;
							u_pkt_len_temp=0;
							memset(UDP_rx_buffer,'\0',sizeof(UDP_rx_buffer));
							UDP_read_state = INIT;
						}
						else
						{
							printf("%s [INFO][Payload Checksum     ]: Verification Failure.\n",timestamp);
							//printf("\n\rPayload checksum fail\n\r");
							u_len=0;
							u_pkt_len=0;
							u_pkt_len_temp=0;
							memset(UDP_rx_buffer,'\0',sizeof(UDP_rx_buffer));
							UDP_read_state = INIT;
						}
					}
		break;
	default:
			printf("%s [INFO][ERROR                ]:Something went wrong\n\r",timestamp);
			UDP_read_state = INIT;
		break;
	}
	return 1;
}
#endif

#if _RestServer
int process_Rest_Server_pipe_byte(unsigned char rs_rx_char)
{
	static int RS_len = 0;
	static int RS_pkt_len = 0;
	static int RS_pkt_len_temp = 0;
	unsigned char Hdr_chksum = 0;
	unsigned char Payload_chksum = 0;

	char timestamp[30];
		get_timestamp(timestamp);

	switch(Rest_Server_pipe_read_state)
	{
	case	INIT:
		if(rs_rx_char == 0xA5)/*for rasb pi prob.*/
		{
				Rest_Server_pipe_rx_buffer[RS_len++]=rs_rx_char;
				if(RS_len == 3)
				{
					if((Rest_Server_pipe_rx_buffer[0] == 0xA5) && (Rest_Server_pipe_rx_buffer[1] == 0xA5) && (Rest_Server_pipe_rx_buffer[2] == 0xA5))
					{
						Rest_Server_pipe_read_state = HDR;
#if DEBUG
						//int i=0;
						//for(i=0;i<3;i++)
						//	printf("%X ",Rest_Server_pipe_rx_buffer[i]);
#endif
					}
					else
					{
						RS_len=0;
						memset(Rest_Server_pipe_rx_buffer,'\0',sizeof(Rest_Server_pipe_rx_buffer));
						Rest_Server_pipe_read_state = INIT;
					}
				}
			}
		break;
	case	HDR:
				Rest_Server_pipe_rx_buffer[RS_len++]=rs_rx_char;
				if(RS_len == 9)
				{
					RestS_cmd_id = Rest_Server_pipe_rx_buffer[5];
					Hdr_chksum = ~(Rest_Server_pipe_rx_buffer[3]+Rest_Server_pipe_rx_buffer[4]+Rest_Server_pipe_rx_buffer[5]+Rest_Server_pipe_rx_buffer[6]+Rest_Server_pipe_rx_buffer[7]);
					Hdr_chksum &= (~((~0)<<8));
#if DEBUG
					//printf("\n\n\rCalculated header checksum = %X\n\r",Hdr_chksum);
					//printf("Received header checksum= %X\n\r",Rest_Server_pipe_rx_buffer[8]);
#endif
					if(Rest_Server_pipe_rx_buffer[8] == Hdr_chksum)
					{
						RS_pkt_len = ((Rest_Server_pipe_rx_buffer[7] << 8) | (Rest_Server_pipe_rx_buffer[6]));/*reverse byte calculation of length*/
#if DEBUG
						printf("%s [INFO][Header Checksum      ]: Verification Success.\n",timestamp);
						//printf("\n\rLength is  %d",RS_pkt_len);
#endif
						Rest_Server_pipe_read_state = RX_DATA;
					}
					else
					{
						printf("%s [INFO][Header Checksum      ]: Verification Failure.\n",timestamp);
						//printf("\n\rHader checksum fail\n\r");
						RS_len=0;
						RS_pkt_len=0;
						//p_pkt_len_temp=0;
						memset(Rest_Server_pipe_rx_buffer,'\0',sizeof(Rest_Server_pipe_rx_buffer));
						Rest_Server_pipe_read_state = INIT;
					}
				}
		break;
	case	RX_DATA:
					Rest_Server_pipe_rx_buffer[RS_len++]=rs_rx_char;
					RS_pkt_len_temp++;
					if(RS_pkt_len_temp == (RS_pkt_len+2))
					{
#if DEBUG
						//printf("\n\rpkt_len_temp = %d\n\r",RS_pkt_len_temp);
#endif
						Payload_chksum = Generate_Checksum_payload(&Rest_Server_pipe_rx_buffer[9],RS_pkt_len);
#if DEBUG
						//printf("\n\rCalculated payload checksum = %X\n\r",Payload_chksum);
						//printf("Received payload checksum = %X\n\n\r",Rest_Server_pipe_rx_buffer[(RS_pkt_len+11)-2]);
#endif
						if(Payload_chksum == Rest_Server_pipe_rx_buffer[(RS_pkt_len+11)-2])
						{
							printf("%s [INFO][Payload Checksum     ]: Verification Success.\n",timestamp);
							printf("%s [INFO][PIPE Communication   ]: UART Deamon Received Data from Rest Server Pipe.\n",timestamp);
							//printf("\n\rRest Cmd Id %x\n\r",RestS_cmd_id);
							print_command_info(RestS_cmd_id, timestamp);
							if(RestS_cmd_id == 0x30) /*for TY-Realese*/
							{
								//print_command_info(RestS_cmd_id);
								//printf("cmd id matched\n\r");
								check_hostAPD_state(Rest_Server_pipe_rx_buffer[9]);
							}
							/*put data on Uart*/
							
							printf("%s [INFO][UART Communication   ]: UART Deamon Sending Data to EFM32 Board.\n",timestamp);
							write_2_Uart(&Rest_Server_pipe_rx_buffer[0],(RS_pkt_len+11));
						/*write on Uart end here*/
#if DEBUG_1
							//printf("----------------------------------------------------------------------------------------\n\r");
							printf("%s [INFO][RECEIVED DATA        ]: ",timestamp);
							int i=0;
							for(i=0;i<(RS_pkt_len+11);i++)
								printf("%x ",Rest_Server_pipe_rx_buffer[i]);
							printf("\n\r");
							printf("****************************************************************************************\n\n");
#endif
							//printf("write Pkt to uart count = %d\n",++tx_count);
							RS_len=0;
							RS_pkt_len=0;
							RS_pkt_len_temp=0;
							memset(Rest_Server_pipe_rx_buffer,'\0',sizeof(Rest_Server_pipe_rx_buffer));
							Rest_Server_pipe_read_state = INIT;
						}
						else
						{
							printf("%s [INFO][Payload Checksum     ]: Verification Failure.\n",timestamp);
							//printf("\n\rPayload checksum fail\n\r");
							RS_len=0;
							RS_pkt_len=0;
							RS_pkt_len_temp=0;
							memset(Rest_Server_pipe_rx_buffer,'\0',sizeof(Rest_Server_pipe_rx_buffer));
							Rest_Server_pipe_read_state = INIT;
						}
					}
		break;
	default:
			printf("%s [INFO][ERROR                ]: Something went wrong\n\r",timestamp);
			Rest_Server_pipe_read_state = INIT;
		break;
	}
	return 1;
}
#endif


void check_hostAPD_state(unsigned char current_state)
{
	char timestamp[30];
		get_timestamp(timestamp);

	printf("%s [INFO][HOSTAPD STATE        ]: Previous state: %x\n",timestamp,Prev_state);
	printf("%s [INFO][HOSTAPD STATE        ]: Current state : %x\n",timestamp,current_state);
	switch(Prev_state)
	{
		case START:
			if(current_state == START)
			{
				printf("%s [INFO][HOSTAPD STATE        ]: HostAPD Already Running\n",timestamp);
			}
			else if(current_state == STOP)
			{	
				printf("%s [INFO][HOSTAPD STATE        ]: Stoping HostAPD\n\r",timestamp);
				system("/home/pi/start_hostapd.sh 0");
				Prev_state = STOP;
			}
			else
			{
				printf("%s [INFO][HOSTAPD STATE        ]: Received Wrong Cmd Data from START state\n\r",timestamp);
			}
			break;
		case STOP:
			if(current_state == START)
			{
				system("/home/pi/start_hostapd.sh 1");
				Prev_state = START;
			}
			else if(current_state == STOP)
			{	
				printf("%s [INFO][HOSTAPD STATE        ]: Not in Running State\n\r",timestamp);
			}
			else
			{
				printf("%s [INFO][HOSTAPD STATE        ]: Received Wrong Cmd Data from STOP state\n\r",timestamp);
			}
			break;
		default :
			printf("%s [INFO][ERROR                ]: Something went wrong\n\r",timestamp);
			break;
	}
}

void Create_HIF_Packet_And_send(unsigned char* data, unsigned short  len, char CMDId, char LayerId)
{
#if	0
	unsigned char	*dataPtr = NULL ;
	int  total_buff_len = (len+ 9 + 2 ) ; // +2 is for Payload Check sum and com prot
	dataPtr= (unsigned char*)malloc (total_buff_len*sizeof(char));
	memset (dataPtr , 0x00, total_buff_len);

	if (dataPtr == NULL)
	{
		printf("\n\rdataPtr got NULL\n\r");
		exit(EXIT_FAILURE);
	}

	/*Header*/
	dataPtr[0] = 0xA5;
	dataPtr[1] = 0xA5;
	dataPtr[2] = 0xA5;

	dataPtr[3] = 0x01;
	dataPtr[4] = DHCP_LAYER_ID;
	dataPtr[5] = CMDId;

	dataPtr[7] = (char)((len >> 8) & 0xFF);
	dataPtr[6] = (char) len;

	printf("\n\r Generated length [6] = %d ",dataPtr[6]);
	printf("Generated length [7] = %d \n\r",dataPtr[7]);

	dataPtr[8] = ~(0x01+ DHCP_LAYER_ID + CMDId + (char) len  +(char )((len >> 8) & 0xFF));
	memcpy (&dataPtr[9] , data , len);
	printf("length is %d \n\r",len);
	dataPtr +=len;
	*dataPtr++ = Generate_Checksum_payload (&data[0] ,len );
	*dataPtr = 0xDD;

	/*Write data to uart*/
	printf("%s [INFO][UART Communication   ]: UART Deamon Sending Data to EFM32 Board.",timestamp);
	print_command_info(CMDId, timestamp);
	write_2_Uart(&dataPtr[0],len+11);

#if DEBUG_1
	printf("After Write");
#endif



	free(dataPtr);
#endif

	char timestamp[30];
	get_timestamp(timestamp);

	int res;
	int ii;
	unsigned char *orig_dataPtr = NULL ;
	unsigned char *dataPtr = NULL ;
	unsigned short total_buff_len = (len+ 9 + 2 ) ; // +2 is for Payload Check sum and com prot

	//Data to send to METER
	if(CMDId == 0x48){
		total_buff_len += 37;
	}

	orig_dataPtr= (unsigned char *)malloc (total_buff_len+1);
    	printf ("%s [INFO][DATA Receiving           ]: at HIF Function [%d]: ",timestamp, len);
        for(int i =0; i<len;i++)
	{

		printf("%x ",data[i]);
	}

	printf("\n");

	if (orig_dataPtr == NULL)
	{
		exit(EXIT_FAILURE);
	}

	memset (orig_dataPtr , 0x00, total_buff_len);
	dataPtr= orig_dataPtr;

	*dataPtr++ = 0xA5;
	*dataPtr++ = 0xA5;
	*dataPtr++ = 0xA5;

	*dataPtr++ = 0x01; 	//PROTOCOL_ID;
	*dataPtr++ = LayerId;	 	//LAYER_ID;
	*dataPtr++ = CMDId;

	if(CMDId == 0x48){
        *(unsigned short*)dataPtr = len + 37;
	   	}
	else{
	*(unsigned short*)dataPtr = len;
	}
	dataPtr += 2;
//	*dataPtr++ = (uint8_t )((len >> 8) & 0xFF);
//	*dataPtr++ = (uint8_t) len;
//
        /*printf ("%s [INFO][DATAPTR           ]: HIF packet ",timestamp);
	dataPtr = orig_dataPtr;
        for(int i =0;i<6;i++)
	{

		printf("%x ",dataPtr[i]);
	}
	printf("\r\n");*/


	if(CMDId == 0x48){
		int length = len+37;
		*dataPtr++ = ~(0x01 +LayerId + CMDId + (char) length +(char) ((length>>8) & 0xFF));
		}
	else{
		*dataPtr++ = ~(0x01 + LayerId + CMDId + (char) len  +(char )((len >> 8) & 0xFF));
		}
	if(CMDId == 0x48){
		memcpy(dataPtr, &packet.src_address, 16);
		dataPtr += 16;

		memcpy(dataPtr, &packet.dst_address, 16);
		dataPtr += 16;

		memcpy(dataPtr, &packet.counter, 2);
		dataPtr += 2;

		memcpy(dataPtr, &packet.type, 1);
		dataPtr += 2;

		*(unsigned short*)dataPtr = len;
		dataPtr += 1;
	}

	memcpy (dataPtr , data , len);
	dataPtr +=len;

	if(CMDId == 0x48){
		int length = len +37;
		char total_data[length];

		memcpy(total_data, &packet.src_address, 16);
		memcpy(total_data +16, &packet.dst_address, 16);
		memcpy(total_data+32, &packet.counter, 2);
		memcpy(total_data+34, &packet.type, 1);

		total_data[35] = (len >> 8) & 0xFF;
		total_data[36] = len & 0xFF;

		memcpy(total_data+37,data,len);
		
		*dataPtr++ = Generate_Checksum_payload (total_data, length);
	}
	else{
	*dataPtr++ = Generate_Checksum_payload (data ,len );
	}
	*dataPtr++ = 0xDD;

    printf ("%s [INFO][DATA COUNT           ]: SENDING HIF packet [%d]: ",timestamp, total_buff_len);
    dataPtr = orig_dataPtr;
    for (ii = 0; ii < total_buff_len; ii++)
    	printf ("%x ", dataPtr[ii]);
    printf ("\r\n");
	
	printf("%s [INFO][UART Communication   ]: UART Deamon Sending Data to EFM32 Board.\n",timestamp);
	print_command_info(CMDId, timestamp);
	res = write_2_Uart(dataPtr, total_buff_len);

	free (orig_dataPtr);

}
