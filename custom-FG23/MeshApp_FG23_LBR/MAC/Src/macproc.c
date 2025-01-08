/** \file macproc.c
 *******************************************************************************
 ** \brief Provides different functions needed by the MAC to process the 
 **        information provided by the NHL
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
#include "mac.h"
#include "timer_service.h"
#include "mac_queue_manager.h"
#include "phy.h"
#include "mac_config.h"

#include "mac_defs_sec.h"
#include "ie_element_info.h"
#include "mac_defs.h"
#include "sm.h"

#if(CFG_MAC_SFTSM_ENABLED == 1)
#include "sftsm.h"
#endif

#include "fan_mac_interface.h"
#if(CFG_MAC_MPMSM_ENABLED == 1)
#include "mpmsm.h"
#endif

#if(CFG_MAC_PENDADDR_ENABLED == 1)
#include "pandesc.h"
#endif

#if(CFG_MAC_SFTSM_ENABLED == 1)  
#include "startsm.h"
#endif
#include "fan_mac_ie.h"
#include "ccasm.h"
#include "trxsm.h"
#include "mac_frame_parse.h"
#include "mac_frame_build.h"
#include "mac_mem.h"
#include "mac_pib.h"
#include "macutils.h"

#if(CFG_MAC_PENDADDR_ENABLED == 1) 
#include "pendaddr.h"
#endif

#include "mac_uplink.h"

#if(CFG_MAC_PTSM_ENABLED == 1)  
#include "ptsm.h"
#endif

//#include "enackwaitsm.h"
#include "event_manager.h"
#include "fan_mac_security.h"

#if ( ( MAC_CFG_LE_RIT_CAPABILITY == 1 ) || ( MAC_CFG_LE_CSL_CAPABILITY == 1 ) )
#include "mac_le.h"
#endif

#ifdef MAC_CFG_SECURITY_ENABLED
#include "mac_security.h"
#endif

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

/* None */

/*
** =============================================================================
** Public Variables Definitions
** =============================================================================
**/

/* None */

/*
** ============================================================================
** External Variable Declarations
** ============================================================================
*/

#if (DEBUG_FLAG == 1)
	extern void log_item(uint8_t item);
#endif
        extern mac_pib_t mac_pib;
        extern uchar heap[];
        extern uint32_t processor_active;

#ifdef MAC_CFG_SECURITY_ENABLED
        extern mac_security_data_t mac_security_data;
#endif
        
#ifdef WISUN_FAN_MAC         
extern fan_mac_security mac_key_list;
extern uint32_t update_ufsi_with_procces_time(mac_tx_t *out_packt);
extern uint32_t update_bfio_with_procces_time(mac_tx_t *out_packt);
extern uint8_t get_node_type( void );
extern void get_eapol_parent(uint8_t *eapol_parent);
#if(FAN_EDFE_FEATURE_ENABLED == 1)
extern edfe_info_t edfe_information;
#endif
#endif
/* state machines */
        extern trxsm_t trxsm;
#if(CFG_MAC_PTSM_ENABLED == 1)         
        extern ptsm_t *ptsm_p;
        extern ptsm_t *enacksm_p;
#endif  
#if(CFG_MAC_SFTSM_ENABLED == 1)          
        extern startsm_t *startsm_p;
#endif
        
#ifdef MAC_CFG_SECURITY_ENABLED
	extern uchar load_auto_request_sec_params( security_params_t* p_sec_params );
#endif

        extern void * app_bm_alloc(
            uint16_t length//base_t length       
            );
            
            
        extern void app_bm_free(
            uint8_t *pMem      
            );
    
extern sm_result_t trxsm_cap_tx_csma(trxsm_t *s, const sm_event_t *e);
extern sm_result_t trxsm_cap_tx_send(trxsm_t *s, const sm_event_t *e);
extern void trxsm_transit( sm_state_t state );

/*
** ============================================================================
** Private Function Prototypes
** ============================================================================
*/

#ifdef MAC_CFG_GTS_ENABLED
        static uchar mac_check_available_gts_length(void);
#endif

        static void mac_transmit_indirect_message(mac_tx_t *tx_table);
        static void mac_process_data_request_command(uchar *rxb);

#if ( ( MAC_CFG_LE_RIT_CAPABILITY == 1 ) || ( MAC_CFG_LE_CSL_CAPABILITY == 1 ) )
        static void mac_process_rit_data_request_command( mac_rx_t* );
        static mac_status_t mac_get_matching_indirect_data_msg( mac_tx_t** p_ind_msg, mac_address_t* p_addr_to_match, uchar match_pan_id );
#endif

#if DISPLAY_DTX_PACKET_REQUIRED
        static void display_dtx_packet(mac_tx_t *dtx);
#endif

/*
** ============================================================================
** Private Function Prototypes
** ============================================================================
*/

#ifdef MAC_CFG_SECURITY_ENABLED
	static void mac_return_unsecured_message(mac_rx_t *rxmsg);
#endif

/*
** ============================================================================
** Public Function Prototypes
** ============================================================================
*/        
    
#ifdef MAC_CFG_SECURITY_ENABLED
	void mac_process_secured_messages(void);
#endif

        uchar mac_create_beacon_request( uint8_t br_type );
        //ulong mac_get_current_symbol_count(void);
        //void set_process_activity(ushort activity_flag);
        //void process_foreground_timers(void);
        
        
/*
** ============================================================================
** Public Function Definitions
** ============================================================================
*/
#ifdef ENHANCED_ACK_SUPPORT
mac_status_t mac_wait_for_enack(  mac_tx_t *txp )
{
        sm_event_t event = { (sm_trigger_t) ENACKSM_TRIGGER_PACKET, { 0 } };


        txp->enack_wait_timer = hw_tmr_get_time(NULL);
        txp->enack_wait_timer += mac_pib.EnhAckWaitDuration;//us


        queue_manager_push_back(QUEUE_ENACKWAIT, (queue_item_t *) txp);

        SM_DISPATCH( (sm_t *) enacksm_p, &event );

        //set_process_activity(AF_IND_MSG_PENDING);
        return MAC_SUCCESS;
}
#endif
                                       



mac_status_t mac_add_indirect_transmission(
						mac_address_t *src_address, 
						mac_address_t *dst_address,
						mac_tx_t *txp                       
                                           )
{
#if ( ( MAC_CFG_LE_RIT_CAPABILITY == 1 ) || ( MAC_CFG_LE_CSL_CAPABILITY == 1 ) )   
	sm_event_t e = { (sm_trigger_t) LE_RIT_TRIGGER_DATA_QUEUED, { 0 }};
#endif

#if(CFG_MAC_PTSM_ENABLED == 1)          
    sm_event_t event = { (sm_trigger_t) PTSM_TRIGGER_PACKET, { 0 } };
#endif
    
    txp->src = *src_address;    /* source address               */
    txp->dst = *dst_address;    /* destination address          */
    
    if( txp->dst.address_mode == ADDR_MODE_EXTENDED )
    {
    	memcpy(txp->dest_long_addr,dst_address->address.ieee_address,8);
    	txp->dst.address.ieee_address = txp->dest_long_addr;
    }
    
    //txp->msdu_handle = msdu_handle; /* transmit handle              */

    /* set the persistence time to the current value of PIB */
    txp->persistence_timer = mac_pib.TransactionPersistenceTime;
    
#if ( ( MAC_CFG_LE_RIT_CAPABILITY == 1 ) || ( MAC_CFG_LE_CSL_CAPABILITY == 1 ) )
		/*During this period (macRITPeriod equals to zero), all transactions shall be 
		handled as those of normal non beacon-enabled PAN (macRxOnWhenIdle: False). 
		RIT is ON or OFF, if LE is enabled, put it into indirect queue to be pulled 
		by dest nodes */
		if( ( mac_pib.LEenabled ) &&  ( low_energy_get_state(&low_energy) != LE_STATE_INIT ) )
		{
			/*mac is running in RIT mode*/
			txp->persistence_timer = mac_pib.RITTxWaitDuration;

			SM_DISPATCH((sm_t *)&low_energy, &e);
		}		
#endif

    /* flag indirect transmission */
    txp->tx_options |= INDIRECT_TRANSMISSION;

#ifdef MAC_CFG_SECURITY_ENABLED
    if ((txp->data->psdu[0] & SECURITY_ENABLED_TRANSMISSION)
        && (txp->security_data->security_level != MAC_SECURITY_NONE)
        && (mac_pib.mac_security_enabled == TRUE))
    {
        txp->security_data->return_queue = (queue_t *)queue_manager_get_list(QUEUE_INDIRECT_TX);
        queue_item_put((queue_t *) & mac_security_data.hallin_tx_queue,
			(queue_item_t *) txp->security_data);
        event_set( SECURE_EVENT );
		//signal_event_to_mac_task();
    }
    else
#endif
    {
        queue_manager_push_back(QUEUE_INDIRECT_TX, (queue_item_t *) txp);
#if( (CFG_MAC_PENDADDR_ENABLED == 1) || (CFG_MAC_PTSM_ENABLED == 1)  )    
        /* update Pending Address List */
        if( pendaddr_add( txp ) != PENDADDR_SUCCESS )
        {
            /*TBD What to do here? */
            //debug(("pendaddr err\r\n"));
        }
#if  (CFG_MAC_BEACON_ENABLED == 1)
        /* update beacon */
        if( mac_beacon_update( MAC_FRAME_BCN_SUB_TYPE_RB ) != MAC_SUCCESS )
        {
            /*TBD What to do here? */
            //debug(("bcn update err\r\n"));
        }
#endif
        /* notify PT-SM */
        
        SM_DISPATCH( (sm_t *) ptsm_p, &event );
#endif
        set_process_activity(AF_IND_MSG_PENDING);
    }

    return MAC_SUCCESS;
} 

