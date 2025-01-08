/** \file pas_tmr.c
 *******************************************************************************
 ** \brief Implements a software timer functionality
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
/*******************************************************************************
* File inclusion
*******************************************************************************/
#include "StackMACConf.h"
#include "common.h"
#include "queue_latest.h"
#include "list_latest.h"
#include "hw_tmr.h"
#include "sw_timer.h"
#include "tri_tmr.h"
#include "stdlib.h"
#include "timer_service.h"
#include "phy.h"
#include "fan_config_param.h"

#if RAIL_TIMER_INTERFACE_USED
#include "rail_timer_interface.h"
#endif

/*FAN TPS 1v10:6.3.4.6.2:Usage of Trickle Timers*/
#define REDUNDANCY_CONST_PC             APP_CFG_DISC_K
#define PAN_CONFIG                      0x02
#define RESET_TRICKLE_ON_STARTUP        0   /*1 = Trickle timer will start with I = rand (Imin/2, Imin)
                                              0 = Trickle timer will start with I = rand (Imin, Imax) */
extern uint8_t trickle_IMIN;
extern uint8_t trickle_IMAX;

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
** ============================================================================
** Private Variable Definitions
** ============================================================================
*/

static uint32_t period_send = 0x00;
#if RAIL_TIMER_INTERFACE_USED
RAIL_MultiTimer_t sw_tri_tmr_pc;
#else
static sw_tmr_t sw_tri_tmr_pc;//Software timer  structure type 
#endif
static trickle_tmr sw_pc_send;//trickle timer structure type
static clock_tick loc_clock; /* A local, general-purpose placeholder */
static uint8_t dont_go_consistency = 0;
uint8_t pc_consistency = 0;

/*
** =============================================================================
** Public Variable Definitions
** =============================================================================
**/

fan_pkt_tmr_t pc_timer={0}; 

/*
** ============================================================================
** External Variable Declarations
** ============================================================================
*/

/*
** ============================================================================
** External Function Prototypes
** ============================================================================
*/

extern void send_trickle_timer_pkt(fan_pkt_tmr_t trickle_timer);
extern void tmr_service_init(void);

#if(FAN_EDFE_FEATURE_ENABLED == 1)
extern uint8_t is_edfe_enable();
#endif

extern uint8_t get_node_type( void );
extern uint8_t gtk_hash_need_to_be_updated ();
/*
** ============================================================================
** Private Function Prototypes
** ============================================================================
*/

#if RAIL_TIMER_INTERFACE_USED
static void fire(struct RAIL_MultiTimer *tmr,
                 RAIL_Time_t expectedTimeOfEvent,
                 void *cbArg );
static void double_interval(struct RAIL_MultiTimer *tmr,
                            RAIL_Time_t expectedTimeOfEvent,
                            void *cbArg );
#else
static void fire(void* context, void  *pTmr );
static void double_interval(void* context, void  *pTmr );
#endif


/*
** ============================================================================
** Private Function Definitions
** ============================================================================
*/



/*---------------------------------------------------------------------------*/
static clock_tick get_t(clock_tick i_cur)
{
  i_cur >>= 1;
  srand (timer_current_time_get ());  
  return (clock_tick)(i_cur + ((clock_tick)rand() % i_cur));
}
/*----------------------------------------------------------------------------*/
#if RAIL_TIMER_INTERFACE_USED
static void double_interval(struct RAIL_MultiTimer *tmr,
                            RAIL_Time_t expectedTimeOfEvent,
                            void *cbArg )
