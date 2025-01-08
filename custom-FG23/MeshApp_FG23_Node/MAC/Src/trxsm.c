  /** \file trxsm.c
 *******************************************************************************
 ** \brief Provides APIs for the Transceiver State Machine 
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

/** \defgroup all_sm MAC State Machines */
/**
 ** \file
 ** \addtogroup trx_sm Transceiver State Machine (TRX-SM)
 ** \ingroup all_sm
 ** @{
 **
 ** \brief Transceiver State Machine (TRX-SM)
 **
 ** The Transceiver State Machine (TRX-SM) is responsible for
 ** controling the transmission and reception activities of the
 ** MAC.
 **
 ** It has direct control over the following activities:
 **
 ** - beacon transmission
 ** - sending broadcast and direct frames
 ** - acknowledgement transmission
 ** - acknowledgement reception
 ** - waiting for an incoming beacon
 ** - switching receiver status on/off
 **/

/*******************************************************************************
* File inclusion
*******************************************************************************/
#include "StackMACConf.h"
#include "common.h"
#include <stdlib.h>
#include <math.h>
#include "queue_latest.h"
#include "list_latest.h"
#include "hw_tmr.h"
#include "sw_timer.h"
#include "timer_service.h"
#include "phy.h"
#include "mac_config.h"
#include "mac.h"
#include "mac_defs_sec.h"
#include "ie_element_info.h"
#include "mac_defs.h"
#include "sm.h"
#include "app_log.h"


#if(CFG_MAC_SFTSM_ENABLED == 1)
#include "sftsm.h"
#endif

#include "fan_sm.h"
#include "fan_mac_interface.h"   
#if(CFG_MAC_LBTSM_ENABLED == 1)
#include "lbtsm.h"
#endif

#if(CFG_MAC_CCA_ENABLED == 1)    
#include "ccasm.h"
#endif

#if(CFG_MAC_PENDADDR_ENABLED == 1)
#include "pandesc.h"
#endif

#if(CFG_MAC_MPMSM_ENABLED == 1)
#include "mpmsm.h"
#endif

#if(CFG_MAC_SFTSM_ENABLED == 1)  
#include "startsm.h"
#endif

#include "mac_pib.h"
#include "mac_mem.h"

#if(CFG_MAC_SYNCSM_ENABLED == 1)
#include "syncsm.h"
#endif

#if(CFG_MAC_DRSM_ENABLED == 1)
#include "drsm.h"
#endif

#if(CFG_MAC_PENDADDR_ENABLED == 1)
#include "pendaddr.h"
#endif

//#include "phy_timing_pib.h"
#include "event_manager.h"


#if(CFG_MAC_SCANSM_ENABLED == 1)  
#include "scansm.h"
#endif
#include "macutils.h"
#include "trxsm.h"
//#include "lpmsm.h"
#include "mac_queue_manager.h"

#if ( ( MAC_CFG_LE_RIT_CAPABILITY == 1 ) || ( MAC_CFG_LE_CSL_CAPABILITY == 1 ) )
#include "mac_le.h"
#endif

#ifdef UTEST_TRXSM
#include "utest_utils.h"
#include "utest_support.h"
#endif

#ifdef UTEST_TRX
#include "utest_utils.h"
#include "utest_support.h"
#endif

/*
** =============================================================================
** Private Macro definitions
** =============================================================================
*/

//#define DEBUG_ACK_TIMINGS
#define BCN_TIMEOUT_DELAY 500 /* a delay of 500 symbols to wait for a beacon */
#define TRXSM_macAckWaitDuration ( aUnitBackoffPeriod + aTurnaroundTime + \
                                   (aMRFSKPHRLength * phySymbolsPerOctet ) + \
                                   ( TRXSM_ACK_FRAME_SIZE * phySymbolsPerOctet ) + \
                                   + phySafetyMargin)
#define MAXIMUM_SLOT_DH51_FUCTION                       65536

extern uint8_t is_configured_as_fixed_channel (void);
extern void set_cca_required(uint8_t cca_bit);
extern void encrypt_data_packet (security_struct_t* p_security_data);
/*
** =============================================================================
** Private Structures, Unions & enums Type Definitions
** =============================================================================
**/

typedef struct phy_tx_info_tag
{
  uint8_t status;
  uint32_t tx_ts_us;
}phy_tx_info_t;

/*
** ============================================================================
** Private Variable Definitions
** ============================================================================
*/

//static int bcn_suspended = 0;
static int bcn_csm_ca_failed = 0;
static int num_of_bcn_sent = 0;
static uint8_t node_address[8]= {0};

sm_state_t backup_state = NULL;
/*Umesh : 02-01-2018*/
//static p3time_t timer_ack_rxxxx_time = 0;
//static p3time_t timer_start_time = 0;
//static p3time_t timer_exp_time = 0;
/*this varrible was not used */

/*Umesh : 02-01-2018*/
//static uint16_t dev_slot_no =0;
/*this varriable not used just assigned value*/

/*
** =============================================================================
** Public Variables Definitions
** =============================================================================
**/

/*None*/

/*
** ============================================================================
** External Variable Declarations
** ============================================================================
*/

#ifdef ENET_MAC_FOR_GU  
  enet_mac_for_te_sm_t enet_mac_te_sm;
  uint8_t enable_abnormal_cmd_tx = 0;
  uint8_t enable_abnormal_assoc_resp_tx  = 0;
  uint8_t enable_coord_realign_cmd_tx = 0;
  uint8_t send_unsolicited_eack = 0;
  uint8_t time_calc_for_EBR_Rx_And_Succ_data_tx = 0x00;
  uint8_t perform_carrier_tx = 0;
  uint8_t confirm_mode_for_cont_tx_req = 0xFF;
  uint8_t send_wrong_fcs = 0;
  void capture_timings(enet_mac_for_te_sm_t *enet_mac_te_sm, uint8_t action);
    //[Kimbal]
  //extern uint8_t hif_send_msg_up(uint8_t* p_Msg, uint16_t msg_len, uint8_t layer_id, uint8_t protocol_id );    
#endif
  
#ifdef MAC_CFG_SECURITY_ENABLED
	extern mac_security_data_t     mac_security_data;
#endif  
  
extern uchar aExtendedAddress[8];
extern mac_pib_t mac_pib;

#if(CFG_MAC_DRSM_ENABLED == 1)
extern drsm_t drsm;
#endif

#if(CFG_MAC_SFTSM_ENABLED == 1)  
extern startsm_t *startsm_p;
#endif

#if(CFG_MLME_SYNC_REQ == 1)
	extern syncsm_t *syncsm_p;
#endif
#if(CFG_MAC_SCANSM_ENABLED == 1)          
extern scansm_t *scansm_p;
#endif

#if(CFG_MAC_LBTSM_ENABLED == 1)
extern lbtsm_t lbtsm;
#elseif( CFG_MAC_CCA_ENABLED == 1)
extern ccasm_t ccasm;
#endif
extern trxsm_t *trxsm_p;

#if ( ( MAC_CFG_LE_RIT_CAPABILITY == 1 ) || ( MAC_CFG_LE_CSL_CAPABILITY == 1 ) )
	extern low_energy_t low_energy;
#endif

#if (DEBUG_FLAG == 1)
	extern void log_item(uint8_t item);
#endif

#if(FAN_FRQ_HOPPING_FEATURE_ENABLED == 1)
extern uint16_t broadcast_slot_nuumber;
#endif

extern phy_pib_t phy_pib;
extern void R_WDT_Restart(void);
/*this added for temp*/
#ifdef WISUN_FAN_MAC
extern self_info_fan_mac_t mac_self_fan_info;
#endif
#ifdef WISUN_FAN_MAC
extern fan_mac_nbr_t fan_mac_nbr;
extern mac_nbr_descriptor_t* gp_nbr_entry;
#if ((APP_LBR_ROUTER == 1) &&  (FAN_EAPOL_FEATURE_ENABLED == 1))
extern parent_child_info_tag eapol_parent_child_info;
#endif

mac_nbr_descriptor_t* get_nbr_desc_from_addr (uint8_t *addr);

#if(FAN_EDFE_FEATURE_ENABLED == 1)
extern edfe_info_t edfe_information;
#endif

#endif

#if(FAN_FRQ_HOPPING_FEATURE_ENABLED == 1)
extern uint16_t unicast_slot_no;
extern uint16_t broadcast_slot_nuumber;
#endif

/*up to here*/
/*
** ============================================================================
** External Function Prototypes
** ============================================================================
*/
/*this trxm function prototype added temp*/
/*this trxm function prototype added temp*/
#ifdef WISUN_FAN_MAC
extern void trxsm_us_channel_hop_actual(void *v);
extern valid_channel_list_t usable_channel;
extern uint32_t update_ufsi_with_procces_time(mac_tx_t *out_packt);
extern uint32_t update_bfio_with_procces_time(mac_tx_t *out_packt);
extern int getBroadcastChannelIndex(uint16_t slotNumber, uint16_t broadcastSchedID, uint16_t nChannels);

#if(FAN_EDFE_FEATURE_ENABLED == 1)
extern void start_edfe_responce_timer(uint64_t responce_wait);
extern void start_edfe_transmit_responce_timer(uint64_t responce_wait);
void stop_edfe_transmit_timer();
#endif

extern void responce_delay(uint8_t responce_delay_time,void *pkt_ptr);
extern void out_data_pkt_after_responce_delay(void *a);

#if(FAN_FRQ_HOPPING_FEATURE_ENABLED == 1)
uint8_t is_UDI_active (void);
#endif



extern uint8_t dont_change_ubcast_channel;
#endif
extern uint32_t update_bfio_with_procces_time(mac_tx_t *out_packt);
extern uint8_t set_channel_for_freq_hop(trxsm_t *s);

extern int getUnicastChannelIndex(uint16_t slotNumber, uint8_t* MACAddr, uint16_t nChannels);
extern void app_bm_free(uint8_t *pMem);
extern void update_link_stats_on_tx(uint8_t  *lladdr);
extern void App_factory_mode_conf_cb(uint8_t status);
extern uint8_t get_cuurunt_active_operation_mode();
void set_frame_counter_and_nonce (mac_tx_t *txd);
void send_mcps_no_ack_indication (void);

/*
** ============================================================================
** Private Function Prototypes
** ============================================================================
*/

static sm_result_t trxsm_idle(trxsm_t *s, const sm_event_t *e);

#ifdef MAC_CFG_BEACONING_ENABLED
static sm_result_t trxsm_bcn_tx_send(trxsm_t *s, const sm_event_t *e);
#endif

static sm_result_t trxsm_mpm_eb_tx_send(trxsm_t *s, const sm_event_t *e);
#if (UPDATE_UFSI_AFTER_CCA == 0) 
static sm_result_t trxsm_bcast_csma(trxsm_t *s, const sm_event_t *e);
#endif
//static sm_result_t trxsm_bcast_send(trxsm_t *s, const sm_event_t *e);
static sm_result_t trxsm_wait_bcn_wait(trxsm_t *s, const sm_event_t *e);
static sm_result_t trxsm_wait_mpm_eb_wait(trxsm_t *s, const sm_event_t *e);
static sm_result_t trxsm_cap_tx_csma(trxsm_t *s, const sm_event_t *e);
static sm_result_t trxsm_cap_tx_send(trxsm_t *s, const sm_event_t *e);
static sm_result_t trxsm_cap_tx_wait(trxsm_t *s, const sm_event_t *e);
static sm_result_t trxsm_cap_ack_tx_send(trxsm_t *s, const sm_event_t *e);
static sm_result_t trxsm_cfp(trxsm_t *s, const sm_event_t *e);

#ifdef ENHANCED_ACK_SUPPORT
static sm_result_t trxsm_cap_enack_tx_send(trxsm_t *s, const sm_event_t *e);
#endif	//ENHANCED_ACK_SUPPORT

#if( CFG_ENERGY_DETECTION_SCAN == 1 )
static sm_result_t trxsm_rssi(trxsm_t *s, const sm_event_t *e);
#endif	/*( CFG_ENERGY_DETECTION_SCAN == 1 )*/

static sm_result_t trxsm_off(trxsm_t *s, const sm_event_t *e);

#if RAIL_TIMER_INTERFACE_USED
static void trxsm_alarm(struct RAIL_MultiTimer *tmr,
                 RAIL_Time_t expectedTimeOfEvent,
                 void *s );
#else
static void trxsm_alarm(void *s, void* tmr );
#endif
static void trxsm_alarm_wait_bcn_wait(void *s);
static void trxsm_alarm_wait_mpm_eb_wait(void *v);

#if( CFG_ENERGY_DETECTION_SCAN == 1 )
static void trxsm_alarm_rssi(void *v);
#endif	/*( CFG_ENERGY_DETECTION_SCAN == 1 )*/

static void trxsm_alarm_cap_tx_wait(void *s);


static trxsm_result_t trxsm_requeue( trxsm_t *, uchar );
//static void trxsm_transit_cap_tx_csma( trxsm_t * s);
//static void trxsm_transit_cap_tx_csma_broacast( trxsm_t *s );
#ifdef MAC_CFG_BEACONING_ENABLED
static uchar trxsm_ble_active( trxsm_t * );
#endif

static void trxsm_activate_csm( trxsm_t *s, uint16_t OrigChannel );
static void trxsm_deactivate_csm( trxsm_t *s );
static void trxsm_cancel_curr_activities( trxsm_t *s, uchar sw_off_radio );
static trxsm_result_t trxsm_process_mpm_eb_tx_conf(trxsm_t* s);
void trxsm_transit( sm_state_t state );
static void trxsm_enqueue_completed_msg(trxsm_t *s);
//#if (CFG_MAC_CCA_ENABLED == 1)    
//static void create_cca_event(trxsm_t *s,ccasm_param_t* p_cca_param);
//#endif
#ifdef DEBUG_ACK_TIMINGS
p3time_t ack_tx_completion;
p3time_t ack_tx_start;
#endif
//extern ch_change_set ch_set_val [128];
//extern uint8_t TRX_Set_Channel_form_hopFrequency_list( uint8_t intc ,uint32_t frac_val, uint8_t set_ch_num );

/*
** ============================================================================
** Public Function Definitions
** ============================================================================
*/

void trxsm_initialise(trxsm_t *s, sm_t *ccasm /* scansm_t *scansm */ )
{
    s->state_ind = TRXSM_STATE_NONE;

    /* initialise state machines */
#if (CFG_MAC_CCA_ENABLED == 1)       
    s->ccasm = ccasm;
#endif
    
#if(CFG_MAC_SFTSM_ENABLED == 1)      
    s->sftsm = NULL_POINTER;
#endif
#if RAIL_TIMER_INTERFACE_USED
//    s->sw_timer.callback = &trxsm_alarm;
//    s->sw_timer.cbArg = s;
    //RAIL_SetMultiTimer(&(s->sw_timer),0,RAIL_TIME_DELAY,&trxsm_alarm,s);
#else    
    if( sw_tmr_create ( &(s->sw_timer),0, (sw_tmr_cb_t)&trxsm_alarm, s ))
#endif      
    {
#ifdef UTEST_TRXSM
        utu_timestamp(UTUL_ERR_CLIENT, -1);
#endif
        /* FIXME: shouldn't get here */
        /* maybe return with some error value */
    }


    /* FIXME: set message queues */
    s->curr_bcn_queue = NULL_POINTER;
    s->bcast_queue = NULL_POINTER;
    s->direct_queue = NULL_POINTER;
    s->indirect_queue = NULL_POINTER;
    s->completed_queue = NULL_POINTER;

    /* set packets */
    s->packet = NULL_POINTER;
    s->delay_ack = 0;
     
   
}

/******************************************************************************/
#if(CFG_MAC_SFTSM_ENABLED == 1)  
void trxsm_use_sftsm( trxsm_t *s, sftsm_t *sftsm )
{
    s->sftsm = sftsm;
}

/******************************************************************************/

sftsm_t* trxsm_get_sftsm( trxsm_t *s )
{
    return s->sftsm;
}
#endif 
/******************************************************************************/

void trxsm_start( trxsm_t *s )
{
    sm_event_t e = { (sm_trigger_t) TRXSM_TRIGGER_ENTRY, { 0 } };

    /* configuration */
    s->result = TRXSM_NONE;

    /* start */
    s->super.state = (sm_state_t)&trxsm_idle;
    SM_DISPATCH((sm_t *)s, &e);
}

/******************************************************************************/

ushort trxsm_pack( void *s, uchar *dst )
{
    memcpy( (void *) dst, s, sizeof( trxsm_t ) );
    return sizeof( trxsm_t );
}

/******************************************************************************/

void trxsm_unpack( void *s, uchar *src )
{
    memcpy( s, (void *) src, sizeof( trxsm_t ) );
}

/******************************************************************************/

trxsm_state_ind_t trxsm_get_state(trxsm_t *s)
{
    return s->state_ind;
}

/******************************************************************************/

trxsm_result_t trxsm_get_result(trxsm_t *s)
{
    return s->result;
}

/******************************************************************************/

/* PD-DATA.confirm from PHY
 * Send a data confirm event to the MAC state machine.
 * PHY status*/
void PD_Data_Confirmation_cb( void *s, uchar status, uint32_t tx_ts_us )
{
#if (APP_LBR_ROUTER  == 0x01 ) 
  if(get_cuurunt_active_operation_mode() == 0x00)  //0x00 for factory mode
  {
    status = 0;
    App_factory_mode_conf_cb(status);
    return;
  }
#endif  
    sm_event_t e = { (sm_trigger_t) TRXSM_TRIGGER_PD_DATA_CONFIRM, { 0 } };
    phy_tx_info_t phy_tx_info;
    phy_tx_info.status = status;
    phy_tx_info.tx_ts_us = tx_ts_us;
#ifdef UTEST_TRX
    utu_timestamp( UTUL_TX_OFF, status );
#endif

#ifdef ENET_MAC_FOR_GU
//if(time_calc_for_EBR_Rx_And_Succ_data_tx)
//{
//  capture_timings(&enet_mac_te_sm, CAPTURE_START_TIME);
//}
    
if (enable_abnormal_cmd_tx == 1 || send_unsolicited_eack == 1 || enable_abnormal_assoc_resp_tx == 1 || enable_coord_realign_cmd_tx==1||send_wrong_fcs==1)
{
    msg_t* nhle_msg = allocate_nhle_2_mac_msg( 2 );
    uint8_t* ptr = nhle_msg->data;
  if( NULL != nhle_msg )
  {
    *ptr++ = CONFIG_TE_BEHAVIOUR_CONF; 
    if(status == 0x07 /*PHY_SUCCESS*/)
    {
      *ptr++ = 0x00;
    }
    else
    {
      *ptr++ = status;
    }
    //hif_send_msg_up(nhle_msg->data,nhle_msg->data_length-1,1);
    free_mac_2_nhle_msg( nhle_msg );
    send_unsolicited_eack = 0;
    enable_abnormal_cmd_tx = 0;
    enable_abnormal_assoc_resp_tx = 0;
    enable_coord_realign_cmd_tx = 0;
    send_wrong_fcs = 0;
  } 
  else
  {
    // Buff of 2 bytes failed --- This should never come here.
  }
  return;
}
else
{
#endif
    /* forward the status to the application if the MAC is disabled */
    if ( mac_pib.ConfigFlags & MAC_DISABLED )
    {
//#if (PRINT_DEBUG_LEVEL==ERROR_DEBUG)
//    stack_print_debug ("D phy state = %d\n", plme_get_trx_state_request ());
//#endif       
        /* turn the transmitter off */
        PLME_Set_TRX_State( PHY_TRX_OFF );
        
//#if (PRINT_DEBUG_LEVEL==ERROR_DEBUG)
//    stack_print_debug ("E phy state = %d\n", plme_get_trx_state_request ());
//#endif         
        
        //send_pd_data_confirm( status );
        return;
    }

    /* convert PHY status to MACSM status */
    switch( status )
    {
    case PHY_SUCCESS:
        status = TRXSM_SUCCESS;
        break;

    default:
        status = TRXSM_FAILURE;
        break;
    }
    phy_tx_info.status = status;
      e.param.vector = &phy_tx_info;
    //e.param.scalar = status;
    SM_DISPATCH( (sm_t *)s, &e );
#ifndef EFM32_TARGET_IAR
    lpmsm_clear_flag( LPMSM_FLAG_TX_ON );
#endif

#ifdef ENET_MAC_FOR_GU    
}
#endif
}