/*****************************************************************************/

mac_status_t mac_queue_beacon_transmission(
					   mac_tx_t *txd,
					   uchar sub_type
                                          )
{
	uchar q_id = 0;

	if( MAC_FRAME_BCN_SUB_TYPE_RB == sub_type )
	{
		q_id = QUEUE_BCN_TX_CURR;
	}
	else if ( MAC_FRAME_BCN_SUB_TYPE_EB == sub_type )
	{
		q_id = QUEUE_EB_TX_CURR;
	}
	else
	{
		q_id = QUEUE_MPM_EB_TX_CURR;
	}

    /* ensure non-reentrant critical section */
//    cpu_save_imask_and_clear(flags);

#ifdef MAC_CFG_SECURITY_ENABLED
    if ( ( mac_pib.ConfigFlags & ( USE_2006_PRIMITIVES|USE_2011_PRIMITIVES ) )
        && (txd->data->psdu[0] & SECURITY_ENABLED_TRANSMISSION)
        && (txd->security_data->security_level != MAC_SECURITY_NONE)
        && (mac_pib.mac_security_enabled == TRUE))
    {
		txd->security_data->return_queue = (queue_t *) queue_manager_get_list( q_id );
		
        queue_item_put((queue_t *) & mac_security_data.hallin_tx_queue,
			(queue_item_t *) txd->security_data);
        event_set( SECURE_EVENT );
		//signal_event_to_mac_task();
    }
    else
#endif
    {
        queue_manager_push_back( q_id, (queue_item_t *) txd );
        /*TBD We should free previous beacons here */
    }

//    cpu_restore_imask(flags);

    return MAC_SUCCESS;
}

/*****************************************************************************/

#ifdef WISUN_FAN_MAC
void send_subscribed_packet(uint16_t length, uint8_t* buf);
uint32_t buff_for_subscription_not_available = 0;
uint32_t subscribe_tx_count = 0;

/* Debdeep modified this function for proper Tx subscription */
void subscribe_tx_frame (mac_tx_t *txd)
{
#if SUBSCRIBE_TX_PACKET
  uint8_t *buf_for_frame_subscription = NULL;
  uint16_t size = txd->data->psduLength;
  
  subscribe_tx_count++;
  
  buf_for_frame_subscription = (uint8_t *)app_bm_alloc (size + 5);
  if (buf_for_frame_subscription == NULL)
    buff_for_subscription_not_available++;
  buf_for_frame_subscription[0] = 0xEF;

  memcpy(&buf_for_frame_subscription[1], txd->data->psdu, size);
  if ((txd->sub_type == 0x04) || ((txd->sub_type == 0x05) && (size > 30)))
  {
    if (txd->dst.address_mode == 0)
    {
      buf_for_frame_subscription[14] = buf_for_frame_subscription[14] >> 3;
      buf_for_frame_subscription[14] = buf_for_frame_subscription[14] << 3;
    }
    else
    {
      buf_for_frame_subscription[20] = buf_for_frame_subscription[20] >> 3;
      buf_for_frame_subscription[20] = buf_for_frame_subscription[20] << 3;
    }
    size -= 8;  /*For MIC*/
  }
//  if (txd->sub_type == 0x02)
//  {
//    buf_for_frame_subscription[13] = buf_for_frame_subscription[13] >> 3;
//    buf_for_frame_subscription[13] = buf_for_frame_subscription[13] << 3;
//  }
  send_subscribed_packet(size+1, buf_for_frame_subscription);
  app_bm_free((uint8_t*)buf_for_frame_subscription);
#endif
}
#endif//#ifdef WISUN_FAN_MAC

mac_status_t mac_queue_direct_transmission (mac_tx_t *txd)
{ 
  /* ensure non-reentrant critical section */
  //  cpu_save_imask_and_clear(flags);
#ifdef WISUN_FAN_MAC
#if (UPDATE_UFSI_AFTER_CCA == 1)/*Umesh : 21-02-2018*//*for sepration of 802.15.04*/
  uint32_t ufsi_update = update_ufsi_with_procces_time(txd);
  memcpy(txd->p_ufsi,(uint8_t*)&ufsi_update,3);
  uint32_t ufio_update = update_bfio_with_procces_time(txd);
  memcpy(txd->p_bfsi,(uint8_t*)&ufio_update,3);
#endif
  subscribe_tx_frame (txd);  
#endif 
  
//  /* Debdeep :: Instead of posting event for security, we do security directly just before calling PD_data_request */
//#ifdef MAC_CFG_SECURITY_ENABLED
//  if ( ( mac_pib.ConfigFlags & ( USE_2006_PRIMITIVES|USE_2011_PRIMITIVES ) )
//      && (txd->data->psdu[0] & SECURITY_ENABLED_TRANSMISSION)
//        &&(txd->security_data->state == MAC_DTX_STATE_NOT_PROCESSED)
//          && (txd->security_data->security_level != MAC_SECURITY_NONE)
//            && (mac_pib.mac_security_enabled == TRUE)
//              && ((txd->data->psdu[0] & MAC_FRAME_TYPE_MASK) != MAC_FRAME_TYPE_BEACON)) /* Beacons have already been Secured */
//  {
//    txd->security_data->return_queue = (queue_t *) queue_manager_get_list(QUEUE_CAP_TX);
//    queue_item_put((queue_t *) & mac_security_data.hallin_tx_queue,
//                   (queue_item_t *) txd->security_data);
//    event_set( SECURE_EVENT );
//    signal_event_to_mac_task();
//  }
//  else
//#endif
//  {
  
  /*TBD Do we really need to send this event? (main loop should take care) */
  sm_event_t event = {(sm_trigger_t)TRXSM_TRIGGER_NEW_DIRECT_PACKET, {0}};
  queue_manager_push_back (QUEUE_CAP_TX, (queue_item_t *)txd);
  set_process_activity (AF_CAP_MSG_PENDING);
  
//#if (PRINT_DEBUG_LEVEL==ERROR_DEBUG)
//  stack_print_debug ("Posting PENDING_TX_EVENT for Unicast\n");
//  stack_print_debug ("ccasm idle = %d\n", ccasm_is_idle (trxsm.ccasm));                  
//#endif          
  
  //event_set (PENDING_TX_EVENT);
  if (trxsm_get_state (&trxsm) == TRXSM_STATE_IDLE)
  {    
    SM_DISPATCH ((sm_t *)&trxsm, &event);
  }        
//  }
//  cpu_restore_imask(flags);
  
  return MAC_SUCCESS;
}

/*****************************************************************************/

mac_status_t mac_queue_bcast_transmission (mac_tx_t *txd)
{
  /* ensure non-reentrant critical section */
  //    cpu_save_imask_and_clear(flags);
#ifdef WISUN_FAN_MAC
#if (UPDATE_UFSI_AFTER_CCA == 1)/*Umesh : 21-02-2018*//*for sepration of 802.15.04*/
  uint32_t ufsi_update = update_ufsi_with_procces_time(txd);
  memcpy(txd->p_ufsi,(uint8_t*)&ufsi_update,3);
  uint32_t ufio_update = update_bfio_with_procces_time(txd);
  memcpy(txd->p_bfsi,(uint8_t*)&ufio_update,3);
#endif  
  if((txd->sub_type == 04) || (txd->sub_type == 06))
    subscribe_tx_frame (txd);
#endif  
  
/* Debdeep :: Instead of posting event for security, we do security directly just before calling PD_data_request */
//#ifdef MAC_CFG_SECURITY_ENABLED
//  if ( ( mac_pib.ConfigFlags & ( USE_2006_PRIMITIVES|USE_2011_PRIMITIVES ) )
//      && (txd->data->psdu[0] & SECURITY_ENABLED_TRANSMISSION)
//        && (txd->security_data->security_level != MAC_SECURITY_NONE)
//          && (mac_pib.mac_security_enabled == TRUE)
//            && ((txd->data->psdu[0] & MAC_FRAME_TYPE_MASK) != MAC_FRAME_TYPE_BEACON)) /* Beacons have already been Secured */
//  {
//    txd->security_data->return_queue = (queue_t *) queue_manager_get_list(QUEUE_BCAST);
//    queue_item_put((queue_t *) & mac_security_data.hallin_tx_queue,
//                   (queue_item_t *) txd->security_data);
//    event_set( SECURE_EVENT );
//    signal_event_to_mac_task();
//  }
//  else
//#endif
//  {
  sm_event_t event = {(sm_trigger_t)TRXSM_TRIGGER_NEW_BCAST_PACKET, {0}};
  queue_manager_push_back (QUEUE_BCAST, (queue_item_t *)txd);
  set_process_activity (AF_CAP_MSG_PENDING);
  
//#if (PRINT_DEBUG_LEVEL==ERROR_DEBUG)
//  stack_print_debug ("Posting PENDING_TX_EVENT for Braodcast\n");
//  stack_print_debug ("ccasm idle = %d\n", ccasm_is_idle (trxsm.ccasm));                
//#endif  
  
  //event_set(PENDING_TX_EVENT);
  if (trxsm_get_state (&trxsm) == TRXSM_STATE_IDLE)
  {
    SM_DISPATCH ((sm_t *)&trxsm, &event);
  }
//  }
//  cpu_restore_imask(flags);
  
  return MAC_SUCCESS;
}

/*****************************************************************************/

