/** \file mac_rcv.c
 *******************************************************************************
 ** \brief This file processes all the received packets 
 **
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
#include "buff_mgmt.h"
#include "list_latest.h"
#include "sw_timer.h"
#include "timer_service.h"
#include "phy.h"
#include "mac_config.h"
#include "mac.h"
#include "mac_queue_manager.h"
#include "mac_defs_sec.h"
#include "ie_element_info.h"
#include "ie_manager.h"
#include "mac_defs.h"
#include "mac_pib.h"
#include "macutils.h"
#include "sm.h"

#if(CFG_MAC_SFTSM_ENABLED == 1)
#include "pandesc.h"
#include "sftsm.h"
#endif

#if(CFG_MAC_MPMSM_ENABLED == 1)
#include "mpmsm.h"
#endif
#if(CFG_MAC_SFTSM_ENABLED == 1)  
#include "startsm.h"
#endif
#include "ccasm.h"
#include "trxsm.h"

#if(CFG_MAC_DRSM_ENABLED == 1)
#include "drsm.h"
#endif

#if(CFG_MAC_SYNCSM_ENABLED ==1)
#include "syncsm.h"
#endif

#include "mac_frame_parse.h"

#if(CFG_MAC_SCANSM_ENABLED == 1)  
#include "scansm.h"
#endif
#if(CFG_ASSOCSM_ENABLED == 1)
#include "assocsm.h"
#include "enackwaitsm.h"
#endif

#include "mac_uplink.h"
//#include "trx_utility.h"
#include "event_manager.h"
#include "fan_mac_ie.h"
#include "mac_frame_build.h"
   
#if ( ( MAC_CFG_LE_RIT_CAPABILITY == 1 ) || ( MAC_CFG_LE_CSL_CAPABILITY == 1 ) )
#include "mac_le.h"
#endif

#ifdef UTEST_TRX
#include "utest_utils.h"
#endif

#ifdef MAC_CFG_SECURITY_ENABLED
#include "mac_security.h"
#endif

/*
** =============================================================================
** Private Macro definitions
** =============================================================================
*/
#define FRAME_SUBSCRIPTION_BUF_SIZE 1500


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

rx_Frame_stats_t rcv_pkts_details;
//static uint16_t rakaDebugCount = 0;


/*
** =============================================================================
** Public Variables Definitions
** =============================================================================
**/

uint8_t sendSubscribedPacket =0;

/*
** ============================================================================
** External Variable Declarations
** ============================================================================
*/

  extern self_info_fan_mac_t mac_self_fan_info;
  extern uchar aExtendedAddress[8];
  extern mac_tx_t ack_out;/*Umesh : 15-01-2018*//*this varribale should be in fan_mac_frame_parse.c for temp reason added here*/
  extern void backup_trxsm_state_for_ack_sending();
#ifdef MAC_CFG_SECURITY_ENABLED
        extern int get_join_state (void);
	extern mac_security_data_t mac_security_data;
        extern void add_dev_desc_on_MAC_for_security(uint8_t* macAddrOfNeighbour);
#endif

        extern mac_pib_t mac_pib;
        extern trxsm_t trxsm;
#if(CFG_MLME_SYNC_REQ == 1)
	extern syncsm_t syncsm;
	extern syncsm_t *syncsm_p;
#endif
#if(CFG_MAC_SCANSM_ENABLED == 1)          
        extern scansm_t scansm;
#endif        
#if(CFG_MAC_DRSM_ENABLED == 1)        
        extern drsm_t drsm;
#endif        
        
        
#if(CFG_MAC_SFTSM_ENABLED == 1)          
        extern startsm_t *startsm_p;
#endif 

#if(CFG_ASSOCSM_ENABLED ==1)        
        extern assocsm_t assocsm;
        extern enacksm_t *enacksm_p;
#endif        

#if ( ( MAC_CFG_LE_RIT_CAPABILITY == 1 ) || ( MAC_CFG_LE_CSL_CAPABILITY == 1 ) )
	extern low_energy_t low_energy;
#endif

        extern uint32_t hw_tmr_get_symbols( uint32_t time );
        extern mac_data_t mac_data;
        extern void app_bm_free(uint8_t *pMem);
        extern void swapbits(uint8_t* data);
#if (APP_LBR_ROUTER == 1) 
#ifdef MAC_ADDRESS_FILTERING_ENABLE
        extern uint8_t validate_filter_mac_address(uint8_t* ieee_address);
        extern void App_factory_mode_Data_ind_cb( uint16_t msduLength,uint8_t* pMsdu, int8_t mpduLinkQuality,uint16_t fcs_length);   
#endif
 extern void App_factory_mode_Data_ind_cb( uint16_t msduLength,uint8_t* pMsdu, int8_t mpduLinkQuality,uint16_t fcs_length);        
#endif        
     
extern uint8_t hif_send_msg_up(uint8_t* p_Msg, uint16_t msg_len, uint8_t layer_id, uint8_t protocol_id ); 
#ifdef WISUN_FAN_MAC
extern uchar Create_ACK_FRAME_Request( mac_rx_t *mrp,
                                      uint32_t sub_hdr_bitmap,
                                      uint32_t sub_pld_bitmap);
extern uint8_t mac_frame_parse_payload_ie(mac_rx_t *mrp, uchar *data);
extern uchar extract_header_ie( uchar *content, int content_len, mac_rx_t *mrp, mac_nbr_descriptor_t* p_nbr_desc);
extern void process_ws_async_frame(mac_rx_t* mac_rec_msg);
extern uint8_t mac_frame_update_payload_ie(mac_rx_t *mrp, uchar *data);
extern void acquire_lbr_brodcast_shedule_start_self_broadcast_shedule (mac_rx_t *mrp);
extern void add_ie_list_in_fan_ack_pkt(fan_mac_param_t *fan_mac_params);
//extern void update_ack_received(uint8_t *ack_rcvd_addr, uint8_t *self_addr);
extern parse_result_t mac_frame_find_hdr_ie_offset (mac_rx_t *mrp, uint8_t *data);
extern uint8_t extract_ies (mac_rx_t *mrp);
mac_nbr_descriptor_t* get_nbr_desc_from_addr (uint8_t *addr);
extern uint8_t is_valid_btie_bsie_received (mac_rx_t *mrp);
#if (APP_LBR_ROUTER == 1)  
extern uint8_t get_node_type( void );
extern uint8_t is_in_pantimeout (void);
extern void update_rssi_for_every_pakt(int8_t mpduLinkQuality,uint8_t *recvd_mac_addr);
#endif
mac_nbr_descriptor_t* get_nbr_desc_from_addr (uint8_t* p_nbr_addr);
#endif
/*
** ============================================================================
** Private Function Prototypes
** ============================================================================
*/

#ifdef MAC_CFG_SECURITY_ENABLED
        static void mac_trigger_incoming_sec_processing(  mac_rx_t *mrp );
#endif

#ifndef WISUN_FAN_MAC 
        static void process_mac_command( mac_rx_t *rxmsg );
#endif
        static void to_symbols(uint32_t *time);
        static void process_pd_2_mac_incoming_frames( mac_rx_t *mrp );

        void send_subscribed_packet(uint16_t length, uint8_t* buf);                              //Arjun: 14-11-17 to send subscribed packet
#ifdef ENHANCED_ACK_SUPPORT
        static uint8_t trigger_enhanced_ack_transmission( mac_rx_t *mac_rxp );
        static void process_enack( mac_rx_t *rxmsg );
#endif

#ifdef MAC_CFG_SECURITY_ENABLED
        static void clear_security_fields( mac_rx_t *mac_rxp );
#endif
        
/*
** ============================================================================
** Public Function Prototypes
** ============================================================================
*/



/*
** ============================================================================
** Public Function Definitions
** ============================================================================
*/
        
