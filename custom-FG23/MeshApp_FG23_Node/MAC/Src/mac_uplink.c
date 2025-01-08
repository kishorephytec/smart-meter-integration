/** \file mac_uplink.c
 *******************************************************************************
 ** \brief Processes the packets received from the UART
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
#include "mac_pib.h"
#include "sm.h"

#if(CFG_MAC_SFTSM_ENABLED == 1)  
#include "sftsm.h"
#include "pandesc.h"
#endif

#if(CFG_MAC_MPMSM_ENABLED == 1)
#include "mpmsm.h"
#endif

#include "ccasm.h"
#include "trxsm.h"
#include "mac_frame_parse.h"
#if(CFG_MAC_SCANSM_ENABLED == 1)  
#include "scansm.h"
#endif
#include "fan_mac_ie.h"
#include "macutils.h"
#include "mac_uplink.h"

#ifdef MAC_CFG_SECURITY_ENABLED
#include "mac_security.h"
#endif 
   
/*
** =============================================================================
** Private Macro definitions
** =============================================================================
*/
#define DATA_CONFIRM_SIZE           3   /* primitive, handle, status  */
#define DATA_INDICATION_SIZE        0   /* primitive, dst address mode, .... */
#define PURGE_CONFIRM_SIZE          3   /* primitive, handle, status */
#define ASSOCIATE_CONFIRM_SIZE      4   /* primitive, short address and status */
#define ASSOCIATE_INDICATION_SIZE   11  /* primitive, IEEE address, capability info, flags */
#define BEACON_NOTIFY_MAX_LENGTH    (1+(1+2+8+1+2+1+1+3+1)+1+(7*8)+1+aMaxBeaconPayloadLength)
#define DISASSOCIATE_INDICATION_SIZE   11  /* primitive, IEEE address, reason, flags */
#define ORPHAN_INDICATION_SIZE      (1+8+1) /* primtive, address and security stuff ) */
#define DISASSOCIATE_CONFIRM_SIZE   2   /* primitive and status */
#define POLL_CONFIRM_SIZE           2   /* primitive and status */
#define RESET_CONFIRM_SIZE          2   /* primitive and status */
#define RX_ENABLE_CONFIRM_SIZE      2   /* primitive and status */
#define SCAN_CONFIRM_SIZE           (2+1+8+1) /* primitive and status, type,
                                    unscanned channels, result list size */
#define START_CONFIRM_SIZE          2   /* primitive and status */
#define SYNC_LOSS_INDICATION_SIZE   6   /* primitive, loss reason, PAN ID, LogicalChannel and ChannelPage */
#define GET_CONFIRM_SIZE            3   /* primitive, status, id */
#define SET_CONFIRM_SIZE            3   /* primitive, status, id */
#define COMM_STATUS_INDICATION_SIZE (1+2+1+1+1) /* primitive, panid, src address mode, dest addr mode, status */
#define GTS_INDICATION_SIZE         (1+2+1+1) /* prmitive, address, gts characteristics and security */
#define GTS_CONFIRM_SIZE            (1+1+1)
#define SCAN_RESULT_MAX_SIZE		1000

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

/*None*/

/*
** =============================================================================
** Private Variables Definitions
** =============================================================================
**/

/* None */

/*
** ============================================================================
** External Variable Declarations
** ============================================================================
*/
        extern mac_pib_t mac_pib;
#if(CFG_MAC_SCANSM_ENABLED == 1)  
        extern scansm_t scansm;
#endif        
        extern trxsm_t* trxsm_p;

mac_nbr_descriptor_t* get_nbr_desc_from_addr (uint8_t* p_nbr_addr);
extern void * app_bm_alloc(
    uint16_t length//base_t length       
    );

/*
** ============================================================================
** Private Function Prototypes
** ============================================================================
*/
        static mac_status_t mac_prim_append_address( uchar **, mac_address_t * );
#if( (CGF_MLME_BEACON_NOTIFY_IND == 1) || (CFG_MLME_SCAN_REQ_CONF == 1 ))        
        static void mac_prim_append_pandesclist( uchar **, pandesc_t *, uchar );
#endif

#ifdef MAC_CFG_SECURITY_ENABLED
	static uchar *append_sec_params( uchar *buf, security_params_t *sec_params, uchar length );
#endif

/*
** ============================================================================
** Public Function Definitions
** ============================================================================
*/        

/* create and send the mcps_data_confirm primitive to the NHL */
uchar send_mcps_data_confirm( uchar msdu_handle, uchar status, ulong timestamp,uchar nb )
{
    msg_t *msg = NULL;
    uchar *buf = NULL;
    uint16_t length = DATA_CONFIRM_SIZE+1; // for nb
	
    if (mac_pib.ConfigFlags & USE_2006_PRIMITIVES)
    {
        /* Make buffer big enough for Time Stamp */
       length += 3;
    }
    else if (mac_pib.ConfigFlags & USE_2011_PRIMITIVES)
    {
        /* Make buffer big enough for Time Stamp */
        length += 4;
    }

    msg = allocate_mac_2_nhle_msg( length );
//#if (PRINT_DEBUG_LEVEL==ERROR_DEBUG)
//    stack_print_debug ("1M tx_confirm %d %p\n", length, msg);
//#endif     

    if( msg != NULL_POINTER )
    {
        buf = (uchar *) msg->data;
/*Uemsh : 11-01-2018 this thing goes to modified after sepration of mac and fan mac*/
#ifdef WISUN_FAN_MAC
        *buf++ = FAN_MAC_MCPS_DATA_CONFIRM;
#else
        *buf++ = MAC_MCPS_DATA_CONFIRM;
#endif        
        *buf++ = msdu_handle;
        *buf++ = status;        
        
        if (mac_pib.ConfigFlags & USE_2011_PRIMITIVES)
        {
            *buf++ = nb;	
        }

        /* Check if we are using 802.15.4-2006 primtives */
        if( mac_pib.ConfigFlags & ( USE_2006_PRIMITIVES|USE_2011_PRIMITIVES ) )
        {			
            *buf++ = timestamp & 0xff;
            *buf++ = (timestamp >> 8) & 0xff;
            *buf++ = (timestamp >> 16) & 0xff;            
        }

        send_mcps_2_nhle( msg );
        return 1;
    }
    return 0;
}

/******************************************************************************/
#if APP_LBR_ROUTER
uchar send_mcps_ack_indication(
                                mac_rx_t *rxmsg 
                                )
{
  msg_t *msg = NULL;
  uchar *buf = NULL;
  phy_rx_t phy_rx_pkt = {0};
  memcpy((uint8_t*)&phy_rx_pkt,(uint8_t*)(rxmsg->pd_rxp), sizeof( phy_rx_pkt ));
  
  msg = (msg_t*)rxmsg->pd_rxp;
  buf = msg->data;
  
  *buf++ = FAN_MAC_ACK_INDICATION;
  mac_prim_append_address( &buf, &rxmsg->src );
  mac_prim_append_address( &buf, &rxmsg->dst );
  *buf++ = rxmsg->sn;
  mac_nbr_descriptor_t* p_nbr_desc = get_nbr_desc_from_addr ( rxmsg->src.address.ieee_address);
  
  if(p_nbr_desc != NULL)
  {
    *buf++ = p_nbr_desc->rsl;
  }
  *buf++ = rxmsg->security_status;
  msg->data_length = (buf - msg->data);
  send_mcps_2_nhle( msg );
  return 1;
}