mac_status_t mac_purge_direct_transmission(
						uchar type, 
						uchar msdu_handle 
					  )
{
    mac_tx_t *txd = NULL_POINTER;

    /* scan the queue, looking for the item */
    while ((txd =
            (mac_tx_t *) queue_manager_scan_next(QUEUE_CAP_TX,
						 (queue_item_t *) txd)) != NULL_POINTER)
    {
        if ((type == txd->type) && (msdu_handle == txd->msdu_handle))
        {
            queue_manager_remove(QUEUE_CAP_TX, (queue_item_t *) txd);
            mac_mem_free_tx(txd);
            return MAC_SUCCESS;
        }
    }
    return MAC_INVALID_HANDLE;
}

/*****************************************************************************/

#ifdef MAC_CFG_GTS_ENABLED
uchar mac_purge_gts_transmission(
                                    uchar msdu_handle 
                                )
{
    mac_tx_t *txd = NULL_POINTER;

    /* scan the queue, looking for the item */
    while ((txd =
            (mac_tx_t *) queue_manager_scan_next(QUEUE_GTS_TX,
						 (queue_item_t *) txd)) != NULL_POINTER)
    {
        if (msdu_handle == txd->msdu_handle)
        {
            queue_manager_remove(QUEUE_GTS_TX, (queue_item_t *) txd);
            mac_mem_free_tx(txd);
            return MAC_SUCCESS;
        }
    }
    /* FIXME: What about GTSs on encyrption queue? */
    return INVALID_HANDLE;
}
#endif

/*****************************************************************************/

mac_status_t mac_purge_indirect_transmission(
                                              uchar type,  
                                              uchar msdu_handle 
											)
{
    mac_tx_t *txd = NULL_POINTER;
#if (CFG_MAC_STARTSM_ENABLED == 1)  
    if( ( startsm_get_flags( startsm_p ) & STARTSM_FLAG_COORD_MASK ) != STARTSM_FLAG_PANCOORD )
    {
        return MAC_INVALID_HANDLE;
    }
#endif
    /* scan the queue, looking for the item */
    while ((txd =
            (mac_tx_t *) queue_manager_scan_next(QUEUE_INDIRECT_TX,
						 (queue_item_t *) txd)) != NULL_POINTER)
    {
        if ((type == txd->type) && (msdu_handle == txd->msdu_handle))
        {
#if( (CFG_MAC_PENDADDR_ENABLED == 1))           
            /* update Pending Address List */
            if( pendaddr_remove( txd ) != PENDADDR_SUCCESS )
            {
                /*TBD What to do here? */
                //debug(("pendaddr err\r\n"));
            }
#endif            
#if  (CFG_MAC_BEACON_ENABLED == 1) 
            /* update beacon */
            if( mac_beacon_update( MAC_FRAME_BCN_SUB_TYPE_RB ) != MAC_SUCCESS )
            {
                /*TBD What to do here? */
               // debug(("bcn update err\r\n"));
            }
#endif
            

            queue_manager_remove( QUEUE_INDIRECT_TX, (queue_item_t *) txd );
            mac_mem_free_tx( txd );
            return MAC_SUCCESS;
        }
    }
    return MAC_INVALID_HANDLE;
}

/*****************************************************************************/
#ifdef MAC_CFG_GTS_ENABLED
uchar mac_add_gts_transmission(mac_address_t *dst_address,  
                               mac_tx_t *dtx   
    )
{
    uchar status = MAC_SUCCESS;
    uchar i = 0;
    uchar start_slot = 0;
    uchar gts_slots_required = 0;
    uchar gts_transaction_time = 0;

    status = INVALID_GTS;
    start_slot = 0;

    dtx->gts_data = NULL_POINTER;
    dtx->missed_ack_count = 0;

    /* is this a valid GTS? */
    if (mac_data.coordinator /*& mac_data.coordinator_started*/)
    {
        /* we are a coordinator and have been started, so check we have a GTS assigned */
        for (i = 0; (i < MAX_GTS_DESCRIPTORS) && (status != MAC_SUCCESS); i++)
        {
            /* loop searching for a short address match */
            if ((mac_data.gts_params[i].gts_desc.
                 device_short_address[0] | (mac_data.gts_params[i].gts_desc.
                                            device_short_address[1] << 8)) ==
                dst_address->address.ShortAddress)
            {
                /* found a match */
                /* is it the right direction - check the mask ? */
                if (mac_data.gts_params[i].direction == GTS_DIR_TX_ONLY)
                {
                    continue;
                }
                if (mac_data.gts_params[i].gts_state != GTS_ALLOCATED)
                {
                    continue;
                }

                /* calculated the required length (in slots) - dtx->length is number of bytes */
                /* gts_transaction_time in backoff slots */
                gts_transaction_time = (1 + (dtx->length + 5) / 10) + 5 + 1;    /* be extra generous for the preamble and SOM */
                gts_slots_required =
                    1 + (gts_transaction_time / number_of_backoffs_in_slot());

                /* will it fit? */
                if ((((mac_data.gts_params[i].gts_desc.
                       length_and_start_slot) >> 4) & 0x0f) < gts_slots_required)
                {
                    /* no, it won't fit !! */
                    return FRAME_TOO_LONG;
                }
                /* everything checks out, so remember the starting slot */
                status = MAC_SUCCESS;

                dtx->gts_data = &mac_data.gts_params[i];
                /* FIXME: Is this valid? If not, save a pointer into beacon */
                /* start_slot = mac_data.beacon.gts_info_fields.gts_desc[i].length_and_start_slot & 0x0f; */
                start_slot = mac_data.gts_params[i].gts_desc.length_and_start_slot & 0x0f;
            }
        }
    }                           /* if(mac_data.coordinator_started) */
    else
    {
        /* not a started coordinator, check if we have a GTS available */

        /* do we have a GTS allocated */
        if (mac_data.gts_tx_start_slot == 0)
        {
            return INVALID_GTS;
        }

        if (mac_data.gts_tx_start_slot < 16)
        {
            /* calculated the required length (in slots) - dtx->length is number of bytes */
            /* gts_transaction_time in backoff slots */
            gts_transaction_time = (1 + (dtx->length + 5) / 10) + 5 + 1;    /* be extra generous for the preamble and SOM */
            gts_slots_required =
                1 + (gts_transaction_time / number_of_backoffs_in_slot());

            /* will it fit? */
            if (mac_data.gts_tx_length < gts_slots_required)
            {
                /* no, it won't fit !! */
                return FRAME_TOO_LONG;
            }
            status = MAC_SUCCESS;
            start_slot = mac_data.gts_tx_start_slot;
        }
    }

    if (status != MAC_SUCCESS)
    {
        return status;
    }

    dtx->type = TX_TYPE_GTS_DATA;

    set_process_activity(AF_GTS_ACTIVITY_PENDING);

    queue_manager_push_back(QUEUE_GTS_TX, (queue_item_t *) dtx);

    return status;
}
#endif

/*****************************************************************************/
uchar mac_create_beacon_request( uint8_t br_type)
{
    //uchar status;
    mac_tx_t *dr = NULL;

    mac_frame_build_beacon_request( &dr,br_type );

    if( dr )
    {
        return (mac_queue_direct_transmission( dr ));
    }
    else
    {
        return MAC_TRANSACTION_OVERFLOW;
    }
    //return status;
}

/*****************************************************************************/

#ifdef MAC_CFG_GTS_ENABLED
uchar mac_create_gts_request(
                             uchar *buf 
                            )
{
    mac_tx_t *dtx = NULL;
    uchar status = MAC_SUCCESS;
    mac_address_t src_address = {0};  /* source address */
    mac_address_t dst_address = {0};  /* destination address */
    uchar tx_options = 0;           /* transmit options */
    uchar payload[2] = {0};           /* packet payload */
    uchar gts_characteristics  = 0;
    uchar security_enable = 0;

#ifdef MAC_CFG_SECURITY_ENABLED
    /* 802.15.4-2006 Parameters */
    uchar security_level = 0;
    uchar key_id_mode = 0;
    uchar key_identifier[KEY_IDENTIFIER_MAX_LENGTH];
#endif

    gts_characteristics = (*buf++ & 0x3f);

    /* Check if we are using 802.15.4-2006 primtives */
    if ( mac_pib.ConfigFlags & ( USE_2006_PRIMITIVES|USE_2011_PRIMITIVES ) )
    {
#ifdef MAC_CFG_SECURITY_ENABLED
        security_level = (*buf & SEC_LEVEL_MASK);
        if (security_level != MAC_SECURITY_NONE)
        {
            key_id_mode = (*buf++ & KEY_ID_MASK) >> 4;

            if (key_id_mode == 1)
            {
                /* KEY_IDENTIFIER - 1 octet Key Index */
                key_identifier[0] = *buf++;
            }
            else if (key_id_mode == 2)
            {
                /* KEY_IDENTIFIER - 4 octet Key Source + 1 octet Key Index */
                memcpy(key_identifier, buf, 4);
                buf += 4;
                key_identifier[4] = *buf++;
            }
            else if (key_id_mode == 3)
            {
                /* KEY_IDENTIFIER - 8 octet Key Source + 1 octet Key Index */
                memcpy(key_identifier, buf, 8);
                buf += 8;
                key_identifier[8] = *buf++;
            }
        }

        /* Check if we are trying to secure a frame when security is not enabled */
        if ((security_level != MAC_SECURITY_NONE)
            && (mac_pib.mac_security_enabled == FALSE))
        {
            send_mlme_gts_confirm(mac_data.gts_rx_characteristics, UNSUPPORTED_SECURITY);
            return 1;
        }
#endif
    }
    else                        /* 2003 Primitives */
    {
        security_enable = *buf & 1;
    }

    payload[0] = GTS_REQUEST;   /* Command frame identifier */
    payload[1] = gts_characteristics;   /* GTS characteristics */

    tx_options = ACKNOWLEDGED_TRANSMISSION;

    src_address.address_mode = MAC_SHORT_ADDRESS;
    src_address.pan_id = mac_pib.PANId;
    src_address.address.ShortAddress = mac_pib.ShortAddress;

    dst_address.address_mode = MAC_NO_ADDRESS;

    /* Create the packet to send */
    status = mac_frame_build( &dtx, MAC_FRAME_TYPE_MAC_COMMAND,MAC_FRAME_BCN_SUB_TYPE_DONT_CARE,
                           &src_address, &dst_address, tx_options, fc_options, payload,
                           2, 1, key_id_mode, key_identifier, security_level);

    if (dtx != NULL_POINTER)
    {
        /* clear any current GTS transmit or receive fields */
        if (buf[0] & GTS_DIR_RX_ONLY)
        {
            mac_data.gts_rx_characteristics = buf[0];
            mac_data.gts_rx_start_slot = 0;
            mac_data.gts_rx_length = buf[0] & GTS_LENGTH_MASK;
            disable_interrupt();
            mac_data.gts_rx_timer = aGTSDescPersistenceTime;
            mac_data.gts_rx_request_pending = 1;
            enable_interrupt();
        }
        else
        {
            mac_data.gts_tx_characteristics = buf[0];
            mac_data.gts_tx_start_slot = 0;
            mac_data.gts_tx_length = buf[0] & GTS_LENGTH_MASK;
            mac_data.gts_tx_timer = aGTSDescPersistenceTime;
            mac_data.gts_tx_request_pending = 1;
        }
        /* and now transmit */
        set_process_activity(AF_GTS_ACTIVITY_PENDING);
        status = mac_queue_direct_transmission(dtx);
    }

    return status;

}
#endif

