/*
 * common.h
 *
 *  Created on: 06-Jul-2018
 *      Author: pro3
 */

#ifndef COMMON_H_
#define COMMON_H_

#define DEBUG				0
#define DEBUG_1				0

#define ONE_ARG				0	/*only device /ttyUSBXX is provided to Application*/

#define _AT_CMD				0	/*AT CMD interface enable/disable*/
#define _Stack_Validation		0	/*Stack validation tool*/
#define	_RestServer			1	/*RestServer interface enable/disable*/
#define _DHCPv6				0	/*Enable DHCPv6 on Radius Linux Board*/

/*Interface IDs*/
#define HostAPD				0x01
#define Stack_Validation	0x02
#define Rest_Server			0x03


//#define IP				"192.168.1.162"	/*Own IP Address*/
//#define SERVER_IP		"192.168.1.152"	/*DHCP Server IP Address*/

#define IPv4				0
#define IPv6				1

//Function to Print command Id
static void  print_command_info(unsigned char cmdid, char *time);


#endif /* COMMON_H_ */
