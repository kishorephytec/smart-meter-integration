  /** \file pcs_tmr.c
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
#include "fan_config_param.h"

#if RAIL_TIMER_INTERFACE_USED
#include "rail_timer_interface.h"
#endif


/*
** =============================================================================
** Private Macro definitions
** =============================================================================
*/

#define MAX_ATTEMT_PCS_PKT       APP_CFG_PCS_MAX

/*FAN TPS 1v10:6.3.4.6.2:Usage of Trickle Timers*/
#define REDUNDANCY_CONST_PCS            APP_CFG_DISC_K
#define RESET_TRICKLE_ON_STARTUP        0   /*1 = Trickle timer will start with I = rand (Imin/2, Imin)
                                              0 = Trickle timer will start with I = rand (Imin, Imax) */

extern uint8_t trickle_IMIN;
extern uint8_t trickle_IMAX;
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
RAIL_MultiTimer_t sw_tri_tmr_pcs;
#else
static sw_tmr_t sw_tri_tmr_pcs;//Software timer  structure type  
#endif

static trickle_tmr sw_pcs_send;//trickle timer structure type
static clock_tick loc_clock; /* A local, general-purpose placeholder */
static uint8_t PCS_attemp_index = 0;

/*
** =============================================================================
** Public Variable Definitions
** =============================================================================
**/

fan_pkt_tmr_t pcs_timer={0};

/*
** ============================================================================
** External Variable Declarations
** ============================================================================
*/

/* None */

/*
** ============================================================================
** External Function Prototypes
** ============================================================================
*/

extern void send_trickle_timer_pkt(fan_pkt_tmr_t trickle_timer);
uint8_t is_configured_as_fixed_channel (void);

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

static void check_pcs_attemp_count ();
//extern void change_join_state(uint8_t attemt_index);
extern void reset_to_join_state_1 (void);
extern void change_join_state(uint8_t attemt_index);

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
  clock_tick last_end = sw_pcs_send.i_start + sw_pcs_send.i_cur;;
  sw_pcs_send.c = 0;

  /*When the interval I expires, Trickle doubles the interval length.
  If this new interval length would be longer than the time
  specified by Imax, Trickle sets the interval length I to be the
  time specified by Imax*/
  if (sw_pcs_send.i_cur <= sw_pcs_send.i_max_abs >> 1) 
    sw_pcs_send.i_cur <<= 1;
  else 
   sw_pcs_send.i_cur = sw_pcs_send.i_max_abs;

  /* Random t in [I/2, I) */
  loc_clock = get_t (sw_pcs_send.i_cur);
#if RAIL_TIMER_INTERFACE_USED
  clock_tick current_time = get_currunt_time_from_rail() / 1000000;
#else  
  clock_tick current_time = get_time_now_64 () / 1000000;
#endif  
  loc_clock = (last_end + loc_clock) - current_time;
  if (loc_clock > (TRICKLE_TIMER_CLOCK_MAX >> 1)) 
  {
    /* Oops, that's in the past */
    loc_clock = 0;
  }
  period_send = (uint32_t)(loc_clock * 100000);
  /*Stop the software timer */
#if RAIL_TIMER_INTERFACE_USED
  RAIL_CancelMultiTimer(&sw_tri_tmr_pcs); 
  RAIL_SetMultiTimer(&sw_tri_tmr_pcs,period_send,RAIL_TIME_DELAY,&fire,NULL);
#else  
  tmr_stop (&sw_tri_tmr_pcs);
  sw_tri_tmr_pcs.period = period_send;
  sw_tri_tmr_pcs.cb = fire;
  /*Start the software timer */
  tmr_start_relative (&sw_tri_tmr_pcs);
#endif   

  sw_pcs_send.i_start = last_end;
}
/*----------------------------------------------------------------------------*/
static void schedule_for_end(void)
{
#if RAIL_TIMER_INTERFACE_USED
  clock_tick now = get_currunt_time_from_rail()/ 1000000;
#else  
  clock_tick now = get_time_now_64 () / 1000000;
#endif 
  loc_clock = (sw_pcs_send.i_start + sw_pcs_send.i_cur) - now;
  /* Interval's end will happen in loc_clock ticks. Make sure this isn't in
   * the past... */
  if(loc_clock > (TRICKLE_TIMER_CLOCK_MAX >> 1)) 
  {
    loc_clock = 0; /* Interval ended in the past, schedule for in 0 */
  }
  /*After expire fire call back set double interval callback to double 
  the interval of timer */
  period_send = (uint32_t)(loc_clock * 100000);
#if RAIL_TIMER_INTERFACE_USED
  RAIL_CancelMultiTimer(&sw_tri_tmr_pcs);
  RAIL_SetMultiTimer(&sw_tri_tmr_pcs,period_send,RAIL_TIME_DELAY,&double_interval,NULL);
#else  
  tmr_stop (&sw_tri_tmr_pcs);
  sw_tri_tmr_pcs.cb = double_interval;
  sw_tri_tmr_pcs.period = period_send;
  tmr_start_relative (&sw_tri_tmr_pcs);
#endif  
}
/*----------------------------------------------------------------------------*/
static void new_interval(void)
{ 
  /* When an interval begins, Trickle resets c to 0 and sets t to a
  random point in the interval, taken from the range [I/2, I), that
  is, values greater than or equal to I/2 and less than I.  The
  interval ends at I.*/
  sw_pcs_send.c = 0;
  /* Random t in [I/2, I)  */
  loc_clock = get_t (sw_pcs_send.i_cur);
  period_send = (uint32_t)(loc_clock * 100000);

#if RAIL_TIMER_INTERFACE_USED
  RAIL_SetMultiTimer(&sw_tri_tmr_pcs,period_send,RAIL_TIME_DELAY,&fire,NULL);
#else  
 /*create one time software send fire call back */
  tmr_create_one_shot_timer (&sw_tri_tmr_pcs, period_send, fire, NULL);
 /*Start Software  timer */
  tmr_start_relative(&sw_tri_tmr_pcs);
#endif
  /* Store the actual interval start (absolute time), we need it later */
#if RAIL_TIMER_INTERFACE_USED
 sw_pcs_send.i_start = get_currunt_time_from_rail() / 1000000;
#else
  /* Store the actual interval start (absolute time), we need it later */
  sw_pcs_send.i_start = get_time_now_64 () / 100000;
#endif
}
/*----------------------------------------------------------------------------*/

