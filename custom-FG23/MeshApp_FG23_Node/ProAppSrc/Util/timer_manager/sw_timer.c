/** \file sw_timer.c
 *******************************************************************************
 ** \brief Implements a software timer functionality
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

/*
** =============================================================================
** Private Macro definitions
** =============================================================================
*/
	
/* Indicates software timer expiry event */
//#define SW_TMR_EXPIRE_EVENT    SST_HIGHEST_PRIO_EVENT

/* Indicates software timer ISR request event */
//#define SW_TMR_ISR_REQ_EVENT   0x02

/* Indicates the minimum difference allowed between expiry time and the current time */
#define MIN_DIFFERENCE 15

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

/* None */

/*
** =============================================================================
** Private Function Prototypes
** =============================================================================
*/

/* None */

/*
** =============================================================================
** Private Function Definitions
** =============================================================================
*/

/* None */

/*
** =============================================================================
** External Variable Declarations
** =============================================================================
*/

#ifndef EFM32_TARGET_IAR
extern volatile uint16_t system_time;
#endif

/*
** =============================================================================
** External Function Prototypes
** =============================================================================
*/

/* None */

/*
** =============================================================================
** Public Variables Definitions
** =============================================================================
**/

uint16_t a_big_problem = 0;
sw_tmr_module_t* gptmr_mod_ins = {0};

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