static void process_pd_2_mac_incoming_frames( mac_rx_t *mrp )
{
  /*Uemsh : 11-01-2018 this thing goes to modified after sepration of mac and fan mac*/
#ifndef WISUN_FAN_MAC  
  uint8_t cmd_id  = 0xFF;
  uint8_t aNullExtendedAddress[8] = {0x00};
#endif
  
#if ( ( CFG_MLME_SYNC_REQ == 1 ) || ( CFG_MAC_SCANSM_ENABLED == 1 ) || (CFG_MAC_DRSM_ENABLED == 1) )    
  sm_event_t event;
#endif    
  
#ifdef MAC_CFG_SECURITY_ENABLED
  uint8_t security_enable ; //= mac_pib.mac_security_enabled;
#endif
  
#if ( ( MAC_CFG_LE_RIT_CAPABILITY == 1 ) || ( MAC_CFG_LE_CSL_CAPABILITY == 1 ) )
  bool discard_le_frame = false;
  low_energy_state_ind_t curr_le_rit_state = low_energy_get_state(&low_energy);
#endif
  
#ifdef MAC_CFG_SECURITY_ENABLED    
  // Raka  for security configuration ... change ..
  if ( mrp->pd_rxp->psdu[0] & MAC_SECURITY_ENABLED )
  {
    security_enable  =  mac_pib.mac_security_enabled;
  }
  else
  {
    security_enable  =  0;
  }
#endif
  
  /*============================================================================*/	
  
#if ( ( MAC_CFG_LE_RIT_CAPABILITY == 1 ) || ( MAC_CFG_LE_CSL_CAPABILITY == 1 ) ) 
  /* ignore frame when RIT is ON and when waiting for RIT data request command frames */
  if( mac_pib.LEenabled )
  {				
    if( ( mrp->type == MAC_FRAME_TYPE_BEACON ) || ( mrp->type == MAC_FRAME_TYPE_DATA ) )
    {
      if( curr_le_rit_state == LE_STATE_DATA_TX_WAIT )
      {
        discard_le_frame = true;	
      }
    }
    else
    {
      if ( ((curr_le_rit_state == LE_STATE_DATA_TX_WAIT) && (cmd_id != LE_RIT_DATA_REQUEST))
          || ((curr_le_rit_state == LE_STATE_RIT_DATA_WAIT)&&(cmd_id == LE_RIT_DATA_REQUEST))
            || ((curr_le_rit_state == LE_STATE_INIT)&&(cmd_id == LE_RIT_DATA_REQUEST)) )
      {
        discard_le_frame = true;
      }
    }    	    	
  }
  
  else if ( ( mrp->type == MAC_FRAME_TYPE_MAC_COMMAND ) && ( cmd_id == LE_RIT_DATA_REQUEST ) )
  {    			
    discard_le_frame = true;		
  }
  
  if( discard_le_frame )
  {
    discard_le_frame = false;
    mac_free_rcv_buffer( mrp );
    return;
  }	    
#endif
  
  /* check if we are in promiscuous mode */
  if( mac_pib.PromiscuousMode )
  {
    /* convert timestamp to symbols */
    /*pdp->sfd_rx_time = hw_tmr_get_symbols( pdp->sfd_rx_time );*/ // Sagar
    
    to_symbols(&(mrp->pd_rxp->sfd_rx_time));
    
    /* queue it on the received message queue */
    queue_manager_push_back( QUEUE_RX_MSG, (queue_item_t *) mrp );
    
    event_set(CMD_OR_DATA_FRAME_RX_EVENT);
    
    /* and finish */
    return;
  }
  
  /*send enhanced ack if AR=1 in the received frame*/
#ifdef ENHANCED_ACK_SUPPORT
#ifdef MAC_CFG_SECURITY_ENABLED            
  if( !security_enable )
#endif	    
  {
    /*if security build is enabled and if the  security is not 
    enabled then enhanced ack preparation and queing will be initiated.
    Otherwise if sec is enabled, lets wait till the sec processing is done*/
    trigger_enhanced_ack_transmission(mrp);  
  }
#endif     
  
#if (APP_LBR_ROUTER == 1) 
#ifdef MAC_ADDRESS_FILTERING_ENABLE  
  /* Debdeep :: Commented this :: Mac address filter validation 
  is done just after mac_frame_parse_addresses */
  //     /* Raka: Decive MAC Address filtering of receiving Packets
  //    adding for rpl address filtering for source */
  //    
  //      if((validate_filter_mac_address( mrp->src.address.ieee_address)!=0))
  //      {
  //          mac_free_rcv_buffer( mrp );
  //          return;
  //      }
#endif
#endif
  
  /* check message type and queue message */
  switch( mrp->frame_type )
  {//start switch mrp->type
  case MAC_FRAME_TYPE_BEACON:
    {
      /*Uemsh : 11-01-2018 this thing goes to modified after sepration of mac and fan mac*/
#ifdef WISUN_FAN_MAC          
      mac_free_rcv_buffer( mrp );//for WISUN-FAN droping the unsupported BEACON frame type          
#else  
      
#ifdef UTEST_TRX
      utu_timestamp( UTUL_RX_BCN, pdp->sfd_rx_time );
#endif
      
#ifdef ENABLE_DEBUG_EVENTS			
      Indicate_Debug_Event(BEACON_RX);
      utu_timestamp( BEACON_RX, pdp->sfd_rx_time );
#endif	
#if(CFG_MLME_SYNC_REQ == 1)
      /* indicate received beacon to SYNC-SM */
      if( ( syncsm_p ) && ( mrp->frame_ver != FRAME_VERSION_2011 ) )
      {
        param.beacon_order = mrp->payload[0] & SF0_BEACON_ORDER_MASK;
        param.superframe_order = ( mrp->payload[0] & SF0_SUPERFRAME_ORDER_MASK ) >> 4;
        param.final_cap_slot = mrp->payload[1] & SF1_FINAL_CAP_SLOT;
        param.battery_life_ext = mrp->payload[1] & SF1_BATTERY_LIFE_EXTENSION;
        param.time_reference = mrp->pd_rxp->sfd_rx_time;
        
        event.trigger = (sm_trigger_t) SYNCSM_TRIGGER_BCN_RECEIVED;
        event.param.vector = &param;
        SM_DISPATCH( (sm_t *) syncsm_p, &event );
      }
#endif
      
      /* Drop the beacon frame if:
      1. If the node is NOT scanning and the received beacon is not 
      from same PAN.
      2. If the node is NOT scanning and the received beacon is unicast 
      and not for yourself.
      */
#if(CFG_MAC_SCANSM_ENABLED == 1)           
      if( (scansm_get_state( &scansm ) == SCANSM_STATE_NONE) && /*not Scanning*/
         ((mrp->src.pan_id != mac_pib.PANId)|| 
          (ieeeaddr_cmp( mrp->dst.address.ieee_address, aExtendedAddress)!=0)))
      {
        mac_free_rcv_buffer( mrp );
        break;
      }
      
      
      /* check if not scanning */
      /*note: during a scan no need to change transceiver state after 
      receiving a beacon, receiver must remain enabled until the end 
      of the scan */
      if( (scansm_get_state( &scansm ) == SCANSM_STATE_NONE) && 
         ( mrp->frame_ver != FRAME_VERSION_2011 ) )
      {
        /* indicate received beacon to TRX-SM */
        event.trigger = (sm_trigger_t) TRXSM_TRIGGER_BCN_RECEIVED;
        event.param.scalar = 0;
        SM_DISPATCH( (sm_t *) &trxsm, &event );
      }
      
      /*drop the regular/normal beacons during MPM EB scanning*/
      if( (scansm_get_state( &scansm ) == SCANSM_STATE_ACTPASS) 
         && (( scansm.flags & SCANSM_FLAG_TYPE_MASK ) == SCANSM_FLAG_TYPE_MPM_EB_PASSIVE)
           && (mrp->frame_ver != FRAME_VERSION_2011) )
      {
        mac_free_rcv_buffer( mrp );
        break;
      }
#endif 
      /* convert timestamp to symbols */
      /*pdp->sfd_rx_time = hw_tmr_get_symbols( pdp->sfd_rx_time );*/ // Sagar
      
      to_symbols(&(mrp->pd_rxp->sfd_rx_time));
      
#ifdef MAC_CFG_SECURITY_ENABLED            
      if( security_enable )
      {
        mac_trigger_incoming_sec_processing( mrp );
      }
      else
#endif            
      {
        /* queue beacon for further processing */
        queue_manager_push_back( QUEUE_BCN_RX,
                                (queue_item_t *) mrp );
        
        set_process_activity(AF_BEACON_RECEIVED);
        
        event_set(BCN_RX_EVENT);
      }
#endif //#ifdef WISUN_FAN_MAC
    }
    break;
    
    
  case MAC_FRAME_TYPE_ACK:/*fall through*/  
  case MAC_FRAME_TYPE_DATA:
    { 
      
#ifdef UTEST_TRX
      utu_timestamp( UTUL_RX_DATA, 0 );
#endif
      
#ifdef ENABLE_DEBUG_EVENTS
      Indicate_Debug_Event(DATA_FRAME_RX);
      utu_timestamp( DATA_FRAME_RX, 0 );
#endif
      
#if(CFG_MAC_SCANSM_ENABLED == 1)          
      /* ignore frame during active or passive scan */
      if( scansm_get_state( &scansm ) == SCANSM_STATE_ACTPASS )
      {				
        mac_free_rcv_buffer( mrp );
        break;
      }
#endif //#if(CFG_MAC_SCANSM_ENABLED == 1)  
      /* convert timestamp to symbols */
      
      to_symbols(&(mrp->pd_rxp->sfd_rx_time));
#if(CFG_MAC_DRSM_ENABLED == 1)
      /* indicate packet to DR-SM */
      event.trigger = (sm_trigger_t) DRSM_TRIGGER_ARRIVED;
      event.param.vector = mrp;
      SM_DISPATCH( (sm_t *) &drsm, &event );
#endif
#ifdef MAC_CFG_SECURITY_ENABLED            
      if( security_enable )
      {
//#if (PRINT_DEBUG_LEVEL==ERROR_DEBUG)
//        stack_print_debug ("\nSPR: ");
//        print_mac_address (mrp->src.address.ieee_address);
//#endif
        mac_trigger_incoming_sec_processing( mrp );
      }
      else
#endif
      {
//#if (PRINT_DEBUG_LEVEL==ERROR_DEBUG)
//        stack_print_debug ("\nUSPR: ");
//        print_mac_address (mrp->src.address.ieee_address);
//#endif        
        /* queue it on the received message queue */
        queue_manager_push_back( QUEUE_RX_MSG, (queue_item_t *) mrp );
        set_process_activity(AF_RCV_MSG_PENDING);
        event_set(CMD_OR_DATA_FRAME_RX_EVENT);
      }
      
#ifdef MAC_CFG_GTS_ENABLED
      if( mac_data->current_slot >= mac_data->gts_start_slot)
      {
        if( mac_data->current_gts != NULL_POINTER )
        {
          mac_data->current_gts->expiration_timer = mac_get_gts_expiration_time();
        }
      }
#endif
    }
    break;
    
  case MAC_FRAME_TYPE_MAC_COMMAND:
    {
      /*Uemsh : 11-01-2018 this thing goes to modified after sepration of mac and fan mac*/
#ifdef WISUN_FAN_MAC          
      mac_free_rcv_buffer( mrp );//for WISUN-FAN droping the unsupported BEACON frame type
      
#else 
      
#ifdef UTEST_TRX
      utu_timestamp( UTUL_RX_CMD, mrp->payload[0] );
#endif
      
      /* if the command is a beacon request/Enhanced beacon request, we 
      should drop it here if the node is not a PAN coordinator: this needs to 
      be more generic*/	
      if( (cmd_id == BEACON_REQUEST) &&
         (ieeeaddr_cmp( mac_pib.CoordExtendedAddress, aNullExtendedAddress)!=0))
      {
        mac_free_rcv_buffer( mrp );
        break;
      }
#if(CFG_MAC_SCANSM_ENABLED == 1) 
      /* ignore frame during active or passive scan */
      if( scansm_get_state( &scansm ) == SCANSM_STATE_ACTPASS )
      {				
        mac_free_rcv_buffer( mrp );
        break;
      }
#endif
      /* convert timestamp to symbols */
      /*pdp->sfd_rx_time = hw_tmr_get_symbols( pdp->sfd_rx_time );*/ // Sagar
      
      to_symbols(&(mrp->pd_rxp->sfd_rx_time));
      
      if( cmd_id == DATA_REQUEST 
#if ( ( MAC_CFG_LE_RIT_CAPABILITY == 1 ) || ( MAC_CFG_LE_CSL_CAPABILITY == 1 ) )
         || (cmd_id == LE_RIT_DATA_REQUEST)
#endif
           )
      {
#ifdef ENABLE_DEBUG_EVENTS                
        Indicate_Debug_Event( DATA_REQUEST_RX );
#endif
        if( cmd_id == LE_RIT_DATA_REQUEST )
        {
          if (	(mac_find_indirect_transmission( &mrp->src, 0 ) == MAC_SUCCESS ) || 
              ( (mrp->dst.address_mode == MAC_SHORT_ADDRESS)&&
               (mrp->dst.pan_id == BROADCAST_PAN_ID)&&
                 (mrp->dst.address.short_address == BROADCAST_SHORT_ADDRESS) &&
                   ( mac_find_indirect_transmission( &mrp->dst, 1 ) == MAC_SUCCESS ) )
                )
          {
            /*there is a data pending with the dest address as the src 
            address of the node which sent this RIT Data req command frame*/
            mrp->frame_pending_out = 1;
          }
        }
        /* only queue data request if we indicated a frame pending */
        if( mrp->frame_pending_out )
        {
#ifdef MAC_CFG_SECURITY_ENABLED            
          if( security_enable )
          {
            mac_trigger_incoming_sec_processing( mrp );
          }
          else
#endif
          {
            queue_manager_push_back( QUEUE_DATA_REQUEST, (queue_item_t *) mrp );
            event_set(DATA_REQ_RX_EVENT);	
          }                		
        }
        else
        {
          /* discard this frame */
          mac_free_rcv_buffer( mrp );
          break;
        }
      }
      else
      {
#if(CFG_MAC_SCANSM_ENABLED == 1)           
        /* indicate packet to DR-SM */
        event.trigger = (sm_trigger_t) DRSM_TRIGGER_ARRIVED;
        event.param.vector = mrp;
        SM_DISPATCH( (sm_t *) &drsm, &event );
#endif
        
#ifdef MAC_CFG_SECURITY_ENABLED            
        if( security_enable )
        {
          mac_trigger_incoming_sec_processing( mrp );
        }
        else
#endif
        {
          /* queue it on the received message queue */
          queue_manager_push_back( QUEUE_RX_MSG, (queue_item_t *) mrp );
          
          event_set(CMD_OR_DATA_FRAME_RX_EVENT);
        }  				
        set_process_activity(AF_RCV_MSG_PENDING);                              
      }
#ifdef MAC_CFG_GTS_ENABLED
      if( mac_data->current_slot >= mac_data->gts_start_slot)
      {
        if( mac_data->current_gts != NULL_POINTER )
        {
          mac_data->current_gts->expiration_timer = mac_get_gts_expiration_time();
        }
      }
#endif
#endif      //#ifdef WISUN_FAN_MAC 
    }
    
    break;
    
  default:
#ifdef UTEST_TRX
    utu_timestamp(UTUL_RX_ERR, -1);
#endif
    break;
  }//end switch mrp->type
  return;
}