#if RAIL_TIMER_INTERFACE_USED
static void fire(struct RAIL_MultiTimer *tmr,
                 RAIL_Time_t expectedTimeOfEvent,
                 void *cbArg )
#else
static void fire(void* context, void  *pTmr )
#endif
{  
  /*At time t, Trickle transmits if and only if the counter c is less
  than the redundancy constant k.*/  
  /*Call tx callback */
  if ((sw_pcs_send.c < sw_pcs_send.k) || 
     (sw_pcs_send.k == TRICKLE_TIMER_INFINITE_REDUNDANCY))
  {
    pcs_timer.frame_type = PAN_CONFIG_SOL;
#if (PRINT_DEBUG_LEVEL == CONSOLE_ERROR_DEBUG)
    stack_print_debug ("Trig PCS\n");
#endif
    send_trickle_timer_pkt(pcs_timer);
  } 
  
  if (pcs_timer.ready_to_change_state)
    change_join_state (0);      //Debdeep
  
  if ((sw_pcs_send.i_cur != TRICKLE_TIMER_IS_STOPPED)) 
  {
    schedule_for_end();
  }
  
  check_pcs_attemp_count ();
}
/*----------------------------------------------------------------------------*/

/*
** ============================================================================
** Public Function Definitions
** ============================================================================
*/

void trickle_timer_config_pcs_send(void)
{
  sw_pcs_send.i_min = trickle_IMIN;
  sw_pcs_send.i_max = trickle_IMAX;
  /*When the algorithm starts execution, it sets I to a value in the
  range of [Imin, Imax] -- that is, greater than or equal to Imin
  and less than or equal to Imax.  The algorithm then begins the
  first interval. */
  sw_pcs_send.i_max_abs = trickle_IMIN << trickle_IMAX;
  sw_pcs_send.k = REDUNDANCY_CONST_PCS;
  /* Random I in [Imin , Imax] */
  srand (0);
#if RESET_TRICKLE_ON_STARTUP
  sw_pcs_send.i_cur = sw_pcs_send.i_min;
#else
  sw_pcs_send.i_cur = sw_pcs_send.i_min + 
                       (rand() % 
                        ((sw_pcs_send.i_max_abs - sw_pcs_send.i_min) + 1));
#endif
  new_interval();
}

/*----------------------------------------------------------------------------*/
void trickle_timer_consistency_pcs(void)
{
  /*Whenever Trickle hears a transmission that is "consistent", it
       increments the counter c.*/
  if(sw_pcs_send.c < 0xFF) 
  {
#if (PRINT_DEBUG_LEVEL == CONSOLE_ERROR_DEBUG)
    stack_print_debug ("Suppress PCS\n");
#endif
    sw_pcs_send.c++;
  }
}
                                                      
/*----------------------------------------------------------------------------*/
void trickle_timer_pcs_stop(void)
{
  do{
#if RAIL_TIMER_INTERFACE_USED
    RAIL_CancelMultiTimer(&sw_tri_tmr_pcs);
#else  
    tmr_stop (&sw_tri_tmr_pcs);;
#endif      
    sw_pcs_send.i_cur = TRICKLE_TIMER_IS_STOPPED; 
  } while(0);
  PCS_attemp_index = 0;
}
/*----------------------------------------------------------------------------*/
void process_schedule_end_pcs()
{
#if RAIL_TIMER_INTERFACE_USED
  RAIL_CancelMultiTimer(&sw_tri_tmr_pcs);
#else  
   tmr_stop (&sw_tri_tmr_pcs);
#endif   
    return;
}
/*----------------------------------------------------------------------------*/
void process_schedule_start_pcs()
{
    trickle_timer_config_pcs_send();
    return;
}

static void check_pcs_attemp_count ()
{
  PCS_attemp_index++;
  
  if (is_configured_as_fixed_channel () == 1)
  {
    if (MAX_ATTEMT_PCS_PKT <= PCS_attemp_index)
    {
      PCS_attemp_index = 0;
      trickle_timer_pcs_stop();
      reset_to_join_state_1 ();
    }  
  }
  else
  {
    if (20 <= PCS_attemp_index)
    {
      PCS_attemp_index = 0;
      trickle_timer_pcs_stop();
      reset_to_join_state_1 ();
    }  
  }
}
/*----------------------------------------------------------------------------*/
uint8_t recv_pcs_attement_count()
{
  
  return PCS_attemp_index;
}