/*****************************************************************************/

uchar mac_create_panid_conflict_notification( void )
{
        mac_tx_t *dtx = NULL;
        mac_status_t status = MAC_SUCCESS;
        mac_address_t src = {0}, dst = {0};
    
#ifdef MAC_CFG_SECURITY_ENABLED    
        security_params_t sec_param = {0};
        sec_param.security_level = 0x08;
        load_auto_request_sec_params(&sec_param);

	status = mac_frame_build_panid_conflict_notification( &dtx, &src, &dst, (sec_param.security_level <8)?&sec_param:NULL_POINTER );
#else
	status = mac_frame_build_panid_conflict_notification( &dtx, &src, &dst, NULL_POINTER );
#endif

    if( dtx != NULL_POINTER )
    {
        /* transmit it */
        status = mac_queue_direct_transmission( dtx );
    }
#ifdef MAC_CFG_SECURITY_ENABLED
	else if( status == MAC_UNAVAILABLE_KEY )
	{
	    /* Failed to send it - Report Error */
	    send_mlme_comm_status_indication( &src,
	                                        &dst,
	                                        status,
	                                        (sec_param.security_level <8)?&sec_param:NULL_POINTER );
	}
#endif

    return status;
}

/*****************************************************************************/
#if	(CFG_ORPHAN_SCAN == 1)
uchar mac_create_orphan_notification( void )
{
    uchar status = MAC_SUCCESS;
    mac_tx_t *dtx = NULL;

    status = mac_frame_build_orphan_notification( &dtx,
#ifdef MAC_CFG_SECURITY_ENABLED
                                                  /*TBD This seems like a bug here */
                                                  &mac_security_data.coord_realign_sec_param
#else
                                                  NULL_POINTER
#endif
                                                  );

    if( dtx != NULL_POINTER )
    {
        /* and now transmit */
        status = mac_queue_direct_transmission( dtx );
    }
    return status;
}
#endif	/*(CFG_ORPHAN_SCAN == 1)*/

/*****************************************************************************/

#ifdef MAC_CFG_GTS_ENABLED
uchar mac_gts_allocate(ushort address, uchar *params)
{
        uchar gts = 0;
        gts_data_t *gd = NULL;             /* pointer to the first free gts parameter item */
        gts_descriptor_t *gts_info = NULL; /* pointer to the gts descriptor in gd above */
        uchar status = MAC_SUCCESS;
        uchar i = 0;

        uchar deallocated_start_slot = 0;
        uchar deallocated_length = 0;
        uchar available_length = 0;

        deallocated_start_slot = 0;
        deallocated_length = 0;

        /* set the GTS value */
        gts = params[0];

#define DEBUG_GTS 0

#if DEBUG_GTS
    print("\r\nmac_gts_allocate: ");
    hexdump(params, 8);
    print("gts = ");
    puthexbyte(gts);
#endif

    /* are we attempting to allocate ? */
    if ((gts & GTS_TYPE_MASK) == GTS_TYPE_ALLOCATE)
    {
#if DEBUG_GTS
        print(", ALLOC, ");
#endif
        /* yes, but are we allowed more? */
        if (mac_data.gts_count >= MAX_GTS_DESCRIPTORS)
        {
            /* no, so return error status */
#if DEBUG_GTS
            print("too many GTS's\r\n");
#endif
            status = MAC_INVALID_PARAMETER;
        }

        /* check the GTS length */
        available_length = mac_check_available_gts_length();
        if (available_length < (gts & GTS_LENGTH_MASK))
        {
            /* no, so return error status */
#if DEBUG_GTS
            print("gts length fails\r\n");
#endif
            status = MAC_INVALID_PARAMETER;
        }

        /* find the first free GTS parameter location */
        for (gd = mac_data.gts_params, i = 0; i < MAX_GTS_DESCRIPTORS; gd++, i++)
        {
            /* check for an entry with duplicate address and direction
             *(desc timer will be non-zero during deallocation
             */
            if ((gd->desc_timer != 0) || (gd->gts_state != GTS_UNUSED))
            {
                gts_info = &gd->gts_desc;
                /* test for same address */
                if ((gts_info->device_short_address[0] != LOW_BYTE(address))
                    || (gts_info->device_short_address[1] != HIGH_BYTE(address)))
                {
                    /* not same address */
                    continue;
                }

                /* check direction */
                if ((((gts & GTS_DIRECTION_MASK) == GTS_DIR_RX_ONLY)
                     && (gd->direction == 0))
                    || (((gts & GTS_DIRECTION_MASK) == GTS_DIR_TX_ONLY)
                        && (gd->direction == 1)))
                {
                    /* not same direction */
                    continue;
                }

                /* address and direction match - so just ignore this request */
                return MAC_INVALID_PARAMETER;
            }

            /* falls through pointing to a free GTS data structure */
            if (status == MAC_SUCCESS)
            {
                /* only set allocated IF this will be successful */
                gd->gts_state = GTS_ALLOCATED;
            }

            /* set the direction */
            if ((gts & GTS_DIRECTION_MASK) == GTS_DIR_RX_ONLY)
            {
                gd->direction = 1;
            }
            else
            {
                gd->direction = 0;
            }

            /* pointer to the GTS info */
            gts_info = &gd->gts_desc;

            gts_info->device_short_address[0] = LOW_BYTE(address);
            gts_info->device_short_address[1] = HIGH_BYTE(address);

            /* set the start slot IF successful, else set start slot of zero to show invalid */
            if (status == MAC_SUCCESS)
            {
                /* we update the beacon parameters ONLY if we can allocate the GTS */
                mac_data.gts_start_slot =
                    mac_data.gts_start_slot - (gts & GTS_LENGTH_MASK);

                /* update the final cap slot */
                mac_data.final_cap_slot = mac_data.gts_start_slot - 1;

                gts_info->length_and_start_slot =
                    ((gts & GTS_LENGTH_MASK) << 4) + mac_data.gts_start_slot;
            }
            else
            {
                /* start slot and length = 0 for deallocate / invalid */
                gts_info->length_and_start_slot = available_length << 4;
            }

            /**********************************************************************
			* finally set the expiration timer ( decremented once every beacon )
			* see section 7.5.7.6 Draft 18 802.15.4
			**********************************************************************/

            gd->expiration_timer = mac_get_gts_expiration_time();

            gd->desc_timer = aGTSDescPersistenceTime;
            mac_data.gts_count++;
            return status;
        }                       /* end of for() loop testing for free GTS */
    }
    else                        /* GTS type is deallocation */
    {
        /* assume we don't find it */
        status = INVALID_GTS;
#if DEBUG_GTS
        print(", gts DEALLOC: ");
#endif

#define DEBUG_GTS_DEALLOCATION 0
#if DEBUG_GTS_DEALLOCATION
        putshort(address);
        print("\r\n");
#endif
        /* deallocate requested, so find the device */
        for (gd = mac_data.gts_params, i = 0; i < MAX_GTS_DESCRIPTORS; gd++, i++)
        {
            /* skip unused entries */
            if (gd->gts_state != GTS_ALLOCATED)
            {
                continue;
            }
#if DEBUG_GTS_DEALLOCATION
            puthexbyte(gd->gts_desc.device_short_address[0]);
            puthexbyte(gd->gts_desc.device_short_address[1]);
#endif
            /* skip address match failures */
            if ((gd->gts_desc.device_short_address[0] != (LOW_BYTE(address)))
                || (gd->gts_desc.device_short_address[1] != (HIGH_BYTE(address))))
            {
                continue;
            }
#if DEBUG_GTS_DEALLOCATION
            print("<< matched address, ");
#endif
            /* skip direction failures */
            if (gd->direction != ((gts & GTS_DIRECTION_MASK) == GTS_DIR_RX_ONLY))
            {
                continue;
            }
#if DEBUG_GTS_DEALLOCATION
            print("matched direction, ");
#endif
            /*
             * this is the one - so ...
             * i) we may need to notify this deallocation in the beacon
             * ii) we may need to adjust the other GTS's to defragment the allocations
             */

            status = MAC_SUCCESS;
            deallocated_start_slot = gd->gts_desc.length_and_start_slot & 0x0f;
            deallocated_length = (gd->gts_desc.length_and_start_slot >> 4) & 0x0f;

            gd->gts_state = GTS_UNUSED;
            /* set the start slot field (lower nibble) to 0 to show deallocated */
            gd->gts_desc.length_and_start_slot &= 0xf0;
            gd->desc_timer = aGTSDescPersistenceTime;


            mac_data.gts_count--;
            mac_data.gts_update_in_progress = 1;
        }                       /* end of for loop() */

    }                           /* end of gts_type test */

    /* now defrag the GTS allocations */

    if (deallocated_length != 0)
    {
#if DEBUG_GTS_DEALLOCATION
        print("deallocate: ");
        puthexbyte(deallocated_length);
        print("\r\n");
#endif
        mac_data.gts_start_slot = 16;
        for (gd = mac_data.gts_params, i = 0; i < MAX_GTS_DESCRIPTORS; gd++, i++)
        {
#if DEBUG_GTS_DEALLOCATION
            hexdump((uchar *) gd, sizeof(gts_data_t));
#endif
            /* skip non allocated entries */
            if (gd->gts_state != GTS_ALLOCATED)
            {
                continue;
            }
#if DEBUG_GTS_DEALLOCATION
            print("\r\nAllocated: ");
            puthexbyte(gd->gts_desc.length_and_start_slot & 0x0f);
            print("\r\n");
#endif
            /* test if this start slot is earlier than the one we just removed */
            if ((gd->gts_desc.length_and_start_slot & 0x0f) < deallocated_start_slot)
            {
                /* it is, so move the start slot by the length of the deallocated slot */
                gd->gts_desc.length_and_start_slot += deallocated_length;
                /* and mark it for update in the beacon */
                gd->desc_timer = aGTSDescPersistenceTime;
            }
            if ((gd->gts_desc.length_and_start_slot & 0x0f) < mac_data.gts_start_slot)
            {
                mac_data.gts_start_slot = (gd->gts_desc.length_and_start_slot & 0x0f);
#if DEBUG_GTS_DEALLOCATION
                puthexbyte(mac_data.gts_start_slot);
#endif
            }
        }
        mac_data.final_cap_slot = mac_data.gts_start_slot - 1;
    }                           /* deallocated_length != 0 */

    return status;
}                               /* end of mac_gts_allocate() */
#endif