/******************************************************************************/

volatile int phy_state_event_post = 0;
volatile int phy_state_event_post1 = 0;
void * app_bm_alloc(
    uint16_t length //uint16_t length
    );

//typedef struct phy_2_mac_queue_data
//{
//  struct phy_2_mac_queue *link;
//  phy_rx_t *pdp;
//  bool state_change_req;
//}phy_2_mac_q_data_t;
//
//static queue_t phy_2_mac_q;
//
//void PD_Data_Indication_cb( phy_rx_t *pdp, bool state_change_req  )
//{
//  phy_2_mac_q_data_t *p_2_m_q = NULL;
//  
//  p_2_m_q = app_bm_alloc (sizeof (phy_2_mac_q_data_t));
//  if (p_2_m_q == NULL)
//  {
//    app_bm_free ((uint8_t *)pdp);
//    return;
//  }
//  
//  p_2_m_q->pdp = pdp;
//  p_2_m_q->state_change_req = state_change_req;
//  
//  queue_item_put (&phy_2_mac_q, (queue_item_t *)p_2_m_q);
//  event_set (PHY_2_MAC_EVENT);
//}

//void process_phy_packet ( void )
void PD_Data_Indication_cb( phy_rx_t *pdp, bool state_change_req  )
{
  mac_rx_t *mrp = NULL_POINTER;
  sm_event_t event;
//  trxsm_ack_t ack_in = {0};   
  parse_result_t parse_res = PARSE_STORED;
  //uchar frame_type = 0,frame_ver = 0;

//  phy_2_mac_q_data_t *p_2_m_q = (phy_2_mac_q_data_t *)queue_item_get (&phy_2_mac_q);
//  
//  if (p_2_m_q == NULL)
//    return;
//  
//  phy_rx_t *pdp = p_2_m_q->pdp;
//  bool state_change_req = p_2_m_q->state_change_req;
//  app_bm_free ((uint8_t *)p_2_m_q);
  
#ifdef UTEST_TRX
  utu_timestamp( UTUL_RX_IND, pdp->data[2] ); /* parameter: sequence number */
#endif
  if( pdp == NULL )
  {
    if( state_change_req )
    {
      phy_state_event_post++;
      /* if TRX state is required, then indicate to TRX-SM which does TRX state changes as per MAC spec */
      event.trigger = (sm_trigger_t) TRXSM_TRIGGER_PD_DATA_INDICATION;
      event.param.scalar = 0;
      if( trxsm.super.state != NULL )
      {
        phy_state_event_post1++;
        SM_DISPATCH( (sm_t *) &trxsm, &event );	
      }		
    }
    return;
  }
#if (APP_LBR_ROUTER == 1)    
  if(mac_pib.PromiscuousMode)
  {   
      App_factory_mode_Data_ind_cb(pdp->psduLength,pdp->psdu,pdp->rssi,pdp->FCSLength);
      app_bm_free( (uint8_t*)pdp );
      PLME_Set_TRX_State( PHY_RX_ON );
      return;
  }
#endif  
  /* forward the received data to the application if the MAC is disabled */
  if (mac_pib.ConfigFlags & MAC_DISABLED)
  {    
    to_symbols(&pdp->sfd_rx_time);
    app_bm_free( (uint8_t*)pdp );  
    
//#if (PRINT_DEBUG_LEVEL==ERROR_DEBUG)
//    stack_print_debug ("B phy state = %d\n", plme_get_trx_state_request ());
//#endif       
    
    PLME_Set_TRX_State( PHY_RX_ON );
    
//#if (PRINT_DEBUG_LEVEL==ERROR_DEBUG)
//    stack_print_debug ("C phy state = %d\n", plme_get_trx_state_request ());
//#endif 
    
    return;
  }
#if(CFG_MAC_SCANSM_ENABLED == 1) 
  /* ignore frame during ED scan */
  if( scansm_get_state( &scansm ) == SCANSM_STATE_ED )
  {		
    app_bm_free( (uint8_t*)pdp );/*free mem*/
    /* if TRX state is required, then indicate to TRX-SM which does TRX state changes as per MAC spec */
    event.trigger = (sm_trigger_t) TRXSM_TRIGGER_PD_DATA_INDICATION;
    phy_state_event_post++;
    event.param.scalar = 0;
    if( trxsm.super.state != NULL )
    {
      phy_state_event_post1++;
      SM_DISPATCH( (sm_t *) &trxsm, &event );	
    }
    return;
  }
#endif //#if(CFG_MAC_SCANSM_ENABLED == 1) 
  
  /* allocate a MAC receive message buffer */
  /*TBD Shouldn't we check NULL return value? */
  mrp = (mac_rx_t *) queue_manager_pop_front( QUEUE_RX_FREE_MSG );
  
  if( mrp == NULL_POINTER )
  {
    app_bm_free( (uint8_t*)pdp );/*free mem*/
    
    if( state_change_req )
    {
      /* if TRX state is required, then indicate to TRX-SM which does TRX state changes as per MAC spec */
      event.trigger = (sm_trigger_t) TRXSM_TRIGGER_PD_DATA_INDICATION;
      phy_state_event_post++;
      event.param.scalar = 0;
      if( trxsm.super.state != NULL )
      {
        phy_state_event_post1++;
        SM_DISPATCH( (sm_t *) &trxsm, &event );	
      }		
    }
#if ((PRINT_DEBUG_LEVEL==ERROR_DEBUG)||(PRINT_DEBUG_LEVEL==CONSOLE_ERROR_DEBUG))
    stack_print_debug ("### MAC RCV Q not available\n");
#endif
    return;
  }
#if (APP_LBR_ROUTER == 1)  
  mrp->wisun_fan_ies = (struct ieee802154_ies *)app_bm_alloc (sizeof (struct ieee802154_ies));
  if (NULL == mrp->wisun_fan_ies)
  {
    app_bm_free((uint8_t*)mrp->pd_rxp);
    queue_manager_push_back( QUEUE_RX_FREE_MSG, (queue_item_t *) mrp );
    return;
  }
  
//#if (PRINT_DEBUG_LEVEL==ERROR_DEBUG)
//  stack_print_debug ("2M %d %p\n", sizeof (struct ieee802154_ies), mrp->wisun_fan_ies);
//#endif   
  
#endif  
#ifdef MAC_CFG_SECURITY_ENABLED    
  mrp->security_status = MAC_SUCCESS;
#endif
  
  mrp->headerIEListLen = mrp->headerIEFieldLen  = mrp->payloadIEListLen = mrp->payloadIEFieldLen = 0;		 
  mrp->headerIEList = mrp->payloadIEList = NULL;	 		
  mrp->pld_ies_present = 0;
  
  parse_res = mac_frame_parse( &trxsm, pdp, mrp);
  
  event.trigger = (sm_trigger_t) TRXSM_TRIGGER_PD_DATA_INDICATION;
  phy_state_event_post++;
  event.param.scalar = 0;
  if( trxsm.super.state != NULL )
  {
    phy_state_event_post1++;
    SM_DISPATCH( (sm_t *) &trxsm, &event );
    PLME_Set_TRX_State( PHY_RX_ON ); 
  }
  else
  {
    PLME_Set_TRX_State( PHY_RX_ON );
  }
  
//  frame_type = pdp->psdu[0] & MAC_FRAME_TYPE_MASK;
//  frame_ver = pdp->psdu[1] & FRAME_VERSION_MASK;
//  if(frame_type == MAC_FRAME_TYPE_ACK )
//  {
//#if APP_LBR_ROUTER  
//    update_ack_received(ack_in.src_long_addr, ack_in.dest_long_addr);
//#endif
//  }
  /*check if the IES are present*/
  if ( mrp->ie_present ) 
  {
    if ((parse_res == PARSE_DISCARD) || (parse_res == PARSE_ERROR))
    {
      mac_free_rcv_buffer( mrp );/*free mem*/
      return;
    }
    
    mrp->payload_length = (mrp->pd_rxp->psduLength -(mrp->pd_rxp->FCSLength & 0x7FFF));
    
    if( mrp->security_enable ) 
    {
      static const uint8_t MIC_Len[4] = {0, 4, 8, 16};
      
//      // Auxiliary Security Header Fieled checking .. Raka ..
//      if(!( pdp->psdu[1] & MAC_SEQ_NUM_SUPPRESSION))
//        mrp->payload_length -= MIC_Len[pdp->psdu[19]&3]; 
//      else
//        mrp->payload_length -= MIC_Len[pdp->psdu[18]&3]; 
      
      /* Debdeep :: mrp->auxiliary_secoffset_index is calculated in mac_frame_parse &*/
      mrp->payload_length -= MIC_Len[pdp->psdu[mrp->auxiliary_secoffset_index]&3];
    }
    
    mrp->headerIEListLen = 0;	 
    mrp->headerIEList = NULL;	 
    mrp->headerIEFieldLen = 0; 
#if APP_LBR_ROUTER  
    mrp->wisun_fan_ies->hdr_subIE_bitmap = 0;
    mrp->wisun_fan_ies->pld_subIE_bitmap = 0;
    mrp->wisun_fan_ies->mrp_ctx = (mac_rx_t *)mrp;
    /*Uemsh : 10-01-2018 this thing goes to modified after sepration of mac and fan mac*/
//#ifdef WISUN_FAN_MAC 
    
    parse_res = mac_frame_find_hdr_ie_offset (mrp, &mrp->pd_rxp->psdu[mrp->header_ie_offset]);
    mrp->header_ie_offset += mrp->headerIEFieldLen;                           
    
#endif 
    
  }
  
  /* fill MAC receive packet */
  mrp->payload = &mrp->pd_rxp->psdu[mrp->header_ie_offset]; /*ANAND*/
  /*TBD Should this include the MIC if present? */
  mrp->payload_length = mrp->pd_rxp->psduLength - mrp->header_ie_offset - (mrp->pd_rxp->FCSLength & 0x7FFF); /* 2 or 4 bytes CRC */ /*ANAND*/
  //mrp->type = frame_type;
  //mrp->frame_ver = frame_ver;
  
  if ( ( parse_res != PARSE_DISCARD ) &&( parse_res != PARSE_ERROR ))
    process_pd_2_mac_incoming_frames(mrp);	
  else		
    mac_free_rcv_buffer( mrp );/*free mem*/
  
//  event_clear (PHY_2_MAC_EVENT);
}

