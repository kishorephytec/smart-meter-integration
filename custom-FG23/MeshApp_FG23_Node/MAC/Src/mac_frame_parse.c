/** \file mac_frame_parse.c
 *******************************************************************************
 ** \brief Provides APIs for parsing MAC frames
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
#include "sw_timer.h"
#include "phy.h"
#include "mac_config.h"
#include "mac.h"
#include "mac_defs_sec.h"
#include "ie_element_info.h"
#include "mac_defs.h"
#include "sm.h"

#if(CFG_MAC_SFTSM_ENABLED == 1)
#include "pandesc.h"
#include "sftsm.h"
#endif

#if(CFG_MAC_MPMSM_ENABLED == 1)
#include "mpmsm.h"
#endif

#include "ccasm.h"
#include "trxsm.h"
#if(CFG_MAC_SCANSM_ENABLED == 1)  
#include "scansm.h"
#endif
#if(CFG_MAC_SFTSM_ENABLED == 1)  
#include "startsm.h"
#endif
#include "macutils.h"
#include "mac_pib.h"
#include "fan_mac_ie.h"
#include "mac_frame_parse.h"

#ifdef UTEST_TRX
#include "utest_utils.h"
#endif

/*
** =============================================================================
** Private Macro definitions
** =============================================================================
*/

#define ACK_FIELD_CF0			0
#define ACK_FIELD_CF1			1
#define ACK_FIELD_DSN			2

#define ACK_FIELD_DEST_PAN		3

/*Uemsh : 10-01-2018 this thing goes to modified after sepration of mac and fan mac*/
#ifdef WISUN_FAN_MAC
#define ACK_FIELD_DEST_ADDR		2//5
#else
#define ACK_FIELD_DEST_ADDR		3//5
#endif

#ifdef WISUN_FAN_MAC
#define ACK_FIELD_SOURCE_ADDR		11//5
#else
#define ACK_FIELD_SOURCE_ADDR		11//5
#endif

#define ACK_FIELD_CRC_LO		3
#define ACK_FIELD_CRC_HI		4

/*
** =============================================================================
** Private Structures, Unions & enums Type Definitions
** =============================================================================
**/

/*
** ============================================================================
** Private Variable Definitions
** ============================================================================
*/

/*TODO: need to check if the array is being constructed properly to have all 
the contents of the phy_tx_t struct*/

//#if ( WISUN_ENET_EACK == 0 )
//#if ( WISUN_ENET_EACK == 0 )
//static uchar ack_data[sizeof(phy_tx_t) + 7 ];
//#else
//#ifdef WISUN_FAN_MAC
//static uchar ack_data[sizeof(phy_tx_t) + 50 ];
//#else
//static uchar ack_data[sizeof(phy_tx_t) + 15 ];
//#endif

//#endif

//static uint32_t rakaMACDebug = 0;
/*Umesh : 28-1-2017*/
//static uint8_t mac_frame_check = 0;
/*this varriable not used*/

/*
** =============================================================================
** Public Variables Definitions
** =============================================================================
**/

#ifdef ENET_MAC_FOR_GU
#include "hw_tmr.h"
      uint8_t set_wrong_dsn_in_ack = 0;

      uint16_t delay_in_us = 0x00;
      //p3time_t timestamp_after_delay;
      p3time_t sync_detect_time_in_us;
      p3time_t last_symbol_rx_time;
      p3time_t time_at_start_of_delay;
      p3time_t time_at_end_of_delay;
      p3time_t time_just_before_tx_start_cmd;
#endif

      int32_t rcvd_siganl_strength = 0x00000000;
      

/*
** ============================================================================
** External Variable Declarations
** ============================================================================
*/

      extern mac_tx_t ack_out;/*Umesh : 15-01-2018*//*this varribale should be in fan_mac_frame_parse.c for temp reason added here*/
#ifdef ENET_MAC_FOR_GU
      extern uint8_t time_calc_for_EBR_Rx_And_Succ_data_tx;
      extern uint8_t perform_carrier_tx;
#endif
#if(CFG_MAC_SFTSM_ENABLED == 1)        
      extern startsm_t *startsm_p;
#endif 
#if(CFG_MAC_SCANSM_ENABLED == 1)        
      extern scansm_t scansm;
#endif      
      extern mac_pib_t mac_pib;
      extern uchar aExtendedAddress[8];
      
      extern fan_mac_param_t fan_mac_params;
      extern void change_to_wait_ack_state(void);
      extern void backup_trxsm_state_for_ack_sending();

      extern void * app_bm_alloc(
          uint16_t length       
          );
      extern void app_bm_free(
          uint8_t *pMem      
          );
#ifdef MAC_ADDRESS_FILTERING_ENABLE       
      uint8_t validate_filter_mac_address(uint8_t* ieee_address);
#endif
/*
** ============================================================================
** Private Function Prototypes
** ============================================================================
*/      


/*Umesh : 28-12-2017*/
#if defined(OMRON_LPM_APP) || defined(WISUN_HEMS_SM) || defined(PANA_FOR_SINGLE_HOP_HAN) 
      void update_last_ack_LQI( int8_t lqi,mac_address_t *src);
      void save_rssi_src_addr(int8_t lqi,uint8_t mode, uint8_t *src);
