/** \file ccasm.c
*******************************************************************************
** \brief This file provides differrnt CCA state machine details.
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
#include "timer_service.h"
#include "mac_config.h"
#include "sm.h"
#if(CFG_MAC_SFTSM_ENABLED == 1)
#include "sftsm.h"
#endif
#include "phy.h"
#include "mac.h"
#include "mac_defs_sec.h"
#include "ie_element_info.h"
#include "mac_pib.h"
#include "mac_defs.h"
#include "event_manager.h"
#if(CFG_MAC_MPMSM_ENABLED == 1)
#include "mpmsm.h"
#endif

#include "ccasm.h"
#include "trxsm.h"
//#include "lbtsm.h"

/*
** =============================================================================
** Private Macro definitions
** =============================================================================
*/

/*! Defines function prototype for LBT random function*/
#define CCASM_RAND_FUNCTION   sw_tmr_rand_get( gptmr_mod_ins ) 
#define CCA_SAMPLE_PERIOD 20
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
#if (CFG_MAC_CCA_ENABLED == 1)
//#define MEASURE_CCA_INTERVAL
#ifdef MEASURE_CCA_INTERVAL
#define MAX_CCA_RETRY_TS        200
p3time_t ccaretryinterval[MAX_CCA_RETRY_TS];
uint8_t ccaretryintervalindex = 0;
#endif

/*
** =============================================================================
** Public Variables Definitions
** =============================================================================
**/

ccasm_t ccasm;
/*
** ============================================================================
** External Variable Declarations
** ============================================================================
*/
extern void TRX_Stop_ED( void );
extern trxsm_t trxsm;
extern mac_pib_t mac_pib;

/*
** ============================================================================
** Private Function Prototypes
** ============================================================================
*/
static sm_result_t ccasm_idle (ccasm_t *s, const sm_event_t *e);
static sm_result_t ccasm_backoff (ccasm_t *s, const sm_event_t *e);
static sm_result_t ccasm_perform_cca (ccasm_t *s, const sm_event_t *e);
static void ccasm_alarm( void *s, void  *pTmr );
ccasm_state_ind_t ccasm_get_state(ccasm_t *s);

//static sm_result_t ccasm_active( ccasm_t *, const sm_event_t * );
//static sm_result_t ccasm_sample_busyloop( ccasm_t *, const sm_event_t * );
//static sm_result_t ccasm_delay( ccasm_t *s, const sm_event_t *e );
//static sm_result_t ccasm_sample( ccasm_t *s, const sm_event_t *e );
//static sm_result_t ccasm_warmup( ccasm_t *s, const sm_event_t *e );
//static sm_result_t ccasm_suspended( ccasm_t *s, const sm_event_t *e );
//static sm_state_t state_sample;
/*None*/

/*
** ============================================================================
** Public Function Definitions
** ============================================================================
*/
uint8_t is_configured_as_fixed_channel (void);
uint64_t get_time_now_64 (void);
/******************************************************************************/
extern void set_cca_required(uint8_t cca_bit);