void send_mcps_no_ack_indication (void)
{
  msg_t *msg =  (msg_t *)app_bm_alloc (sizeof (msg_t));
  msg->data[0] = FAN_MAC_NO_ACK_INDICATION;
  msg->data_length = 1;
  send_mcps_2_nhle( msg );
}
#endif
uchar send_mcps_data_indication(
                                mac_rx_t *rxmsg 
                                )
{
    msg_t *msg = NULL;
    uchar *buf = NULL;
    uint16_t msdu_length = 0;
    uint16_t length = rxmsg->pd_rxp->psduLength + 3;
    phy_rx_t phy_rx_pkt = {0};
    uint8_t src_address[8] = {0};
    
#ifdef MAC_CFG_SECURITY_ENABLED
	uchar key_id_length = get_key_identifier_length(rxmsg->sec_param.key_id_mode);
#endif

    /* allocate an SPI buffer for the message to the network layer
    * The format of the incoming message needs modifying, but
    * the space required is the same as that of the incoming message
    * with the possible addition of 2 bytes for the Source PAN ID if the Intra PAN bit is set.
    */
    /*back up the PHY related info in the buffer before it gets overwritten*/
    memcpy((uint8_t*)&phy_rx_pkt,(uint8_t*)(rxmsg->pd_rxp), sizeof( phy_rx_pkt ));
    
    /*TBD check allocation lengths */
    if ( mac_pib.ConfigFlags & USE_2006_PRIMITIVES )
    {
        /* add space for msdu Length (1), LQI (1), DSN (1), Timestamp (3), Security Data (1) + key_id_length */
        //msg = allocate_mac_2_nhle_msg( rxmsg->pd_rxp->psduLength + 1 + 1 + 1 + 1 +
		length += 4;

		#ifdef MAC_CFG_SECURITY_ENABLED
			length += key_id_length; 
		#endif

    }
    else if ( mac_pib.ConfigFlags & USE_2011_PRIMITIVES )
    {
        //msg = allocate_mac_2_nhle_msg( rxmsg->pd_rxp->psduLength + 2 + 1 + 1 + 1 + 20 );
		length += 25;
    }

    //msg = allocate_mac_2_nhle_msg( length );

    /* check if allocation was successful */
   // if ( msg == NULL_POINTER )
    //{
   //     return 1;//TBD the data gets dropped before even initimating to the NHLE as a result of buffer crunch
    //}
    msg = (msg_t*)rxmsg->pd_rxp;
    buf = msg->data;
    /*back up LQI*/
    //psdu_lqi = (uint8_t)rxmsg->pd_rxp->psduLinkQuality;
    //sfd_rx_time = rxmsg->pd_rxp->sfd_rx_time;
    
    //buf = (uint8_t*)&(rxmsg->pd_rxp->sfd_rx_time);
/*Uemsh : 11-01-2018 this thing goes to modified after sepration of mac and fan mac*/
#ifdef WISUN_FAN_MAC
        *buf++ = FAN_MAC_MCPS_DATA_INDICATION;
#else
        *buf++ = MAC_MCPS_DATA_INDICATION;
#endif 

	/* set MSDU length */
	msdu_length = rxmsg->payload_length;
        memcpy (src_address, rxmsg->src.address.ieee_address, 8);

    if( !( mac_pib.PromiscuousMode ) )
    {
	    /* store the source and destination addresses into the primitive buffer */
	    mac_prim_append_address( &buf, &rxmsg->src );
	    mac_prim_append_address( &buf, &rxmsg->dst );

	    /* add MSDU length and MSDU */
	    //*buf++ = msdu_length;
	    put_ushort(buf,msdu_length);
	    buf += 0x02;
    
	    memcpy( buf, rxmsg->payload, msdu_length );
/*Uemsh : 11-01-2018 this thing goes to modified after sepration of mac and fan mac*/
//#ifdef WISUN_FAN_MAC
//#ifdef TEST_CHOP
//            if( rxmsg->payload[4] != (uint8_t)trxsm_p->current_us_channel )
//            {
//              utu_timestamp( DATA_FRAME_RX, rxmsg->payload[0] );
//              utu_timestamp( DATA_FRAME_RX, rxmsg->payload[4] );
//              utu_timestamp( DATA_FRAME_RX, rxmsg->payload[8] );
//              
//              if( rxmsg->payload[0] == (uint8_t)trxsm_p->current_us_channel )
//              {
//                rxed_in_next_chan++;
//              }
//              else if (  rxmsg->payload[8] == (uint8_t)trxsm_p->current_us_channel )
//              {
//                rxed_in_prev_chan++;
//              }
//              else
//              {
//                rxed_in_unknown_chan++;
//              }
//              
//              if(  ++synhro_problem > 21)
//              {
//                synhro_problem = 0;
//              }
//              
//            }
//#endif
//#endif            

	    /* step over the data in the dest buffer */
	    buf += msdu_length;    
    

	    /* add link quality (last byte of PD message ( not included in length ) */
	    *buf++ = (uint8_t)(phy_rx_pkt.rssi); //(uint8_t)rxmsg->pd_rxp->psduLinkQuality;
	

	    /* check if we are using 802.15.4-2006 primtives */
	    if( mac_pib.ConfigFlags & ( USE_2006_PRIMITIVES|USE_2011_PRIMITIVES ) )
	    {
	        /* add DSN */
	        *buf++ = rxmsg->sn;
                 *buf++  = rxmsg->pld_ies_present;
	        /* add timestamp */
		*buf++ = phy_rx_pkt.sfd_rx_time & 0xff; //rxmsg->pd_rxp->sfd_rx_time & 0xff;
	        *buf++ = (phy_rx_pkt.sfd_rx_time >> 8) & 0xff;
	        *buf++ = (phy_rx_pkt.sfd_rx_time >> 16) & 0xff;
			
	        
			#ifdef MAC_CFG_SECURITY_ENABLED
				buf = append_sec_params( buf, &rxmsg->sec_param, key_id_length );
				buf++;
			#else
				*buf++ = 0;
			#endif
	        
	    }
	    else
	    {
	        /* 802.15.4-2003 - SecurityUse & ACL entry */
	        *buf++ = (MAC_SECURITY_NOT_IN_ACL << 1);
	    }
    }
    else
    {
    	//*buf++ = MAC_MCPS_DATA_INDICATION;
    	//put_ulong(buf,0);
    	/**buf++ = 0x0;
    	*buf++ = 0x0;
    	*buf++ = 0x0;
    	*buf++ = 0x0;*/

    	memset(buf,0x0,4);
		buf += 4;

//    	 /* set MSDU length */
//	    msdu_length -= phy_rx_pkt.FCSLength & 0x7FFF; //already cut when chaeck header ie 

	    /* add MSDU length and MSDU */
	    //*buf++ = msdu_length;
	    put_ushort(buf,msdu_length);
	    buf += 0x02;
	               
	    put_ushort(buf,(phy_rx_pkt.FCSLength));
	    buf += 0x02;
	    
	    memcpy(buf,&(phy_rx_pkt.rssi),1);
	    buf += 0x01;
	    
            memcpy(buf,&(phy_rx_pkt.lqi),1);
	    buf += 0x01;
	    
	    memcpy(buf,&(phy_rx_pkt.channel),4);
	    buf += 0x04;
	    
	    memcpy( buf,rxmsg->payload, msdu_length );

	    /* step over the data in the dest buffer */
	    buf += msdu_length; 
	    
	    
	    *buf++ = rxmsg->sn;
        /* add timestamp */
//        *buf++ = phy_rx_pkt.sfd_rx_time & 0xFF;// rxmsg->pd_rxp->sfd_rx_time & 0xff;
//        *buf++ = (phy_rx_pkt.sfd_rx_time >> 8) & 0xff;
//        *buf++ = (phy_rx_pkt.sfd_rx_time >> 16) & 0xff;
            /*Suneet add time stamp when sync detect on radio*/
            memcpy(buf,&phy_rx_pkt.complete_rx_hw_time,sizeof( phy_rx_pkt.complete_rx_hw_time));
            buf += sizeof( phy_rx_pkt.complete_rx_hw_time);
    }

    /* data length is current position minus start of buffer */
    msg->data_length = (buf - msg->data);
#if APP_LBR_ROUTER    
    mac_nbr_descriptor_t* p_nbr_desc = get_nbr_desc_from_addr (src_address);
    if(p_nbr_desc != NULL)
    {
//#if ((PRINT_DEBUG_LEVEL==ERROR_DEBUG)||(PRINT_DEBUG_LEVEL==CONSOLE_ERROR_DEBUG))
//      stack_print_debug ("SN = %d sent to Upper layer: ", p_nbr_desc->incoming_pkt_seq_no);
//      print_mac_address (src_address);
//#endif
      p_nbr_desc->packet_status = INCOMING_PKT_SEND_TO_UPL;
    }
#endif    
    send_mcps_2_nhle( msg );

    return 1;
}

/******************************************************************************/
#if(CFG_MCPS_PURGE_REQ_CONF == 1)
/* create and send a purge confirm to the NHL*/
void send_mcps_purge_confirm(
								 uchar msdu_handle,
								 uchar status
                             )
{
    msg_t *msg = NULL;
    uchar *buf = NULL;

    msg = allocate_mac_2_nhle_msg( PURGE_CONFIRM_SIZE );

    if( msg != NULL_POINTER )
    {
        buf = (uchar *) msg->data;

        *buf++ = MAC_MCPS_PURGE_CONFIRM;
        *buf++ = msdu_handle;
        *buf   = status;

        send_mcps_2_nhle( msg );
    }
}

