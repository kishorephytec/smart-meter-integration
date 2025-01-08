/** \file mac_frame_build.c
 *******************************************************************************
 ** \brief Provides APIs for building MAC frames
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
#include <math.h>
#include "queue_latest.h"
#include "buff_mgmt.h"
#include "list_latest.h"
#include "sw_timer.h"
#include "hw_tmr.h"
#include "timer_service.h"
#include "phy.h"
#include "mac_config.h"
#include "mac.h"
#include "mac_defs_sec.h"
#include "ie_element_info.h"
#include "mac_defs.h"
#include "sm.h"
//#include "pandesc.h"

#if(CFG_MAC_SFTSM_ENABLED == 1)
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
#include "fan_mac_ie.h"
#include "mac_mem.h"
#include "mac_pib.h"
#if(CFG_MAC_SFTSM_ENABLED == 1)  
#include "startsm.h"
#endif
#if(CFG_MAC_PENDADDR_ENABLED == 1)
#include "pendaddr.h"
#endif
#include "mac_security.h"
#include "mac_frame_build.h"

#include "tri_tmr.h"
#include "fan_sm.h"
#include "fan_mac_interface.h"

/*
** =============================================================================
** Private Macro definitions
** =============================================================================
*/

#define MAX_HDR_IE_LIST_SIZE				200
#define MAXIMUM_SLOT_DH51_FUCTION                       65536
#define MAC_FRAME_CONTROL_FIELD_LEN                 2


/*
** =============================================================================
** Private Structures, Unions & enums Type Definitions
** =============================================================================
**/
/*Umesh : 15-01-2018 here is no need of this enum its required only in fan mac but temp it req. it will removed after last clean*/
enum
{
    DEV_NOT_ADDED = 0x00,
    DEV_ADDED = 0x01,
    ACK_REQUIRED = 0x02,
    ACK_RECEIVED = 0x03
};
/*Umesh : 15-01-2018 moved to fan_mac_frame_build.c file*/
//typedef struct seq_no
//{
//    uint8_t src_addr[8];
//    uint8_t dest_addr[8];
//    uint8_t dev_join_status;
//    uint8_t dev_ack_status;
//}seq_no_t ;

/*
** ============================================================================
** Private Variable Definitions
** ============================================================================
*/



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

//extern seq_no_t seq_number_table[MAX_NBR_SUPPORT];

/*Umesh : 15-01-2018*//*this varriable should be in fan_mac_frame_build.c*/
//Raka 
#ifdef PANA_FOR_SINGLE_HOP_HAN
extern uint8_t force_ie_list_in_frame_control;
#endif
extern uchar heap[];
extern mac_pib_t mac_pib;
extern uchar aExtendedAddress[8];
extern trxsm_t *trxsm_p;

#if(FAN_FRQ_HOPPING_FEATURE_ENABLED == 1)
extern uint16_t broadcast_slot_nuumber;
#endif

/* STARTSM is needed to access pending values */
#if(CFG_MAC_SFTSM_ENABLED == 1)  
extern startsm_t *startsm_p;
#endif
#if(CFG_MAC_SCANSM_ENABLED == 1)  
extern scansm_t scansm;
#endif

#ifdef MAC_CFG_SECURITY_ENABLED
extern const uint8_t  m_lut[4];
#endif

#ifdef WISUN_FAN_MAC
extern self_info_fan_mac_t mac_self_fan_info;
extern fan_mac_param_t fan_mac_params;
extern uint8_t key_id_index;
#if(FAN_EDFE_FEATURE_ENABLED == 1)
extern edfe_info_t edfe_information;  
#endif
#endif

#ifdef WISUN_FAN_MAC 
extern uint8_t* extract_ufsi_and_pkt_type(uint8_t *ptr, uint16_t length);
extern  uint8_t* extract_bfsi(uint8_t *ptr, uint16_t length);
extern uint32_t update_ufsi_with_procces_time(mac_tx_t *out_packt);
uint32_t update_bfio_with_procces_time(mac_tx_t *out_packt);
extern uint8_t* extract_vender_ie(uint8_t *ptr, uint16_t length);

#if(FAN_FRQ_HOPPING_FEATURE_ENABLED == 1)
extern uint16_t broadcast_slot_nuumber;
#endif

#endif

#if(CFG_MAC_LBTSM_ENABLED == 1)
extern lbtsm_t lbtsm;/*Umesh : 21-02-2018*//*for sniffer N WITHOUT FAN-MAC*/
#endif

uint64_t get_time_now_64 (void);
extern void * app_bm_alloc( uint16_t length );
extern void app_bm_free( uint8_t *pMem );
extern p3time_t timer_current_time_get(void);

#ifdef WISUN_FAN_MAC
extern  uint8_t append_security_parameters(uint8_t** p_to, security_params_t* pSecstuff );
extern mac_status_t mac_prim_parse_security( uchar **, security_params_t *, uchar );
uint16_t build_ie (uint8_t *buf, uint8_t type, uint32_t ie_bitmap, uint32_t sub_ie_bitmap, void *data);
#endif

/*
** ============================================================================
** Private Function Prototypes
** ============================================================================
*/

#ifdef MAC_CFG_SECURITY_ENABLED
extern uint8_t get_MIC_length( security_params_t *p );
extern mac_status_t set_sec_fields( mac_tx_t*, uchar, security_params_t* );	
extern int add_aux_sec_header( uchar, uchar*, security_params_t* );
extern void set_sec_fc( uchar, uchar* );
#if(FAN_FRQ_HOPPING_FEATURE_ENABLED == 1)
extern uint16_t broadcast_slot_nuumber;
#endif

#endif

/*
** ============================================================================
** Public Function Prototypes
** ============================================================================
*/
static uint8_t check_pan_id_status(uchar frame_type,uchar tx_options,mac_address_t *src, mac_address_t *dst, gboolean *dstPanPresent,gboolean *srcPanPresent,gboolean *pan_id_compression);        

mac_status_t mac_frame_build( mac_tx_t**, uchar, uchar, mac_address_t*, mac_address_t*,
                                     uchar, uchar,uchar*, uint16_t, uchar, uint32_t/*IE_ids_list_t**/, phy_data_params_t*,security_params_t*, uint8_t );

/*
** ============================================================================
** Public Function Definitions
** ============================================================================
*/

mac_status_t mac_frame_build_data( 
                                  mac_tx_t** p, 
                                  mac_address_t *src, 
                                  mac_address_t *dst, 
                                  uchar tx_options,
                                  uchar fc_options,
                                  uchar *payload, 
                                  uint16_t payload_length,
                                  uchar enc_offset,
                                  IE_ids_list_t* p_hdr_ie_ids,
                                  phy_data_params_t* phy_params,
                                  security_params_t *sec_params
                                  )
{
    
	return (mac_frame_build( p,
                                MAC_FRAME_TYPE_DATA,
                                MAC_FRAME_BCN_SUB_TYPE_DONT_CARE,
                                src,
                                dst,
                                tx_options,
                                fc_options,
                                payload,
                                payload_length,
                                enc_offset,
                                0,//p_hdr_ie_ids,
                                phy_params,
                                sec_params,0 ));
}

/******************************************************************************/
#if(CFG_MLME_ASSOCIATE_REQ_CONF == 1)
/* build an association request frame for transmission */
mac_status_t mac_frame_build_association_request( 
                                                  mac_tx_t** p,
                                                  uchar capinfo,
                                                  mac_address_t *dst,
                                                  phy_data_params_t* phy_params,
                                                  security_params_t *sec_param 
                                                )
{
  mac_address_t src= {0};
  uchar payload[2] = {0};

    /* setup source address */
    src.address_mode = MAC_IEEE_ADDRESS;
    src.pan_id = ( mac_pib.ConfigFlags & USE_PIB_PANID_FOR_ASSOCIATION ) ?
                   mac_pib.PANId : BROADCAST_PAN_ID;
    src.address.ieee_address = aExtendedAddress;

    payload[0] = ASSOCIATION_REQUEST;
    payload[1] = capinfo;

    return ( mac_frame_build( p,
                             MAC_FRAME_TYPE_MAC_COMMAND,
                             MAC_FRAME_BCN_SUB_TYPE_DONT_CARE,
                             &src,
                             dst,
                             ACKNOWLEDGED_TRANSMISSION,
                             0,
                             payload,
                             sizeof(payload),
                             1,
                             NULL,
                             phy_params,
                             sec_param,0 
                             ));

    
}
#endif	/*(CFG_MLME_ASSOCIATE_REQ_CONF == 1)*/

/******************************************************************************/
#if(CFG_MLME_ASSOCIATE_IND_RESP == 1)
/*build an association response frame for transmission */
mac_status_t mac_frame_build_association_response( 
                                                  mac_tx_t** p,
                                                  mac_address_t *src,
                                                  mac_address_t *dst,
                                                  uchar *short_addr,
                                                  uchar status,
                                                  security_params_t *sec_param 
                                                 )
{
    //mac_status_t ret;
        uchar payload[4] = {0};
        phy_data_params_t phy_params = {0};
	ulong current_channel = 0;
	uint16_t len = 0;
	PLME_get_request( phyCurrentChannel, &len, &current_channel );
	phy_params.tx_channel = (uint8_t)current_channel;  

        payload[0] = ASSOCIATION_RESPONSE;
        payload[1] = *short_addr;      /* low byte short address */
        payload[2] = *(short_addr+1);  /* high byte short address */
        payload[3] = status;           /* association status */

        return ( mac_frame_build( 
                                    p,
                                    MAC_FRAME_TYPE_MAC_COMMAND,
                                    MAC_FRAME_BCN_SUB_TYPE_DONT_CARE,
                                    src,
                                    dst,
                                    ACKNOWLEDGED_TRANSMISSION,
                                    0,
                                    payload,
                                    sizeof(payload),
                                    1,
                                    NULL,
                                    &phy_params,
                                    sec_param ,0
						));

    //return ret;
}
#endif	/*(CFG_MLME_ASSOCIATE_IND_RESP == 1)*/

/******************************************************************************/
#if(CFG_MLME_DISASSOCIATE_REQ_CONF == 1)
/*build an disassociation notification frame for transmission */
mac_status_t mac_frame_build_disassociation_notification( 
                                                              mac_tx_t** p,
                                                              mac_address_t *src,
                                                              mac_address_t *dst,
                                                              uchar reason,
                                                              security_params_t *sec_param 
							)
{
    //mac_status_t ret;
        uchar payload[2] = {0};
	
	phy_data_params_t phy_params = {0};
	ulong current_channel = 0;
	uint16_t len = 0;
	PLME_get_request( phyCurrentChannel, &len, &current_channel );
	phy_params.tx_channel = (uint8_t)current_channel;

    src->address_mode = MAC_IEEE_ADDRESS;
    src->pan_id = dst->pan_id;
    src->address.ieee_address = aExtendedAddress;


    payload[0] = DISASSOCIATION_NOTIFICATION;
    payload[1] = reason;

    return ( mac_frame_build( 
                                    p,
                                    MAC_FRAME_TYPE_MAC_COMMAND,
                                    MAC_FRAME_BCN_SUB_TYPE_DONT_CARE,
                                    src,
                                    dst,
                                    ACKNOWLEDGED_TRANSMISSION,
                                    0,
                                    payload,
                                    sizeof(payload),
                                    1,
                                    NULL,
                                    &phy_params,
                                    sec_param,0 
						 ));

    //return ret;
}
#endif	/*(CFG_MLME_DISASSOCIATE_REQ_CONF == 1)*/
/******************************************************************************/

/*build an data request command frame for transmission*/
mac_status_t mac_frame_build_data_request( 
                                          mac_tx_t** p,
                                          mac_address_t *src,
                                          mac_address_t *coord,
                                          security_params_t *sec_param,
                                          uchar cmd_id )
{
    //mac_status_t ret;
        uchar payload = 0;
        //uchar payload_len = 1;
        uchar tx_options = 0;
        uchar fc_options = 0;

        phy_data_params_t phy_params = {0};
        ulong current_channel = 0;
        uint16_t len = 0;
        PLME_get_request( phyCurrentChannel, &len, &current_channel );
        phy_params.tx_channel = (uint8_t)current_channel;
	
	
	if( cmd_id == DATA_REQUEST )
	{
		tx_options = ACKNOWLEDGED_TRANSMISSION;
		payload = DATA_REQUEST;
	}
	else
	{
		//tx_options &= (FRAME_VERSION_2011);
		tx_options |= FRAME_VERSION_2011;
		fc_options &= ~(MAC_IE_LIST_PRESENT);
		payload = LE_RIT_DATA_REQUEST;
	}

    

    return ( mac_frame_build( 
                            p,
                            MAC_FRAME_TYPE_MAC_COMMAND,
                            MAC_FRAME_BCN_SUB_TYPE_DONT_CARE,
                            src,
                            coord,
                            tx_options,
                            fc_options,
                            &payload,
                            1,//payload_len,
                            1,// changed from 0 to 1 to exclude the command id field from being secured
                            NULL,
                            &phy_params,
                            sec_param,0 
                            ));

    //return ret;
}
/******************************************************************************/

