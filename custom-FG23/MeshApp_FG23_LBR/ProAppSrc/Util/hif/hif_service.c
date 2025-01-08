/** \file hif_service.c
 *******************************************************************************
 ** \brief 
 ** Implements the Host interface functionality
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

/*
********************************************************************************
* File inclusion
********************************************************************************
*/

#include "StackPHYConf.h"
#include "common.h"
#include "queue_latest.h"
#include "uart_hal.h"
#include "hif_utility.h"
#include "event_manager.h"
#include "sw_timer.h"
#include "timer_service.h"
#if APP_LBR_ROUTER
#if(APP_HIF_PROCESS_FEATURE_ENABLED == 1)
#include "hif_process.h"
#endif
#endif
//#include "buffer_service.h"

/*
** =============================================================================
** Private Macro definitions
** =============================================================================
*/
	
/* None */

/*
** =============================================================================
** Private Structures, Unions & enums Type Definitions
** =============================================================================
**/

/* None */

/*
** =============================================================================
** Private Variable Definitions
** =============================================================================
*/

drv_calls_t driver = 
{
	NULL,
	uart_hal_write,
	uart_hal_read,
	UART_init,
	UART_close
};

static hif_t* mp_hif;
static uint16_t rx_buff_wrong_queue_cnt = 0;

/*
** =============================================================================
** Private Function Prototypes
** =============================================================================
*/

static void hif_service_notify( uint8_t evt );

/*
** =============================================================================
** External Variable Declarations
** =============================================================================
*/

extern uint8_t heap[];
#if APP_LBR_ROUTER
extern sw_tmr_t uart_debug;//@Umesh 05_12_2018
#endif
//extern uart_hal_data_t uart_hal_info;

/*
** =============================================================================
** External Function Prototypes
** =============================================================================
*/

//extern void signal_event_to_mac_task( uint8_t event );

/*
** =============================================================================
** Public Variables Definitions
** =============================================================================
**/

void clear_buffer_and_state(void *uart_data);

/*
** =============================================================================
** Public Function Prototypes
** =============================================================================
*/

/* None */

/*
** =============================================================================
** Public Function Definitions
** =============================================================================
*/

void hif_service_init(hif_t* p_hif_data) //To be called from main
{

	mp_hif = p_hif_data;
	p_hif_data->p_hif_heap = heap;

	hif_module_init
	( 
		p_hif_data,
		hif_service_notify,
		&driver 
	);
#if APP_LBR_ROUTER        
        /*calling one-shot timer for hif*//*Umesh 17-sep-2018*/
        tmr_create_one_shot_timer
        (
            &(uart_debug),
            180000,//200000, //code working on 180000 for 2048 pkt,from cal culation timer value is set to 213333. 
            (sw_tmr_cb_t)&clear_buffer_and_state,
            NULL//&uart_hal_info
        );
#endif
}

/*----------------------------------------------------------------------------*/

void hif_task_init(void) //To be called from main after hif_service_init() call
{
	//create task
}

/*----------------------------------------------------------------------------*/

void hif_task(hif_t* p_hif)
{
	hif_buff_t* pBuffer = NULL;
	pBuffer = ( hif_buff_t* )queue_peek( &( p_hif->recv_q ));
	if ( NULL != pBuffer )
	{
		/*Packet recieved*/
		hif_proc_packet_from_host( p_hif );
	}

	pBuffer = ( hif_buff_t* )queue_peek( &( p_hif->send_q ));
	
	if ( NULL != pBuffer )
	{
                /*Packet Transmitt*/
		hif_send_packet_to_host( p_hif );
	}

}
/*----------------------------------------------------------------------------*/

void ReceiveProcess(void)
{
	uint16_t queueCnt = 0;
	hif_buff_t* pBuffer = NULL;
	pBuffer = ( hif_buff_t* )queue_peek( &( mp_hif->recv_q ));
        queueCnt = queue_count_get( &( mp_hif->recv_q ));
	if ( NULL != pBuffer )
	{
		/*Packet recieved*/
		hif_proc_packet_from_host( mp_hif );
                // Raka  :: As per current logic this condition should never fail 
                // and we should not get into else  anytime. If we are going to else
                // we need to relook into this condition.
                if (queueCnt == 1)
                {
#if ((RADIO_VALIDATION ==1 )|| ( SNIFFER == 1) || (APP_HIF_PROCESS_FEATURE_ENABLED == 0))          
                  event_clear(HIF_RX_EVENT);
#endif
                }
                else
                {
                  rx_buff_wrong_queue_cnt++;
                }
                
	}
#if (RADIO_VALIDATION || SNIFFER)     
	else
	{
		event_clear(HIF_RX_EVENT);
	}
#endif
}
/*----------------------------------------------------------------------------*/

void TransmitProcess(void)
{
  uint16_t queueCnt = 0;
  hif_buff_t* pBuffer = NULL;
  queueCnt = queue_count_get( &( mp_hif->send_q ));
  
/* Debdeep :: This do-while loop was implemented becouse tx packet was being accumulated in mp_hif->send_q.
But if uart driver is busy transmitting a previous packet then multiple hif tx event is posted by this do-while loop.
Now we need to see why tx packet was being accumulted in mp_hif->send_q previously */
//  do
//  {
  
  if (queueCnt <= 0)
  {
#if ((RADIO_VALIDATION ==1 )|| ( SNIFFER == 1) || (APP_HIF_PROCESS_FEATURE_ENABLED == 0))
    event_clear(HIF_TX_EVENT);
#endif
    return;
  }
  
  pBuffer = ( hif_buff_t* )queue_peek( &( mp_hif->send_q ));
  if ( NULL != pBuffer )
  {
    /*Packet transmitted*/
    hif_send_packet_to_host( mp_hif );
  }
  else
  {
#if ((RADIO_VALIDATION ==1 )|| ( SNIFFER == 1) || (APP_HIF_PROCESS_FEATURE_ENABLED == 0))
    event_clear(HIF_TX_EVENT);
#endif
    mp_hif->send_q.count = 0;
  }

//    queueCnt = queue_count_get( &( mp_hif->send_q ));
//  }while (queueCnt != 0);
}

/*
** ============================================================================
** Private Function Definitions
** ============================================================================
*/ 


static void hif_service_notify( uint8_t evt )
{
  if (evt == RX_PACKET_EVENT)
  {
#if (((RADIO_VALIDATION == 1)|| (SNIFFER == 1) ) || (APP_HIF_PROCESS_FEATURE_ENABLED == 0))
    event_set(HIF_RX_EVENT);
#else
    hif_process_rx (NULL);
#endif
  }
}

/*----------------------------------------------------------------------------*/

