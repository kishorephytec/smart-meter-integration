/*
 * PIPE.c
 *
 *  Created on: 06-Jul-2018
 *      Author: pro3
 */

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <pthread.h>
#include <common.h>
#include <PIPE.h>
#include <time.h>
#include <timestamp.h>

#define BUF_SIZE 1024

int PIPE_initialisation(void)
{
	char timestamp[30];
	get_timestamp(timestamp);

	int pipe_create_return = creat_PIPE(&HostAPD_fd_recv,HostAPD_PIPE_recv,HostAPD_recv_PIPE_Permission);
	if(pipe_create_return != EXIT_SUCCESS)
	{
        fprintf( stderr, "%s [INFO][ERROR                ]: HostAPD Write PIPE Init Failed!\n\r",timestamp );
        exit(EXIT_FAILURE);
	}

	pipe_create_return = creat_PIPE(&HostAPD_fd_send,HostAPD_PIPE_send,HostAPD_send_PIPE_Permission);
	if(pipe_create_return != EXIT_SUCCESS)
	{
        fprintf( stderr, "%s [INFO][ERROR                ]: HostAPD Receive PIPE Init Failed!\n\r" ,timestamp);
        exit(EXIT_FAILURE);
	}
#if _RestServer
	pipe_create_return = creat_PIPE(&RestServer_fd_recv,RestServer_PIPE_recv,RestServer_recv_PIPE_Permission);
	if(pipe_create_return != EXIT_SUCCESS)
	{
        fprintf( stderr, "%s [INFO][ERROR                ]: RestServer Write PIPE Init Failed!\n\r",timestamp );
        exit(EXIT_FAILURE);
	}

	pipe_create_return = creat_PIPE(&RestServer_fd_send,RestServer_PIPE_send,RestServer_send_PIPE_Permission);
	if(pipe_create_return != EXIT_SUCCESS)
	{
        fprintf( stderr, "%s [INFO][ERROR                ]: RestServer Receive PIPE Init Failed!\n\r" ,timestamp);
        exit(EXIT_FAILURE);
	}
#endif

#if _AT_CMD
 	pipe_create_return = creat_PIPE(&AT_CMD_fd_recv,AT_CMD_PIPE_recv,AT_CMD_recv_PIPE_Permission);
	if(pipe_create_return != EXIT_SUCCESS)
	{
        fprintf( stderr, "%s [INFO][ERROR                ]: AT_CMD Write PIPE Init Failed!\n\r",timestamp );
        exit(EXIT_FAILURE);
	}

	pipe_create_return = creat_PIPE(&AT_CMD_fd_send,AT_CMD_PIPE_send,AT_CMD_send_PIPE_Permission);
	if(pipe_create_return != EXIT_SUCCESS)
	{
        fprintf( stderr, "[%s][%s][INFO][ERROR                ]: AT_CMD Receive PIPE Init Failed!\n\r",timestamp );
        exit(EXIT_FAILURE);
	}
#endif

	return EXIT_SUCCESS;
}