/******************************************************************************/
void subscribe_tx_frame (mac_tx_t *txd);
/* Debdeep :: This same function used for sending Secure and Unsecure ACK */
uint8_t trigger_SECURE_NON_SECURE_ACK (mac_tx_t *dr)   //Debdeep
{
  sm_event_t event;
  event.trigger = (sm_trigger_t) TRXSM_TRIGGER_ACK_REQUIRED;
  event.param.vector = dr;
  SM_DISPATCH( (sm_t *) &trxsm, &event );
  return MAC_SUCCESS;
}
#if APP_LBR_ROUTER
uint8_t validate_seq_no_incoming_pkt(mac_rx_t *mrp)
{
  mac_nbr_descriptor_t* p_nbr_desc = get_nbr_desc_from_addr (mrp->src.address.ieee_address);
  if(p_nbr_desc != NULL)
  {   
    if(p_nbr_desc->incoming_pkt_seq_no!= mrp->sn)
    {
      p_nbr_desc->incoming_pkt_seq_no = mrp->sn;
      p_nbr_desc->packet_status = INCOMING_PKT_STATUS_NONE;
      return 0;
    }
//#if ((PRINT_DEBUG_LEVEL==ERROR_DEBUG)||(PRINT_DEBUG_LEVEL==CONSOLE_ERROR_DEBUG))
//    stack_print_debug ("$$$ Rx seq num = %d, nbr seq num = %d, packet status = %d\n", mrp->sn, p_nbr_desc->incoming_pkt_seq_no, p_nbr_desc->packet_status);      
//#endif 
    if((p_nbr_desc->incoming_pkt_seq_no == mrp->sn)
            &&(p_nbr_desc->packet_status == INCOMING_PKT_SEND_TO_UPL))
    {
#if ((PRINT_DEBUG_LEVEL==ERROR_DEBUG)||(PRINT_DEBUG_LEVEL==CONSOLE_ERROR_DEBUG))
      stack_print_debug ("Discarding PKT\n");
#endif
      return 1;
    }
  }
//#if ((PRINT_DEBUG_LEVEL==ERROR_DEBUG)||(PRINT_DEBUG_LEVEL==CONSOLE_ERROR_DEBUG))
//  stack_print_debug ("Processing PKT\n");
//#endif
  return 0;
}
#endif
uint32_t subscribe_rx_count = 0;
uchar mac_process_received_messages( void )
{
  uint8_t status = 0xFF;
  mac_rx_t *rxmsg = NULL;
#ifdef MAC_CFG_SECURITY_ENABLED
  mac_address_t src_addr = {0};
  mac_address_t dst_addr = {0};
#endif
#ifdef WISUN_FAN_MAC
//  uint16_t size = 0;
//  uint8_t *buf_for_frame_subscription = NULL;
#endif
  
#if(CFG_MAC_DRSM_ENABLED == 1)    
  sm_event_t event;
#endif
  
  /* there is a buffer, so check for receive messages */
  while((rxmsg = (mac_rx_t *) queue_manager_pop_front(QUEUE_RX_MSG)) != NULL_POINTER)
  {//start rxmsg
    if (rxmsg == NULL_POINTER)
    {
      event_clear(CMD_OR_DATA_FRAME_RX_EVENT);
      /* nothing on the rcv queue to process so exit */
      return 0;
    }
    
//#if (PRINT_DEBUG_LEVEL==ERROR_DEBUG)
//    stack_print_debug ("frame fetched from queue for IE parsing: ");
//    print_mac_address (rxmsg->src.address.ieee_address);
//#endif      
    
#ifdef WISUN_FAN_MAC     
    if (rxmsg->ack_request)
    {
      if ((get_node_type () == 0x01) /*&& (is_in_pantimeout () == 0x00)*/)   //SUNEET :: Do it properly 
      {
        backup_trxsm_state_for_ack_sending(); 
        Create_ACK_FRAME_Request(rxmsg, UTT_IE_MASK | RSL_IE_MASK, NO_IE_MASK);
      }
      if (get_node_type () == 0x00)
      {
        backup_trxsm_state_for_ack_sending(); 
        Create_ACK_FRAME_Request(rxmsg, UTT_IE_MASK | RSL_IE_MASK, NO_IE_MASK);
      }
      
    }
#endif
  
    
#ifdef MAC_CFG_SECURITY_ENABLED
    /* Check the results of the Authentication Procedure */
    check_authentication_status(rxmsg);
#endif
    /*Uemsh : 11-01-2018 this thing goes to modified after sepration of mac and fan mac*/
#ifdef WISUN_FAN_MAC 
#if (APP_LBR_ROUTER == 1)       
    status = extract_ies (rxmsg);
    if(status == 0x01)//PARSE_DISCARD
    {
#if ((PRINT_DEBUG_LEVEL==ERROR_DEBUG)||(PRINT_DEBUG_LEVEL==CONSOLE_ERROR_DEBUG))
      stack_print_debug ("### IE Parsing fail :: Rx [%d] from ", rxmsg->sn);
      print_mac_address (rxmsg->src.address.ieee_address);
      stack_print_debug ("Discarding PKT\n\n");
#endif
      mac_free_rcv_buffer( rxmsg );
      event_clear(CMD_OR_DATA_FRAME_RX_EVENT);
      if ((rxmsg->frame_type == MAC_FRAME_TYPE_ACK) && (rxmsg->recived_frame_type == FAN_ACK))
#if APP_LBR_ROUTER         
        send_mcps_no_ack_indication ();
#endif      
      return 0;
    }
      
    if (((rxmsg->seqno_suppression) == 0) && (rxmsg->frame_type != MAC_FRAME_TYPE_ACK))
    {
      status = validate_seq_no_incoming_pkt(rxmsg);
      if(status == 0x01)
      {
        mac_free_rcv_buffer( rxmsg );
        event_clear(CMD_OR_DATA_FRAME_RX_EVENT);
        return 0;
      }
    }
#endif // #if (APP_LBR_ROUTER == 1)        
#endif        
    if (mac_pib.PromiscuousMode)
    {
      send_mcps_data_indication(rxmsg);
      rxmsg->pd_rxp = NULL;
      /*buffer allocated by the PHY layer being used for sending MCPS Data 
      Indication. SO just free the static buffer used for mac rx msg*/
      queue_manager_push_back( QUEUE_RX_FREE_MSG, (queue_item_t *) rxmsg );
      return 1;
    }
    else
    {
#if(CFG_MAC_DRSM_ENABLED == 1)        
      /* indicate packet to DR-SM */
      event.trigger = (sm_trigger_t) DRSM_TRIGGER_PROCESSING;
      event.param.vector = rxmsg;
      SM_DISPATCH( (sm_t *) &drsm, &event );
#endif

/* Debdeep :: ALL received packets are subscribed */
//#ifdef WISUN_FAN_MAC
//      size = rxmsg->pd_rxp->psduLength - (rxmsg->pd_rxp->FCSLength & 0x7FFF);
//      buf_for_frame_subscription = (uint8_t *)app_bm_alloc (size + 5);
////#if (PRINT_DEBUG_LEVEL==ERROR_DEBUG)
////      stack_print_debug ("4M %d %p\n", size + 5, buf_for_frame_subscription);
////#endif       
//      buf_for_frame_subscription[0] = WS_ASYNC_FRAME_INDICATION;
//
//      if (rxmsg->security_enable)  //Suneet :: change for frame 
//      {
//        rxmsg->pd_rxp->psdu[rxmsg->auxiliary_secoffset_index] = rxmsg->pd_rxp->psdu[rxmsg->auxiliary_secoffset_index] >> 3;
//        rxmsg->pd_rxp->psdu[rxmsg->auxiliary_secoffset_index] = rxmsg->pd_rxp->psdu[rxmsg->auxiliary_secoffset_index] << 3;
//        size -= 8;      /*For MIC*/
//      }
//      memcpy(&buf_for_frame_subscription[1], rxmsg->pd_rxp->psdu, size);
//      subscribe_rx_count++;
//      send_subscribed_packet(size+1, buf_for_frame_subscription);
////#if (PRINT_DEBUG_LEVEL==ERROR_DEBUG)
////      stack_print_debug ("4F %p\n", buf_for_frame_subscription);
////#endif        
//      app_bm_free((uint8_t*)buf_for_frame_subscription);
//#endif
      
      /* now decide what type of frame this is */
      switch (rxmsg->frame_type)
      {
      case MAC_FRAME_TYPE_DATA:
        {
#ifdef MAC_CFG_SECURITY_ENABLED
          if (rxmsg->security_status == SUCCESS)
          {
#endif
            /*Uemsh : 11-01-2018 this thing goes to modified after sepration of mac and fan mac*/
#ifdef WISUN_FAN_MAC 
#if(FAN_FRQ_HOPPING_FEATURE_ENABLED == 1)
            if((mac_self_fan_info.bcast_sched.bs_schedule.channel_function == CF_DH1) ||
               (mac_self_fan_info.bcast_sched.bs_schedule.channel_function == CF_FIXED_CHANNEL))
            {
              if ((get_node_type() != 0x00) && 
                  (is_valid_btie_bsie_received (rxmsg)))
              {
                acquire_lbr_brodcast_shedule_start_self_broadcast_shedule(rxmsg);
              }
            }
#endif
            
#if APP_LBR_ROUTER
            if (rxmsg->recived_frame_type != PAN_ADVERT_SOLICIT)
              update_rssi_for_every_pakt((int8_t)(rxmsg->pd_rxp->rssi),rxmsg->src.address.ieee_address);
#endif            
            switch(rxmsg->recived_frame_type)
            {
            case PAN_ADVERT_FRAME:/*0x00*/
#if (PRINT_DEBUG_LEVEL == CONSOLE_ERROR_DEBUG)
              stack_print_debug ("Rx-PA from ");
              print_mac_address (rxmsg->src.address.ieee_address);
#endif
#ifdef MAC_CFG_SECURITY_ENABLED               
              if((rxmsg->recived_frame_type == PAN_ADVERT_FRAME &&  get_join_state() == 0x01))
              {
                uint8_t node_addr[8] = {0x00};
                mem_rev_cpy (node_addr, rxmsg->src.address.ieee_address, 8);
                add_dev_desc_on_MAC_for_security(&node_addr[0]);
              }
#endif              
              process_ws_async_frame(rxmsg);    
              break;
            case PAN_ADVERT_SOLICIT:/*0x01*/
#if (PRINT_DEBUG_LEVEL == CONSOLE_ERROR_DEBUG)
              stack_print_debug ("Rx-PAS from ");
              print_mac_address (rxmsg->src.address.ieee_address);
#endif
#ifdef MAC_CFG_SECURITY_ENABLED 
              if (rxmsg->recived_frame_type == PAN_ADVERT_SOLICIT &&   get_join_state() == 0x5)
              {
                uint8_t node_addr[8] = {0x00};
                mem_rev_cpy (node_addr, rxmsg->src.address.ieee_address, 8);
                add_dev_desc_on_MAC_for_security(&node_addr[0]);
              }
#endif    
              process_ws_async_frame(rxmsg);    
              break;
            case PAN_CONFIG:/*0x02*/
#if (PRINT_DEBUG_LEVEL == CONSOLE_ERROR_DEBUG)
              stack_print_debug ("Rx-PC from ");
              print_mac_address (rxmsg->src.address.ieee_address);
#endif
              process_ws_async_frame(rxmsg);
              break;
            case PAN_CONFIG_SOLICIT:/*0x03*/
#if (PRINT_DEBUG_LEVEL == CONSOLE_ERROR_DEBUG)
              stack_print_debug ("Rx-PCS from ");
              print_mac_address (rxmsg->src.address.ieee_address);
#endif
              process_ws_async_frame(rxmsg);    
              break;
            case FAN_DATA_PKT :/*0x04*/
//#if (PRINT_DEBUG_LEVEL == CONSOLE_ERROR_DEBUG)
//              stack_print_debug ("Rx-DATA [%d] from ", rxmsg->sn);
//              print_mac_address (rxmsg->src.address.ieee_address);
//#endif
              send_mcps_data_indication(rxmsg);
              break; 
#if(FAN_EAPOL_FEATURE_ENABLED == 1)
            case EAPOL:/*0x06*/
//#if (PRINT_DEBUG_LEVEL == CONSOLE_ERROR_DEBUG)
//              stack_print_debug ("Rx-EAPOL [%d] from ", rxmsg->sn);
//              print_mac_address (rxmsg->src.address.ieee_address);
//#endif
              send_mcps_data_indication(rxmsg);
              break;
#endif  //#if(FAN_EAPOL_FEATURE_ENABLED == 1)
              
            default :
#if (PRINT_DEBUG_LEVEL == CONSOLE_ERROR_DEBUG)
              stack_print_debug ("Rx-INV from ");
              print_mac_address (rxmsg->src.address.ieee_address);
#endif
              app_bm_free((uint8_t*)rxmsg->pd_rxp);
              break;
            } 
#endif
            
            rxmsg->pd_rxp = NULL;
            /*buffer allocated by the PHY layer being used for sending MCPS Data
            Indication. SO just free the static buffer used for mac rx msg*/
//            queue_manager_push_back( QUEUE_RX_FREE_MSG, (queue_item_t *) rxmsg );
            
#ifdef MAC_CFG_SECURITY_ENABLED
          }
          else /* The Data Packet Failed Security Processing */
          {
            /* Extract SRC and DST addresses from the message */
            mac_frame_parse_addresses(rxmsg,&rxmsg->pd_rxp->psdu[0], &dst_addr, &src_addr);
            
            /* Report Errror to NHL */
            send_mlme_comm_status_indication(&src_addr, &dst_addr,
                                             rxmsg->security_status,
                                             &rxmsg->sec_param);
          }
#endif          
        }
        break;
        
      case MAC_FRAME_TYPE_ACK:
        switch(rxmsg->recived_frame_type)
        {
        case FAN_ACK:
#if APP_LBR_ROUTER          
          send_mcps_ack_indication(rxmsg);
#endif          
          break;
        default : 
          app_bm_free((uint8_t*)rxmsg->pd_rxp);
          break;
        }
        rxmsg->pd_rxp = NULL;
        break;
      case MAC_FRAME_TYPE_MAC_COMMAND:
        /* now process the MAC command Frame*/
        /*Uemsh : 11-01-2018 this thing goes to modified after sepration of mac and fan mac*/
#ifndef WISUN_FAN_MAC   
        process_mac_command(rxmsg);
#endif
        break;
        
      default:
        break;
        
      }/* end of switch */        
    }//end else PromiscuousMode
    /* this message has been processed so release it back*/
    mac_free_rcv_buffer( rxmsg );
//#if (PRINT_DEBUG_LEVEL==ERROR_DEBUG)
//    stack_print_debug ("MAC rcv buffer freed\n");
//#endif  
  }//end rxmsg // end of While loop
  
  if (rxmsg == NULL_POINTER)
  {
    event_clear(CMD_OR_DATA_FRAME_RX_EVENT);
    /* nothing on the rcv queue to process so exit */
    return 0;
  }
  return 1;
}