#else
static void double_interval(void* context, void  *pTmr )
#endif
{
    clock_tick last_end = sw_pc_send.i_start + sw_pc_send.i_cur;
    sw_pc_send.c = 0;

    /*When the interval I expires, Trickle doubles the interval length.
    If this new interval length would be longer than the time
    specified by Imax, Trickle sets the interval length I to be the
    time specified by Imax*/
    if (sw_pc_send.i_cur <= sw_pc_send.i_max_abs >> 1) 
      sw_pc_send.i_cur <<= 1;
    else 
      sw_pc_send.i_cur = sw_pc_send.i_max_abs;
    
    /* Random t in [I/2, I) */
    loc_clock = get_t (sw_pc_send.i_cur);
#if RAIL_TIMER_INTERFACE_USED
    clock_tick current_time = get_currunt_time_from_rail() / 1000000;
#else  
    clock_tick current_time = get_time_now_64 () / 1000000;
#endif 
    loc_clock = (last_end + loc_clock) - current_time;
    if(loc_clock > (TRICKLE_TIMER_CLOCK_MAX >> 1)) 
    {
      /* Oops, that's in the past */
      loc_clock = 0;
    }
    period_send = (uint32_t)(loc_clock * 1000000);

#if RAIL_TIMER_INTERFACE_USED
     /*Stop the software timer */
    RAIL_CancelMultiTimer(&sw_tri_tmr_pc); 
    RAIL_SetMultiTimer(&sw_tri_tmr_pc,period_send,RAIL_TIME_DELAY,&fire,NULL);
#else  
     /*Stop the software timer */
    tmr_stop(&sw_tri_tmr_pc);
    sw_tri_tmr_pc.period = period_send;
    sw_tri_tmr_pc.cb = fire;
    /*Start the software timer */
    tmr_start_relative (&sw_tri_tmr_pc);
#endif     

    sw_pc_send.i_start = last_end;
}
/*----------------------------------------------------------------------------*/
static void schedule_for_end(void)
{
#if RAIL_TIMER_INTERFACE_USED
  clock_tick now = get_currunt_time_from_rail()/ 1000000;
#else  
  clock_tick now = get_time_now_64 () / 1000000;
#endif 
  loc_clock = (sw_pc_send.i_start + sw_pc_send.i_cur) - now;
  /* Interval's end will happen in loc_clock ticks. Make sure this isn't in
  * the past... */
  if (loc_clock > (TRICKLE_TIMER_CLOCK_MAX >> 1)) 
  {
    loc_clock = 0; /* Interval ended in the past, schedule for in 0 */
  }
  /*After expire fire call back set double interval callback to double the inte-
  rval of timer*/
  period_send = (uint32_t)(loc_clock * 1000000);
#if RAIL_TIMER_INTERFACE_USED
  RAIL_CancelMultiTimer(&sw_tri_tmr_pc);
  RAIL_SetMultiTimer(&sw_tri_tmr_pc,period_send,RAIL_TIME_DELAY,&double_interval,NULL);
#else  
  tmr_stop (&sw_tri_tmr_pc);
  sw_tri_tmr_pc.cb = double_interval;
  sw_tri_tmr_pc.period = period_send;
  tmr_start_relative (&sw_tri_tmr_pc);
#endif    
}
/*----------------------------------------------------------------------------*/
static void new_interval(void)
{ 
  /* When an interval begins, Trickle resets c to 0 and sets t to a random po-
  int in the interval, taken from the range [I/2, I), that is, values greater 
  than or equal to I/2 and less than I. The interval ends at I.*/
  sw_pc_send.c = 0;
  /* Random t in [I/2, I)  */
  loc_clock = get_t (sw_pc_send.i_cur);
  period_send = loc_clock*1000000;

#if RAIL_TIMER_INTERFACE_USED
  RAIL_SetMultiTimer(&sw_tri_tmr_pc,period_send,RAIL_TIME_DELAY,&fire,NULL);
#else  
  /*create one time software send fire call back */
  tmr_create_one_shot_timer (&sw_tri_tmr_pc, period_send, fire, NULL);
/*Start Software timer */
  tmr_start_relative (&sw_tri_tmr_pc);
#endif
  /* Store the actual interval start (absolute time), we need it later */
#if RAIL_TIMER_INTERFACE_USED
  sw_pc_send.i_start = get_currunt_time_from_rail() / 1000000;
#else
  sw_pc_send.i_startt = get_time_now_64 () / 1000000;
#endif    
}
/*----------------------------------------------------------------------------*/
//void kill_process_and_clean_rpl_nbr();
#if RAIL_TIMER_INTERFACE_USED
static void fire(struct RAIL_MultiTimer *tmr,
                 RAIL_Time_t expectedTimeOfEvent,
                 void *cbArg )