#endif

/*check this functions we are using or not*/
/*
** ============================================================================
** Public Function Definitions
** ============================================================================
*/

/********** Raka 24-Feb-2020 ***********************************/
static void  parse_ieee802154_fcf(trxsm_t *trxsm,
                               phy_rx_t *pdp, 
                               mac_rx_t *mrp)
{
  uint16_t     fcf  ;
  
  fcf = *(uint16_t*)&pdp->psdu[0] ;
  /* Parse FCF Flags. */
  mrp->frame_type              = (fcf & IEEE802154_FCF_TYPE_MASK);
  mrp->security_enable         = (fcf & IEEE802154_FCF_SEC_EN) >> 3;
  mrp->frame_pending           = (fcf & IEEE802154_FCF_FRAME_PND) >> 4;
  mrp->ack_request             = (fcf & IEEE802154_FCF_ACK_REQ) >> 5;
  mrp->pan_id_compression      = (fcf & IEEE802154_FCF_PAN_ID_COMPRESSION) >> 6;
  /* bit 7 reserved */
  mrp->seqno_suppression       = (fcf & IEEE802154_FCF_SEQNO_SUPPRESSION) >> 8;
  mrp->ie_present              = (fcf & IEEE802154_FCF_IE_PRESENT) >> 9;
  mrp->dst.address_mode        = (fcf & IEEE802154_FCF_DADDR_MASK) >> 10;
  mrp->frame_ver               = (fcf & IEEE802154_FCF_VERSION) >> 12;
  mrp->src.address_mode        = (fcf & IEEE802154_FCF_SADDR_MASK) >> 14;

} /* parse_ieee802154_fcf */      
      
/*****************************************************************
          MAC Header Parsing 
******************************************************************/
      
      
      
      
      
uint8_t rssi_to_rsl (int8_t rssi_val)
{
  uint8_t i=0;
  int16_t rssi_table [255]={-174,-173,-172,-171,-170-169,-168,-167,-166,-165,-164,-163,-162,-161,-160,-159,-158,-157,-156,-155,-154,-153,
                            -152,-151,-150,-149,-148,-147,-146,-145,-144,-143,-142,-141,-140,-139,-138,-137,-136,-135,-134,-133,-132,-131,
                            -130,-129,-128,-127,-126,-125,-124,-123,-122,-121,-120,-119,-118,-117,-116,-115,-114,-113,-112,-111,-110,-109,
                            -108,-107,-106,-105,-104,-103,-102,-101,-100,-99,-98,-97,-96,-95,-94,-93,-92,-91,-90,-89,-88,-87,-86,-85,-84,
                            -83,-82,-81,-80,-79,-78,-77,-76,-75,-74,-73,-72,-71,-70,-69,-68,-67,-66,-65,-64,-63,-62,-61,-60,-59,-58,-57,
                            -56,-55,-54,-53,-52,-51,-50,-49,-48,-47,-46,-45,-44,-43,-42,-41,-40,-39,-38,-37,-36,-35,-34,-33,-32,-31,-30,
                            -29,-28,-27,-26,-25,-24,-23,-22,-21,-20,-19,-18,-17,-16,-15,-14,-13,-12,-11,-10,-9,-8,-7,-6,-5,-4,-3,-2,-1,0,
                            1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,
                            40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,
                            76,77,78,79,80};
  for(i=0;i<255;i++)
  {
    if(rssi_table[i]==rssi_val)
    {
      return (i+1);
    }
  }
  return 0xFF;
}

