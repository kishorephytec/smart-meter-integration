/** \file mac_mem.c
 *******************************************************************************
 ** \brief Provides APIs for Memory Management
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
#include "phy.h"
#include "mac_config.h"
#include "mac.h"
#include "mac_defs_sec.h"
#include "ie_element_info.h"
#include "mac_defs.h"
#include "mac_queue_manager.h"
#include "mac_pib.h"
#include "mac_mem.h"

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

extern uchar heap[];
extern mac_pib_t mac_pib;

extern void * app_bm_alloc(
    uint16_t length//base_t length       
    );
    

extern void app_bm_free(
    uint8_t *pMem      
    );

/*
** ============================================================================
** Private Function Prototypes
** ============================================================================
*/

/* None */

/*
** ============================================================================
** Public Function Definitions
** ============================================================================
*/

mac_tx_t* mac_mem_alloc_tx(
                          uchar type,      
                          uint16_t length,    
                          uchar secure,     
                          uchar sub_type	
                          )
{
    mac_tx_t *txd = NULL;
    uchar tx_curr_q_id = 0, tx_free_q_id = 0;

    switch( type )
    {
      case MAC_FRAME_TYPE_BEACON:
      tx_curr_q_id = QUEUE_MPM_EB_TX_CURR; 
      tx_free_q_id = QUEUE_MPM_EB_TX_FREE;

      if( MAC_FRAME_BCN_SUB_TYPE_RB == sub_type )
      {
        tx_curr_q_id = QUEUE_BCN_TX_CURR;
        tx_free_q_id = QUEUE_BCN_TX_FREE;
      }
      else if ( MAC_FRAME_BCN_SUB_TYPE_EB == sub_type )
      {
        tx_curr_q_id = QUEUE_EB_TX_CURR;
        tx_free_q_id = QUEUE_EB_TX_FREE;
      }
      /*else
      {
      tx_curr_q_id = QUEUE_MPM_EB_TX_CURR;
      tx_free_q_id = QUEUE_MPM_EB_TX_FREE;
      }*/

      /* allocate frame from specific beacon lists */
      if( (txd = (mac_tx_t *) queue_manager_pop_front( tx_free_q_id )) == NULL_POINTER &&
      (txd = (mac_tx_t *) queue_manager_pop_front( tx_curr_q_id )) == NULL_POINTER )
      {
        /*TBD Shall we wait here if both queues are empty? */
        goto end;
      }

      /* free potentially allocated memory (if packet is from QUEUE_BCN_TX_CURR) */
      if( txd->data )
      {
        app_bm_free((uint8_t*)txd->data);
      }

      /*TBD get space for beacon data properly */

      if( (txd->data = (phy_tx_t *) app_bm_alloc(length)) == NULL_POINTER )
      {
        goto free_msg;
      }

      if( (mac_pib.mac_security_enabled) && ( mac_data.security ) )
      {
        if(  MAC_FRAME_BCN_SUB_TYPE_RB == sub_type )
        {
          txd->security_data = &mac_data.security->beacon_data;
        }
        else if ( MAC_FRAME_BCN_SUB_TYPE_MPM_EB == sub_type  )
        {
          txd->security_data = &mac_data.security->mpm_eb_data;
        }
      }
      else
      {
        txd->security_data = NULL_POINTER;
      }
      break;

      case MAC_FRAME_TYPE_MAC_COMMAND:/*fall through*/
      case MAC_FRAME_TYPE_DATA:
#ifdef ENHANCED_ACK_SUPPORT
      case MAC_FRAME_TYPE_ACK:
#else
      case MAC_FRAME_TYPE_ACK://for fan_ack_type	
#endif //#ifdef ENHANCED_ACK_SUPPORT		

      /* get a free message from the pool */
      if( (txd = (mac_tx_t *) queue_manager_pop_front( QUEUE_TX_FREE )) == NULL_POINTER )
      {
        goto end;
      }

      /* allocate a buffer to store the data */
      if( (txd->data = (phy_tx_t *) app_bm_alloc(length)) == NULL_POINTER )
      {
        goto free_msg;
      }

      if( secure )
      {
        /* allocate a buffer to store security data */
        //txd->security_data = (security_struct_t *) bm_alloc( heap,sizeof(security_struct_t) );
        if( (txd->security_data = (security_struct_t *) app_bm_alloc( sizeof(security_struct_t) ) )== NULL_POINTER )
        {
            goto free_data;
        }
        memset (txd->security_data, 0, sizeof (*txd->security_data));
      }
      else
      {
        txd->security_data = NULL_POINTER;
      }
      break;

      default:
      goto end;
    }

    /* some initialisation */
    txd->length = length;
    txd->src.address_mode = MAC_NO_ADDRESS;
    txd->dst.address_mode = MAC_NO_ADDRESS;
    
/*Uemsh : 11-01-2018 this thing goes to modified after sepration of mac and fan mac*/
#ifdef WISUN_FAN_MAC    
    txd->p_ufsi = NULL;
#endif    
    /* all went well, so return message */
    return txd;

free_data:
    app_bm_free( (uint8_t*)txd->data );
    txd->data = NULL_POINTER;

free_msg:
    queue_manager_push_back( QUEUE_TX_FREE, (queue_item_t *) txd );

end:
    return NULL_POINTER;
    }

