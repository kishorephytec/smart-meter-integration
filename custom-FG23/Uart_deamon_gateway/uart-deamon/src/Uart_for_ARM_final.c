/*
 ============================================================================
 Name        : Uart_for_ARM_final.c
 Author      : Umesh
 Version     :
 Copyright   : Your copyright notice
 Description : UART Daemon in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <common.h>
#include <time.h>
#include <timestamp.h>
#include <mqtt.h>

char	*Interface;

#if ONE_ARG
int uart_initialisation(char	*Port);
#else
int uart_initialisation(char	*Port,char *buadrate);
#endif


int PIPE_initialisation(void);
int start_uart_rx_thread(void);
int start_HostAPD_pipe_read_thread(void);

#if _DHCPv6
int _Radius_Tx_udp_init(void);
int start_Radius_UDP_read_thread(void);
#endif

int udp_init(void);
int start_UDP_read_thread(void);

#if _RestServer
int start_Rest_Server_read_thread(void);
#endif

#if _AT_CMD
int start_AT_CMD_pipe_read_thread(void);
#endif

int main(int argc, char	*argv[]) {
	char timestamp[30];
	get_timestamp(timestamp);
#if ONE_ARG
	if(argc == 2)
	{
		Interface = argv[1];

	#if DEBUG
		printf("****************************************************************************************\n\n");
		printf("%s [INFO][Intefacing Device    ]: Opening Port On Interface[%s]\n\n",timestamp,Interface);
		printf("****************************************************************************************\n\n");
	#endif

		int _uart_init_ret = uart_initialisation(Interface);
		if( _uart_init_ret != EXIT_SUCCESS )
		{
			fprintf( stderr, "[INFO][ERROR                ]: Serial Port Initialization Failed!\n\r" );
			exit(EXIT_FAILURE);
		}
	}
	else
	{
		printf("%s [INFO][ERROR                ]: Invalid argument!!\n\r",timestamp);
		printf("%s [INFO][ERROR                ]: Execute with ./exe /dev/ttyUSBxx\n\r",timestamp);
		exit(EXIT_FAILURE);
	}
#else
	mqtt_initialise();
	if(argc == 3)
	{
		Interface = argv[1];
		
	#if DEBUG
		printf("****************************************************************************************\n\n");
		printf("%s [INFO][Intefacing Device    ]: Opening Port On Interface[%s]\n\n",timestamp,Interface);
		printf("****************************************************************************************\n\n");
	#endif
		
		
		int _uart_init_ret = uart_initialisation(Interface,argv[2]);
		if( _uart_init_ret != EXIT_SUCCESS )
		{
			fprintf( stderr, "[INFO][ERROR                ]: Serial Port Initialization Failed!\n\r" );
			exit(EXIT_FAILURE);
		}
	}
	else
	{
		printf("%s [INFO][ERROR                ]: Invalid argument!!\n\r",timestamp);
		printf("%s [INFO][ERROR                ]: Execute with ./exe /dev/ttyUSBxx 115200\n\r",timestamp);
		exit(EXIT_FAILURE);
	}
#endif


    int _pipe_init_ret = PIPE_initialisation();
    if( _pipe_init_ret != EXIT_SUCCESS )
    {
        fprintf( stderr, "%s [INFO][ERROR                ]: PIPE Initialization Failed!\n\r" ,timestamp);
        exit(EXIT_FAILURE);
    }

    int _udp_init_ret = udp_init();
    if( _udp_init_ret != EXIT_SUCCESS )
    {
	fprintf( stderr, "%s [INFO][ERROR                ]: UDP Initialization Failed!\n\r" ,timestamp);
	exit(EXIT_FAILURE);
    }

#if _DHCPv6
	int _Radius_Tx_udp_init_ret = _Radius_Tx_udp_init();
	if( _Radius_Tx_udp_init_ret != EXIT_SUCCESS )
	{
		fprintf( stderr, "%s [INFO][ERROR                ]: Radius Tx UDP Initialization Failed!\n\r",timestamp );
		exit(EXIT_FAILURE);
	}
#endif

	printf("****************************************************************************************\n\n");

    // With UART_Data_Reading function as entry function
    int uart_thread_ret = start_uart_rx_thread();
    if( uart_thread_ret != EXIT_SUCCESS )
    {
        fprintf( stderr, "%s [INFO][ERROR                ]: Serial Port Thread Creation Failed!\n\r" ,timestamp);
        exit(EXIT_FAILURE);
    }

    //HostAPD PIPE read thread creation here
    int HostAPD_pipe_thread_ret = start_HostAPD_pipe_read_thread();
    if( HostAPD_pipe_thread_ret != EXIT_SUCCESS )
    {
        fprintf( stderr, "%s [INFO][ERROR                ]: HostAPD PIPE Thread Creation Failed!\n\r" ,timestamp);
        exit(EXIT_FAILURE);
    }
#if _AT_CMD
    //AT_CMD PIPE read thread creation here
    int AT_CMD_pipe_thread_ret = start_AT_CMD_pipe_read_thread();
    if( AT_CMD_pipe_thread_ret != EXIT_SUCCESS )
    {
        fprintf( stderr, "%s [INFO][ERROR                ]: AT_CMD PIPE Thread Creation Failed!\n\r" ,timestamp);
        exit(EXIT_FAILURE);
    }
#endif

    //HostAPD PIPE read thread creation here
    int UDP_pipe_thread_ret = start_UDP_read_thread();
    if( UDP_pipe_thread_ret != EXIT_SUCCESS )
    {
        fprintf( stderr, "%s [INFO][ERROR                ]: UDP Thread Creation Failed!\n\r" ,timestamp);
        exit(EXIT_FAILURE);
    }

#if _RestServer
    //Rest_Server PIPE read thread creation here
    int Rest_Server_pipe_thread_ret = start_Rest_Server_read_thread();
    if( Rest_Server_pipe_thread_ret != EXIT_SUCCESS )
    {
        fprintf( stderr, "%s [INFO][ERROR                ]: Rest_Server Thread Creation Failed!\n\r",timestamp );
        exit(EXIT_FAILURE);
    }
#endif

#if _DHCPv6
    //Radius read thread creation here
    int Radius_UDP_thread_ret = start_Radius_UDP_read_thread();
    if( Radius_UDP_thread_ret != EXIT_SUCCESS )
    {
        fprintf( stderr, "%s [INFO][ERROR                ]: UDP Thread Creation Failed!\n\r" ,timestamp);
        exit(EXIT_FAILURE);
    }
#endif
	printf("\n****************************************************************************************\n\n");
    
	//Packet sent to LBR to fetch the IP of LBR
	Create_HIF_Packet_And_send(NULL,NULL,0x4C,0x02);

	while(1);

    printf("\n\r%s  UART Deamon Exiting\n\r",timestamp);
	return EXIT_SUCCESS;
}