uchar is_ccasm_cca_needed( ccasm_t *s, mac_tx_t *packet )
{
#ifdef WISUN_FAN_MAC
  if(packet->sub_type == FAN_ACK)
    return 0;
  else 
  {
    if (is_configured_as_fixed_channel () == 1)
    {
      set_cca_required(1);
    }	
    else
      set_cca_required(0);
      //return 0; /* Suneet :: Right now CCA is disabled */ /* Only for freq hopping */
    //suneet :: 11/19/20222     use cca for rail lib not using 
    return 0;
  }
#else
  return 1;
#endif   
}
/******************************************************************************/
ccasm_state_ind_t ccasm_get_state(ccasm_t *s)
{
  return s->state_ind;
}
/******************************************************************************/
void ccasm_initialise( sm_t **s )
{
  cca_sm_initialise (&ccasm, &trxsm_ntfy_cca_done,&trxsm_ntfy_cca_request, (void *)&trxsm );
  *s = (sm_t *) &ccasm;
}
/******************************************************************************/
uchar ccasm_is_idle(sm_t *s )
{
  return ( ccasm_get_state( (ccasm_t *) s ) == CCASM_STATE_IDLE );
}
/******************************************************************************/
//uchar ccasm_is_suspended(sm_t *s )
//{
//  return ( ccasm_get_state( (ccasm_t *) s ) == CCASM_STATE_SUSPENDED );;
//}
/******************************************************************************/
void ccasm_go_to_idle (ccasm_t *s)
{
  sm_transit( (sm_t *)s, (sm_state_t) &ccasm_idle );
}
/******************************************************************************/
uchar ccasm_cca_needed( sm_t *s, mac_tx_t *packet )
{
  return ( is_ccasm_cca_needed( (ccasm_t *) s, packet ) );
}
/******************************************************************************/
void cca_sm_initialise( ccasm_t *s,
                       ccasm_callback_t fn_final_call,ccasm_callback_t fn_cca_call_req,
                       void *hn_final_call )
{
  sm_event_t e = {(sm_trigger_t) CCASM_TRIGGER_ENTRY, {0}};
  s->fn_final_call = fn_final_call;
  s->hn_final_call = hn_final_call;
  s->fn_cca_call_req = fn_cca_call_req;
  
  sw_tmr_create (&(s->cca_sw_timer), 0, (sw_tmr_cb_t)&ccasm_alarm, s);
  s->flags = (ccasm_flag_t)(CCASM_FLAG_USE_BACKOFF | CCASM_FLAG_KEEP_RX_ON | CCASM_FLAG_ENABLE_TOT_TX_DUR_THRESHOLD) ; //mac_pib.LBTFlags;
  
  /* go to IDLE state */
  s->super.state = (sm_state_t) &ccasm_idle;
  SM_DISPATCH ((sm_t *)s, &e);
}
/*****************************************************************************/
static sm_result_t ccasm_idle( ccasm_t *s, const sm_event_t *e )
{  
  switch ((ccasm_trigger_t) e->trigger)
  {
  case CCASM_TRIGGER_ENTRY:
    s->state_ind = CCASM_STATE_IDLE;
//#if (PRINT_DEBUG_LEVEL==ERROR_DEBUG)
//    stack_print_debug ("CCASM_STATE_IDLE ENTRY\n");
//#endif  
    break;
    
  case CCASM_TRIGGER_TRANSMIT:    
    /* supporting beaconless only */
    s->nb = 0;
    s->be = mac_pib.MinBE;
    s->s = mac_pib.LBTSamplingDuration;
    sm_transit( (sm_t *) s, (sm_state_t) &ccasm_backoff );
    break;
    
  case CCASM_CCA_TRIGGER_REQUEST: 
    sm_transit ((sm_t *) s, (sm_state_t)&ccasm_perform_cca );
    break;
    
  case CCASM_TRIGGER_EXIT:
    /* set the reference time */
    s->t0 = timer_current_time_get();
    break;

  default:
    break;
  }
  return NULL_POINTER;
}
/*****************************************************************************/
/* state: BACKOFF */
static sm_result_t ccasm_backoff( ccasm_t *s, const sm_event_t *e )
{
  int r = 1; 
  long t = 0;
  switch ((ccasm_trigger_t) e->trigger)
  {
  case CCASM_TRIGGER_ENTRY:
    s->state_ind = CCASM_STATE_BACKOFF;
//#if (PRINT_DEBUG_LEVEL==ERROR_DEBUG)
//    stack_print_debug ("CCASM_STATE_BACKOFF Entry\n");
//#endif
    
    if (s->be)
      r += (((unsigned int)CCASM_RAND_FUNCTION) & ((1 << s->be) - 1));
    
    /* random delay */
    t = (long)r * aUnitBackoffPeriod;
//#if (PRINT_DEBUG_LEVEL==ERROR_DEBUG)
//    stack_print_debug ("start backoff timer at %lld\n", get_time_now_64 ());
//#endif    
    /* set alarm */
    if (!tmr_start_absolute (&(s->cca_sw_timer), s->t0, t * (HWTIMER_SYMBOL_LENGTH)))
    {
      /* indicate result */
      (*s->fn_final_call) (s->hn_final_call, CCASM_ERROR);
      sm_transit ((sm_t *) s, (sm_state_t)&ccasm_idle );
    }
    break;
    
  case CCASM_TRIGGER_ALARM:	
//#if (PRINT_DEBUG_LEVEL==ERROR_DEBUG)
//    stack_print_debug ("backoff timer fired at %lld\n", get_time_now_64 ());
//#endif
    sm_transit ((sm_t *)s, (sm_state_t)&ccasm_idle);
    (*s->fn_cca_call_req) (s->hn_final_call, CCASM_SUCCESS);
    //sm_transit ((sm_t *) s, (sm_state_t)&ccasm_perform_cca );
    break;
    
  case CCASM_TRIGGER_EXIT:	
    /* set the reference time */
    s->t0 = timer_current_time_get();
    break;
    
  case CCASM_TRIGGER_CANCEL:
    /* cancel timeout */
    tmr_stop (&(s->cca_sw_timer));
    /* go to IDLE state */
    (*s->fn_final_call) (s->hn_final_call, CCASM_CANCELLED );
    sm_transit ((sm_t *)s, (sm_state_t)&ccasm_idle);
    break;
    
  default:
    break;
  }
  return NULL_POINTER;
}
/*****************************************************************************/
/* state: PERFORM_CCA */
static sm_result_t ccasm_perform_cca (ccasm_t *s, const sm_event_t *e)
{
#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))
  
  phy_status_t status = PHY_SUCCESS;
  
  switch ((ccasm_trigger_t) e->trigger)
  {
  case CCASM_TRIGGER_ENTRY:
    s->state_ind = CCASM_STATE_PERFORM_CCA;
//#if (PRINT_DEBUG_LEVEL==ERROR_DEBUG)
//    stack_print_debug ("CCASM_STATE_PERFORM_CCA Entry\n");
//#endif
    
//#if (PRINT_DEBUG_LEVEL==ERROR_DEBUG)
//    stack_print_debug ("phy_cca_on at %lld\n", get_time_now_64 ());
//#endif     
    status = phy_cca_on (s);
    switch (status)
    {
    case PHY_RX_ON:/*fall through*/
    case PHY_SUCCESS:
      /*go ahead with CCA*/
      break;
      
    case PHY_BUSY_RX:/*fall through*/
      /*CCA fail*/
    case PHY_BUSY_TX:
      /*what to do. For now lets report CCA fail */
      /* channel is busy */
//#if (PRINT_DEBUG_LEVEL==ERROR_DEBUG)
//      stack_print_debug ("PHY BUSY RX/TX :: ");
//#endif      
      s->nb += 1;
      s->be = MIN ((s->be+1), mac_pib.MaxBE);
      if (s->nb < mac_pib.MaxCSMABackoffs)
      {
//#if (PRINT_DEBUG_LEVEL==ERROR_DEBUG)
//        stack_print_debug ("Going to backoff\n");
//#endif
        sm_transit ((sm_t *)s, (sm_state_t)&ccasm_backoff);
      }
      else
      {
        /* channel is busy */
#if (PRINT_DEBUG_LEVEL==ERROR_DEBUG)
        stack_print_debug ("Going to TRXSM [FAILURE]\n");
#endif
        (*s->fn_final_call) (s->hn_final_call, CCASM_CHANNEL_BUSY);
        sm_transit ((sm_t *)s, (sm_state_t)&ccasm_idle);
      }
      return NULL_POINTER;
    default:
      break;
    }

    /* take sample */
//    phy_cca_sample( );
//#if (PRINT_DEBUG_LEVEL==ERROR_DEBUG)
//    stack_print_debug ("1st phy_cca_sample at %lld\n", get_time_now_64 ());
//#endif       
    
//#if (PRINT_DEBUG_LEVEL==ERROR_DEBUG)
//    stack_print_debug ("set alarm for next sample for %d us at %lld\n", s->s, get_time_now_64 ());
//#endif
    /* set alarm for next sample */
    if (!tmr_start_absolute (&(s->cca_sw_timer), s->t0, s->s * HWTIMER_SYMBOL_LENGTH))
    {
      /* indicate result */
      (*s->fn_final_call)( s->hn_final_call, CCASM_ERROR );
      sm_transit( (sm_t *) s, (sm_state_t) &ccasm_idle );
    }
    break;
    
  case CCASM_TRIGGER_ALARM:    
    /* take sample */
//    phy_cca_sample( );
//#if (PRINT_DEBUG_LEVEL==ERROR_DEBUG)
//    stack_print_debug ("2nd phy_cca_sample at %lld\n", get_time_now_64 ());
//#endif     
    
//#if (PRINT_DEBUG_LEVEL==ERROR_DEBUG)
//    stack_print_debug ("phy_cca_stop at %lld\n", get_time_now_64 ());
//#endif        
    /* stop sampling */
    if (phy_cca_stop () == PHY_IDLE)
    {
      /* channel is clear */
//#if (PRINT_DEBUG_LEVEL==ERROR_DEBUG)
//      stack_print_debug ("channel is clear :: Going to TRXSM [SUCCESS]\n");
//#endif     
      /* start transmission */
      (*s->fn_final_call) (s->hn_final_call, CCASM_SUCCESS);
      sm_transit ((sm_t *)s, (sm_state_t)&ccasm_idle);
    }
    else
    {
      /* channel is busy */
//#if (PRINT_DEBUG_LEVEL==ERROR_DEBUG)
//      stack_print_debug ("channel is busy :: ");
//#endif
      s->nb += 1;
      s->be = MIN ((s->be+1), mac_pib.MaxBE);
      if (s->nb < mac_pib.MaxCSMABackoffs)
      {
//#if (PRINT_DEBUG_LEVEL==ERROR_DEBUG)
//        stack_print_debug ("Going to backoff\n");
//#endif
        sm_transit ((sm_t *)s, (sm_state_t)&ccasm_backoff);
      }
      else
      {
        /* channel is busy */
#if (PRINT_DEBUG_LEVEL==ERROR_DEBUG)
        stack_print_debug ("Going to TRXSM [FAILURE]\n");
#endif
        (*s->fn_final_call) (s->hn_final_call, CCASM_CHANNEL_BUSY);
        sm_transit ((sm_t *)s, (sm_state_t)&ccasm_idle);
      }
    }
    break;
    
  case CCASM_TRIGGER_CANCEL:
    /* cancel timeout */
    tmr_stop (&(s->cca_sw_timer));
    /* go to IDLE state */
    (*s->fn_final_call) (s->hn_final_call, CCASM_CANCELLED);
    sm_transit ((sm_t *)s, (sm_state_t)&ccasm_idle);
    break;   
    
  default:
    break;
  }
  return NULL_POINTER;
}
/******************************************************************************/
/* callback function for general alarm */
static void ccasm_alarm( void *s, void  *pTmr )
{
  sm_event_t e = { (sm_trigger_t) CCASM_TRIGGER_ALARM, { 0 } };
  SM_DISPATCH( (sm_t *)s, &e );
}
/******************************************************************************/
#endif //#if (CFG_MAC_CCA_ENABLED == 1)