/*build an beacon request command frame for transmission */
mac_status_t mac_frame_build_beacon_request( mac_tx_t** p, uchar br_type )
{
    //mac_status_t ret;
        mac_address_t src = {0}, dst = {0};
#if(CFG_MAC_SCANSM_ENABLED == 1)          
	scan_param_t* p_scan_params = &scansm.param;
#endif        
	uchar tx_options = 0;
	uchar fc_options = 0;
	ushort payload_length = 1;//including command id field of the 
	
        uchar payload[50] = {0};
        uchar* pld = payload;
        phy_data_params_t phy_params = {0};
//        IE_ids_list_t* p_hdr_ie_ids = NULL;
        IE_ids_list_t hdr_ie_ids = {0};
        ulong current_channel = 0;
        uint16_t len = 0;
        PLME_get_request( phyCurrentChannel, &len, &current_channel );
        phy_params.tx_channel = (uint8_t)current_channel;

    src.address_mode = MAC_NO_ADDRESS;

    dst.address_mode = MAC_SHORT_ADDRESS;
    dst.address.short_address = dst.pan_id = BROADCAST_PAN_ID;
    //dst.address.short_address = BROADCAST_SHORT_ADDRESS;

	if ( br_type == 1 )
	{
		/* Explicitly insert dest addressing fields ( as 0xFFFF ) if implicit 
		broadcast is set to FALSE */
		/*dst.address_mode = ( (mac_pib.ImplicitBroadcast)?
		( MAC_NO_ADDRESS ) :( MAC_SHORT_ADDRESS) );*/
		if(mac_pib.ImplicitBroadcast)
		{
			dst.address_mode = MAC_NO_ADDRESS;
		}
		if( mac_pib.ShortAddress == SHORT_ADDRESS_UNKNOWN )
		{
			/*While scanning when the device is not yet associated with a PAN */
			//src.address_mode = MAC_NO_ADDRESS;
		}
		else if ( mac_pib.ShortAddress == USE_IEEE_ADDRESS )
		{
			/*Scanning when the device is associated with a PAN and using a extended 
			address for communication */
			src.pan_id = mac_pib.PANId;
			src.address_mode = MAC_IEEE_ADDRESS;
			src.address.ieee_address = aExtendedAddress;
		}
		else
		{
			/*Scanning when the device is associated with a PAN and using a extended 
			address for communication */
			src.address_mode = MAC_SHORT_ADDRESS;
			src.pan_id = mac_pib.PANId;
			src.address.short_address = mac_pib.ShortAddress;
		}
#if(CFG_MAC_SCANSM_ENABLED == 1)  		
		/*EBR*/
		memcpy(hdr_ie_ids.ie_list,p_scan_params->hdr_ie_list,p_scan_params->hdr_ies_cnt);
		hdr_ie_ids.ie_list_len = p_scan_params->hdr_ies_cnt;
#endif		 
		hdr_ie_ids.ie_flags = ( INCLUDE_TIE|IE_REQUEST );
		//p_hdr_ie_ids = &hdr_ie_ids;
#ifdef WISUN_ENET_PROFILE
          len = 	build_ie_list
			(	
				pld,
				IE_TYPE_PLD,
				p_scan_params->pld_ies_cnt,
				p_scan_params->pld_ie_list,
				(INCLUDE_TIE)
			);
                if(len)
                {
                  pld += len; 
                  fc_options |= MAC_IE_LIST_PRESENT;
                }
                else
                {
                  fc_options &= ~(MAC_IE_LIST_PRESENT);
                }
                
                //fc_options = p_scan_params->fc_options;
			/*Since IEs will be present in the frame being built, indicate that 
			the IE List present bit should be enabled in the frame control */
			tx_options |= FRAME_VERSION_2011;
			

                payload_length += pld - payload;
          
#else          
//		payload_length += build_ie_desc_list
//							(
//								pld,
//								IE_TYPE_PLD, 
//								p_scan_params->pld_ies_cnt, 
//								p_scan_params->pld_ie_list,
//								INCLUDE_TIE 
//							);
			
			
		pld += (payload_length -1);
		
		if( pld - payload )
		{
#if(CFG_MAC_SCANSM_ENABLED == 1)                    
			fc_options = p_scan_params->fc_options;
#endif                        
			/*Since IEs will be present in the frame being built, indicate that 
			the IE List present bit should be enabled in the frame control */
			tx_options |= FRAME_VERSION_2011;
			fc_options |= MAC_IE_LIST_PRESENT;			
		}
		else
		{
			
			if( !( hdr_ie_ids.ie_list_len ) )
			{
				/*to make as a normal beacon*/
#ifndef WISUN_ENET_PROFILE
				src.address_mode = MAC_NO_ADDRESS;
				tx_options &= ~(FRAME_VERSION_2011);
				fc_options &= ~(MAC_IE_LIST_PRESENT);
#endif
			}
		}
          
#endif             
	}
	
	*pld++ = BEACON_REQUEST;

    return ( mac_frame_build( 
                                      p,
                                      MAC_FRAME_TYPE_DATA,
                                      br_type,
                                      &src,
                                      &dst,
                                      tx_options,
                                      fc_options,
                                      payload,
                                      payload_length,//includes everything which is after header IEs
                                      payload_length,// issue
                                      0,//p_hdr_ie_ids,
                                      &phy_params,
                                      NULL_POINTER,0 
						 ));
    //return ret;
}

/******************************************************************************/
#if((CFG_MLME_ORPHAN_IND_RESP == 1) || (CFG_MLME_START_REQ_CONF == 1))
/*build an coordinator realignment command frame for transmission*/
mac_status_t mac_frame_build_coordinator_realignment( 
                                                      mac_tx_t** p,
                                                      mac_address_t *src,
                                                      mac_address_t *dst,
                                                      uchar *ieee_addr,
                                                      ushort short_addr,
                                                      uchar channel,
                                                      uchar channel_page,
                                                      security_params_t *sec_param 
										 )
{
    //mac_status_t ret;
    mac_address_t my_src = {0}, my_dst = {0};
    uchar payload[9] = {0};
    uchar length = 0;
    uchar tx_options = 0;
    uint32_t attr_value = 0;
	uint16_t len = 0;

	phy_data_params_t phy_params  = {0};
	
    /* Command frame identifier */
    payload[length++] = COORDINATOR_REALIGNMENT;

    /* PAN Id */
    if( ieee_addr )
    {
        /* called as a response to orphan notification */
        payload[length++] = LOW_BYTE( mac_pib.PANId );
        payload[length++] = HIGH_BYTE( mac_pib.PANId );
    }
    else
    {
        /* called as a result of MLME-START.request */
        /*TBD Make sure startsm_p is not NULL_POINTER */
#if(CFG_MAC_SFTSM_ENABLED == 1)        
        payload[length++] = LOW_BYTE( startsm_p->pending.pan_id );
        payload[length++] = HIGH_BYTE( startsm_p->pending.pan_id );
#endif        
    }

    /* coordinator short address */
    payload[length++] = LOW_BYTE( mac_pib.ShortAddress );
    payload[length++] = HIGH_BYTE( mac_pib.ShortAddress );

    /* logical channel */
    /* tx_channel */
	phy_params.tx_channel = payload[length++] = channel;

	/* called as a result of MLME-START.request for channel change,it
	needs to be broadcasted on the current channel itself */
	PLME_get_request( phyCurrentChannel, &len, &attr_value );
	
	if ( channel != attr_value )
	{
		/* tx_channel */
		phy_params.tx_channel = attr_value;
	}
	/*else
	{
		
		phy_params.tx_channel = channel;
	}*/

    /* short address */
    if( ieee_addr )
    {
        /* called as a response to orphan notification */
        payload[length++] = LOW_BYTE( short_addr );
        payload[length++] = HIGH_BYTE( short_addr );
    }
    else
    {
        /* called as a result of MLME-START.request */
        payload[length++] = 0xff;
        payload[length++] = 0xff;
    }

    /* channel page */
    /*TBD should be: if(pending_channel_page != channel_page)
        probably checked along with security */
    //if( mac_pib.ConfigFlags & USE_2006_PRIMITIVES &&
    //    add_channel_page )
    //PLME_get_request( phyCurrentPage, &len, &attr_value );
    //if( channel_page != (attr_value & 0xff) )
    //{
        payload[length++] = channel_page;
        tx_options |= FRAME_VERSION_2006;
    //}

    /* set addresses if called as a result of MLME-START.request */
    if( !ieee_addr )
    {
        /* set packet source address */
        my_src.address_mode = MAC_IEEE_ADDRESS;
        my_src.pan_id = mac_pib.PANId;
        my_src.address.ieee_address = aExtendedAddress;

        /* set packet destination address */
        my_dst.address_mode = MAC_SHORT_ADDRESS;
        my_dst.address.short_address = my_dst.pan_id = BROADCAST_PAN_ID;
        //my_dst.address.short_address = BROADCAST_SHORT_ADDRESS;

        /* use local addresses */
        src = &my_src;
        dst = &my_dst;
    }

    /* transmit options */
    tx_options |= ieee_addr ? ACKNOWLEDGED_TRANSMISSION : 0;

    /* Check if we are trying to secure a frame when security is not enabled */
    if( ( sec_param ) &&
        ( sec_param->security_level != MAC_SECURITY_NONE ) &&
        ( mac_pib.mac_security_enabled == FALSE ) &&
        ( mac_pib.ConfigFlags & USE_2006_PRIMITIVES ) )
    {
        *p = NULL_POINTER;
        return MAC_UNSUPPORTED_SECURITY;
    }

    /* Create the packet */
    return ( mac_frame_build(
                                  p,
                                  MAC_FRAME_TYPE_MAC_COMMAND,
                                  MAC_FRAME_BCN_SUB_TYPE_DONT_CARE,
                                  src,
                                  dst,
                                  tx_options,
                                  0,
                                  payload,
                                  length,
                                  1,
                                  NULL,
                                  &phy_params,
                                  sec_param,0 
                                                    ));
    //return ret;
}
#endif	/*((CFG_MLME_ORPHAN_IND_RESP == 1) || (CFG_MLME_START_REQ_CONF == 1))*/

/******************************************************************************/

/*build an panid conflict command frame for transmission */
mac_status_t mac_frame_build_panid_conflict_notification( 
                                                                    mac_tx_t** p,
                                                                    mac_address_t *src,
                                                                    mac_address_t *dst,
                                                                    security_params_t *sec_param 
														)
{
    
    uchar payload = PAN_ID_CONFLICT_NOTIFICATION;

    phy_data_params_t phy_params = {0};
#if(CFG_MAC_SCANSM_ENABLED == 1)  	
    phy_params.tx_channel = (uint8_t)scansm.channel0.channel;
#endif
    dst->address_mode = src->address_mode = MAC_IEEE_ADDRESS;
    
    dst->pan_id = src->pan_id = mac_pib.PANId;

    src->address.ieee_address = aExtendedAddress;
    dst->address.ieee_address = mac_pib.CoordExtendedAddress;


   return ( mac_frame_build( 
                                      p,
                                      MAC_FRAME_TYPE_MAC_COMMAND,
                                      MAC_FRAME_BCN_SUB_TYPE_DONT_CARE,
                                      src,
                                      dst,
                                      ACKNOWLEDGED_TRANSMISSION,
                                      0,
                                      &payload,
                                      sizeof(payload),
                                      1,
                                      NULL,
                                      &phy_params,
                                      sec_param,0 
						  ));

    
}

/******************************************************************************/
#if	(CFG_ORPHAN_SCAN == 1)
/*build an orphan notification command frame for transmission*/
mac_status_t mac_frame_build_orphan_notification( 
													mac_tx_t** p,
													security_params_t *sec_param 
												)
{
    //mac_status_t ret;
        mac_address_t src = {0}, dst = {0};
        uchar payload[1] = {0};
	phy_data_params_t phy_params = {0};
	ulong current_channel = 0;
	uint16_t len = 0;
	PLME_get_request( phyCurrentChannel, &len, &current_channel );
	phy_params.tx_channel = (uint8_t)current_channel;

    dst.address_mode = MAC_SHORT_ADDRESS;
    dst.pan_id = src.pan_id = dst.address.short_address = BROADCAST_SHORT_ADDRESS;
    //dst.pan_id = BROADCAST_PAN_ID;

    src.address_mode = MAC_IEEE_ADDRESS;
    src.address.ieee_address = aExtendedAddress;
    //src.pan_id = BROADCAST_PAN_ID;

    payload[0] = ORPHAN_NOTIFICATION;

    return ( mac_frame_build( 
                                    p,
                                    MAC_FRAME_TYPE_MAC_COMMAND,
                                    MAC_FRAME_BCN_SUB_TYPE_DONT_CARE,
                                    &src,
                                    &dst,
                                    0,
                                    0,
                                    payload,
                                    sizeof(payload),
                                    1,
                                    NULL,
                                    &phy_params,
                                    sec_param,0 
						  ));

    //return ret;
}

#endif	/*(CFG_ORPHAN_SCAN == 1)*/

/******************************************************************************/

