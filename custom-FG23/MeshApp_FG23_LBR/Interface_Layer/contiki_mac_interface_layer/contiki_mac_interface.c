/** \file fan_app_main.c
 *******************************************************************************
 ** \brief this is main file for application
 **
 ** \cond STD_FILE_HEADER
 **
 ** COPYRIGHT(c) 2010-11 Procubed Technology Solutions Pvt Ltd. 
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

#include "StackAppConf.h"
#include "common.h"
#include "l3_configuration.h"
#include "contiki-net.h"
#include "fan_mac_ie.h"
#include "mac_interface_layer.h"
#include "buffer_service.h"
#include "queue_latest.h"

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

enum
{
  MAC_POLL
};

enum
{
  MIL_POLL
};

/*
** =============================================================================
** Private Variable Definitions
** =============================================================================
*/

/* None */

/*
** =============================================================================
** Private Function Prototypes
** =============================================================================
*/

/* None */

/*
** =============================================================================
** External Variable Declarations
** =============================================================================
*/

/* None */

/*
** =============================================================================
** External Function Prototypes
** =============================================================================
*/

L3_PROCESS(promac_process, "MAC-process");
//PROCESS(mil_process,"MAC Interface Layer process");

extern void tmr_service_init(void);

/*
** =============================================================================
** Public Variables Definitions
** =============================================================================
**/

/* None */

/*
** =============================================================================
** Public Function Prototypes
** =============================================================================
*/

/* None */


/*
** =============================================================================
** Private Function Definitions
** =============================================================================
*/


//static void mac_poll( void )
//{
//    process_poll(&promac_process);
//}
/******************************************************************************/
static void post_mac_event( uint8_t event )
{
    l3_process_post(&promac_process, (l3_process_event_t)event, NULL); 
}
/******************************************************************************/
//static void post_mil_event( uint8_t event )
//{
//    l3_process_post(&mil_process, (process_event_t)event, NULL); 
//}
/******************************************************************************/  
//static void mil_poll( void )
//{
//    //l3_process_post(&mil_process, MIL_POLL, NULL);
//    process_poll(&mil_process);
//    //l3_process_post(&mil_process, (process_event_t)event, NULL); 
//}
/******************************************************************************/
static void mac_eventhandler(l3_process_event_t ev, l3_process_data_t data)
{
    switch(ev) {
      /* This is the event we get if a process has exited. Anything we need to do 
      here for proper exit( cleaning up resources). */
      case L3_PROCESS_EVENT_EXITED:
      break;
      
      case L3_PROCESS_EVENT_POLL:
      //     case MAC_POLL:
      //       
      //     break;

      default:
      //num_events_processed++;
      MIL_Task();
      break;
    }
}
/******************************************************************************/
//static void mil_eventhandler(process_event_t ev, process_data_t data)
//{
//    switch(ev) {
//      /* This is the event we get if a process has exited. Anything we need to do 
//      here for proper exit( cleaning up resources). */
//      case PROCESS_EVENT_EXITED:
//      break;
//      
//      case PROCESS_EVENT_POLL:
//      case MIL_POLL:
//      process_mil_msgs();
//      break;
//
//      default:
//      process_mil_msgs();
//      break;
//    }
//}

/******************************************************************************/

/*
** =============================================================================
** Public Function Definitions
** =============================================================================
*/

L3_PROCESS_THREAD(promac_process, ev, data)
{
    L3_PROCESS_BEGIN();

    tmr_service_init();
    MIL_Init(1);

    while(1) {
      L3_PROCESS_YIELD();
      mac_eventhandler(ev, data);
      //PROCESS_YIELD_UNTIL(ev == PROCESS_EVENT_POLL);
      //MIL_Task();
    }

    L3_PROCESS_END();
}

/******************************************************************************/
//PROCESS_THREAD(mil_process, ev, data)
//{
//    PROCESS_BEGIN();
//
//    while(1) {
//      PROCESS_YIELD();
//      mil_eventhandler(ev, data);
//      // PROCESS_YIELD_UNTIL(ev == PROCESS_EVENT_POLL);
//      //process_mil_msgs();
//    }
//
//    PROCESS_END();
//}

/******************************************************************************/
void signal_event_to_mac_task( uint8_t event )
{
    post_mac_event(event);
}
/******************************************************************************/
//void indicate_mac_2_nhle( uint8_t prio )
//{
//    /*enable the following code only in mac_test and not in any apps running 
//    over MIL
////    event_set(MAC_2_NHLE_EVENT);
//    signal_event_to_mac_task();*/
//    mil_poll();
//    //l3_process_post(&promac_process, MAC_POLL, NULL);
//}
/******************************************************************************/