#endif	/*(CFG_MCPS_PURGE_REQ_CONF == 1)*/
/******************************************************************************/
#if(CFG_MLME_ASSOCIATE_REQ_CONF == 1)
/*create and send associate confirm primitive to NHL*/
void send_mlme_associate_confirm(
									ushort short_address,
									uchar status,
									security_params_t *sec_params
                                )
{
    msg_t *msg = NULL;
    uchar *buf = NULL;
    uint8_t adv_prim_2011 = mac_pib.ConfigFlags & ( USE_2006_PRIMITIVES|USE_2011_PRIMITIVES);
    uint16_t length = ASSOCIATE_CONFIRM_SIZE;
	
#ifdef MAC_CFG_SECURITY_ENABLED
	uchar key_ident_len = 0;
    if( sec_params != NULL )
    {
        key_ident_len = get_key_identifier_length(sec_params->key_id_mode);
    }
#endif 
	
    if( adv_prim_2011 )
    {
        /* Add space for Security Data (1 + key_ident_len) */
        //msg = allocate_mac_2_nhle_msg( ASSOCIATE_CONFIRM_SIZE + 1 +
		length += 1;

		#ifdef MAC_CFG_SECURITY_ENABLED
			length += key_ident_len;
		#endif
    }

    msg = allocate_mac_2_nhle_msg( length );


    if( msg != NULL_POINTER )
    {
        buf = (uchar *) msg->data;

        *buf++ = MAC_MLME_ASSOCIATE_CONFIRM;
		put_ushort(buf,short_address);
		buf+=2;
        //*buf++ = LOW_BYTE( short_address );
        //*buf++ = HIGH_BYTE( short_address );
        *buf++ = status;

        if( adv_prim_2011 )
        {
#ifdef MAC_CFG_SECURITY_ENABLED
            append_sec_params( buf, sec_params, key_ident_len );
#else
            /* Add Dummy security Level Byte */
            *buf++ = 0;
#endif
        }
        send_mlme_2_nhle( msg );
    }
}
#endif	/*(CFG_MLME_ASSOCIATE_REQ_CONF == 1)*/

/******************************************************************************/

#if(CFG_MLME_ASSOCIATE_IND_RESP == 1)
/*create and send the associate indication primitive to the NHL */
void send_mlme_associate_indication (
                                               uchar *rxb,
                                               uchar *params,
                                               security_params_t *sec_params
                                     )
{
    msg_t *msg = NULL;
    uchar *buf = NULL;
    uchar cf0 = 0;
    
        uint8_t adv_prim_2011 = mac_pib.ConfigFlags & ( USE_2006_PRIMITIVES|USE_2011_PRIMITIVES);
	uint16_t length = ASSOCIATE_INDICATION_SIZE;

#ifdef MAC_CFG_SECURITY_ENABLED
	uchar key_ident_len = 0;
    if( sec_params != NULL )
    {
        key_ident_len = get_key_identifier_length(sec_params->key_id_mode);
    }
#endif
    if( adv_prim_2011 )
    {
        /* - Old Security (1) + New Security (1) + key_ident_len */
        //msg = allocate_mac_2_nhle_msg( ASSOCIATE_INDICATION_SIZE + 5 +
		length += 5;
		#ifdef MAC_CFG_SECURITY_ENABLED	
				length += key_ident_len;
		#endif
    }

    msg = allocate_mac_2_nhle_msg( length );


    if( msg != NULL_POINTER )
    {
        buf = (uchar *) msg->data;

        /* save control field byte 1*/
        cf0 = rxb[1];

        /* check address mode */
        switch( rxb[2] & MAC_DST_ADDRESS_MASK )
        {
        case MAC_SHORT_DST_ADDRESS:
            rxb += 8; /* length, control 0&1, DSN, Dst PAN, DST SA */
            break;

        case MAC_IEEE_DST_ADDRESS:
            rxb += 14; /* length, control 0&1, DSN, Dst PAN, DST SA */
            break;

        default:
            disable_interrupt();
            free_mac_2_nhle_msg( msg );
            enable_interrupt();
            return;
        }

        if( (cf0 & MAC_INTRA_PAN) == 0 )
        {
            /* adjust for source PAN id if present */
            rxb += 2;
        }

        /* Set primitive */
        *buf++ = MAC_MLME_ASSOCIATE_INDICATION;

        /* copy the source IEEE address */
        memcpy( buf, rxb, 8 );
        buf += 8;

        /* now for the capability information */
        *buf++ = *params;

        if( adv_prim_2011 )
        {
#ifdef MAC_CFG_SECURITY_ENABLED
				append_sec_params( buf, sec_params, key_ident_len );
#else
				*buf++ = 0;
#endif
        }
        else
        {
            /* 2003 Security */
            *buf = (MAC_SECURITY_NOT_IN_ACL << 1);
        }
        send_mlme_2_nhle( msg );
    }
}
#endif	/*(CFG_MLME_ASSOCIATE_IND_RESP == 1)*/

/******************************************************************************/
#if(CGF_MLME_BEACON_NOTIFY_IND == 1)
/*create and send the beacon notify indication mac primitive to the NHL */
uchar send_mlme_beacon_notify( mac_rx_t *rxmsg, pandesc_t *pandesc )
{
    msg_t *msg = NULL;
    uchar *buf = NULL;
    uchar *bcn = NULL;
    uchar cf1 = 0;
    uchar cf0 = 0;

   //uint8_t adv_prim_2011 = mac_pib.ConfigFlags & ( USE_2006_PRIMITIVES|USE_2011_PRIMITIVES);
    /*TODO: need to be a uint16_t to acccomodate the maxPhypacket length which is 2047 bytes*/
    ushort count = 0;
    coex_spec_ie_t coex_spec = {0};

#ifdef MAC_CFG_SECURITY_ENABLED    
    uint8_t adv_prim_2011 = mac_pib.ConfigFlags & ( USE_2006_PRIMITIVES|USE_2011_PRIMITIVES);
#endif
          //cf1 = cf0 = 0;
          /* allocate some memory - rxmsg->data[0] is the receive pdsu length byte
          * memory required is
          * 1 byte for the BEACON_NOTIFY token +
          * length of beacon,
          * 1 byte for channel,
          * 1 byte for channelpage
          * 1 byte for GTS permit flag
          * 1 byte for link quality,
          * 3 bytes for timestamp,
          * max 3 additional bytes for security
          * + 1 byte for sduLength
          */
#define BEACON_NOTIFY_OVERHEAD 21
    /* Length of beacon + beacon Notify Overhead gives buffer larger than is required */
    msg = allocate_mac_2_nhle_msg( BEACON_NOTIFY_OVERHEAD + rxmsg->pd_rxp->psduLength );

    if( msg == NULL_POINTER )
    {
//        debug(( "failed\r\n" ));
        return 0;
    }

    buf = (uchar *) msg->data;
    bcn = rxmsg->pd_rxp->psdu;

	/* Following code expects bcn to point to a byte before the first byte of 
    MAC header. psdu points to first byte of MAC frame, so the decrementation 
    by one byte*/

	bcn -= 0x01;

    *buf++ = MAC_MLME_BEACON_NOTIFY_INDICATION;

    *buf++ = bcn[3]; /* the beacon sequence number */
    
    cf0 = bcn[1];

	/* add the PAN Descriptor */
	mac_prim_append_pandesclist( &buf, pandesc, 1);

	/* Step thru the beacon to find the start of the GTS field */
	bcn += 2; /* step over length and cf0 */
	
	cf1 = *bcn;
	
	/* regular beacon parsing. Skip the address addressing field bytes */
	if( (cf1 & MAC_SRC_ADDRESS_MASK) == MAC_SHORT_SRC_ADDRESS )
	{
		bcn += 4;
	}
	else
	{
#ifdef WISUN_ENET_PROFILE
          bcn += 8;// this is because we do not have the src pan id along with the src addr in the EB as per ENET profile
#else
		bcn += 10;
#endif          
		
	}
	
	if( rxmsg->frame_ver == FRAME_VERSION_2011 )
	{
		/*if it is normal beacon skip the dest addresssing fields accordingly*/
		if( (cf1 & MAC_DST_ADDRESS_MASK) == MAC_SHORT_DST_ADDRESS )
		{
			bcn += 4;
		}
		else if ( (cf1 & MAC_DST_ADDRESS_MASK) == MAC_IEEE_DST_ADDRESS )
		{
			bcn += 10;
		}
		
		if( cf0 & MAC_INTRA_PAN )
		{
			/*we have considered two bytes extra for the pan id*/
			
			bcn -= 0x02;
		}
	}
	
	bcn +=2; /* cf1 and BSN */

	#ifdef MAC_CFG_SECURITY_ENABLED
		/* Security Data has already been extracted */
		/* So just step over */
		if (( rxmsg->sec_param.security_level != MAC_SECURITY_NONE)
			&& (mac_pib.mac_security_enabled == TRUE)
			&& ( adv_prim_2011 ) )
		{
			bcn += 5 + get_key_identifier_length(rxmsg->sec_param.key_id_mode);
		}

	#endif
		/*now bcn will be pointing to superframe spec if it is a regular 
		beacon OR the IE list if it is an enhance beacon*/

	if( rxmsg->frame_ver != FRAME_VERSION_2011 )
	{
		bcn += 2; /* superframe spec */

		/* bcn still pointing at 1st GTS byte, so step over GTS fields */
		if( (*bcn & GTS_LENGTH_MASK) == 0 )
		{
			bcn++;
		}
		else
		{
			bcn += (*bcn & GTS_LENGTH_MASK) + 1;
		}

		/* bcn now pointing at pending address spec */
		count = 1 + ((*bcn & SHORT_ADDRESSES_PENDING_MASK) * 2)
			+ (((*bcn & IEEE_ADDRESSES_PENDING_MASK)/16) * 8);

		if((buf + count) < (((uchar *) msg->data) + BEACON_NOTIFY_OVERHEAD + rxmsg->pd_rxp->psduLength))
		{
			memcpy( buf, bcn, count );
			buf += count;
			bcn += count;
		}
		else
		{
			free_mac_2_nhle_msg( msg );
			return 1;
		}	
	}
	else
	{
		/*Check for the presence of the CoexSpec IE in the payload IE list and 
		assign that as paramter in the BCN-NOTIFY indication param.*/
		*buf++ = 0x0; // pending address spec is assigned as 0	
	}
	
	/* now for the beacon payload - total beacon length is ...*/
	count = rxmsg->pd_rxp->psduLength - (rxmsg->pd_rxp->FCSLength & 0x7FFF); /* exclude CRC */

	/* adjust for beacon bytes consumed */
	// count -= (bcn - &rxmsg->pd_rxp->psdu[0]);
	
	/* adjust for beacon bytes consumed */
	if( count >=  (bcn - &rxmsg->pd_rxp->psdu[0]))
	{
		/*sometimes whenever a error packet(garbled beacon is received and still 
		it goes thru CRC verification properly, then we need to make sure not to 
		corrupt memory */
		count -= (bcn - &rxmsg->pd_rxp->psdu[0]);
	}
	else
	{			
		free_mac_2_nhle_msg( msg );
		return 1;	
	}
	
	

#ifdef MAC_CFG_SECURITY_ENABLED
	if (( rxmsg->sec_param.security_level != MAC_SECURITY_NONE)
		&& (mac_pib.mac_security_enabled == TRUE)
		&& ( adv_prim_2011 ) )
	{
		/* If beacon is secured then subtract the length of the MIC Data */
		count -= integrity_code_length(rxmsg->sec_param.security_level);
	}
#endif
	/* bytes remaining are the payload */
	//*buf++ = count;
    
	put_ushort(buf,count );
    
	buf += 0x02;
    
	if( count != 0 )
	{
		memcpy( buf, bcn, count );
	}
	buf += count;

	/*sdu has been copied. Now copy the EBSN,beaconType and CoexSpecification fields*/
	if( rxmsg->frame_ver != FRAME_VERSION_2011 )
	{
		/*update EBSN field with dummy 0 for regular beacon*/
		*buf++ = 0x0;

		/*update beacon type field to indicate regular beacon*/
		*buf++ = 0x0;
		
	}
	else
	{
		/*copy EBSN*/
		*buf++ = rxmsg->pd_rxp->psdu[2];
		*buf++ = 0x1; // enhanced beacon	
	}

	memset((uint8_t*)&coex_spec,0x0,sizeof(coex_spec_ie_t));
	ie_mgr_extract_coex_spec_ie(rxmsg->payloadIEList,rxmsg->payloadIEListLen,&coex_spec);
	memcpy(buf,(uint8_t*)&coex_spec,sizeof(coex_spec_ie_t));

#ifdef RL78_TARGET_CUBESUITE 	
	buf += (sizeof(coex_spec_ie_t)-1);// to take care of 1 byte padding issue
#else
	buf += sizeof(coex_spec_ie_t);
#endif	
    
    /* quickest way to tell the length is to take the difference in buffer pointers */
    msg->data_length = (buf - msg->data);

    send_mlme_2_nhle( msg );

    return 1;
}
#endif	/*(CGF_MLME_BEACON_NOTIFY_IND == 1)*/