/*****************Arjun: func for subscribed frame*****************************/

void send_subscribed_packet(uint16_t length, uint8_t* buf)
{
  if(sendSubscribedPacket == 1)
  {
    hif_send_msg_up(buf,length,5,1); // 1 for PROTOCOL_ID_FOR_APP
  }
}

/*******************************************************************************/

/*
** ============================================================================
** Private Function Definitions
** ============================================================================
*/
#ifndef WISUN_FAN_MAC 
/*processes the MAC command frame stored in rxmsg.This function is called from the main loop*/
static void process_mac_command(
				mac_rx_t *rxmsg /* received message to process */
                                )
{
    uchar *cmd_params = NULL;          /* pointer to MAC command parameters */
    uchar *rxb = NULL;
    sm_event_t event;
    assocsm_param_t assoc_param = {0};
    mac_address_t *src_addr = NULL, *dst_addr = NULL;
#ifdef MAC_CFG_SECURITY_ENABLED
    uint8_t sec_status = 0;
#endif

    /* point at the parameters */
    rxb = rxmsg->pd_rxp->psdu;	    
    
    /* Following code expects rxb to point to a byte before the first byte of 
    MAC header. psdu points to first byte of MAC frame, so the decrementation 
    by one byte*/
    rxb -= 0x01;
    
    cmd_params = &rxmsg->payload[rxmsg->payloadIEFieldLen + 1];

    /* Extract SRC and DST addresses from the message */
    src_addr = &rxmsg->src;
    dst_addr = &rxmsg->dst;
#ifdef MAC_CFG_SECURITY_ENABLED
    sec_status = rxmsg->security_status;
#endif
    switch (rxmsg->payload[rxmsg->payloadIEFieldLen])
    {
#if(CFG_MLME_ASSOCIATE_IND_RESP == 1)
    case ASSOCIATION_REQUEST:
        {
#ifdef ENABLE_DEBUG_EVENTS        	
        Indicate_Debug_Event( ASSOCIATION_REQUEST_RX ); 
#endif
#ifdef MAC_CFG_SECURITY_ENABLED
            if (sec_status == SUCCESS)
            {
#endif
              
#if(CFG_MLME_ASSOCIATE_IND_RESP == 1)              

		/* incoming association request causes ASSOCIATE.indication only if association is permitted */
                if (mac_pib.AssociationPermit != 0)
                {
                    send_mlme_associate_indication(rxb,cmd_params, &rxmsg->sec_param);
                }
#endif //#if(CFG_MLME_ASSOCIATE_IND_RESP == 1)

#ifdef MAC_CFG_SECURITY_ENABLED
            }
            else /* The Packet Failed Security Processing */
            {
                /* Report Error to NHL */
                send_mlme_comm_status_indication( src_addr, dst_addr,
                                                    sec_status,
                                                    &rxmsg->sec_param);
            }
#endif
            break;
        }
#endif	/*(CFG_MLME_ASSOCIATE_IND_RESP == 1)*/

#if(CFG_MLME_ASSOCIATE_REQ_CONF == 1)
    case ASSOCIATION_RESPONSE:
#ifdef ENABLE_DEBUG_EVENTS    	
		Indicate_Debug_Event( ASSOCIATION_RESPONSE_RX );
#endif
        /* process parameters */
        /*TBD Safe enough not to check address mode first? */
        //assoc_param.coord_ieee_address = src_addr->address.ieee_address;
        //assoc_param.pan_id = rxmsg->dst.pan_id;
        //assoc_param.short_address = cmd_params[0] | (cmd_params[1] << 8);
        //assoc_param.status = cmd_params[2];

        /* indicate association response to ASSOC-SM */
        event.trigger = (sm_trigger_t) ASSOCSM_TRIGGER_RESP_RECEIVED;
        event.param.vector = rxmsg;
        SM_DISPATCH( (sm_t *) &assocsm, &event );
        break;
#endif	/*(CFG_MLME_ASSOCIATE_REQ_CONF == 1)*/

#if(CFG_MLME_DISASSOCIATE_IND == 1)
    case DISASSOCIATION_NOTIFICATION:
        {   
			/*received disassociation cmd from the PAN coord so the device goes to disassociate state*/   
            if( (((rxmsg->src.address_mode == 2) && (rxmsg->src.address.short_address == mac_pib.CoordShortAddress)) )
                || ((rxmsg->src.address_mode == 3)
                && (ieeeaddr_cmp(rxmsg->src.address.ieee_address, mac_pib.CoordExtendedAddress) == 0) ))
            {
            	#ifdef MAC_CFG_SECURITY_ENABLED
	            if (sec_status == SUCCESS)
	            {
				#endif
#if(CFG_MLME_SYNC_REQ == 1)
	                /* stop tracking beacons */
	                event.trigger = (sm_trigger_t) SYNCSM_TRIGGER_CANCEL;
	                event.param.scalar = 0;
	                SM_DISPATCH( (sm_t *) &syncsm, &event );
#endif
                     
#if(CFG_MLME_DISASSOCIATE_IND == 1)
	                /* send event to ASSOC-SM */
	                event.trigger = (sm_trigger_t) ASSOCSM_TRIGGER_DISASSOCIATE;
	                //event.param.scalar = 0;
	                SM_DISPATCH( (sm_t *) &assocsm, &event );

	                /* send indication primitive */
	                send_mlme_disassociate_indication( rxmsg,rxb, cmd_params, &rxmsg->sec_param );
#endif                     

				#ifdef MAC_CFG_SECURITY_ENABLED
        		}
        		else /* The Packet Failed Security Processing */
	            {
	                /* Report Errror to NHL */
	                send_mlme_comm_status_indication( src_addr, dst_addr,
	                                                   sec_status,
	                                                   &rxmsg->sec_param
												    );
	            }
				#endif              	
            } 
#if(CFG_MLME_DISASSOCIATE_IND == 1)            
            else
            {				
                /* PAN Coord received disassociation cmd from its child,it just send disassociation indication to upper layer*/
                /*What to do: if the dissociation cmd is not received from immediate Coord???*/
            	send_mlme_disassociate_indication( rxmsg,rxb, cmd_params, &rxmsg->sec_param );
            }  
#endif            
            break;
        }
#endif	/*(CFG_MLME_DISASSOCIATE_IND == 1)*/

#if(CFG_MLME_ORPHAN_IND_RESP == 1)
    case ORPHAN_NOTIFICATION:
        {
#ifdef ENABLE_DEBUG_EVENTS        	
			Indicate_Debug_Event( ORPHAN_NOTIFICATION_RX );
#endif
#ifdef MAC_CFG_SECURITY_ENABLED
            if (sec_status == SUCCESS)
            {
#endif
                /* incoming orphan notification causes orphan.indication to NWK */
                send_mlme_orphan_indication(rxb, &rxmsg->sec_param);
#ifdef MAC_CFG_SECURITY_ENABLED
            }
            else /* The Packet Failed Security Processing */
            {
                /* Report Errror to NHL */
                send_mlme_comm_status_indication( src_addr, dst_addr,
                                                    sec_status,
                                                    &rxmsg->sec_param);
            }
#endif
            break;
        }
#endif	/*(CFG_MLME_ORPHAN_IND_RESP == 1)*/

    case PAN_ID_CONFLICT_NOTIFICATION:
        {
#ifdef ENABLE_DEBUG_EVENTS        		
			Indicate_Debug_Event( PAN_ID_CONFLICT_NOTIFICATION_RX );
#endif
#ifdef MAC_CFG_SECURITY_ENABLED
            if (sec_status == SUCCESS)
            {
#endif
#if(CFG_MLME_SYNC_LOSS_IND == 1)
#if(CFG_MAC_STARTSM_ENABLED == 1)                
                if ( startsm_get_flags( startsm_p ) & STARTSM_FLAG_COORD_MASK ) 
                {
                    /* Report a Sync Loss to the NHL */
                     send_mlme_sync_loss_indication( MAC_PAN_ID_CONFLICT,
                         &rxmsg->sec_param );
                }
#endif //#if (CFG_MAC_STARTSM_ENABLED == 1)                  
#endif	/*(CFG_MLME_SYNC_LOSS_IND == 1)*/
#ifdef MAC_CFG_SECURITY_ENABLED
            }
            else /* The Packet Failed Security Processing */
            {
                /* Report Errror to NHL */
                send_mlme_comm_status_indication( src_addr, dst_addr,
                                                    sec_status,
                                                    &rxmsg->sec_param);
            }
#endif
            break;
        }

    case BEACON_REQUEST:
        {
        	
#ifdef ENABLE_DEBUG_EVENTS        	
			Indicate_Debug_Event(BEACON_REQUEST_RX);
#endif  
#if(CFG_MAC_BEACON_ENABLED == 1)                          
            process_beacon_request( rxmsg ); 
#endif            
            break;        
        }
/*Uemsh : 11-01-2018 this thing goes to modified after sepration of mac and fan mac*/
//#ifdef WISUN_FAN_MAC
////     case FAN_PAN_ADV_SOLCIT:
////    case FAN_PAN_CONF_SOLCIT:
////       process_async_frame( rxmsg ); 
//       break;
//#endif        

    case COORDINATOR_REALIGNMENT:
#ifdef ENABLE_DEBUG_EVENTS
		Indicate_Debug_Event(COORDINATOR_REALIGNMENT_RX);
#endif
#ifdef MAC_CFG_SECURITY_ENABLED
        if (sec_status == SUCCESS)
        {
#endif
            /* check if unicast packet */
            if( dst_addr->address_mode == MAC_IEEE_ADDRESS )
            {
                /*
                 * directed to us as an orphaned device
                 */
#if(CFG_MAC_SCANSM_ENABLED == 1)               
                if( scansm_get_state( &scansm ) == SCANSM_STATE_ORPHAN )
                {
					trxsm_channel_t c;
                    /* process parameters */
                    assoc_param.coord_ieee_address = src_addr->address.ieee_address;
                    assoc_param.coord_short_address = cmd_params[2] | (((uint16_t)(cmd_params[3])) << 8); // (cmd_params[3] << 8);
                    assoc_param.pan_id =  cmd_params[0] | (((uint16_t)(cmd_params[1])) << 8); //(cmd_params[1] << 8);
                    assoc_param.short_address = cmd_params[5] | (((uint16_t)(cmd_params[6])) << 8); //(cmd_params[6] << 8);
                    assoc_param.status = MAC_SUCCESS;

					/*Sarika: TBD Is it correct to ignore channel and page here? */
					c.channel = cmd_params[4];
	                event.trigger = (sm_trigger_t) TRXSM_TRIGGER_CHANNEL;
	                event.param.vector = &c;
	                SM_DISPATCH( (sm_t *) &trxsm, &event );

                    /* send event to ASSOC-SM */
                    event.trigger = (sm_trigger_t) ASSOCSM_TRIGGER_CRA_RECEIVED;
                    event.param.vector = &assoc_param;
                    SM_DISPATCH( (sm_t *) &assocsm, &event );

                    /* send event to SCAN-SM */
                    event.trigger = (sm_trigger_t) SCANSM_TRIGGER_CRA_RECEIVED;
                    event.param.scalar = 0;
                    SM_DISPATCH( (sm_t *) &scansm, &event );
                    
                }
#endif //#if(CFG_MAC_SCANSM_ENABLED == 1)                 
            }
            else
            {
                /*
                 * broadcast packet
                 */
                 
                 if( ( src_addr->address.short_address == mac_pib.CoordShortAddress ) || (ieeeaddr_cmp(src_addr->address.ieee_address, mac_pib.CoordExtendedAddress) == 0))
                 {

                        trxsm_channel_t c  = {0};

	                /* new PAN ID */
	                //mac_pib.PANId = cmd_params[0] | (cmd_params[1] << 8);

			mac_pib.PANId = get_ushort((uint8_t*)cmd_params);
			
	                /* new channel */
	                c.page = cmd_params[7]; /*TBD Get channel page from packet */
	                c.channel = cmd_params[4];
	                event.trigger = (sm_trigger_t) TRXSM_TRIGGER_CHANNEL;
	                event.param.vector = &c;
	                SM_DISPATCH( (sm_t *) &trxsm, &event );
#if(CFG_MLME_SYNC_LOSS_IND == 1)                
	                /* inform next higher layer */
	                send_mlme_sync_loss_indication( MAC_REALIGNMENT, &rxmsg->sec_param );
#endif	/*(CFG_MLME_SYNC_LOSS_IND == 1)*/
                 }
                
            }

#ifdef MAC_CFG_SECURITY_ENABLED
        }
        else /* The Packet Failed Security Processing */
        {
            /* Report Errror to NHL */
            send_mlme_comm_status_indication( src_addr, dst_addr,
                                              sec_status,
                                              &rxmsg->sec_param);
        }
#endif
        break;


#ifdef MAC_CFG_GTS_ENABLED
    case GTS_REQUEST:
        {
#ifdef MAC_CFG_SECURITY_ENABLED
            if (sec_status == SUCCESS)
            {
#endif
                ushort src_address = 0;
                ushort src_pan_id = 0;

                rxb += 2;           /* step over length and cf0 */
                /* get the short address */
                if ((rxb[0] & MAC_SRC_ADDRESS_MASK) == MAC_SHORT_SRC_ADDRESS)
                {
                    switch ((rxb[0] & MAC_DST_ADDRESS_MASK))
                    {
                    case MAC_NO_DST_ADDRESS:
                        rxb += 2;   /* control field 0, control field 1 */
                        break;

                    case MAC_SHORT_DST_ADDRESS:
                        rxb += 6;   /* control field 1, pan id and short address */
                        break;

                    case MAC_IEEE_DST_ADDRESS:
                        rxb += 12;  /* control field 1, pan id and IEEE address */
                        break;
                    }
                    src_pan_id = (((uint16_t)(rxb[1])) << 8) | rxb[0]; //(((uint16_t)(rxb[1])) << 8); //
                    src_address = (((uint16_t)(rxb[3])) << 8) | rxb[2];
                    /*
                     * allocate the GTS - rxb points at the mac header, cmd_params at the GTS stuff
                     * and gts_info a place to store the GTS info pointer
                     */
                    if (mac_gts_allocate(src_address, cmd_params) == MAC_SUCCESS)
                    {
                        send_mlme_gts_indication( src_pan_id,
                                                 (address_t *) & src_address,
                                                 MAC_SHORT_SRC_ADDRESS,
                                                 src_address, *cmd_params, rxmsg->security_level,
                                                 rxmsg->key_id_mode, rxmsg->key_identifier);
                    }
                }
                else                /* Not short source address */
                {
                }
#ifdef MAC_CFG_SECURITY_ENABLED
            }
            else /* The Packet Failed Security Processing */
            {
                /* Report Errror to NHL */
                send_mlme_comm_status_indication(
                                                    src_addr.pan_id,
                                                    src_addr.address_mode,
                                                    &src_addr.address,
                                                    dst_addr.address_mode,
                                                    &dst_addr.address,
                                                    sec_status,
                                                    rxmsg->security_level,
                                                    rxmsg->key_id_mode,
                                                    rxmsg->key_identifier
                                                                              );
            }
#endif
            break;
        }
#endif

    default:
        break;
    }
}  /* process mac command */

