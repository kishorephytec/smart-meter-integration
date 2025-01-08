/** \file WISUN_L3_MODIFIED.h
 *******************************************************************************
 ** \brief  Contains all the Processor architecture selection macros for selection 
 ** of processor Architecture.
 **
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

#ifndef WISUN_L3_MODIFIED_H
#define WISUN_L3_MODIFIED_H


/* Debdeep added NUD_RETRY_NS_TIMER as 10 seconds on 06-aug-2018.
   Because in 50kbps, time required for 20 MAC retry is approximately 
   20*400 = 8000 milliseconds = 8 seconds */
//#define NUD_NEXT_NS_TIMER               5*CLOCK_SECOND
//#define NUD_RETRY_NS_TIMER              10*CLOCK_SECOND

void change_join_state(uint8_t attemt_index);
void send_ns_to_nbr (void *a);
void event_to_send_dhcp_solicit_request();
void send_ns_with_aro(void);
void send_event_to_tcpip_process(void);
void handle_dao_callback(void *ptr);

extern uint8_t get_node_type( void );

#if(FAN_EDFE_FEATURE_ENABLED == 1)
extern void send_edfe_initial_frame(uint8_t *src_addr , uint8_t value,uint8_t edfe_frame_type);
extern void enable_disable_edfe_frame(uint8_t value,uint8_t edfe_frame_type);
#endif

extern uint16_t get_current_pan_id( void );
extern uint8_t get_LQI_from_RSSI( int8_t rssi_val );

#if(FAN_FRQ_HOPPING_FEATURE_ENABLED == 1)
extern uint8_t is_BDI_active (void);
extern uint8_t is_UDI_active (void);
#endif

extern void * app_bm_alloc(
    uint16_t length//base_t length      
    );
    
extern void app_bm_free(
    uint8_t *pMem      
    );
#endif //WISUN_L3_MODIFIED_H