/******************************************************************************/
#if(CFG_MLME_DISASSOCIATE_IND == 1)
/*create and send the disassociate indication primitive to the NHL */
void send_mlme_disassociate_indication (
                                            mac_rx_t *rxmsg
                                            uchar *rxb,
                                            uchar *params,
                                            security_params_t *sec_params
                                        )
{
        msg_t *msg = NULL;
        uchar *buf = NULL;

        mac_address_t dst_address = {0};
        mac_address_t src_address = {0};
        uint8_t adv_prim_2011 = mac_pib.ConfigFlags & ( USE_2006_PRIMITIVES|USE_2011_PRIMITIVES);
        uint16_t length = DISASSOCIATE_INDICATION_SIZE;

#ifdef MAC_CFG_SECURITY_ENABLED
        uchar key_ident_len = 0;    
    if( sec_params != NULL )
    {
        key_ident_len = get_key_identifier_length(sec_params->key_id_mode);
    }
#endif

    if( adv_prim_2011 )
    {
        /* - Old Security (1) + New Security (1) + key_ident_len */
        //msg = allocate_mac_2_nhle_msg( DISASSOCIATE_INDICATION_SIZE + 

		#ifdef MAC_CFG_SECURITY_ENABLED
			length +=  key_ident_len;
		#endif
    }

    msg = allocate_mac_2_nhle_msg( length );

    if( msg != NULL_POINTER )
    {
        buf = (uchar *) msg->data;

        *buf++ = MAC_MLME_DISASSOCIATE_INDICATION;

        /* get the destination and source addresses from the received frame */
        mac_frame_parse_addresses(rxmsg,rxb+1, &dst_address, &src_address );
        /* and store in the indication buffer */
        memcpy( buf, src_address.address.ieee_address, 8 );
        buf+=8;

        /* reason */
        *buf++ = params[0];


        if( adv_prim_2011 )
        {
#ifdef MAC_CFG_SECURITY_ENABLED
				append_sec_params( buf, sec_params, key_ident_len );
#else
				*buf++ = 0;
#endif
        }
        else
        {
            /* 2003 Security */
            *buf = (MAC_SECURITY_NOT_IN_ACL << 1);
        }
        send_mlme_2_nhle( msg );
    }
}
#endif	/*(CFG_MLME_DISASSOCIATE_IND == )*/

/******************************************************************************/

/*create and send orphan indication to the NHL */
void send_mlme_orphan_indication(
                                       uchar *rxb,
                                       security_params_t *sec_params
                                 )
{
        msg_t *msg = NULL;
        uchar *buf = NULL;

        uint8_t adv_prim_2011 = mac_pib.ConfigFlags & ( USE_2006_PRIMITIVES|USE_2011_PRIMITIVES);
        uint16_t length = ORPHAN_INDICATION_SIZE;
#ifdef MAC_CFG_SECURITY_ENABLED
	uchar key_ident_len = 0;   
    if( sec_params != NULL )
    {
		key_ident_len = get_key_identifier_length(sec_params->key_id_mode);
    }
#endif

    if( adv_prim_2011 )
    {
        /* - Old Security (1) + New Security (1) + key_ident_len */
       // msg = allocate_mac_2_nhle_msg( ORPHAN_INDICATION_SIZE + 
		#ifdef MAC_CFG_SECURITY_ENABLED
			length +=  key_ident_len;
		#endif 
    }

    msg = allocate_mac_2_nhle_msg( length );


    if( msg != NULL_POINTER )
    {
        buf = (uchar *) msg->data;
        *buf++ = MAC_MLME_ORPHAN_INDICATION;

        /* step over control fields, Destination and source PAN ID. */
        if( (rxb[1] & MAC_INTRA_PAN) == 0 )
        {
            rxb += 2;
        }
        rxb += 8;

        /* Copy IEEE source address */
        memcpy( buf, rxb, 8 );
        buf += 8;

        if( adv_prim_2011 )
        {
#ifdef MAC_CFG_SECURITY_ENABLED
				append_sec_params( buf, sec_params, key_ident_len );
#else
				*buf++ = 0;
#endif
        }
        else
        {
            /* 2003 Security */
            *buf = (MAC_SECURITY_NOT_IN_ACL << 1);
        }
        send_mlme_2_nhle( msg );
    }
}