/*****************************************************************************/

void mac_cap_msg_complete ( 
				mac_tx_t *dm,  
                            	mac_status_t reason 
			   )
{
    dm->status = reason;

    if ((reason != MAC_SUCCESS) && (reason != MAC_TRANSACTION_EXPIRED)
        && (dm->tx_options & INDIRECT_TRANSMISSION))
    {
        /* it was an indirect message, so requeue it */
        queue_manager_push_front(QUEUE_INDIRECT_TX, (queue_item_t *) dm);
        set_process_activity(AF_IND_MSG_PENDING);
    }
    else
    {
        queue_manager_push_back(QUEUE_TX_DONE, (queue_item_t *) dm);
        set_process_activity(AF_TX_MSG_SENT_PENDING);
		event_set(FRAME_TX_DONE_EVENT);
		//signal_event_to_mac_task();
    }
}

/*****************************************************************************/
#ifdef MAC_CFG_GTS_ENABLED
void mac_gts_msg_complete(
                          mac_tx_t *dm, /* message that was dealt with */
                          mac_status_t reason /* status */
                         )
{
    dm->status = reason;

    queue_manager_push_back(QUEUE_TX_DONE, (queue_item_t *) dm);
    set_process_activity(AF_TX_MSG_SENT_PENDING);
	event_set(FRAME_TX_DONE_EVENT);
	//signal_event_to_mac_task();

}
#endif
/*****************************************************************************/

#ifdef MAC_CFG_SECURITY_ENABLED
/**
 ** \brief returns a message that has failed Authetication to its original msg queue
 */
static void mac_return_unsecured_message(
                                  mac_rx_t *rxmsg /* The message that failed Authentication */
                                 )
{
    if ((rxmsg->pd_rxp->psdu[0] & MAC_FRAME_TYPE_MASK) == MAC_FRAME_TYPE_BEACON)
    {
        queue_manager_push_back(QUEUE_BCN_RX, (queue_item_t *) rxmsg);
        event_set(BCN_RX_EVENT);
		set_process_activity(AF_BEACON_RECEIVED);
		//signal_event_to_mac_task();
    }
    else
    {
        if (((rxmsg->pd_rxp->psdu[0] & MAC_FRAME_TYPE_MASK) == MAC_FRAME_TYPE_MAC_COMMAND)
            && (rxmsg->payload[0] == DATA_REQUEST))
        {
            queue_manager_push_back(QUEUE_DATA_REQUEST, (queue_item_t *) rxmsg);
            event_set(DATA_REQ_RX_EVENT);
			//signal_event_to_mac_task();
        }
        else
        {
            queue_manager_push_back(QUEUE_RX_MSG, (queue_item_t *) rxmsg);
            event_set(CMD_OR_DATA_FRAME_RX_EVENT);
			//signal_event_to_mac_task();
        }
    }
}

/*****************************************************************************/
int get_join_state (void);
/* Churns the security loop, checking if there are messages to decrypt
 */
void mac_process_secured_messages(void)
{
  mac_rx_t *rxmsg = NULL;
  security_struct_t *pSecurity = NULL;
  uint8_t src_addr[8];
  device_descriptor_t *pDD = NULL;
  uint8_t mkey_index = 0;
  uchar header_length = 0;
  uint16_t payload_length = 0;
  uint16_t recved_pkt_len = 0;
  
  rxmsg = (mac_rx_t *) queue_item_get((queue_t *) & mac_security_data.rx_security_queue);
  
  if (rxmsg == NULL_POINTER)
  {
    clear_process_activity(AF_SECURITY_IN_PROGRESS);
    event_clear(RX_SEC_FRAME_EVENT);
    return;
  }
  
//#if (PRINT_DEBUG_LEVEL==ERROR_DEBUG)
//  stack_print_debug ("frame fetched from queue for doing security: ");
//  print_mac_address (rxmsg->src.address.ieee_address);
//#endif  
#if APP_LBR_ROUTER  
#if(FAN_EDFE_FEATURE_ENABLED == 1)
  if(edfe_information.edfe_frame_enabled == 0x01)
  {
    mem_rev_cpy(src_addr, edfe_information.edfe_ini_mac_addr, IEEE_ADDRESS_LENGTH);
  }
  else
#endif
  {
    memcpy (src_addr, rxmsg->src.address.ieee_address, IEEE_ADDRESS_LENGTH);
  }
#endif 
  pDD = get_device_descriptior_from_pib (src_addr);
  if (pDD == NULL_POINTER)
  {
    rxmsg->security_status = MAC_UNAVAILABLE_KEY;
    /* Put unsecured message on to message queue */
#if ((PRINT_DEBUG_LEVEL==ERROR_DEBUG)||(PRINT_DEBUG_LEVEL==CONSOLE_ERROR_DEBUG))
    stack_print_debug ("### Dev descriptor not found\n");
#endif
    mac_return_unsecured_message(rxmsg);
    set_process_activity(AF_RCV_MSG_PENDING);
    return;
  }
  
  mac_get_security_data(rxmsg);
  
  /* Allocate a buffer for security data associated with this frame */
  pSecurity = (security_struct_t *) app_bm_alloc(sizeof(security_struct_t));
  if (pSecurity == NULL_POINTER)
  {
    rxmsg->security_status = MAC_SECURITY_ERROR;
    /* Put unsecured message on to message queue */
#if ((PRINT_DEBUG_LEVEL==ERROR_DEBUG)||(PRINT_DEBUG_LEVEL==CONSOLE_ERROR_DEBUG))
    stack_print_debug ("### pSecurity malloc fail\n");
#endif
    mac_return_unsecured_message(rxmsg);
    set_process_activity(AF_RCV_MSG_PENDING);
    return;
  }
  
//#if (PRINT_DEBUG_LEVEL==ERROR_DEBUG)
//  stack_print_debug ("3M %d %p\n", sizeof(security_struct_t), pSecurity);
//#endif 
  
  pSecurity->q_item.link = NULL_POINTER;
  /* Create the links between the data and Security Data */
  pSecurity->private_msg_data = rxmsg;
  rxmsg->security_data = pSecurity;
  
  mkey_index = rxmsg->sec_param.key_identifier[0];
  
  /* Check the Frame Counter */
  if ((rxmsg->frame_counter == 0xffffffffUL) || (rxmsg->frame_counter < pDD->frame_count[mkey_index-1]))
  {
#if ((PRINT_DEBUG_LEVEL==ERROR_DEBUG)||(PRINT_DEBUG_LEVEL==CONSOLE_ERROR_DEBUG))
    stack_print_debug ("### Frame counter error r[%d] d[%d]\n", rxmsg->frame_counter, pDD->frame_count[mkey_index-1]);
#endif
    rxmsg->security_status = MAC_COUNTER_ERROR;
    /* Put unsecured message on to message queue */
    mac_return_unsecured_message(rxmsg);
    /* Delete Memory used by Security data */
//#if (PRINT_DEBUG_LEVEL==ERROR_DEBUG)
//    stack_print_debug ("3F %p\n", pSecurity);
//#endif        
    app_bm_free((uint8_t*)pSecurity);    
    rxmsg->security_data = NULL_POINTER;
    set_process_activity(AF_RCV_MSG_PENDING);
    return;
  }
  
  /* Calculate the start of the MAC payload */
  header_length = ( rxmsg->payload - rxmsg->pd_rxp->psdu ) + 1;//added one to include the command ID also
  
  /* If it is Data - Decrease header length to start of payload */
  if ((rxmsg->pd_rxp->psdu[0] & 0x7) == MAC_FRAME_TYPE_DATA)
  {
    /* Move back 1 byte as there is no command frame identifier */
    header_length--;
  }
  
  // Raka .. 05-Dec-2017 we need to do for ACK also ...
  if ((rxmsg->pd_rxp->psdu[0] & 0x7) == MAC_FRAME_TYPE_ACK)
  {
    /* Move back 1 byte as there is no command frame identifier */
    header_length--;
  }
  
  /* Calc length of payload = total - header - MIC - FCS */
  /*This shall include only the "payload" part and not include the command ID*/
  recved_pkt_len = (header_length + 
                    integrity_code_length(rxmsg->sec_param.security_level) +
                      (rxmsg->pd_rxp->FCSLength & 0x7FFF));  
  payload_length = rxmsg->pd_rxp->psduLength - recved_pkt_len ;
  
  if (rxmsg->pd_rxp->psduLength < recved_pkt_len)
    payload_length = 0;
  
  /* Save details required to perform decryption */
  pSecurity->security_level = rxmsg->sec_param.security_level;
  pSecurity->header = &rxmsg->pd_rxp->psdu[0];//points to MAC Frame control
  pSecurity->header_length = header_length;//includes hdr+aux hdr+1 byte cmd id if command frame
  pSecurity->payload = &rxmsg->pd_rxp->psdu[0] + header_length;
  pSecurity->payload_length = payload_length;//actual pld len without the command id, FCS, and even the MIC
  pSecurity->state = MAC_RX_STATE_NOT_PROCESSED;
#if APP_LBR_ROUTER  
  copy_key (pSecurity->key, mac_key_list.MAC_SECURITY_KEY_LIST[mkey_index - 1].MAC_SECURITY_KEY);
#endif  
  /* Setup nonce */
  init_nonce (&pSecurity->nonce[0], rxmsg->frame_counter, rxmsg->sec_param.security_level, &pDD->ieee_addr[0]);
  
  pSecurity->return_queue = queue_manager_get_list(QUEUE_RX_MSG);
  pSecurity->event_prio = CMD_OR_DATA_FRAME_RX_EVENT;
  
  pDD->frame_count[mkey_index - 1] = ++rxmsg->frame_counter;
  
  /* Add to Security queue to be processed */
  queue_item_put((queue_t *) & mac_security_data.hallin_rx_queue,
                 (queue_item_t *) pSecurity);
  event_set( UNSECURE_EVENT );
}
void reset_direct_mac_frame_counter (void)
{
  mac_security_data.pib.mac_frame_counter = 0;
}