#else
static void fire(void* context, void  *pTmr )
#endif
{  
//  kill_process_and_clean_rpl_nbr ();
//  return;
  
  /*At time t, Trickle transmits if and only if the counter c is less than the
  redundancy constant k.*/
  /*Call tx callback */
  if ((sw_pc_send.k == TRICKLE_TIMER_INFINITE_REDUNDANCY/*0x00*/) || \
    (sw_pc_send.c < sw_pc_send.k))
  {
    pc_timer.frame_type = PAN_CONFIG;
    
#if(FAN_EDFE_FEATURE_ENABLED == 1)
    if (!is_edfe_enable())
    {
#endif
      
#if (PRINT_DEBUG_LEVEL == CONSOLE_ERROR_DEBUG)
      stack_print_debug ("Trig PC\n");
#endif
      if ((get_node_type () == 0x01) /*&& (gtk_hash_need_to_be_updated () == 0)*/)
        send_trickle_timer_pkt(pc_timer); 
      if (get_node_type () == 0x00)
        send_trickle_timer_pkt(pc_timer);
      
      if (dont_go_consistency > 0)
        dont_go_consistency--;
      if(pc_consistency == 10)
        pc_consistency = 0;
//    }
  } 
  if ((sw_pc_send.i_cur != TRICKLE_TIMER_IS_STOPPED/*0*/)) 
  {
    schedule_for_end();
  }
}
/*----------------------------------------------------------------------------*/
       
/*
** ============================================================================
** Public Function Definitions
** ============================================================================
*/


void trickle_timer_config_pc_send (void)
{
  sw_pc_send.i_min = trickle_IMIN;
  sw_pc_send.i_max = trickle_IMAX;
  /*When the algorithm starts execution, it sets I to a value in the range of 
  [Imin, Imax] -- that is, greater than or equal to Imin and less than or equ-
  al to Imax.  The algorithm then begins the first interval. */
  sw_pc_send.i_max_abs = trickle_IMIN << trickle_IMAX;
  sw_pc_send.k = REDUNDANCY_CONST_PC/*1*/;
  /* Random I in [Imin , Imax] */
  srand (0);
#if RESET_TRICKLE_ON_STARTUP
  sw_pc_send.i_cur = sw_pc_send.i_min;
#else
  sw_pc_send.i_cur = sw_pc_send.i_min +
                      (rand() % 
                       ((sw_pc_send.i_max_abs - sw_pc_send.i_min) + 1));
#endif
  new_interval();
}
/*----------------------------------------------------------------------------*/

void trickle_timer_consistency_pc(void)
{
  if (dont_go_consistency > 0)
    return;
  if(pc_consistency == 10)
    return;
  
  /*Whenever Trickle hears a transmission that is "consistent", it increments 
  the counter c.*/
  if(sw_pc_send.c < 0xFF) 
  {
#if (PRINT_DEBUG_LEVEL == CONSOLE_ERROR_DEBUG)
    stack_print_debug ("Suppress PC\n");
#endif
    sw_pc_send.c++;
  }
}
/*----------------------------------------------------------------------------*/
void trickle_timer_send_pc_pkt_immediately()
{
  pc_timer.frame_type = PAN_CONFIG;
  send_trickle_timer_pkt(pc_timer);
}
/*----------------------------------------------------------------------------*/
void throttle_pc (void)
{
  dont_go_consistency = 5;
}

/*----------------------------------------------------------------------------*/
void
trickle_timer_inconsistency_pc(void)
{
  if (sw_pc_send.i_cur == TRICKLE_TIMER_IS_STOPPED)
    return;
  
  /* "If I is equal to Imin when Trickle hears an "inconsistent" transmission,
   * Trickle does nothing." */
  if(sw_pc_send.i_cur != sw_pc_send.i_min) 
  {
#if RAIL_TIMER_INTERFACE_USED
    RAIL_CancelMultiTimer(&sw_tri_tmr_pc);
#else    
    tmr_stop (&sw_tri_tmr_pc);
#endif      

    sw_pc_send.i_cur = sw_pc_send.i_min;
    sw_pc_send.c = 0;
    /* Random t in [I/2, I)  */
    loc_clock = get_t (sw_pc_send.i_cur);
    period_send = (uint32_t)(loc_clock * 1000000);
#if RAIL_TIMER_INTERFACE_USED
    RAIL_SetMultiTimer(&sw_tri_tmr_pc,period_send,RAIL_TIME_DELAY,&fire,NULL);;
#else  
    sw_tri_tmr_pc.period = period_send;;
    sw_tri_tmr_pc.cb = fire;    //Debdeep :: 08-March-18
    /*Start Software timer */
    /*Start Software timer */
    tmr_start_relative(&sw_tri_tmr_pc);
#endif 
#if RAIL_TIMER_INTERFACE_USED   
    sw_pc_send.i_start = get_currunt_time_from_rail()/ 1000000;
#else    
    /* Store the actual interval start (absolute time), we need it later */
    sw_pc_send.i_start = get_time_now_64 () / 1000000;
#endif        
    dont_go_consistency++;
  }
}
/*----------------------------------------------------------------------------*/
void process_schedule_end_pc()
{
#if RAIL_TIMER_INTERFACE_USED
  RAIL_CancelMultiTimer(&sw_tri_tmr_pc);
#else  
  tmr_stop(&sw_tri_tmr_pc);
#endif    
  
  sw_pc_send.i_cur = TRICKLE_TIMER_IS_STOPPED;
  return;
}
/*----------------------------------------------------------------------------*/
void process_schedule_start_pc()
{
    trickle_timer_config_pc_send();
    return;
}
/*----------------------------------------------------------------------------*/
void update_pc_consistency(void)
{
  if(pc_consistency != 10)
  pc_consistency++;
}