/******************************************************************************/
#if(CFG_MAC_CCA_ENABLED == 1)    
void trxsm_ntfy_cca_done( void *s, ccasm_result_t result )
{
    sm_event_t e;

    e.trigger = (sm_trigger_t) TRXSM_TRIGGER_CCA_DONE;
    e.param.scalar = result;

    SM_DISPATCH((sm_t *)s, &e);

    return;
}
/******************************************************************************/   
void trxsm_ntfy_cca_request( void *s, ccasm_result_t result )
{
    sm_event_t e;

    e.trigger = (sm_trigger_t) TRXSM_TRIGGER_CCA_MODE_1_REQ;
    e.param.scalar = result;

    SM_DISPATCH((sm_t *)s, &e);

    return;
}
#endif
/******************************************************************************/
void trxsm_set_pending_tx_event( trxsm_t *s )
{
	//if( !ccasm_is_suspended( s->ccasm ) )
	//{
		//event_set(PENDING_TX_EVENT);
		//signal_event_to_mac_task();
	//}
}
/*
** ============================================================================
** Private Function Definitions
** ============================================================================
*/
/* Debdeep removed commented code in this function on 12-July-2018 */
/* state: IDLE*/
uint16_t retransmit_packet = 0xFFFF;
uint16_t retransmit_count = 0x0000;
volatile int phy_state_event_execute = 0;
static sm_result_t trxsm_idle(trxsm_t *s, const sm_event_t *e)
{
  ulong temp = 0;
  uint16_t len = 0;
  phy_status_t status = PHY_SUCCESS;
  uint32_t channel = 0;
  queue_t * p_queue = NULL;
  phy_status_t state = PHY_DEV_ON;
  
#if ( ( MAC_CFG_LE_RIT_CAPABILITY == 1 ) || ( MAC_CFG_LE_CSL_CAPABILITY == 1 ) )
  low_energy_state_ind_t curr_le_rit_state = low_energy_get_state(&low_energy);
#endif
  
  switch ((trxsm_trigger_t) e->trigger)
  {
  case TRXSM_TRIGGER_ENTRY:
    s->state_ind = TRXSM_STATE_IDLE;
#ifdef UTEST_TRXSM
    utu_timestamp(UTUL_TRXSM_IDLE, s->alarm.client.index);
#endif
     PLME_Set_TRX_State( PHY_RX_ON );
     if((queue_manager_size(QUEUE_CAP_TX)) != 0 )
     {

#if(FAN_EDFE_FEATURE_ENABLED == 1)
       if(edfe_information.edfe_frame_enabled == 0x01)
       {
        //do nothing
       }
       else
#endif
         event_set(PENDING_TX_EVENT_UCAST);
       
     }
     if( (queue_manager_size(QUEUE_BCAST)) != 0 )
     {
#if(FAN_EDFE_FEATURE_ENABLED == 1)
       if(edfe_information.edfe_frame_enabled == 0x01)
       {
        //do nothing
       }
       else
#endif
         event_set(PENDING_TX_EVENT_BCAST);
     }
     
    //ccasm_go_to_idle ((ccasm_t *)s->ccasm);
    
    /* Note: deliberate fallthrough to set receiver */
    
  case TRXSM_TRIGGER_UPDATE_RX:/*fall through*/
  case TRXSM_TRIGGER_PD_DATA_INDICATION:
    if ((trxsm_trigger_t) e->trigger == TRXSM_TRIGGER_PD_DATA_INDICATION)
      phy_state_event_execute++;
    /* check if we are in promiscuous mode */
    if( mac_pib.PromiscuousMode )
    {
      /* just turn the receiver on */
#ifndef EFM32_TARGET_IAR
      lpmsm_set_flag( LPMSM_FLAG_RX_ON );
#endif
      state =  PHY_RX_ON;
    }
    else
#ifdef MAC_CFG_BEACONING_ENABLED
      /* set receiver according to RxOnWhenIdle */
      if( s->sftsm )
      {
        /* beacon enabled */
        /*TBD The spec says "incoming superframe" (page 165) */
        if( sftsm_get_state( s->sftsm ) == SFTSM_STATE_CAP )
        {
          if( ( mac_pib.RxOnWhenIdle && !trxsm_ble_active( s ) ) ||
#if (CFG_MAC_DRSM_ENABLED == 1)                       
             drsm_get_state( &drsm ) != DRSM_STATE_NONE ||
#endif   
#if (CFG_MAC_SCANSM_ENABLED == 1)                            
               scansm_get_state( scansm_p ) != SCANSM_STATE_NONE )
#endif                    
          {
#ifndef EFM32_TARGET_IAR
            lpmsm_set_flag( LPMSM_FLAG_RX_ON );
#endif
            state =  PHY_RX_ON;
          }
          else
          {
#ifndef EFM32_TARGET_IAR
            lpmsm_clear_flag( LPMSM_FLAG_RX_ON );
#endif
          }
        }
      }
      else
#endif
      {
        /* beaconless */
        if( 
#if ( ( MAC_CFG_LE_RIT_CAPABILITY == 1 ) || ( MAC_CFG_LE_CSL_CAPABILITY == 1 ) )                
           (( mac_pib.RxOnWhenIdle ) && ( curr_le_rit_state == LE_STATE_INIT ))||
#else
             ( mac_pib.RxOnWhenIdle )
#endif 
#if (CFG_MAC_DRSM_ENABLED == 1)                                  
               ||( drsm_get_state( &drsm ) != DRSM_STATE_NONE )
#endif      
#if (CFG_MAC_SCANSM_ENABLED == 1)                  
                 ||( scansm_get_state( scansm_p ) != SCANSM_STATE_NONE ) 
#endif      
#if ( ( MAC_CFG_LE_RIT_CAPABILITY == 1 ) || ( MAC_CFG_LE_CSL_CAPABILITY == 1 ) )          
                   || (curr_le_rit_state == LE_STATE_RIT_DATA_WAIT ) ||
                     (curr_le_rit_state == LE_STATE_DATA_TX_WAIT ) 
#endif                                                          
                       )
        {   
#ifndef EFM32_TARGET_IAR                
          lpmsm_set_flag( LPMSM_FLAG_RX_ON );
#endif  
          state =  PHY_RX_ON;
        }
        else
        {
#ifndef EFM32_TARGET_IAR
          lpmsm_clear_flag( LPMSM_FLAG_RX_ON );
#endif //ndef EFM32_TARGET_IAR
        }
      }
    /*Uemsh : 11-01-2018 this thing goes to modified after sepration of mac and fan mac*/
#ifdef WISUN_FAN_MAC
    state = PHY_RX_ON;
#endif            
    PLME_Set_TRX_State( state );            
    break;     
    
#ifdef MAC_CFG_BEACONING_ENABLED
  case TRXSM_TRIGGER_ALARM_BEACON:
    trxsm_transit((sm_state_t)&trxsm_bcn_tx_send);
    break;
#endif    
    
  case TRXSM_TRIGGER_ALARM_CFP:
    trxsm_transit((sm_state_t)&trxsm_cfp);
    break;
    
  case TRXSM_TRIGGER_ALARM_INACTIVE:
    /* turn receiver off */
    
//#if (PRINT_DEBUG_LEVEL==ERROR_DEBUG)
//    stack_print_debug ("F phy state = %d\n", plme_get_trx_state_request ());
//#endif     
    
    PLME_Set_TRX_State( PHY_DEV_ON );
    
    break;
  case TRXSM_TRIGGER_ALARM_MPM_EB:    
  case TRXSM_TRIGGER_BCN_TX_REQUIRED:
    if( 
#if (CFG_MAC_SFTSM_ENABLED == 1)                   
       (startsm_get_flags( startsm_p ) & STARTSM_FLAG_COORD) &&   /* running as coordinator */
         s->sftsm == NULL_POINTER &&      /* beaconless network */
#endif                
           !PHY_rcv_in_progress() )         /* no reception going on*/
    {
      /* check if there is new beacon */
      if( e->param.scalar == MAC_FRAME_BCN_SUB_TYPE_RB )
        p_queue = s->curr_bcn_queue;
      else if ( e->param.scalar == MAC_FRAME_BCN_SUB_TYPE_EB )
        p_queue = s->curr_eb_queue;
      else
        p_queue = s->curr_mpm_eb_queue;
      
      if( (s->packet = (mac_tx_t *) queue_item_get_last( p_queue )) == NULL_POINTER )
      {
        /* no new beacon, don't send */
        /*TBD Shall we wait for a beacon to get ready? */
        break;
      }
      
      if( (s->packet->type == MAC_FRAME_TYPE_BEACON ) && 
         ( MAC_FRAME_BCN_SUB_TYPE_MPM_EB == s->packet->sub_type ) )
      {
        if( !( s->iUnitRadioChan ) )
        {
          PLME_Get_CSM_Unit_Radio_Chan_List
            (
             s->packet->data->TxChannel,
             &(s->CSMCurrTotalUnitRadioChanCount),
             s->CSMUnitRadioChanList
               );
          
          trxsm_activate_csm( s,s->packet->data->TxChannel );
          
          s->packet->data->TxChannel = 
            s->CSMUnitRadioChanList[ s->iUnitRadioChan++ ];
          
          channel = s->packet->data->TxChannel;
          PLME_set_request( phyCurrentChannel, 2, &channel );
          
        }
      }
      
      
      /*TBD Shall we check the result of security processing here? */
      
      /* send packet now */
#if (CFG_MAC_CCA_ENABLED == 1)      
      trxsm_transit((ccasm_cca_needed( s->ccasm, s->packet ))?((sm_state_t) &trxsm_cap_tx_csma) :((sm_state_t) &trxsm_cap_tx_send));
#else
      if (is_configured_as_fixed_channel () == 1)
      {
        set_cca_required(1);
      }
      trxsm_transit((sm_state_t) &trxsm_cap_tx_send);
#endif      
    }
    break;
    
  case TRXSM_TRIGGER_NEW_DIRECT_PACKET:
    if(
#if (CFG_MAC_SFTSM_ENABLED == 1)                
       (s->sftsm == NULL_POINTER) || 	/* beaconless network or... */
#endif                 
#ifdef MAC_CFG_BEACONING_ENABLED	    
         ( sftsm_get_state( s->sftsm ) == SFTSM_STATE_CAP) &&	/* ...in CAP */
#endif		                 
           !PHY_rcv_in_progress() )             /* and no reception going on */
    {
      /* see next packet of direct queue */
      if( (s->packet = (mac_tx_t *) queue_item_get( s->direct_queue )) == NULL_POINTER )
      {
        clear_process_activity( AF_CAP_MSG_PENDING );
        break;
      }
      if( (s->packet->type == MAC_FRAME_TYPE_BEACON ) && 
         ( MAC_FRAME_BCN_SUB_TYPE_MPM_EB == s->packet->sub_type ) )
      {
        if( !( s->iUnitRadioChan ) )
        {
          PLME_Get_CSM_Unit_Radio_Chan_List
            (
             s->packet->data->TxChannel,
             &(s->CSMCurrTotalUnitRadioChanCount),
             s->CSMUnitRadioChanList
               );
          
          trxsm_activate_csm( s,s->packet->data->TxChannel );
          
          s->packet->data->TxChannel = 
            s->CSMUnitRadioChanList[ s->iUnitRadioChan++ ];
          
          channel = s->packet->data->TxChannel;
          PLME_set_request( phyCurrentChannel, 2, &channel );	            
        }
      }
      
      if (s->packet!= NULL)
      {
        event_clear(PENDING_TX_EVENT_UCAST);
        clear_process_activity( AF_CAP_MSG_PENDING );
      }
      
      /* check if packet has been tried in this superframe */
#ifdef MAC_CFG_BEACONING_ENABLED		
      /* check if syncing is needed first */
      /*TBD Shouldn't we check packet type as well (like, Beacon Request) - are there anymore exceptions? */
      if( (mac_pib.BeaconOrder < 15) &&
         syncsm_p &&
           (syncsm_get_state( syncsm_p ) == SYNCSM_STATE_STOPPED)  &&
             ((s->packet->type == MAC_FRAME_TYPE_MAC_COMMAND) && (s->packet->cmd == BEACON_REQUEST)) )
      {
        /* requeue packet */
        queue_front_put( s->direct_queue, (queue_item_t *) s->packet );
        trxsm_set_pending_tx_event(s);
        s->packet = NULL_POINTER;
        
        event.trigger = (sm_trigger_t) SYNCSM_TRIGGER_SYNC_REQ;
        event.param.scalar = SYNCSM_FLAG_AUTO | 1;
        SM_DISPATCH( (sm_t *) syncsm_p, &event );
        break;
      }
#endif
      
#if (UPDATE_UFSI_AFTER_CCA == 0)
      /*suneet:: CSMA/CA MUST NOT be used before asynchronous frame transmissions.
      s->packet->sub_type 
      1.PA(0)2.PAS(1)3.PC(2)4.PCS(3)
      */
      if(s->packet->sub_type >= 0x04)
      {
#if (CFG_MAC_CCA_ENABLED == 1)            
        trxsm_transit((ccasm_cca_needed( s->ccasm, s->packet ))?((sm_state_t) &trxsm_cap_tx_csma):((sm_state_t) &trxsm_cap_tx_send));
#else
      if (is_configured_as_fixed_channel () == 1)
      {
        set_cca_required(1);
      }
         trxsm_transit((sm_state_t) &trxsm_cap_tx_send);
#endif 
      }
      else{
        trxsm_transit((sm_state_t) &trxsm_cap_tx_send);
      }
#else
      trxsm_transit((sm_state_t) &trxsm_cap_tx_send);
#endif
    }
    break;
    
  case TRXSM_TRIGGER_NEW_BCAST_PACKET:
    
    if(
#if(CFG_MAC_SFTSM_ENABLED == 1)               
       s->sftsm != NULL_POINTER ||              /* beacon-enabled network or... */
#endif //#if(CFG_MAC_SFTSM_ENABLED == 1)                 
         !PHY_rcv_in_progress() )                  /* reception going on */
    {
      
      /* get next packet from broadcast queue */
      if( (s->packet = (mac_tx_t *) queue_item_get( s->bcast_queue )) == NULL_POINTER )
      {
        break;
      }
      
      if(s->packet!= NULL)
      {
        event_clear(PENDING_TX_EVENT_BCAST);
        clear_process_activity( AF_CAP_MSG_PENDING );
      }
      
#ifdef UTEST_TRXSM
      utu_timestamp(CCA_TRIGGERED_FOR_BCAST_DATA_TX,s->packet->sn);
#endif
#ifdef WISUN_FAN_MAC/*Umesh : 21-02-2018*//*for sepration of 802.15.04*/
#if (UPDATE_UFSI_AFTER_CCA == 0) 
      
      /*suneet:: CSMA/CA MUST NOT be used before asynchronous frame transmissions.
      s->packet->sub_type 
      1.PA(0)2.PAS(1)3.PC(2)4.PCS(3)
      */
      if(s->packet->sub_type >= 0x04)
      {
#if CFG_MAC_CCA_ENABLED        
        trxsm_transit((ccasm_cca_needed( s->ccasm, s->packet ))?((sm_state_t) &trxsm_bcast_csma):((sm_state_t) &trxsm_cap_tx_send));
#else 
        if (is_configured_as_fixed_channel () == 1)
        {
          set_cca_required(1);
        }
        trxsm_transit((sm_state_t) &trxsm_cap_tx_send);
#endif
      }
      else
      {
        trxsm_transit((sm_state_t) &trxsm_cap_tx_send);  
      }
#else
      trxsm_transit((sm_state_t) &trxsm_cap_tx_send);
#endif
#else
      trxsm_transit((sm_state_t)&trxsm_bcast_csma);
#endif
    }
    break;
    
#ifdef ENHANCED_ACK_SUPPORT
  case TRXSM_TRIGGER_ENACK_TMR_EXPIRY:
    s->result = TRXSM_NONE;
    trxsm_requeue( s, 0 );
    s->packet = e->param.vector;
    trxsm_requeue( s, 1 );
    //trxsm_transit_cap_tx_csma( s );
    break;
#endif //ENHANCED_ACK_SUPPORT
    
    
    
  case TRXSM_TRIGGER_ACK_REQUIRED:      
                  s->result = TRXSM_NONE;
                  //requeue packet 
                  trxsm_requeue( s, 0 );                  
#if RAIL_TIMER_INTERFACE_USED
                  RAIL_CancelMultiTimer(&s->sw_timer);
#else  
                  tmr_stop
                    ( 
                     &(s->sw_timer)
                       );
#endif  
                  
                  /*TBD Do we really need this guard here? */
//              #if (CFG_MAC_SFTSM_ENABLED == 1)                          
//                  if(( s->sftsm == NULL_POINTER ) 
//                     
//              #ifdef MAC_CFG_BEACONING_ENABLED	    
//                     || ( sftsm_get_state( s->sftsm ) == SFTSM_STATE_CAP )	/* ...in CAP */
//              #endif
//                       )                
//                  {
//                    /* set ack as the current packet */
//                    s->packet = e->param.vector;
//                    sm_back_up_state( (sm_t *)s );
//                    trxsm_transit((sm_state_t)&trxsm_cap_ack_tx_send);
//                  }
//              #else
                  s->packet = e->param.vector;
                  //sm_back_up_state( (sm_t *)s );
                  trxsm_transit((sm_state_t)&trxsm_cap_ack_tx_send);
                  
//              #endif //#if(CFG_MAC_SFTSM_ENABLED == 1)            
    break;
    
  case TRXSM_TRIGGER_BCN_EXPECTED:
    
    /* set timeout to wait for beacon (in symbols) */
    s->tmp = *((ulong *) e->param.vector);
    /* go and wait for beacon */
    trxsm_transit((sm_state_t)&trxsm_wait_bcn_wait);
    break;
    
  case TRXSM_TRIGGER_MPM_EB_EXPECTED:
    
    /* set timeout to wait for beacon (in symbols) */
    s->tmp = *((ulong *) e->param.vector);
    /* go and wait for mpm EBs */
    trxsm_transit((sm_state_t)&trxsm_wait_mpm_eb_wait);
    break;
#if( CFG_ENERGY_DETECTION_SCAN == 1 )
  case TRXSM_TRIGGER_RSSI:
    
    /* set timeout to wait for beacon (in symbols) */
    s->tmp = *((ulong *) e->param.vector);
    /* turn RSSI on */    
    if( ( status = phy_ed_on( NULL_POINTER )) == PHY_SUCCESS )
    {
      trxsm_transit((sm_state_t)&trxsm_rssi);
      s->result = TRXSM_SUCCESS;    
    }
    else
      s->result = TRXSM_FAILURE;    
    break;
    
  case TRXSM_TRIGGER_ED_SCAN_DONE:
    break;
    
#endif	/*( CFG_ENERGY_DETECTION_SCAN == 1 )*/
  case TRXSM_TRIGGER_CHANNEL:
    temp = 0;
    /* set page and channel */
    /* no change if it's the same as the current channel */
    PLME_get_request( phyCurrentPage, &len, &temp );
    if( (uchar) temp != ((trxsm_channel_t *)e->param.vector)->page )
    {
      
      temp = 0;
      temp = ((trxsm_channel_t *)e->param.vector)->page;
      status = PLME_set_request( phyCurrentPage, 1, &temp );
      if(  status != PHY_SUCCESS )
      {
        s->result = TRXSM_FAILURE;
        break;
      }
    }
    
    temp = 0;            
    PLME_get_request( phyCurrentChannel, &len, &temp );
    if( (uchar) temp != ((trxsm_channel_t *)e->param.vector)->channel )
    {
      temp = 0;
      temp = ((trxsm_channel_t *)e->param.vector)->channel;
      status = PLME_set_request( phyCurrentChannel, 2, &temp );
      
      if(  status != PHY_SUCCESS )
      {
        s->result = TRXSM_FAILURE;
        break;
      }
    }
    
    s->result = TRXSM_SUCCESS;
    break;
    
  case TRXSM_TRIGGER_CANCEL:    
    if( e->param.scalar == TRXSM_PARAM_OFF )
      trxsm_transit((sm_state_t)&trxsm_off);
    break;
    
  default:
    break;
  }
  return NULL_POINTER;
}

