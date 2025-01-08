/** \file AppUDPprocess.h
 *******************************************************************************
 ** \brief It Configures the parameter used UDP socket communication.
 **
 **  This file contains the administrative configured parameter for UDP 
 **   
 **
 ** \cond STD_FILE_HEADER
 **
 ** COPYRIGHT(c) 2023-24 Procubed Innovations Pvt Ltd.  
 ** All rights reserved.
 **
 ** THIS SOFTWARE IS PROVIDED BY "AS IS" AND ALL WARRANTIES OF ANY KIND,
 ** INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR USE,
 ** ARE EXPRESSLY DISCLAIMED.  THE DEVELOPER SHALL NOT BE LIABLE FOR ANY
 ** DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE. THIS SOFTWARE
 ** MAY NOT BE USED IN PRODUCTS INTENDED FOR USE IN IMPLANTATION OR OTHER
 ** DIRECT LIFE SUPPORT APPLICATIONS WHERE MALFUNCTION MAY RESULT IN THE DIRECT
 ** PHYSICAL HARM OR INJURY TO PERSONS. ALL SUCH IS USE IS EXPRESSLY PROHIBITED.
 **
 *******************************************************************************
**/
 
#ifndef _APP_UDP_SERVER_H_
#define _APP_UDP_SERVER_H_


#define is_broadcast_address(a)          \
  ((((a)->addr[0])  == 0xFF) &&                  \
   (((a)->addr[1])  == 0xFF) &&                  \
   (((a)->addr[2]) == 0xFF) &&                  \
   (((a)->addr[3]) == 0xFF) &&                  \
   (((a)->addr[4]) == 0xFF) &&                  \
   (((a)->addr[5]) == 0xFF) &&                  \
   (((a)->addr[6]) == 0xFF) &&                  \
   (((a)->addr[7]) == 0xFF))
    

#define UDP_PORT                                8355
#define UDP_PORT1                                502
#define UDP_PORT2                                503
#define UDP_PORT3                                504
#define UDP_PORT4                                505   
    
#define MAX_UDP_LENGTH_SUPPORT                  1500

#define SEND_INTERVAL		                (10*CLOCK_SECOND)
#define SEND_INTERVAL_ROOT		        (5*CLOCK_SECOND)
#define SEND_TIME		                (l3_random_rand() % (SEND_INTERVAL))    

void start_te_udp_server( void );


#endif /* _APP_UDP_SERVER_H_ */