/*
 * UDP_Server.h
 *
 *  Created on: 09-Aug-2018
 *      Author: pro3
 */

#ifndef UDP_SERVER_H_
#define UDP_SERVER_H_

//#define PORT_NO	9123
#define PORT_NO	49388

unsigned char udp_buffer[2250]={'\0'};

int i=0;
int udpSocket = 0, nBytes = 0;
struct sockaddr_in serverAddr, clientAddr;
struct sockaddr_storage serverStorage;
socklen_t addr_size, client_addr_size;
int len;

void	*UDP_rx_thread(void	*unused);

#endif /* UDP_SERVER_H_ */