/******************************************************************************/
#if(CGF_MLME_COMM_STATUS_IND == 1)
/*create and send the comm status indication mac primitive to the NHL */
uchar send_mlme_comm_status_indication (
                                            mac_address_t *src,
                                            mac_address_t *dst,
                                            uchar status,
                                            security_params_t *sec_param
                                        )
{
        msg_t *msg = NULL;
        uchar *buf = NULL;

        uint8_t adv_prim_2011 = mac_pib.ConfigFlags & ( USE_2006_PRIMITIVES|USE_2011_PRIMITIVES);
        uint16_t length = COMM_STATUS_INDICATION_SIZE + IEEE_ADDRESS_LENGTH + IEEE_ADDRESS_LENGTH;
    /* Get the length of the Keying Data */
#ifdef MAC_CFG_SECURITY_ENABLED
	uchar key_ident_len = 0;
    if( sec_param != NULL )
    {
        key_ident_len = get_key_identifier_length(sec_param->key_id_mode);
    }
#endif

    if( adv_prim_2011 )
    {
        /* New Security (1) + key_ident_len */
        //msg = allocate_mac_2_nhle_msg( COMM_STATUS_INDICATION_SIZE + IEEE_ADDRESS_LENGTH + IEEE_ADDRESS_LENGTH + 5 + 
			length += 5;
			#ifdef MAC_CFG_SECURITY_ENABLED
				length += key_ident_len;
			#endif
    }

    /* always allocate max space required, as it is quicker */
    msg = allocate_mac_2_nhle_msg( length );

    if( msg != NULL_POINTER )
    {
        buf = &msg->data[0];

        *buf++ = MAC_MLME_COMM_STATUS_INDICATION;
		put_ushort(buf,src->pan_id);
		buf+=2;
        //*buf++ = LOW_BYTE( src->pan_id );
        //*buf++ = HIGH_BYTE( src->pan_id );
        *buf++ = src->address_mode;
        switch( src->address_mode )
        {
        case MAC_SHORT_ADDRESS:
			put_ushort(buf,src->address.short_address);
			buf+=2;
            //*buf++ = LOW_BYTE( src->address.short_address );
            //*buf++ = HIGH_BYTE( src->address.short_address );
            break;

        case MAC_IEEE_ADDRESS:
            memcpy( buf, src->address.ieee_address, 8 );
            buf += 8;
            break;
        }

        *buf++ = dst->address_mode;
        switch( dst->address_mode )
        {
        case MAC_SHORT_ADDRESS:
			put_ushort(buf,dst->address.short_address);
			buf+=2;
            //*buf++ = LOW_BYTE( dst->address.short_address );
            //*buf++ = HIGH_BYTE( dst->address.short_address );
            break;

        case MAC_IEEE_ADDRESS:
            memcpy( buf, dst->address.ieee_address, 8 );
            buf += 8;
            break;
        }

        *buf++ = status;

        if( adv_prim_2011 )
        {
#ifdef MAC_CFG_SECURITY_ENABLED
				buf = append_sec_params( buf, sec_param, key_ident_len );
#else
				*buf++ = 0;
#endif
        }

        /* data length is current position minus start of buffer */
        msg->data_length = (buf - msg->data);
        send_mlme_2_nhle( msg );

        return 1;
    }
   return 0;
}

#endif	/*(CGF_MLME_COMM_STATUS_IND == 1)*/

/******************************************************************************/
#if(CFG_MLME_DISASSOCIATE_REQ_CONF == 1)
/*create and send the disassociate confirmation mac primitive to the NHL */
void send_mlme_disassociate_confirm(
                                        uchar status,
                                        mac_address_t *addr
                                    )
{
        msg_t *msg = NULL;
        uchar *buf = NULL;
        uint8_t adv_prim_2011 = mac_pib.ConfigFlags & ( USE_2006_PRIMITIVES|USE_2011_PRIMITIVES);
        uint16_t length = DISASSOCIATE_CONFIRM_SIZE;

    /* Allocate buffer large enough for IEEE Address */
    if( adv_prim_2011 )
    {
        /* Allocate buffer large enough for IEEE Address + PANID + Addr mode */
        //msg = allocate_mac_2_nhle_msg( DISASSOCIATE_CONFIRM_SIZE + IEEE_ADDRESS_LENGTH + 1 + 2);
		length += IEEE_ADDRESS_LENGTH + 3;
    }

    msg = allocate_mac_2_nhle_msg( length );

    if( msg != NULL_POINTER )
    {
        buf = (uchar *) msg->data;

        *buf++ = MAC_MLME_DISASSOCIATE_CONFIRM;
        *buf++ = status;

        if( adv_prim_2011 )
        {
            *buf++ = addr->address_mode;
			put_ushort(buf,addr->pan_id);
			buf+=2;
            //*buf++ = LOW_BYTE( addr->pan_id );
            //*buf++ = HIGH_BYTE( addr->pan_id );

            switch( addr->address_mode )
            {
            case ADDR_MODE_NONE:/*fall through*/
            case ADDR_MODE_RESVD:
                break;

            case ADDR_MODE_SHORT:
				put_ushort(buf,addr->address.short_address);
				buf+=2;
                //*buf++ = LOW_BYTE( addr->address.short_address );
                //*buf++ = HIGH_BYTE( addr->address.short_address );
                break;

            case ADDR_MODE_EXTENDED:
                memcpy( buf, addr->address.ieee_address, 8 );
                buf += 8;
                break;
            }
        }
        send_mlme_2_nhle( msg );
    }
}
#endif	/*(CFG_MLME_DISASSOCIATE_REQ_CONF == 1)*/

/******************************************************************************/
#if(CFG_MLME_GET_REQ_CONF == 1)
uchar send_mlme_get_confirm ( 
                                 uchar status,
                                 uchar id,
                                 uchar attribute_index,
                                 ushort length,
                                 void *value 
                                                )
{
      msg_t *msg = NULL;
      uchar *buf = NULL;

      uint8_t adv_prim_2011 = mac_pib.ConfigFlags & ( USE_2006_PRIMITIVES|USE_2011_PRIMITIVES);
      uint16_t buf_length = GET_CONFIRM_SIZE + length;

    /* If 802.15.4-2006 Primitives are being used */
    if( adv_prim_2011 )
    {
        /* Add 2 to the length for the new parameters */
        buf_length += 7;
    }

    msg = allocate_mac_2_nhle_msg( buf_length );

    if( msg != NULL_POINTER )
    {
        buf = (uchar *) msg->data;
        *buf++ = MAC_MLME_GET_CONFIRM;
        *buf++ = status;
        *buf++ = id;

        if( adv_prim_2011 )
        {
            *buf++ = attribute_index;   
			put_ushort(buf,length);
			buf+=2;
            //*buf++ = LOW_BYTE( length );
            //*buf++ = HIGH_BYTE( length );
        }

        if( length != 0 )
        {
            memcpy( buf, value, length );
        }
        msg->data_length = buf_length ;

        send_mlme_2_nhle( msg );

        return 1;
    }

    return 0;
}
#endif	/*(CFG_MLME_GET_REQ_CONF == 1)*/

/******************************************************************************/

#ifdef MAC_CFG_SECURITY_ENABLED
uchar send_key_table_get_confirm( uchar attribute_index )
{
    msg_t *msg = NULL;
    uchar *buf = NULL;
    uchar length = 0;
    key_descriptor_t *p = NULL;

    /* If 802.15.4-2006 Primitives are being used */
    if( mac_pib.ConfigFlags & ( USE_2006_PRIMITIVES|USE_2011_PRIMITIVES ) )
    {
        /* Get the Key Table Entry */
        p = (key_descriptor_t*) find_key_table_entry( attribute_index );

        if ( p != NULL_POINTER )
        {
#if(CFG_MAC_BLACK_LIST_IS_USE == 1) 
            /* Calculate how big a buffer we need */
            length = ( KEY_DESC_LENGTH
                + (p->key_device_list_entries * KEY_DEVICE_DESC_LENGTH )
                + (p->key_id_lookup_list_entries * KEY_ID_LOOKUP_DESC_LENGTH)
                + (p->key_usage_list_entries * KEY_USAGE_DESC_LENGTH));
#endif 

            /* Add 2 to the length for the new parameters */
            msg = allocate_mac_2_nhle_msg( GET_CONFIRM_SIZE + length + 3 );

            if ( msg != NULL_POINTER )
            {
                buf = (uchar *) msg->data;

                *buf++ = MAC_MLME_GET_CONFIRM;
                *buf++ = SUCCESS;
                *buf++ = macKeyTable;
                *buf++ = attribute_index;
                *buf++ = LOW_BYTE( length );
                *buf++ = HIGH_BYTE( length );
                
                write_key_table_entry_to_buffer( buf, p, FALSE );

                msg->data_length = GET_CONFIRM_SIZE + length + 3 ;
            }
            else
            {
                return 0;
            }
        }
        else /* Key table Entry not found */
        {
            msg = allocate_mac_2_nhle_msg( GET_CONFIRM_SIZE + 3 );

            if ( msg != NULL_POINTER )
            {
                buf = (uchar *) msg->data;

                *buf++ = MAC_MLME_GET_CONFIRM;
                *buf++ = MAC_INVALID_INDEX;
                *buf++ = macKeyTable;
                *buf++ = attribute_index;
                *buf++ = 0;
                *buf++ = 0;
                msg->data_length = GET_CONFIRM_SIZE + 3;
            }
            else
            {
                return 0;
            }
        }
    }
    else /* 2003 Mode */
    {
        msg = allocate_mac_2_nhle_msg( GET_CONFIRM_SIZE );

        if ( msg != NULL_POINTER )
        {
            buf = (uchar *) msg->data;

            *buf++ = MAC_MLME_GET_CONFIRM;
            *buf++ = MAC_INVALID_PARAMETER;
            *buf++ = macKeyTable;
            msg->data_length = GET_CONFIRM_SIZE;
        }
        else
        {
            return 0;
        }
    }

    send_mlme_2_nhle( msg );
    return 1;
}