#endif

/*****************************************************************************/

uchar mac_process_data_requests(void)
{
    //mac_rx_t *rxmsg;
#ifdef MAC_CFG_SECURITY_ENABLED
  mac_address_t src_addr = {0};
  mac_address_t dst_addr = {0};
#endif

    mac_rx_t *rxmsg = (mac_rx_t *) queue_manager_pop_front(QUEUE_DATA_REQUEST);

    if( rxmsg == NULL_POINTER )
    {
    	event_clear(DATA_REQ_RX_EVENT);
    	return 0;
    }
#ifdef MAC_CFG_SECURITY_ENABLED
    /* Check the results of the Authentication Procedure */
    check_authentication_status(rxmsg);

    if (rxmsg->security_status == SUCCESS)
    {
#endif
        if (rxmsg->frame_pending_out)
        {
#if ( ( MAC_CFG_LE_RIT_CAPABILITY == 1 ) || ( MAC_CFG_LE_CSL_CAPABILITY == 1 ) )
			if ( rxmsg->payload[rxmsg->payloadIEFieldLen] == LE_RIT_DATA_REQUEST )
			{
				mac_process_rit_data_request_command(rxmsg);
			}
			else
#endif			 
			{				
				/* now process this specific MAC command */
                                /* Raka [ 20- Nov-2018] : I chnaged the pointer referance for compilation issues
                                mac_process_data_request_command((rxmsg->pd_rxp->psdu)-1);

                                rxmsg->pd_rxp->psdu : this is the palce holder for the data popinter
                                uint8_t psdu[1]; //< Place holder from where the psdu bytes are placed >
                                We need to check the behaviour at run time 

                                WHY DATA POINTER - 1, NEEDS TO BE CHECKED
                          */
                          
                          mac_process_data_request_command(rxmsg->pd_rxp->psdu);
			}
			
        }

#ifdef MAC_CFG_SECURITY_ENABLED
    }
    else /* The Data Request Failed Security Processing */
    {
        /* Extract SRC and DST addresses from the message */
        mac_frame_parse_addresses(rxmsg,&rxmsg->pd_rxp->psdu[0], &dst_addr, &src_addr);

        /* Report Errror to NHL */
        send_mlme_comm_status_indication( &src_addr, &dst_addr,
                                         rxmsg->security_status,
                                         &rxmsg->sec_param );
    }
#endif
    mac_free_rcv_buffer( rxmsg );
    return 1;
}

/*****************************************************************************/ 

/*
** ============================================================================
** Private Function Definitions
** ============================================================================
*/

/*move an indirect message to the direct queue*/
static void mac_transmit_indirect_message( mac_tx_t *txd )
{
#if( CFG_MAC_PENDADDR_ENABLED == 1)   
  /* update Pending Address List */
    if( pendaddr_remove( txd ) != PENDADDR_SUCCESS )
    {
        /*TBD What to do here? */
        //debug(("pendaddr err\r\n"));
    }
#endif    
#if (CFG_MAC_BEACON_ENABLED == 1)    /* update beacon */
    if( mac_beacon_update( MAC_FRAME_BCN_SUB_TYPE_RB ) != MAC_SUCCESS )
    {
        /*TBD What to do here? */
        //debug(("bcn update err\r\n"));
    }
#endif
    /* there is a packet ready to send, so remove it from the table */
    queue_manager_remove( QUEUE_INDIRECT_TX, (queue_item_t *) txd );

    /* and check if there is another for this device */
    /*TBD This may be called twice... */
    if( mac_find_indirect_transmission( &txd->dst, 0 ) == MAC_SUCCESS )
    {
        /* there is more, so set the frame pending bit in the header */
        txd->data->psdu[0] |= MAC_FRAME_PENDING;
    }

    /* clear status to allow transmission */
    txd->status = MAC_SUCCESS;

    /* then queue it */
    mac_queue_direct_transmission( txd );
}
/*****************************************************************************/

/*called to process a data request MAC command frame
 *rxb[0]: length, rxb[1,2]: control fields, rxb[3]: dsn, rxb[4]: pan id */
static void mac_process_data_request_command(
                                             uchar *rxb /* pointer to received data buffer */
                                             )
{
        mac_tx_t *tx_table = NULL;               /* pointer for the indirect tx table entries */
        uchar src_addr_offset = 0;               /* offset to the source of the Data Request  */
        mac_address_t dst_address = {0};         /* where data request command came from */
        mac_address_t src_address = {0};         /* us! */
        mac_tx_t *dtx = NULL;

        phy_data_params_t phy_params = {0};
        ulong current_channel = 0;
        uint16_t len = 0;
        PLME_get_request( phyCurrentChannel, &len, &current_channel );
        phy_params.tx_channel = (uint8_t)current_channel;

    switch (rxb[2] & MAC_DST_ADDRESS_MASK)
    {
    case MAC_NO_DST_ADDRESS:
        src_addr_offset = 6;    /* step over length, 2 control field bytes, DSN, and 2 bytes pan id */
        break;

    case MAC_SHORT_DST_ADDRESS:
        src_addr_offset = 10;   /* length, 2 control field bytes, DSN, 4 bytes dest address, 2 bytes pan id */
        break;

    case MAC_IEEE_DST_ADDRESS:
        src_addr_offset = 16;   /* length, 2 control field bytes, DSN, 10 bytes dest address, 2 bytes pan id, */
        break;

    default:
        /* shouldn't happen ! */
        return;
    }

    /* override the battery life extension timer to ensure we transmit before the
     * device times out
     */
    // FIXME: PEB 9/1/06
#define MAX_BLE_TIME 3
    mac_data.ble_expiry_timer = MAX_BLE_TIME;

    /* there is no src pan id if the Intra PAN bit is set (so deduct the additional offset) */
    if (rxb[1] & MAC_INTRA_PAN)
    {
        src_addr_offset -= 2;
    }

    /* get the incoming source address information */
    dst_address.address_mode = ((rxb[2] & MAC_SRC_ADDRESS_MASK) >> MAC_SRC_ADDRESS_SHIFT);

    switch (dst_address.address_mode)
    {
    case MAC_NO_ADDRESS:
        break;

    case MAC_SHORT_ADDRESS:
        dst_address.address.short_address =
            //rxb[src_addr_offset] | (rxb[src_addr_offset + 1] << 8);
	    ( rxb[src_addr_offset] + (((uint16_t)(rxb[src_addr_offset+1])) << 8));
        break;

    case MAC_IEEE_ADDRESS:
        dst_address.address.ieee_address = &rxb[src_addr_offset];
        break;
    }                           /* end of switch statement */


    /* find the corresponding pending data */
    for (tx_table =
	     (mac_tx_t *) queue_manager_scan_next(QUEUE_INDIRECT_TX,
						  NULL_POINTER);
         tx_table != NULL_POINTER;
         tx_table =
	     (mac_tx_t *) queue_manager_scan_next(QUEUE_INDIRECT_TX,
						  (queue_item_t *) tx_table))
	
    {
        /* check that the destination address matches */
        if (tx_table->dst.address_mode == MAC_SHORT_ADDRESS)
        {
            if (dst_address.address_mode == MAC_SHORT_ADDRESS)
            {
                if (dst_address.address.short_address !=
                    tx_table->dst.address.short_address)
                {
                    continue;
                }

                /* this is a matching packet, so send it clearing up on the way */
                mac_transmit_indirect_message(tx_table);
                return;
            }
        }
        else if (tx_table->dst.address_mode == MAC_IEEE_ADDRESS)
        {
            if (dst_address.address_mode == MAC_IEEE_ADDRESS)
            {
                if (ieeeaddr_cmp(&rxb[src_addr_offset], tx_table->dst.address.ieee_address) != 0)
                {
                    /* match failed */
                    continue;
                }

                /* this is a matching packet, so send it clearing up on the way */
                mac_transmit_indirect_message(tx_table);
                return;
            }
        }
    }

    /**************************************************************************
     * falls through to here if no message was found for the requesting device
     ***************************************************************************/

    /* there is no src pan id if the Intra PAN bit is set (so deduct the additional offset) */
    if (rxb[1] & MAC_INTRA_PAN)
    {
        /* use the destination PAN id (step over length, 2 control bytes, dsn) */
        dst_address.pan_id = rxb[4] + (rxb[5] << 8);
    }
    else
    {
        /* pan id is the 2 bytes before the source address */
        dst_address.pan_id = rxb[src_addr_offset - 2] + (rxb[src_addr_offset - 1] << 8);
    }

    src_address.address_mode = 0;
    /* create an empty data packet to transmit */
    mac_frame_build_data( &dtx, &src_address, &dst_address, 0,0,
                         NULL, 0, 0, NULL,&phy_params,NULL_POINTER );

    if (dtx != NULL_POINTER)
    {
        dtx->tx_options |= TX_OPTION_MAC_INITIATED_DATA;
        /* change its type, so the foreground doesn't send a confirm to the network layer */
        mac_queue_direct_transmission(dtx);
    }

} /* mac_process_data_request_command() */

