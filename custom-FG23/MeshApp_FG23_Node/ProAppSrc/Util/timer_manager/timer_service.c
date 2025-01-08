/** \file timer_service.c
 *******************************************************************************
 ** \brief Implements the OS dependant part of the timer service
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
#include "list_latest.h"
#include "hw_tmr.h"
#include "sw_timer.h"
#include "timer_service.h"
#include "event_manager.h"

#if RAIL_TIMER_INTERFACE_USED
#include "rail_timer_interface.h"
#endif
/*
** =============================================================================
** Private Macro definitions
** =============================================================================
*/
	
/**< Indicates a software timer is just created  */
#define SW_TMR_CREATED 1

/**< Indicates that a software timer is active/running  */
#define SW_TMR_ACTIVE  2

/**< Indicates that a software timer is expired  */
#define SW_TMR_EXPIRED 3

/**< Indicates that a software timer is stopped */
#define SW_TMR_STOPPED 4

/**< Indicates that the software timer was requested to be stopped from timer 
*    interrupt context */
//#define SW_TMR_ISR_REQ_STOP  1

/**< Indicates that the software timer was requested to be start from timer 
*    interrupt context */
//#define SW_TMR_ISR_REQ_START 2

/**< Indicates software timer expiry event */
//#define SW_TMR_EXPIRE_EVENT    SST_HIGHEST_PRIO_EVENT

/**< Indicates software timer ISR request event */
//#define SW_TMR_ISR_REQ_EVENT   0x02

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

static sw_tmr_module_t tmr_mod_ins;
static hw_tmr_t hw_tmr_ins;
volatile unsigned char timer_trigger = 0;
volatile unsigned char sys_time_roll_over_event = 0;

/*
** =============================================================================
** Private Function Definitions
** =============================================================================
*/

static bool  tmr_start( sw_tmr_t *pTmr_ins, uint8_t add_to_pend );
//Sagar: Not Used
/*static void sw_tmr_thread_notif( sw_tmr_module_t *pTmr_mod_ins, base_t event, void *pParam );*/
static void timer_expiry_notify( sw_tmr_module_t *pTmr_mod_ins );

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

extern void handle_system_time_rollover( sw_tmr_module_t *pTmr_mod_ins );

/*
** =============================================================================
** Public Variables Definitions
** =============================================================================
**/

p3time_t system_time_high_32 = 0;
sw_tmr_module_t* gpTmr_mod_ins  = 0;

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

void tmr_service_init(void)
{

  gpTmr_mod_ins = &tmr_mod_ins;  
  hw_tmr_ins.tmr_channel = HW_TIMER_CHANNEL_0;        
  hw_tmr_ins.cb  = (cb_routine_t)timer_expiry_notify;
  hw_tmr_ins.ctx = ( void* )&tmr_mod_ins;
  hw_tmr_ins.timer_added_in_irq = 0x0;
  
  gpTmr_mod_ins->hw_timer_if.pHw_tmr_ins = &hw_tmr_ins;
  gpTmr_mod_ins->hw_timer_if.hw_tmr_get_time = hw_tmr_get_time;
  gpTmr_mod_ins->hw_timer_if.hw_tmr_init = hw_tmr_init;
  gpTmr_mod_ins->hw_timer_if.hw_tmr_start = hw_tmr_start;
  gpTmr_mod_ins->hw_timer_if.hw_tmr_stop = hw_tmr_stop;
  gpTmr_mod_ins->hw_timer_if.hw_tmr_delay = hw_tmr_delay;
  gpTmr_mod_ins->hw_timer_if.hw_tmr_rand = hw_tmr_rand;
    
  sw_tmr_init(gpTmr_mod_ins);
  
// create a thred in case of an OS being used
  return;
}

/******************************************************************************/

void timer_task( void )
{
	sw_tmr_t* p_sw_tmr = NULL;
// 	irq_state_t flags = __get_interrupt_state();//Umesh
//        flags = irq_disable();
    
	if( timer_trigger )
	{
		timer_trigger--;
		/*process roll over event if any, first*/
		if(sys_time_roll_over_event)
		{
			handle_system_time_rollover( gpTmr_mod_ins );
			sys_time_roll_over_event = 0;
		}
		
		update_expired_tmr_list(gpTmr_mod_ins);
	}
//	irq_enable( flags );	
	invoke_expired_timer_cbs(gpTmr_mod_ins);								  
	if( ( p_sw_tmr = (sw_tmr_t*)list_entry_get_start(&(gpTmr_mod_ins->isr_req_q))  ) != NULL )
	{
		
		/*timer is stopped start it again*/
		tmr_stop( p_sw_tmr);
		tmr_start_relative( p_sw_tmr );
                list_entry_set_start( &(gpTmr_mod_ins->isr_req_q), NULL );
		
	}
//	flags = irq_disable();	
	if(!timer_trigger)
	{
	    event_clear(TIMER_EXPIRY_EVENT);
	}
//	irq_enable( flags );		
	return;
}
/******************************************************************************/
void tmr_create_one_shot_timer( sw_tmr_t *pTmr_ins,stime_t period,sw_tmr_cb_t cb,void* param )
{
	sw_tmr_create( pTmr_ins, period, cb, param );
	return;
}
/******************************************************************************/
// Sagar: Not Used
/*
bool tmr_create_periodic_timer( sw_tmr_t *pTmr_ins,stime_t period,sw_tmr_cb_t cb,void* param )
{
	return sw_tmr_create( pTmr_ins, SW_TMR_PERIODIC, period, cb, param );
    
}
*/
/******************************************************************************/

