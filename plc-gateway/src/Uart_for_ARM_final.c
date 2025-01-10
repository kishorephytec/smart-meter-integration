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
#include <pthread.h>
#include "mqtt.h"
#include "boot-up-NC.h"
#include <unistd.h>
#include "database.h"
#include "network-sync.h"
char	*Interface;




#if ONE_ARG
int uart_initialisation(char	*Port);
#else
int uart_initialisation(char	*Port,char *buadrate);
#endif


int start_uart_rx_thread(void);

void *scheduler(void *arg) {
    while (1) {
        int nc_network_status = network_sync_NC();
        if (nc_network_status != EXIT_SUCCESS) {
            fprintf(stderr, "[ERROR]: Failed to update network data in the database.\n");
        } else {
            printf("[INFO]: Successfully updated network data in the database.\n");
        }

        sleep(300); // Wait for 5 minutes
 	is_boot_up_phase = 2;
     
    }
    return NULL;
}

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
	sleep(1);
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


	printf("****************************************************************************************\n\n");

    // With UART_Data_Reading function as entry function
    int uart_thread_ret = start_uart_rx_thread();
    if( uart_thread_ret != EXIT_SUCCESS )
    {
        fprintf( stderr, "%s [INFO][ERROR                ]: Serial Port Thread Creation Failed!\n\r" ,timestamp);
        exit(EXIT_FAILURE);
    }

    printf("\n****************************************************************************************\n\n");

    /*NC database initialization*/
    int status = initialize_database();
    if (status!= 0) 
    {
	    fprintf(stderr, "[ERROR]: Database initialization failed.\n");

    }
    
    printf("\n****************************************************************************************\n\n");

    /*Network sync database initialization*/
    int network_status = initialize_network_syncDB();
    if (network_status!= 0)
    {
	    fprintf(stderr, "[ERROR]: Network sync Database initialization failed.\n");

    }

    printf("\n****************************************************************************************\n\n");

    /* NC boot up commands */
    int boot_status = boot_up_NC();
    if(boot_status != EXIT_SUCCESS)
    {

        fprintf( stderr, "%s [INFO][ERROR                ]: Boot up  NC Device Failed!\n\r" ,timestamp);
        exit(EXIT_FAILURE);

    }
    
    sleep(2);
    printf("\n****************************************************************************************\n\n");

    /* Create a thread for the scheduler */
    pthread_t scheduler_thread;

    if (pthread_create(&scheduler_thread, NULL, scheduler, NULL) != 0) {
        fprintf(stderr, "[ERROR]: Failed to create scheduler thread.\n");
        exit(EXIT_FAILURE);
    }

    printf("\n****************************************************************************************\n\n");


    while(1);

    printf("\n\r[%s][%s] UART Deamon Exiting\n\r",__DATE__,__TIME__);
	return EXIT_SUCCESS;
}