#endif
/*******************************************************************************/

uchar mac_find_indirect_transmission(
                                      mac_address_t *dst, 
                                      uchar check_pan_id	 
                                     )
{
    mac_tx_t *txd = NULL_POINTER;

    /* scan the queue, looking for the item */
    while( (txd = (mac_tx_t *) queue_manager_scan_next( QUEUE_INDIRECT_TX, (queue_item_t *) txd) )
           != NULL_POINTER )
    {
        /* check the address mode */
        if( dst->address_mode == txd->dst.address_mode )
        {
            /* check the address */
            switch( dst->address_mode )
            {
            case MAC_SHORT_ADDRESS:
                if( dst->address.short_address == txd->dst.address.short_address )
                {
                    if ( check_pan_id )
					{
						if( dst->pan_id == txd->dst.pan_id )
						{
							return MAC_SUCCESS;
						}						
					}
					else
					{
						return MAC_SUCCESS;
					}					
                }
                break;
            case MAC_IEEE_ADDRESS:
                if( ieeeaddr_cmp( dst->address.ieee_address,
                                  txd->dst.address.ieee_address ) == 0 )
                {
					if ( check_pan_id )
					{
						if( dst->pan_id == txd->dst.pan_id )
						{
							return MAC_SUCCESS;
						}						
					}
					else
					{
						return MAC_SUCCESS;
					}
                }
                break;
            default:
                break;
            }
        }
    }

    return MAC_INVALID_HANDLE;
}