mac_status_t mac_frame_build_beacon(
                                          mac_tx_t **txp, /*place to store address of tx packet */
                                          uchar sub_type,
                                          mac_address_t *dest,
                                          uchar* p_hdr_ie_ids,
                                          uchar* p_pld_ie_ids
								   )
{
          mac_address_t src = {0}, dst = {0};
          //mac_status_t ret;
          uchar payload[100] = {0};
          uchar dest_address[8] = {0};
          uint16_t payload_length = 0;
          uint16_t available_length = 0;     /* number of bytes available for pended addresses, GTS etc */
          uchar *p = NULL;
          ushort len = 0x00;
          phy_data_params_t phy_params = {0};
          uint8_t enc_offset = 0;
          uchar tx_options = 0;
          uchar fc_options = 0;
          security_params_t *sec_params = NULL;
//          IE_ids_list_t* p_hdr_ies = NULL;
          IE_ids_list_t hdr_ie_ids = {0};
#if(CFG_MAC_SFTSM_ENABLED == 1)            
          startsm_param_t* pt_pending = &(startsm_p->pending);      
          phy_params.tx_channel = startsm_p->pending.logical_channel;
          src.pan_id = pt_pending->pan_id;
#endif  	
          available_length = aMaxPHYPacketSize - 9;

          /*TBD Make sure startsm_p is not NULL_POINTER */
          

    //if( ( MAC_FRAME_BCN_SUB_TYPE_RB == sub_type ) || ( MAC_FRAME_BCN_SUB_TYPE_MPM_EB == sub_type ) )
	//{
		/* Source address */
		if ( mac_pib.ShortAddress == USE_IEEE_ADDRESS )
		{
			src.address_mode = MAC_IEEE_ADDRESS;
			src.address.ieee_address = aExtendedAddress;
		}
		else
		{
			src.address_mode = MAC_SHORT_ADDRESS;
			src.address.short_address = mac_pib.ShortAddress;
		}
		
		dst.pan_id = dst.address_mode = MAC_NO_ADDRESS;
		//dst.pan_id = MAC_NO_ADDRESS;

	//}
	
	if ( MAC_FRAME_BCN_SUB_TYPE_RB == sub_type )
	{
		/*construct the regular beacons payload part*/
		/*TBD Make sure startsm_p is not NULL_POINTER */
#if(CFG_MAC_SFTSM_ENABLED == 1)            
		payload[0] = pt_pending->beacon_order | (pt_pending->superframe_order << 4);
#endif                
		payload[1] = mac_data.final_cap_slot;
#if (CFG_MAC_STARTSM_ENABLED == 1)  
		if( ( startsm_get_flags( startsm_p ) & STARTSM_FLAG_COORD_MASK ) == STARTSM_FLAG_PANCOORD )
		{
			payload[1] |= SF1_PAN_COORDINATOR;
		}
#endif
		if( mac_pib.AssociationPermit != 0 )
		{
			payload[1] |= SF1_ASSOCIATION_PERMIT;
		}
#if(CFG_MAC_SFTSM_ENABLED == 1)  
		if( pt_pending->battery_life_ext != 0 )
		{
			payload[1] |= SF1_BATTERY_LIFE_EXTENSION;
		}
#endif
		/* NB available length current min = 87 */

		/* point at the GTS fields */
		p = &payload[2];

		/* now update the GTS fields in the beacon */
	#ifdef MAC_CFG_GTS_ENABLED

		/* GTS is only permitted for beacon enabled PANs */
		/*TBD Make sure startsm_p is not NULL_POINTER */
		if( pt_pending->beacon_order != 15 )
		{
			p = mac_update_beacon_gts(p);
			available_length -= 21;
		}
		else
		{
			*p++ = 0;
		}
	#else
		*p++ = 0;
	#endif

		/* available length current min = 87-7*3 = 66 */
		/* Add length of GTS fields to header length */
		available_length -= mac_pib.BeaconPayloadLength;
		/* available length current min = 66 - 52 = 14 ie must be > 0 */

#if(CFG_MAC_PENDADDR_ENABLED == 1)                
		/* add Pending Address List */
		p += pendaddr_get( p, available_length );
#endif                

		/* Copy in beacon payload */
		//memcpy(p, mac_pib.BeaconPayload, mac_pib.BeaconPayloadLength);
		//p += mac_pib.BeaconPayloadLength;
		payload_length += mac_pib.BeaconPayloadLength;
		
        enc_offset = p - payload;
		
	}
	else if ( MAC_FRAME_BCN_SUB_TYPE_MPM_EB == sub_type )
	{
		tx_options |= FRAME_VERSION_2011;
		fc_options |= MAC_IE_LIST_PRESENT;
		p = &payload[0];
#if(CFG_MAC_SFTSM_ENABLED == 1) 
		/* construct the MPM EBs payload IE List section */
		p += build_ie_list
			(	
				p,
				IE_TYPE_PLD, 
				pt_pending->mpm_ie_list_len,
				pt_pending->mpm_ie_list,			
                                (INCLUDE_TIE)
			);
#endif		
	}
	else if ( MAC_FRAME_BCN_SUB_TYPE_EB == sub_type )
	{
		tx_options |= FRAME_VERSION_2011;
		//fc_options |= MAC_IE_LIST_PRESENT;
		p = &payload[0];
		
		dst.address_mode = dest->address_mode;
		dst.pan_id = dest->pan_id;
		if( dst.address_mode == MAC_NO_ADDRESS )
		{
			dst.address.short_address = SHORT_ADDRESS_UNKNOWN;
		}
		else if( dst.address_mode == MAC_SHORT_ADDRESS )
		{
			dst.address.short_address = dest->address.short_address;
		}
		else if ( dst.address_mode == MAC_IEEE_ADDRESS )
		{
			memcpy(dest_address,dest->address.ieee_address,8);
			dst.address.ieee_address =  dest_address;
               tx_options |= ACKNOWLEDGED_TRANSMISSION;
		}

		if( ( p_hdr_ie_ids != NULL ) && ( p_hdr_ie_ids[0] ))
		{
			/* There are some IEs */
			memcpy( hdr_ie_ids.ie_list, &p_hdr_ie_ids[1],p_hdr_ie_ids[0] );
			hdr_ie_ids.ie_list_len = p_hdr_ie_ids[0];
			hdr_ie_ids.ie_flags = INCLUDE_TIE;
//			p_hdr_ies = &hdr_ie_ids;
		}
                if(  p_pld_ie_ids != NULL )
                {
                   len = 	build_ie_list
                                (	
                                  p,
                                  IE_TYPE_PLD,
                                  p_pld_ie_ids[0],
                                  &p_pld_ie_ids[1],
                                  (INCLUDE_TIE)
                                );

                }
                if(len)
                {
                  p += len; 
                  fc_options |= MAC_IE_LIST_PRESENT;
                }
                else
                {
                  fc_options &= ~(MAC_IE_LIST_PRESENT);
                }
	}

	payload_length += p - payload;

	if( (mac_pib.mac_security_enabled) && ( mac_data.security ) )
	{
		if( ( MAC_FRAME_BCN_SUB_TYPE_RB == sub_type ) ||
			( MAC_FRAME_BCN_SUB_TYPE_EB == sub_type ))
		{
			sec_params = &mac_data.security->beacon_sec_param;
		}
		else if (MAC_FRAME_BCN_SUB_TYPE_MPM_EB == sub_type)
		{
			sec_params = &mac_data.security->mpm_eb_sec_param;
		}
	}
    
    return ( mac_frame_build( 
                                    txp,
                                    MAC_FRAME_TYPE_BEACON,
                                    sub_type,
                                    &src,
                                    &dst,
                                    tx_options,
                                    fc_options,
                                    payload,
                                    payload_length,
                                    enc_offset,//payload_length - mac_pib.BeaconPayloadLength,
                                    0,//p_hdr_ies,
                                    &phy_params,
                                    /* Anand: Include this when implementing beacon security*/
                                    sec_params,0
                                    //NULL_POINTER
                                                            ));
//end:
   // return ret;
}