/******************************************************************************/
#ifdef MAC_CFG_BEACONING_ENABLED
/*state: BCN_TX_SEND*/
static sm_result_t trxsm_bcn_tx_send(trxsm_t *s, const sm_event_t *e)
{
    mac_tx_t *packet = NULL;
    sm_event_t event;

    switch ((trxsm_trigger_t) e->trigger)
    {
        case TRXSM_TRIGGER_ENTRY:
            s->state_ind = TRXSM_STATE_BCN_TX_SEND;
#ifdef UTEST_TRXSM
            utu_timestamp(UTUL_TRXSM_BCN_TX, -1);
#endif
            /* check if there is new beacon */
            

            if( (s->packet = (mac_tx_t *) queue_item_get_last( s->curr_bcn_queue )) == NULL_POINTER )
            {
                /* no new beacon, don't send */
                break;
            }

            /*TBD Shall we check the result of security processing here? */

            /* inform power management */
#ifndef EFM32_TARGET_IAR
            lpmsm_set_flag( LPMSM_FLAG_TX_ON );
#endif

            /* force the transmitter off */

			PLME_Set_TRX_State( PHY_TX_ON );
            /* send beacon */
            PD_Data_Request(s, s->packet->data );
        
            /*TBD Remember the transmission time ? */
            //mac_pib.BeaconTxTime = mac_data.current_symbol_count;

            /* free elements of current beacon queue
               Note: if there are packets in the current beacon queue, they
                     are all considered out of date, as we have taken out the
                     last (and latest) element of the queue above. */
            while( (packet = (mac_tx_t *) queue_item_get( s->curr_bcn_queue )) != NULL_POINTER )
            {
                mac_mem_free_tx( packet );
            }

            /* reset next packet of direct queue */
            if( (packet = (mac_tx_t *) queue_peek( s->direct_queue )) != NULL_POINTER )
            {
                packet->status = MAC_SUCCESS;
               
            }
            break;

        case TRXSM_TRIGGER_PD_DATA_CONFIRM:

            /*TBD Add proper error handling here */
            if( ((phy_tx_info_t *)e->param.vector)->status == TRXSM_SUCCESS )
            {
                s->result = TRXSM_SUCCESS;
                
            }
            else
            {
                s->result = TRXSM_FAILURE;
            }

            /* increment Beacon Sequence Number */
            mac_pib.BSN += 1;

			if( !(mac_pib.mac_security_enabled) )

			{
				/* update current beacon without rebuilding */
				s->packet->data->psdu[2] = mac_pib.BSN;

				/* requeue new beacon */
				mac_queue_beacon_transmission( s->packet,MAC_FRAME_BCN_SUB_TYPE_RB );
				s->packet = NULL_POINTER;
			}
			else
			{
				mac_mem_free_tx(s->packet);
				s->packet = NULL_POINTER;
#if(CFG_MAC_BEACON_ENABLED == 1)                               
				mac_beacon_update(MAC_FRAME_BCN_SUB_TYPE_RB);
#endif                                
			}
		
            /* send event to SFTSM */
            /*TBD Shouldn't the event contain the superframe parameters? */
            if( s->sftsm )
            {
                event.trigger = (sm_trigger_t) SFTSM_TRIGGER_BCN_DONE;
                SM_DISPATCH( (sm_t *)(s->sftsm), &event );
            }

            /* trigger STARTSM */
            event.trigger = (sm_trigger_t) STARTSM_TRIGGER_BCN_SENT;
            event.param.scalar = 0;
#if (CFG_MAC_STARTSM_ENABLED == 1)              
            SM_DISPATCH( (sm_t*) startsm_p, &event );
#endif
            /* go IDLE on beaconless network */
            if( s->sftsm == NULL_POINTER )
            {
                //sm_transit( (sm_t *) s, (sm_state_t) &trxsm_idle );
		trxsm_transit((sm_state_t)&trxsm_idle);
                break;
            }

            /* get next packet from broadcast queue */
            if( (s->packet = (mac_tx_t *) queue_item_get( s->bcast_queue )) != NULL_POINTER )
            {
                //sm_transit((sm_t *) s, (sm_state_t) &trxsm_bcast_csma);
		trxsm_transit((sm_state_t)&trxsm_bcast_csma);
                break;
            }

            /* back to IDLE */
            //sm_transit( (sm_t *) s, (sm_state_t) &trxsm_idle );
	    trxsm_transit((sm_state_t)&trxsm_idle);
            break;

        case TRXSM_TRIGGER_CANCEL:

            /* requeue beacon */
			if( !(mac_pib.mac_security_enabled) )
			{
				mac_queue_beacon_transmission( s->packet, MAC_FRAME_BCN_SUB_TYPE_RB );
				s->packet = NULL_POINTER;
			}
            else
			{
				mac_mem_free_tx(s->packet);
				s->packet = NULL_POINTER;
#if(CFG_MAC_BEACON_ENABLED == 1)                                
				mac_beacon_update(MAC_FRAME_BCN_SUB_TYPE_RB);
#endif                                
			}

			trxsm_cancel_curr_activities( s, e->param.scalar );

            break;

        default:
            break;
    }
    return NULL_POINTER;
}
#endif

/******************************************************************************/

/* state: MPM_EB_TX_SEND*/
static sm_result_t trxsm_mpm_eb_tx_send(trxsm_t *s, const sm_event_t *e)
{
    mac_tx_t *packet = NULL;
    sm_event_t event;

    switch ((trxsm_trigger_t) e->trigger)
    {
        case TRXSM_TRIGGER_ENTRY:
            s->state_ind = TRXSM_STATE_MPM_EB_TX_SEND;
#ifdef UTEST_TRXSM
            utu_timestamp(UTUL_TRXSM_MPM_EB_TX_TRIGGERED, -1);
#endif
			
            /* check if there is new beacon 
            s->packet = (mac_tx_t *) queue_item_get_last( s->curr_mpm_eb_queue );

            if( s->packet == NULL_POINTER )
            {
                
                break;
            }*/
            /*
            if( !( s->iUnitRadioChan ) )
            {
            	PLME_Get_CSM_Unit_Radio_Chan_List
				(
					s->packet->data->TxChannel,
					&(s->CSMCurrTotalUnitRadioChanCount),
					s->CSMUnitRadioChanList
				);
			
				trxsm_activate_csm( s,s->packet->data->TxChannel );
            }*/

			
            
			/*TBD Shall we check the result of security processing here? */

            /* inform power management */
#ifndef EFM32_TARGET_IAR
            lpmsm_set_flag( LPMSM_FLAG_TX_ON );
#endif

            /* force the transmitter off */

			PLME_Set_TRX_State( PHY_TX_ON );
			
            /* send MPM EB */
            PD_Data_Request(s, s->packet->data );
            
            /*TBD Remember the transmission time ? */
            //mac_pib.BeaconTxTime = mac_data.current_symbol_count;

            /* free elements of current beacon queue
               Note: if there are packets in the current beacon queue, they
                     are all considered out of date, as we have taken out the
                     last (and latest) element of the queue above. */
            while( (packet = (mac_tx_t *) queue_item_get( s->curr_mpm_eb_queue )) != NULL_POINTER )
            {
                mac_mem_free_tx( packet );
            }

            /* reset next packet of direct queue */
            if( (packet = (mac_tx_t *) queue_peek( s->direct_queue )) != NULL_POINTER )
            {
                packet->status = MAC_SUCCESS;
               
            }
            break;

        case TRXSM_TRIGGER_PD_DATA_CONFIRM:

            /*TBD Add proper error handling here */
            if( ((phy_tx_info_t *)e->param.vector)->status == TRXSM_SUCCESS )
            {
                s->result = TRXSM_SUCCESS;              
            }
            else
            {
                s->result = TRXSM_FAILURE;
            }
            		
#if(CFG_MAC_LBTSM_ENABLED == 1)
                //SM_DISPATCH( (sm_t *)&lbtsm, &event );
#elseif(CFG_MAC_CCA_ENABLED == 1)
                /*start the pause duration timer*/
                event.trigger = (sm_trigger_t) CCASM_TRIGGER_SUSPEND;	
                SM_DISPATCH( (sm_t *)&ccasm, &event );
#endif                
            trxsm_process_mpm_eb_tx_conf(s);
            
            //sm_transit( (sm_t *) s, (sm_state_t) &trxsm_idle );	
	    trxsm_transit((sm_state_t)&trxsm_idle);
            
            
            break;

        case TRXSM_TRIGGER_CANCEL:

            /* requeue beacon */
			if( !(mac_pib.mac_security_enabled) )
			{
				mac_queue_beacon_transmission( s->packet, MAC_FRAME_BCN_SUB_TYPE_MPM_EB );
				s->packet = NULL_POINTER;
			}
            else
			{
				mac_mem_free_tx(s->packet);
				s->packet = NULL_POINTER;
#if(CFG_MAC_BEACON_ENABLED == 1)                                
				mac_beacon_update(MAC_FRAME_BCN_SUB_TYPE_MPM_EB);
#endif                                
			}
			trxsm_cancel_curr_activities( s, e->param.scalar );
            
            break;

        default:
            break;
    }
    return NULL_POINTER;
}

/******************************************************************************/
#if (CFG_MAC_CCA_ENABLED == 1)
int trxsm_bcast_csma_default_case = 0;
void encrypt_data_packet (security_struct_t* p_security_data);
#if (UPDATE_UFSI_AFTER_CCA == 0) 
/*state: BCAST_CSMA*/
static sm_result_t trxsm_bcast_csma(trxsm_t *s, const sm_event_t *e)
{
  uint32_t bfio=0;
  uint32_t ufsi=0;
  uint32_t channel = 0;
  uint32_t TxChannel = 0;
  uint16_t len = 0;
  sm_event_t event;
//  ccasm_param_t cca_param = {0};
  
  switch ((trxsm_trigger_t) e->trigger)
  {
  case TRXSM_TRIGGER_ENTRY:
    s->state_ind = TRXSM_STATE_BCAST_CSMA;    
    /* indicate transmission to CCA state machine */
    event.trigger = (sm_trigger_t) CCASM_TRIGGER_TRANSMIT;
    SM_DISPATCH( s->ccasm, &event );
    break;
   
  case TRXSM_TRIGGER_CCA_MODE_1_REQ:
        set_frame_counter_and_nonce (s->packet);
        /* Debdeep :: Update UFSI and do security just before sending packet to phy */
        ufsi = update_ufsi_with_procces_time(s->packet);
        bfio =  update_bfio_with_procces_time(s->packet);
        if(s->packet->p_ufsi!=NULL)
        {
          memcpy(s->packet->p_ufsi,(uint8_t*)&ufsi,3);
        }
        if(s->packet->p_bfsi!=NULL)
        {
          memcpy(s->packet->p_bfsi,(uint8_t*)&bfio,3);
        }
        encrypt_data_packet (s->packet->security_data);
        
        if((mac_self_fan_info.unicast_listening_sched.us_schedule.channel_function == CF_DH1)
           || (mac_self_fan_info.unicast_listening_sched.us_schedule.channel_function == CF_TR51))
        {
          #if(FAN_EDFE_FEATURE_ENABLED == 1)
          if(edfe_information.edfe_frame_enabled == 0x01)
          {
            if(dont_change_ubcast_channel == 0x89)
            {
              channel = set_channel_for_freq_hop(s);
              s->packet->data->TxChannel = channel;
            }
            else
            {
              s->packet->data->TxChannel = phy_pib.CurrentChannel;
            }
            if((mac_self_fan_info.unicast_listening_sched.us_schedule.channel_function != CF_FIXED_CHANNEL)
               &&!( s->packet->tx_options & ACKNOWLEDGED_TRANSMISSION))
            {
              dont_change_ubcast_channel = 0x88;
            }
            PLME_Set_TRX_State( PHY_TX_ON );
            PD_Data_Request( s, s->packet->data );
            break;
          }
          else
#endif
          {
            if(s->packet->dst.address_mode == ADDR_MODE_NONE)
            {
              PLME_get_request( phyCurrentChannel, &len, &TxChannel );
              channel = TxChannel;  
            } 
            else
            {
              channel = set_channel_for_freq_hop(s);
            }
            s->packet->data->TxChannel = channel;
            if((mac_self_fan_info.unicast_listening_sched.us_schedule.channel_function != CF_FIXED_CHANNEL)
               && (s->packet->tx_options & ACKNOWLEDGED_TRANSMISSION ))
            {
              dont_change_ubcast_channel = 0x88;
            }
          }
        }  
        PLME_set_request(phyCurrentChannel,2,&channel);
        event.trigger = (sm_trigger_t) CCASM_CCA_TRIGGER_REQUEST;
        SM_DISPATCH( (sm_t *)&ccasm, &event );
    break;
    
  case TRXSM_TRIGGER_CCA_DONE:          
    switch ((ccasm_result_t)(e->param.scalar))
    {
    case CCASM_SUCCESS:
//#if (PRINT_DEBUG_LEVEL==ERROR_DEBUG)
//      stack_print_debug ("trxsm_bcast_csma CCASM_SUCCESS\n");
//#endif        
      /*Suneet :: if */
      /* ready to send */
      trxsm_transit((sm_state_t)&trxsm_cap_tx_send);
      break;
      
    case CCASM_CHANNEL_BUSY:
#if (PRINT_DEBUG_LEVEL==ERROR_DEBUG)
      stack_print_debug ("trxsm_bcast_csma CCASM_CHANNEL_BUSY\n");
#endif               
      /*NOTE: The frame should be dropped at once without retries as being done here. 
      This is as per the WI-SUN ENET profile for broadcast frames. 
      For now let this be retried......*/
      /* set status of packet */
      s->packet->status = MAC_CHANNEL_ACCESS_FAILURE;
      /* increase retry count and requeue */
      trxsm_requeue( s, 1 );
      /* try again next time */
      trxsm_transit((sm_state_t)&trxsm_idle);
      break;
      
    case CCASM_ERROR:/*fall through*/
    default:
#if (PRINT_DEBUG_LEVEL==ERROR_DEBUG)
      stack_print_debug ("trxsm_bcast_csma CCASM_ERROR\n");
#endif
      break;
    }
    break;
    
  case TRXSM_TRIGGER_CANCEL:
//#if (PRINT_DEBUG_LEVEL==ERROR_DEBUG)
//    stack_print_debug ("trxsm_bcast_csma TRXSM_TRIGGER_CANCEL\n");
//#endif          
    s->result = TRXSM_NONE;
    /* requeue packet */
    trxsm_requeue( s, 0 );
    trxsm_cancel_curr_activities( s, e->param.scalar );
    break;
    
  default:
    trxsm_bcast_csma_default_case++;
    break;
  }
  return NULL_POINTER; 
}
#endif
#endif  //CFG_MAC_CCA_ENABLED

/******************************************************************************/
//
///* state: BCAST_SEND*/
//static sm_result_t trxsm_bcast_send(trxsm_t *s, const sm_event_t *e)
//{
//   // sm_event_t event = { (sm_trigger_t) CCASM_TRIGGER_SUSPEND, { 0 } };
//    sm_event_t event;
//#if(CFG_MAC_CCA_ENABLED == 1)  
//    event.trigger = (sm_trigger_t) CCASM_TRIGGER_SUSPEND;
//#endif
//    
//	switch ((trxsm_trigger_t) e->trigger)
//    {
//        case TRXSM_TRIGGER_ENTRY:
//            s->state_ind = TRXSM_STATE_BCAST_SEND;
//
//#ifndef EFM32_TARGET_IAR
//            lpmsm_set_flag( LPMSM_FLAG_TX_ON );
//#endif
//			PLME_Set_TRX_State( PHY_TX_ON );
//            PD_Data_Request(s, s->packet->data);
//            break;
//
//        case TRXSM_TRIGGER_PD_DATA_CONFIRM:
//            /*start the pause duration timer*/
//          
//#if(CFG_MAC_LBTSM_ENABLED == 1)
//                SM_DISPATCH( (sm_t *)&lbtsm, &event );
//#else
//                SM_DISPATCH( (sm_t *)&ccasm, &event );
//#endif 
//        
//            if( ((phy_tx_info_t *)e->param.vector)->status == TRXSM_SUCCESS )
//            {
//                s->result = TRXSM_SUCCESS;
//
//                /* set status of packet */
//                s->packet->status = MAC_SUCCESS;
//                s->packet->tx_timestamp = ((phy_tx_info_t *)e->param.vector)->tx_ts_us;
//                
//                /* put to queue of completed messages */
//                trxsm_enqueue_completed_msg(s);
//            }
//            else
//            {
//                s->result = TRXSM_FAILURE;
//
//                /* increase retry count and requeue */
//                trxsm_requeue( s, 1 );
//            }
//            trxsm_transit_cap_tx_csma( s );
//            break;
//
//        case TRXSM_TRIGGER_CANCEL:
//
//            s->result = TRXSM_NONE;
//
//            /* requeue packet */
//            trxsm_requeue( s, 0 );
//
//            trxsm_cancel_curr_activities( s, e->param.scalar );
//
//            break;
//
//        default:
//            break;
//    }
//    return NULL_POINTER;
//}