/*******************************************************************************/

/*clear the security Fields ready for security processing*/
#ifdef MAC_CFG_SECURITY_ENABLED
static void clear_security_fields(
                           	mac_rx_t *mac_rxp /* mac receive struct in which to clear security fields */
                           )
{
    mac_rxp->sec_param.security_level = 0;
    mac_rxp->security_data = NULL_POINTER;
    mac_rxp->sec_param.key_id_mode = 0;
    mac_rxp->frame_counter = 0;
    mac_rxp->security_status = MAC_SUCCESS;
    memset(mac_rxp->sec_param.key_identifier,0,KEY_IDENTIFIER_MAX_LENGTH);
}
#endif


#ifdef MAC_CFG_SECURITY_ENABLED
static void mac_trigger_incoming_sec_processing(  mac_rx_t *mrp )
{
        clear_security_fields( mrp );

        /* Put on the Unprocessed Security Queue */
        if( mrp->frame_type == MAC_FRAME_TYPE_BEACON )
        {
            queue_front_put( (queue_t *) &mac_security_data.rx_security_queue,
                              (queue_item_t *) mrp );
        }
        else
        {
            queue_item_put( (queue_t *) &mac_security_data.rx_security_queue,
                             (queue_item_t *) mrp );
        }
        
        set_process_activity(AF_SECURITY_IN_PROGRESS);
        
        event_set(RX_SEC_FRAME_EVENT);
		//signal_event_to_mac_task();
}

