
/** \file contiki_mac_interface.h
 *******************************************************************************
 ** \brief Provides information about the MAC process and event
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
 **  \endcond
 */


#ifndef CONTIKI_MAC_INTERFACE_H
#define CONTIKI_MAC_INTERFACE_H

#ifdef __cplusplus
extern "C" {
#endif

void mac_poll( void );
void post_mac_event( uint8_t event );
void post_mil_event( uint8_t event );
void mil_poll( void );
void timer_tick_poll(void);
void spi_poll(void);

L3_PROCESS_NAME(promac_process);
L3_PROCESS_NAME(mil_process);
L3_PROCESS_NAME(spi_process);
L3_PROCESS_NAME(timer_tick_process);


#ifdef __cplusplus
}
#endif
#endif /* CONTIKI_MAC_INTERFACE_H */