/******************************************************************************/
#ifdef ENHANCED_ACK_SUPPORT
mac_status_t mac_frame_build_enhanced_ack
(
	mac_tx_t **txp,
	mac_rx_t *rxp
)
{
	uchar tx_options = 0;
	uchar fc_options = 0;
	security_params_t *sec_param = NULL;
	uint8_t payload[10] = {0};
	uint16_t payload_len = 0;
	
	IE_ids_list_t hdr_ie_ids = {0};
	IE_ids_list_t* p_hdr_ie_ids =  &hdr_ie_ids;
	
	phy_data_params_t phy_params = {0};
	ulong current_channel = 0;
	uint16_t len = 0;
	PLME_get_request( phyCurrentChannel, &len, &current_channel );
	phy_params.tx_channel = (uint8_t)current_channel;
	
	tx_options |= FRAME_VERSION_2011;
	// check if you have LE and RIT enabled and then put the packet in the indirect Q
	if( mac_pib.LEenabled && mac_pib.RITPeriod )
	{
		tx_options |= INDIRECT_TRANSMISSION; // needed for putting the enack in the indirect to be pulled by RIT enabled node
	}
		
	fc_options &= ~(MAC_IE_LIST_PRESENT);
	
	
	//dst.address_mode = MAC_SHORT_ADDRESS;
	//dst.pan_id = src.pan_id = dst.address.short_address = BROADCAST_SHORT_ADDRESS;
	////dst.pan_id = BROADCAST_PAN_ID;

	//src.address_mode = MAC_IEEE_ADDRESS;
	//src.address.ieee_address = aExtendedAddress;
	////src.pan_id = BROADCAST_PAN_ID;
	
	
	hdr_ie_ids.ie_list_len = 0;
	//hdr_ie_ids.ie_list
	
	
	hdr_ie_ids.ie_flags = ( INCLUDE_TIE );
		
	   
	return ( mac_frame_build( 
			   txp,
                           MAC_FRAME_TYPE_ACK,
			   MAC_FRAME_BCN_SUB_TYPE_DONT_CARE,                           
                           &(rxp->dst),
			   &(rxp->src),
                           tx_options,
                           fc_options,
                           NULL,
                           0,
                           0,
			   p_hdr_ie_ids,
			   &phy_params,
			   NULL_POINTER,
				rxp->sn
                         ));			 			 			 			 			 			
}
#endif /*#ifdef ENHANCED_ACK_SUPPORT*/								  
/******************************************************************************/ 
//uint8_t debug_sun_arr[2] = {0x00};
 mac_status_t mac_frame_build( 
                               mac_tx_t **p, /* place to store the address of resulting packet */
                               uchar type, /* type of request      */
                               uchar sub_type,
                               mac_address_t *src, /* source address       */
                               mac_address_t *dst, /* destination address  */
                               uchar tx_options,   /* transmit options     */
                               uchar fc_options,   /* frame control options */
                               uchar *payload, /* pointer to payload   */
                               uint16_t payload_length,   /* length of payload    */
                               uchar enc_offset,
//                               IE_ids_list_t* p_hdr_ie_ids,
                               uint32_t sub_hdr_bitmap,
                               phy_data_params_t *phy_params,
                               security_params_t *sec_params,
                               uint8_t sn
                            )
{
  mac_tx_t *txd = NULL;
  uchar *bp = NULL;
  uchar* hdr_ie = NULL;
  uint16_t length = 0;
  mac_status_t err = MAC_SUCCESS;
  uchar secured = 0;
  uint16_t alloc_len = 0;
  uint16_t ie_field_len = 0;
  uint16_t iCmdIdInPld = enc_offset - 1;//take a back up of the index info before the enc_offset is changed
  uchar fcs_len = 0;
  uint8_t* ufsi_ptr = NULL;
  uint8_t* bfsi_ptr = NULL;
  gboolean   dstPanPresent = FALSE;
  gboolean   srcPanPresent = FALSE;
  gboolean   pan_id_compression = FALSE;
     
    if( ((mac_pib.mac_security_enabled) && ( mac_data.security )) && sec_params && sec_params->security_level != MAC_SECURITY_NONE )
    {
        secured = 1;
        
         /*set the tx options with the info that the packet is being secured and 
        sent. All packets introduced in IEEE 802.15.4-2006 should have 
        FRAME_VERSION_2006 version. Secured packets should bear this frame version. 
        Refer:  Table 3a—Possible values of the Frame Version field in D5 802.15.4e spec*/
//        if( (tx_options & FRAME_VERSION_MASK) != FRAME_VERSION_2011 )
//        {
////        	tx_options |= FRAME_VERSION_2006;	
//        }
        
        /* check if the frame counter wrapped */
        if( mac_data.security->pib.mac_frame_counter == 0xffffffffUL )
        {
            *p = NULL_POINTER;
            return MAC_COUNTER_ERROR;
        }
    }
    
    /* 2 bytes of control fields + DSN */
    if( fc_options & MAC_SEQ_NUM_SUPPRESSION)
    {
          length = 2;  
    }
    else
    {
          /* 2 bytes of control fields + DSN */
          length = 2 + 1;
    }
    
    length += check_pan_id_status(type,tx_options,src,dst,&dstPanPresent,&srcPanPresent,&pan_id_compression);   
    /* now the address fields */
//    if (src != NULL_POINTER)
//    {
//        switch (src->address_mode)
//        {
//          case MAC_NO_ADDRESS:
//              break;
//
//          case MAC_SHORT_ADDRESS:
//              /* pan id and short address */
//              length += 2 + 2;
//              break;
//
//          case MAC_IEEE_ADDRESS:
//            /* pan id and ieee address */
//            if(tx_options & MAC_INTRA_PAN)
//              length += 8;
//            else
//              length += 2 + 8;
//            //length +=  8;
//              break;
//        }
//    }
//    
//    if (dst != NULL_POINTER)
//    {
//      switch (dst->address_mode)
//      {
//      case MAC_NO_ADDRESS:
//        break;
//        
//      case MAC_SHORT_ADDRESS:
//        /* pan id and short address */
//        length += 2 + 2;
//        break;
//        
//      case MAC_IEEE_ADDRESS:
//        /* pan id and ieee address */
//        if(tx_options & MAC_INTRA_PAN)
//          length += 8;         
//            break;
//        }
//    }

//    /* only one PAN ID is needed if intra-pan */
//#ifndef WISUN_ENET_PROFILE
//    if( src && dst &&
//        src->address_mode != MAC_NO_ADDRESS &&
//        dst->address_mode != MAC_NO_ADDRESS &&
//        src->pan_id == dst->pan_id )
//#endif
//    {    /*Suneet :: here we not add pan id both src and dst packet */
//#if (RADIO_VALIDATION == 1)  
//        length -= 2;  //Suneet :: Added for FAN T108 Where we send pan id in packet 
//#else
//        length -= 4;
//#endif        
//    }  //Suneet :: check condition (tx_options & MAC_INTRA_PAN )

    /* Auxiliary Security Header + MIC data */
	#ifdef MAC_CFG_SECURITY_ENABLED
		length += sec_stuff_length( sec_params );
	#endif
    
    /*store the header + AUX hdr length*/
    enc_offset += (length - 
		#ifdef MAC_CFG_SECURITY_ENABLED
			get_MIC_length( sec_params )
		#else
			0
		#endif
			);

    /* payload length */
    length += payload_length ;

	/*Check if header ie list has to be constructed*/
//	if( ( p_hdr_ie_ids != NULL ) && (p_hdr_ie_ids->ie_list_len))
        if (sub_hdr_bitmap != 0)
	{
		hdr_ie = app_bm_alloc(MAX_HDR_IE_LIST_SIZE );//200
		if( hdr_ie == NULL )
		{
#if ((PRINT_DEBUG_LEVEL==ERROR_DEBUG)||(PRINT_DEBUG_LEVEL==CONSOLE_ERROR_DEBUG))
                  stack_print_debug ("### Malloc fail for Tx HDR IE\n");
#endif
			*p = NULL_POINTER;
			return MAC_TRANSACTION_OVERFLOW;
		}
#ifdef WISUN_FAN_MAC
                ie_field_len = build_ie (hdr_ie, IE_TYPE_HDR, WH_IE_MASK | HEADER_TIE1_MASK, sub_hdr_bitmap, NULL);
#endif
//                p_hdr_ie_ids->ie_flags = ( INCLUDE_TIE );
//		if( p_hdr_ie_ids->ie_flags & IE_REQUEST )
//		{
//    		/*build the IEs with content length as 0*/
////			ie_field_len  =  build_ie_desc_list
////							 (
////								hdr_ie,
////								IE_TYPE_HDR, 
////								p_hdr_ie_ids->ie_list_len, 
////								p_hdr_ie_ids->ie_list,
////								p_hdr_ie_ids->ie_flags 
////							 );
//		}
//		else
//		{
    		/*build the IEs with content as well*/
//			ie_field_len  =  build_ie_list
//							 (
//								hdr_ie,
//								IE_TYPE_HDR, 
//								p_hdr_ie_ids->ie_list_len, 
//								p_hdr_ie_ids->ie_list,
//                                                                ( INCLUDE_TIE )
//								//p_hdr_ie_ids->ie_flags 
//							 );
//		}
		
		
		if( ie_field_len )
		{
			length += ie_field_len;
			enc_offset += ie_field_len;
			/*ensure that the IE list present bit in the frame control is set*/ 
			fc_options |= MAC_IE_LIST_PRESENT;
			tx_options |= FRAME_VERSION_2011;
		}
		else
		{
			/* free the buffer allocated for constructing the header IE list */
		
			if( hdr_ie != NULL )
			{
				app_bm_free( hdr_ie );
				hdr_ie = NULL;	
			}
		
			/*header IE list is not present. check if payload IE is present. 
			IF payload IE is also not present, then there is no point in sending this packet.*/
			if( (( type == MAC_FRAME_TYPE_MAC_COMMAND ) && (!( payload_length-1 )))
			|| ( (type == MAC_FRAME_TYPE_BEACON)&&(!(payload_length)) ) )
			{
				/*no payload and no payload IEs and as well as no header IEs. So there is no point in sending this frame. Drop this*/
				*p = NULL_POINTER;
	   			return MAC_INVALID_PARAMETER;
			}

		}
	}

    /* PHY can only transmit packets up to aMaxPHYPacketSize bytes */
    //if( length > aMaxPHYPacketSize )
    
    fcs_len = ((type == MAC_FRAME_TYPE_DATA)?((phy_params->FCSLength)?4:2):((mac_pib.FCSType)?2:4));
	if( length > aMaxPHYPacketSize - fcs_len  )
    {
      if( hdr_ie != NULL )
      {
        app_bm_free( hdr_ie );
        hdr_ie = NULL;	
      }
        *p = NULL_POINTER;
        return MAC_FRAME_TOO_LONG;
    }
    alloc_len = (mac_pib.FCSType)?2:4;
    
    alloc_len += (sizeof( phy_tx_t ) + length);

    /* allocate packet to send */
    if( (txd = mac_mem_alloc_tx( type,alloc_len, secured,sub_type )) == NULL_POINTER )
    {
      if( hdr_ie != NULL )
      {
        app_bm_free( hdr_ie );
        hdr_ie = NULL;	
      }
        *p = NULL_POINTER;
        return MAC_TRANSACTION_OVERFLOW;
    }

    /* save transmit options */
    txd->tx_options = tx_options;

    /*
     * first byte of frame control
     */
    bp = txd->data->psdu;

    /* frame type */
    *bp = txd->type = type;
	txd->sub_type = sub_type;

    if ( type == MAC_FRAME_TYPE_MAC_COMMAND )
    {
		txd->cmd = payload[ iCmdIdInPld ];  /* save the MAC command */	
    }

    /* ack required */
    if ( tx_options & ACKNOWLEDGED_TRANSMISSION )
    {
        *bp |= MAC_ACK_REQUIRED;
    }

    /*
     * second byte of frame control
     */
    bp++;

    if (src != NULL_POINTER)
    {
        *bp = (src->address_mode << MAC_SRC_ADDRESS_SHIFT);
    }

    if (dst != NULL_POINTER)
    {
        *bp |= (dst->address_mode << MAC_DST_ADDRESS_SHIFT);
    }

    /* ensure the frame version is set correctly */
    if (tx_options & FRAME_VERSION_2006)
    {
        /* Set Frame Version in the packet - 2006 Frame */
        *bp |= FRAME_VERSION_2006;
    }
    
    *bp |= (tx_options & FRAME_VERSION_MASK);
    
    if( (tx_options & FRAME_VERSION_MASK) == FRAME_VERSION_MASK )
    {
    	tx_options &= ~FRAME_VERSION_MASK;
    	tx_options |= FRAME_VERSION_2011;
    	
    }
    
    *bp |= fc_options;

    /* security settings to frame control */
	#ifdef MAC_CFG_SECURITY_ENABLED
		set_sec_fc( secured, txd->data->psdu );
	#endif
   
//#ifdef WISUN_ENET_FRAME_FORMAT	//WISUN_ENET_PROFILE
#if( WISUN_ENET_FRAME_FORMAT == 1)
    /*overwrite the frame version with 2, irrespective of whatever present for ENET profile*/
     //tx_options &= ~FRAME_VERSION_MASK;
    	//tx_options |= FRAME_VERSION_2011;
      //*bp |= (tx_options & FRAME_VERSION_MASK);
      *bp &= ~FRAME_VERSION_MASK;
      *bp |= FRAME_VERSION_2011;
#endif
     /*
     * sequence number
     */      
    bp++;
    if( type == MAC_FRAME_TYPE_BEACON )
    {
        /* BSN is incremented once after sending the beacon,
           rather than for each re-building of the beacon frame */
        *bp = ( sub_type == MAC_FRAME_BCN_SUB_TYPE_RB )? mac_pib.BSN: mac_pib.EBSN;
        txd->sn = *bp;
    }
#ifdef ENHANCED_ACK_SUPPORT
	else if (  type == MAC_FRAME_TYPE_ACK )
	{
          txd->sn = sn;
          *bp = sn;
	}
#endif//ENHANCED_ACK_SUPPORT
    else
    {
    if( fc_options & MAC_SEQ_NUM_SUPPRESSION)
    {
       //Do nothing Added Suneet for FAN 
      // If the sequence number is supressed in mac header than do not add sequence number in MHR
    }
    else
    {
          txd->sn = mac_pib.DSN;
/*Uemsh : 10-01-2018 this thing goes to modified after sepration of mac and fan mac*/
//#ifdef WISUN_FAN_MAC        
//          static uint8_t i = 0;
//          if(dst->address_mode != MAC_NO_ADDRESS)
//          {
//            for(i = 0; i < MAX_NBR_SUPPORT; i++)
//            {
//              if(seq_number_table[i].dev_join_status == 0x00)
//              {
//                memcpy(seq_number_table[i].src_addr, src->address.ieee_address, 8);
//                memcpy(seq_number_table[i].dest_addr, dst->address.ieee_address, 8);
//                seq_number_table[i].dev_join_status = DEV_ADDED;
//                seq_number_table[i].dev_ack_status = ACK_REQUIRED;
//                break;
//              }          
//            }
//          }
//#endif
          *bp = mac_pib.DSN++;
          bp++;
    }
    }

    /*
     * address fields
     */
    
    
    if( dst != NULL_POINTER )
    {
        txd->dst.address_mode = dst->address_mode;

        if( dst->address_mode != MAC_NO_ADDRESS )
        {
          
            /* set the Dest PAN id */
//			put_ushort(bp,dst->pan_id);
//			bp+=2;
                 // length-=2;
        if(dstPanPresent == TRUE) 
        {
          put_ushort(bp,dst->pan_id);
          bp+=2;
        }
        else
        {
          if(pan_id_compression == TRUE)
            txd->data->psdu[0] |= MAC_INTRA_PAN;
          else
            txd->dst.pan_id = dst->pan_id;
        }
          /*pan should be copmressed*/
#if (RADIO_VALIDATION == 1)            
            /* set the Source PAN id */
            put_ushort(bp,src->pan_id);
            bp+=2;
#endif 
            //*bp++ = LOW_BYTE( dst->pan_id );
            //*bp++ = HIGH_BYTE( dst->pan_id );
            /* set the intra pan bit and adjust the length */
//             if(tx_options & MAC_INTRA_PAN)
//                txd->data->psdu[0] |= MAC_INTRA_PAN;
//             else
//            txd->dst.pan_id = dst->pan_id;

            if( dst->address_mode == MAC_SHORT_ADDRESS )
            {
                txd->dst.address.short_address = dst->address.short_address;
				put_ushort(bp,dst->address.short_address);
				bp+=2;
                //*bp++ = LOW_BYTE( dst->address.short_address );
                //*bp++ = HIGH_BYTE( dst->address.short_address );
            }
            else
            {
                txd->dst.address.ieee_address = bp;
                memcpy( bp, dst->address.ieee_address, 8 );
                
                /*
                if( txd->dst.address_mode == ADDR_MODE_EXTENDED )
			    {
			    	memcpy(txd->dest_long_addr,dst->address.ieee_address,8);
			    	txd->dst.address.ieee_address = txd->dest_long_addr;
			    }
                */
                bp += 8;
            }
        }
    }
    if( src != NULL_POINTER )
    {
        txd->src.address_mode = src->address_mode;

        if( src->address_mode != MAC_NO_ADDRESS )
        {
          if(srcPanPresent == TRUE)
          {
            put_ushort(bp,src->pan_id);
            bp+=2;
          }
          else
          {
            if(pan_id_compression == TRUE)
            /* set the intra pan bit and adjust the length */
            txd->data->psdu[0] |= MAC_INTRA_PAN;
          }
            /* check for intra pan */
            /*TBD dst can be NULL here */
            //if( src->pan_id == dst->pan_id )
//#ifndef WISUN_ENET_FRAME_FORMAT //WISUN_ENET_PROFILE          
//#if ( WISUN_ENET_FRAME_FORMAT == 0 )
//            if( ( dst!= NULL ) 
//            && ( dst->address_mode != MAC_NO_ADDRESS ) 
//            && ( src->pan_id == dst->pan_id ))
//            {
//                /* set the intra pan bit and adjust the length */
//                txd->data->psdu[0] |= MAC_INTRA_PAN;
//            }
//            else
//            {
//                /* set the Source PAN id */
//				put_ushort(bp,src->pan_id);
//				bp+=2;
//                //*bp++ = LOW_BYTE( src->pan_id );
//                //*bp++ = HIGH_BYTE( src->pan_id );
//            }
//#endif                

          
            txd->src.pan_id = src->pan_id;

            if( src->address_mode == MAC_SHORT_ADDRESS )
            {
                txd->src.address.short_address = src->address.short_address;
				put_ushort(bp,src->address.short_address);
				bp+=2;
                //*bp++ = LOW_BYTE( src->address.short_address );
                //*bp++ = HIGH_BYTE( src->address.short_address );
            }
            else
            {
                txd->src.address.ieee_address = bp;
                memcpy( bp, src->address.ieee_address, 8 );
                bp += 8;
            }
        }
    }

    /*
     * auxiliary security header
     */
#ifdef MAC_CFG_SECURITY_ENABLED
    txd->security_data->frame_counter_ptr = bp + 1;
    bp += add_aux_sec_header( secured, bp, sec_params );
#endif

    
    /* copy the header IE if present */
    if( ie_field_len )
    {
    	memcpy( bp, hdr_ie,ie_field_len );
#ifdef  WISUN_FAN_MAC /*umesh to resolve function declaration*/
        ufsi_ptr = extract_ufsi_and_pkt_type(bp,ie_field_len);

        bfsi_ptr = extract_bfsi(bp,ie_field_len);
#endif
    	bp += ie_field_len;
    	
    	/* free the buffer allocated for constructing the header IE list */
    	app_bm_free( hdr_ie );//santosh_heap:256
    }
    
    /* Copy MAC payload */
    if ( payload != NULL_POINTER )
    {

	    if( type != MAC_FRAME_TYPE_BEACON )
	    {
//#ifdef TEST_CHOP              
//                extern trxsm_t *trxsm_p;
//    		txd->p_channel_holder = bp;
//#endif                
    		memcpy( bp, payload, payload_length );
	    }
	    else
	    {
	    	if( sub_type == MAC_FRAME_BCN_SUB_TYPE_RB )
	    	{
				memcpy( bp, payload, payload_length - mac_pib.BeaconPayloadLength );
	    	
		    	bp += (payload_length - mac_pib.BeaconPayloadLength);
	    	
		    	memcpy(bp,mac_pib.BeaconPayload,mac_pib.BeaconPayloadLength);	    		
	    	}
	    	else
	    	{
	    		memcpy( bp, payload, payload_length );
	    		bp += payload_length;
	    	}
	    }
    }

    /* set the total length */
    txd->data->psduLength = txd->length = length;
    
    txd->data->TxChannel = phy_params->tx_channel;
    
    txd->data->PPDUCoding = (mac_pib.ConfigFlags & CONFIG_USE_PPDU_CODING)?true:false;
    
    txd->data->FCSLength = (mac_pib.FCSType)?false:true;
    
    txd->data->ModeSwitch = false;
    
    txd->data->NewModeSUNPage = 0x00;
    
    txd->data->ModeSwitchParameterEntry = 0x00;
    
    if( type == MAC_FRAME_TYPE_DATA ) 
    {
    	txd->data->PPDUCoding = phy_params->PPDUCoding;
    	txd->data->FCSLength = phy_params->FCSLength;
    	txd->data->ModeSwitch = phy_params->ModeSwitch;
    	txd->data->NewModeSUNPage = 0x0;//phy_params->NewModeSUNPage;
    	txd->data->ModeSwitchParameterEntry = 0x00;
    }
	
/* Debdeep :: We should update UFSI and BFIO after setting security field. 
As we check security field and add offset time accordingly with current time to calculate UFSI and BFIO */  
    
    /* set security fields of packet */
	
	#ifdef MAC_CFG_SECURITY_ENABLED
            if (secured == 1)
            {
                err = 	set_sec_fields( txd, enc_offset, sec_params );
            }
//	#else
//		err = MAC_SUCCESS;
	#endif

    if( err != MAC_SUCCESS )
    {
        mac_mem_free_tx( txd );
        *p = NULL_POINTER;
        return err;
    }
    
#ifdef WISUN_FAN_MAC/*Umesh : 21-02-2018*//*for sepration of 802.15.04*/

/* Debdeep :: No need to update from here. 
Later we update UFSI just before doing security encryption for secure packets. 
And just before doing PD_data_request for unsecure packets */
    uint32_t ufsi_update = 0;//update_ufsi_with_procces_time(txd);
    if(ufsi_ptr!=NULL)
    {
      txd->sub_type = *(ufsi_ptr);  //Debdeep:: fan_pkt_type
      ufsi_ptr++;
      memcpy(ufsi_ptr,(uint8_t*)&ufsi_update,3);
      txd->p_ufsi = ufsi_ptr;
    }
    
/* Debdeep :: No need to update from here. 
Later we update BFIO just before doing security encryption for secure packets. 
And just before doing PD_data_request for unsecure packets */
    uint32_t ufio_update = 0;//update_bfio_with_procces_time(txd);
    if(bfsi_ptr!=NULL)
    {
      txd->broadcast_slot_no = (bfsi_ptr);
#if(FAN_FRQ_HOPPING_FEATURE_ENABLED == 1)
      memcpy(txd->broadcast_slot_no,(uint8_t*)&broadcast_slot_nuumber,2);
#endif
      bfsi_ptr+=2;
      memcpy(bfsi_ptr,(uint8_t*)&ufio_update,3);
      txd->p_bfsi = bfsi_ptr;
    } 
#endif 
    
    //memcpy(debug_sun_arr,txd->data->psdu,2);//Suneet :: this condition to chack which is currupted
    *p = txd;
    return MAC_SUCCESS;
}