parse_result_t mac_frame_parse(
                               trxsm_t *trxsm,
                               phy_rx_t *pdp, 
                               mac_rx_t *mrp
                                 )
{
  //uchar frame_type = 0,frame_ver = 0;
  uint16_t pos = 0;
  uint8_t data_point = 0;
  mac_address_t *dst_ptr = NULL, *src_ptr = NULL;
  sm_event_t event;
  trxsm_ack_t ack_in = {0};   
  parse_result_t retstatus= PARSE_STORED;    
  uint16_t l_panid = mac_pib.PANId;
  /* link MAC and PHY buffers */
  mrp->pd_rxp = pdp;
  
  memcpy(&mrp->receive_frame_sfd,&pdp->sfd_rx_time,4);
  
  /* first test if we are in promiscuous mode */
  if( mac_pib.PromiscuousMode )
  {
    /* we are in promiscuous mode, so pass this up the stack */
    if( mrp == NULL_POINTER )
    {
      retstatus = PARSE_DISCARD;
      goto ENDRETURNSTATUS;
    }
    
    /* sequence number */
    mrp->sn = pdp->psdu[2];
    mrp->payload = pdp->psdu; /* need to adjust for phy length */ /*Anand*/
    mrp->payload_length = pdp->psduLength;/*Anand*/
    mrp->pd_rxp->rssi = pdp->rssi;
    mrp->src.address_mode = MAC_NO_ADDRESS;
    mrp->dst.address_mode = MAC_NO_ADDRESS;
    /*TBD Initialise these as well: type, frame_pend_*, sec_*, frame_counter = 0 */
    
    set_process_activity(AF_RCV_MSG_PENDING);
    
    goto ENDRETURNSTATUS;
  }
  
  //used for sending the rssi value to the sender with RSL-IE in ack.
  rcvd_siganl_strength = pdp->rssi;
  
  /*
  * check if this is a valid frame
  */
  if( ( pdp->psduLength < 5 ) || ( pdp->psduLength > 2047 )
     || ( (pdp->psdu[0] & 0x07) > MAC_FRAME_TYPE_MAC_COMMAND )
       || (( (pdp->psdu[1] & FRAME_VERSION_MASK) > FRAME_VERSION_2006 ) && \
         ((mac_pib.ConfigFlags & USE_2006_PRIMITIVES)))
         || (( (pdp->psdu[1] & FRAME_VERSION_MASK) > FRAME_VERSION_2011 ) && \
           ((mac_pib.ConfigFlags & USE_2011_PRIMITIVES))))
  {
    /* illegal frame */
    return PARSE_ERROR;
  }
  
  /*
  * get frame type
  */
  //frame_type = pdp->psdu[0] & MAC_FRAME_TYPE_MASK;
  //frame_ver = pdp->psdu[1] & FRAME_VERSION_MASK;
  /* check if there is space available */
  if( mrp == NULL_POINTER )
  {
    return PARSE_DISCARD;
  }
  //////////////////////////////////////////////////////////////
  
  /* set some defaults */
  mrp->frame_pending_out = mrp->frame_pending_in = 0;
  /* set pointers for address parsing */
  dst_ptr = &mrp->dst;
  src_ptr = &mrp->src;
  
  /*
  * parse addresses (starting at MAC Frame control field)
  */

  /******************************************************************************
*******************************************************************************/
#if 1 // Raka :: To Do [ 24 - Feb -2020 ] MAC header parsing integeration....

    parse_ieee802154_fcf(trxsm, pdp,mrp);
    data_point = 2;   //complete two byte for fcs bytes
    pos += data_point;
/* Sequence Number */
    if (mrp->seqno_suppression) {
        if (mrp->frame_ver != IEEE802154_VERSION_2015) {
        }
    } else { /* IEEE 802.15.4 Sequence Number Suppression */
       mrp->sn  = pdp->psdu[2]; ; 
        data_point += 1; //two byte for fcs bytes + 1 is sequence number
        pos += 1;
    }

#endif // Raka :: To Do [ 24 - Feb -2020 ] MAC header parsing integeration....
 
 /******************************************************************************
*******************************************************************************/
 
  if( (int16_t)(pos += mac_frame_parse_addresses( mrp,&pdp->psdu[data_point], dst_ptr, src_ptr ) ) < 0 )
  {
    /* parse error */
    return PARSE_ERROR;
  }
  
#if (APP_LBR_ROUTER == 1) 
#ifdef MAC_ADDRESS_FILTERING_ENABLE 
  /* Debdeep :: 09-jan-2019 */
  //Suneet :: check packt have src address 
  if((mrp->src.address_mode) != MAC_NO_ADDRESS)
{
  if((validate_filter_mac_address( mrp->src.address.ieee_address)!=0))
  {
    //    mac_free_rcv_buffer( mrp );
    return PARSE_DISCARD;
  }
}
#endif
#endif
  
//  //Raka: Check if sequence number is suppressed or not
//  if(!( pdp->psdu[1] & MAC_SEQ_NUM_SUPPRESSION)) // Raka [29-05-2017] :: validate
//  {
//    /* sequence number */
//    mrp->sn = pdp->psdu[2];
//  }
  
  /*
  * check frame type and destination address
  */
  switch( mrp->frame_type )
  {
  case MAC_FRAME_TYPE_BEACON:
    /*Uemsh : 10-01-2018 this thing goes to modified after sepration of mac and fan mac*/
#ifndef WISUN_FAN_MAC          
    /* check source PAN ID */
    /*TBD What if we are scanning? */        
#if (CFG_MAC_SCANSM_ENABLED == 1)        
    if( scansm_get_state( &scansm ) == SCANSM_STATE_NONE )
    {
      /*if the node is NOT scanning, and if the received beacon is not from 
      the same PAN that of the node, DROP the beacon. 
      Accept the beacons from the same pan if you are not scanning*/ 
      if( ( ( src_ptr->pan_id != l_panid ) ||
           ( src_ptr->address_mode == MAC_NO_ADDRESS ) ) &&
         ( l_panid != BROADCAST_PAN_ID ) )
      {
        return PARSE_DISCARD;
      }        	
    }
#endif
    
    if( mrp->frame_ver != FRAME_VERSION_2011 )
    {
      /*normal beacon*/
      break; 
    }
    else
    {
      // Hurrrrrayyy!!!!!!!!! GOT a EB.
      if( ( dst_ptr->address_mode == MAC_NO_ADDRESS)  )
      {
        dst_ptr->address_mode = MAC_SHORT_ADDRESS;
        dst_ptr->address.short_address = dst_ptr->pan_id = BROADCAST_PAN_ID;
      }
      //      goto send_ack;    
    }
#endif //#ifndef WISUN_FAN_MAC 
    
    retstatus = PARSE_ERROR;
    break;
    
  case MAC_FRAME_TYPE_MAC_COMMAND:
    retstatus = PARSE_ERROR;
    break;
    
  case MAC_FRAME_TYPE_ACK:
    /** process ACK frames*/
    /*Uemsh : 10-01-2018 this thing goes to modified after sepration of mac and fan mac*/
#ifdef WISUN_FAN_MAC
    /*When recvied  pkt is unicast and Pan id comparison is true pan id set to zero
    check dst address and self mac address  */  
    if (mrp->pan_id_compression)
    {
      if (ieeeaddr_cmp (dst_ptr->address.ieee_address, aExtendedAddress) != 0)
      {
        return PARSE_DISCARD;
      }
    }
    else
    {
      /* check PAN ID */
      if (((dst_ptr->pan_id != l_panid) &&
           (dst_ptr->pan_id != BROADCAST_PAN_ID)) 
          || (ieeeaddr_cmp (dst_ptr->address.ieee_address, aExtendedAddress) != 0))
      {
        return PARSE_DISCARD;
      }
    } 

    ack_in.dsn = mrp->sn;
    ack_in.fp = mrp->frame_pending;//pdp->psdu[ ACK_FIELD_CF0 ] & MAC_FRAME_PENDING ? 1 : 0;
    ack_in.sfd_rx_time = pdp->sfd_rx_time;
    
    /* destination address */
    if ( (mrp->frame_ver == IEEE802154_VERSION_2015) && ( mrp->dst.address_mode ) == IEEE802154_FCF_ADDR_EXT )
    {         
      ack_in.dst.address_mode = MAC_IEEE_ADDRESS;
      if(!( mrp->pan_id_compression ))
      {
        ack_in.dst.pan_id = pdp->psdu[ACK_FIELD_DEST_PAN] + \
          (((uint16_t)(pdp->psdu[ACK_FIELD_DEST_PAN+1])) << 8); //Suneet :: if pan-id is presentes in ack
        ack_in.dst.address.ieee_address  = &pdp->psdu[ACK_FIELD_DEST_ADDR]; 
        memcpy(ack_in.dest_long_addr,&pdp->psdu[ACK_FIELD_DEST_ADDR],8);//self address
      }
      else
      {
        ack_in.dst.address.ieee_address  = &pdp->psdu[ACK_FIELD_DEST_ADDR+1]; //+1 is for sequence number
        memcpy(ack_in.dest_long_addr,&pdp->psdu[ACK_FIELD_DEST_ADDR+1],8);//self address
        
      }
      ack_in.src.address_mode = MAC_IEEE_ADDRESS;
      ack_in.src.pan_id = ack_in.dst.pan_id;
      ack_in.src.address.ieee_address  = ack_in.src_long_addr;
      memcpy( ack_in.src_long_addr,&pdp->psdu[ACK_FIELD_SOURCE_ADDR],8 );//rcvd address
    }
    else
    {
      retstatus = PARSE_ERROR;
      goto ENDRETURNSTATUS;
    }         
    
    event.trigger = (sm_trigger_t) TRXSM_TRIGGER_ACK_RECEIVED;
    event.param.vector = &ack_in;
    SM_DISPATCH((sm_t *) trxsm, &event);
#endif
    break;
    
  case MAC_FRAME_TYPE_DATA:
    
#ifdef WISUN_ENET_PROFILE
    /*drop all commands and data frames, if you are doing active scan, 
    EB passive scan, EB active where you only wait for Beacons be 
    it a normal beacon or a enhanced beacon*/
#if(CFG_MAC_SCANSM_ENABLED == 1)       
    if( scansm_get_state( &scansm ) == SCANSM_STATE_ACTPASS )
    {
      return PARSE_DISCARD;
    }
#endif //#if(CFG_MAC_SCANSM_ENABLED == 1)       
#endif
    
    /* check destination address */
    switch( dst_ptr->address_mode )
    {
    case MAC_NO_ADDRESS:
      /*Uemsh : 10-01-2018 this thing goes to modified after sepration of mac and fan mac*/
#ifndef WISUN_FAN_MAC
      /* only PAN coordinator can receive frame without destination address */
      if ( 
#if (CFG_MAC_STARTSM_ENABLED == 1)                 
          ((startsm_get_flags (startsm_p) & STARTSM_FLAG_COORD_MASK) != STARTSM_FLAG_PANCOORD) ||
#endif                 
            ((src_ptr->pan_id != l_panid) || (src_ptr->address_mode == MAC_NO_ADDRESS)) 
              )
      {
        return PARSE_DISCARD;
      }
#endif
      break;
      
    case MAC_SHORT_ADDRESS:
      /* check PAN ID */
      if (((dst_ptr->pan_id != l_panid) &&
           (dst_ptr->pan_id != BROADCAST_PAN_ID)) || ((dst_ptr->address.short_address != BROADCAST_SHORT_ADDRESS) &&
                                                      (dst_ptr->address.short_address != mac_pib.ShortAddress)))
      {
        return PARSE_DISCARD;
      }
      break;
      
    case MAC_IEEE_ADDRESS:
      /*When recvied  pkt is unicast and Pan id comparison is true pan id set to zero
      check dst address and self mac address  */  
      if (mrp->pan_id_compression)
      {
        if (ieeeaddr_cmp (dst_ptr->address.ieee_address, aExtendedAddress) != 0)
        {
          return PARSE_DISCARD;
        }
      }
      else
      {
        /* check PAN ID */
        if (((dst_ptr->pan_id != l_panid) &&
             (dst_ptr->pan_id != BROADCAST_PAN_ID)) 
            || (ieeeaddr_cmp (dst_ptr->address.ieee_address, aExtendedAddress) != 0))
        {
          return PARSE_DISCARD;
        }
      }    
      
      break;
      
    default:
      /* parse error -- internal error, actually */
      return PARSE_ERROR;
    }
    
    break;      /*case MAC_FRAME_TYPE_DATA*/
    
  default:
    /* parse error */
    return PARSE_ERROR;
  } // Raka ::  switch( frame_type )
  
  /* save incoming frame pending bit */
  if( mrp->frame_pending )
  {
    mrp->frame_pending_in = 1;
  }
  
  /* skip security fields */
  if( mrp->security_enable )
  {
    mrp->auxiliary_secoffset_index = pos;  //For security offset 
    /* length of Security Control (1) and Frame Counter (4)
    and Key Identifier (0..9) */
    switch( (pdp->psdu[pos] >> 3) & 0x3 )
    {
    case 1:
      pos += 1 + 4 + 1;
      break;
      
    case 2:
      pos += 1 + 4 + 5;
      break;
      
    case 3:
      pos += 1 + 4 + 9;
      break;
    default:
      pos += 1 + 4;
      break;
    }
  }
  mrp->header_ie_offset = pos;
ENDRETURNSTATUS:
  return retstatus; 
}