/******************************************************************************/
/* Debdeep removed commented code in this function on 12-July-2018 */
/*state: WAIT_BCN_WAIT*/
static sm_result_t trxsm_wait_bcn_wait(trxsm_t *s, const sm_event_t *e)
{
  mac_tx_t *packet = NULL;
  switch ((trxsm_trigger_t) e->trigger)
  {
  case TRXSM_TRIGGER_ENTRY:
    s->state_ind = TRXSM_STATE_WAIT_BCN_WAIT;
#ifdef UTEST_TRXSM
    utu_timestamp(UTUL_TRXSM_WAIT_BCN, s->tmp);
#endif
#ifndef EFM32_TARGET_IAR
    /* inform power management */
    lpmsm_set_flag( LPMSM_FLAG_RX_ON );
#endif
    
    /* turn receiver on */
    if( plme_get_trx_state_request() == PHY_TX_ON )
    {
      //PLME_Set_TRX_State( PHY_FORCE_TRX_OFF ); 
    }

    /* reset next packet of direct queue */
    if( (packet = (mac_tx_t *) queue_peek( s->direct_queue )) != NULL_POINTER )
      packet->status = MAC_SUCCESS;
    
    /* set alarm for timeout */
    s->fn_alarm = &trxsm_alarm_wait_bcn_wait;
#if RAIL_TIMER_INTERFACE_USED    
    s->sw_timer.relPeriodic = s->tmp * HWTIMER_SYMBOL_LENGTH;
#else
    s->sw_timer.period = s->tmp * HWTIMER_SYMBOL_LENGTH;
#endif    
    
#if RAIL_TIMER_INTERFACE_USED
      if( !RAIL_SetMultiTimer(&s->sw_timer,s->sw_timer.relPeriodic,RAIL_TIME_DELAY,&trxsm_alarm,s))
#else      
    if( !tmr_start_relative(&(s->sw_timer)))
#endif 
    {
      /* TBD What to do here? */
#ifdef UTEST_TRXSM
      utu_timestamp(UTUL_ERR_CLIENT, -1);
#endif
    }
    s->result = TRXSM_SUCCESS;
    PLME_Set_TRX_State( PHY_RX_ON );
    break;
    
  case TRXSM_TRIGGER_PD_DATA_INDICATION:
    phy_state_event_execute++;
    PLME_Set_TRX_State( PHY_RX_ON );
    break;
    
  case TRXSM_TRIGGER_BCN_RECEIVED:
#if RAIL_TIMER_INTERFACE_USED
    RAIL_CancelMultiTimer(&s->sw_timer);
#else  
    tmr_stop
      ( 
       &(s->sw_timer)
         );
#endif  
    /* set result */
    s->result = TRXSM_SUCCESS;
    //trxsm_transit_cap_tx_csma( s );
    break;
    
  case TRXSM_TRIGGER_ALARM:
    s->result = TRXSM_FAILURE;
#ifdef MAC_CFG_BEACONING_ENABLED
    /* indicate missed beacon to SFT-SM */
    if( s->sftsm )
    {
      event.trigger = (sm_trigger_t) SFTSM_TRIGGER_BCN_MISSED;
      event.param.scalar = 0;
      SM_DISPATCH( (sm_t *)(s->sftsm), &event );
    }
#endif            
#if(CFG_MAC_SCANSM_ENABLED == 1)  
    if( scansm_p->state_ind &&
       scansm_get_state( scansm_p ) != SCANSM_STATE_NONE )
    {
      /* go to IDLE state if scanning */
      trxsm_transit((sm_state_t)&trxsm_idle);				
    }
    //else
      //trxsm_transit_cap_tx_csma( s );
#else
   // trxsm_transit_cap_tx_csma( s );
#endif    // #if(CFG_MAC_SCANSM_ENABLED == 1)     
    
    //// following code moved from the timer call back fucntion
#ifdef MAC_CFG_BEACONING_ENABLED
    /* indicate lost beacon to SYNC-SM */
    if( syncsm_p )
    {
      event.trigger = (sm_trigger_t) SYNCSM_TRIGGER_BCN_LOST;
      SM_DISPATCH( (sm_t *) syncsm_p, &event );
    }
#endif
    s->result = TRXSM_FAILURE;
#if(CFG_MAC_SCANSM_ENABLED == 1)              
    /* indicate timeout to SCAN-SM */
    if( scansm_p )
    {
      event.trigger = (sm_trigger_t) SCANSM_TRIGGER_TIMEOUT;
      SM_DISPATCH( (sm_t *)(scansm_p), &event );
    }
#endif            
    s->result = TRXSM_SUCCESS;
    break;
    
  case TRXSM_TRIGGER_ACK_REQUIRED:
    /*Sending acknowledgments for the received packets while 
    performing scanning*/
#if(CFG_MAC_SFTSM_ENABLED == 1)  			
    /*TBD Do we really need this guard here? */
    if( ( s->sftsm == NULL_POINTER )          
#ifdef MAC_CFG_BEACONING_ENABLED	    
       || ( sftsm_get_state( s->sftsm ) == SFTSM_STATE_CAP )		    /* ...in CAP */
#endif
         )                 /* ...in CAP */
    {
      /* set ack as the current packet */
      s->packet = e->param.vector;
      sm_back_up_state( (sm_t *)s );
      trxsm_transit((sm_state_t)&trxsm_cap_ack_tx_send);
    }
#endif      //  (CFG_MAC_SFTSM_ENABLED == 1)            
    break;
    
  case TRXSM_TRIGGER_CANCEL:
    trxsm_cancel_curr_activities( s, e->param.scalar );
    break;
    
  default:
    break;
  }
  return NULL_POINTER;
}

/******************************************************************************/

/*state: WAIT_MPM_EB_WAIT*/
static sm_result_t trxsm_wait_mpm_eb_wait(trxsm_t *s, const sm_event_t *e)
{
        mac_tx_t *packet = NULL;
        ulong current_channel = 0;
        uint16_t len = 0; 

    switch ((trxsm_trigger_t) e->trigger)
    {
        case TRXSM_TRIGGER_ENTRY:
            s->state_ind = TRXSM_STATE_WAIT_MPM_EB_WAIT;
#ifdef UTEST_TRXSM
            utu_timestamp(UTUL_TRXSM_WAIT_MPM_EB, s->tmp);
#endif
#ifndef EFM32_TARGET_IAR
            /* inform power management */
           lpmsm_set_flag( LPMSM_FLAG_RX_ON );
#endif

            /* turn receiver on */
            if( plme_get_trx_state_request() == PHY_TX_ON )
            {
                //PLME_Set_TRX_State( PHY_FORCE_TRX_OFF );
            }

		PLME_get_request( phyCurrentChannel, &len, &current_channel );
		PLME_Get_CSM_Unit_Radio_Chan_List
		(
			(uint16_t)current_channel,
			&(s->CSMCurrTotalUnitRadioChanCount),
			s->CSMUnitRadioChanList
		);
		
		/* Switch ON the CSM */
		trxsm_activate_csm( s,(uint16_t)current_channel );

		/*select the first CSM unit channel on which scanning should be done */
		current_channel = s->CSMUnitRadioChanList[ s->iUnitRadioChan++ ];

		/* set the CSM unit radio channel */
		PLME_set_request( phyCurrentChannel, 2, &current_channel );
	/*AAAAAAAAAAAAAAAA: what if the channel setting failed.*/
            PLME_Set_TRX_State( PHY_RX_ON );

            /* reset next packet of direct queue */
            if( (packet = (mac_tx_t *) queue_peek( s->direct_queue )) != NULL_POINTER )
            {
                packet->status = MAC_SUCCESS;
            }

            /* set alarm for timeout */
            s->fn_alarm = &trxsm_alarm_wait_mpm_eb_wait; 
            
#if RAIL_TIMER_INTERFACE_USED    
            s->sw_timer.relPeriodic = s->tmp * HWTIMER_SYMBOL_LENGTH;
#else
            s->sw_timer.period = s->tmp * HWTIMER_SYMBOL_LENGTH;
#endif    
            
#if RAIL_TIMER_INTERFACE_USED
            if( !RAIL_SetMultiTimer(&s->sw_timer,s->sw_timer.relPeriodic,RAIL_TIME_DELAY,&trxsm_alarm,s))
#else      
              if( !tmr_start_relative(&(s->sw_timer)))
#endif 
              {
                /* TBD What to do here? */
#ifdef UTEST_TRXSM
                utu_timestamp(UTUL_ERR_CLIENT, -1);
#endif
            }
	    /*bug: what if channel setting failed????*/
	    s->result = TRXSM_SUCCESS;
            break;
        case TRXSM_TRIGGER_PD_DATA_INDICATION:
                phy_state_event_execute++;
        	PLME_Set_TRX_State( PHY_RX_ON );
        	break;

        case TRXSM_TRIGGER_ALARM:
            s->result = TRXSM_FAILURE;

			if( s->iUnitRadioChan < s->CSMCurrTotalUnitRadioChanCount )
			{
				/*select the next unit radio channel on which scanning should be done */
				current_channel = s->CSMUnitRadioChanList[ s->iUnitRadioChan++ ];

				/* set the CSM unit radio channel */
				PLME_set_request( phyCurrentChannel, 2, &current_channel );

#if RAIL_TIMER_INTERFACE_USED    
                                s->sw_timer.relPeriodic = s->tmp * HWTIMER_SYMBOL_LENGTH;
#else
                                s->sw_timer.period = s->tmp * HWTIMER_SYMBOL_LENGTH;
#endif    
                                
#if RAIL_TIMER_INTERFACE_USED
                                if( !RAIL_SetMultiTimer(&s->sw_timer,s->sw_timer.relPeriodic,RAIL_TIME_DELAY,&trxsm_alarm,s))
#else      
                                  if( !tmr_start_relative(&(s->sw_timer)))
#endif 
				{
					/* TBD What to do here? */
	#ifdef UTEST_TRXSM
					utu_timestamp(UTUL_ERR_CLIENT, -1);
	#endif
				}
			}
			else
			{
				trxsm_deactivate_csm(s);
				/* go to IDLE state if scanning */
                            //sm_transit((sm_t *)s, (sm_state_t) &trxsm_idle);
                            trxsm_transit((sm_state_t)&trxsm_idle);
#if(CFG_MAC_SCANSM_ENABLED == 1)                  
			    /* indicate timeout to SCAN-SM */							
			    event.trigger = (sm_trigger_t) SCANSM_TRIGGER_TIMEOUT;
			    event.param.scalar = 0;
			    SM_DISPATCH( (sm_t *)(scansm_p), &event );
#endif				
				
				
    
			}
            break;

        case TRXSM_TRIGGER_CANCEL:

            /* cancel alarm for timeout */
            //alarm_unset( &s->alarm );
            trxsm_cancel_curr_activities( s, e->param.scalar );
			
            break;

        default:
            break;
    }
    return NULL_POINTER;
}

/******************************************************************************/
int trxsm_cap_tx_csma_default_event = 0;
extern trxsm_t trxsm;
/* Debdeep removed commented code in this function on 12-July-2018 */
/* Debdeep added case TRXSM_TRIGGER_PD_DATA_INDICATION on 12-July-2018 */
/*state: CAP_TX_CSMA*/
static sm_result_t trxsm_cap_tx_csma(trxsm_t *s, const sm_event_t *e)
{
#if (CFG_MAC_CCA_ENABLED == 1)  
  uint32_t bfio=0;
  uint32_t ufsi=0;
  uint32_t channel = 0;
  uint32_t TxChannel = 0;
  uint16_t len = 0; 
  // uint8_t dest[8] = {0};
  sm_event_t event;
//  ccasm_param_t cca_param = {0};
  
  switch ((trxsm_trigger_t) e->trigger)
  {
  case TRXSM_TRIGGER_ENTRY:
    s->state_ind = TRXSM_STATE_CAP_TX_CSMA;    
//#if (PRINT_DEBUG_LEVEL==ERROR_DEBUG)
//    stack_print_debug ("trxsm_cap_tx_csma ENTRY\n");
//#endif    
    /* indicate transmission to CCA state machine */
    event.trigger = (sm_trigger_t) CCASM_TRIGGER_TRANSMIT;
//    event.param.vector = &cca_param;
//    create_cca_event (s, &cca_param);    
    SM_DISPATCH( s->ccasm, &event );
    break;
    
  case TRXSM_TRIGGER_CCA_MODE_1_REQ:
    switch(s->packet->sub_type)
    {
    case EAPOL:
//      mem_rev_cpy (dest, s->packet->dst.address.ieee_address, 8);
//#if (APP_LBR_ROUTER == 1)             
//      update_link_stats_on_tx(dest);
//#endif    
//      if((mac_self_fan_info.unicast_listening_sched.us_schedule.channel_function == CF_DH1)
//         || (mac_self_fan_info.unicast_listening_sched.us_schedule.channel_function == CF_TR51))
//      {
//        ufsi = update_ufsi_with_procces_time(s->packet);
//        bfio =  update_bfio_with_procces_time(s->packet);
//        if(s->packet->p_ufsi!=NULL)
//        {
//          memcpy(s->packet->p_ufsi,(uint8_t*)&ufsi,3);
//        }
//        if(s->packet->p_bfsi!=NULL)
//        {
//          memcpy(s->packet->p_bfsi,(uint8_t*)&bfio,3);
//        }
//        channel = set_channel_for_freq_hop(s);             
//        if((mac_self_fan_info.unicast_listening_sched.us_schedule.channel_function != CF_FIXED_CHANNEL)
//           &&( s->packet->tx_options & ACKNOWLEDGED_TRANSMISSION) )
//        {
//          dont_change_ubcast_channel = 0x88;
//        }
//        s->packet->data->TxChannel = channel;
//      }
//      PLME_set_request(phyCurrentChannel,2,&channel);
//      event.trigger = (sm_trigger_t) CCASM_CCA_TRIGGER_REQUEST;
//      SM_DISPATCH( (sm_t *)&ccasm, &event );
      break;//end EAPOL
    case FAN_DATA_PKT:
#if (APP_LBR_ROUTER == 1)
      set_frame_counter_and_nonce (s->packet);
      /* Debdeep :: Update UFSI and do security just before sending packet to phy */
      ufsi = update_ufsi_with_procces_time(s->packet);
      bfio =  update_bfio_with_procces_time(s->packet);
      if(s->packet->p_ufsi!=NULL)
      {
        memcpy(s->packet->p_ufsi,(uint8_t*)&ufsi,3);
      }
      if(s->packet->p_bfsi!=NULL)
      {
        memcpy(s->packet->p_bfsi,(uint8_t*)&bfio,3);
      }
      encrypt_data_packet (s->packet->security_data);
      if((mac_self_fan_info.unicast_listening_sched.us_schedule.channel_function == CF_DH1)
         || (mac_self_fan_info.unicast_listening_sched.us_schedule.channel_function == CF_TR51))
      {
        #if(FAN_EDFE_FEATURE_ENABLED == 1)
        if(edfe_information.edfe_frame_enabled == 0x01)
        {
          if(dont_change_ubcast_channel == 0x89)
          {
            channel = set_channel_for_freq_hop(s);
            s->packet->data->TxChannel = channel;
          }
          else
          {
            s->packet->data->TxChannel = phy_pib.CurrentChannel;
          }
          if((mac_self_fan_info.unicast_listening_sched.us_schedule.channel_function != CF_FIXED_CHANNEL)
             &&!( s->packet->tx_options & ACKNOWLEDGED_TRANSMISSION))
          {
            dont_change_ubcast_channel = 0x88;
          }
          PLME_Set_TRX_State( PHY_TX_ON );
          PD_Data_Request( s, s->packet->data );
          break;
        }
        else
#endif
        {
          if(s->packet->dst.address_mode == ADDR_MODE_NONE)
          {
            PLME_get_request( phyCurrentChannel, &len, &TxChannel );
            channel = TxChannel;  
          } 
          else
          {
            channel = set_channel_for_freq_hop(s);
          }
          s->packet->data->TxChannel = channel;
          if((mac_self_fan_info.unicast_listening_sched.us_schedule.channel_function != CF_FIXED_CHANNEL)
             && (s->packet->tx_options & ACKNOWLEDGED_TRANSMISSION ))
          {
            dont_change_ubcast_channel = 0x88;
          }
        }
      }
#endif     
      PLME_set_request(phyCurrentChannel,2,&channel);
      event.trigger = (sm_trigger_t) CCASM_CCA_TRIGGER_REQUEST;
      SM_DISPATCH( (sm_t *)&ccasm, &event );
      break;//end FAN_DATA_PKT  
    }
    break;
    
  case TRXSM_TRIGGER_CCA_DONE:
    switch ((ccasm_result_t)(e->param.scalar))
    {
    case CCASM_SUCCESS:
//#if (PRINT_DEBUG_LEVEL==ERROR_DEBUG)
//    stack_print_debug ("trxsm_cap_tx_csma CCASM_SUCCESS\n");
//#endif         
      /* set status of packet */
      s->packet->status = MAC_SUCCESS;
      if( (s->packet->type == MAC_FRAME_TYPE_BEACON ) && 
         ( MAC_FRAME_BCN_SUB_TYPE_MPM_EB == s->packet->sub_type ) )
        trxsm_transit((sm_state_t)&trxsm_mpm_eb_tx_send);
#ifdef ENHANCED_ACK_SUPPORT		
      else if ( s->packet->type == MAC_FRAME_TYPE_ACK )
        trxsm_transit((sm_state_t)&trxsm_cap_enack_tx_send);
#endif		//ENHANCED_ACK_SUPPORT			
      else
      {
        /* ready to send */
#if (UPDATE_UFSI_AFTER_CCA == 0)
        //suneet :: send pkt after pkt sucess 
        trxsm_transit((sm_state_t)&trxsm_cap_tx_send);
#else
        trxsm_transit((sm_state_t)&trxsm_idle);
        mac_queue_direct_transmission(trxsm.packet);
#endif
      }
      break;
      
    case CCASM_CHANNEL_BUSY:
#if (PRINT_DEBUG_LEVEL==ERROR_DEBUG)
    stack_print_debug ("trxsm_cap_tx_csma CCASM_CHANNEL_BUSY\n");
#endif          
      bcn_csm_ca_failed++;
      /* set status of packet */
      s->packet->status = MAC_CHANNEL_ACCESS_FAILURE;
      /* requeue packet */
      trxsm_requeue( s, 1 );
      /* start again with next packet */
      trxsm_transit((sm_state_t)&trxsm_idle);
      break;
      
    case CCASM_ERROR:
    default:
#if (PRINT_DEBUG_LEVEL==ERROR_DEBUG)
    stack_print_debug ("trxsm_cap_tx_csma CCASM_ERROR\n");
#endif
      break;
    }
    break;
    
  case TRXSM_TRIGGER_CANCEL:
#if (PRINT_DEBUG_LEVEL==ERROR_DEBUG)
    stack_print_debug ("trxsm_cap_tx_csma TRXSM_TRIGGER_CANCEL\n");
#endif     
    s->result = TRXSM_NONE;
    /* requeue packet */
    trxsm_requeue( s, 0 );
    trxsm_cancel_curr_activities( s, e->param.scalar );
    break;
    
    /* Debdeep:: 12-July-2018 */
  case TRXSM_TRIGGER_PD_DATA_INDICATION:
    PLME_Set_TRX_State (PHY_RX_ON);   
//#if (PRINT_DEBUG_LEVEL==ERROR_DEBUG)
//    stack_print_debug ("TRXSM_TRIGGER_PD_DATA_INDICATION in trxsm_cap_tx_csma\n");
//#endif
    break;
    
  default:
    trxsm_cap_tx_csma_default_event++;
    break;
  }

#endif    
    return NULL_POINTER;
}