/******************************************************************************/
uchar send_device_table_get_confirm( uchar attribute_index )
{
    msg_t *msg = NULL;
    uchar *buf = NULL;
    device_descriptor_t *p = NULL;

    /* If 802.15.4-2006 Primitives are being used */
    if ( mac_pib.ConfigFlags & ( USE_2006_PRIMITIVES|USE_2011_PRIMITIVES ) )
    {
        /* Add 2 to the length for the new parameters */
        msg = allocate_mac_2_nhle_msg( GET_CONFIRM_SIZE + DEVICE_DESC_LENGTH + 3 );

        if ( msg != NULL_POINTER )
        {
            buf = (uchar *) msg->data;
            *buf++ = MAC_MLME_GET_CONFIRM;
            

            p = (device_descriptor_t*) find_device_table_entry( attribute_index );

            if (p != NULL_POINTER)
            {
               
               *buf++ = SUCCESS;
               *buf++ = macDeviceTable;
               *buf++ = attribute_index;            
               *buf++ = LOW_BYTE( DEVICE_DESC_LENGTH );
               *buf++ = HIGH_BYTE( DEVICE_DESC_LENGTH );

                write_device_table_entry_to_buffer( buf, p, FALSE );

                msg->data_length = GET_CONFIRM_SIZE + DEVICE_DESC_LENGTH + 3 ;
            }
            else
            {
                
               *buf++ = MAC_INVALID_INDEX;
               *buf++ = macDeviceTable;
               *buf++ = attribute_index;                                 
               *buf++ = 0;
               *buf = 0;
               msg->data_length = GET_CONFIRM_SIZE + 3;
            }
        }
        else
        {
            return 0;
        }
    }
    else /* 2003 Mode */
    {
        msg = allocate_mac_2_nhle_msg( GET_CONFIRM_SIZE );

        if ( msg != NULL_POINTER )
        {
            buf = (uchar *) msg->data;

            *buf++ = MAC_MLME_GET_CONFIRM;
            *buf++ = MAC_INVALID_PARAMETER;
            *buf = macDeviceTable;
            msg->data_length = GET_CONFIRM_SIZE;
        }
        else
        {
            return 0;
        }
    }

    send_mlme_2_nhle( msg );
    return 1;
}


/******************************************************************************/

uchar send_security_level_table_get_confirm( uchar attribute_index )
{
    msg_t *msg = NULL;
    uchar *buf = NULL;
    security_level_descriptor_t *p = NULL;

    /* If 802.15.4-2006 Primitives are being used */
    if ( mac_pib.ConfigFlags & ( USE_2006_PRIMITIVES|USE_2011_PRIMITIVES ) )
    {
        /* Add 2 to the length for the new parameters */
        msg = allocate_mac_2_nhle_msg( GET_CONFIRM_SIZE + SEC_LEVEL_DESC_LENGTH + 2 );

        if ( msg != NULL_POINTER )
        {
            buf = (uchar *) msg->data;

            *buf++ = MAC_MLME_GET_CONFIRM;
            

            p = (security_level_descriptor_t*) find_security_level_table_entry( attribute_index );

            if (p != NULL_POINTER)
            {
                *buf++ = SUCCESS;
                *buf++ = macSecurityLevelTable;
                *buf++ = attribute_index;
               
                *buf++ = LOW_BYTE( SEC_LEVEL_DESC_LENGTH );
                *buf++ = HIGH_BYTE( DEVICE_DESC_LENGTH );
                

                write_security_level_table_entry_to_buffer( buf, p, FALSE );

                msg->data_length = GET_CONFIRM_SIZE + SEC_LEVEL_DESC_LENGTH + 3;
            }
            else
            {
               *buf++ = MAC_INVALID_INDEX;
                *buf++ = macSecurityLevelTable;
               *buf++ = attribute_index;
                *buf++ = 0;
                *buf = 0;
                msg->data_length = GET_CONFIRM_SIZE + 3;
            }
        }
        else
        {
            return 0;
        }
    }
    else /* 2003 Mode */
    {
        msg = allocate_mac_2_nhle_msg( GET_CONFIRM_SIZE );

        if ( msg != NULL_POINTER )
        {
            buf = (uchar *) msg->data;

            *buf++ = MAC_MLME_GET_CONFIRM;
            *buf++ = MAC_INVALID_PARAMETER;
            *buf = macSecurityLevelTable;
            msg->data_length = GET_CONFIRM_SIZE;
        }
        else
        {
            return 0;
        }
    }

    send_mlme_2_nhle( msg );
    return 1;
}

#endif
/******************************************************************************/

#ifdef MAC_CFG_GTS_ENABLED
/*create and send gts confirm primitive to NHL */
void send_mlme_gts_confirm( uchar gts_characteristics, uchar status )
{
   
    uchar *buf = NULL;
    msg_t *msg = allocate_mac_2_nhle_msg( GTS_CONFIRM_SIZE );

    if( msg != NULL_POINTER )
    {
        buf = (uchar *) msg->data;

        *buf++ = MAC_MLME_GTS_CONFIRM;

        *buf++ = gts_characteristics;
        *buf++ = status;

        send_mlme_2_nhle( msg );
    }
}
#endif

/******************************************************************************/
#if(CFG_MLME_POLL_REQ_CONF == 1)
/*create and send poll confirm primitive to NHL */
uchar send_mlme_poll_confirm( uchar status )
{
    
    uchar *buf = NULL;
    msg_t *msg = allocate_mac_2_nhle_msg( POLL_CONFIRM_SIZE );

    if( msg != NULL_POINTER )
    {
        buf = (uchar *) msg->data;

        *buf++ = MAC_MLME_POLL_CONFIRM;
        *buf = status;
        send_mlme_2_nhle( msg );
        return 1;
    }
   return 0;
}
#endif	/*(CFG_MLME_POLL_REQ_CONF == 1)*/

/******************************************************************************/
#if(CGF_MLME_RESET_REQ_CONF == 1)
/*create and send reset confirm primitive to the NHL */
void send_mlme_reset_confirm( uchar status )
{
    msg_t *msg = allocate_mac_2_nhle_msg( RESET_CONFIRM_SIZE );
    
    uint8_t* buf = NULL;

    if( msg != NULL_POINTER )
    {
          buf = (uchar *) msg->data;
          
          *buf++ = MAC_MLME_RESET_CONFIRM;
          *buf = status;
     
          send_mlme_2_nhle( msg );   
    }
    else
    {
        //debug(( "failed\r\n" ));
    }
}
#endif	/*(CGF_MLME_RESET_REQ_CONF == 1)*/

/******************************************************************************/
#if(CFG_MLME_RX_ENABLE_REQ_CONF == 1)
void send_mlme_rx_enable_confirm( uchar status )
{
    //msg_t *msg;
    uchar *buf = NULL;
    msg_t *msg = allocate_mac_2_nhle_msg( RX_ENABLE_CONFIRM_SIZE );

    if( msg != NULL_POINTER )
    {
        buf = (uchar *) msg->data;

        *buf++ = MAC_MLME_RX_ENABLE_CONFIRM;
        *buf = status;

        send_mlme_2_nhle( msg );
    }
}
#endif

/******************************************************************************/
#if(CFG_MLME_SCAN_REQ_CONF == 1 )
void send_mlme_scan_confirm( mac_status_t status, scan_param_t param )
{
    uchar *buf = NULL, *tmp = NULL;
    uint8_t i = 0;
    uint16_t scan_prim_len = 20;
    msg_t *msg = NULL;
    
    switch( param.type )
    {
	    case ENERGY_DETECT_SCAN:
	    	scan_prim_len += scansm.list_count;
		break;

	    case ACTIVE_SCAN:
	    case PASSIVE_SCAN:
	    case EB_ACTIVE_SCAN:
	    case MPM_EB_PASSIVE_SCAN:
    	scan_prim_len += (scansm.list_count * 60);
		break;
		
	    case ORPHAN_SCAN:
	    break;
	    
	    default:
	    break;
    }
    /*TBD We don't need big buffer for orphan scan and macAutoRequest==TRUE */
    msg = allocate_mac_2_nhle_msg( scan_prim_len );

    if( msg == NULL_POINTER )
    {
        return;
    }

    buf = (uchar *) msg->data;

    /* set base length */
    msg->data_length = SCAN_CONFIRM_SIZE;

    /* set message data */
    *buf++ = MAC_MLME_SCAN_CONFIRM;
    *buf++ = status;
    /*TBD This may be incorrect for SCAN_IN_PRORESS status */
    *buf++ = param.type;

    /* check if we are using 802.15.4-2006 primitives */
    if( mac_pib.ConfigFlags & ( USE_2006_PRIMITIVES|USE_2011_PRIMITIVES ) )
    {
        /* insert ChannelPage */
        *buf++ = param.page;
        msg->data_length++;
    }

    /* UnscannedChannels */
    *buf++ = param.channels[7];
    *buf++ = param.channels[6];
    *buf++ = param.channels[5];
    *buf++ = param.channels[4];
    *buf++ = param.channels[3];
    *buf++ = param.channels[2];
    *buf++ = param.channels[1];
    *buf++ = param.channels[0];

    switch( param.type )
    {
    case ENERGY_DETECT_SCAN:

        /* ResultListSize */
        *buf++ = scansm.list_count;

        tmp = buf;

        /* EnergyDetectList */
        for(; i < scansm.list_count; i++ )
        {
            *buf++ = scansm.list.ed[ i ];
        }

        /* update message length */
        msg->data_length += buf - tmp;
        break;

    case ACTIVE_SCAN:/*fall through*/
    case PASSIVE_SCAN:/*fall through*/
    case EB_ACTIVE_SCAN:/*fall through*/
    case MPM_EB_PASSIVE_SCAN:

        /* ResultListSize */
        *buf++ = scansm.list_count;

        tmp = buf;

        /* PANDescriptorList */
        mac_prim_append_pandesclist( &buf, scansm.list.pandesc, scansm.list_count );

        /* update message length */
        msg->data_length += buf - tmp;
        break;

    case ORPHAN_SCAN:
    default:

        /* ResultListSize */
        *buf++ = 0;
        break;
    }

    /* send message */
    send_mlme_2_nhle( msg );
}