/******************************************************************************/

#ifdef MAC_CFG_SECURITY_ENABLED

/* length of some security stuff:
   1 (Security Control) + 4 (Frame Counter) + Key ID length + Integrity Code length */
int sec_stuff_length( security_params_t *p )
{
     if((p) && (p->security_level))
    {
			return ( 1 + 4 +
             get_key_identifier_length( p->key_id_mode ) +
             integrity_code_length( p->security_level ) );    	
    }
    else
    {
    	return 0;
    }
}

/******************************************************************************/

/*
** ============================================================================
** Private Function Definitions
** ============================================================================
*/

//static int add_aux_sec_header
int add_aux_sec_header( uchar secure,
                               uchar *bp,
                               security_params_t *params )
{
    uchar *bp0 = NULL;

    if( !secure )
    {
        return 0;
    }

    bp0 = bp;

    /* add Security Mode Parameter */
    *bp++ = ((params->key_id_mode & 0x3) << 3) | params->security_level;

//    append_frame_counter( bp );
    /* Clear Frame Counter field in TXD structure */
    memset (bp, 0, 4);
    bp+= 4;

    /* copy the Key Identifer if present */
    if( params->key_id_mode != 0 )
    {
        memcpy(bp, params->key_identifier, get_key_identifier_length(params->key_id_mode));
        bp += get_key_identifier_length(params->key_id_mode);
    }

    return bp - bp0;
}

/******************************************************************************/

//static void set_sec_fc( uchar secure, uchar *bp )
void set_sec_fc( uchar secure, uchar *bp )
{
    if( !secure )
    {
        return;
    }

    /* Set the security Enabled bit in the packet */
    bp[0] |= MAC_SECURITY_ENABLED;

    if( mac_pib.ConfigFlags & ( USE_2006_PRIMITIVES|USE_2011_PRIMITIVES ) )
    {
        /* Set Frame Version in the packet - 2006 Frame */
        if(( bp[1] & FRAME_VERSION_MASK) != FRAME_VERSION_2011 )
        {
        	bp[1] |= FRAME_VERSION_2006;	
        } 
    }

    return;
}

/******************************************************************************/

//static mac_status_t set_sec_fields
mac_status_t set_sec_fields( mac_tx_t *txd, uchar enc_offset, security_params_t *sec_params )
{
    if( !txd->security_data )
    {
        return MAC_SUCCESS;
    }

    /* copy security parameters to packet */
    txd->sec_param.security_level = sec_params->security_level;
    txd->sec_param.key_id_mode = sec_params->key_id_mode;
    memcpy(txd->sec_param.key_identifier, sec_params->key_identifier, get_key_identifier_length(sec_params->key_id_mode));

    /* obtain security key for data and command frames
       (for beacon frames the key should be in place) */
    //if( txd->type != MAC_FRAME_TYPE_BEACON )
    //{
        if( outgoing_key_retrieval_procedure( 
                                                  &txd->src,
                                                  &txd->dst,
                                                  txd->sec_param.key_id_mode,
                                                  txd->sec_param.key_identifier,
                                                  txd->security_data->key ) == FALSE 
											)
        {
            return MAC_UNAVAILABLE_KEY;
        }
    //}

    /*TBD Is this duplicated? */
    txd->security_data->q_item.link = NULL_POINTER;

    /* link MAC data and security info */
    txd->security_data->private_msg_data = txd;

    /* save details required to perform encryption */
    txd->security_data->security_level = txd->sec_param.security_level;
    txd->security_data->header = txd->data->psdu;
    txd->security_data->header_length = enc_offset;
    txd->security_data->payload = txd->data->psdu + enc_offset;
    txd->security_data->payload_length = txd->length - enc_offset - m_lut[txd->sec_param.security_level & 0x03];
    txd->security_data->state = MAC_DTX_STATE_NOT_PROCESSED;

    /* setup nonce */
//    init_nonce( &txd->security_data->nonce[0],
//                mac_data.security->pib.mac_frame_counter,
//                txd->sec_param.security_level,
//                aExtendedAddress);
    
//#if ((PRINT_DEBUG_LEVEL==ERROR_DEBUG)||(PRINT_DEBUG_LEVEL==CONSOLE_ERROR_DEBUG))      
//    stack_print_debug ("FC = %d\n", mac_data.security->pib.mac_frame_counter);
//#endif

    /* increment the Frame Counter */
//    mac_data.security->pib.mac_frame_counter++;

    return MAC_SUCCESS;
}

void set_frame_counter_and_nonce (mac_tx_t *txd)
{
  append_frame_counter (txd->security_data->frame_counter_ptr);
  /* setup nonce */
  init_nonce( &txd->security_data->nonce[0],
             mac_data.security->pib.mac_frame_counter,
             txd->sec_param.security_level,
             aExtendedAddress);
  
  /* increment the Frame Counter */
  mac_data.security->pib.mac_frame_counter++;
  
#if(APP_NVM_FEATURE_ENABLED == 1)
  nvm_store_write_mac_frame_counter();
#endif
}