/******************************************************************************/

#ifdef ENHANCED_ACK_SUPPORT
static sm_result_t trxsm_cap_enack_tx_send(trxsm_t *s, const sm_event_t *e)
{
    sm_event_t event;

    event.trigger = (sm_trigger_t) CCASM_TRIGGER_SUSPEND;

    switch ((trxsm_trigger_t) e->trigger)
    {
      case TRXSM_TRIGGER_ENTRY:
      s->state_ind = TRXSM_STATE_CAP_TX_SEND;
#ifndef EFM32_TARGET_IAR
      lpmsm_set_flag( LPMSM_FLAG_TX_ON );
#endif
      PLME_Set_TRX_State( PHY_TX_ON );

      PD_Data_Request( s, s->packet->data );
      break;

      case TRXSM_TRIGGER_PD_DATA_CONFIRM:
      if( ((phy_tx_info_t *)e->param.vector)->status == TRXSM_SUCCESS )
      {            	              
        s->result = TRXSM_SUCCESS;
        /*start the pause duration timer*/
        SM_DISPATCH( (sm_t *)&lbtsm, &event );
        s->packet->tx_timestamp = ((phy_tx_info_t *)e->param.vector)->tx_ts_us;
        /* set status of packet */
        s->packet->status = MAC_SUCCESS;
                 
        trxsm_enqueue_completed_msg(s);

        s->packet = NULL_POINTER;
        trxsm_transit((sm_state_t)&trxsm_idle);
      }
      else
      {
        s->result = TRXSM_FAILURE;
        /*start the pause duration timer*/
        SM_DISPATCH( (sm_t *)&lbtsm, &event );

        /* increase retry count and requeue */
        trxsm_requeue( s, 1 );
        /* try again */
        //trxsm_transit_cap_tx_csma( s );
      }
      break;

      case TRXSM_TRIGGER_CANCEL:
      s->result = TRXSM_NONE;
      /* requeue packet */
      trxsm_requeue( s, 0 );
      trxsm_cancel_curr_activities( s, e->param.scalar );
      break;

      default:
      break;
    }
    return NULL_POINTER;
}
#endif //ENHANCED_ACK_SUPPORT

/******************************************************************************/

void subscribe_tx_frame (mac_tx_t *txd);
/*state: CAP_TX_SEND*/
//extern uint8_t debug_sun_arr[2];
//uint64_t pckt_out[50];
//extern int hopping_test_index;
//uint64_t get_time_now_64 (void);
static sm_result_t trxsm_cap_tx_send(trxsm_t *s, const sm_event_t *e)
{
  sm_event_t event;
  /*Uemsh : 11-01-2018 this thing goes to modified after sepration of mac and fan mac*/
#ifdef WISUN_FAN_MAC
  uint32_t unicast_hop_channel = 0;
  int unicast_channel_index = 0;
  int brodcast_channel_index = 0;
  uint32_t broadcast_hop_channel = 0;
  //uint16_t len = 0;
  uint16_t symbol_length = HWTIMER_SYMBOL_LENGTH;
  uint32_t bfio=0;
  uint32_t ufsi=0;
  //uint32_t TxChannel = 0;
#endif
//  uint8_t dest[8] = {0};
#if (CFG_MAC_CCA_ENABLED == 1)  
  event.trigger = (sm_trigger_t) CCASM_TRIGGER_SUSPEND;
#endif  
  
//#if (PRINT_DEBUG_LEVEL==ERROR_DEBUG)
//  stack_print_debug ("S phy state = %d\n", plme_get_trx_state_request ());
//#endif     
  
  switch ((trxsm_trigger_t) e->trigger)
  {
  case TRXSM_TRIGGER_ENTRY:
    {
      s->state_ind = TRXSM_STATE_CAP_TX_SEND;
#ifndef EFM32_TARGET_IAR
      lpmsm_set_flag( LPMSM_FLAG_TX_ON );
#endif
      /*Uemsh : 11-01-2018 this thing goes to modified after sepration of mac and fan mac*/
#ifdef WISUN_FAN_MAC
      mem_rev_cpy(node_address,s->packet->src.address.ieee_address,8);
      uint32_t channel = s->packet->data->TxChannel;          
      
      switch(s->packet->sub_type)
      {
        
#if(FAN_EAPOL_FEATURE_ENABLED == 1)
      case EAPOL:
//        mem_rev_cpy (dest, s->packet->dst.address.ieee_address, 8);
//#if (APP_LBR_ROUTER == 1)             
//        update_link_stats_on_tx(dest);
//#endif    
//        if((mac_self_fan_info.unicast_listening_sched.us_schedule.channel_function == CF_DH1)
//           || (mac_self_fan_info.unicast_listening_sched.us_schedule.channel_function == CF_TR51))
//        {
//          ufsi = update_ufsi_with_procces_time(s->packet);
//          bfio =  update_bfio_with_procces_time(s->packet);
//          if(s->packet->p_ufsi!=NULL)
//          {
//            memcpy(s->packet->p_ufsi,(uint8_t*)&ufsi,3);
//          }
//          if(s->packet->p_bfsi!=NULL)
//          {
//            memcpy(s->packet->p_bfsi,(uint8_t*)&bfio,3);
//          }
//          channel = set_channel_for_freq_hop(s);             
//          if((mac_self_fan_info.unicast_listening_sched.us_schedule.channel_function != CF_FIXED_CHANNEL)
//             &&( s->packet->tx_options & ACKNOWLEDGED_TRANSMISSION) )
//          {
//            dont_change_ubcast_channel = 0x88;
//          }
//          s->packet->data->TxChannel = channel;
//        }
//        trxsm_transit((sm_state_t)&trxsm_cap_tx_csma);
//        event.trigger = (sm_trigger_t) CCASM_CCA_TRIGGER_REQUEST;
//        SM_DISPATCH( (sm_t *)&ccasm, &event );
        PLME_Set_TRX_State( PHY_TX_ON );
        PD_Data_Request( s, s->packet->data );
//#if (PRINT_DEBUG_LEVEL == CONSOLE_ERROR_DEBUG)
//        stack_print_debug ("Tx-EAPOL\n");
//#endif
        break;//end EAPOL
#endif // #if(FAN_EAPOL_FEATURE_ENABLED == 1)
        
      case FAN_DATA_PKT:
//#if (APP_LBR_ROUTER == 1)
        set_frame_counter_and_nonce (s->packet);
//        /* Debdeep :: Update UFSI and do security just before sending packet to phy */
        ufsi = update_ufsi_with_procces_time(s->packet);
        bfio =  update_bfio_with_procces_time(s->packet);
        if(s->packet->p_ufsi!=NULL)
        {
          memcpy(s->packet->p_ufsi,(uint8_t*)&ufsi,3);
        }
        if(s->packet->p_bfsi!=NULL)
        {
          memcpy(s->packet->p_bfsi,(uint8_t*)&bfio,3);
        }
        encrypt_data_packet (s->packet->security_data);
//        if((mac_self_fan_info.unicast_listening_sched.us_schedule.channel_function == CF_DH1)
//           || (mac_self_fan_info.unicast_listening_sched.us_schedule.channel_function == CF_TR51))
//        {
//          if(edfe_information.edfe_frame_enabled == 0x01)
//          {
//            if(dont_change_ubcast_channel == 0x89)
//            {
//              channel = set_channel_for_freq_hop(s);
//              s->packet->data->TxChannel = channel;
//            }
//            else
//            {
//              s->packet->data->TxChannel = phy_pib.CurrentChannel;
//            }
//            if((mac_self_fan_info.unicast_listening_sched.us_schedule.channel_function != CF_FIXED_CHANNEL)
//               &&!( s->packet->tx_options & ACKNOWLEDGED_TRANSMISSION))
//            {
//              dont_change_ubcast_channel = 0x88;
//            }
//            PLME_Set_TRX_State( PHY_TX_ON );
//            PD_Data_Request( s, s->packet->data );
//            break;
//          }
//          else
//          {
//            if(s->packet->dst.address_mode == ADDR_MODE_NONE)
//            {
//              PLME_get_request( phyCurrentChannel, &len, &TxChannel );
//              channel = TxChannel;  
//            } 
//            else
//            {
//              channel = set_channel_for_freq_hop(s);
//            }
            s->packet->data->TxChannel = channel;
//            if((mac_self_fan_info.unicast_listening_sched.us_schedule.channel_function != CF_FIXED_CHANNEL)
//               && (s->packet->tx_options & ACKNOWLEDGED_TRANSMISSION ))
//            {
//              dont_change_ubcast_channel = 0x88;
//            }
//          }
//        }
//#endif     
//        if(s->packet->dst.address_mode == 0x03)
//          trxsm_transit((sm_state_t)&trxsm_cap_tx_csma);
//        else
//           trxsm_transit((sm_state_t)&trxsm_bcast_csma);
//        
//        event.trigger = (sm_trigger_t) CCASM_CCA_TRIGGER_REQUEST;
//        SM_DISPATCH( (sm_t *)&ccasm, &event );
        PLME_Set_TRX_State( PHY_TX_ON );
        PD_Data_Request( s, s->packet->data );
//        if (hopping_test_index < 50)
//        {
//          pckt_out[hopping_test_index++] = get_time_now_64 ();
//        }  
//#if (PRINT_DEBUG_LEVEL == CONSOLE_ERROR_DEBUG)
//        stack_print_debug ("Tx-DATA [%d] to ", s->packet->sn);
//        print_mac_address (s->packet->dst.address.ieee_address);
//#endif
        break;//end FAN_DATA_PKT             
        
      case PAN_ADVERT_FRAME:/*Fall through*/
      case PAN_ADVERT_SOLICIT:/*Fall through*/              
      case PAN_CONFIG_SOLICIT:/*Fall through*/          
#if(FAN_FRQ_HOPPING_FEATURE_ENABLED == 1)
        if((mac_self_fan_info.unicast_listening_sched.us_schedule.channel_function == CF_DH1)
           || (mac_self_fan_info.unicast_listening_sched.us_schedule.channel_function == CF_TR51))
        {
          ufsi = update_ufsi_with_procces_time(s->packet);
          if(s->packet->p_ufsi!=NULL)
          {
            memcpy(s->packet->p_ufsi,(uint8_t*)&ufsi,3);
          }
          s->packet->data->TxChannel = channel;
        }
#endif
        
        PLME_Set_TRX_State( PHY_TX_ON );
        PD_Data_Request( s, s->packet->data );
        break;
        
      case PAN_CONFIG:
        set_frame_counter_and_nonce (s->packet);
        /* Debdeep :: Update UFSI and do security just before sending packet to phy */
        ufsi = update_ufsi_with_procces_time(s->packet);
        bfio =  update_bfio_with_procces_time(s->packet);
        if(s->packet->p_ufsi!=NULL)
        {
          memcpy(s->packet->p_ufsi,(uint8_t*)&ufsi,3);
        }
        if(s->packet->p_bfsi!=NULL)
        {
          memcpy(s->packet->p_bfsi,(uint8_t*)&bfio,3);
        }
        
        encrypt_data_packet (s->packet->security_data);
        
        PLME_Set_TRX_State( PHY_TX_ON );
        s->packet->data->TxChannel = channel;
        PD_Data_Request( s, s->packet->data );
        break;
        
      case  FAN_ACK:
        PLME_Set_TRX_State( PHY_TX_ON );
        PD_Data_Request( s, s->packet->data );
        break;    
      }
#else
      PLME_Set_TRX_State( PHY_TX_ON );
      PD_Data_Request( s, s->packet->data );
#endif            
      break;
    }         
    
  case TRXSM_TRIGGER_PD_DATA_CONFIRM:          
#ifdef WISUN_FAN_MAC/*Umesh : 21-02-2018*//*for sepration of 802.15.04*/  
    if((mac_self_fan_info.unicast_listening_sched.us_schedule.channel_function != CF_FIXED_CHANNEL)
       &&!( s->packet->tx_options & ACKNOWLEDGED_TRANSMISSION))
    {
#if(FAN_EDFE_FEATURE_ENABLED == 1)
      if(edfe_information.edfe_frame_enabled == 0x01)
      {
#if (APP_LBR_ROUTER == 1)
        if(edfe_information.edfe_frame_tx_type == 0x99)  //suneet :: send final frame
        {
          dont_change_ubcast_channel = 0x89;
          enable_disable_edfe_frame(0,255);
        }
        else
        {
          edfe_information.edfe_frame_rx_type = 0xFF;
          edfe_information.edfe_frame_tx_type = 0xFF;
          stop_edfe_transmit_timer();
          start_edfe_transmit_responce_timer(edfe_information.edfe_receiver_flow_contrl);
        }
#endif          
      }
      else
#endif  //#if(FAN_EDFE_FEATURE_ENABLED == 1)
      {
#if(FAN_FRQ_HOPPING_FEATURE_ENABLED == 1)
        if(is_UDI_active())
        {
          if(mac_self_fan_info.unicast_listening_sched.us_schedule.channel_function == CF_DH1)
          {
            unicast_channel_index = getUnicastChannelIndex(unicast_slot_no,node_address,usable_channel.total_usable_ch_unicast);
            unicast_hop_channel = usable_channel.unicast_usable_channel_list[unicast_channel_index];
          }
          PLME_set_request( phyCurrentChannel, 2, &unicast_hop_channel ); 

        }
        else
        {
          if(mac_self_fan_info.bcast_sched.bs_schedule.channel_function == CF_DH1)
          {
            if(fan_mac_information_data.fan_node_type == 0x01)
            {
              mac_nbr_descriptor_t *p_nbr_desc = NULL;
              #if ((APP_LBR_ROUTER == 1) && (FAN_EAPOL_FEATURE_ENABLED == 1))
                p_nbr_desc = get_nbr_desc_from_addr (eapol_parent_child_info.sle_eapol_parent);
              #endif
                if(p_nbr_desc != NULL)
                {
                  brodcast_channel_index = getBroadcastChannelIndex(broadcast_slot_nuumber,mac_self_fan_info.bcast_sched.bcast_sched_id,p_nbr_desc->nbrchannel_usable_list.total_usable_ch_broadcast);
                  broadcast_hop_channel =  p_nbr_desc->nbrchannel_usable_list.broad_usable_channel_list[brodcast_channel_index];
                }
            }
            else
            {
              brodcast_channel_index = getBroadcastChannelIndex(broadcast_slot_nuumber,mac_self_fan_info.bcast_sched.bcast_sched_id,usable_channel.total_usable_ch_broadcast);
              broadcast_hop_channel = usable_channel.broad_usable_channel_list[brodcast_channel_index];
            }
          }
            PLME_set_request( phyCurrentChannel, 2, &broadcast_hop_channel );
        }
#endif
      }
    }  
#endif
    if( ((phy_tx_info_t *)e->param.vector)->status == TRXSM_SUCCESS )
    {
      PLME_Set_TRX_State( PHY_RX_ON );
      s->packet->tx_timestamp = ((phy_tx_info_t *)e->param.vector)->tx_ts_us;
      if ( s->packet->tx_options & ACKNOWLEDGED_TRANSMISSION )
      {
#ifdef ENHANCED_ACK_SUPPORT
        /* put the packet in a seperate completed queue. 
        When we get an enhanced ack we can go through the queue 
        to see the received ack is for one of the queued packets */
        s->result = TRXSM_SUCCESS;
        
        /* set status of packet */
        s->packet->status = MAC_SUCCESS;
        
        mac_wait_for_enack(s->packet);
        s->packet = NULL_POINTER;
        
        /*start the pause duration timer*/
        SM_DISPATCH( (sm_t *)&lbtsm, &event );
        
        trxsm_transit((sm_state_t)&trxsm_idle);
#else
        trxsm_transit((sm_state_t)&trxsm_cap_tx_wait);
#endif		    
      }
      else
      {
        /*Uemsh : 11-01-2018 this thing goes to modified after sepration of mac and fan mac*/                   
        s->result = TRXSM_SUCCESS;
        
        /*start the pause duration timer*/
//#if (CFG_MAC_LBTSM_ENABLED == 1)
//        SM_DISPATCH( (sm_t *)&lbtsm, &event );
//#else
//        SM_DISPATCH( (sm_t *)&ccasm, &event );
//#endif 
        
        /* set status of packet */
        s->packet->status = MAC_SUCCESS;
        
        if( ( s->packet->type == MAC_FRAME_TYPE_BEACON )
           && ( mac_pib.BeaconAutoRespond ))
        {
          num_of_bcn_sent++;
#ifdef ENABLE_DEBUG_EVENTS                        
          Indicate_Debug_Event(UTUL_TX_BCN_SENT);
#endif
          /* increment BSN or EBSN accordingly */                       
          if( s->packet->sub_type == MAC_FRAME_BCN_SUB_TYPE_RB )
          {
            mac_pib.BSN += 1;
          }
          else
          {
            mac_pib.EBSN += 1;
          }
          
          s->packet->cap_retries = 0;
          
          if( !(mac_pib.mac_security_enabled) )
          {
            /* requeue new beacon. Do not construct and keep 
            the Enhanced beacons, as the contents of EB are 
            determined by the EBR */
            if( s->packet->sub_type == MAC_FRAME_BCN_SUB_TYPE_RB )
            {
              s->packet->data->psdu[2] = mac_pib.BSN;
              mac_queue_beacon_transmission( s->packet,s->packet->sub_type );
            }
            else
            {
              /*After transmitting EBs should be freed. New 
              EBs should not constructed as the contents are 
              determined by the receiving EBR.*/
              mac_mem_free_tx(s->packet);
            }							
          }
          else
          {
            /*since secured beacons are being sent, they have 
            to be re constructed. So free up the sent beacon 
            and construct it newly. Contruct a new beacon and 
            not a EB*/
            //sub_type = s->packet->sub_type;
            mac_mem_free_tx(s->packet);
            s->packet = NULL_POINTER;
#if(CFG_MAC_BEACON_ENABLED == 1)                 
            if( sub_type == MAC_FRAME_BCN_SUB_TYPE_RB )
            {
              mac_beacon_update( sub_type );	
            }
#endif                
          }
          s->packet = NULL_POINTER;
        }
        else
        {
          /* 
          1) This can be a data or command frame which should be put into 
          the completed queue for further processing
          
          2) This can be a beacon which was sent by NHLE 
          since the macBeaconAutoRespond is set to FALSE */
          
          /* put to queue of completed messages */
          trxsm_enqueue_completed_msg(s);
        }
        
        s->packet = NULL_POINTER;
        /* back to IDLE */
        trxsm_transit((sm_state_t)&trxsm_idle);
      }
    }
    else
    {
      s->result = TRXSM_FAILURE;
      
      /*start the pause duration timer*/
//#if (CFG_MAC_LBTSM_ENABLED == 1)
//      SM_DISPATCH( (sm_t *)&lbtsm, &event );
//#else
//      SM_DISPATCH( (sm_t *)&ccasm, &event );
//#endif 
      
      /*This could be the only reason why it will hit at this place. 
      So update the status accordingly*/
      s->packet->status = MAC_CHANNEL_ACCESS_FAILURE;
      
      /* increase retry count and requeue */
      trxsm_requeue( s, 1 );
      /* try again */
          //trxsm_transit_cap_tx_csma( s );
          trxsm_transit((sm_state_t)&trxsm_idle);
    }
    break;//end TRXSM_TRIGGER_PD_DATA_CONFIRM
    
  case TRXSM_TRIGGER_CANCEL:
    s->result = TRXSM_NONE;
    /* requeue packet */
    trxsm_requeue( s, 0 );
    trxsm_cancel_curr_activities( s, e->param.scalar );
    break;
    
  default:
    break;
  }
  return NULL_POINTER;
}