#endif	/*(CFG_MLME_SCAN_REQ_CONF == 1 )*/

/******************************************************************************/
#if(CFG_MLME_SET_REQ_CONF == 1)
uchar send_mlme_set_confirm(
	                            uchar status,          
	                            uchar id,               
	                            uchar attribute_index  
                            )
{
          msg_t *msg = NULL;
          uchar *buf = NULL;
          uint8_t adv_prim_2011 = mac_pib.ConfigFlags & ( USE_2006_PRIMITIVES|USE_2011_PRIMITIVES);
          uint16_t length = SET_CONFIRM_SIZE;
    if ( adv_prim_2011 )
    {
        /* Add one to length for PIBAttributeIndex */
        //msg = allocate_mac_2_nhle_msg(  + 1 + 5 );
		length += 6;
    }
    msg = allocate_mac_2_nhle_msg( length );

    if( msg != NULL_POINTER )
    {
        buf = msg->data;

        *buf++ = MAC_MLME_SET_CONFIRM;
        *buf++ = status;
        *buf++ = id;

        if ( adv_prim_2011 )
        {
            *buf++ = attribute_index;
        }

        send_mlme_2_nhle( msg );
        return 1;
    }
    return 0;
}
#endif	/*(CFG_MLME_SET_REQ_CONF == 1)*/

/******************************************************************************/

#ifdef MAC_CFG_GTS_ENABLED
/*create and send the gts indication primitive to the uplink msg queue*/
void send_mlme_gts_indication(
                                    ushort src_pan_id,
                                    address_t *src_addr,
                                    uchar src_addr_mode,
                                    ushort address,
                                    uchar gts_info,
                                    uchar security_level,
                                    uchar key_id_mode,
                                    uchar *key_identifier
                              )
{
    msg_t *msg = NULL;
    uchar *buf = NULL;
    uchar key_ident_len = 0;

    /* Get the length of the Keying Data */
    key_ident_len = get_key_identifier_length(key_id_mode);

    if ( mac_pib.ConfigFlags & ( USE_2006_PRIMITIVES|USE_2011_PRIMITIVES ) )
    {
        /* - Old Security (1) + New Security (2) + key_ident_len */
        msg = allocate_mac_2_nhle_msg( GTS_INDICATION_SIZE + 1 + key_ident_len );
    }
    else
    {
        msg = allocate_mac_2_nhle_msg( GTS_INDICATION_SIZE );
    }

    if( msg != NULL_POINTER )
    {
        buf = (uchar *) msg->data;

        *buf++ = MAC_MLME_GTS_INDICATION;
		put_ushort(buf,address);
		buf+=2;
        //*buf++ = LOW_BYTE( address );
        //*buf++ = HIGH_BYTE( address );
        *buf++ = gts_info; /* GTS info is a one byte structure */

        if ( mac_pib.ConfigFlags & ( USE_2006_PRIMITIVES|USE_2011_PRIMITIVES ) )
        {
#ifdef MAC_CFG_SECURITY_ENABLED
            if (mac_pib.mac_security_enabled == TRUE)
            {
                /* Now add the security Data */
                *buf++ = (key_id_mode << 4) | security_level;
                if ( security_level != MAC_SECURITY_NONE)
                {
                    /* Copy Over KeySource and KeyIndex */
                    memcpy(buf, key_identifier, key_ident_len);
                }
            }
            else
            {
                /* Add Dummy security Level Byte */
                *buf++ = 0;
            }
#else
            /* Add Dummy security Level Byte */
            *buf++ = 0;
#endif
        }
        else
        {
            /* 2003 Security */
            *buf = (MAC_SECURITY_NOT_IN_ACL << 1);
        }

        send_mlme_2_nhle( msg );
    }
}
#endif

/******************************************************************************/
#if(CFG_MLME_START_REQ_CONF == 1)
void send_mlme_start_confirm(
                                    uchar status 
							)
{

    msg_t *msg = allocate_mac_2_nhle_msg( START_CONFIRM_SIZE );
    uint8_t* buf = NULL;

    if( msg != NULL_POINTER )
    {
        buf = (uint8_t*)msg->data;
        
        *buf++  = MAC_MLME_START_CONFIRM;
        *buf = status;

        send_mlme_2_nhle( msg );
    }
}

#endif	/*(CFG_MLME_START_REQ_CONF == 1)*/

/******************************************************************************/
#if(CFG_MLME_SYNC_LOSS_IND == 1)
void send_mlme_sync_loss_indication(
                                    uchar loss_reason,
                                    security_params_t *sec_params
                                    )
{
          msg_t *msg = NULL;
          uchar *buf = NULL;
          ulong temp = 0;
          uint16_t len = 0;

          uchar msg_size = 0;

          uint8_t adv_prim_2011 = mac_pib.ConfigFlags & ( USE_2006_PRIMITIVES|USE_2011_PRIMITIVES);
#ifdef MAC_CFG_SECURITY_ENABLED
	uchar key_ident_len = 0;
    /* Get the length of the Keying Data */
    if( sec_params != NULL_POINTER )
    {
            key_ident_len = get_key_identifier_length(sec_params->key_id_mode);
    }
#endif

    if( adv_prim_2011 )
    {
        /* + panid (2) + channel(1) + page(1) + security(1) + key_id_len ) */
        msg_size = SYNC_LOSS_INDICATION_SIZE + 5 + 
#ifdef MAC_CFG_SECURITY_ENABLED			
                    key_ident_len 
#else
                    0
#endif
                    ;
    }
    else
    {
        msg_size = SYNC_LOSS_INDICATION_SIZE;
    }
    msg = allocate_mac_2_nhle_msg( msg_size );

    if( msg != NULL_POINTER )
    {
        buf = (uchar *) msg->data;

        *buf++ = MAC_MLME_SYNC_LOSS_INDICATION;
        *buf++ = loss_reason;

        if( adv_prim_2011 )
        {
            put_ushort(buf,mac_pib.PANId);
            buf+=2;
            //*buf++ = LOW_BYTE( mac_pib.PANId );
            //*buf++ = HIGH_BYTE( mac_pib.PANId );
            PLME_get_request( phyCurrentChannel, &len, &temp );
            *buf++ = temp & 0xff;
            temp = 0;
            PLME_get_request( phyCurrentPage, &len, &temp );
            *buf++ = temp & 0xff;
        }
#ifdef MAC_CFG_SECURITY_ENABLED
            buf = append_sec_params( buf, sec_params, key_ident_len );
#else
            *buf++ = 0;
#endif

        send_mlme_2_nhle( msg );
    }
}
#endif	/*(CFG_MLME_SYNC_LOSS_IND == 1)*/

/******************************************************************************/
#if(CGF_MLME_BEACON_REQUEST_IND == 1)
void send_mlme_beacon_request_indicaiton(
                                            uchar bcn_type,
                                            mac_rx_t *mrp,
                                            ushort ebfie_len,
                                            uchar* eb_filter_ie
                                          )
{

    uchar *buf = NULL;
    ushort ie_fld_len = mrp->payloadIEFieldLen;	
    //ushort msg_size = 13 + mrp->headerIEFieldLen + mrp->payloadIEFieldLen;

    msg_t* msg = allocate_mac_2_nhle_msg( 13 + mrp->headerIEFieldLen +  ie_fld_len);

    if( msg != NULL_POINTER )
    {
      buf = (uchar *) msg->data;

      *buf++ = MAC_MLME_BEACON_REQUEST_INDICATION;
      *buf++ = bcn_type;

      *buf++ = mrp->src.address_mode;

      switch( mrp->src.address_mode )
      {
        case MAC_SHORT_ADDRESS:
        put_ushort(buf,mrp->src.address.short_address);
        buf+=2;
        //*buf++ = LOW_BYTE( mrp->src.address.short_address );
        //*buf++ = HIGH_BYTE( mrp->src.address.short_address );
        break;

        case MAC_IEEE_ADDRESS:
        memcpy( buf, mrp->src.address.ieee_address, 8 );
        buf += 8;
        break;

        default:
        break;
      }

      put_ushort(buf, mrp->dst.pan_id);
      buf+=2;

      //*buf++ = LOW_BYTE( mrp->dst.pan_id );
      //*buf++ = HIGH_BYTE( mrp->dst.pan_id );

      //ie_fld_len = /*mrp->headerIEFieldLen + */mrp->payloadIEFieldLen;
      put_ushort(buf, ie_fld_len);
      buf+=2;
      //*buf++ = LOW_BYTE( ie_fld_len);
      //*buf++ = HIGH_BYTE( ie_fld_len );

      //memcpy(buf,mrp->headerIEList,mrp->headerIEFieldLen );
      //buf += mrp->headerIEFieldLen;

      memcpy(buf,mrp->payloadIEList,ie_fld_len );
      buf += ie_fld_len;

      send_mlme_2_nhle( msg );
    }
}
#endif	/*(CGF_MLME_BEACON_REQUEST_IND == 1)*/