/******************************************************************************/

int mac_frame_parse_addresses(
                                mac_rx_t *mrp,
                                uchar *data, 
                                mac_address_t *dst,
                                mac_address_t *src
                              )
{
  
#if 0 // Raka .. To do this function will change as the address parsing is differnt
    int offset = 3; /* start after Frame header and DSN */

    //Raka: Check if sequence number is suppressed or not
    if( data[1] & MAC_SEQ_NUM_SUPPRESSION)
    {
      offset = 2;
    }
    
    /* initialise */
    dst->address_mode = src->address_mode = MAC_NO_ADDRESS;

    /* destination address */
    switch( data[1] & MAC_DST_ADDRESS_MASK )
    {
    case MAC_NO_DST_ADDRESS:
        //dst->address_mode = MAC_NO_ADDRESS;
        dst->pan_id = 0;
        offset += 0;
        break;

    case MAC_SHORT_DST_ADDRESS:
        dst->address_mode = MAC_SHORT_ADDRESS;
        dst->pan_id = data[3] + (((uint16_t)(data[4])) << 8); 
        dst->address.short_address = data[5] + (((uint16_t)(data[6])) << 8);
        offset += 2 + 2;
        break;

    case MAC_IEEE_DST_ADDRESS:
        dst->address_mode = MAC_IEEE_ADDRESS;
        /*When recvied  pkt is unicast and Pan id comparison is true pan id set to zero  */
        if( data[0] & MAC_INTRA_PAN )
        {
          dst->pan_id=0;
          // Raka .. Commneted this line.. do not know why we are checking 
          //mac_frame_check == MAC_FRAME_TYPE_ACK [ 05-dec-2017] testing with Cisco
          
          //if((mac_frame_check == MAC_FRAME_TYPE_ACK) || (data[1] & MAC_SEQ_NUM_SUPPRESSION))
          
          if (data[1] & MAC_SEQ_NUM_SUPPRESSION)
            dst->address.ieee_address = &data[2];
          else
            dst->address.ieee_address = &data[3];  
          
          offset += 8;
        }
        else
        {
          dst->pan_id = data[3] + (((uint16_t)(data[4])) << 8);
          dst->address.ieee_address  = &data[5];
          offset += 2 + 8;
        }
        break;

    default:
        return -1;
    }

    /* Source PAN ID is only present if source address is present */
    if( (data[1] & MAC_SRC_ADDRESS_MASK) != MAC_NO_ADDRESS)
    {
     /* check PAN ID Compression bit */     
#if ( WISUN_ENET_FRAME_FORMAT == 0 )
        if( data[0] & MAC_INTRA_PAN )
#else
        if( !( data[0] & MAC_INTRA_PAN ))
#endif    
        {
            src->pan_id = dst->pan_id;
        }
        else
        {
            src->pan_id = data[offset] + (((uint16_t)(data[offset+1])) << 8); 
            offset += 2;
        }
    }

    /* source address */
    switch (data[1] & MAC_SRC_ADDRESS_MASK)
    {
    case MAC_NO_SRC_ADDRESS:
        //src->address_mode = MAC_NO_ADDRESS;
        break;

    case MAC_SHORT_SRC_ADDRESS:
        src->address_mode = MAC_SHORT_ADDRESS;
        src->address.short_address = data[offset] + (((uint16_t)(data[offset+1])) << 8); 
        offset += 2;
        break;

    case MAC_IEEE_SRC_ADDRESS:
        src->address_mode = MAC_IEEE_ADDRESS;
        src->address.ieee_address = &data[offset];
        offset += 8;
        break;

    default:
        return -1;
    }

    return offset;
    
    
#else
    gboolean                dstPanPresent = FALSE;
    gboolean                srcPanPresent = FALSE;
    uint32_t offset = 0;  // Raka To Do : initialize is as per the input packet parsing ...as this is the return value of the function. 
    //ieee802154_frame_info *packet  = temp_pkt_for_compilation;
    
    /*
     * ADDRESSING FIELDS
     */
    
    if (mrp->dst.address_mode == IEEE802154_FCF_ADDR_RESERVED) {
        /* Invalid Destination Address Mode. Abort Dissection. */
        return -1;
    }

    if (mrp->src.address_mode == IEEE802154_FCF_ADDR_RESERVED) {
        /* Invalid Source Address Mode. Abort Dissection. */
        return  -1;
    }

    if (mrp->frame_ver == IEEE802154_VERSION_RESERVED) {
        /* Unknown Frame Version. Abort Dissection. */
        return  -1;
    }
    else if ((mrp->frame_ver == IEEE802154_VERSION_2003) ||  /* For Frame Version 0b00 and */
             (mrp->frame_ver == IEEE802154_VERSION_2006))  { /* 0b01 effect defined in section 7.2.1.5 */

        if ((mrp->dst.address_mode != IEEE802154_FCF_ADDR_NONE) && /* if both destination and source */
            (mrp->src.address_mode != IEEE802154_FCF_ADDR_NONE)) { /* addressing information is present */
            if (mrp->pan_id_compression == 1) { /* PAN IDs are identical */
                dstPanPresent = TRUE;
                srcPanPresent = FALSE; /* source PAN ID is omitted */
            }
            else { /* PAN IDs are different, both shall be included in the frame */
                dstPanPresent = TRUE;
                srcPanPresent = TRUE;
            }
        }
        else {
            if (mrp->pan_id_compression == 1) { /* all remaining cases pan_id_compression must be zero */
                return -1;
            }
            else {
                /* only either the destination or the source addressing information is present */
                if ((mrp->dst.address_mode != IEEE802154_FCF_ADDR_NONE) &&        /*   Present   */
                    (mrp->src.address_mode == IEEE802154_FCF_ADDR_NONE)) {        /* Not Present */
                    dstPanPresent = TRUE;
                    srcPanPresent = FALSE;
                }
                else if ((mrp->dst.address_mode == IEEE802154_FCF_ADDR_NONE) &&   /* Not Present */
                         (mrp->src.address_mode != IEEE802154_FCF_ADDR_NONE)) {   /*   Present   */
                    dstPanPresent = FALSE;
                    srcPanPresent = TRUE;
                }
                else if ((mrp->dst.address_mode == IEEE802154_FCF_ADDR_NONE) &&   /* Not Present */
                         (mrp->src.address_mode == IEEE802154_FCF_ADDR_NONE)) {   /* Not Present */
                    dstPanPresent = FALSE;
                    srcPanPresent = FALSE;
                }
                else {
                    return  -1;
                }
            }
        }
    }
    else if (mrp->frame_ver == IEEE802154_VERSION_2015) {
        /* for Frame Version 0b10 PAN Id Compression only applies to these frame types */
        if ((mrp->frame_type == IEEE802154_FCF_BEACON) ||
            (mrp->frame_type == IEEE802154_FCF_DATA)   ||
            (mrp->frame_type == IEEE802154_FCF_ACK)    ||
            (mrp->frame_type == IEEE802154_FCF_CMD)       ) {

            /* Implements Table 7-6 of IEEE 802.15.4-2015
             *
             *      Destination Address  Source Address  Destination PAN ID  Source PAN ID   PAN ID Compression
             *-------------------------------------------------------------------------------------------------
             *  1.  Not Present          Not Present     Not Present         Not Present     0
             *  2.  Not Present          Not Present     Present             Not Present     1
             *  3.  Present              Not Present     Present             Not Present     0
             *  4.  Present              Not Present     Not Present         Not Present     1
             *
             *  5.  Not Present          Present         Not Present         Present         0
             *  6.  Not Present          Present         Not Present         Not Present     1
             *
             *  7.  Extended             Extended        Present             Not Present     0
             *  8.  Extended             Extended        Not Present         Not Present     1
             *
             *  9.  Short                Short           Present             Present         0
             * 10.  Short                Extended        Present             Present         0
             * 11.  Extended             Short           Present             Present         0
             *
             * 12.  Short                Extended        Present             Not Present     1
             * 13.  Extended             Short           Present             Not Present     1
             * 14.  Short                Short           Present             Not Present     1
             */

            /* Row 1 */
            if ((mrp->dst.address_mode == IEEE802154_FCF_ADDR_NONE) &&      /* Not Present */
                (mrp->src.address_mode == IEEE802154_FCF_ADDR_NONE) &&      /* Not Present */
                (mrp->pan_id_compression == 0)) {
                        dstPanPresent = FALSE;
                        srcPanPresent = FALSE;
            }
            /* Row 2 */
            else if ((mrp->dst.address_mode == IEEE802154_FCF_ADDR_NONE) && /* Not Present */
                     (mrp->src.address_mode == IEEE802154_FCF_ADDR_NONE) && /* Not Present */
                     (mrp->pan_id_compression == 1)) {
                        dstPanPresent = TRUE;
                        srcPanPresent = FALSE;
            }
            /* Row 3 */
            else if ((mrp->dst.address_mode != IEEE802154_FCF_ADDR_NONE) && /*  Present    */
                     (mrp->src.address_mode == IEEE802154_FCF_ADDR_NONE) && /* Not Present */
                     (mrp->pan_id_compression == 0)) {
                        dstPanPresent = TRUE;
                        srcPanPresent = FALSE;
            }
            /* Row 4 */
            else if ((mrp->dst.address_mode != IEEE802154_FCF_ADDR_NONE) && /*  Present    */
                     (mrp->src.address_mode == IEEE802154_FCF_ADDR_NONE) && /* Not Present */
                     (mrp->pan_id_compression == 1)) {
                        dstPanPresent = FALSE;
                        srcPanPresent = FALSE;
            }
            /* Row 5 */
            else if ((mrp->dst.address_mode == IEEE802154_FCF_ADDR_NONE) && /* Not Present */
                     (mrp->src.address_mode != IEEE802154_FCF_ADDR_NONE) && /*  Present    */
                     (mrp->pan_id_compression == 0)) {
                        dstPanPresent = FALSE;
                        srcPanPresent = TRUE;
            }
            /* Row 6 */
            else if ((mrp->dst.address_mode == IEEE802154_FCF_ADDR_NONE) && /* Not Present */
                     (mrp->src.address_mode != IEEE802154_FCF_ADDR_NONE) && /*  Present    */
                     (mrp->pan_id_compression == 1)) {
                        dstPanPresent = FALSE;
                        srcPanPresent = FALSE;
            }
            /* Row 7 */
            else if ((mrp->dst.address_mode == IEEE802154_FCF_ADDR_EXT) && /*  Extended    */
                     (mrp->src.address_mode == IEEE802154_FCF_ADDR_EXT) && /*  Extended    */
                     (mrp->pan_id_compression == 0)) {
                        dstPanPresent = TRUE;
                        srcPanPresent = FALSE;
            }
            /* Row 8 */
            else if ((mrp->dst.address_mode == IEEE802154_FCF_ADDR_EXT) && /*  Extended    */
                     (mrp->src.address_mode == IEEE802154_FCF_ADDR_EXT) && /*  Extended    */
                     (mrp->pan_id_compression == 1)) {
                        dstPanPresent = FALSE;
                        srcPanPresent = FALSE;
            }
            /* Row 9 */
            else if ((mrp->dst.address_mode == IEEE802154_FCF_ADDR_SHORT) && /*  Short     */
                     (mrp->src.address_mode == IEEE802154_FCF_ADDR_SHORT) && /*  Short     */
                     (mrp->pan_id_compression == 0)) {
                        dstPanPresent = TRUE;
                        srcPanPresent = TRUE;
            }
            /* Row 10 */
            else if ((mrp->dst.address_mode == IEEE802154_FCF_ADDR_SHORT) && /*  Short    */
                     (mrp->src.address_mode == IEEE802154_FCF_ADDR_EXT) &&   /*  Extended */
                     (mrp->pan_id_compression == 0)) {
                        dstPanPresent = TRUE;
                        srcPanPresent = TRUE;
            }
            /* Row 11 */
            else if ((mrp->dst.address_mode == IEEE802154_FCF_ADDR_EXT)   &&   /*  Extended */
                     (mrp->src.address_mode == IEEE802154_FCF_ADDR_SHORT) &&   /*  Short    */
                     (mrp->pan_id_compression == 0)) {
                        dstPanPresent = TRUE;
                        srcPanPresent = TRUE;
            }
            /* Row 12 */
            else if ((mrp->dst.address_mode == IEEE802154_FCF_ADDR_SHORT) &&   /*  Short    */
                     (mrp->src.address_mode == IEEE802154_FCF_ADDR_EXT)   &&   /*  Extended */
                     (mrp->pan_id_compression == 1)) {
                        dstPanPresent = TRUE;
                        srcPanPresent = FALSE;
            }
            /* Row 13 */
            else if ((mrp->dst.address_mode == IEEE802154_FCF_ADDR_EXT)   &&   /*  Extended */
                     (mrp->src.address_mode == IEEE802154_FCF_ADDR_SHORT) &&   /*  Short    */
                     (mrp->pan_id_compression == 1)) {
                        dstPanPresent = TRUE;
                        srcPanPresent = FALSE;
            }
            /* Row 14 */
            else if ((mrp->dst.address_mode == IEEE802154_FCF_ADDR_SHORT) &&   /*  Short    */
                     (mrp->src.address_mode == IEEE802154_FCF_ADDR_SHORT) &&   /*  Short    */
                     (mrp->pan_id_compression == 1)) {
                        dstPanPresent = TRUE;
                        srcPanPresent = FALSE;
            }
            else {
                return -1;
            }
        }
        else { /* Frame Type is neither Beacon, Data, Ack, nor Command: PAN ID Compression is not used */
            dstPanPresent = FALSE; /* no PAN ID will */
            srcPanPresent = FALSE; /* be present     */
        }
    }
    else {
        /* Unknown Frame Version. Abort Dissection. */
        return  -1;
    }
    
    
    
    /*
     * Addressing Fields
     */

    /* Destination PAN Id */
    if (dstPanPresent) {
        dst->pan_id = data[offset] + (((uint16_t)(data[offset])) << 8); 
        offset += 2;
    }

    /* Destination Address  */
    if (mrp->dst.address_mode == IEEE802154_FCF_ADDR_SHORT) {
        
        /* Get the address. */
        dst->address.short_address = data[offset] + (((uint16_t)(data[offset])) << 8); 
        offset += 2;
    }
    else if (mrp->dst.address_mode == IEEE802154_FCF_ADDR_EXT) {
        /* Get the address */
        dst->address.ieee_address = &data[offset];
        offset += 8;
    }

    /* Source PAN Id */
    if (srcPanPresent) {
        src->pan_id = data[offset] + (((uint16_t)(data[offset+1])) << 8);; 
        offset += 2;
    }
    else {
        if (dstPanPresent) {
            src->pan_id = dst->pan_id;
        }
        else {
            src->pan_id = IEEE802154_BCAST_PAN;
        }
    }
    

    /* Source Address */
    if (mrp->src.address_mode == IEEE802154_FCF_ADDR_SHORT) {
        /* Get the address. */
        src->address.short_address = data[offset] + (((uint16_t)(data[offset+1])) << 8); 
        offset += 2;
    }
    else if (mrp->src.address_mode == IEEE802154_FCF_ADDR_EXT) {
        /* Get the address. */
        src->address.ieee_address = &data[offset];
        /* Copy and convert the address to network byte order. */
        offset += 8;
    }

    return  offset;
    
#endif
    
    
}