/******************************************************************************/
uint16_t ACK_RECVD = 0;
int get_join_state (void);
sw_tmr_t ack_wait_timer_debug = {0};
uint64_t events_posted_while_in_ack_wait_state = 0;
/* state: CAP_TX_WAIT*/
static sm_result_t trxsm_cap_tx_wait(trxsm_t *s, const sm_event_t *e)
{
        uint32_t unicast_hop_channel = 0;

        int unicast_channel_index = 0;
        uint16_t len = 0;
        sm_event_t event;
        uint32_t shr_duration = 0;
        uint32_t sym_per_octet = 0;
        uint32_t fec_config = 0;
	//sm_event_t event = { (sm_trigger_t) CCASM_TRIGGER_SUSPEND, { 0 } };
	trxsm_ack_t* p_ack_in = NULL;
	uint16_t symbol_length = HWTIMER_SYMBOL_LENGTH;
        uint16_t safety_margin = 0;
#if(CFG_MAC_CCA_ENABLED == 1)
    event.trigger = (sm_trigger_t) CCASM_TRIGGER_SUSPEND;
#endif    
    switch ((trxsm_trigger_t) e->trigger)
    {
        case TRXSM_TRIGGER_ENTRY:
            s->state_ind = TRXSM_STATE_CAP_TX_WAIT;

#ifdef UTEST_TRXSM
            utu_timestamp(UTUL_TRXSM_CAP_ACK_WAIT, 0 );
#endif

        /* turn receiver on */  
        PLME_Set_TRX_State( PHY_RX_ON );

        /* set alarm for timeout */
        s->fn_alarm = &trxsm_alarm_cap_tx_wait;
       
        PLME_get_request(phySHRDuration,&len,&shr_duration);
        PLME_get_request(phySymbolsPerOctet,&len,&sym_per_octet);
        PLME_get_request(phyFECEnabled,&len,&fec_config);

#if RAIL_TIMER_INTERFACE_USED    
    s->sw_timer.relPeriodic = s->tmp * HWTIMER_SYMBOL_LENGTH;
#else
    s->sw_timer.period = s->tmp * HWTIMER_SYMBOL_LENGTH;
#endif  
    
#if RAIL_TIMER_INTERFACE_USED    
    s->sw_timer.relPeriodic = ( aUnitBackoffPeriod + aTurnaroundTime + shr_duration + ( aMRFSKPHRLength * sym_per_octet )
                                                            + ( TRXSM_ACK_FRAME_SIZE * sym_per_octet ) ) * symbol_length;
#else
    s->sw_timer.period = ( aUnitBackoffPeriod + aTurnaroundTime + shr_duration + ( aMRFSKPHRLength * sym_per_octet )
                                                            + ( TRXSM_ACK_FRAME_SIZE * sym_per_octet ) ) * symbol_length ;
#endif      
        
       

#ifdef WISUN_ENET_PROFILE
    //Raka 
    //s->sw_timer.period = 50* 6000;//5000 plus 1000 (for processing and validation)
#if RAIL_TIMER_INTERFACE_USED
    s->sw_timer.relPeriodic = 6000;
#else    
    s->sw_timer.period = 6000;//5000 plus 1000 (for processing and validation)
#endif      
#else
#if RAIL_TIMER_INTERFACE_USED
    s->sw_timer.relPeriodic += ((safety_margin)*symbol_length);
#else    
    s->sw_timer.period += ((safety_margin)*symbol_length);  
#endif     
      
#endif
/*Uemsh : 11-01-2018 this thing goes to modified after sepration of mac and fan mac*/
    #ifdef WISUN_FAN_MAC
#if RAIL_TIMER_INTERFACE_USED
     s->sw_timer.relPeriodic = ( aUnitBackoffPeriod + aTurnaroundTime + shr_duration + ( aMRFSKPHRLength * sym_per_octet )
                                                            + ( aMaxPHYPacketSize * sym_per_octet ) ) * symbol_length;
      s->sw_timer.relPeriodic += (rand () % 15);  
#else    
      /*Suneet :: we are not added proccesing time because we calculate on max pakt size which is aMaxPHYPacketSize 2047*/
      s->sw_timer.period = ( aUnitBackoffPeriod + aTurnaroundTime + shr_duration + ( aMRFSKPHRLength * sym_per_octet )
                                                            + ( aMaxPHYPacketSize * sym_per_octet ) ) * symbol_length;
      s->sw_timer.period += (rand () % 15); //suneet :: increase Ack wait duration for testing need to club with rail phy
#endif      
    #endif
		
//    timer_start_time = timer_current_time_get();
      memcpy (&ack_wait_timer_debug, &s->sw_timer, sizeof (ack_wait_timer_debug));
#if RAIL_TIMER_INTERFACE_USED
      if( !RAIL_SetMultiTimer(&s->sw_timer,s->sw_timer.relPeriodic,RAIL_TIME_DELAY,&trxsm_alarm,s))
#else      
    if( !tmr_start_relative(&(s->sw_timer)))
#endif     
    {
      
#ifdef UTEST_TRXSM
      utu_timestamp(UTUL_ERR_CLIENT, -1);
#endif
    }
    break;

        case TRXSM_TRIGGER_ACK_RECEIVED:
#ifndef ENHANCED_ACK_SUPPORT
         //PLME_Set_TRX_State( PHY_RX_ON );
//        timer_ack_rxxxx_time = timer_current_time_get();
        s->result = TRXSM_SUCCESS;
        p_ack_in = ((trxsm_ack_t *)e->param.vector);
      ACK_RECVD++;
#ifdef WISUN_ENET_PROFILE            
            // the time difference in us between the time at which the last symbol of the data sent and the time at which the last symbol of the PHR received should be less than 5000 us as per ENET profile as it is macEnhAckWaitDurtion
            if( ( p_ack_in->sfd_rx_time - s->packet->tx_timestamp )>5000 ) //replace thhis 5000 with some meaningful macro or use MAC PIB
            {
              /* received EACK  after 5000 us.*/
                break;
            } 
               if( ( ( p_ack_in->dst.pan_id != mac_pib.PANId ) &&
                    ( p_ack_in->dst.pan_id != BROADCAST_PAN_ID ) ) 
                    || (ieeeaddr_cmp(  p_ack_in->dst.address.ieee_address, aExtendedAddress ) != 0)
			  )
            {
                /* acknowledgement not for me*/
                break;
            }
#endif            
/*Uemsh : 11-01-2018 this thing goes to modified after sepration of mac and fan mac*/
#ifdef WISUN_FAN_MAC /*Umesh changed here*/
            //#elifdef (WISUN_FAN_MAC) /*Umesh changed here*/
            
            if( (ieeeaddr_cmp(  p_ack_in->dst.address.ieee_address, aExtendedAddress ) != 0) 
               &&(p_ack_in->dsn != s->packet->sn) )
            {
              /* acknowledgement not for me*/
              /*Sequence Number not matched */
              break;
            }
#else
         if( ( ( p_ack_in->dst.pan_id != mac_pib.PANId ) &&
                ( p_ack_in->dst.pan_id != BROADCAST_PAN_ID ) ) 
				|| (ieeeaddr_cmp(  p_ack_in->dst.address.ieee_address, aExtendedAddress ) != 0)
			  )
            {
                /* acknowledgement not for me*/
                break;
            }     

#endif
#ifdef WISUN_FAN_MAC    

#if(FAN_FRQ_HOPPING_FEATURE_ENABLED == 1)            
         if(mac_self_fan_info.unicast_listening_sched.us_schedule.channel_function != CF_FIXED_CHANNEL)
         {
           if(mac_self_fan_info.unicast_listening_sched.us_schedule.channel_function == CF_DH1)
           {
             unicast_channel_index = getUnicastChannelIndex(unicast_slot_no,node_address,usable_channel.total_usable_ch_unicast);
             unicast_hop_channel = usable_channel.unicast_usable_channel_list[unicast_channel_index];
           }
           PLME_set_request( phyCurrentChannel, 2, &unicast_hop_channel ); 
           dont_change_ubcast_channel = 0x89;  
         }
#endif
         
#endif

#if RAIL_TIMER_INTERFACE_USED
         RAIL_CancelMultiTimer(&s->sw_timer);
#else  
         tmr_stop
           ( 
            &(s->sw_timer)
              );
#endif  

        /* set status of packet */
        s->packet->status = MAC_SUCCESS;
        
            /* indicate if ack had frame pending set */
            if( ((trxsm_ack_t *)e->param.vector)->fp )
            {
                s->packet->tx_options |= TX_OPTION_FRAME_PENDING_IN;
            }
			
	     /*start the pause duration timer*/
#if(CFG_MAC_LBTSM_ENABLED == 1)
                SM_DISPATCH( (sm_t *)&lbtsm, &event );
#elseif(CFG_MAC_CCA_ENABLED ==1)
                SM_DISPATCH( (sm_t *)&ccasm, &event );
#endif 

            /* put to queue of completed messages */
            trxsm_enqueue_completed_msg(s);
 
            /* back to IDLE */
  
	    trxsm_transit((sm_state_t)&trxsm_idle);
#endif  //#ifndef ENHANCED_ACK_SUPPORT
	
            break;

        case TRXSM_TRIGGER_CANCEL:
            s->result = TRXSM_NONE;
#if RAIL_TIMER_INTERFACE_USED
            RAIL_CancelMultiTimer(&s->sw_timer);
#else  
            tmr_stop
              ( 
               &(s->sw_timer)
                 );
#endif  
            /* requeue packet */
            trxsm_requeue( s, 0 );
            trxsm_cancel_curr_activities( s, e->param.scalar );

            break;

        case TRXSM_TRIGGER_ALARM:
//            timer_exp_time = timer_current_time_get();	     
            s->result = TRXSM_FAILURE;
            /* set status of packet */
            s->packet->status = MAC_NO_ACK;
            /*start the pause duration timer*/
#if(CFG_MAC_LBTSM_ENABLED == 1)
                SM_DISPATCH( (sm_t *)&lbtsm, &event );
#elseif(CFG_MAC_CCA_ENABLED==1)
                SM_DISPATCH( (sm_t *)&ccasm, &event );
#endif 
	    /* increase retry count and requeue */
            if( trxsm_requeue( s, 1 ) == TRXSM_FAILURE_MAXRETRY )
            {	            
            	/* reached retry count */
                trxsm_transit((sm_state_t)&trxsm_idle);
            }
            else
            {
                /* requeued message */
                trxsm_transit((sm_state_t)&trxsm_idle);
            }
            break;
            
      case TRXSM_TRIGGER_EXIT:
        break;

        default:
          events_posted_while_in_ack_wait_state++;
          break;
    }
    return NULL_POINTER;
}

/******************************************************************************/
uint16_t ACK_SEND = 0;
/*state: CAP_ACK_TX_SEND*/
static sm_result_t trxsm_cap_ack_tx_send(trxsm_t *s, const sm_event_t *e)
{  
//  sm_event_t event;
  int unicast_channel_index = 0;
  uint32_t unicast_hop_channel = 0;
//  event.trigger = (sm_trigger_t) CCASM_TRIGGER_SUSPEND;
  switch ((trxsm_trigger_t) e->trigger)
  {
  case TRXSM_TRIGGER_ENTRY:
    s->state_ind = TRXSM_STATE_CAP_ACK_TX_SEND;
    PLME_Set_TRX_State( PHY_TX_ON );
    /*Uemsh : 11-01-2018 this thing goes to modified after sepration of mac and fan mac*/
#ifdef WISUN_FAN_MAC			
    ACK_SEND++;
    PD_Data_Request( s, s->packet->data );
//#if (PRINT_DEBUG_LEVEL == CONSOLE_ERROR_DEBUG)
//    stack_print_debug ("Tx-ACK [%d] for ", s->packet->data->psdu[2]);
//    print_mac_address (s->packet->dst.address.ieee_address);
//#endif
#endif  //WISUN_FAN_MAC
    break;
    
  case TRXSM_TRIGGER_PD_DATA_CONFIRM:
#ifdef DEBUG_ACK_TIMINGS        
    ack_tx_completion = timer_current_time_get();
#endif/*DEBUG_ACK_TIMINGS*/
    
    
#ifdef WISUN_FAN_MAC
#if(FAN_FRQ_HOPPING_FEATURE_ENABLED == 1)
      if(mac_self_fan_info.unicast_listening_sched.us_schedule.channel_function != CF_FIXED_CHANNEL)
      {
        if(mac_self_fan_info.unicast_listening_sched.us_schedule.channel_function == CF_DH1)
        {
          unicast_channel_index = getUnicastChannelIndex(unicast_slot_no,node_address,usable_channel.total_usable_ch_unicast);
          unicast_hop_channel = usable_channel.unicast_usable_channel_list[unicast_channel_index];
        }
        PLME_set_request( phyCurrentChannel, 2, &unicast_hop_channel ); 

      }      
#endif
      
#endif
    s->result = TRXSM_SUCCESS;
    
    /* set status of packet */
    s->packet->status = MAC_SUCCESS;
    
    PLME_Set_TRX_State( PHY_RX_ON );
    s->packet->tx_timestamp = ((phy_tx_info_t *)e->param.vector)->tx_ts_us;
    
    /* set status of packet */
    s->packet->status = MAC_SUCCESS;
    trxsm_enqueue_completed_msg(s);
    s->packet = NULL_POINTER;
    
    trxsm_transit((sm_state_t)&trxsm_idle);
  }
  return NULL_POINTER;
}

/******************************************************************************/

/*state: CFP*/
static sm_result_t trxsm_cfp(trxsm_t *s, const sm_event_t *e)
{
    switch ((trxsm_trigger_t) e->trigger)
    {
        case TRXSM_TRIGGER_ENTRY:
            s->state_ind = TRXSM_STATE_CFP;
#ifdef UTEST_TRXSM
            utu_timestamp(UTUL_TRXSM_CFP, -1);
#endif
            break;

        default:
            break;
    }
    return NULL_POINTER;
}

/******************************************************************************/

/*state: RSSI*/

#if( CFG_ENERGY_DETECTION_SCAN == 1 )
static sm_result_t trxsm_rssi(trxsm_t *s, const sm_event_t *e)
{
    switch ((trxsm_trigger_t) e->trigger)
    {
        case TRXSM_TRIGGER_ENTRY:
            s->state_ind = TRXSM_STATE_RSSI;
#ifdef UTEST_TRXSM
            utu_timestamp(UTUL_TRXSM_RSSI, s->tmp);
#endif
            /* turn RSSI on */
            //phy_ed_on( NULL_POINTER );

            /* set alarm for timeout */
            s->fn_alarm = &trxsm_alarm_rssi;
            //if( alarm_refset( &s->alarm, s->tmp, HWTIMER_ALIGN_TO_SYMBOL ) != ALARM_SUCCESS )
            
            s->sw_timer.period = s->tmp * HWTIMER_SYMBOL_LENGTH;
            
#if RAIL_TIMER_INTERFACE_USED
      if( !RAIL_SetMultiTimer(&sw_tri_tmr,s->sw_timer.period,RAIL_TIME_DELAY,&trxsm_alarm,s))
#else      
    if( !tmr_start_relative(&(s->sw_timer)))
#endif 
            {
                /*TBD What to do here? */
#ifdef UTEST_TRXSM
                utu_timestamp(UTUL_ERR_CLIENT, s->tmp);
#endif
            }
            break;

        case TRXSM_TRIGGER_CANCEL:

            /* cancel alarm for timeout */
            //alarm_unset( &s->alarm );
			trxsm_cancel_curr_activities( s, e->param.scalar );
			
            break;
	
	     
        case TRXSM_TRIGGER_ALARM:

            s->result = TRXSM_SUCCESS;
			phy_ed_off( NULL_POINTER );
            /* return to IDLE state */          
	    trxsm_transit((sm_state_t)&trxsm_idle);
            break;
	

        default:
            break;
    }
    return NULL_POINTER;
}

#endif	/*( CFG_ENERGY_DETECTION_SCAN == 1 )*/

/******************************************************************************/

/*state: OFF*/
static sm_result_t trxsm_off(trxsm_t *s, const sm_event_t *e)
{
    switch ((trxsm_trigger_t) e->trigger)
    {
        case TRXSM_TRIGGER_ENTRY:
            s->state_ind = TRXSM_STATE_OFF;
#ifdef UTEST_TRXSM
            utu_timestamp(UTUL_TRXSM_OFF, 0);
#endif

            //PLME_Set_TRX_State( PHY_FORCE_TRX_OFF );

           // PLME_Set_TRX_State( PHY_FORCE_TRX_OFF );
            
//#if (PRINT_DEBUG_LEVEL==ERROR_DEBUG)
//    stack_print_debug ("I phy state = %d\n", plme_get_trx_state_request ());
//#endif             
            
			PLME_Set_TRX_State( PHY_DEV_ON );
                        
//#if (PRINT_DEBUG_LEVEL==ERROR_DEBUG)
//    stack_print_debug ("J phy state = %d\n", plme_get_trx_state_request ());
//#endif                         
                        
#ifndef EFM32_TARGET_IAR
            lpmsm_clear_flag( (lpmsm_flag_t)(LPMSM_FLAG_RX_ON | LPMSM_FLAG_TX_ON) );
#endif
#if(CFG_MAC_SFTSM_ENABLED == 1)              
            s->sftsm = NULL_POINTER;
#endif            
            break;

        case TRXSM_TRIGGER_CANCEL:

			trxsm_cancel_curr_activities( s, e->param.scalar );
            
            break;

        default:
            break;
    }
    return NULL_POINTER;
}

/******************************************************************************/
#if RAIL_TIMER_INTERFACE_USED
static void trxsm_alarm(struct RAIL_MultiTimer *tmr,
                 RAIL_Time_t expectedTimeOfEvent,
                 void *s )