void sw_tmr_init( sw_tmr_module_t *pTmr_mod_ins )
{ 
        gptmr_mod_ins = pTmr_mod_ins;
	p3list_init( &pTmr_mod_ins->tmr_active_list );    
	p3list_init( &pTmr_mod_ins->tmr_exp_list );
	p3list_init( &pTmr_mod_ins->tmr_pend_list );
	p3list_init( &pTmr_mod_ins->isr_req_q );
        pTmr_mod_ins->hw_timer_if.hw_tmr_init( pTmr_mod_ins->hw_timer_if.pHw_tmr_ins, pTmr_mod_ins );

    return;
}
/*----------------------------------------------------------------------------*/
//Why sw_tmr_module_t *pTmr_mod_ins parameter is recieved when we are not using 
//it in this function. 
bool sw_tmr_create( sw_tmr_t *pTmr_ins, stime_t period, sw_tmr_cb_t cb, void* param )
{
    pTmr_ins->period = period;
    pTmr_ins->cb = cb;
    pTmr_ins->cb_param = param;
    pTmr_ins->type = SW_TMR_ONESHOT;
    pTmr_ins->state = SW_TMR_CREATED;
    return TRUE;
}
/*----------------------------------------------------------------------------*/
void handle_system_time_rollover( sw_tmr_module_t *pTmr_mod_ins )
{
	sw_tmr_t *pTmr_cur_ins = NULL;

	/*All active timers will be expired by now. So copy active Q into Expired Q */
	list_concat(&(pTmr_mod_ins->tmr_exp_list),&(pTmr_mod_ins->tmr_active_list) );
	pTmr_cur_ins = ( sw_tmr_t* )list_entry_get_start( &pTmr_mod_ins->tmr_exp_list );
	
	while((NULL != pTmr_cur_ins) /*&& (((pTmr_cur_ins->exp_time >> 16) == 0x0000FFFF))*/)
	{		
            /*if the HIGH part of the expiry time is same as */
            pTmr_cur_ins->state = SW_TMR_EXPIRED;
            pTmr_cur_ins = ( sw_tmr_t* )list_entry_scan_next( &pTmr_mod_ins->tmr_exp_list, ( list_item_t* )pTmr_cur_ins );
	}
	
	list_entry_set_start( &pTmr_mod_ins->tmr_active_list,( list_item_t* )pTmr_cur_ins );	
	list_entry_set_start( &pTmr_mod_ins->tmr_active_list,( list_item_t* )pTmr_cur_ins );	
	list_concat(&(pTmr_mod_ins->tmr_active_list),&(pTmr_mod_ins->tmr_pend_list) );
	list_entry_set_start( &pTmr_mod_ins->tmr_pend_list, NULL );

}
/*----------------------------------------------------------------------------*/
p3time_t sw_current_time_get( sw_tmr_module_t *pTmr_mod_ins )
{
    return pTmr_mod_ins->hw_timer_if.hw_tmr_get_time( pTmr_mod_ins->hw_timer_if.pHw_tmr_ins);
}
/*----------------------------------------------------------------------------*/
uint8_t sw_tmr_rand_get( sw_tmr_module_t *pTmr_mod_ins )
{
    return pTmr_mod_ins->hw_timer_if.hw_tmr_rand( pTmr_mod_ins->hw_timer_if.pHw_tmr_ins);
}
/*----------------------------------------------------------------------------*/
void sw_tmr_delay( sw_tmr_module_t *pTmr_mod_ins, uint32_t duration_tks )
{
     pTmr_mod_ins->hw_timer_if.hw_tmr_delay( pTmr_mod_ins->hw_timer_if.pHw_tmr_ins, duration_tks);
}
/*----------------------------------------------------------------------------*/
void sw_tmr_stop( sw_tmr_module_t *pTmr_mod_ins, sw_tmr_t *pTmr_ins )
{
    list_item_t* next_tmr;       
    if( list_entry_is_start( &pTmr_mod_ins->tmr_active_list, ( list_item_t* )pTmr_ins ))
    {
        pTmr_mod_ins->hw_timer_if.hw_tmr_stop(  pTmr_mod_ins->hw_timer_if.pHw_tmr_ins );        
        next_tmr = list_entry_scan_next	( &pTmr_mod_ins->tmr_active_list, ( list_item_t* )pTmr_ins );
            		
        if( NULL != next_tmr )
        {
            //printf("Place1\n\r");
            pTmr_mod_ins->hw_timer_if.hw_tmr_start ( pTmr_mod_ins->hw_timer_if.pHw_tmr_ins, pTmr_ins->exp_time );
        }
    }    
    list_entry_delete( &pTmr_mod_ins->tmr_active_list, (list_item_t*)pTmr_ins );
    list_entry_delete( &pTmr_mod_ins->tmr_pend_list, (list_item_t*)pTmr_ins );
    return;
}
/*----------------------------------------------------------------------------*/
void sw_tmr_add_to_active_list( sw_tmr_module_t *pTmr_mod_ins, sw_tmr_t *timer,uint8_t add_to_pend )
{
    sw_tmr_t* pTmr_cur, *pTmr_prev;
    p3_list_t* p_list = (add_to_pend)?&(pTmr_mod_ins->tmr_pend_list ):&(pTmr_mod_ins->tmr_active_list);
	
    /* Stop the timer if it is alredy running */
    if( timer->state == SW_TMR_ACTIVE )
        sw_tmr_stop( pTmr_mod_ins, timer );
    else if( timer->state == SW_TMR_EXPIRED )
        list_entry_delete( &pTmr_mod_ins->tmr_exp_list, (list_item_t*)timer );

    /* Adding the timer into the timer link list at an appropriate location */
    pTmr_cur = pTmr_prev = ( sw_tmr_t* )list_entry_get_start( p_list );

    while( NULL != pTmr_cur )
    {
        if( pTmr_cur->exp_time > timer->exp_time )
          break;
        pTmr_prev = pTmr_cur;
        pTmr_cur = ( sw_tmr_t* )list_entry_scan_next( p_list, ( list_item_t* )pTmr_cur );	
    }
    timer->state = SW_TMR_ACTIVE;
    /* The new timer is the first to expire, so need to be put at the head */
    if( pTmr_prev == pTmr_cur )
    {
        p3list_entry_add_start( p_list, ( list_item_t* )timer );	    
        if ( add_to_pend == 0 )
        {
        /* Stop the timer and start the timer with the new expire time */
            if( NULL != pTmr_cur )
            {
                    pTmr_mod_ins->hw_timer_if.hw_tmr_stop(pTmr_mod_ins->hw_timer_if.pHw_tmr_ins );
            }
            //printf("Place3\n\r");    
            pTmr_mod_ins->hw_timer_if.hw_tmr_start(pTmr_mod_ins->hw_timer_if.pHw_tmr_ins, timer->exp_time );
        }
    }
    else /* If it is at the end or in between the list */
    {
        if( (( list_item_t* )timer)  != (( list_item_t* )pTmr_prev))
        {
        	list_entry_add_after( p_list, ( list_item_t* )timer, ( list_item_t* )pTmr_prev );	    	     
        }    	
    }                
}
/*----------------------------------------------------------------------------*/
void update_expired_tmr_list(sw_tmr_module_t *pTmr_mod_ins)
{
	sw_tmr_t *pTmr_prv_ins = NULL;
	p3time_t current_time;
	p3time_t diff;

	sw_tmr_t *pTmr_cur_ins = ( sw_tmr_t* )list_entry_get_start( &pTmr_mod_ins->tmr_active_list );	
	list_concat(&(pTmr_mod_ins->tmr_exp_list),&(pTmr_mod_ins->tmr_active_list) );
	
	while( NULL != pTmr_cur_ins )
	{  
          current_time = pTmr_mod_ins->hw_timer_if.hw_tmr_get_time(pTmr_mod_ins->hw_timer_if.pHw_tmr_ins ); 
          /* Find all expired timer. A timer is expired if the current  
          time is greater or less than the timer expire time. */
          if( pTmr_cur_ins->exp_time <= current_time )
          {
              pTmr_cur_ins->state = SW_TMR_EXPIRED;
              pTmr_prv_ins = pTmr_cur_ins;
              pTmr_cur_ins = ( sw_tmr_t* )list_entry_scan_next( &pTmr_mod_ins->tmr_active_list, ( list_item_t* )pTmr_cur_ins );
          }
          else
          {
              /* Start the timer with the next expire time     NextLocation->expireTime */
              //printf("Place2\n\r");      
              /*may come here as a result of high time (system_time) overflow 
              while timer0 channel is running. So do not again start the same timer.*/
              if( ((hw_tmr_t*)pTmr_mod_ins->hw_timer_if.pHw_tmr_ins)->low_time_us )
              {
                 return;				
              }
              current_time = pTmr_mod_ins->hw_timer_if.hw_tmr_get_time(pTmr_mod_ins->hw_timer_if.pHw_tmr_ins ); 
              diff = pTmr_cur_ins->exp_time - current_time;
              if ( pTmr_cur_ins->exp_time <= current_time )
              {
                  diff = 0;
              }
              
              if( diff >= MIN_DIFFERENCE )
              {
                  /* NULL check is not needed, but good to have a safe check */	    
                  if( NULL != pTmr_prv_ins )
                  {
                      /* Terminate the expire link list */										   
                      pTmr_prv_ins->next = NULL;					  
                  }
                  pTmr_mod_ins->hw_timer_if.hw_tmr_start( pTmr_mod_ins->hw_timer_if.pHw_tmr_ins, pTmr_cur_ins->exp_time );
                  break;
              }
              else
              {				
                  pTmr_cur_ins->state = SW_TMR_EXPIRED;
                  pTmr_prv_ins = pTmr_cur_ins;
                  pTmr_cur_ins = ( sw_tmr_t* )list_entry_scan_next( &pTmr_mod_ins->tmr_active_list, ( list_item_t* )pTmr_cur_ins );
              }	                  
          }
	}
	/* Detach expired timer list from the the active timer list */
	list_entry_set_start( &pTmr_mod_ins->tmr_active_list, ( list_item_t* )pTmr_cur_ins );	
}
/*----------------------------------------------------------------------------*/
void invoke_expired_timer_cbs(sw_tmr_module_t *pTmr_mod_ins)
{
    sw_tmr_t *pTmr_prv_ins = NULL;
    p3time_t tmp = 0;
    //irq_state_t flags = irq_disable();
    
    sw_tmr_t *pTmr_cur_ins = ( sw_tmr_t* )list_entry_get_start( &pTmr_mod_ins->tmr_exp_list );
    /* If any timer expired then call the cb of the expired timer one by one */
    while( NULL != pTmr_cur_ins )
    {
        pTmr_prv_ins = pTmr_cur_ins;
        pTmr_cur_ins = ( sw_tmr_t* )list_entry_scan_next( &(pTmr_mod_ins->tmr_exp_list),( list_item_t* )pTmr_cur_ins );
        if( pTmr_prv_ins->state == SW_TMR_EXPIRED )
        {
            //irq_enable( flags );          
	    /* Call the CB of the expired soft timer */
            if( pTmr_prv_ins->cb != NULL )
            {
              pTmr_prv_ins->cb( pTmr_prv_ins->cb_param, pTmr_prv_ins );
            }
            else            
            {
              a_big_problem++;
            } 	    
	    //flags = irq_disable(); 
         }
        
        /* If it is a periodic timer then start it again by adding it to 
        the active timer list. If the periodic timer is stopped in the cb, 
        then the timer type is marked as stopped ,
        so the timer is not started again below */
        if( ( SW_TMR_PERIODIC == pTmr_prv_ins->type ) &&  ( SW_TMR_STOPPED != pTmr_prv_ins->state ) )
        {   
            	tmp = pTmr_prv_ins->exp_time;
		pTmr_prv_ins->exp_time = tmp + pTmr_prv_ins->period;
		/*TBD: may be it is good to have a check before deciding 
		to add the timer into the pending Q. After the summation, 
		if the sum is lesser  than the value what it held previosuly 
		before adding the period, then we are deciding it to add into the pending Q. 
		Do we also have to make sure that the sum is lesser than the 
		current time to indicate that it should be put into pending state???? */
		sw_tmr_add_to_active_list( pTmr_mod_ins, pTmr_prv_ins,(pTmr_prv_ins->exp_time < tmp)?1:0);
        }              
    }
     list_entry_set_start( &(pTmr_mod_ins->tmr_exp_list), NULL );     
    //irq_enable( flags );
}
/*----------------------------------------------------------------------------*/