#if ( ( MAC_CFG_LE_RIT_CAPABILITY == 1 ) || ( MAC_CFG_LE_CSL_CAPABILITY == 1 ) )
static void mac_process_rit_data_request_command(
                                             		mac_rx_t *rxmsg 
                                             	 )
{
    mac_tx_t *ind_msg = NULL;         /* pointer for the indirect tx table entries */
  
    /*
     * override the battery life extension timer to ensure we transmit before the
     * device times out
     */
    // FIXME: PEB 9/1/06
#define MAX_BLE_TIME 3
    mac_data.ble_expiry_timer = MAX_BLE_TIME;
    	
	if( ( (rxmsg->dst.address_mode == MAC_SHORT_ADDRESS)&&
					(rxmsg->dst.pan_id == BROADCAST_PAN_ID)&&
					(rxmsg->dst.address.short_address == BROADCAST_SHORT_ADDRESS)  ))
	{
		if( mac_get_matching_indirect_data_msg(&ind_msg, &rxmsg->dst, 1) == MAC_SUCCESS )
		{
			/*change the dest address and dest pan id of the frame to be sent out to 
			that of the recived RIT data request command's src address information */
			ind_msg->dst.pan_id = rxmsg->src.pan_id;
			ind_msg->dst.address.short_address = rxmsg->src.address.short_address;
			
			put_ushort(&(ind_msg->data->psdu[3]),ind_msg->dst.pan_id);
			
			put_ushort(&(ind_msg->data->psdu[5]),ind_msg->dst.address.short_address);
			
			mac_transmit_indirect_message( ind_msg );
		}
		/*since the recieved RIT command was having dest addres and dest pan id as 
		0xFFFF, we checked for any data with dest addr and pan id as 0xFFFF. If found 
		any, send the data or else terminate the RIT data request command processing */
		return;
	}
	
	if( mac_get_matching_indirect_data_msg(&ind_msg, &rxmsg->src, 0) == MAC_SUCCESS )		
	{
		mac_transmit_indirect_message( ind_msg );
		return;
	}
	

} /* mac_process_rit_data_request_command() */

static mac_status_t mac_get_matching_indirect_data_msg( mac_tx_t** p_ind_msg, mac_address_t* p_addr_to_match, uchar match_pan_id )
{
	 mac_tx_t *tx_table = NULL;         /* pointer for the indirect tx table entries */
	/* find the corresponding pending data */
    for (tx_table =
	     (mac_tx_t *) queue_manager_scan_next(QUEUE_INDIRECT_TX,
						  NULL_POINTER);
         tx_table != NULL_POINTER;
         tx_table =
	     (mac_tx_t *) queue_manager_scan_next(QUEUE_INDIRECT_TX,
						  (queue_item_t *) tx_table))
	
    {
        /* check that the destination address matches */
        if (tx_table->dst.address_mode == MAC_SHORT_ADDRESS)
        {
            if (p_addr_to_match->address_mode == MAC_SHORT_ADDRESS)
            {
                if (p_addr_to_match->address.short_address ==
                    tx_table->dst.address.short_address)
                {					 
					if( match_pan_id )
					{
						if( p_addr_to_match->pan_id != tx_table->dst.pan_id )
						{
							continue;
						}
					}

					*p_ind_msg = tx_table;
					return MAC_SUCCESS;  									                 
                }               
            }
        }
        else if (tx_table->dst.address_mode == MAC_IEEE_ADDRESS)
        {
            if (p_addr_to_match->address_mode == MAC_IEEE_ADDRESS)
            {
                if (ieeeaddr_cmp(p_addr_to_match->address.ieee_address, tx_table->dst.address.ieee_address) == 0)
                {
                    if( match_pan_id )
					{
						if( p_addr_to_match->pan_id != tx_table->dst.pan_id )
						{
							continue;
						}
					}

					*p_ind_msg = tx_table;
					return MAC_SUCCESS; 
                }
            }
        }
    }
	return MAC_NO_DATA;
}
#endif

#ifdef MAC_CFG_GTS_ENABLED
/*checks if a gts data packet will fit in the allocated GTS */
static uchar mac_check_available_gts_length(void)
{
    ushort reserved_length = 0;
    ushort reserved_slots = 0;

    /* beacon length + 440 symbols + allow 10 symbols for PHY preamble */
    reserved_length = ((440 + 12 + (mac_data.beacon_tx.length * 2)) / 20); /* in 20 symbol slots */

    /* round reserved length DOWN to ensure reserved_slots is correct for integrally divisable numerator */
    reserved_slots = ((reserved_length - 1) / number_of_backoffs_in_slot()) + 1;

    return (mac_data.final_cap_slot - reserved_slots);
}
#endif

/*****************************************************************************/

#if MAC_CFG_SECURITY_ENABLED
//#ifdef MAC_CFG_SECURITY_ENABLED /*Umesh changed here*/

void append_frame_counter( uchar *bp )
{
        if (!(mac_security_data.security_flags & SF_FRAME_COUNTER))
        {
            *bp++ = mac_security_data.pib.mac_frame_counter;
            *bp++ = mac_security_data.pib.mac_frame_counter >> 8;
            *bp++ = mac_security_data.pib.mac_frame_counter >> 16;
            *bp   = mac_security_data.pib.mac_frame_counter >> 24;
        }
        else
        {
            *bp++ = mac_security_data.pib.mac_frame_counter >> 24;
            *bp++ = mac_security_data.pib.mac_frame_counter >> 16;
            *bp++ = mac_security_data.pib.mac_frame_counter >> 8;
            *bp   = mac_security_data.pib.mac_frame_counter;
        }
}

/*****************************************************************************/