/******************************************************************************/
//static uint8_t get_MIC_length( security_params_t *p )
uint8_t get_MIC_length( security_params_t *p )
{
	if((p) && (p->security_level))
	{
		return m_lut[p->security_level  & 0x03 ];
	}
	else
	{
		return 0;
	}	
}
#endif
/******************************************************************************/
static uint8_t check_pan_id_status(uchar frame_type,uchar tx_options,mac_address_t *src, mac_address_t *dst, gboolean *dstPanPresent,gboolean *srcPanPresent,gboolean *pan_id_compression)
{
  uint32_t offset = 0;  // Raka To Do : initialize is as per the input packet parsing ...as this is the return value of the function. 
  *pan_id_compression  = tx_options & MAC_INTRA_PAN;
  if (dst->address_mode == IEEE802154_FCF_ADDR_RESERVED) {
    /* Invalid Destination Address Mode. Abort Dissection. */
    return -1;
  }
  
  if (src->address_mode == IEEE802154_FCF_ADDR_RESERVED) {
    /* Invalid Source Address Mode. Abort Dissection. */
    return  -1;
  }
  if ((frame_type == IEEE802154_FCF_BEACON) ||
      (frame_type == IEEE802154_FCF_DATA)   ||
        (frame_type == IEEE802154_FCF_ACK)    ||
          (frame_type == IEEE802154_FCF_CMD)       ) {
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
              if ((dst->address_mode == IEEE802154_FCF_ADDR_NONE) &&      /* Not Present */
                  (src->address_mode == IEEE802154_FCF_ADDR_NONE) &&      /* Not Present */
                    (*pan_id_compression == 0)) {
                      *dstPanPresent = FALSE;
                      *srcPanPresent = FALSE;
                    }
              /* Row 2 */
              else if ((dst->address_mode == IEEE802154_FCF_ADDR_NONE) && /* Not Present */
                       (src->address_mode == IEEE802154_FCF_ADDR_NONE) && /* Not Present */
                         (*pan_id_compression == 1)) {
                           *dstPanPresent = TRUE;
                           *srcPanPresent = FALSE;
                         }
              /* Row 3 */
              else if ((dst->address_mode != IEEE802154_FCF_ADDR_NONE) && /*  Present    */
                       (src->address_mode == IEEE802154_FCF_ADDR_NONE) && /* Not Present */
                         (pan_id_compression == 0)) {
                           *dstPanPresent = TRUE;
                           *srcPanPresent = FALSE;
                         }
              /* Row 4 */
              else if ((dst->address_mode != IEEE802154_FCF_ADDR_NONE) && /*  Present    */
                       (src->address_mode == IEEE802154_FCF_ADDR_NONE) && /* Not Present */
                         (*pan_id_compression == 1)) {
                           *dstPanPresent = FALSE;
                           *srcPanPresent = FALSE;
                         }
              /* Row 5 */
              else if ((dst->address_mode == IEEE802154_FCF_ADDR_NONE) && /* Not Present */
                       (src->address_mode != IEEE802154_FCF_ADDR_NONE) && /*  Present    */
                         (*pan_id_compression == 0)) {
                           *dstPanPresent = FALSE;
                           *srcPanPresent = TRUE;
                         }
              /* Row 6 */
              else if ((dst->address_mode == IEEE802154_FCF_ADDR_NONE) && /* Not Present */
                       (src->address_mode != IEEE802154_FCF_ADDR_NONE) && /*  Present    */
                         (*pan_id_compression == 1)) {
                           *dstPanPresent = FALSE;
                           *srcPanPresent = FALSE;
                         }
              /* Row 7 */
              else if ((dst->address_mode == IEEE802154_FCF_ADDR_EXT) && /*  Extended    */
                       (src->address_mode == IEEE802154_FCF_ADDR_EXT) && /*  Extended    */
                         (*pan_id_compression == 0)) {
                           *dstPanPresent = TRUE;
                           *srcPanPresent = FALSE;
                         }
              /* Row 8 */
              else if ((dst->address_mode == IEEE802154_FCF_ADDR_EXT) && /*  Extended    */
                       (src->address_mode == IEEE802154_FCF_ADDR_EXT) && /*  Extended    */
                         (*pan_id_compression == 1)) {
                           *dstPanPresent = FALSE;
                           *srcPanPresent = FALSE;
                         }
              /* Row 9 */
              else if ((dst->address_mode == IEEE802154_FCF_ADDR_SHORT) && /*  Short     */
                       (src->address_mode == IEEE802154_FCF_ADDR_SHORT) && /*  Short     */
                         (*pan_id_compression == 0)) {
                           *dstPanPresent = TRUE;
                           *srcPanPresent = TRUE;
                         }
              /* Row 10 */
              else if ((dst->address_mode == IEEE802154_FCF_ADDR_SHORT) && /*  Short    */
                       (src->address_mode == IEEE802154_FCF_ADDR_EXT) &&   /*  Extended */
                         (*pan_id_compression == 0)) {
                           *dstPanPresent = TRUE;
                           *srcPanPresent = TRUE;
                         }
              /* Row 11 */
              else if ((dst->address_mode == IEEE802154_FCF_ADDR_EXT)   &&   /*  Extended */
                       (src->address_mode == IEEE802154_FCF_ADDR_SHORT) &&   /*  Short    */
                         (*pan_id_compression == 0)) {
                           *dstPanPresent = TRUE;
                           *srcPanPresent = TRUE;
                         }
              /* Row 12 */
              else if ((dst->address_mode == IEEE802154_FCF_ADDR_SHORT) &&   /*  Short    */
                       (src->address_mode == IEEE802154_FCF_ADDR_EXT)   &&   /*  Extended */
                         (*pan_id_compression == 1)) {
                           *dstPanPresent = TRUE;
                           *srcPanPresent = FALSE;
                         }
              /* Row 13 */
              else if ((dst->address_mode == IEEE802154_FCF_ADDR_EXT)   &&   /*  Extended */
                       (src->address_mode == IEEE802154_FCF_ADDR_SHORT) &&   /*  Short    */
                         (*pan_id_compression == 1)) {
                           *dstPanPresent = TRUE;
                           *srcPanPresent = FALSE;
                         }
              /* Row 14 */
              else if ((dst->address_mode == IEEE802154_FCF_ADDR_SHORT) &&   /*  Short    */
                       (src->address_mode == IEEE802154_FCF_ADDR_SHORT) &&   /*  Short    */
                         (*pan_id_compression == 1)) {
                           *dstPanPresent = TRUE;
                           *srcPanPresent = FALSE;
                         }
              else {
                return -1;
              }
            }
    else { /* Frame Type is neither Beacon, Data, Ack, nor Command: PAN ID Compression is not used */
      *dstPanPresent = FALSE; /* no PAN ID will */
      *srcPanPresent = FALSE; /* be present     */
    }
  
  /*
  * Addressing Fields
  */
  
  /* Destination PAN Id */
  if (*dstPanPresent) {
    offset += 2;
  }
  
  /* Destination Address  */
  if (dst->address_mode == IEEE802154_FCF_ADDR_SHORT) {
    offset += 2;
  }
  else if (dst->address_mode == IEEE802154_FCF_ADDR_EXT) {
    offset += 8;
  }
  
  /* Source PAN Id */
  if (*srcPanPresent) {
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
  if (src->address_mode == IEEE802154_FCF_ADDR_SHORT) {
    offset += 2;
  }
  else if (src->address_mode == IEEE802154_FCF_ADDR_EXT) {
    offset += 8;
  }
  
  return  offset;
  
}


#ifdef WISUN_FAN_MAC

/******************************************************************************/
#if(FAN_EDFE_FEATURE_ENABLED == 1)

mac_status_t build_EDFE_frame_Req_res(mac_tx_t** p,
                                      uint8_t *dest_addr,
                                      uint32_t hdr_bitmap,
                                      uint32_t pld_bitmap)
{
  mac_status_t returnStatus = MAC_SUCCESS;
  uchar mac_frame_type = MAC_FRAME_TYPE_DATA;
  mac_address_t src, dst;
  uchar fc_options_byte_2 = 0;
  uchar fc_options_byte_1 = 0;
  ushort payload_length = 0;
  uchar payload[200] = {0}; 
  uchar* pld = payload;
  uchar enc_offset = 0;
  mac_tx_t *txd = NULL;
  uchar *bp = NULL;
  uchar* hdr_ie = NULL;
  uint16_t length = 0;
  uchar secured = 0;
  uint16_t alloc_len = 0;
  uint16_t ie_field_len = 0;
  uint32_t TxChannel = 0;
  uint16_t len = 0;
  
  PLME_get_request( phyCurrentChannel, &len, &TxChannel );
  
  memcpy(edfe_information.edfe_ini_mac_addr,dest_addr,8);
  fan_mac_params.type = 4;
  uint8_t fan_frame_type = fan_mac_params.type;
  if(edfe_information.edfe_frame_tx_type == INITIAL_FRAME)
  {
    dst.address_mode = MAC_IEEE_ADDRESS;
    dst.address.ieee_address = dest_addr;
    src.address_mode = MAC_IEEE_ADDRESS;
  }
  else if(edfe_information.edfe_frame_tx_type == RESPONSE_FRAME)
  {
    dst.address_mode = MAC_IEEE_ADDRESS;
    dst.address.ieee_address = dest_addr;
    src.address_mode = MAC_NO_ADDRESS;
  }
  else if(edfe_information.edfe_frame_tx_type == FINAL_RESPONSE_FRAME)
  {
    dst.address_mode = MAC_IEEE_ADDRESS;
    dst.address.ieee_address = dest_addr;
    src.address_mode = MAC_IEEE_ADDRESS;
  }
 
  src.pan_id = dst.pan_id = mac_pib.PANId;
  uint8_t* bfsi_ptr = NULL;
  uint8_t* ufsi_ptr = NULL;
  security_params_t *sec_params = NULL_POINTER;
#if !(ENABLE_FAN_MAC_WITHOUT_SECURITY )
  security_params_t sec_data = {0};   
  sec_data.security_level = 0x06; 
  sec_data.key_id_mode = 0x01;
  memset(sec_data.key_identifier, 0x00, 0x09);
  sec_data.key_identifier[0] = /* 0x01; */ (key_id_index+1);  
  sec_params = &sec_data;
  secured = 1;
#endif 
   
  
  
  /* check if the frame counter wrapped */
  if( mac_data.security->pib.mac_frame_counter == 0xffffffffUL )
  {
    *p = NULL_POINTER;
    return MAC_COUNTER_ERROR;
  }
  
  payload_length = build_ie (pld, IE_TYPE_PLD, WP_IE_MASK | PAYLOAD_TIE_MASK, pld_bitmap, NULL);
  fc_options_byte_1 |= MAC_INTRA_PAN;
  /* [Raka :: 22-March- 2018 ] ::::  All MLME-WS-ASYNC-FRAME does not have Sequence Number   */
  fc_options_byte_2 |= MAC_SEQ_NUM_SUPPRESSION;
  fc_options_byte_2 |= MAC_IE_LIST_PRESENT;
  
  /* [Raka :: 22-March- 2018 ] ::::  All MLME-WS-ASYNC-FRAME does not have Sequence Number so length is starting from only FCF*/
  length = MAC_FRAME_CONTROL_FIELD_LEN;
  
  /* [Raka :: 22-March- 2018 ] ::::For Source Address [MAC_IEEE_ADDRESS 8 Byte] and Source PAN ID [ 2 byte]*/
  if(dst.address_mode == MAC_IEEE_ADDRESS)
  {
    length += 8; 
  }
  if(src.address_mode == MAC_IEEE_ADDRESS)
  {
    length += 8;
  }
  
  /* Auxiliary Security Header + MIC data */
#ifdef MAC_CFG_SECURITY_ENABLED
  length += sec_stuff_length( sec_params );
#endif
  
  /*store the header + AUX hdr length*/
  enc_offset = length ;
  
#ifdef MAC_CFG_SECURITY_ENABLED
  enc_offset -= get_MIC_length( sec_params );
#endif
  
  /* [Raka :: 22-March- 2018 ] ::::  payload length  is added to the tx packet length*/
  length += payload_length ;
  
  /*[Raka :: 22-March- 2018 ] ::::  Check if header ie list has to be constructed
  This condition should be always True if False then some memory corruption happened , cross check*/
  if (hdr_bitmap != 0)
  {
    hdr_ie = app_bm_alloc(MAX_HDR_IE_LIST_SIZE );
    if( hdr_ie == NULL )
    {
      *p = NULL_POINTER;
      return MAC_TRANSACTION_OVERFLOW;
    }
    
    /*[Raka :: 22-March- 2018 ] :::: Build the Header IEs with content as well*/
    ie_field_len  =  build_ie (hdr_ie, IE_TYPE_HDR, WH_IE_MASK | HEADER_TIE1_MASK, hdr_bitmap, NULL);
    
    if( ie_field_len )
    { 
      /*[Raka :: 22-March- 2018 ] :::: Added the Length */
      length += ie_field_len;
      enc_offset += ie_field_len;
    }
    else
    {
      /* free the buffer allocated for constructing the header IE list 
      [Raka :: 22-March- 2018 ] ::::  The Code should Never come here */
      if( hdr_ie != NULL )
      {
        app_bm_free( hdr_ie );
        hdr_ie = NULL;	
      }
    }
  }
  
  /*[Raka :: 22-March- 2018 ] ::::  Calculating the buffer size required at PHY layer to transmit the packet  */
  alloc_len = (mac_pib.FCSType)?2:4;        
  alloc_len += (sizeof( phy_tx_t ) + length);
  
  /* allocate packet to send */
  if( (txd = mac_mem_alloc_tx( mac_frame_type,alloc_len, secured,fan_frame_type )) == NULL_POINTER )
  {
    if( hdr_ie != NULL )
    {
      app_bm_free( hdr_ie );
      hdr_ie = NULL;	
    }
    *p = NULL_POINTER;
    return MAC_TRANSACTION_OVERFLOW;
  }
  
  /* [Raka :: 22-March- 2018 ] :::: save transmit options 
  For these typr of packet we do not have tx_options*/
  txd->tx_options = 0x00;
  
  /*
  * first byte of frame control
  */
  bp = txd->data->psdu;
  
  /* frame type */
  *bp = txd->type = MAC_FRAME_TYPE_DATA;
  txd->sub_type = fan_frame_type; //sub_type :: Debdeep changed on 20-March-2018
  *bp |= fc_options_byte_1;
  bp++;
  
  /*
  * second byte of frame control
  */
  
  *bp = (src.address_mode << MAC_SRC_ADDRESS_SHIFT);
  *bp |= (dst.address_mode << MAC_DST_ADDRESS_SHIFT);
  *bp |= (FRAME_VERSION_2011 & FRAME_VERSION_MASK);
  *bp |= fc_options_byte_2;
  bp++;
  
  /* security settings to frame control and the MAC Frame Version */
#ifdef MAC_CFG_SECURITY_ENABLED
  set_sec_fc( secured, txd->data->psdu );
#endif
  
  /*[Raka :: 22-March- 2018 ] ::::  Set the PAN ID*/
  if (!( fc_options_byte_1 & MAC_INTRA_PAN))
  {
    put_ushort(bp,src.pan_id);
    bp+=2;
  }
  
  /*
  * address fields
  */
  txd->dst.address_mode = dst.address_mode;
  txd->src.address_mode = src.address_mode;
  txd->src.pan_id = src.pan_id;
  if(txd->dst.address_mode == MAC_IEEE_ADDRESS)
  {
    txd->dst.address.ieee_address = bp;
    mem_rev_cpy(txd->dst.address.ieee_address,dst.address.ieee_address, 8 );
    bp += 8;
  }
  if(txd->src.address_mode == MAC_IEEE_ADDRESS)
  {
    txd->src.address.ieee_address = bp;
    memcpy( txd->src.address.ieee_address, aExtendedAddress, 8 );
    bp += 8;
  }
  
  /*
  * auxiliary security header
  */
#ifdef MAC_CFG_SECURITY_ENABLED
  txd->security_data->frame_counter_ptr = bp + 1;
  bp += add_aux_sec_header( secured, bp, sec_params );
#endif
  
  /* copy the header IE if present */
  if( ie_field_len )
  {
    memcpy( bp, hdr_ie,ie_field_len );
    ufsi_ptr = extract_ufsi_and_pkt_type(bp,ie_field_len);
    
    bfsi_ptr = extract_bfsi(bp,ie_field_len);
     /*LOOK FOR ufsi  field and assign p_usfi param of mac_tx_t for future use*/
 
   
     bp += ie_field_len;
    /* free the buffer allocated for constructing the header IE list */
    app_bm_free( hdr_ie );
  }
  
  /* Raka :: 22-March- 2018 ] ::::  Copy payload  which is nothing but the content of payload IE data*/
  memcpy( bp, payload, payload_length );
  bp += payload_length;
  
  /* set the total length */
  txd->data->psduLength = txd->length = length;
  txd->data->TxChannel = TxChannel;//phy_params.tx_channel;
  txd->data->PPDUCoding = (mac_pib.ConfigFlags & CONFIG_USE_PPDU_CODING)?true:false;    
  txd->data->FCSLength = (mac_pib.FCSType)?false:true;    
  txd->data->ModeSwitch = false;    
  txd->data->NewModeSUNPage = 0x00;    
  txd->data->ModeSwitchParameterEntry = 0x00;

/* Debdeep :: We should update UFSI and BFIO after setting security field. 
As we check security field and add offset time accordingly with current time to calculate UFSI and BFIO */  
  /* set security fields of packet */
  
#ifdef MAC_CFG_SECURITY_ENABLED
  if (secured == 1)
  {
    returnStatus = set_sec_fields( txd, enc_offset, sec_params );
  }
#endif
  
  if( returnStatus != MAC_SUCCESS )
  {
    mac_mem_free_tx( txd );
    *p = NULL_POINTER;
    return returnStatus;
  }
  
  uint32_t ufio_update = update_bfio_with_procces_time(txd);
  if(bfsi_ptr!=NULL)
  {
    txd->broadcast_slot_no = (bfsi_ptr);
    
#if(FAN_FRQ_HOPPING_FEATURE_ENABLED == 1)
    memcpy(txd->broadcast_slot_no,(uint8_t*)&broadcast_slot_nuumber,2);
#endif
    
    bfsi_ptr+=2;
    memcpy(bfsi_ptr,(uint8_t*)&ufio_update,3);
    txd->p_bfsi = bfsi_ptr;
  } 
  
  uint32_t ufsi_update = update_ufsi_with_procces_time(txd);
  if(ufsi_ptr!=NULL)
  {
    txd->sub_type = *(ufsi_ptr);  //Debdeep:: fan_pkt_type
    ufsi_ptr++;
    memcpy(ufsi_ptr,(uint8_t*)&ufsi_update,3);
    txd->p_ufsi = ufsi_ptr;
  }
  
  *p = txd;
  
  return returnStatus;
}
#endif //#if(FAN_EDFE_FEATURE_ENABLED == 1)


mac_status_t Build_MLME_WS_ASYNC_FRAME_Request(mac_tx_t** p, 
                                               uint32_t chan_value,
                                               uint8_t fan_frame_type,
                                               uint32_t hdr_bitmap,
                                               uint32_t pld_bitmap)
{
  mac_status_t returnStatus = MAC_SUCCESS;
  uchar mac_frame_type = MAC_FRAME_TYPE_DATA;
  mac_address_t src, dst;
  uchar fc_options_byte_2 = 0;
  uchar fc_options_byte_1 = 0;
  ushort payload_length = 0;
  uchar payload[200] = {0}; 
  uchar* pld = payload;
  uchar enc_offset = 0;
  mac_tx_t *txd = NULL;
  uchar *bp = NULL;
  uchar* hdr_ie = NULL;
  uint16_t length = 0;
  uchar secured = 0;
  uint16_t alloc_len = 0;
  uint16_t ie_field_len = 0;

  fan_mac_params.type = fan_frame_type;
  dst.address_mode = MAC_NO_ADDRESS;
  src.address_mode = MAC_IEEE_ADDRESS;
  src.pan_id = dst.pan_id = mac_pib.PANId;
  uint8_t* bfsi_ptr = NULL;
  uint8_t* ufsi_ptr = NULL;
  security_params_t *sec_params = NULL_POINTER;
#if !(ENABLE_FAN_MAC_WITHOUT_SECURITY )
  security_params_t sec_data = {0};   
  sec_data.security_level = 0x06; 
  sec_data.key_id_mode = 0x01;
  memset(sec_data.key_identifier, 0x00, 0x09);
  sec_data.key_identifier[0] = /* 0x01; */ (key_id_index+1);  
  
  if(fan_mac_params.type == PAN_CONFIG)
  {
    sec_params = &sec_data;
    secured = 1;
    
    /* check if the frame counter wrapped */
    if( mac_data.security->pib.mac_frame_counter == 0xffffffffUL )
    {
      *p = NULL_POINTER;
      return MAC_COUNTER_ERROR;
    }
  }
#endif 
   
  payload_length = build_ie (pld, IE_TYPE_PLD, WP_IE_MASK | PAYLOAD_TIE_MASK, pld_bitmap, NULL);
  
  /* [Raka :: 22-March- 2018 ] ::::  All MLME-WS-ASYNC-FRAME does not have Sequence Number   */
  fc_options_byte_2 |= MAC_SEQ_NUM_SUPPRESSION;
  fc_options_byte_2 |= MAC_IE_LIST_PRESENT;
  
  /* [Raka :: 22-March- 2018 ] ::::  All MLME-WS-ASYNC-FRAME does not have Sequence Number so length is starting from only FCF*/
  length = MAC_FRAME_CONTROL_FIELD_LEN;
  
  /* [Raka :: 22-March- 2018 ] ::::For Source Address [MAC_IEEE_ADDRESS 8 Byte] and Source PAN ID [ 2 byte]*/
  length += 2 + 8; 
  
  /* [Raka :: 22-March- 2018 ] :::: PAN_ADVERT_SOLICIT Frame does not contain PAN ID  so we have to adjust the length */
  if(fan_mac_params.type == PAN_ADVERT_SOLICIT)
  {
    fc_options_byte_1 |= MAC_INTRA_PAN;
    length -= 2;
  }
  
  /* Auxiliary Security Header + MIC data */
#ifdef MAC_CFG_SECURITY_ENABLED
  length += sec_stuff_length( sec_params );
#endif
  
  /*store the header + AUX hdr length*/
  enc_offset = length ;
  
#ifdef MAC_CFG_SECURITY_ENABLED
  enc_offset -= get_MIC_length( sec_params );
#endif
  
  /* [Raka :: 22-March- 2018 ] ::::  payload length  is added to the tx packet length*/
  length += payload_length ;
  
  /*[Raka :: 22-March- 2018 ] ::::  Check if header ie list has to be constructed
  This condition should be always True if False then some memory corruption happened , cross check*/
  if (hdr_bitmap != 0)
  {
    hdr_ie = app_bm_alloc(MAX_HDR_IE_LIST_SIZE );
    if( hdr_ie == NULL )
    {
      *p = NULL_POINTER;
      return MAC_TRANSACTION_OVERFLOW;
    }
    
    /*[Raka :: 22-March- 2018 ] :::: Build the Header IEs with content as well*/
    ie_field_len  =  build_ie (hdr_ie, IE_TYPE_HDR, WH_IE_MASK | HEADER_TIE1_MASK, hdr_bitmap, NULL);
    
    if( ie_field_len )
    { 
      /*[Raka :: 22-March- 2018 ] :::: Added the Length */
      length += ie_field_len;
      enc_offset += ie_field_len;
    }
    else
    {
      /* free the buffer allocated for constructing the header IE list 
      [Raka :: 22-March- 2018 ] ::::  The Code should Never come here */
      if( hdr_ie != NULL )
      {
        app_bm_free( hdr_ie );
        hdr_ie = NULL;	
      }
    }
  }
  
  /*[Raka :: 22-March- 2018 ] ::::  Calculating the buffer size required at PHY layer to transmit the packet  */
  alloc_len = (mac_pib.FCSType)?2:4;        
  alloc_len += (sizeof( phy_tx_t ) + length);
  
  /* allocate packet to send */
  if( (txd = mac_mem_alloc_tx( mac_frame_type,alloc_len, secured,fan_frame_type )) == NULL_POINTER )
  {
    if( hdr_ie != NULL )
    {
      app_bm_free( hdr_ie );
      hdr_ie = NULL;	
    }
    *p = NULL_POINTER;
    return MAC_TRANSACTION_OVERFLOW;
  }
  
  /* [Raka :: 22-March- 2018 ] :::: save transmit options 
  For these typr of packet we do not have tx_options*/
  txd->tx_options = 0x00;
  
  /*
  * first byte of frame control
  */
  bp = txd->data->psdu;
  
  /* frame type */
  *bp = txd->type = MAC_FRAME_TYPE_DATA;
  txd->sub_type = fan_frame_type; //sub_type :: Debdeep changed on 20-March-2018
  *bp |= fc_options_byte_1;
  bp++;
  
  /*
  * second byte of frame control
  */
  
  *bp = (src.address_mode << MAC_SRC_ADDRESS_SHIFT);
  *bp |= (dst.address_mode << MAC_DST_ADDRESS_SHIFT);
  *bp |= (FRAME_VERSION_2011 & FRAME_VERSION_MASK);
  *bp |= fc_options_byte_2;
  bp++;
  
  /* security settings to frame control and the MAC Frame Version */
#ifdef MAC_CFG_SECURITY_ENABLED
  set_sec_fc( secured, txd->data->psdu );
#endif
  
  /*[Raka :: 22-March- 2018 ] ::::  Set the PAN ID*/
  if (!( fc_options_byte_1 & MAC_INTRA_PAN))
  {
    put_ushort(bp,src.pan_id);
    bp+=2;
  }
  
  /*
  * address fields
  */
  txd->dst.address_mode = dst.address_mode;
  txd->src.address_mode = src.address_mode;
  txd->src.pan_id = src.pan_id;
  txd->src.address.ieee_address = bp;
  memcpy( txd->src.address.ieee_address, aExtendedAddress, 8 );
  bp += 8;
  
  /*
  * auxiliary security header
  */
#ifdef MAC_CFG_SECURITY_ENABLED
  if(txd->security_data != NULL)
  {
    txd->security_data->frame_counter_ptr = bp + 1;
    bp += add_aux_sec_header( secured, bp, sec_params );
  }
#endif
  
  /* copy the header IE if present */
  if( ie_field_len )
  {
    memcpy( bp, hdr_ie,ie_field_len );
    ufsi_ptr = extract_ufsi_and_pkt_type(bp,ie_field_len);
    if(fan_mac_params.type == PAN_CONFIG)
      bfsi_ptr = extract_bfsi(bp,ie_field_len);
     /*LOOK FOR ufsi  field and assign p_usfi param of mac_tx_t for future use*/
 
   
     bp += ie_field_len;
    /* free the buffer allocated for constructing the header IE list */
    app_bm_free( hdr_ie );
  }
  
  /* Raka :: 22-March- 2018 ] ::::  Copy payload  which is nothing but the content of payload IE data*/
  memcpy( bp, payload, payload_length );
  bp += payload_length;
  
  /* set the total length */
  txd->data->psduLength = txd->length = length;
  txd->data->TxChannel = chan_value;//phy_params.tx_channel;
  txd->data->PPDUCoding = (mac_pib.ConfigFlags & CONFIG_USE_PPDU_CODING)?true:false;    
  txd->data->FCSLength = (mac_pib.FCSType)?false:true;    
  txd->data->ModeSwitch = false;    
  txd->data->NewModeSUNPage = 0x00;    
  txd->data->ModeSwitchParameterEntry = 0x00;
  
/* Debdeep :: We should update UFSI and BFIO after setting security field. 
As we check security field and add offset time accordingly with current time to calculate UFSI and BFIO */
  
  /* set security fields of packet */
  
#ifdef MAC_CFG_SECURITY_ENABLED
  if (secured == 1)
  {
    returnStatus = set_sec_fields( txd, enc_offset, sec_params );
  }
#endif
  
  if( returnStatus != MAC_SUCCESS )
  {
    mac_mem_free_tx( txd );
    *p = NULL_POINTER;
    return returnStatus;
  }
  
  if(fan_mac_params.type == PAN_CONFIG)
  {
    uint32_t ufio_update = update_bfio_with_procces_time(txd);
    if(bfsi_ptr!=NULL)
    {
      txd->broadcast_slot_no = (bfsi_ptr);
#if(FAN_FRQ_HOPPING_FEATURE_ENABLED == 1)
      memcpy(txd->broadcast_slot_no,(uint8_t*)&broadcast_slot_nuumber,2);
#endif
      bfsi_ptr+=2;
      memcpy(bfsi_ptr,(uint8_t*)&ufio_update,3);
      txd->p_bfsi = bfsi_ptr;
    } 
  }
  uint32_t ufsi_update = update_ufsi_with_procces_time(txd);
  if(ufsi_ptr!=NULL)
  {
    txd->sub_type = *(ufsi_ptr);  //Debdeep:: fan_pkt_type
    ufsi_ptr++;
    memcpy(ufsi_ptr,(uint8_t*)&ufsi_update,3);
    txd->p_ufsi = ufsi_ptr;
  }
  
  *p = txd;
  
  return returnStatus;
}

mac_status_t Build_ACK_FRAME_Request(mac_tx_t** p,
                           mac_rx_t *mrp,
                           uint32_t sub_hdr_bitmap,
                           uint32_t sub_pld_bitmap)
{
  mac_status_t returnStatus = MAC_SUCCESS;
  uchar mac_frame_type = MAC_FRAME_TYPE_ACK;
  mac_address_t src, dst;
  uchar fc_options_byte_2 = 0;
  uchar fc_options_byte_1 = 0;
  uchar enc_offset = 0;
  mac_tx_t *txd = NULL;
  uchar *bp = NULL;
  uchar* hdr_ie = NULL;
  uint16_t length = 0;
  uchar secured = 0;
  uint16_t alloc_len = 0;
  uint16_t ie_field_len = 0;
//  uint8_t* bfsi_ptr = NULL;
  uint8_t* ufsi_ptr = NULL;
  fan_mac_params.type = FAN_ACK;
  dst.address_mode = MAC_IEEE_ADDRESS;
  src.address_mode = MAC_IEEE_ADDRESS;
  security_params_t *sec_params = NULL_POINTER;
  
#if !(ENABLE_FAN_MAC_WITHOUT_SECURITY )
  security_params_t sec_data = {0};   
  sec_data.security_level = 0x06; 
  sec_data.key_id_mode = 0x01;
  memset(sec_data.key_identifier, 0x00, 0x09);
  sec_data.key_identifier[0] = /* 0x01; */ (key_id_index+1);  
  
  if((mac_pib.mac_security_enabled) && 
     ( mrp->pd_rxp->psdu[0] & MAC_SECURITY_ENABLED ))
  {
    sec_params = &sec_data;
    secured = 1;
    
    /* check if the frame counter wrapped */
    if( mac_data.security->pib.mac_frame_counter == 0xffffffffUL )
    {
      *p = NULL_POINTER;
      return MAC_COUNTER_ERROR;
    }
  }
#endif
    
  fc_options_byte_1 |= MAC_INTRA_PAN;
  fc_options_byte_2 |= MAC_SEQ_NUM_SUPPRESSION;
  fc_options_byte_2 |= MAC_IE_LIST_PRESENT;
  
  /* [Raka :: 22-March- 2018 ] ::::  All ACK-FRAME does not have Sequence Number so length is starting from only FCF*/
  length = MAC_FRAME_CONTROL_FIELD_LEN;
  
  /* [Debdeep :: 23-March- 2018 ] :::: For Destination Address [MAC_IEEE_ADDRESS 8 Byte]*/
  length += 8;
  
  /* [Debdeep :: 23-March- 2018 ] :::: For Source Address [MAC_IEEE_ADDRESS 8 Byte]*/
  length += 8;
  
  /* [Debdeep :: 23-March- 2018 ] :::: If Sequence number is not suppressed in received packet
  then ACK packet should have sequence number suppression field as 0 and length should be 
  incremented by length of sequence number [1]*/
  if(!(mrp->pd_rxp->psdu[1] & MAC_SEQ_NUM_SUPPRESSION))
  {
    fc_options_byte_2 &= ~MAC_SEQ_NUM_SUPPRESSION;
    length += 1;
  }
  
  /* Auxiliary Security Header + MIC data */
#ifdef MAC_CFG_SECURITY_ENABLED
  length += sec_stuff_length( sec_params );
#endif
  
  /*store the header + AUX hdr length*/
  enc_offset = length ;
  
#ifdef MAC_CFG_SECURITY_ENABLED
  enc_offset -= get_MIC_length( sec_params );
#endif
  
  /*[Raka :: 22-March- 2018 ] ::::  Check if header ie list has to be constructed
  This condition should be always True if False then some memory corruption happened , cross check*/
  if (sub_hdr_bitmap != 0)
  {
    hdr_ie = app_bm_alloc(MAX_HDR_IE_LIST_SIZE );
    if( hdr_ie == NULL )
    {
      *p = NULL_POINTER;
      return MAC_TRANSACTION_OVERFLOW;
    }
    
    /*[Raka :: 22-March- 2018 ] :::: Build the Header IEs with content as well*/
    ie_field_len = build_ie (hdr_ie, IE_TYPE_HDR, WH_IE_MASK | HEADER_TIE2_MASK, sub_hdr_bitmap, NULL);
    
    if( ie_field_len )
    {
      /*[Raka :: 22-March- 2018 ] :::: Added the Length */
      length += ie_field_len;
      enc_offset += ie_field_len;
    }
    else
    {
      /* free the buffer allocated for constructing the header IE list 
      [Raka :: 22-March- 2018 ] ::::  The Code should Never come here */
      if( hdr_ie != NULL )
      {
        app_bm_free( hdr_ie );
        hdr_ie = NULL;	
      }
    }
  }
  
  /*[Raka :: 22-March- 2018 ] ::::  Calculating the buffer size required at PHY layer to transmit the packet  */
  alloc_len = (mac_pib.FCSType) ? 2 : 4;        
  alloc_len += (sizeof( phy_tx_t ) + length);
  
  /* allocate packet to send */
  if ((txd = mac_mem_alloc_tx (mac_frame_type, alloc_len, secured,fan_mac_params.type)) == NULL_POINTER)
  {
    if( hdr_ie != NULL )
    {
      app_bm_free( hdr_ie );
      hdr_ie = NULL;	
    }
    *p = NULL_POINTER;
    return MAC_TRANSACTION_OVERFLOW;
  }
  
  /* [Raka :: 22-March- 2018 ] :::: save transmit options 
  For these typr of packet we do not have tx_options*/
  txd->tx_options = 0x00;
  
  /*
  * first byte of frame control
  */
  bp = txd->data->psdu;
  
  /* frame type */
  *bp = txd->type = MAC_FRAME_TYPE_ACK;
  txd->sub_type = fan_mac_params.type;
  *bp |= fc_options_byte_1;
  bp++;
  
  /*
  * second byte of frame control
  */
  
  *bp = (src.address_mode << MAC_SRC_ADDRESS_SHIFT);
  *bp |= (dst.address_mode << MAC_DST_ADDRESS_SHIFT);
  *bp |= (FRAME_VERSION_2011 & FRAME_VERSION_MASK);
  *bp |= fc_options_byte_2;
  bp++;
  
  /* security settings to frame control and the MAC Frame Version */
#ifdef MAC_CFG_SECURITY_ENABLED
  set_sec_fc( secured, txd->data->psdu );
#endif
  
  /*[Raka :: 22-March- 2018 ] ::::  Set the Sequence Number*/
  if (!( fc_options_byte_2 & MAC_SEQ_NUM_SUPPRESSION))
  {
    *bp = mrp->pd_rxp->psdu[2];
    bp+=1;
//#if ((PRINT_DEBUG_LEVEL==ERROR_DEBUG)||(PRINT_DEBUG_LEVEL==CONSOLE_ERROR_DEBUG))      
//    stack_print_debug ("build ACK sn = %02X\n", mrp->pd_rxp->psdu[2]);
//#endif
  }
  
  /*
  * address fields
  */
  txd->dst.address_mode = dst.address_mode;
  txd->src.address_mode = src.address_mode;
  txd->dst.address.ieee_address = bp;
  memcpy( txd->dst.address.ieee_address, mrp->src.address.ieee_address, 8 );
  bp += 8;
  txd->src.address.ieee_address = bp;
  memcpy( txd->src.address.ieee_address, aExtendedAddress, 8 );
  bp += 8;
  
  /*
  * auxiliary security header
  */
#ifdef MAC_CFG_SECURITY_ENABLED
  txd->security_data->frame_counter_ptr = bp + 1;
  bp += add_aux_sec_header( secured, bp, sec_params );
#endif
  
  /* copy the header IE if present */
  if (ie_field_len)
  {
    memcpy( bp, hdr_ie,ie_field_len );
    
    // LOOK FOR ufsi  field and assign p_usfi param of mac_tx_t for future use
   // uint8_t* bfsi_ptr = extract_bfsi(bp,ie_field_len);
    ufsi_ptr = extract_ufsi_and_pkt_type(bp,ie_field_len);
    // bfsi_ptr = extract_bfsi(bp,ie_field_len);
    bp += ie_field_len;
    
    /* free the buffer allocated for constructing the header IE list */
    app_bm_free( hdr_ie );
  }
  
  /* set the total length */
  txd->data->psduLength = txd->length = length;
  txd->data->TxChannel = mrp->pd_rxp->channel;
  txd->data->PPDUCoding = (mac_pib.ConfigFlags & CONFIG_USE_PPDU_CODING)?true:false;    
  txd->data->FCSLength = (mac_pib.FCSType)?false:true;    
  txd->data->ModeSwitch = false;    
  txd->data->NewModeSUNPage = 0x00;    
  txd->data->ModeSwitchParameterEntry = 0x00;
  
/* Debdeep :: We should update UFSI and BFIO after setting security field. 
As we check security field and add offset time accordingly with current time to calculate UFSI and BFIO */

  /* set security fields of packet */
  
#ifdef MAC_CFG_SECURITY_ENABLED
  if (secured == 1)
  {
    returnStatus = set_sec_fields( txd, enc_offset, sec_params );
  }
#endif
  
  if( returnStatus != MAC_SUCCESS )
  {
    mac_mem_free_tx( txd );
    *p = NULL_POINTER;
    return returnStatus;
  }
  
/* Debdeep :: No need to update ufsi from here. Just store the UFSI pointer in txd structure*/
  uint32_t ufsi_update = 0;//update_ufsi_with_procces_time(txd);
  if(ufsi_ptr!=NULL)
  {
    txd->sub_type = *(ufsi_ptr);  //Debdeep:: fan_pkt_type
    ufsi_ptr++;
    memcpy(ufsi_ptr,(uint8_t*)&ufsi_update,3);
    txd->p_ufsi = ufsi_ptr;
  }

/* Debdeep :: We dont include BFIO in ACK packet */
//  uint32_t ufio_update = update_bfio_with_procces_time(txd);
//  if(bfsi_ptr!=NULL)
//  {
//    txd->broadcast_slot_no = (bfsi_ptr);
//    memcpy(txd->broadcast_slot_no,(uint8_t*)&broadcast_slot_nuumber,2);
//    bfsi_ptr+=2;
//    memcpy(bfsi_ptr,(uint8_t*)&ufio_update,3);
//    txd->p_bfsi = bfsi_ptr;
//  } 
  
  *p = txd;
  
  return returnStatus;
}

/******************************************************************************/
//static uint8_t* extract_ufsi_and_pkt_type(uint8_t *ptr, uint16_t length)
uint8_t* extract_ufsi_and_pkt_type(uint8_t *ptr, uint16_t length)
{
  uint16_t i=0;
  uint8_t content_len = 0;
  uint8_t wisun_hdr_id=0xFF;
  uint8_t sub_id=0xFF;
  uint8_t* local_ptr = ptr;
  
  
      while(i<length)
        {
           content_len = ptr[i] & 0x7F;
           wisun_hdr_id = (ptr[i+1]<<1) | (ptr[i]>>7);
           sub_id = ptr[i+2];
           local_ptr+=2;
           if(wisun_hdr_id ==  WH_IE)                
           {
             if(sub_id==WH_IE_SUBID_UTT_IE_SHORT)
             {
               return (local_ptr+1);
             } 
             else
             {
               local_ptr+=content_len;
               i+=(2+content_len);
               
             }
           }
           else
           {
             local_ptr+=content_len;
             i+=(2+content_len);
           }
        }
      return NULL;
} 

/******************************************************************************/
uint8_t* extract_vender_ie(uint8_t *ptr, uint16_t length)
{
  uint16_t i=0;
  uint8_t content_len = 0;
  uint8_t wisun_hdr_id=0xFF;
  uint8_t sub_id=0xFF;
  uint8_t* local_ptr = ptr;
  
  
      while(i<length)
        {
           content_len = ptr[i] & 0x7F;
           wisun_hdr_id = (ptr[i+1]<<1) | (ptr[i]>>7);
           sub_id = ptr[i+2];
           local_ptr+=2;
           if(wisun_hdr_id ==  WH_IE)                
           {
             if(sub_id==WH_IE_SUBID_VH_IE_SHORT)
             {
               return (local_ptr+1);
             } 
             else
             {
               local_ptr+=content_len;
               i+=(2+content_len);
               
             }
           }
           else
           {
             local_ptr+=content_len;
             i+=(2+content_len);
           }
        }
      return NULL;
}

uint8_t* extract_bfsi(uint8_t *ptr, uint16_t length)
{
  uint16_t i=0;
  uint8_t content_len = 0;
  uint8_t wisun_hdr_id=0xFF;
  uint8_t sub_id=0xFF;
  uint8_t* local_ptr = ptr;
  
  
      while(i<length)
        {
           content_len = ptr[i] & 0x7F;
           wisun_hdr_id = (ptr[i+1]<<1) | (ptr[i]>>7);
           sub_id = ptr[i+2];
           local_ptr+=2;
           if(wisun_hdr_id ==  WH_IE)                
           {
             if(sub_id==WH_IE_SUBID_BT_IE_SHORT)
             {
               return (local_ptr+1);
             } 
             else
             {
               local_ptr+=content_len;
               i+=(2+content_len);
               
             }
           }
           else
           {
             local_ptr+=content_len;
             i+=(2+content_len);
           }
        }
      return NULL;
}

/******************************************************************************/
const float k1 = 256;   /* 2^24 divided by 2^16 */
//uint64_t ufsi_calculation[50];
//int hopping_test_index = 0;
//uint16_t packet_length_see[50];
uint32_t update_ufsi_with_procces_time(mac_tx_t *out_packt)
{
  
  uint32_t ufsi = 0;
  
#if(FAN_FRQ_HOPPING_FEATURE_ENABLED == 1)  

  uint32_t t1_t0 = 0;
  uint64_t time_now = get_time_now_64();
  
  /*T1 – T0 is the milliseconds since the beginning of the channel hopping sequence for Node1
  SL (Sequence length) is the number of slots in the channel hopping sequence.
  SL = N for the TR51 Channel Function which is the number of valid channels in the channel table
  SL = 2^16 for the Direct Hash Channel Function
  UFSINode1 = floor ( ( Milliseconds since beginning of sequence * 2^24) / Duration of
  sequence in milliseconds)*/


  if(mac_self_fan_info.bcast_sched.bs_schedule.channel_function== CF_DH1)
  {
/* Debdeep :: Different offset time is required for different type of packets.
These values are measured in run time with CCA disabled and UPDATE_UFSI_AFTER_CCA = 0.
Whenever CCA will be enabled and  UPDATE_UFSI_AFTER_CCA will be 1, 
then again we need to measure these values properly*/
    if(out_packt->sec_param.security_level == MAC_SECURITY_NONE)
    {
      if (out_packt->sub_type == FAN_ACK)
        t1_t0 = (uint32_t) ceil((float)((time_now + 1055) - 
                                        trxsm_p->uc_chan_hop_seq_start_time)/(float)1000);
      #if(FAN_EAPOL_FEATURE_ENABLED == 1)
      else if (out_packt->sub_type == EAPOL)
        t1_t0 = (uint32_t) ceil((float)((time_now + 1640) - 
                                        trxsm_p->uc_chan_hop_seq_start_time)/(float)1000);
#endif
      else
        t1_t0 = (uint32_t) ceil((float)((time_now + 1520) - 
                                        trxsm_p->uc_chan_hop_seq_start_time)/(float)1000);
    }
    else
    {
      if (out_packt->sub_type == PAN_CONFIG)
        t1_t0 = (uint32_t) ceil((float)((time_now + 1827) - 
                                       trxsm_p->uc_chan_hop_seq_start_time)/(float)1000);
      else if (out_packt->sub_type == FAN_ACK)
        t1_t0 = (uint32_t) ceil((float)((time_now + 1180) - 
                                        trxsm_p->uc_chan_hop_seq_start_time)/(float)1000);
      else
      {
        /* Debdeep :: Time required for doing security encryption on a packet depends on it's length. 
        So we calculate time offset using the packet length. This equation is derived from time stamp statistics */
        uint64_t offset = (out_packt->security_data->payload_length * 2) + 1250;
        
        t1_t0 = (uint32_t) ceil((float)((time_now + offset) - 
                                        trxsm_p->uc_chan_hop_seq_start_time)/(float)1000);
//        if (hopping_test_index < 50)
//        {
//          packet_length_see[hopping_test_index] = out_packt->security_data->payload_length;
//          ufsi_calculation[hopping_test_index] = get_time_now_64 ();
//        }
      }
    }
    float raw_ufsi = (float)t1_t0 * (float)(k1/(float)mac_self_fan_info.unicast_listening_sched.us_schedule.dwell_interval);
    ufsi = (uint32_t)floor(raw_ufsi);
  }
  else if(mac_self_fan_info.bcast_sched.bs_schedule.channel_function == CF_TR51)
  {
    //PLME_get_request( phyMaxSUNChannelSupported, &length, &current_max_channels );
    //6690 is clock drift to send data packet UFSI_UPDATE + TX_COMPLete 
    //ufsi = ((((timer_current_time_get() + 6690) - trxsm_p->uc_chan_hop_seq_start_time)*2^24)/current_max_channels*mac_self_fan_info.unicast_listening_sched.us_schedule.dwell_interval);  
  }
  
#endif
  
  return ufsi;      
}


/******************************************************************************/
uint32_t  update_bfio_with_procces_time(mac_tx_t *out_packt)
{
  uint32_t bfio = 0;
  uint64_t time_now = get_time_now_64();
  
/* Debdeep :: Different offset time is required for different type of packets.
These values are measured in run time with CCA disabled and UPDATE_UFSI_AFTER_CCA = 0.
Whenever CCA will be enabled and  UPDATE_UFSI_AFTER_CCA will be 1, 
then again we need to measure these values properly*/ 
  if(out_packt->sec_param.security_level == MAC_SECURITY_NONE)
  {
    if (out_packt->sub_type == FAN_ACK)
      bfio = ((time_now + 1055) - (trxsm_p->bc_chan_hop_seq_current_slot_time))/1000;
#if(FAN_EAPOL_FEATURE_ENABLED == 1)
    else if (out_packt->sub_type == EAPOL)
      bfio = ((time_now + 1640) - (trxsm_p->bc_chan_hop_seq_current_slot_time))/1000;
#endif
    else
      bfio = ((time_now + 1520) - (trxsm_p->bc_chan_hop_seq_current_slot_time))/1000;
  }
  else
  {
    if (out_packt->sub_type == PAN_CONFIG)
      bfio = ((time_now + 1827) - (trxsm_p->bc_chan_hop_seq_current_slot_time))/1000;
    else if (out_packt->sub_type == FAN_ACK)
      bfio = ((time_now + 1180) - (trxsm_p->bc_chan_hop_seq_current_slot_time))/1000;
    else
    {
      /* Debdeep :: Time required for doing security encryption on a packet depends on it's length. 
      So we calculate time offset using the packet length. This equation is derived from time stamp statistics */
      uint64_t offset = (out_packt->security_data->payload_length * 2) + 1250;
      
      bfio = ((time_now + offset) - (trxsm_p->bc_chan_hop_seq_current_slot_time))/1000;
    }
  }

  return bfio;
}

/******************************************************************************/
/*maintaing sequence number table for updating ack receive status of packet sent
*/

//void update_ack_received(uint8_t *ack_rcvd_addr, uint8_t *self_addr)
//{
//  static uint8_t i = 0;
//  for(i = 0; i < MAX_NBR_SUPPORT; i++)
//  { 
//    if(!memcmp(seq_number_table[i].dest_addr, ack_rcvd_addr, 8) &&
//       !memcmp(seq_number_table[i].src_addr, self_addr, 8))
//    {
//      seq_number_table[i].dev_ack_status = ACK_RECEIVED;
//      seq_number_table[i].dev_join_status = DEV_NOT_ADDED;
//      return;
//    }      
//  }
//  return;
//}

#endif


uint8_t is_configured_as_fixed_channel (void)
{
  if (mac_self_fan_info.unicast_listening_sched.us_schedule.channel_function == CF_FIXED_CHANNEL)
    return 1;
  else
    return 0;
}