#endif

/*******************************************************************************/

void mac_free_rcv_buffer( mac_rx_t *mac_rxp )
{
#ifdef MAC_CFG_SECURITY_ENABLED
  //mac_rxp->pd_rxp = NULL; // Raka ..
  mac_rxp->security_data = NULL; 
#endif    
  
  if (mac_rxp->pd_rxp != NULL)
  {
//#if (PRINT_DEBUG_LEVEL==ERROR_DEBUG)
//    stack_print_debug ("1F %p\n", mac_rxp->pd_rxp);
//#endif     
    app_bm_free((uint8_t*)mac_rxp->pd_rxp);
  }
  
  if (mac_rxp->wisun_fan_ies != NULL)
  {
//#if (PRINT_DEBUG_LEVEL==ERROR_DEBUG)
//    stack_print_debug ("2F %p\n", mac_rxp->wisun_fan_ies);
//#endif  
    app_bm_free((uint8_t*)mac_rxp->wisun_fan_ies);
  }
  queue_manager_push_back( QUEUE_RX_FREE_MSG, (queue_item_t *) mac_rxp );
}

/*******************************************************************************/

static void to_symbols(uint32_t *time)
{
	/*uint32_t returned_time = hw_tmr_get_symbols( *time );
	memcpy((uint8_t*)time,(uint8_t*)&returned_time,4);*/
	*time = hw_tmr_get_symbols( *time );
}

/*******************************************************************************/

#ifdef ENHANCED_ACK_SUPPORT
static uint8_t trigger_enhanced_ack_transmission( mac_rx_t *mac_rxp )
{
    mac_tx_t *dtx = NULL;

    mac_status_t status = MAC_SUCCESSs;
    /* is this check required. thsi is already done in mac_frame_parse. See if you can have some other way. 
    Waste of energy in doing teh same ting twice*/
    if( /* check ack required flag */
    ( mac_rxp->pd_rxp->psdu[0] & MAC_ACK_REQUIRED ) &&
    /* check if not broadcast */	
    ( mac_rxp->dst.address_mode != MAC_SHORT_ADDRESS ||
    mac_rxp->dst.address.short_address != BROADCAST_SHORT_ADDRESS ) )
    {
      status =  mac_frame_build_enhanced_ack
                                          (
                                          &dtx,
                                          mac_rxp
                                          );
      
      if( mac_find_indirect_transmission( &(mac_rxp->src), 0 ) == MAC_SUCCESS )
      {
        mac_rxp->frame_pending_out = 1;
        /*  set the frame pending bit in the ack */
        dtx->data->psdu[0] |= MAC_FRAME_PENDING;
      }

      if( ( status == MAC_SUCCESS ) && ( dtx != NULL_POINTER ) )
      {
        /* and now transmit */
        status = mac_queue_direct_transmission( dtx );
      }
    }
    return 1;
}
#endif

/*******************************************************************************/

#ifdef ENHANCED_ACK_SUPPORT
static void process_enack( mac_rx_t *rxmsg )
{
    sm_event_t event;
    trxsm_ack_t ack_in = {0};
    //ack_in.dsn = rxmsg->pd_rxp->psdu[ ACK_FIELD_DSN ];
    //ack_in.fp = rxmsg->pd_rxp->psdu[ ACK_FIELD_CF0 ] & MAC_FRAME_PENDING ? 1 : 0;
    ack_in.dsn = rxmsg->pd_rxp->psdu[ 2 ];
    ack_in.fp = rxmsg->pd_rxp->psdu[ 0 ] & MAC_FRAME_PENDING ? 1 : 0;

    event.trigger = (sm_trigger_t) ENACKSM_TRIGGER_ACK_RECEIVED;
    event.param.vector = &ack_in;
    SM_DISPATCH((sm_t *) enacksm_p, &event);
}
#endif