void Enqueue_Secure_Item( security_struct_t * p_sec_item )
{
	
	mac_tx_t * txp = (mac_tx_t*)(p_sec_item->private_msg_data);
	mac_rx_t *rxmsg = (mac_rx_t*)(p_sec_item->private_msg_data);
#if(CFG_MAC_PTSM_ENABLED == 1)          
	sm_event_t event = { (sm_trigger_t) PTSM_TRIGGER_PACKET, { 0 } };
#endif
	
	if( p_sec_item->return_queue == (queue_t *)queue_manager_get_list(QUEUE_INDIRECT_TX))
	{
		
		queue_manager_push_back(QUEUE_INDIRECT_TX, (queue_item_t *) txp);
#if( CFG_MAC_PENDADDR_ENABLED == 1)    
        /* update Pending Address List */
        if( pendaddr_add( txp ) != PENDADDR_SUCCESS )
        {
            /*TBD What to do here? */
            //debug(("pendaddr err\r\n"));
        }
#endif        
#if (CFG_MAC_BEACON_ENABLED == 1)
        /* update beacon */
        if( mac_beacon_update( MAC_FRAME_BCN_SUB_TYPE_RB ) != MAC_SUCCESS )
        {
            /*TBD What to do here? */
            //debug(("bcn update err\r\n"));
        }
#endif
        /* notify PT-SM */
#if(CFG_MAC_PTSM_ENABLED == 1)          
        SM_DISPATCH( (sm_t *) ptsm_p, &event );
#endif 
        
        set_process_activity(AF_IND_MSG_PENDING);	
	}
	else if ( p_sec_item->return_queue == (queue_t *)queue_manager_get_list(QUEUE_BCN_TX_CURR) )
	{
		queue_manager_push_back( QUEUE_BCN_TX_CURR, (queue_item_t *) txp );
	}
	else if ( p_sec_item->return_queue == (queue_t *)queue_manager_get_list(QUEUE_MPM_EB_TX_CURR) )
	{
		queue_manager_push_back( QUEUE_MPM_EB_TX_CURR, (queue_item_t *) txp );
	}
	else if ( p_sec_item->return_queue == (queue_t *)queue_manager_get_list(QUEUE_EB_TX_CURR) )
	{
		queue_manager_push_back( QUEUE_EB_TX_CURR, (queue_item_t *) txp );	
	}	
	else if ( p_sec_item->return_queue == (queue_t *)queue_manager_get_list(QUEUE_CAP_TX) )
	{
                queue_manager_push_back( QUEUE_CAP_TX, (queue_item_t *) txp );
               // set_process_activity( AF_CAP_MSG_PENDING );
		//trxsm_set_pending_tx_event(&trxsm);		
	}
	else if ( p_sec_item->return_queue == (queue_t *)queue_manager_get_list(QUEUE_BCAST))
	{
                queue_manager_push_back(QUEUE_BCAST, (queue_item_t *) txp);
                //set_process_activity(AF_CAP_MSG_PENDING);
		//trxsm_set_pending_tx_event(&trxsm);
	}
	else if ( p_sec_item->return_queue == (queue_t *)queue_manager_get_list(QUEUE_BCN_RX) )
	{
                /* queue beacon for further processing */
                queue_manager_push_back( QUEUE_BCN_RX,
                (queue_item_t *) rxmsg );
                set_process_activity(AF_BEACON_RECEIVED);

                event_set(BCN_RX_EVENT);
                //signal_event_to_mac_task();
	}
	else if ( p_sec_item->return_queue == (queue_t *)queue_manager_get_list(QUEUE_DATA_REQUEST) )
	{
		queue_manager_push_back(QUEUE_DATA_REQUEST, (queue_item_t *) rxmsg);
        event_set(DATA_REQ_RX_EVENT);
		//signal_event_to_mac_task();
	}
	else if ( p_sec_item->return_queue == (queue_t *)queue_manager_get_list(QUEUE_RX_MSG) )
	{
//#if (PRINT_DEBUG_LEVEL==ERROR_DEBUG)
//                stack_print_debug ("frame enqueued for mac rcv msg process and CMD_OR_DATA_FRAME_RX_EVENT set: ");
//                print_mac_address (rxmsg->src.address.ieee_address);
//#endif  
		queue_manager_push_back(QUEUE_RX_MSG, (queue_item_t *) rxmsg);
                event_set(CMD_OR_DATA_FRAME_RX_EVENT);
		//signal_event_to_mac_task();
	}
	
}
#endif

/*****************************************************************************/

void set_process_activity(ushort activity_flag )
{
//    cpu_save_imask_and_clear(flags);
    processor_active |= activity_flag;
//    cpu_restore_imask(flags);
}

/*****************************************************************************/

void clear_process_activity(ushort activity_flag)
{
//    cpu_save_imask_and_clear(flags);
    processor_active &= ~activity_flag;
//    cpu_restore_imask(flags);
}

void set_uart_tx_activity(uint8_t status)
{
	if( status )
	{
		set_process_activity(AF_UART_MSG_TX_PENDING);
	}
	else
	{
		clear_process_activity(AF_UART_MSG_TX_PENDING);
	}
}

/*****************************************************************************/

uint32_t get_processor_activity( void )
{
    return processor_active;
}

/*****************************************************************************/

/*
** ============================================================================
** External Function Declarations
** ============================================================================
*/
#ifdef WISUN_FAN_MAC
extern mac_status_t Build_MLME_WS_ASYNC_FRAME_Request(mac_tx_t** p, 
                                               uint32_t chan_value,
                                               uint8_t fan_frame_type,
                                               uint32_t hdr_bitmap,
                                               uint32_t pld_bitmap);
extern mac_status_t Build_MCPS_EAPOL_FRAME_Request(mac_tx_t** p, 
                                               uint32_t chan_value,
                                               uint8_t fan_frame_type,
                                               uint8_t *hdr_ie_list,
                                               uint8_t *payload_ie_list );
mac_status_t Build_ACK_FRAME_Request(mac_tx_t** p,
                           mac_rx_t *mrp,
                           uint32_t sub_hdr_bitmap,
                           uint32_t sub_pld_bitmap);

#if(FAN_EDFE_FEATURE_ENABLED == 1)
mac_status_t build_EDFE_frame_Req_res(mac_tx_t** p,
                                      uint8_t *dest_addr,
                                      uint32_t hdr_bitmap,
                                      uint32_t pld_bitmap);
#endif

uint8_t trigger_SECURE_NON_SECURE_ACK (mac_tx_t *dr);
mac_status_t mac_cca_event_do(mac_tx_t *txd);
#endif

/*
** =============================================================================
** Public Variables Definitions
** =============================================================================
**/

/* None */

/*
** ============================================================================
** Public Function Definitions
** ============================================================================
*/

/*****************************************************************************/
#ifdef WISUN_FAN_MAC
uchar Create_MLME_WS_ASYNC_FRAME_Request( uint32_t Start_Channel, 
                                          uint8_t fan_frame_type, 
                                          uint32_t hdr_bitmap,//uint8_t *hdr_ie_list, 
                                          uint32_t pld_bitmap)//uint8_t *payload_ie_list)

{
    mac_tx_t *dr = NULL;
  
    Build_MLME_WS_ASYNC_FRAME_Request(&dr,Start_Channel ,fan_frame_type, hdr_bitmap, pld_bitmap );
    if( dr )
    {
#if (UPDATE_UFSI_AFTER_CCA == 0)
      return mac_queue_bcast_transmission( dr );
#else
      return mac_cca_event_do(dr);
#endif
    }
    else
    {
        return MAC_TRANSACTION_OVERFLOW;
    }
  
}

/*****************************************************************************/
//uchar Create_MCPS_EAPOL_FRAME_Request( uint32_t Start_Channel, 
//                                          uint8_t fan_frame_type, 
//                                          uint8_t *hdr_ie_list, 
//                                          uint8_t *payload_ie_list)
//
//{
//    mac_tx_t *dr = NULL;
//  
//    Build_MCPS_EAPOL_FRAME_Request(&dr,Start_Channel ,fan_frame_type, hdr_ie_list,payload_ie_list );
//    if( dr )
//    {
//        return (mac_queue_direct_transmission(dr));
//    }
//    else
//    {
//        return MAC_TRANSACTION_OVERFLOW;
//    }
//  
//}     //Debdeep

#if(FAN_EDFE_FEATURE_ENABLED == 1)
uchar create_edfe_frame(uint8_t *dest_addr, uint32_t sub_hdr_bitmap, uint32_t sub_pld_bitmap)
{
  mac_tx_t *dr = NULL;
  build_EDFE_frame_Req_res(&dr,dest_addr ,sub_hdr_bitmap,sub_pld_bitmap );
  if( dr )
  {
    return (mac_queue_direct_transmission(dr));
  }
  else
  {
    return MAC_TRANSACTION_OVERFLOW;
  }
}
#endif

/*****************************************************************************/

/* Debdeep :: Instead of posting event, we do security encryption for ACK packet directly from here */
extern self_info_fan_mac_t mac_self_fan_info;
void subscribe_tx_frame (mac_tx_t *txd);
void encrypt_ack_frame (security_struct_t* p_security_data);
uint32_t update_ufsi_with_procces_time(mac_tx_t *out_packt);
uint32_t update_bfio_with_procces_time(mac_tx_t *out_packt);
void set_frame_counter_and_nonce (mac_tx_t *txd);
uchar Create_ACK_FRAME_Request( mac_rx_t *mrp,
                               uint32_t sub_hdr_bitmap,//uint8_t *hdr_ie_list, 
                               uint32_t sub_pld_bitmap)

{
  mac_tx_t *dr = NULL;
  uint32_t ufsi = 0;
  uint32_t bfio = 0;
  
  Build_ACK_FRAME_Request(&dr,mrp, sub_hdr_bitmap, sub_pld_bitmap);
  if( dr )
  {
    /* Debdeep :: Enabled frame subscription for non secured ACK Tx */
    subscribe_tx_frame (dr);
    
    if (dr->security_data->security_level == MAC_SECURITY_AES_CCMSTAR_ENC_MIC_64)     //Security level 6
      set_frame_counter_and_nonce (dr);

    if(mac_self_fan_info.unicast_listening_sched.us_schedule.channel_function != CF_FIXED_CHANNEL)
    {
      if(dr->p_ufsi!=NULL)
      {
        ufsi = update_ufsi_with_procces_time(dr);
        memcpy(dr->p_ufsi,(uint8_t*)&ufsi,3);
      }
      if(dr->p_bfsi!=NULL)
      {
        bfio =  update_bfio_with_procces_time(dr);
        memcpy(dr->p_bfsi,(uint8_t*)&bfio,3);
      }
    }
    
    if (dr->security_data->security_level == MAC_SECURITY_AES_CCMSTAR_ENC_MIC_64)     //Security level 6
      encrypt_ack_frame (dr->security_data);
    
    trigger_SECURE_NON_SECURE_ACK (dr);
    
    return MAC_SUCCESS;
  }
  else
  {
    return MAC_TRANSACTION_OVERFLOW;
  }
}
#endif


