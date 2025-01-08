/*
 * UDP_Server.c
 *
 *  Created on: 09-Aug-2018
 *      Author: pro3
 */

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <UDP_Server.h>
#include <errno.h>
#include <pthread.h>
#include <common.h>
#include <unistd.h>
#include <time.h>
#include <timestamp.h>

time_t t;   // not a primitive datatype

#define OWN_IP_ADDRESS "127.0.0.0"

#if _Stack_Validation
int process_UDP_rx_byte(unsigned char udp_rx_char, char *timestamp);
#endif
int UDP_Data_Reading (void);

int udp_init(void)
{
	char timestamp[30];
	get_timestamp(timestamp);

	/*Create UDP socket*/
	udpSocket = socket(AF_INET, SOCK_DGRAM, 0);

	/*Configure settings in address struct*/
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT_NO);	/*Port no*/
	serverAddr.sin_addr.s_addr = inet_addr(OWN_IP_ADDRESS);/*self IP addr*/
	//serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);
	memset(clientAddr.sin_zero, '\0', sizeof clientAddr.sin_zero);

	int count = 1;
	int ret = 0;
	/*Bind socket with address struct*/
	while( ( count <= 10 ) && ( ret = bind(udpSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr) ) ) )
	{
			sleep(2);
			printf("%s [INFO][ERROR                ]: Bind fails, Retry count %d\n\rret val = %d\n\r",timestamp,count,ret);
			count++;
	}
	if(count > 10)
	{
			printf("%s [INFO][ERROR                ]: Bind fails with error[%d].\n\r",timestamp, errno );
			return EXIT_FAILURE;
	}
	else
	{
		/*Initialize size variable to be used later on*/
		//addr_size = sizeof serverStorage;
		len = sizeof(clientAddr);
		return EXIT_SUCCESS;
		printf("%s [INFO][Socket created                ]: EXIT_SUCCESS [%d]\n\r",timestamp,EXIT_SUCCESS);

	}
	return EXIT_FAILURE;
		printf("%s [INFO][Socket failed                ]: EXIT_FAILURE [%d]\n\r",timestamp,EXIT_FAILURE);
}

int start_UDP_read_thread(void)
{
	char timestamp[30];
	get_timestamp(timestamp);
	pthread_t thread_id_UDP_read;

    int ret = pthread_create (&thread_id_UDP_read, NULL, UDP_rx_thread, NULL);
    if(ret == 0){
    	printf("%s [INFO][THREAD Creation      ]: UDP Read Thread created\n\r",timestamp);
    	return EXIT_SUCCESS;
    }
    else{
	    printf("%s [INFO][THREAD Creation      ]: UDP Read Thread not created\n\r",timestamp);
	    return EXIT_FAILURE;
    }
}


void	*UDP_rx_thread(void	*unused)
{
		UDP_Data_Reading();
	    return NULL;
}

int UDP_Data_Reading(void)
{
	while(1){
		char timestamp[30];
		get_timestamp(timestamp);
	    /* Try to receive any incoming UDP datagram. Address and port of
	      requesting client will be stored on serverStorage variable */
	    nBytes = recvfrom(udpSocket,udp_buffer,2250,0,(struct sockaddr *)&clientAddr, &len);
#if DEBUG
	    printf("%s [INFO][RECEIVED DATA        ]: Received string from UDP is  =",timestamp);
		for(i = 0;i < nBytes;i++)
		printf("%x ",udp_buffer[i]);
		printf("\n");
#endif

#if _Stack_Validation
		for(i = 0;i < nBytes;i++)
		{
			process_UDP_rx_byte(udp_buffer[i],timestamp);
		}
#else
		
		printf("%s [INFO][SOCKET Communication	]: UART Deamon Received Data from DHCPv6 Server.\n",timestamp);
				printf("%s [INFO][DATA COUNT           ]: Recived bytes from Radius %d \n\r",timestamp,nBytes);
				usleep (500000);
				/*Create HIF Packet here for board*/
				Create_HIF_Packet_And_send(udp_buffer,nBytes,0xB5,0x02);
#endif				

	  }
}


int send_2_udp_client(unsigned char *write_bufer,int length)
{
	char timestamp[30];
	get_timestamp(timestamp);

	#if DEBUG
	int k;
	printf("%s [INFO][TRANSMIT DATA        ]:Transmitt from UART Deamon to UDP :",timestamp);
	for(k = 0;k < length ;k++)
		printf("%x ",write_bufer[k]);
	printf("\n");
#endif
/*Send uppercase message back to client, using serverStorage as the address*/
	int write_count = sendto(udpSocket,write_bufer,length,0,(struct sockaddr *)&clientAddr, len);
		if(write_count != length)
		{
		# if DEBUG
			printf("%s [INFO][ERROR                ]: UDP write count [%d] with error no [%d]\n\r",timestamp,write_count,errno);
			return EXIT_FAILURE;
		#endif
		}
		printf("%s [INFO][DATA COUNT           ]:  UDP data bytes write %d\n\r",timestamp,write_count);
	return EXIT_SUCCESS;
}
