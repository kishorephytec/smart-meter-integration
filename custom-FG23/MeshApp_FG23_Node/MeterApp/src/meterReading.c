/** \file meterReading.c
 *******************************************************************************
 ** \brief Implements the functionality of Meter Integeration.
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

/*******************************************************************************
* File inclusion
*******************************************************************************/
#include "common.h"
#include "meterreading.h"
#include "queue_latest.h"
#include "buff_mgmt.h"
#include "uart_hal.h"
#include "list_latest.h"
#include "hw_tmr.h"
#include "sw_timer.h"
#include "sm.h"
#include "hif_utility.h"
#include "event_manager.h"
#include "l3_process_interface.h"
#include "uip.h"
#include "simple-udp.h"

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

//[Kimbal]
//extern hif_t hif;

//hif_service_t hif_meter_Communication_service;

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

uint8_t hif2MeterCommunicationInterface( uint8_t* pBuff,uint16_t len)
{
  //send_udp_pkt(pBuff,len);
  // TO Do 
  
  return 0;
}
/******************************************************************************/
void meter_data_wr_for_read(uint8_t *buff, uint8_t length)
{

   // hif_send_msg_up(buff,length,0xAA);
  // TO Do 
   
            
}								
/******************************************************************************/
//kimbal
#if 0
void APPhif_raw_for_meter_interface ()
{
  
   // hif group ID : 0xAA is used for RAW UART Communication with Meter...
  
  hif_register_parser
     (
          &hif,
          &hif_meter_Communication_service,
          0xAA,
          hif2MeterCommunicationInterface
     );

   
   
  
}

/******************************************************************************/
/******************************************************************************/


L3_PROCESS_NAME(app_process_manager);
L3_PROCESS(app_process_manager, "Application Main process");



void app_thread_start( void )
{
  l3_process_start(&app_process_manager, NULL);   
}

static uint8_t application_thread_process_event_handler(l3_process_event_t ev, l3_process_data_t data)
{

  switch (ev)
  {
  case APP_EV_UART_DATA_RCVD :
    {
      
    }
    break;
  case APP_EV_UART_DATA_SEND_OVER_UDP:
    {
      // Receive the data from UART .. now send Over UDP based on data type
    }
    break;
  case APP_EV_UART_DATA_SEND:
    {
      
    }
    break;
  case APP_EV_RECVD_UDP_SEND_TO_UART:
    {
      
    }
    break;    
    
  default:
    break;
    
  }
  return 0;
}



L3_PROCESS_THREAD(app_process_manager, ev, data)
{
  L3_PROCESS_BEGIN();
  
  while(1) 
  {
    L3_PROCESS_YIELD();
    application_thread_process_event_handler(ev, data);
  }
  L3_PROCESS_END();
}


void application_thread_post_event( l3_process_event_t ev, uint8_t* data)
{
  l3_process_post(&app_process_manager, ev, data);
}


void app_uart_data_recvd_send_over_udp(uint8_t* Appdata)
{
  application_thread_post_event(APP_EV_UART_DATA_SEND_OVER_UDP, NULL);    
} 


/******************************************************************************/
static struct simple_udp_connection normal_data_udp_connection;
#define ADD_PUSH_DATA_PORT_NO  8355

void normal_data_udp_rcv_callback (struct simple_udp_connection *c,
         const uip_ipaddr_t *sender_addr,
         uint16_t sender_port,
         const uip_ipaddr_t *receiver_addr,
         uint16_t receiver_port,
         const uint8_t *data,
         uint16_t datalen)
{
  
  // received the data from UDP as command to put on Meter or for self process
  // If it is for Meter put it over UART
  
  
}

void App_UDP_init_register_port ()
{
    simple_udp_register(&normal_data_udp_connection, ADD_PUSH_DATA_PORT_NO,
                      NULL, ADD_PUSH_DATA_PORT_NO, normal_data_udp_rcv_callback);
    
}


/******************************************************************************/


/******************************************************************************/



/******************************************************************************/



/******************************************************************************/



/******************************************************************************/


/******************************************************************************/





#endif
/******************************************************************************/