/******************************************************************************/
#if(CGF_MLME_BEACON_REQ_CONF == 1)
void send_mlme_beacon_confirm( uchar status )
{
    msg_t *msg = allocate_mac_2_nhle_msg( START_CONFIRM_SIZE );
    uint8_t* buf = NULL;

    if( msg != NULL_POINTER )
    {
        buf = msg->data;
        *buf++= MAC_MLME_BEACON_CONFIRM;
        *buf = status;

        send_mlme_2_nhle( msg );
    }
}
#endif	/*(CGF_MLME_BEACON_REQ_CONF == 1)*/

/******************************************************************************/

/*
** ============================================================================
** Private Function Definitions
** ============================================================================
*/
#ifdef MAC_CFG_SECURITY_ENABLED
static uchar *append_sec_params( 
                                uchar *buf, 
                                security_params_t *sec_params, 
                                uchar key_ident_len 
                                )
{

    if( (mac_pib.mac_security_enabled == TRUE) && (sec_params != NULL))
    {
        /* Now add the security Data */
        *buf++ = (sec_params->key_id_mode << 4) | sec_params->security_level;
        if ( sec_params->security_level != MAC_SECURITY_NONE)
        {
            /* Copy Over KeySource and KeyIndex */
            memcpy(buf, sec_params->key_identifier, key_ident_len);
            buf+= (key_ident_len);
            //*buf++ = sec_params->key_identifier[8];
        }
    }
    else
    {
        /* Add Dummy security Level Byte */
        *buf++ = 0;
    }

    return buf;
}
#endif
/******************************************************************************/

static mac_status_t mac_prim_append_address( uchar **buf, mac_address_t *addr )
{
    /* set the address mode */
    (*buf)[0] = addr->address_mode;

    /* increment the buffer pointer */
    *buf+=1;

    switch( addr->address_mode )
    {
    case ADDR_MODE_NONE:/*fall through*/
    case ADDR_MODE_RESVD:
        break;

    case ADDR_MODE_SHORT:
		put_ushort((*buf), addr->pan_id);
		(*buf) += 2;
        //(*buf)[0] = LOW_BYTE( addr->pan_id);
       // (*buf)[1] = HIGH_BYTE( addr->pan_id);
        // *buf += 2;
		put_ushort((*buf), addr->address.short_address);
		 (*buf) += 2;
        //(*buf)[0] = LOW_BYTE( addr->address.short_address );
        //(*buf)[1] = HIGH_BYTE( addr->address.short_address );
        //*buf += 2;
        break;

    case ADDR_MODE_EXTENDED:
		put_ushort(((*buf)), addr->pan_id);
		(*buf) += 2;
        //(*buf)[0] = LOW_BYTE( addr->pan_id);
        //(*buf)[1] = HIGH_BYTE( addr->pan_id);
        //*buf += 2;
        memcpy( *buf, addr->address.ieee_address, 8 );
        (*buf) += 8;
        break;

    default:
        return MAC_INVALID_ADDRESS;
    }
    return MAC_SUCCESS;
}

/******************************************************************************/
#if( (CGF_MLME_BEACON_NOTIFY_IND == 1) || (CFG_MLME_SCAN_REQ_CONF == 1 ))
static void mac_prim_append_pandesclist( uchar **buf, pandesc_t *pandesc, uchar count )
{
    uchar i = 0;
    pandesc_t *p = NULL;
	
#if MAC_CFG_SECURITY_ENABLED
//#ifdef MAC_CFG_SECURITY_ENABLED /*Umesh changed here*/
    /* get the length of the Keying Data */
    uchar key_id_length = get_key_identifier_length(pandesc->sec_params.key_id_mode);
#endif
	uint8_t adv_prim_2011 = mac_pib.ConfigFlags & ( USE_2006_PRIMITIVES|USE_2011_PRIMITIVES);
    for( ; i < count; i++ )
    {
        p = pandesc + i;

        /* address fields */
        mac_prim_append_address( buf, &p->address );

        /* LogicalChannel */
        *(*buf)++ = p->channel;

        /* ChannelPage */
        if( adv_prim_2011 )
        {
            *(*buf)++ = p->page;
        }

        /* SuperFrameSpec */
        *(*buf)++ = p->sf_spec[0];
        *(*buf)++ = p->sf_spec[1];

        /* GTSPermit */
        *(*buf)++ = p->gts_permit;

        /* LinkQuality */
        *(*buf)++ = p->link_quality;

        /* TimeStamp */
        (*buf)[0] = p->timestamp[0];
        (*buf)[1] = p->timestamp[1];
        (*buf)[2] = p->timestamp[2];
        *buf += 3;

        /* Security */
        if( adv_prim_2011 )
        {
            /* Security failure parameter is being updated accordingly */
			//*(*buf)++ = MAC_SECURITY_NOT_IN_ACL << 1;
			
			*(*buf)++ = pandesc->security;
#if MAC_CFG_SECURITY_ENABLED
//#ifdef MAC_CFG_SECURITY_ENABLED /*Umesh changed here*/
			*buf = append_sec_params( *buf, &pandesc->sec_params, key_id_length );
#else
			*(*buf)++ = 0;
#endif
		}
        else
        {
            /* 2003 Security */
            *(*buf)++ = MAC_SECURITY_NOT_IN_ACL << 1;
        }
    }
    return;
}
#endif

/******************************************************************************/            

/*
** =============================================================================
** Private Macro definitions
** =============================================================================
*/
	
#define START_CONFIRM_SIZE          2   /* primitive and status */

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

//#ifdef WISUN_FAN_MAC
//#ifdef TEST_CHOP
//static uint8_t synhro_problem = 0;
//static uint8_t rxed_in_next_chan = 0;
//static uint8_t rxed_in_prev_chan = 0;
//static uint8_t rxed_in_unknown_chan = 0;
//#endif
//#endif


/*
** ============================================================================
** Private Function Prototypes
** ============================================================================
*/

/* None */

/*
** ============================================================================
** Private Function Definitions
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

/* None */

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

#ifdef WISUN_FAN_MAC   
//uint8_t async_conf_buffer[20]={0};
uchar send_ws_async_confirm( uchar status , uchar frame_type)
{
  
    msg_t *msg = NULL;
    uchar *buf = NULL;

    msg = allocate_mac_2_nhle_msg( 20 );
    //msg = (msg_t*)async_conf_buffer;

    if( msg != NULL_POINTER )
    {
         buf = (uchar *) msg->data;

        *buf++ = WS_ASYNC_FRAME_REQ_CONF;
        *buf++ = status;
        *buf++ = frame_type;
        send_mlme_2_nhle( msg );
        return 1;
    }
    return 0;        
}

#endif


#if(CFG_MLME_START_REQ_CONF == 1)

#ifdef WISUN_FAN_MAC 
void send_mlme_set_mac_channel_info_confirm( uchar status )
{
   msg_t *msg = allocate_mac_2_nhle_msg( START_CONFIRM_SIZE );
    uint8_t* buf = NULL;

    if( msg != NULL_POINTER )
    {
        buf = (uint8_t*)msg->data;        
        *buf++  = MAC_MLME_SET_MAC_CHANNEL_INFO_CONFIRM;
        *buf = status;
        send_mlme_2_nhle( msg );
    }
} 

void send_mlme_set_fan_mac_confirm( uchar status, uchar subie_value )
{
 
   msg_t *msg = allocate_mac_2_nhle_msg( START_CONFIRM_SIZE+1 );
    uint8_t* buf = NULL;

    if( msg != NULL_POINTER )
    {
        buf = (uint8_t*)msg->data;        
        *buf++  = FAN_MAC_MLME_SET_CONFIRM;
        *buf++ = status;
        *buf = subie_value;
        send_mlme_2_nhle( msg );
    }
} 

#endif /*(end of WISUN_FAN_MAC)*/

#endif /*(CFG_MLME_START_REQ_CONF == 1)*/

/******************************************************************************/
#ifdef WISUN_FAN_MAC
void send_mlme_async_frame_indicaiton(uint8_t* src_mac_addr , uint8_t frame_type , uint8_t status)
{
    msg_t* msg = allocate_mac_2_nhle_msg(15);
    uchar *buf = NULL;       
    buf = (uchar *) msg->data;
    *buf++ = WS_ASYNC_FRAME_INDICATION;
    *buf++ = 0;
    *buf++ = frame_type;
    //*buf++ =8;        
    memcpy(buf,src_mac_addr,8);               
    send_mlme_2_nhle( msg );
}
#endif


