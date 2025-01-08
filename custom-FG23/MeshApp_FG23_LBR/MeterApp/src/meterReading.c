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

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/


extern hif_t hif;

hif_service_t hif_meter_Communication_service;

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