int creat_PIPE(int 	*fd,char	*PIPE_Path,char	*file_permission)
{
	char timestamp[30];
	get_timestamp(timestamp);
#if DEBUG
	printf("%s [INFO][PIPE Creation        ]: Received file path = %s\n\r",timestamp,PIPE_Path);
	printf("%s [INFO][PIPE Creation        ]: Received file permission = %s\n\r",timestamp,file_permission);
#endif
	int ret_mkfifo = mkfifo(PIPE_Path,0666);
	if(ret_mkfifo == -1)
		printf("%s [INFO][PIPE Creation        ]: Check,is PIPE already created?\n\r\n\r",timestamp);
	system(file_permission);
	*fd=open(PIPE_Path,O_RDWR);
#if DEBUG
	printf("%s [INFO][PIPE Creation        ]: Received PIPE file descriptor = %d\n\r\n\r",timestamp,*fd);
#endif
	if(*fd <= -1)
	{
		printf("%s [INFO][ERROR                ]: File descriptor [%d] failed with error[%d] \n\r",timestamp,*fd,errno);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;

}

int write_2_HostAPD_PIPE(unsigned char *write_bufer,int length)
{
	int write_count = write(HostAPD_fd_send,write_bufer,length);

	if(write_count != length)
	{
# if DEBUG
		printf("[%s][%s][INFO][ERROR                ]: Write fail & Write count [%d] with error no [%d]\n\r",__DATE__,__TIME__,write_count,errno);
		return write_count;
#endif
	}
		return EXIT_SUCCESS;
}
#if _AT_CMD
int write_2_AT_CMD_PIPE(unsigned char *write_bufer,int length)
{
	int write_count = write(AT_CMD_fd_send,write_bufer,length);

	if(write_count != length)
	{
# if DEBUG
		printf("[%s][%s][INFO][ERROR                ]: Write fail & Write count [%d] with error no [%d]\n\r",__DATE__,__TIME__,write_count,errno);
		return write_count;
#endif
	}
		return EXIT_SUCCESS;
}
#endif

int start_HostAPD_pipe_read_thread(void)
{
	char timestamp[30];
	get_timestamp(timestamp);

	pthread_t thread_id_hostapd_pipe_read;

    int ret = pthread_create (&thread_id_hostapd_pipe_read, NULL, HostAPD_pipe_rx_thread, NULL);
    if(ret == 0){
    	printf("%s [INFO][THREAD Creation      ]: HostAPD PIPE Read Thread created\n\r",timestamp);
    	return EXIT_SUCCESS;
    }
    else{
	    printf("%s [INFO][THREAD Creation      ]: HostAPD PIPE Read Thread not created\n\r",timestamp);
	    return EXIT_FAILURE;
    }
}


void	*HostAPD_pipe_rx_thread(void	*unused)
{
	    HostAPD_PIPE_Data_Reading ();

	    return NULL;
}


int HostAPD_PIPE_Data_Reading(void)
{
    unsigned char rx_char;
    fd_set hostapd_read_pipe; //file descriptor set

    while( 1 )
    {
        FD_ZERO( &hostapd_read_pipe );
        //set testing for PIPE source
        FD_SET( HostAPD_fd_recv, &hostapd_read_pipe);
        if( read( HostAPD_fd_recv, &rx_char, 1 ) )
        {
            //printf("\n Got the data  - %x \n ", rx_char);
        	process_HostAPD_pipe_byte(rx_char);
        }//END of if condition...
    }//END of while loop....
    return EXIT_SUCCESS;
}

#if _AT_CMD
int start_AT_CMD_pipe_read_thread(void)
{
    pthread_t thread_id_AT_CMD_pipe_read;

    int ret = pthread_create (&thread_id_AT_CMD_pipe_read, NULL, AT_CMD_pipe_rx_thread, NULL);
    if(ret == 0){
    	printf("[%s][%s][INFO][THREAD Creation      ]: AT CMD PIPE Read Thread created\n\r",__DATE__,__TIME__);
    	return EXIT_SUCCESS;
    }
    else{
	    printf("[%s][%s][INFO][THREAD Creation      ]: AT CMD PIPE Read Thread not created\n\r",__DATE__,__TIME__);
	    return EXIT_FAILURE;
    }
}

void *AT_CMD_pipe_rx_thread(void *unused)
{
	    AT_CMD_PIPE_Data_Reading ();

	    return NULL;
}

int AT_CMD_PIPE_Data_Reading(void)
{
    unsigned char rx_char;
    fd_set AT_CMD_read_pipe; //file descriptor set

    while( 1 )
    {
        FD_ZERO( &AT_CMD_read_pipe );
        //set testing for PIPE source
        FD_SET( AT_CMD_fd_recv, &AT_CMD_read_pipe);
        if( read( AT_CMD_fd_recv, &rx_char, 1 ) )
        {
            //printf("\n Got the data  - %x \n ", rx_char);
        	process_AT_CMD_pipe_byte(rx_char);
        }//END of if condition...
    }//END of while loop....
    return EXIT_SUCCESS;
}
#endif


#if _RestServer
int start_Rest_Server_read_thread(void)
{
	char timestamp[30];
	get_timestamp(timestamp);

	pthread_t thread_id_Rest_Server_pipe_read;

    int ret = pthread_create (&thread_id_Rest_Server_pipe_read, NULL, Rest_Server_pipe_rx_thread, NULL);
    if(ret == 0){
    	printf("%s [INFO][THREAD Creation      ]: Rest Server PIPE Read Thread created\n\r",timestamp);
    	return EXIT_SUCCESS;
    }
    else{
	    printf("%s [INFO][THREAD Creation      ]: Rest Server PIPE Read Thread not created\n\r",timestamp);
	    return EXIT_FAILURE;
    }
}

void *Rest_Server_pipe_rx_thread(void *unused)
{
		Rest_Server_PIPE_Data_Reading ();

	    return NULL;
}

int Rest_Server_PIPE_Data_Reading(void)
{
    int nBytes;
	unsigned char rx_char[BUF_SIZE];
    fd_set Rest_Server_read_pipe; //file descriptor set

    while( 1 )
    {
    	char timestamp[30];
		get_timestamp(timestamp);

    	FD_ZERO( &Rest_Server_read_pipe );
        //set testing for PIPE source
        FD_SET( RestServer_fd_recv, &Rest_Server_read_pipe);
 /*       if( read( RestServer_fd_recv, &rx_char, 1 ) )
        {
            //printf("\n Got the data  - %x \n ", rx_char);
        	process_Rest_Server_pipe_byte(rx_char);
        }//END of if condition... */
       nBytes= read( RestServer_fd_recv, &rx_char, BUF_SIZE );

       printf("%s [INFO][SOCKET Communication	]: UART Deamon Received Data from DHCPv6 Server.\n",timestamp);
       		printf("%s [INFO][DATA COUNT           ]: Recived bytes from Radius %d \n\r",timestamp,nBytes);
       		usleep (500000);
       		/*Create HIF Packet here for board*/
       		Create_HIF_Packet_And_send(rx_char,nBytes,0xB5,0x05);

    }//END of while loop....
    return EXIT_SUCCESS;
}

int write_2_Rest_Server_PIPE(unsigned char *write_bufer,int length)
{
	char timestamp[30];
	get_timestamp(timestamp);

	int write_count = write(RestServer_fd_send,write_bufer,length);

	if(write_count != length)
	{
# if DEBUG
		printf("%s [INFO][ERROR                ]: Write fail & Write count [%d] with error no [%d]\n\r",timestamp,write_count,errno);
		return write_count;
#endif
	}
		return EXIT_SUCCESS;
}

#endif