/******************************************************************************/

void mac_mem_free_tx(
                     mac_tx_t *txd 
                       )
{
  uchar tx_free_q_id = 0;
  switch( txd->type )
  {
  case MAC_FRAME_TYPE_BEACON:
    /* release the frame */
    app_bm_free((uint8_t*)txd->data);
    
    txd->data = NULL_POINTER;
    
    tx_free_q_id = QUEUE_MPM_EB_TX_FREE;
    
    if( MAC_FRAME_BCN_SUB_TYPE_RB == txd->sub_type )
    {			
      tx_free_q_id = QUEUE_BCN_TX_FREE;
    }
    else if ( MAC_FRAME_BCN_SUB_TYPE_EB == txd->sub_type )
    {			
      tx_free_q_id = QUEUE_EB_TX_FREE;
    }
    /*else
    {			
    tx_free_q_id = QUEUE_MPM_EB_TX_FREE;
  }*/
    
    /* back to free queue */
    queue_manager_push_back( tx_free_q_id, (queue_item_t *) txd );
    break;
    //#ifdef ENHANCED_ACK_SUPPORT
    //	case MAC_FRAME_TYPE_ACK:
    //          /* release the frame */
    //            app_bm_free((uint8_t*)txd->data);
    //            txd->data = NULL_POINTER;
    //         break;   
    
    //#endif
  case MAC_FRAME_TYPE_ACK:
  case MAC_FRAME_TYPE_MAC_COMMAND:
  case MAC_FRAME_TYPE_DATA:
    
    /* relase security data */
    if( txd->security_data != NULL_POINTER )
    {
      if((txd->security_data->raw_payload_data != NULL)
         &&(txd->security_data->payload_length != 0))
        app_bm_free( (uint8_t*)txd->security_data->raw_payload_data);
      app_bm_free( (uint8_t*)txd->security_data );
      txd->security_data->raw_payload_data = NULL_POINTER;
      txd->security_data = NULL_POINTER;
/* Debdeep :: This structure retains its value and corrupt next packet */
      memset ((uint8_t *)&txd->sec_param, 0, sizeof (txd->sec_param));
    }
    
    /* release the frame */
    app_bm_free( (uint8_t*)txd->data);
    txd->data = NULL_POINTER;
    
    /* some reset */
    txd->cap_retries = 0;
    txd->num_csmaca_backoffs = 0;
    txd->missed_ack_count = 0;
    txd->status = MAC_SUCCESS;
    memset (txd, 0, sizeof (*txd));
    /* back to free queue */
    queue_manager_push_back( QUEUE_TX_FREE, (queue_item_t *) txd );
    break;
    
  default:
    break;
    
  }				
}

/******************************************************************************/

/*
** ============================================================================
** Private Function Definitions
** ============================================================================
*/

/* None */
