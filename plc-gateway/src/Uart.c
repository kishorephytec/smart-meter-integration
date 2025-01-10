/*
 * Uart.c
 *
 *  Created on: 06-Jul-2018
 *      Author: pro3
 */

#include <sys/time.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <errno.h>
#include <termios.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <Uart.h>
#include <common.h>
#include <time.h>
#include <timestamp.h>
#include "boot-up-NC.h"

int is_boot_up_phase = 1;
#if ONE_ARG
int uart_initialisation(char	*Port)
{
		char timestamp[30];
		get_timestamp(timestamp);
#else

int uart_initialisation(char	*Port,char *buadrate)
{
		BAUDRATE = get_baud(atoi(buadrate));
		char timestamp[30];
		get_timestamp(timestamp);


		if(BAUDRATE == -1)
		{
	        fprintf( stderr, "%s [INFO][ERROR                ]: Setting Baud rate Failed!\n\r",timestamp );
	        printf("%s [INFO][ERROR                ]: Setting baud rate returns %ld\n\r",timestamp,BAUDRATE);
	        exit(EXIT_FAILURE);
		}
#endif
	    //We are ignoring SIGIO signal since we are using "select" instead
		
		signal( SIGIO, SIG_IGN );
	    //Open device to be non-blocking (read will return immediately)
	    uart_file_discripter = open( (char *)Port,
	                                          O_RDWR |
	                                          O_NOCTTY |
	                                          O_NONBLOCK );

	    //printf("uart_file_descripter: %d\n",uart_file_discripter);
	    if( uart_file_discripter <= 0 )
	    {
	        printf( "%s [INFO][ERROR                ]: Serial port[%s] failed to open with error[%d].\n\r",timestamp, Port, errno );
	        return 1;
	    }
	    //Make serial port do asynchronous input/outpuAt.
	    int ret_value = fcntl( uart_file_discripter, F_SETFL, O_ASYNC );
	    if( ret_value != 0 )
	    {
	        printf( "%s [INFO][ERROR                ]: Fcntl F_SETFL O_ASYNC on new port[%s] failed with "\
	                "error[%d].\n\r",timestamp,
	                Port, errno );
	        return 2;
	    }
	    //save old port settings
	    ret_value = tcgetattr( uart_file_discripter, &oldtio );
	    if( ret_value != 0 )
	    {
	        printf( "%s [INFO][ERROR                ]: Preserving old port[%s] failed with error[%d].\n\r",timestamp, Port, errno );
	        return 3;
	    }
	    //register this function to be executed if program exits.
	    atexit( restore_old_port_settings );

	    //clear up struct for new port
	    if( memset( &newtio, 0, sizeof(newtio) ) < (void *)1 )
	    {
	        printf( "%s [INFO][ERROR                ]: Clearing up new port[%s] failed with error[%d].\n\r",timestamp, Port, errno );
	        return 4;
	    }
	    //set new port 8N1 settings for non-canonical input processing
	    //must be NOCTTY, set baud rate
#if ONE_ARG
	    newtio.c_cflag = B460800 | CS8 | CLOCAL | CREAD;
#else
	    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
#endif
	    //(ignore parity | map '\r' to '\n')
	    newtio.c_iflag = ( IGNPAR | ICRNL );
	    newtio.c_oflag = 0; //ONLCR converts '\n' to CR-LF pairs
	    newtio.c_lflag = ~(ICANON | ECHOE | ISIG);
	    newtio.c_cc[VMIN] = 0;  //return as soon as there is at least 1-byte
	    newtio.c_cc[VTIME] = 1; //otherwise it returns in 0.1 s regardless.

	    ret_value = tcflush( uart_file_discripter, TCIFLUSH );
	    if( ret_value != 0 )
	    {
	        printf( "%s [INFO][ERROR                ]: Setting up, flushing Input data to port[%s] "\
	                "if not read, failed with error[%d].\n\r",timestamp, Port, errno );
	        return 5;
	    }

	    ret_value = tcsetattr( uart_file_discripter,TCSANOW,  &newtio );
	    if( ret_value != 0 )
	    {
	        printf( "%s [INFO][ERROR                ]: Activating port[%s] settings failed with error[%d].\n\r",timestamp,Port, errno );
	        return 6;
	    }

			printf( "%s [INFO][PORT INITIALIZE      ]: Activating port[%s] settings successful.\n\r\n\r",timestamp, Port );

	    return EXIT_SUCCESS;

	}//END init_serial_port-----------------------------------------------


	void restore_old_port_settings(void)
	{
	    //restore old port settings if there was any.
	    tcsetattr( uart_file_discripter, TCSANOW, &oldtio );
	}//END restore_old_port_settings--------------------------------------


	void *uart_rx_thread( void *unused)
	{

	    UART_Data_Reading ();

	    return NULL;
	}


	int start_uart_rx_thread(void)
	{

		char timestamp[30];
		get_timestamp(timestamp);
		pthread_t thread_id_uart;

	    int ret = pthread_create (&thread_id_uart, NULL, uart_rx_thread, NULL);
	    if(ret == 0){
	    	printf("%s [INFO][THREAD Creation      ]: UART Read Thread created\n\r",timestamp);
	    	return EXIT_SUCCESS;
	    }
	    else{
		    printf("%s [INFO][THREAD Creation      ]: UART Read Thread not created\n\r",timestamp);
		    return EXIT_FAILURE;
	    }
	}

int UART_Data_Reading (void)
	{
	    unsigned char rx_char;
	    fd_set readfs; //file descriptor set

	    while( 1 )
	    {
	    	char timestamp[30];
		get_timestamp(timestamp);

	    	FD_ZERO( &readfs );
	        //set testing for UART source
	        FD_SET( uart_file_discripter, &readfs);
	        if( read( uart_file_discripter, &rx_char, 1 ) )
	        {

			if (is_boot_up_phase == 1) {
                		// Process boot-up-related UART bytes
                		process_boot_up_byte(rx_char, timestamp);
            		}
			else if(is_boot_up_phase == 2) {
				//process Nc-Network related UART bytes
				process_network_sync_byte(rx_char, timestamp);
		        }
	        }//END of if condition...
	    }//END of while loop....
	    return EXIT_SUCCESS;
	}                 //----------------------------END of UART_Data_Reading----------------------------

int write_2_Uart(unsigned char *write_buffer, int length) {
    char timestamp[30];
    get_timestamp(timestamp);

    printf("%s [INFO][RECEIVED DATA]: Sending to UART: ", timestamp);
    for (int j = 0; j < length; j++) {
        printf("%02X ", write_buffer[j]);
    }
    printf("\n");

    if (uart_file_discripter != -1) {
        int write_count = write(uart_file_discripter, write_buffer, length);

        if (write_count == length) {
            return EXIT_SUCCESS; // Full write success
        } else if (write_count >= 0) {
            printf("%s [ERROR]: Partial write: Expected %d bytes, wrote %d bytes\n", timestamp, length, write_count);
            return 1; // Partial write
        } else {
            printf("%s [ERROR]: Write failed with errno %d\n", timestamp, errno);
            return EXIT_FAILURE; // Write failure
        }
    } else {
        printf("%s [ERROR]: Invalid UART file descriptor, errno %d\n", timestamp, errno);
        return EXIT_FAILURE; // File descriptor error
    }
}



int get_baud(int baud)
{
	char timestamp[30];
	get_timestamp(timestamp);

	switch (baud) {
	case 9600:
		return B9600;
	case 19200:
		return B19200;
	case 38400:
		return B38400;
	case 57600:
		return B57600;
	case 115200:
		return B115200;
	case 230400:
		return B230400;
	case 460800:
		return B460800;
	case 500000:
		return B500000;
	case 576000:
		return B576000;
	case 921600:
		return B921600;
	case 1000000:
		return B1000000;
	case 1152000:
		return B1152000;
	case 1500000:
		return B1500000;
	case 2000000:
		return B2000000;
	case 2500000:
		return B2500000;
	case 3000000:
		return B3000000;
	case 3500000:
		return B3500000;
	case 4000000:
		return B4000000;
	default:
		printf("%s [INFO][WARNING              ]: Please enter correct Baud rate (from 9600 to 4000000)\n\r",timestamp);
		return -1;
	}
}