stime_t sw_tmr_remaining_time_get( sw_tmr_module_t *pTmr_mod_ins, sw_tmr_t *pTmr_ins )
{
    stime_t remaing_time;
    sw_tmr_stime_sub( remaing_time, pTmr_ins->exp_time, pTmr_mod_ins->hw_timer_if.hw_tmr_get_time(pTmr_mod_ins->hw_timer_if.pHw_tmr_ins ));
    return remaing_time;
}

/*----------------------------------------------------------------------------*/
stime_t sw_tmr_get_min_of_all_remaining_time( sw_tmr_module_t *pTmr_mod_ins )
{
    stime_t remaing_time;
    sw_tmr_t* pTmr = ( sw_tmr_t* )( list_entry_get_start( &pTmr_mod_ins->tmr_active_list ));
    sw_tmr_stime_sub( remaing_time, pTmr->exp_time, pTmr_mod_ins->hw_timer_if.hw_tmr_get_time( pTmr_mod_ins->hw_timer_if.pHw_tmr_ins ));
    return remaing_time;
}
/*----------------------------------------------------------------------------*/
bool sw_tmr_is_active( sw_tmr_t *pTmr_ins )
{
    return ( pTmr_ins->state == SW_TMR_ACTIVE );
}
/*----------------------------------------------------------------------------*/
stime_t sw_tmr_period_get( sw_tmr_module_t *pTmr_mod_ins, sw_tmr_t *pTmr_ins )
{
    return pTmr_ins->period;
}
/*----------------------------------------------------------------------------*/
void sw_tmr_time_update_req ( sw_tmr_module_t *pTmr_mod_ins, sw_tmr_cb_t cb, void *param)
{

}
/*----------------------------------------------------------------------------*/
void sw_tmr_deinit( sw_tmr_module_t *pTmr_mod_ins )
{
  
}
/*----------------------------------------------------------------------------*/
uint32_t  sw_tmr_get_sys_time_high( sw_tmr_module_t *pTmr_mod_ins )
{
#if defined (EFM32_TARGET_IAR)
    return pTmr_mod_ins->hw_timer_if.hw_tmr_get_time( pTmr_mod_ins->hw_timer_if.pHw_tmr_ins);
#else
     return system_time; 
#endif  
}
/*----------------------------------------------------------------------------*/