bool  tmr_start_relative( sw_tmr_t *pTmr_ins )
{
  uint8_t add_to_pend = 0;
  
  p3time_t curr  = gpTmr_mod_ins->hw_timer_if.hw_tmr_get_time( gpTmr_mod_ins->hw_timer_if.pHw_tmr_ins );
  tmr_stop(pTmr_ins);		      
  if( pTmr_ins->state >= SW_TMR_CREATED )
  {
    pTmr_ins->exp_time = pTmr_ins->period + curr;
   
    if( pTmr_ins->exp_time < curr )
    {
      add_to_pend = 1;
    }
          
    return tmr_start( pTmr_ins,add_to_pend );
  }  
  return FALSE;
}

/******************************************************************************/

bool  tmr_start_absolute( sw_tmr_t *pTmr_ins, p3time_t point_in_time, stime_t period)
{
  uint8_t add_to_pend = 0;
  tmr_stop(pTmr_ins);
  if( pTmr_ins->state >= SW_TMR_CREATED )
  {
    pTmr_ins->exp_time = point_in_time + period;

	if( pTmr_ins->exp_time < point_in_time )
	{
		add_to_pend = 1;
	}
    return tmr_start( pTmr_ins,add_to_pend );   
  } 
  return FALSE;
}


/******************************************************************************/


void tmr_delay(uint32_t delay_tks)
{
    if ( delay_tks !=0 )
    {
          sw_tmr_delay(gpTmr_mod_ins,delay_tks);
    }
    return;
}

/******************************************************************************/

void  tmr_stop( sw_tmr_t *pTmr_ins )
{
    if( pTmr_ins != NULL )
    {
    	if( pTmr_ins->state != SW_TMR_ACTIVE )
        goto sw_tmr_stop_exit;
    	
    	sw_tmr_stop(gpTmr_mod_ins,pTmr_ins );
    }
    else 
    {
    	return;
    }
    sw_tmr_stop_exit:
        pTmr_ins->state = SW_TMR_STOPPED;
    //sw_tmr_stop_exit1:  // Raka
    return;
}

/******************************************************************************/

p3time_t timer_current_time_get(void)
{
	return sw_current_time_get( gpTmr_mod_ins );
}
/******************************************************************************/

p3time_t timer_current_time_get_high_32(void)
{
	return system_time_high_32;
}
/******************************************************************************/

uint64_t get_time_now_64 (void)
{
  uint64_t current_time = 0;
#if RAIL_TIMER_INTERFACE_USED
    current_time = get_currunt_time_from_rail();
#else
      current_time = (uint64_t)system_time_high_32 << 32;
  
  current_time = current_time|(uint32_t)sw_current_time_get( gpTmr_mod_ins );  
  
#endif  
  return current_time;
}

uint8_t timer_rand_get(void)
{
	return sw_tmr_rand_get( gpTmr_mod_ins );
}
/******************************************************************************/
// Sagar: Not Used
/*
static void 
sw_tmr_thread_notif( sw_tmr_module_t *pTmr_mod_ins, base_t event, 
                      void *pParam )
{
   
	queue_item_put( &pTmr_mod_ins->isr_req_q, pParam );
	//post the event to timer thread in case of OS being used

	return;
}
*/

/******************************************************************************/

//Sagar Not Used

stime_t tmr_get_remaining_time(sw_tmr_t *pTmr_ins)
{
      stime_t rem_time = 0;
      if(pTmr_ins != NULL)
      {
              rem_time = sw_tmr_remaining_time_get( gpTmr_mod_ins, pTmr_ins );
              return rem_time;
      }	
      return 0;
    
}


/******************************************************************************/

//Sagar Not Used
/*
stime_t tmr_get_min_of_all_rem_timer()
{	
	stime_t min_time = 0;
	if( gpTmr_mod_ins != NULL )
	{
		min_time = sw_tmr_get_min_of_all_remaining_time( gpTmr_mod_ins );
		return min_time;
	}
	return min_time;
}
*/
/******************************************************************************/

/*
** =============================================================================
** Private Function Prototypes
** =============================================================================
*/

static bool  tmr_start( sw_tmr_t *pTmr_ins, uint8_t add_to_pend )
{   
//    irq_state_t flags = __get_interrupt_state();//Umesh
//    flags = irq_disable();	
    sw_tmr_add_to_active_list( gpTmr_mod_ins, pTmr_ins	,add_to_pend );    
//    irq_enable(flags);

    return TRUE;
}

/******************************************************************************/
static void timer_expiry_notify( sw_tmr_module_t *pTmr_mod_ins )
{
  
#ifndef EFM32_TARGET_IAR
	if(!system_time)
          
#endif          
	{
            if( !(((hw_tmr_t*)(gpTmr_mod_ins->hw_timer_if.pHw_tmr_ins))->rollover_handled ) )
            {
                 sys_time_roll_over_event = 1;
                 system_time_high_32++;
                 ((hw_tmr_t*)(gpTmr_mod_ins->hw_timer_if.pHw_tmr_ins))->rollover_handled = 1;
            }
	}

        
	timer_trigger += 1;
	event_set(TIMER_EXPIRY_EVENT);
	//signal_event_to_mac_task();
}
/******************************************************************************/