/* Debdeep :: Only for debugging TACK */                  
//void high_gpio ();
//void low_gpio ();
//uint64_t get_time_now_64 (void);
//static uint64_t packet_rcv_time[40] = {0};
//static uint64_t ack_send_time[40] = {0};
//uint32_t stamp_index = 0;
//
//void take_time_stamp_if_ack_required (uint8_t data)
//{
////  if ((data & MAC_ACK_REQUIRED) && (data & 0x08))
//  if (data & MAC_ACK_REQUIRED)
//  {
//    high_gpio ();
//    packet_rcv_time[stamp_index] = get_time_now_64();
//  }
//}
//
//void take_time_stamp_if_packet_is_ack (uint8_t data, uint64_t stamp)
//{
////  if ((data & 0x02) && (data & 0x08))
//  if (data & 0x02)
//  {
//    low_gpio ();
//    ack_send_time[stamp_index++] = stamp;//get_time_now_64();
//  }
//  
//}

/******************************************************************************/

/*
** =============================================================================
** Private Macro definitions
** =============================================================================
*/
/*this moved to mac_frame_parse.h file b"se itrequired in both files this 1 and mac_frame_parse.c file*/
//#ifdef WISUN_FAN_MAC
//#define ACK_FIELD_UTIE		        18
//#define MAX_ACK_HDR_IE_LIST_SIZE	50
//#endif

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