#else
static void trxsm_alarm(void *s, void* tmr )
#endif
{
    (*((trxsm_t *)s)->fn_alarm)(s);
}
//static void trxsm_alarm_us_hop(void *s, void* tmr )
//{
//  #ifdef WISUN_FAN_MAC
//    (*((trxsm_t *)s)->fn_alarm_us_hop)(s);
//#endif
//}

/******************************************************************************/

static void trxsm_alarm_wait_bcn_wait(void *v)
{
    trxsm_t *s = NULL;
    sm_event_t e;

    s = v;

    /* indicate timeout to TRX-SM */
    e.trigger = (sm_trigger_t) TRXSM_TRIGGER_ALARM;
    e.param.scalar = 0;
    //SM_DISPATCH( (sm_t *)s, &e );
    
    
    
    while( !((SM_DISPATCH( (sm_t *)(s), &e ) == NULL_POINTER) &&
	( trxsm_get_result( s ) == TRXSM_SUCCESS ))) 
	{
		/*made sure that the event is not missed and it is executed, 
		But there can be failure even if the event was handled, 
		like phy_ed_on might have failed as the node radio was busy doing 
		RX and so on.... so whereever these events are handled in the trxsm, 
		make set the result as SUCCESS or FAILURE so that this can be 
		asserted here*/
#ifndef EFM32_TARGET_IAR
		R_WDT_Restart();
#endif
		
	}	
}

/******************************************************************************/

static void trxsm_alarm_wait_mpm_eb_wait(void *v)
{
    trxsm_t *s = NULL;
    sm_event_t e;

    s = v;
    /* indicate timeout to TRX-SM */
    e.trigger = (sm_trigger_t) TRXSM_TRIGGER_ALARM;
    e.param.scalar = 0;
    SM_DISPATCH( (sm_t *)s, &e );
}

/******************************************************************************/
#if( CFG_ENERGY_DETECTION_SCAN == 1 )
static void trxsm_alarm_rssi(void *v)
{
    trxsm_t *s = NULL;
    sm_event_t e;
    //irq_state_t flags = irq_disable();
    
    s = v;

    /* indicate timeout to TRX-SM */
    e.trigger = (sm_trigger_t) TRXSM_TRIGGER_ALARM;
    e.param.scalar = 0;
    SM_DISPATCH( (sm_t *)s, &e );
    //irq_enable(flags);
    
    
    /* indicate timeout to SCAN-SM */
    if( scansm_p )
    {
        e.trigger = (sm_trigger_t) SCANSM_TRIGGER_TIMEOUT;
     
        SM_DISPATCH( (sm_t *)(scansm_p), &e );
    }
    
}
#endif	/*( CFG_ENERGY_DETECTION_SCAN == 1 )*/

/******************************************************************************/

static void trxsm_alarm_cap_tx_wait(void *v)
{
    trxsm_t *s = NULL;
    sm_event_t e = { (sm_trigger_t) TRXSM_TRIGGER_ALARM, { 0 } };

    s = v;
    SM_DISPATCH( (sm_t *)s, &e );
}

/******************************************************************************/
/* Keep track of retry counter */
static trxsm_result_t trxsm_requeue( trxsm_t *s, uchar count )
{
  uint32_t unicast_hop_channel = 0;
  uint32_t current_max_channels = 0;
  int unicast_channel_index = 0;
  uint16_t length = 0;
//  uint32_t bfio=0;
//  uint32_t ufsi=0;
  trxsm_result_t trxsm_result = TRXSM_SUCCESS;
  uint8_t trx_completed = 0;
    if( s->packet == NULL_POINTER )
    {
    	trxsm_result = TRXSM_SUCCESS;
    	return trxsm_result;
    }

    /* handle indirect packets */
    if( s->packet->tx_options & INDIRECT_TRANSMISSION )
    {
        /* requeue packet to indirect queue */
        queue_front_put( s->indirect_queue, (queue_item_t *) s->packet );
#if(CFG_MAC_PENDADDR_ENABLED == 1)
        /* update Pending Address List */
        if( pendaddr_add( s->packet ) != PENDADDR_SUCCESS )
        {
            /*TBD What to do here? */
            //debug(("pendaddr err\r\n"));
        }
#endif
        set_process_activity(AF_IND_MSG_PENDING);

        trxsm_result = TRXSM_FAILURE_MAXRETRY;
        goto exit;
    }

    if( count )
    {
        /* increase retry count */
        s->packet->cap_retries += count;

        /* check if max retries has been reached */
        if ( s->packet->cap_retries > mac_pib.MaxFrameRetries )
        {
            s->packet->cap_retries = 0;
                        
	    if( s->packet->type == MAC_FRAME_TYPE_BEACON )
            {

#if(CFG_MAC_MPMSM_ENABLED == 1)				
            	if(  s->mpmsm->state_ind == MPMSM_STATE_MPM_EB )
	            {
			        /* MAC is performing MPM EB transmissions in CSM */
					/* increment Beacon Sequence Number */
		            trxsm_process_mpm_eb_tx_conf(s);
		            
		            trxsm_result = TRXSM_FAILURE_MAXRETRY;
		            return trxsm_result; 	            	
	            	
	            }
	            else
	            {
            		/* MAC is performing EB or regular beacon transmissions */
            		if(  mac_pib.BeaconAutoRespond )
            		{
            			/*no confirmation required if beacon/EB are sent out 
            			automatically without NHLE's intervention */
            			if( !(mac_pib.mac_security_enabled) )
						{
							/* requeue beacon */
							mac_queue_beacon_transmission( s->packet,s->packet->sub_type );
						}
						else
						{
							mac_mem_free_tx(s->packet);
							s->packet = NULL_POINTER;
#if(CFG_MAC_BEACON_ENABLED == 1)                                                        
							mac_beacon_update( s->packet->sub_type );
#endif                                                        
						}	
            		}
            		else
            		{
            			/* NHLE has triggered the beacon/EB transmisison 
            			queue_item_put( s->completed_queue, (queue_item_t *) s->packet );
		                set_process_activity(AF_TX_MSG_SENT_PENDING);
						event_set(FRAME_TX_DONE_EVENT);
						signal_event_to_mac_task();*/
						trx_completed = 1;
            		}            		            		
	            }
#endif //  #if(CFG_MAC_MPMSM_ENABLED == 1)                
            }
            else
            {
#ifdef WISUN_FAN_MAC
#if ((PRINT_DEBUG_LEVEL==ERROR_DEBUG)||(PRINT_DEBUG_LEVEL==CONSOLE_ERROR_DEBUG))
              stack_print_debug ("*** Full MAC retry to ");
              print_mac_address (s->packet->dst.address.ieee_address);
              printf ("\n");
#endif
              
#if(FAN_FRQ_HOPPING_FEATURE_ENABLED == 1)              
              if(mac_self_fan_info.unicast_listening_sched.us_schedule.channel_function != CF_FIXED_CHANNEL)
              {
                if(mac_self_fan_info.unicast_listening_sched.us_schedule.channel_function == CF_DH1)
                {
                  PLME_get_request( phyMaxSUNChannelSupported, &length, &current_max_channels );
                  unicast_channel_index = getUnicastChannelIndex(unicast_slot_no,node_address,usable_channel.total_usable_ch_unicast);
                  unicast_hop_channel = usable_channel.unicast_usable_channel_list[unicast_channel_index];
                }
                PLME_set_request( phyCurrentChannel, 2, &unicast_hop_channel ); 
                dont_change_ubcast_channel = 0x89;  
              }
#endif
              
#endif
              trx_completed = 1;
#if APP_LBR_ROUTER               
              if(s->packet->msdu_handle == 135) /* check whether it was a ns with aro packet */
              {
                send_mcps_no_ack_indication ();
              }
#endif              
            }
	    
	    if( trx_completed )
	    {
		/* put to queue of completed messages */				
		trxsm_enqueue_completed_msg(s);
	    }
	    
            trxsm_result = TRXSM_FAILURE_MAXRETRY;
            goto exit;
        }
    } 
#ifdef WISUN_FAN_MAC/*Umesh : 21-02-2018*//*for sepration of 802.15.04*/
//    ufsi = update_ufsi_with_procces_time(s->packet);
//    if(s->packet->p_ufsi!=NULL)
//    {
//      memcpy(s->packet->p_ufsi,(uint8_t*)&ufsi,3);
//    }
//    bfio =  update_bfio_with_procces_time(s->packet);
//    if(s->packet->p_bfsi!=NULL)
//    {
//      memcpy(s->packet->p_bfsi,(uint8_t*)&bfio,3);
//    }
    
    if(s->packet->sub_type == FAN_DATA_PKT)     //Debdeep:: fan_pkt_type
    {
//      if(mac_self_fan_info.unicast_listening_sched.us_schedule.channel_function != CF_FIXED_CHANNEL) 

        if((s->packet->security_data->raw_payload_data != NULL)
           && (s->packet->security_data->payload_length != 0))
        {
          memcpy(s->packet->security_data->payload,s->packet->security_data->raw_payload_data,s->packet->security_data->payload_length);
          app_bm_free ((uint8_t*)(s->packet->security_data->raw_payload_data));
          s->packet->security_data->raw_payload_data = NULL;
        }
/* Debdeep :: Security is done just before sending packet to PHY */      
//        s->packet->security_data->return_queue = (queue_t *) queue_manager_get_list(QUEUE_CAP_TX);
//        queue_item_put((queue_t *) & mac_security_data.hallin_tx_queue,
//                       (queue_item_t *) s->packet->security_data);
//        event_set( SECURE_EVENT );
//      else
//      {
//        queue_front_put( (( s->state_ind == TRXSM_STATE_BCAST_CSMA ||
//                           s->state_ind == TRXSM_STATE_BCAST_SEND )?s->bcast_queue:s->direct_queue), (queue_item_t *) s->packet );
//        trxsm_set_pending_tx_event(s);
//      }
        
//        ufsi = update_ufsi_with_procces_time(s->packet);
//        if(s->packet->p_ufsi!=NULL)
//        {
//          memcpy(s->packet->p_ufsi,(uint8_t*)&ufsi,3);
//        }
//        bfio =  update_bfio_with_procces_time(s->packet);
//        if(s->packet->p_bfsi!=NULL)
//        {
//          memcpy(s->packet->p_bfsi,(uint8_t*)&bfio,3);
//        }
//        
//        encrypt_data_packet (s->packet->security_data);
      
    }
//    else
//    {
    
    /* Debdeep :: Subscribe secure MAC retry frame */
    subscribe_tx_frame (s->packet);
    
#if (PRINT_DEBUG_LEVEL==ERROR_DEBUG)    
    if (s->state_ind == TRXSM_STATE_BCAST_CSMA || s->state_ind == TRXSM_STATE_BCAST_SEND)
    {
      stack_print_debug ("BROADCAST packet requeued\n");
    }
#endif
	  
      queue_front_put( (( s->state_ind == TRXSM_STATE_BCAST_CSMA ||
                         s->state_ind == TRXSM_STATE_BCAST_SEND )?s->bcast_queue:s->direct_queue), (queue_item_t *) s->packet );
      //trxsm_set_pending_tx_event(s);
//    } 
    
#else 
    queue_front_put( (( s->state_ind == TRXSM_STATE_BCAST_CSMA ||
                       s->state_ind == TRXSM_STATE_BCAST_SEND )?s->bcast_queue:s->direct_queue), (queue_item_t *) s->packet );
    //trxsm_set_pending_tx_event(s);
#endif
    
    trxsm_result = TRXSM_SUCCESS;
exit:
  s->packet = NULL_POINTER;
  return trxsm_result;
}

/******************************************************************************/
#ifdef MAC_CFG_BEACONING_ENABLED
/*Check if battery life extension period is active*/
static uchar trxsm_ble_active( trxsm_t *s )
{
    return /* check if BLE expired */
           ( s->sftsm->mode & SFTSM_MODE_BEYOND_BLE ) &&
           /* check if outgoing superframe */
           ( (s->sftsm->mode & SFTSM_MODE_INOUT_MASK) == SFTSM_MODE_OUTGOING );
}
#endif

/******************************************************************************/

/* Transits to CAP_TX_CSMA or IDLE depending on the direct queue*/
//static void trxsm_transit_cap_tx_csma( trxsm_t *s )
//{
//    sm_state_t next_state = (sm_state_t)&trxsm_idle;
//    
//	if ((ccasm_is_idle (s->ccasm) == 1) && ((s->packet = (mac_tx_t *)queue_item_get (s->direct_queue)) != NULL_POINTER))
//	{
//		/* get next packet from direct queue */
//		//if( (s->packet = (mac_tx_t *) queue_item_get( s->direct_queue )) != NULL_POINTER )
//		//{
//			//sm_transit( (sm_t *) s, (sm_state_t) &trxsm_idle );
//			//sm_transit( (sm_t *) s,  );
//			next_state = ( ccasm_cca_needed( s->ccasm, s->packet ))?(sm_state_t) &trxsm_cap_tx_csma:(sm_state_t) &trxsm_cap_tx_send;			
//			
//		//}
//		//else
//		//{
//			/*if( ccasm_cca_needed( s->ccasm, s->packet ) )
//			{
//				sm_transit( (sm_t *) s, (sm_state_t) &trxsm_cap_tx_csma );
//			}
//			else
//			{
//				sm_transit( (sm_t *) s, (sm_state_t) &trxsm_cap_tx_send );
//			}*/
//			
//		//}
//	}
//	//else
//	//{
//		//sm_transit( (sm_t *) s, (sm_state_t) &trxsm_idle );
//	//}
//	sm_transit( (sm_t *) s, next_state );
//}

//static void trxsm_transit_cap_tx_csma_broacast( trxsm_t *s )
//{
//  sm_state_t next_state = (sm_state_t)&trxsm_idle;
//  
//  if ((ccasm_is_idle (s->ccasm) == 1) && ((s->packet = (mac_tx_t *)queue_item_get (s->bcast_queue)) != NULL_POINTER))
//  {
//#if (PRINT_DEBUG_LEVEL==ERROR_DEBUG)
//    stack_print_debug ("trxsm_transit_cap_tx_csma_broacast going to trxsm_bcast_csma\n");
//#endif   
//    next_state = ( ccasm_cca_needed( s->ccasm, s->packet ))?(sm_state_t) &trxsm_bcast_csma:(sm_state_t) &trxsm_cap_tx_send;			
//  }
//  
//#if (PRINT_DEBUG_LEVEL==ERROR_DEBUG)
//    stack_print_debug ("trxsm_transit_cap_tx_csma_broacast going to idle\n");
//#endif 
//  
//  sm_transit( (sm_t *) s, next_state );
//}

/******************************************************************************/

static void trxsm_activate_csm( trxsm_t *s, uint16_t OrigChannel )
{
	uint16_t length;
	uint32_t value;
	
	s->orig_channel = OrigChannel;
	PLME_get_request( phyCurrentSUNPageEntry, &length, &value );
	
	s->orig_curr_sun_page = value;
	
	/* clear the PHY mode bits in the current SUN page Entry
	TBD: Need to take care for other PHYs also. 
	This works with only 950 MHz FSK PHY */
	value &= 0xFFFFFFF0;
	
	value |= (uint32_t)( 1<< ( COMMON_SIGNALLING_MODE - 1 ));

	/*Enable CSM in PHY by enabling PHY Mode 1*/
	PLME_set_request( phyCurrentSUNPageEntry, length, &value );
	
	s->iUnitRadioChan = 0;
	

}

/******************************************************************************/

static void trxsm_deactivate_csm( trxsm_t *s )
{
	PLME_set_request( phyCurrentSUNPageEntry, 4, &s->orig_curr_sun_page );
	
	PLME_set_request( phyCurrentChannel, 2, &s->orig_channel );

	s->iUnitRadioChan = 0;
}

/******************************************************************************/

static void trxsm_cancel_curr_activities( trxsm_t *s, uchar sw_off_radio )
{
#if RAIL_TIMER_INTERFACE_USED
  RAIL_CancelMultiTimer(&s->sw_timer);
#else  
	tmr_stop
    ( 
      &(s->sw_timer)
    );
#endif    

	//if( sw_off_radio == TRXSM_PARAM_OFF )
    //{
        sm_transit((sm_t *)s, ( sw_off_radio == TRXSM_PARAM_OFF)?(sm_state_t)&trxsm_off:(sm_state_t)&trxsm_idle);
   // }
    //else
    //{
       // sm_transit((sm_t *)s, (sm_state_t)&trxsm_idle);
    //}
}

/******************************************************************************/

static trxsm_result_t trxsm_process_mpm_eb_tx_conf(trxsm_t* s)
{
	trxsm_result_t r;
	uint32_t channel = 0;
	/* increment Beacon Sequence Number. TBD: Do we need to check the status of 
	the previous mpm eb tx and accordingly proceed or just proceed irrespective 
	of that. Currntly the status is not being checked */
    mac_pib.EBSN += 1;
    
    if( s->iUnitRadioChan < s->CSMCurrTotalUnitRadioChanCount )
    {
        s->packet->data->psdu[2] = mac_pib.EBSN;
        
    	s->packet->data->TxChannel = 
		s->CSMUnitRadioChanList[ s->iUnitRadioChan++ ];
	
        channel = s->packet->data->TxChannel;
        PLME_set_request( phyCurrentChannel, 2, &channel );
		/* requeue packet to specified queue */
        queue_front_put( s->direct_queue, (queue_item_t *) s->packet );	
        
        //trxsm_set_pending_tx_event(s);

		s->packet = NULL_POINTER;

        r = TRXSM_SUCCESS_QUEUE;         
    }
    else
    {
		if( !(mac_pib.mac_security_enabled) )
		{
			/* update current beacon without rebuilding */
			s->packet->data->TxChannel = s->orig_channel;
			s->packet->data->psdu[2] = mac_pib.EBSN;

			/* requeue new beacon */
			mac_queue_beacon_transmission( s->packet,MAC_FRAME_BCN_SUB_TYPE_MPM_EB );
			s->packet = NULL_POINTER;
		}
		else
		{
			mac_mem_free_tx(s->packet);
			s->packet = NULL_POINTER;
#if(CFG_MAC_BEACON_ENABLED == 1)                         
			mac_beacon_update(MAC_FRAME_BCN_SUB_TYPE_MPM_EB);
#endif                        
		}
		

#if(CFG_MAC_MPMSM_ENABLED == 1)	    /* send event to SFTSM */
        /*TBD Shouldn't the event contain the superframe parameters? */
        if( s->mpmsm )
        {
#ifdef UTEST_TRX            
        	utu_timestamp(UTUL_TRXSM_MPM_EB_TX_DONE, -1);
#endif        	
        	event.trigger = (sm_trigger_t) MPMSM_TRIGGER_EB_DONE;
            SM_DISPATCH( (sm_t *)(s->mpmsm), &event );
        }
#endif // #if(CFG_MAC_MPMSM_ENABLED == 1)
        /* go IDLE on beaconless network */
        //if( s->sftsm == NULL_POINTER )
        {
            trxsm_deactivate_csm(s);        	
        }
        
        r = TRXSM_SUCCESS;
    }
    
    return r;
}

