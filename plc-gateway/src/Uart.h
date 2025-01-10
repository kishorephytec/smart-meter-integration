/*
 * Uart.h
 *
 *  Created on: 06-Jul-2018
 *      Author: pro3
 */

#ifndef UART_H_
#define UART_H_

struct termios oldtio; //Just for saving old serail port settings
struct termios newtio; //These are the new serial port parameters

int uart_file_discripter = -1;
long BAUDRATE;


void restore_old_port_settings(void);
void *uart_rx_thread( void *);
int UART_Data_Reading();
int process_uart_byte(unsigned char rx_char,char *timestamp);
int process_boot_up_byte(unsigned char rx_char,char *timestamp);
int process_network_sync_byte(unsigned char rx_char,char *timestamp);
int get_baud(int baud);

#endif /* UART_H_ */