#ifdef WISUN_FAN_MAC
static uint8_t p_ufsi_mem[3]={0};
//static mac_tx_t ack_out = {
//  .p_ufsi = p_ufsi_mem
//};
#endif

/*
** ============================================================================
** Private Function Prototypes
** ============================================================================
*/

/* None */

/*
** ============================================================================
** External Variable Declarations
** ============================================================================
*/

/* None */

/*
** ============================================================================
** External Function Declarations
** ============================================================================
*/

#ifdef WISUN_FAN_MAC
      extern void update_etx_rx_count(uint8_t*, uint8_t*);
//      extern void update_ack_received(uint8_t *ack_rcvd_addr, uint8_t *self_addr);
#endif

/*
** =============================================================================
** Public Variables Definitions
** =============================================================================
**/
#ifdef WISUN_FAN_MAC
mac_tx_t ack_out = {
  .p_ufsi = p_ufsi_mem
};
#endif
/*
** ============================================================================
** Public Function Definitions
** ============================================================================
*/

/* None */

/*
** ============================================================================
** Private Function Definitions
** ============================================================================
*/

#ifdef WISUN_FAN_MAC
//static void add_ie_list_in_fan_ack_pkt(fan_mac_param_t *fan_mac_params)
void add_ie_list_in_fan_ack_pkt(fan_mac_param_t *fan_mac_params)
{
    fan_mac_params->type = FAN_ACK;
    fan_mac_params->hdr_ies_cnt = 1;
    fan_mac_params->hdr_ie_list[0] = WH_IE;/*0x2A*/
    fan_mac_params->hdr_ie_list[1] = 0x03;//Number of wisun sub ids count for  WISUN FAN ACK
    fan_mac_params->hdr_ie_list[2] = WH_IE_SUBID_UTT_IE_SHORT;/*0x01*/
    fan_mac_params->hdr_ie_list[3] = WH_IE_SUBID_BT_IE_SHORT;/*0x02*/
    fan_mac_params->hdr_ie_list[4] = WH_IE_SUBID_RSL_IE_SHORT;/*0x04*/
    fan_mac_params->pld_ies_cnt = 0;//No payload ies count for WISUN FAN ACK
}
#endif