/******************************************************************************/

void trxsm_transit( sm_state_t state )
{
	//irq_state_t flags = irq_disable(); 
	sm_transit( (sm_t *) trxsm_p, (sm_state_t) state );
	//irq_enable(flags); 
	return;
}

/******************************************************************************/

static void trxsm_enqueue_completed_msg(trxsm_t *s)
{
#if(CFG_MAC_LBTSM_ENABLED == 1)  
     s->packet->num_csmaca_backoffs = ((lbtsm_t*)(s->ccasm))->nb+1;
#elseif(CFG_MAC_CCA_ENABLED == 1)
     s->packet->num_csmaca_backoffs = ((ccasm_t*)(s->ccasm))->nb+1;
#endif     
//        if(tx_ts_us)
//        {
//            s->packet->tx_timestamp = tx_ts_us;
//        }
        
	queue_item_put( s->completed_queue, (queue_item_t *) s->packet );
	s->packet = NULL_POINTER;
	set_process_activity(AF_TX_MSG_SENT_PENDING);
	event_set(FRAME_TX_DONE_EVENT);
	//signal_event_to_mac_task();
}

/******************************************************************************/
//#if (CFG_MAC_CCA_ENABLED == 1)    
//static void create_cca_event(trxsm_t *s,ccasm_param_t* p_cca_param)
//{
//	uint16_t length;
//	uint32_t value = 0;
//#if(CFG_MAC_SFTSM_ENABLED == 1)          
//        p_cca_param->sftsm = s->sftsm;
//#endif	
//        PLME_get_request( phyCurrentSUNPageEntry, &length, &value );
//
//	p_cca_param->phy_mode = (phy_mode_t)( value & FSK_PHY_MODE_MASK );
//	p_cca_param->freq_band_id = (freq_band_id_t)(( value & FREQ_BAND_MASK) >> 22 );
//#if defined EFM32_TARGET_IAR
//	p_cca_param->trx_time = phy_tx_duration( s->packet->length + ((s->packet->data->FCSLength)?4:2)  ) + /* packet duration */
//#else
//p_cca_param->trx_time = phy_tx_duration( s->packet->length ) + /* packet duration */
//#endif
//	( s->packet->tx_options & ACKNOWLEDGED_TRANSMISSION ?
//	aTurnaroundTime + aUnitBackoffPeriod + /* max gap between packet and ack */
//	phy_tx_duration( TRXSM_ACK_FRAME_SIZE ) /* acknowledgement duration */
//	: 0 );
//	
//	return;
//	
//}
//#endif
/******************************************************************************/

void change_to_wait_ack_state(void)
{
  trxsm_transit((sm_state_t)&trxsm_cap_tx_wait);
} 

/******************************************************************************/

void backup_trxsm_state_for_ack_sending(void)
{
  
  sm_back_up_state( (sm_t *)trxsm_p ); //trxsm_p
  //sm_restore_state( sm_t * machine )
  trxsm_transit((sm_state_t)&trxsm_idle);
} 
/******************************************************************************/
#ifdef WISUN_FAN_MAC
#if (CFG_MAC_CCA_ENABLED == 1)   
mac_status_t mac_cca_event_do(mac_tx_t *txd)
{
  trxsm.packet = txd;
  trxsm_transit((ccasm_cca_needed( trxsm.ccasm, trxsm.packet ))?((sm_state_t) &trxsm_cap_tx_csma):((sm_state_t) &trxsm_cap_tx_send));
  return MAC_SUCCESS;
}
#endif

uint8_t check_pkt_in_queue()
{
#if(FAN_EDFE_FEATURE_ENABLED == 1)
  if(edfe_information.edfe_frame_enabled == 0x01)
  {
    if((queue_manager_size(QUEUE_CAP_TX)) != 0 )
    {
      event_set(PENDING_TX_EVENT_UCAST);
      return 0;
    }
    else
    {
      return 1; 
    }
  }
#endif
  return 0xFF;
}
#endif
/******************************************************************************/
/*@}*/




/*
** ============================================================================
** Private Function Prototypes
** ============================================================================
*/
/*Umesh : 15-01-2018*//*for temp its commented otherwise this should be static and this file only */
//#ifdef WISUN_FAN_MAC
//static void trxsm_alarm_us_hop(void *s,  void* tmr ); 
//#endif

#ifdef WISUN_FAN_MAC
int getUnicastChannelIndex(uint16_t slotNumber, uint8_t* MACAddr, uint16_t nChannels);
//static void trxsm_us_channel_hop_actual(void *v);/*Umesh : 15-01-2018*//*this should be static change is made for temp*/
//static void trxsm_us_channel_hop_virtual(void *v);
#endif


/*
** ============================================================================
** External Variable Declarations
** ============================================================================
*/

#ifdef WISUN_FAN_MAC
extern self_info_fan_mac_t mac_self_fan_info;
extern fan_mac_nbr_t fan_mac_nbr;
//extern int32_t HopSequenceTable[150];
#endif

/*
** ============================================================================
** External Function Declarations
** ============================================================================
*/

extern int getBroadcastChannelIndex(uint16_t slotNumber, uint16_t broadcastSchedID, 
                                    uint16_t nChannels);
mac_nbr_descriptor_t* get_nbr_desc_from_addr (uint8_t* p_nbr_addr);


#ifdef WISUN_FAN_MAC
extern p3time_t timer_current_time_get(void);
extern bool  tmr_start_relative( sw_tmr_t *pTmr_ins );
uint64_t get_time_now_64 (void);
#endif

/*
** =============================================================================
** Public Variables Definitions
** =============================================================================
**/
/*this is  made for temp*/
#ifdef WISUN_FAN_MAC
/*Umesh : 02-01-2018*/
mac_nbr_descriptor_t* gp_nbr_entry = NULL;//static_mac,,need to check
/*this varrible was not used */
#endif

/*
** ============================================================================
** Public Function Declairations 
** ============================================================================
*/
#ifdef WISUN_FAN_MAC
//void trxsm_alarm_us_hop(void *s,  void* tmr ); 
void trxsm_us_channel_hop_actual(void *v);
//void trxsm_us_channel_hop_virtual(void *v);
#endif
uint8_t set_channel_for_freq_hop(trxsm_t *s);

/*
** ============================================================================
** Public Function Definitions
** ============================================================================
*/

/*None*/

/*
** ============================================================================
** Private Function Definitions
** ============================================================================
*/

//#ifdef WISUN_FAN_MAC
////static void trxsm_us_channel_hop_actual(void *v)
//void trxsm_us_channel_hop_actual(void *v)
//{
//    trxsm_t *s;
//    //sm_event_t e;
//    uint16_t length = 0;
//    uint32_t current_max_channels = 0;
//#ifdef UTEST_TRXSM    
//    utu_timestamp( BCN_TX_TRIGGERED, s->current_us_channel ); 
//#endif
//    PLME_get_request( phyMaxSUNChannelSupported, &length, &current_max_channels );
//    s = (trxsm_t*)v;
//    //uint8_t node_address[8];
//    
//    //mem_rev_cpy(node_address,aExtendedAddress,8);
//
//     //s->current_us_channel = getUnicastChannelIndex(++(s->curr_us_slot_num),node_address,(uint16_t)current_max_channels);
//     s->curr_us_slot_num++;
//#ifndef TEST_CHOP 
//    // Sagar: TBD for channel read (1) is required
//    PLME_set_request( phyCurrentChannel, 2, &s->next_us_channel ); // (1)           
//    //PLME_set_request( phyCurrentChannel, 2, &s->current_us_channel ); 
//#endif
//    
//    s->current_us_slot_start_ts = timer_current_time_get();
//     
//     if(!( s->curr_us_slot_num ) )
//      {
//        s->uc_chan_hop_seq_start_time = s->current_us_slot_start_ts;
//      }   
//     
//     
//    // Sagar: TBD for channel read (1), (2) & (3) are required
//    s->current_us_channel = s->next_us_channel; // (1)
//   
//
//      if(
//          !tmr_start_relative
//          (
//          
//            &(s->dwell_interval_tmr)
//          )
//        )
//      {
//          /*TBD What to do here? */
//#ifdef UTEST_TRXSM
//          utu_timestamp(UTUL_ERR_CLIENT, s->tmp);
//#endif
//      }
//      
//      s->next_us_channel = getUnicastChannelIndex((s->curr_us_slot_num+1),s->self_ieee_addr,(uint16_t)current_max_channels); //   (2)
//     // (3)
//    
//    //utu_timestamp( BCN_TX_TRIGGERED, s->current_us_channel );
//    
//     
//#ifndef TEST_CHOP  
//      channel_hop_list[channel_hop_list_index++] = (uint8_t)s->current_us_channel;
//      if( channel_hop_list_index == 100 )
//      {
//        channel_hop_list_index = 0;
//      }
//
//#endif    
//    /* indicate timeout to TRX-SM */
//    //e.trigger = (sm_trigger_t) TRXSM_TRIGGER_HOP_ALARM;
//    //e.param.scalar = 0;
//    //SM_DISPATCH( (sm_t *)s, &e );
//}
//
////static void trxsm_us_channel_hop_virtual(void *v)
//void trxsm_us_channel_hop_virtual(void *v)
//{
//    trxsm_t *s;
//   // sm_event_t e;
//    uint16_t length = 0;
//   
//    uint32_t current_max_channels = 0;
//    //uint8_t node_address[8];
//    s = (trxsm_t*)v;
//    //mem_rev_cpy(node_address,aExtendedAddress,8);
//    PLME_get_request( phyMaxSUNChannelSupported, &length, &current_max_channels );
//    
//    
//     s->curr_us_slot_num++;
//     s->current_us_channel = s->next_us_channel; 
//     s->next_us_channel = getUnicastChannelIndex((s->curr_us_slot_num+1),s->self_ieee_addr,(uint16_t)current_max_channels); //   (2)
//#ifdef UTEST_TRXSM        
//    utu_timestamp( BCN_TX_TRIGGERED, s->current_us_channel );
//#endif
//    s->current_us_slot_start_ts = timer_current_time_get();
//#ifndef TEST_CHOP     
//     channel_hop_list[channel_hop_list_index++] = (uint8_t)s->current_us_channel;
//      if( channel_hop_list_index == 100 )
//      {
//        channel_hop_list_index = 0;
//      }
//#endif
//      
//    if(
//        !tmr_start_relative
//        (
//        
//          &(s->dwell_interval_tmr)
//        )
//      )
//    {
//          /*TBD What to do here? */
//#ifdef UTEST_TRXSM
//          utu_timestamp(UTUL_ERR_CLIENT, s->tmp);
//#endif
//      }
//      
//    /* indicate timeout to TRX-SM */
//    //e.trigger = (sm_trigger_t) TRXSM_TRIGGER_HOP_ALARM;
//    //e.param.scalar = 0;
//    //SM_DISPATCH( (sm_t *)s, &e );
//}
//
//#endif

//#ifdef WISUN_FAN_MAC    
//#if 0
//uint8_t set_channel_for_freq_hop(trxsm_t *s)
//{    
//    uint8_t mac_addr[8] = {0};
//    uint32_t shr_duration = 0;
//    mac_nbr_descriptor_t *p_nbr_ptr = NULL;
//    uint64_t node_current_time = 0;
//    uint32_t get_time = 0;
//    uint32_t unicast_hop_channel = 0;
//    uint32_t broadcast_hop_channel=0;
//    uint32_t current_max_channels=0x00;
//    uint16_t length = 0;
//    
//    uint8_t i=0;
//    uint8_t idx=0xFF;
//    for(i=0;i<MAX_NBR_SUPPORT;i++)
//    {
//      if(!memcmp((mac_nbr_descriptor_t*)&fan_mac_nbr.desc_table[i].mac_addr, 
//      s->packet->dst.address.ieee_address, 8))
//      {
//        idx = i;
//        break;
//      }
//    }       
////    if((idx != 0xFF))
////    {
////      p_nbr_desc = (mac_nbr_descriptor_t *)&fan_mac_nbr.desc_table[idx];
////    }
////    else
////    {
////      return 0xFF;//returing invalid frame type
////    } 
//      
//    if(p_nbr_ptr == NULL_POINTER)
//    {
//       return 0;
//    }
//    
//    mem_rev_cpy(mac_addr,s->packet->dst.address.ieee_address,8);
//    
//    PLME_get_request(phySHRDuration,&(s->packet->length),&shr_duration);
//    
//    node_current_time = 
//    (uint64_t)((uint64_t)((p_nbr_ptr->ufsi)*(uint64_t)(MAXIMUM_SLOT_DH51_FUCTION*120))/(uint64_t)16777216);
//    
//    node_current_time = node_current_time*1000;   // converting into microsec
//    get_time = timer_current_time_get();
//    get_time= ((get_time- (p_nbr_ptr->ut_ie_rx_time))+380);    
//    node_current_time = (uint64_t)((uint64_t)node_current_time+(uint64_t)(get_time)); 
//    
//    dev_slot_no =(node_current_time/120067);
//    dev_slot_no = (uint16_t)floor(dev_slot_no);  
//   
//    PLME_get_request( phyMaxSUNChannelSupported, &length, &current_max_channels );
//    unicast_hop_channel = getUnicastChannelIndex(dev_slot_no,mac_addr,current_max_channels);
//    
//    broadcast_hop_channel = 
//    getBroadcastChannelIndex(
//                             broadcast_slot_nuumber,
//                             mac_self_fan_info.bcast_sched.bcast_sched_id,
//                             current_max_channels
//                             );
//    
//    if(mac_self_fan_info.unicast_listening_sched.us_schedule.channel_function != CF_FIXED_CHANNEL)
//    {
//        s->packet->data->TxChannel = unicast_hop_channel;
//        PLME_set_request( phyCurrentChannel, 2, &unicast_hop_channel);
//    }
//    
//    if(mac_self_fan_info.bcast_sched.bs_schedule.channel_function != 0x00)
//    {  
//        s->packet->data->TxChannel = unicast_hop_channel;
//        PLME_set_request( phyCurrentChannel, 2, &broadcast_hop_channel);
//    }
//    return 1;
//}
//#else
#if (APP_LBR_ROUTER == 1)  
extern uint8_t get_node_type( void );
#endif
extern const float k1;
#ifdef WISUN_FAN_MAC
#if(FAN_FRQ_HOPPING_FEATURE_ENABLED == 1)

uint8_t set_channel_for_freq_hop(trxsm_t *s)
{ 
  if(mac_self_fan_info.unicast_listening_sched.us_schedule.channel_function != CF_FIXED_CHANNEL)
  {
    //float differnce_of_node1 = 0x00;
    uint32_t differnce_T1T0 = 0x00;
    uint32_t current_max_channels=0x00;
    uint16_t length = 0;
    //    uint32_t TimeIntoSlotT2 = 0x00;
    int unicast_channel_index = 0 ;
    uint32_t CurrentSlotAtNode1 = 0x00;
    uint16_t unicast_hop_channel = 0;
    //    uint64_t total_defrence  = 0;
    uint32_t Diffrence_current_time_and_rx_time = 0x00;
    uint64_t current_time = get_time_now_64();
    uint8_t recv_node_mac_addr[8] = {0};
    /*Node1’s (T1-T0) = Ceiling ( UFSINode1 * SL * UDINode1 / 2^24 )
    Node2 calculates the current slot and intra-slot timing of Node1 for transmission time T2’ 
    CurrentSlotAtNode1 = (floor (((T2’ – T1’) + (T1 – T0)) / UDINode1)) MOD SL 
    TimeIntoSlotT2 = ((T2’ – T1’) + (T1 – T0)) – (CurrentSlotAtNode1 * UDINode1) 
    Note that the implementation must account for roll-over (modulus function) on CurrentSlotAtNode1. 
    TimeIntoSlotT2 may be a value such that Node2’s PHR cannot complete before the UDI of Node1 ends. T2’ 
    (T2)  should be moved such that the Node2 frame transmission begins in the next slot.
    */
    mac_nbr_descriptor_t *p_nbr_desc = (mac_nbr_descriptor_t *)get_nbr_desc_from_addr(s->packet->dst.address.ieee_address);
    if (p_nbr_desc == NULL)
      return unicast_hop_channel;
    
    PLME_get_request( phyMaxSUNChannelSupported, &length, &current_max_channels );
//    if(mac_self_fan_info.unicast_listening_sched.us_schedule.channel_function == CF_TR51)
//    {
//      differnce_T1T0 = (uint32_t)(ceil ((float)((float)p_nbr_desc->ufsi / (k1 / (float)p_nbr_desc->ucl_sched.us_schedule.dwell_interval))));
//      Diffrence_current_time_and_rx_time = (uint32_t) (ceil ((float)(current_time - p_nbr_desc->ut_ie_rx_time) / (float)1000)); //reciv in micro sec 1000 for convert in milisec
//      CurrentSlotAtNode1 = (uint16_t )(floor ((float)(Diffrence_current_time_and_rx_time + differnce_T1T0) / (float)p_nbr_desc->ucl_sched.us_schedule.dwell_interval))% 65536;
//      unicast_hop_channel = HopSequenceTable[CurrentSlotAtNode1];
//      //        TimeIntoSlotT2 =  (Diffrence_current_time_and_rx_time + differnce_T1T0) - (CurrentSlotAtNode1 * p_nbr_desc->ucl_sched.us_schedule.dwell_interval);
//      return unicast_hop_channel;
//    }
    if(mac_self_fan_info.unicast_listening_sched.us_schedule.channel_function == CF_DH1)
    {
      mem_rev_cpy(recv_node_mac_addr,s->packet->dst.address.ieee_address,8);
      //        total_defrence = (65536 * p_nbr_desc->ucl_sched.us_schedule.dwell_interval);
      //        differnce_T1T0 = ceil ( p_nbr_desc->ufsi * total_defrence / 16777216 );
      differnce_T1T0 = (uint32_t)(ceil ((float)((float)p_nbr_desc->ufsi / (k1 / (float)p_nbr_desc->ucl_sched.us_schedule.dwell_interval))));
      Diffrence_current_time_and_rx_time = (uint32_t) (ceil ((float)(current_time - p_nbr_desc->ut_ie_rx_time) / (float)1000)); //reciv in micro sec 1000 for convert in milisec
      CurrentSlotAtNode1 = (uint16_t )(floor ((float)(Diffrence_current_time_and_rx_time + differnce_T1T0) / (float)p_nbr_desc->ucl_sched.us_schedule.dwell_interval))% 65536;
      
      unicast_channel_index = getUnicastChannelIndex(CurrentSlotAtNode1,recv_node_mac_addr,p_nbr_desc->nbrchannel_usable_list.total_usable_ch_unicast);
      unicast_hop_channel = p_nbr_desc->nbrchannel_usable_list.unicast_usable_channel_list[unicast_channel_index];
      
      //        TimeIntoSlotT2 =  (Diffrence_current_time_and_rx_time + differnce_T1T0) - (CurrentSlotAtNode1 * p_nbr_desc->ucl_sched.us_schedule.dwell_interval);
      return unicast_hop_channel;
    }
  }
  return s->packet->data->TxChannel;  
}

#endif

#endif //#ifdef WISUN_FAN_MAC


