/** \file mac_interface_layer.c
 *******************************************************************************
 ** \brief This module describes about the MAC API Layer
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
#include "phy.h"

#include "mac_config.h"
#include "mac.h"
#include "mac_defs_sec.h"
#include "ie_element_info.h"
#include "mac_defs.h"
#include "mac_pib.h"
#include "event_manager.h"
#include "fan_mac_ie.h"
#include "mac_interface_layer.h"

/*******************************************************************************
* Private macro definitions
*******************************************************************************/

/* None */

/******************************************************************************
* Private Structures, Unions & enums Type Definitions
******************************************************************************/

/* None */

/*Umesh : 16-01-2018 added temp here*/
#ifdef WISUN_FAN_MAC 
extern void process_fan_mcps_data_confirm( uint8_t* data );
extern void process_fan_mcps_data_indication( uint8_t* data );
extern void process_ws_async_frame_confirm( uint8_t* data );
extern void process_ws_async_frame_indication( uint8_t *data );
extern void process_fan_mac_ack_indication(uint8_t* data );
extern void process_fan_mac_no_ack_indication(uint8_t* data );
#endif


void send_mac_2_nhle_conf (void);

/*
** ============================================================================
** Private Function Prototypes
** ============================================================================
*/

//static void mac_interface_address_parser(uchar **p, mac_address_t *address);
void mac_interface_address_parser(uchar **p, mac_address_t *address);

//static void mac_interface_addr_parse_From_Buff(mac_address_t *addr, uchar **p);
void mac_interface_addr_parse_From_Buff(mac_address_t *addr, uchar **p);

//static uint8_t find_length(uint8_t mode);

//static void parse_IEs(uchar **p,uint8_t *payloadIElist,uint8_t payloadIElistLen);
void parse_IEs(uchar **p,uint8_t *payloadIElist,uint8_t payloadIElistLen);

static queue_t QUEUE_MCPS_2_NHLE,QUEUE_MLME_2_NHLE;

static void process_mac_response( msg_t* p_msg );

#if(CFG_MLME_ASSOCIATE_IND_RESP == 1)
static void process_mlme_associate_indication( uint8_t* data );
#endif	/*(CFG_MLME_ASSOCIATE_IND_RESP == 1)*/

#if(CFG_MLME_START_REQ_CONF == 1)
static void process_mlme_start_confirm( uint8_t* data );
/*Uemsh : 11-01-2018 this thing goes to modified after sepration of mac and fan mac*/
#ifdef WISUN_FAN_MAC 
static void process_mlme_set_mac_channel_info_confirm ( uint8_t* data );
static void process_fan_mac_mlme_set_confirm ( uint8_t* data );
#endif /*(end of WISUN_FAN_MAC)*/

#endif	/*(CFG_MLME_START_REQ_CONF == 1)*/

static void process_mcps_data_confirm( uint8_t* data );
static void process_mcps_data_indication( uint8_t* data );

#if(CFG_MLME_GET_REQ_CONF == 1)
static void process_mlme_get_confirm( uint8_t* data );
#endif	/*(CFG_MLME_GET_REQ_CONF == 1)*/

#if(CGF_MLME_RESET_REQ_CONF == 1)
static void process_mlme_reset_confirm( uint8_t* data );
#endif	/*(CGF_MLME_RESET_REQ_CONF == 1)*/

#if(CFG_MLME_SET_REQ_CONF ==1)
static void process_mlme_set_confirm( uint8_t* data );
#endif	/*(CFG_MLME_SET_REQ_CONF ==1)*/

#if(CFG_MLME_SCAN_REQ_CONF == 1 )
static void process_mlme_scan_confirm( uint8_t* data );
#endif	/*(CFG_MLME_SCAN_REQ_CONF == 1 )*/

#if(CGF_MLME_COMM_STATUS_IND == 1)
static void process_mlme_comm_status_indication( uint8_t* data );
#endif /*(CGF_MLME_COMM_STATUS_IND == 1)*/

#if(CGF_MLME_BEACON_NOTIFY_IND == 1)
static void process_mlme_beacon_notify_indication( uint8_t* data );
#endif	/*(CGF_MLME_BEACON_NOTIFY_IND == 1)*/

#if(CFG_MLME_ASSOCIATE_REQ_CONF == 1)
static void process_mlme_associate_confirm( uint8_t* data );
#endif	/*(CFG_MLME_ASSOCIATE_REQ_CONF == 1)*/

#if(CFG_MLME_POLL_REQ_CONF == 1)
static void process_mlme_poll_confirm( uint8_t* data );	
#endif	/*(CFG_MLME_POLL_REQ_CONF == 1)*/

#if(CFG_MLME_DISASSOCIATE_IND == 1)
static void process_mlme_disassociate_indication( uint8_t* data );
#endif	/*(CFG_MLME_DISASSOCIATE_IND == 1)*/

#if(CFG_MLME_DISASSOCIATE_REQ_CONF == 1)	
static void process_mlme_disassociate_confirm( uint8_t* data );
#endif	/*(CFG_MLME_DISASSOCIATE_REQ_CONF == 1)*/

#if(CFG_MLME_ORPHAN_IND_RESP == 1)
static void proces_mlme_orphan_indication( uint8_t* data );
#endif	/*(CFG_MLME_ORPHAN_IND_RESP == 1)*/

static void process_mlme_syncloss_indication( uint8_t* data );

#if(CFG_MCPS_PURGE_REQ_CONF == 1)
static void process_mcps_purge_confirm( uint8_t* data );
#endif	/*(CFG_MCPS_PURGE_REQ_CONF == 1)*/

#if(CGF_MLME_BEACON_REQUEST_IND == 1)
static void process_mlme_beacon_request_indication(uint8_t* data);
#endif	/*(CGF_MLME_BEACON_REQUEST_IND == 1)*/

#if(CGF_MLME_BEACON_REQ_CONF == 1)
static void process_mlme_beacon_confirm( uint8_t* data );
#endif	/*(CGF_MLME_BEACON_REQ_CONF == 1)*/

//static void process_mac_set_phy_mode_confirm( uint8_t* data );
static void mil_churn( void );

static void free_data_msgs( queue_t* p_queue );

#ifdef MAC_CFG_SECURITY_ENABLED
 uint8_t append_security_parameters(uint8_t** p_to, security_params_t* pSecstuff );
uint8_t load_security_param_struct(security_params_t* pToSecstuff, uint8_t** p_to );
#endif


const mac_to_nhle_process_table_t process_table[] =
{
#if(CFG_MLME_ASSOCIATE_IND_RESP == 1)
	{ MAC_MLME_ASSOCIATE_INDICATION	  	, process_mlme_associate_indication		  	},
#endif	/*(CFG_MLME_ASSOCIATE_IND_RESP == 1)*/
#if(CFG_MLME_START_REQ_CONF == 1)	
	{ MAC_MLME_START_CONFIRM		    , process_mlme_start_confirm			    },
/*Uemsh : 11-01-2018 this thing goes to modified after sepration of mac and fan mac*/
#ifdef WISUN_FAN_MAC        
        { MAC_MLME_SET_MAC_CHANNEL_INFO_CONFIRM	     , process_mlme_set_mac_channel_info_confirm            },
        { FAN_MAC_MLME_SET_CONFIRM     , process_fan_mac_mlme_set_confirm            },
#endif /*(end of WISUN_FAN_MAC)*/
#endif	/*(CFG_MLME_START_REQ_CONF == 1)*/

	{ MAC_MCPS_DATA_CONFIRM				, process_mcps_data_confirm		      		},
	{ MAC_MCPS_DATA_INDICATION		    , process_mcps_data_indication	    		},
/*Uemsh : 11-01-2018 this thing goes to modified after sepration of mac and fan mac*/
#ifdef WISUN_FAN_MAC        
        { FAN_MAC_MCPS_DATA_CONFIRM		    , process_fan_mcps_data_confirm		      		},
        { FAN_MAC_MCPS_DATA_INDICATION		    , process_fan_mcps_data_indication		      		},
        { FAN_MAC_ACK_INDICATION                    , process_fan_mac_ack_indication                            },
        { FAN_MAC_NO_ACK_INDICATION                 , process_fan_mac_no_ack_indication                         },
        { WS_ASYNC_FRAME_REQ_CONF		    , process_ws_async_frame_confirm		      		},
	{ WS_ASYNC_FRAME_INDICATION		    , process_ws_async_frame_indication	    		},
#endif 
        
#if(CFG_MLME_GET_REQ_CONF == 1)	
	{ MAC_MLME_GET_CONFIRM				, process_mlme_get_confirm				    },
#endif	/*(CFG_MLME_GET_REQ_CONF == 1)*/
	
	{ MAC_MLME_RESET_CONFIRM		    , process_mlme_reset_confirm			    },
	{ MAC_MLME_SET_CONFIRM			    , process_mlme_set_confirm		      		},
#if(CFG_MLME_SCAN_REQ_CONF == 1 )	
	{ MAC_MLME_SCAN_CONFIRM			    , process_mlme_scan_confirm			      	},
#endif	/*(CFG_MLME_SCAN_REQ_CONF == 1 )*/	
#if(CGF_MLME_COMM_STATUS_IND == 1)
	{ MAC_MLME_COMM_STATUS_INDICATION  	, process_mlme_comm_status_indication   	},
#endif     
#if(CGF_MLME_BEACON_NOTIFY_IND == 1)	
	{ MAC_MLME_BEACON_NOTIFY_INDICATION , process_mlme_beacon_notify_indication 	},
#endif	/*(CGF_MLME_BEACON_NOTIFY_IND == 1)	*/

#if(CFG_MLME_ASSOCIATE_REQ_CONF == 1)		
	{ MAC_MLME_ASSOCIATE_CONFIRM		, process_mlme_associate_confirm		    },
#endif	/*(CFG_MLME_ASSOCIATE_REQ_CONF == 1)*/
#if(CFG_MLME_POLL_REQ_CONF == 1)	
    { MAC_MLME_POLL_CONFIRM				, process_mlme_poll_confirm			       	},
#endif	/*(CFG_MLME_POLL_REQ_CONF == 1)*/

#if(CFG_MLME_DISASSOCIATE_IND == 1)
    { MAC_MLME_DISASSOCIATE_INDICATION  , process_mlme_disassociate_indication  	},
#endif	/*(CFG_MLME_DISASSOCIATE_IND == 1)*/
    
#if(CFG_MLME_DISASSOCIATE_REQ_CONF == 1)    
    { MAC_MLME_DISASSOCIATE_CONFIRM     , process_mlme_disassociate_confirm     	},
#endif	/*(CFG_MLME_DISASSOCIATE_REQ_CONF == 1)*/

#if(CFG_MLME_ORPHAN_IND_RESP == 1)    
    { MAC_MLME_ORPHAN_INDICATION        , proces_mlme_orphan_indication 			},
#endif	/*(CFG_MLME_ORPHAN_IND_RESP == 1)*/

    { MAC_MLME_SYNC_LOSS_INDICATION     , process_mlme_syncloss_indication			},

#if(CFG_MCPS_PURGE_REQ_CONF == 1)    
	{ MAC_MCPS_PURGE_CONFIRM   			, process_mcps_purge_confirm				},
#endif	/*(CFG_MCPS_PURGE_REQ_CONF == 1)*/

#if(CGF_MLME_BEACON_REQUEST_IND == 1)
	{ MAC_MLME_BEACON_REQUEST_INDICATION, process_mlme_beacon_request_indication	},
#endif	/*(CGF_MLME_BEACON_REQUEST_IND == 1)*/
		
#if(CGF_MLME_BEACON_REQ_CONF == 1)	
	{ MAC_MLME_BEACON_CONFIRM			, process_mlme_beacon_confirm				},
#endif	/*(CGF_MLME_BEACON_REQ_CONF == 1)	*/
	{ 0                                 , NULL_POINTER                          	}
};

/*
** ============================================================================
** Public Variable Definitions
** ============================================================================
*/

/* None*/

/*
** ============================================================================
** Private Variable Definitions
** ============================================================================
*/
/*Umesh :15-01-2018*/
/*this varriable is move to fan_mac_interface_layer.c*/
//static uint16_t data_tx_triggered_failed = 0;
/*Umesh : 02-01-2018*/
//static uint16_t alloc_length_copy = 0;
/*this variable not used*/
/*
** ============================================================================
** External Variable Declarations
** ============================================================================
*/

extern uchar mac_process_beacons(void);
extern uchar mac_process_data_requests(void);
extern uchar mac_process_received_messages(void);
extern uchar process_completed_dtx_messages( void );
//extern void process_pd_2_mac_incoming_frames( void ); Amarjeet: Changed for IAR Porting, commented as it is not used


#if(CFG_MLME_RX_ENABLE_REQ_CONF == 1)
extern uchar mac_process_receiver_enable_state(void);
#endif	/*(CFG_MLME_RX_ENABLE_REQ_CONF == 1)*/

extern uchar check_pending_data(void);
extern uchar mac_check_expired_pending_messages( void );
extern uchar mac_check_cap_message_ucast(void);
extern uchar mac_check_cap_message_bcast();
extern uchar mac_check_panid_conflict_state( void );

#ifdef MAC_CFG_SECURITY_ENABLED
	extern void mac_process_secured_messages(void);
	extern uint8_t mac_incoming_security_processor( void );
	extern uint8_t mac_outgoing_security_processor( void );
	extern void cleanup_security_queues(uchar SetDefaultPIBValue);
#endif

extern void TransmitProcess(void);
extern void ReceiveProcess(void);
//extern void indicate_mac_2_nhle( uint8_t );
void process_phy_packet ( void );

extern uint8_t heap[];
extern mac_pib_t mac_pib;

extern void * app_bm_alloc(
    uint16_t length//base_t length      
    );
    
extern void app_bm_free(
    uint8_t *pMem      
    );

extern void timer_task(void);

/*
** ============================================================================
** Public Function Definitions
** ============================================================================
*/

void MIL_Init( uint8_t cold_start )
{
    queue_initialise( &QUEUE_MCPS_2_NHLE );

    /*initialize the queues needed for storing the Mgmt conf and ind primitives */
    queue_initialise( &QUEUE_MLME_2_NHLE );

    PHY_Init(cold_start);

    mac_initialise( cold_start );
}


/******************************************************************************/

void MIL_Task( void )
{
	uint32_t event;
     
	while(( event = highest_prio_event_get()) != MAX_BASE_VALUE )
	{
		switch( event )
		{
						
			case TIMER_EXPIRY_EVENT:
				
				timer_task();
              
				break;
                                
//                        case PHY_2_MAC_EVENT:
//                          process_phy_packet ();
//                          break;
                          
#if ((RADIO_VALIDATION || SNIFFER) || (APP_HIF_PROCESS_FEATURE_ENABLED == 0) )	
			case HIF_RX_EVENT:

				ReceiveProcess();
				break;
				
			case HIF_TX_EVENT:

				TransmitProcess();
				break;
#endif			
				
#ifdef MAC_CFG_SECURITY_ENABLED					
			case RX_SEC_FRAME_EVENT:

				if (mac_pib.mac_security_enabled == TRUE)
				{  
					mac_process_secured_messages();
				}
				break;
			case UNSECURE_EVENT:

				mac_incoming_security_processor();
				break;
#endif				
#if(CFG_MAC_BEACON_ENABLED == 1) 
                        case BCN_RX_EVENT:
				mac_process_beacons();	
				break;
#endif                                
				
			case DATA_REQ_RX_EVENT:

				mac_process_data_requests();
				break;
				
			case CMD_OR_DATA_FRAME_RX_EVENT:

				mac_process_received_messages();
				break;
#ifdef MAC_CFG_SECURITY_ENABLED					
			case SECURE_EVENT:

				mac_outgoing_security_processor();
				break;
#endif									
			case PENDING_TX_EVENT_UCAST:
				mac_check_cap_message_ucast();
				break;
				
            case PENDING_TX_EVENT_BCAST:
            	mac_check_cap_message_bcast();
            	break;
			case FRAME_TX_DONE_EVENT:

				process_completed_dtx_messages();
				break;
			case MLME_EVENT:

				process_mlme_msgs();
				break;
			case MCPS_EVENT:

				process_mcps_msgs();
				break;
//			case MAC_2_NHLE_EVENT:
//
//				process_mil_msgs();
//				break;				
			default:
				break;
	    }
	}
   
	mil_churn();
}

/******************************************************************************/

void process_mil_msgs( void )
{
  msg_t* nhle_msg = ( msg_t* )queue_item_get( &QUEUE_MLME_2_NHLE );
  
  if ( nhle_msg != NULL )
  {
    process_mac_response( nhle_msg );
    free_mac_2_nhle_msg( nhle_msg );
    //event_clear(MAC_2_NHLE_EVENT);
  }
  
  nhle_msg = ( msg_t* )queue_item_get( &QUEUE_MCPS_2_NHLE );
  
  if ( nhle_msg != NULL )
  {
    process_mac_response( nhle_msg );
//#if (PRINT_DEBUG_LEVEL==ERROR_DEBUG)
//    stack_print_debug ("1F data %p\n", nhle_msg);
//#endif 
    free_mac_2_nhle_msg( nhle_msg );
    //event_clear(MAC_2_NHLE_EVENT);
  }
}

/******************************************************************************/

static void process_mac_response(msg_t* p_mac_mhle_msg )
{
	mac_to_nhle_process_table_t *pt = NULL;
	//uchar primitive;

	/* check the primitive (first byte is the length) */
	//primitive = p_mac_mhle_msg->data[0];

	for( pt = (mac_to_nhle_process_table_t *) process_table; pt->primitive != 0; pt++ )
	{
		if( pt->primitive == p_mac_mhle_msg->data[0] )
		{
			/*pass the prmitive data for processing*/
			pt->action( &(p_mac_mhle_msg->data[1]) );
			return ; /* break out of for loop */
		} /* end of if */
	} /* end of for */
   return;
}

/******************************************************************************/
//uint16_t data_tx_triggered = 0;

/******************************************************************************/
#if(CFG_MCPS_PURGE_REQ_CONF == 1)
void MCPS_PURGE_Request
	(
	 uchar	msduHandle		/*	Handle associated with payload	*/
	)
{
	uchar *buffer = NULL;
	msg_t *msg = NULL;

#define PURGE_REQUEST_LENGTH 2
	msg = allocate_nhle_2_mac_msg( PURGE_REQUEST_LENGTH );
	if( msg == NULL_POINTER )
	{
		return;
	}
	msg->data_length = PURGE_REQUEST_LENGTH;

	buffer = msg->data;

	buffer[0] = MAC_MCPS_PURGE_REQUEST;
	buffer[1] = msduHandle;

	/* write to the MAC interface*/
	send_nhle_2_mcps( msg );
	event_set(MCPS_EVENT);
	//signal_event_to_mac_task();

} /* end MCPS_Purge_request */

#endif	/*(CFG_MCPS_PURGE_REQ_CONF == 1)*/

/******************************************************************************/
#if(CFG_MLME_ASSOCIATE_REQ_CONF == 1)
void MLME_ASSOCIATE_Request
	(
	uint8_t LogicalChannel,
	uint8_t channelPage,
	mac_address_t *coordAddr,
	uint8_t CapabilityInfo,
	uint8_t* pLowLatencyNwkInfo,
	uint16 channelOffset,
	uint8_t  HoppingSequenceID,
	security_params_t* pSecstuff 
	)
{
	//uchar *mac_wrp = NULL_POINTER;			/* pointer to the packet to write to the */
	uchar *p = NULL_POINTER;
	msg_t *msg = NULL;
	//int allocation_length;	/* length of memory to allocate */
	//int i = 0;
	
	int allocation_length = 6 +	 sizeof(security_params_t) + find_length(coordAddr->address_mode);
							 // + sizeof( uchar )	/* primitive    id */
							 // + sizeof( uchar )	/* logical channel */
							 // + sizeof( uchar )	/* dest addr mode */
							 // + sizeof( uchar )	/* capability info */
							 // + sizeof( uchar ) /* channelOffset enable*/
							 // + sizeof( uchar ) /* HoppingSequenceID */
							 // + sizeof(security_params_t); /*security params*/

	//allocation_length += find_length(coordAddr->address_mode);
	/*switch( coordAddr->address_mode )
		{
		//values for address modes, as passed in primitives 
		// lengths (including length of PANID field 
		case ADDR_MODE_NONE:			
		case ADDR_MODE_RESVD:
			//allocation_length += ADDR_MODE_RESVD_LENGTH;
			break;

		case ADDR_MODE_SHORT:
			allocation_length += ADDR_MODE_SHORT_LENGTH;
			break;

		case ADDR_MODE_EXTENDED:
			allocation_length += ADDR_MODE_EXTENDED_LENGTH;
			break;
		}*/
	msg= allocate_nhle_2_mac_msg( allocation_length + 20/*safer*/ );
	
	if( msg == NULL_POINTER )
	{
		return;
	}
	
	p = msg->data;

	msg->data_length = allocation_length;
	/* allocate some memory */

	if( p == NULL_POINTER )
	{
		return;
	}
	//p = mac_wrp;

	*p++ = MAC_MLME_ASSOCIATE_REQUEST;
	*p++ = LogicalChannel;
	*p++ = channelPage;


	mac_interface_address_parser(&p, coordAddr);
	/*switch( coordAddr->address_mode )
		{
		case ADDR_MODE_NONE:
		case ADDR_MODE_RESVD:
			break;

		case ADDR_MODE_SHORT:			
			*p++ = coordAddr->address.short_address % 256;
			*p++ = coordAddr->address.short_address / 256;
			break;

		case ADDR_MODE_EXTENDED:			
			memcpy(p,coordAddr->address.ieee_address,8);
			p += IEEE_ADDRESS_LENGTH;
			break;
		}*/
		
	*p++ = CapabilityInfo;
	
	*p++ = 0x0; //pLowLatencyNwkInfo is being sent as 0
	
	put_ushort(p,channelOffset);
	
	p += 2;
	
	*p++ = HoppingSequenceID;
#ifdef MAC_CFG_SECURITY_ENABLED		
	append_security_parameters(&p,pSecstuff);
#endif	
	
	/* write to the MAC interface*/
	send_nhle_2_mlme( msg );
	event_set(MLME_EVENT);
	//signal_event_to_mac_task();
} /* end of MLME_ASSOCIATE_Request */

#endif	/*(CFG_MLME_ASSOCIATE_REQ_CONF == 1)*/

/******************************************************************************/

#if(CFG_MLME_ASSOCIATE_IND_RESP == 1)

void MLME_ASSOCIATE_Response
(
	uint8_t *DeviceExtAddress,
	uint16_t  AssocShortAddress,
	uint8_t  Status,
	uint8_t* pLowLatencyNwkInfo,
	uint16 channelOffset,
	uint16 channelHoppingSeqLength,
	uint8_t* pchannelHoppingSeq,
	security_params_t* pSecstuff
)
{
#define ASSOCIATE_RESPONSE_LENGTH 20

	msg_t *msg = NULL;
	uchar *p = NULL_POINTER;
	//int i;

	/* allocate a message buffer */
	msg = allocate_nhle_2_mac_msg( ASSOCIATE_RESPONSE_LENGTH + 20 +  sizeof(security_params_t));
	
	if( msg == NULL_POINTER )
	{
		return;
	}
	
	msg->data_length = ASSOCIATE_RESPONSE_LENGTH;

	p = msg->data;

	*p++ = MAC_MLME_ASSOCIATE_RESPONSE;

	memcpy( p,DeviceExtAddress,IEEE_ADDRESS_LENGTH );
	
	p+= IEEE_ADDRESS_LENGTH;
	
	*p++ = AssocShortAddress % 256;
	*p++ = AssocShortAddress / 256;

	*p++ = Status;
	
	*p++ = 0x0; //pLowLatencyNwkInfo is being sent as 0
	
	put_ushort(p,channelOffset);
	
	p += 2;
	
	put_ushort(p,channelHoppingSeqLength);
	
	p += 2;
	
	*p++ = 0; //pchannelHoppingSeq is being sent as 0
	
#ifdef MAC_CFG_SECURITY_ENABLED		
	append_security_parameters(&p,pSecstuff);
#endif	

	/* write to the MAC interface*/
	send_nhle_2_mlme( msg );
	event_set(MLME_EVENT);
	//signal_event_to_mac_task();

} /* end of MLME_ASSOCIATE_Response */

#endif	/*(CFG_MLME_ASSOCIATE_IND_RESP == 1)*/

/******************************************************************************/
#if(CFG_MLME_GET_REQ_CONF == 1)
void MLME_GET_Request
(
	uint8_t PIBAttribute,
	uint8_t PIBAttributeIndex
)
{
#define GET_REQUEST_LENGTH 3
	msg_t *msg = NULL;
	uchar *buffer = NULL;

	/* allocate a message buffer */
	msg = allocate_nhle_2_mac_msg( GET_REQUEST_LENGTH );
	
	if( msg == NULL_POINTER )
	{
		return;
	}
	
	buffer = msg->data;
	buffer[0] = MAC_MLME_GET_REQUEST;
	buffer[1] = PIBAttribute;
	buffer[2] = PIBAttributeIndex;

	/* write to the MAC interface*/
	send_nhle_2_mlme( msg );
	event_set(MLME_EVENT);
	//signal_event_to_mac_task();

} /* end of MLME_GET_Request */
#endif	/*(CFG_MLME_GET_REQ_CONF == 1)*/

/******************************************************************************/

void MLME_SYNC_Request
(
 uint8_t LogicalChannel, 
 uint8_t ChannelPage,
 uint8_t TrackBeacon         
 )
{
#define SYNC_REQUEST_LENGTH       4			
   	msg_t *msg = NULL;
	uchar *buffer = NULL;
	
	/* allocate a message buffer */
	msg = allocate_nhle_2_mac_msg( SYNC_REQUEST_LENGTH );
	
	if( msg == NULL_POINTER )
	{
		return;
	}

	msg->data_length = SYNC_REQUEST_LENGTH;

	buffer = msg->data;

	*buffer++ = MAC_MLME_SYNC_REQUEST;
	*buffer++ = LogicalChannel;
	*buffer++ = ChannelPage;
	*buffer++ = TrackBeacon;
	
	/* submit to MAC layer*/
	send_nhle_2_mlme( msg );
	event_set(MLME_EVENT);
} 

/******************************************************************************/
#if (CFG_MLME_SCAN_REQ_CONF == 1)

void MLME_SCAN_Request
(
	uint8_t	ScanType,				
	uint64_t scanChannels,		
	uint8_t	ScanDuration,
	uint8_t channelPage,
	bool LinkQualityScan,
	uint8_t frameControlOptions,
	uint8_t  headerIElistLen,
	uint8_t* headerIElist,
	uint8_t payloadIElistLen,
	uint8_t* payloadIElist,	
	uint8_t mpm_scanduration_bpan,
	uint16_t mpm_scanduration_nbpan,
	security_params_t* pSecstuff  
)

{
#define SCAN_REQUEST_LENGTH 22
	msg_t *msg = NULL;
	uchar *buffer = NULL;
	//uchar short_subIdsCnt,long_subIdsCnt;

	/* allocate a message buffer */
	msg = allocate_nhle_2_mac_msg( SCAN_REQUEST_LENGTH  + sizeof(security_params_t) );
	
	if( msg == NULL_POINTER )
	{
		return;
	}

	msg->data_length = SCAN_REQUEST_LENGTH;

	buffer = msg->data;

	*buffer++ = MAC_MLME_SCAN_REQUEST;
	*buffer++ = ScanType;
	/*buffer[2] = (uchar) (( ScanChannels[1] >>  24 ) & 0xFF);
	buffer[3] = (uchar) (( ScanChannels[1] >>  16 ) & 0xFF);
	buffer[4] = (uchar) (( ScanChannels[1] >>  8 ) & 0xFF);
	buffer[5] = (uchar) (( ScanChannels[1] ) & 0xFF);
	
	buffer[6] = (uchar) (( ScanChannels[0] >>  24 ) & 0xFF);
	buffer[7] = (uchar) (( ScanChannels[0] >>  16 ) & 0xFF);
	buffer[8] = (uchar) (( ScanChannels[0] >>  8 ) & 0xFF);
	buffer[9] = (uchar) (( ScanChannels[0] ) & 0xFF);*/
	
	mem_rev_cpy(buffer , (uint8_t*)&scanChannels,8 );
	
	buffer += 8;

	*buffer++ = ScanDuration;
	
	*buffer++ = channelPage;
	*buffer++ = LinkQualityScan;
	*buffer++ = frameControlOptions;
	
	*buffer++ = headerIElistLen;
	memcpy(buffer,headerIElist,headerIElistLen);
	
	buffer += headerIElistLen;
	
	*buffer++ = payloadIElistLen;

	parse_IEs(&buffer, payloadIElist, payloadIElistLen);

	/*while(payloadIElistLen)
	{
		if( *payloadIElist != 0x09)//MLME IE ID
		{
			*buffer++ = *payloadIElist++;		
		}
		else
		{
			*buffer++ = *payloadIElist++;
			//p_attrib_val is pointing to sub IDs count field
			short_subIdsCnt = *payloadIElist++;
			long_subIdsCnt = *payloadIElist++;

			*buffer++ = short_subIdsCnt;
			*buffer++ = long_subIdsCnt;

			memcpy( buffer, payloadIElist, short_subIdsCnt );
			payloadIElist += short_subIdsCnt;
			
			buffer += short_subIdsCnt;
			
			memcpy( buffer, payloadIElist, long_subIdsCnt );
			
			payloadIElist += long_subIdsCnt;
			
			buffer += long_subIdsCnt;
			
		}
		payloadIElistLen--;
	}*/
		
	*buffer++ = mpm_scanduration_bpan;
	
	*buffer++ = mpm_scanduration_nbpan % 256;
	*buffer++ = mpm_scanduration_nbpan / 256;

#ifdef MAC_CFG_SECURITY_ENABLED		
	append_security_parameters(&buffer,pSecstuff);
#endif

	/* write to the MAC interface*/
	send_nhle_2_mlme( msg );
	event_set(MLME_EVENT);
	//signal_event_to_mac_task();
}

#endif  /* (CFG_MLME_SCAN_REQ_CONF == 1) */

/******************************************************************************/
	
void MLME_SET_Request
	(
              uint8_t PIBAttribute,		
              uint8_t PIBAttributeIndex,
              uint16_t PIBAttributeLength,	
              void *PIBAttributeValue	
	)
{
	msg_t *msg = NULL;
	uchar *buffer = NULL;
	uint16_t buffer_len = PIBAttributeLength;
	
#ifdef MAC_CFG_SECURITY_ENABLED	
	if( PIBAttribute == macKeyTable )
	{
		//buffer_len = get_mac_sec_table_attrib_len( PIBAttribute, PIBAttributeValue);
	}
	else if ( PIBAttribute == macDeviceTable )
	{
		buffer_len = 17;
	}
	else if ( PIBAttribute == macSecurityLevelTable )
	{
		buffer_len = 4;
	}
#endif
	
	/* allocate a message buffer */
	msg = allocate_nhle_2_mac_msg( buffer_len + 6 );

	if( msg == NULL_POINTER )
	{
#ifdef MAC_CFG_SECURITY_ENABLED	
          //cleanup_security_queues(1);  // Raka :: why the hell we are clearing the security pib's
#endif				
		App_MLME_SET_Conf_cb
		(
		 MAC_TRANSACTION_OVERFLOW,		//uint8_t status,
		 PIBAttribute,		//uint8_t PIBAttribute,
		 PIBAttributeIndex		//uint8_t PIBAttributeIndex
		); 			
	return;
	}

	msg->data_length = buffer_len + 6;

	buffer = msg->data;

	buffer[0] = MAC_MLME_SET_REQUEST;
	buffer[1] = PIBAttribute;
	buffer[2] = PIBAttributeIndex;
	
	put_ushort(buffer + 3,PIBAttributeLength);
	memcpy( &buffer[5], PIBAttributeValue, buffer_len );

	/* write to the MAC interface */
	send_nhle_2_mlme( msg );
	event_set(MLME_EVENT);
	//signal_event_to_mac_task();
}

/******************************************************************************/
#if(CGF_MLME_RESET_REQ_CONF == 1)
void MLME_RESET_Request
	(
	 uchar	SetDefaultPIB	/*	Also Reset PIB		(B1)*/
	)
{
	msg_t *msg = NULL;
	uchar *buffer = NULL;

#define RESET_REQUEST_LENGTH 2

#ifdef MAC_CFG_SECURITY_ENABLED
	cleanup_security_queues(SetDefaultPIB);
#endif

	/* allocate a message buffer */
	msg = allocate_nhle_2_mac_msg( RESET_REQUEST_LENGTH );
	if( msg == NULL_POINTER )
	{
		return;
	}
	msg->data_length = RESET_REQUEST_LENGTH;

	buffer = msg->data;

	buffer[0] = MAC_MLME_RESET_REQUEST;
	buffer[1] = SetDefaultPIB;

	/* write to the MAC interface*/
	send_nhle_2_mlme( msg );
	event_set(MLME_EVENT);
	//signal_event_to_mac_task();
} // MLME_RESET_Request()
#endif	/*(CGF_MLME_RESET_REQ_CONF == 1)*/

/******************************************************************************/
#if(CFG_MLME_POLL_REQ_CONF == 1)
void MLME_POLL_Request
(
	mac_address_t *CoordAddress,	      
	security_params_t* pSecstuff  				
)

{
	//uchar *mac_wrp = NULL_POINTER;			/* pointer to the packet to write to the */
	uchar *p = NULL_POINTER;
	msg_t *msg = NULL;
	int allocation_length;	/* length of memory to allocate */
	//int i = 0;

	allocation_length = 5 + sizeof(security_params_t) + find_length(CoordAddress->address_mode);
							 // + sizeof( uchar )	            /* primitive */
							 // + sizeof( uchar )	    /* AddressMode */
							 // + sizeof( ushort )	/* PANID */
							 // + sizeof( uchar )  + sizeof(security_params_t);	/* security enable*/

	//allocation_length += find_length(CoordAddress->address_mode);
/*
	switch( CoordAddress->address_mode )
		{
		//values for address modes, as passed in primitives
		// lengths (including length of PANID field 
		case ADDR_MODE_NONE:
			//allocation_length += ADDR_MODE_NONE_LENGTH;
		case ADDR_MODE_RESVD:
			//allocation_length += ADDR_MODE_RESVD_LENGTH;
			break;

		case ADDR_MODE_SHORT:
			allocation_length += ADDR_MODE_SHORT_LENGTH;
			break;

		case ADDR_MODE_EXTENDED:
			allocation_length += ADDR_MODE_EXTENDED_LENGTH;
			break;
		}
*/
	msg= allocate_nhle_2_mac_msg( allocation_length );

	if( msg == NULL_POINTER )
	{
		return;
	}
	
	p = msg->data;
	msg->data_length = allocation_length;
	
	//p = mac_wrp;

	*p++ = MAC_MLME_POLL_REQUEST;
	

	mac_interface_address_parser(&p, CoordAddress);

	/*switch( CoordAddress->address_mode )
	{
	case ADDR_MODE_NONE:
	case ADDR_MODE_RESVD:
		break;

	case ADDR_MODE_SHORT:
		*p++ = CoordAddress->address.short_address % 256;
		*p++ = CoordAddress->address.short_address / 256;
		break;

	case ADDR_MODE_EXTENDED:
		memcpy(p, CoordAddress->address.ieee_address,IEEE_ADDRESS_LENGTH);
		p += IEEE_ADDRESS_LENGTH;
		break;
	}*/
		
#ifdef MAC_CFG_SECURITY_ENABLED		
	append_security_parameters(&p,pSecstuff);
#endif

	/* write to the MAC interface*/
	send_nhle_2_mlme( msg );

	event_set(MLME_EVENT);
	//signal_event_to_mac_task();

} // MLME_POLL_Request()
#endif	/*(CFG_MLME_POLL_REQ_CONF == 1)*/

/******************************************************************************/
#if(CFG_MLME_ORPHAN_IND_RESP == 1)
void MLME_ORPHAN_Response
(
	 uint8_t*	OrphanAddress,			
	 ushort 	ShortAddress,			
	 uint8_t	AssociatedMember,	
	 security_params_t* pSecstuff  				
)
{
	#define ORPHAN_RESPONSE_LENGTH 15

	msg_t *msg = NULL;
	uchar *p = NULL_POINTER;
	//int i;

	/* allocate a message buffer */
	msg = allocate_nhle_2_mac_msg( ORPHAN_RESPONSE_LENGTH + 20 + sizeof(security_params_t) );
	
	if( msg == NULL_POINTER )
	{
		return;
	}
	
	msg->data_length = ORPHAN_RESPONSE_LENGTH;

	p = msg->data;

	*p++ = MAC_MLME_ORPHAN_RESPONSE;

	memcpy( p,OrphanAddress,IEEE_ADDRESS_LENGTH );
	
	p+= IEEE_ADDRESS_LENGTH;
	
	*p++ = ShortAddress % 256;
	*p++ = ShortAddress / 256;

	*p++ = AssociatedMember;
	
#ifdef MAC_CFG_SECURITY_ENABLED		
	append_security_parameters(&p,pSecstuff);
#endif	

	send_nhle_2_mlme( msg );
	event_set(MLME_EVENT);
	//signal_event_to_mac_task();
}
#endif	/*(CFG_MLME_ORPHAN_IND_RESP == 1)*/

/******************************************************************************/
#if(CFG_MLME_DISASSOCIATE_REQ_CONF == 1)
void MLME_DISASSOCIATE_Request
(
	uint8_t  DeviceAddressMode,
	ushort    DevPANID,
	uint8_t *DeviceAddress,			
	uint8_t  DisassociateReason,
	bool        TxDirect,	
	security_params_t* pSecstuff 
)
{
	#define DISSOCIATE_REQUEST_LENGTH 15

	msg_t *msg = NULL;
	uchar *p = NULL_POINTER;
	//int i;

	/* allocate a message buffer */
	msg = allocate_nhle_2_mac_msg( DISSOCIATE_REQUEST_LENGTH + 20  + sizeof(security_params_t) );
	
	if( msg == NULL_POINTER )
	{
		return;
	}

	msg->data_length = DISSOCIATE_REQUEST_LENGTH;

	p = msg->data;

	*p++ = MAC_MLME_DISASSOCIATE_REQUEST;

	*p++ = DeviceAddressMode;

	*p++ = DevPANID % 256;
	*p++ = DevPANID / 256;
	
	if ( DeviceAddressMode == 0x02 )
	{
		memcpy( p,DeviceAddress,SHORT_ADDRESS_LENGTH );
		p+= SHORT_ADDRESS_LENGTH;
	}
	else
	{
		memcpy( p,DeviceAddress,IEEE_ADDRESS_LENGTH );
		p+= IEEE_ADDRESS_LENGTH;
	}

	*p++ = DisassociateReason;

	*p++ = TxDirect;
	
#ifdef MAC_CFG_SECURITY_ENABLED		
	append_security_parameters(&p,pSecstuff);
#endif	

	send_nhle_2_mlme( msg );
	event_set(MLME_EVENT);
	//signal_event_to_mac_task();
}
#endif	/*(CFG_MLME_DISASSOCIATE_REQ_CONF == 1)*/

/******************************************************************************/
#if(CGF_MLME_BEACON_REQ_CONF == 1)
void MLME_BEACON_Request
(
	uint8_t bcnType,
	uint8_t channel, 
	uint8_t channelPage, 
	uint8_t superFrameOrder,
	mac_address_t	*DstAddr,
	bool BSNSuppression,
	uint8_t  headerIElistLen,
	uint8_t* headerIElist,
	uint8_t payloadIElistLen,
	uint8_t* payloadIElist,
	security_params_t* pSecstuff 
)
{
	//#define BEACON_REQUEST_LENGTH 16 // Raka [28 - Sept -2016] Commented for copilation warning

	msg_t *msg = NULL;
	uchar *p = NULL_POINTER;
	//int i;
	//uint8_t short_subIdsCnt = 0;
	//uint8_t long_subIdsCnt = 0;

	/* allocate a message buffer */
	msg = allocate_nhle_2_mac_msg( BEACON_REQUEST_LENGTH + 20  + sizeof(security_params_t) );
	
	if( msg == NULL_POINTER )
	{
		return;
	}

	msg->data_length = BEACON_REQUEST_LENGTH;

	p = msg->data;

	*p++ = MAC_MLME_BEACON_REQUEST;

	*p++ = bcnType;
	*p++ = channel;
	*p++ = channelPage;
	*p++ = superFrameOrder;

	
	

	mac_interface_address_parser(&p, DstAddr);
	/*switch( DstAddr->address_mode )
	{
		case ADDR_MODE_NONE:
			break;

		case ADDR_MODE_RESVD:
			break;

		case ADDR_MODE_SHORT:			
			*p++ = DstAddr->address.short_address % 256;
			*p++ = DstAddr->address.short_address / 256;
			break;

		case ADDR_MODE_EXTENDED:
			for( i = 0; i < IEEE_ADDRESS_LENGTH; i++ )
			{
				*p++ = DstAddr->address.ieee_address[i];
			}
			break;
	}*/

	*p++ = BSNSuppression;
	
	*p++ = headerIElistLen;
	memcpy(p,headerIElist,headerIElistLen);
	
	p += headerIElistLen;
	
	*p++ = payloadIElistLen;

	parse_IEs(&p, payloadIElist, payloadIElistLen);
	
	/*while(payloadIElistLen)
	{
		if( *payloadIElist != 0x09)//MLME IE ID
		{
			*p++ = *payloadIElist++;			
		}
		else
		{
			*p++ = *payloadIElist++;
			//p_attrib_val is pointing to sub IDs count field
			short_subIdsCnt = *payloadIElist++;
			long_subIdsCnt = *payloadIElist++;

			*p++ = short_subIdsCnt;
			*p++ = long_subIdsCnt;

			memcpy( p, payloadIElist, short_subIdsCnt );
			payloadIElist += short_subIdsCnt;
			
			p += short_subIdsCnt;
			
			memcpy( p, payloadIElist, long_subIdsCnt );
			
			payloadIElist += long_subIdsCnt;
			
			p += long_subIdsCnt;			
		}
		payloadIElistLen--;
	}*/
		
#ifdef MAC_CFG_SECURITY_ENABLED		
	append_security_parameters(&p,pSecstuff);
#endif	

	send_nhle_2_mlme( msg );
	event_set(MLME_EVENT);
	//signal_event_to_mac_task();
}
#endif	/*(CGF_MLME_BEACON_REQ_CONF == 1)*/

/******************************************************************************/

msg_t* allocate_mac_2_nhle_msg( uint16_t length )
{
    msg_t *msg = NULL;
    msg = (msg_t *) app_bm_alloc( length + 6 );// 5 for including next and data_length members

    if (msg != NULL)
    {
        msg->data_length = length;
    }
    return msg;
}

/******************************************************************************/

void free_mac_2_nhle_msg( msg_t * msgp )
{
    app_bm_free((uint8_t*)msgp);
}

/******************************************************************************/

void send_mcps_2_nhle( msg_t * msgp )
{
    //set_process_activity(AF_NWK_MSG_PENDING);
    queue_item_put(&QUEUE_MCPS_2_NHLE, (queue_item_t *) msgp);
    //event_set(MAC_2_NHLE_EVENT);
    
  
#if APP_LBR_ROUTER     
    send_mac_2_nhle_conf ();
#endif   

//    indicate_mac_2_nhle(1);
}

/******************************************************************************/

void send_mlme_2_nhle( msg_t * msgp )
{
    //set_process_activity(AF_NWK_MSG_PENDING);
    queue_item_put(&QUEUE_MLME_2_NHLE, (queue_item_t *) msgp);
    //event_set(MAC_2_NHLE_EVENT);

#if APP_LBR_ROUTER     
    send_mac_2_nhle_conf ();
#endif 

//    indicate_mac_2_nhle(2);
}

/******************************************************************************/
#if(CFG_MLME_ASSOCIATE_IND_RESP == 1)
static void process_mlme_associate_indication( uint8_t* data )
{
	security_params_t* p_sec_stuff = NULL;
#ifdef MAC_CFG_SECURITY_ENABLED		
	security_params_t sec_params = {0};
	
	uint8_t* p_sec= data+9;
	
	p_sec_stuff = &sec_params;
	
	load_security_param_struct( &sec_params,&p_sec );
#endif	
	
	App_MLME_Associate_Ind_cb
	(
		data,					// pointer to 64 bit child address
		data[8],				// CapabilityInformation
		NULL,					//uint8_t* pLowLatencyNwkInfo,
		0,						// ignore channelOffset,
		0,  					// ignore HoppingSequenceID,
		p_sec_stuff
	);	
}
#endif	/*(CFG_MLME_ASSOCIATE_IND_RESP == 1)*/

/******************************************************************************/
#if(CFG_MLME_START_REQ_CONF == 1)
static void process_mlme_start_confirm( uint8_t* data )
{
	App_MLME_Start_Conf_cb(data[0]);	
}
/*Uemsh : 11-01-2018 this thing goes to modified after sepration of mac and fan mac*/
#ifdef WISUN_FAN_MAC 
static void process_mlme_set_mac_channel_info_confirm ( uint8_t* data )
{
       //App_MLME_Set_Mac_Channel_Info_Conf_cb(data[0]);
} 
static void process_fan_mac_mlme_set_confirm ( uint8_t* data )
{
#if(APP_LBR_ROUTER == 1) 
     App_MLME_Fan_Mac_Set_Conf_cb(data[0],data[1]);
#endif     
}  
#endif /*(end of WISUN_FAN_MAC)*/
#endif	/*(CFG_MLME_START_REQ_CONF == 1)*/

/******************************************************************************/

static void process_mcps_data_confirm( uint8_t* data )
{
	App_MCPS_Data_Conf_cb
	(
		data[0], 
		data[1], 
		data[2],
		(uint32_t)data[3]
	);	
}

/******************************************************************************/

static void process_mcps_data_indication( uint8_t* data )
{
        mac_address_t  Srcaddr = {0};
        mac_address_t  Dstaddr = {0};
        security_params_t* p_sec_params = NULL;
        uint16_t msduLength = 0;
        uint64_t rx_time_stamp = 0;

#ifdef MAC_CFG_SECURITY_ENABLED		
	security_params_t sec_params = {0};	
	uint8_t* p_sec = NULL;
	p_sec_params = &sec_params;
#endif	

	
	
	if( !( mac_pib.PromiscuousMode ) )
	{
		 

		mac_interface_addr_parse_From_Buff(&Srcaddr, &data);
		/*Srcaddr.address_mode = data[0];
		data += 0x01;

		if(Srcaddr.address_mode > 1)
		{
			Srcaddr.pan_id = get_ushort(data);
			data += 0x02; 	
		}
			
		
		switch( Srcaddr.address_mode )
		{
			case ADDR_MODE_NONE:
			case ADDR_MODE_RESVD:
				break;

			case ADDR_MODE_SHORT:				
				Srcaddr.address.short_address =  get_ushort(data);
				data += 0x02; 			
				break;

			case ADDR_MODE_EXTENDED:					
				//memcpy(Srcaddr.address.ieee_address,data,IEEE_ADDRESS_LENGTH );
				Srcaddr.address.ieee_address = data;
				data += IEEE_ADDRESS_LENGTH;
				break;
		}
		*/

		mac_interface_addr_parse_From_Buff(&Dstaddr, &data);
		/*Dstaddr.address_mode = data[0];
		data += 0x01;
		if(Dstaddr.address_mode > 1)
		{
			Dstaddr.pan_id = get_ushort(data);
			data += 0x02; 
		}
		switch( Dstaddr.address_mode )
		{
			case ADDR_MODE_NONE:
			case ADDR_MODE_RESVD:
				break;

			case ADDR_MODE_SHORT:							
				Dstaddr.address.short_address =  get_ushort(data);
				data += 0x02; 			
				break;

			case ADDR_MODE_EXTENDED:
				//memcpy(Dstaddr.address.ieee_address,data,IEEE_ADDRESS_LENGTH );
				Dstaddr.address.ieee_address = data;
				data += IEEE_ADDRESS_LENGTH;
				break;
		}
		*/
		msduLength =  get_ushort(data);
		data += 0x02;
	
		memcpy((uint8_t*)&rx_time_stamp,( data + msduLength + 2 ),3);
		
	#ifdef MAC_CFG_SECURITY_ENABLED		
		p_sec = data + msduLength + 5 + 1/*Sagar: 1 is added as one byt of pld_ie_present flag is added to this buffer*/;
	
		load_security_param_struct( &sec_params,&p_sec );
	#endif

		App_MCPS_Data_Ind_cb
		(
			&Srcaddr,
			&Dstaddr,
			msduLength,
			data,
			*( data + msduLength ),
			*( data + msduLength + 1 ),//DSN
                        *( data + msduLength + 2 ),// pld_ie_present
			rx_time_stamp,
			p_sec_params 
		);
		
	}
	else
	{
		uint32_t lqi = 0;
		uint8_t dsn = 0;
		uint8_t time_stamp[8] = {0,0,0};
		
		/* Promiscuous Mode */	
		Srcaddr.address_mode = ADDR_MODE_NONE;
		Dstaddr.address_mode = ADDR_MODE_NONE;
		
		msduLength = get_ushort(data+4);

		memcpy((uint8_t*)&time_stamp[0],( data + msduLength + 17 ),8);
		memcpy((uint8_t*)&lqi,( data + 8 ),4);
		dsn = *( data + msduLength + 16);
		memcpy(&rx_time_stamp,&time_stamp[0],8);
		//data += 16;	
		
		App_MCPS_Data_Ind_cb
		(
			&Srcaddr,
			&Dstaddr,
			msduLength,
			data,
			lqi,
			dsn,
                        *( data + msduLength + 17),// pld_ie_present                        
			rx_time_stamp,
			p_sec_params 
		);
	}		
	
}

/******************************************************************************/
#if(CFG_MLME_GET_REQ_CONF == 1)	
static void process_mlme_get_confirm( uint8_t* data )
{
	App_MLME_GET_Conf_cb
	(
		data[0],	//uint8_t status,
		data[1],	//uint8_t PIBAttribute,
		data[2],	//uint8_t PIBAttributeIndex,
		get_ushort(data+3),  //uint16_t PIBAttributeLength
		(data + 5)	//PIBAttributeValue
	);	
}
#endif	/*(CFG_MLME_GET_REQ_CONF == 1)	*/

/******************************************************************************/

#if(CGF_MLME_RESET_REQ_CONF == 1)
static void process_mlme_reset_confirm( uint8_t* data )
{
    free_data_msgs(&QUEUE_MCPS_2_NHLE);
    free_data_msgs(&QUEUE_MLME_2_NHLE);

    App_MLME_Reset_Conf_cb
    (
    data[0]		//uint8_t status
    );
}
#endif	/*(CGF_MLME_RESET_REQ_CONF == 1)*/

/******************************************************************************/

static void process_mlme_set_confirm( uint8_t* data )
{	
	App_MLME_SET_Conf_cb
	(
	 data[0],		//uint8_t status,
	 data[1],		//uint8_t PIBAttribute,
	 data[2]		//uint8_t PIBAttributeIndex
	);	
}

/******************************************************************************/
#if(CFG_MLME_SCAN_REQ_CONF == 1 )
static void process_mlme_scan_confirm( uint8_t* data )
{
	uint32_t i = 0;
	
	app_pandesc_t pan_desc_list[5] = {0};
	uint8_t* p_result_list = NULL;
	uint8_t* bp = NULL;

	switch( data[1] )
	{
		case ENERGY_DETECT_SCAN:
			p_result_list = data + 12;
			break;
			
		case ACTIVE_SCAN:		
		case PASSIVE_SCAN:
		case EB_ACTIVE_SCAN:
		case MPM_EB_PASSIVE_SCAN:
			p_result_list = (uint8_t*)pan_desc_list;
			if( data[11] > 5 )
			{
				data[11] = 0x05;
			}
			
			bp = data + 12;
	
			for ( i=0;i<data[11];i++ )
			{
				pan_desc_list[i].address.address_mode = *bp++;
				pan_desc_list[i].address.pan_id = get_ushort(bp);
				bp += 0x02;
		
				if( pan_desc_list[i].address.address_mode == 0x02)
				{
					pan_desc_list[i].address.address.short_address = get_ushort(bp);
					bp += 0x02;
				}
				else
				{
					memcpy((uint8_t*)&(pan_desc_list[i].address.address.ieee_address),bp,8);
					bp += 0x08;
				}
		
				pan_desc_list[i].channel = *bp++;
		
				pan_desc_list[i].page = *bp++;
		
				pan_desc_list[i].SuperframeSpec = get_ushort(bp);
				bp += 0x02;
		
				pan_desc_list[i].gts_permit = *bp++;
		
				pan_desc_list[i].link_quality = *bp++;
		
				pan_desc_list[i].timestamp = 0x0;
		
				memcpy((uint8_t*)&(pan_desc_list[i].timestamp),bp,3);
				bp += 0x03;
		
				pan_desc_list[i].sec_status = *bp++;
				
#ifdef MAC_CFG_SECURITY_ENABLED			
				load_security_param_struct( &(pan_desc_list[i].secparms),&bp );
#else
				pan_desc_list[i].secparms.security_level = 0x0;
				pan_desc_list[i].secparms.key_id_mode = 0x00;
				bp++;
#endif
			}		
			break;
		
		case ORPHAN_SCAN:
			break;
	}

	//memcpy((uint8_t*)&UnscannedChannels[0],&data[3],8);
	
	App_MLME_Scan_Conf_cb
	(
		data[0], 		//uint8_t status,
		data[1], 		//ScanType,
		data[2], 		//ChannelPage,
		&data[3],		//unscanned channels
		data[11],		//uint8_t ResultListSize,
	    p_result_list	//void *ResultList	
	);	
}
#endif	/*(CFG_MLME_SCAN_REQ_CONF == 1 )*/

/******************************************************************************/
#if(CGF_MLME_COMM_STATUS_IND == 1)
static void process_mlme_comm_status_indication( uint8_t* data )
{
	mac_address_t Srcaddr = {0};
	mac_address_t Dstaddr = {0};
	security_params_t* p_sec_params = NULL;

#ifdef MAC_CFG_SECURITY_ENABLED		
	security_params_t sec_params = {0};
	uint8_t* p_sec = NULL;
	p_sec_params = &sec_params;	
	
#endif	
	
	Dstaddr.pan_id = Srcaddr.pan_id = get_ushort(data);
	
	data += 0x02; 
	
	Srcaddr.address_mode = data[0];
	data += 0x01;
	
	switch( Srcaddr.address_mode )
	{
		case ADDR_MODE_NONE:
		case ADDR_MODE_RESVD:
			break;

		case ADDR_MODE_SHORT:			
			Srcaddr.address.short_address =  get_ushort(data);
			data += 0x02; 			
			break;

		case ADDR_MODE_EXTENDED:			
			//memcpy(Srcaddr.address.ieee_address,data,IEEE_ADDRESS_LENGTH );
			Srcaddr.address.ieee_address = data;
			data += IEEE_ADDRESS_LENGTH;
			break;
	}

	//mac_interface_addr_parse_From_Buff(&Dstaddr, data);
	Dstaddr.address_mode = data[0];
	data += 0x01;

	//Dstaddr.pan_id = Srcaddr.pan_id;
	switch( Dstaddr.address_mode )
	{
		case ADDR_MODE_NONE:
		case ADDR_MODE_RESVD:
			break;

		case ADDR_MODE_SHORT:					
			Dstaddr.address.short_address =  get_ushort(data);
			data += 0x02; 			
			break;

		case ADDR_MODE_EXTENDED:			
			Dstaddr.address.ieee_address = data;
			data += IEEE_ADDRESS_LENGTH;
			break;
	}

#ifdef MAC_CFG_SECURITY_ENABLED		
	p_sec = data + 1;	
	load_security_param_struct( &sec_params,&p_sec );
#endif	
		
	App_MLME_Comm_Status_Ind_cb
	(
		&Srcaddr,				//src address info
		&Dstaddr,				//destination address info
		*data,					//uint8_t status,
		p_sec_params					//security_params_t *sec_param
	);	
}
#endif

/******************************************************************************/
#if(CGF_MLME_BEACON_NOTIFY_IND == 1)
static void process_mlme_beacon_notify_indication( uint8_t* data )
{
	uint16_t count = 0;
	uint8_t* p_rx_pandesc = &data[1];
	ushort sdu_len = 0;

	security_params_t sec_params = {0};
	
	coex_spec_ie_t coex_spec = {0};

	pandesc_t pan_desc = {0};
	pan_desc.secparms = &sec_params; 

	pan_desc.address.address_mode = *p_rx_pandesc++;
	pan_desc.address.pan_id = get_ushort(p_rx_pandesc);

	p_rx_pandesc += SHORT_ADDRESS_LENGTH;

	if( pan_desc.address.address_mode == ADDR_MODE_SHORT )
	{
		pan_desc.address.address.short_address = get_ushort(p_rx_pandesc);
	
		p_rx_pandesc += PANID_LENGTH;
	}
	else if ( pan_desc.address.address_mode == ADDR_MODE_EXTENDED )
	{
		pan_desc.address.address.ieee_address = p_rx_pandesc;
		p_rx_pandesc += IEEE_ADDRESS_LENGTH;
	}

	pan_desc.channel = *p_rx_pandesc++;
	pan_desc.page = *p_rx_pandesc++;
	pan_desc.SuperframeSpec = get_ushort(p_rx_pandesc);
	p_rx_pandesc += 0x02;
	pan_desc.gts_permit = *p_rx_pandesc++;
	pan_desc.link_quality = *p_rx_pandesc++;

	pan_desc.timestamp = 0x0;
	
	memcpy((uint8_t*)&pan_desc.timestamp,p_rx_pandesc,3);
	
	p_rx_pandesc += 0x03;
		
	pan_desc.sec_status = *p_rx_pandesc++;

#ifdef MAC_CFG_SECURITY_ENABLED			
	load_security_param_struct( (pan_desc.secparms),&p_rx_pandesc );
#else
	pan_desc.secparms->security_level = 0x0;
	pan_desc.secparms->key_id_mode = 0x00;
	p_rx_pandesc++;
#endif	
	
	/* p_rx_pandesc now pointing at pending address spec */
    count = 1 + ((*p_rx_pandesc & 0x07) * 2)
        + (((*p_rx_pandesc & 0x70)/16) * 8);
        
    sdu_len = get_ushort(p_rx_pandesc + count);
    
    memcpy((uint8_t*)&coex_spec, (p_rx_pandesc + count + sdu_len + 4),sizeof(coex_spec_ie_t));
	
	App_MLME_Beacon_Notify_Ind_cb
		(
			data[0],				 //BSN
			&pan_desc,				 //PANDescriptor
			*p_rx_pandesc,		   //PendingAddrSpec
			((*p_rx_pandesc)?( p_rx_pandesc + 1 ):NULL),//AddrList
			sdu_len,
			p_rx_pandesc + count + 2,//sdu
			*(p_rx_pandesc + count + 2 + sdu_len), //EBSN
			*(p_rx_pandesc + count + sdu_len + 3), //BeaconType
			&coex_spec // coexspec
		);
}
#endif	/*(CGF_MLME_BEACON_NOTIFY_IND == 1)*/

/******************************************************************************/
#if(CFG_MLME_ASSOCIATE_REQ_CONF == 1)

static void process_mlme_associate_confirm( uint8_t* data )
{
	security_params_t sec_params = {0};
	uint16_t short_address = data[0] + (((uint16_t)(data[1])) << 8);

#ifdef MAC_CFG_SECURITY_ENABLED	
	uint8_t* p_sec= data+3;	
	load_security_param_struct( &sec_params,&p_sec );
#endif	

	App_MLME_Associate_Conf_cb
	(
		short_address,						//uint16_t short_address,
		data[2],							//uint8_t status,
		NULL,								//uint8_t* pLowLatencyNwkInfo,
		0,									//uint16 channelOffset,
		0,									//uint8_t  HoppingSequenceLength,
		0,									//uint8_t HoppingSequence,
		&sec_params
	);
	
}
#endif	/*(CFG_MLME_ASSOCIATE_REQ_CONF == 1)*/

/******************************************************************************/
#if(CFG_MLME_POLL_REQ_CONF == 1)
static void process_mlme_poll_confirm( uint8_t* data )
{	
	App_MLME_Poll_Conf_cb
	(
		data[0] 			//status
	);	
}
#endif	/*(CFG_MLME_POLL_REQ_CONF == 1)*/

/******************************************************************************/
#if(CFG_MLME_DISASSOCIATE_IND == 1)
static void process_mlme_disassociate_indication( uint8_t* data )
{
	security_params_t* p_sec_params = NULL;

#ifdef MAC_CFG_SECURITY_ENABLED		
	security_params_t sec_params = {0};
	uint8_t* p_sec = data+9 ;
	p_sec_params = &sec_params;	
		
	load_security_param_struct( &sec_params,&p_sec );
#endif	
		
	App_MLME_Dissociate_Ind_cb
	(
		data, // pointer to 64 bit address of the disassociate device 
		data[8],// disassociate reason
		p_sec_params
	);	
}
#endif	/*(CFG_MLME_DISASSOCIATE_IND == 1)*/

/******************************************************************************/
#if(CFG_MLME_DISASSOCIATE_REQ_CONF == 1)
static void process_mlme_disassociate_confirm( uint8_t* data )
{
	uint8_t status = 0;
	mac_address_t Deviceaddr = {0};	
	status = *data++;
	mac_interface_addr_parse_From_Buff(&Deviceaddr, &data);
	/*Deviceaddr.address_mode = *data++;
	if(Deviceaddr.address_mode > 1)
	{
		Deviceaddr.pan_id = get_ushort(data);
		data += 0x02; 
	}
				
	switch( Deviceaddr.address_mode )
	{
		case ADDR_MODE_NONE:
		case ADDR_MODE_RESVD:
			break;

		case ADDR_MODE_SHORT:
						
			Deviceaddr.address.short_address =  get_ushort(data);
			data += 0x02; 			
			break;

		case ADDR_MODE_EXTENDED:			
			//memcpy(Deviveaddr.address.ieee_address,data,IEEE_ADDRESS_LENGTH );
			Deviceaddr.address.ieee_address = data;
			data += IEEE_ADDRESS_LENGTH;
			break;
	}
	*/
	App_MLME_Dissociate_Conf_cb
	(
		status,//status
		&Deviceaddr // device address
	);	
}
#endif	/*(CFG_MLME_DISASSOCIATE_REQ_CONF == 1)*/

/******************************************************************************/
#if(CFG_MLME_ORPHAN_IND_RESP == 1)
static void proces_mlme_orphan_indication( uint8_t* data )
{
	security_params_t* p_sec_params = NULL;

#ifdef MAC_CFG_SECURITY_ENABLED		
	security_params_t sec_params = {0};	
	uint8_t* p_sec = data+8 ;
	p_sec_params = &sec_params;	
		
	load_security_param_struct( &sec_params,&p_sec );
#endif	
	
	App_MLME_Orphan_Ind_cb
	(
		data, // 64bit address of orphan device
		p_sec_params 
	);	
}
#endif	/*(CFG_MLME_ORPHAN_IND_RESP == 1)*/

/******************************************************************************/

static void process_mlme_syncloss_indication( uint8_t* data )
{
	security_params_t* p_sec_params = NULL;

#ifdef MAC_CFG_SECURITY_ENABLED		
	security_params_t sec_params = {0};
	uint8_t* p_sec = data+5 ;
	p_sec_params = &sec_params;	
		
	load_security_param_struct( &sec_params,&p_sec );
#endif	
	
	App_MLME_Sync_Loss_Ind_cb
	(
		data[0], 				//loss reason
		get_ushort(data+1),		//PANID
		data[3], 				// LogicalChannel
		data[4], 				// ChannelPage
		p_sec_params
	);	
}

/******************************************************************************/
#if(CFG_MCPS_PURGE_REQ_CONF == 1)
static void process_mcps_purge_confirm( uint8_t* data )
{
	App_MCPS_Purge_Conf_cb
	(
		data[0], 
		data[1]
	);
}
#endif	/*(CFG_MCPS_PURGE_REQ_CONF == 1)*/

#if(CGF_MLME_BEACON_REQUEST_IND == 1)
static void process_mlme_beacon_request_indication( uint8_t* data )
{
        mac_address_t  Srcaddr = {0};

        uchar bcn_type = *data++;

        Srcaddr.address_mode = *data++;
	
	//data += 0x01; 

	switch( Srcaddr.address_mode )
	{
		case ADDR_MODE_NONE:/*fall through*/
		case ADDR_MODE_RESVD:
			break;

		case ADDR_MODE_SHORT:
			
			Srcaddr.address.short_address =  get_ushort(data);
			data += 0x02;
			break;

		case ADDR_MODE_EXTENDED:
			//memcpy(Srcaddr.address.ieee_address,data,IEEE_ADDRESS_LENGTH );
			Srcaddr.address.ieee_address = data;
			data += IEEE_ADDRESS_LENGTH;
			break;
	}
	
	App_MLME_Beacon_Request_Ind_cb
	(
		bcn_type,//beacon type
		&Srcaddr, // Src address details
		get_ushort(data),// Dest Pan ID
		get_ushort(data+2),//len
		( data + 4 ) // IE List
	);
}
#endif	/*(CGF_MLME_BEACON_REQUEST_IND == 1)*/

/******************************************************************************/
#if(CGF_MLME_BEACON_REQ_CONF == 1)
static void process_mlme_beacon_confirm( uint8_t* data )
{
	App_MLME_Beacon_Conf_cb(data[0]);	
}
#endif	/*(CGF_MLME_BEACON_REQ_CONF == 1)*/

/******************************************************************************/

static void mil_churn( void )
{
    uint8_t active = 0;

#if(CFG_MLME_RX_ENABLE_REQ_CONF == 1)
    active |= mac_process_receiver_enable_state();
#endif	/*(CFG_MLME_RX_ENABLE_REQ_CONF == 1)    */

    active |= check_pending_data();

    
    /* check for expired pending messages in a coordinator */
    active |= mac_check_expired_pending_messages();
    
#ifdef MAC_CFG_GTS_ENABLED
    /* check for expired GTS */
    active |= mac_check_expired_gts();
#endif

    /* PAN ID conflict? */
    active |= mac_check_panid_conflict_state();	
}

#ifdef MAC_CFG_SECURITY_ENABLED
//static
 uint8_t append_security_parameters(uint8_t** p_to, security_params_t* pSecstuff )
{
        uint8_t key_id_mode = 0;
        if(pSecstuff== NULL)
        {
          *(*p_to)++ = 0;
            return 1;
        }
        
	key_id_mode = pSecstuff->key_id_mode;
	**p_to = pSecstuff->security_level;
	*(*p_to)++ |= (key_id_mode<<0x04);
	
	switch( key_id_mode )
	{
		case 0:
			//*(*p_to)++ = pSecstuff->key_identifier[8];
			break;
		case 1:
			//memcpy(*p_to,pSecstuff->key_identifier,5);
			//*p_to += 5;
			*(*p_to)++ = pSecstuff->key_identifier[8];
			break;
		case 2:
			memcpy(*p_to,pSecstuff->key_identifier,4);
			*p_to += 4;
			
			*(*p_to)++ = pSecstuff->key_identifier[8];
			break;
		case 3:
			memcpy(*p_to,pSecstuff->key_identifier,9);
			*p_to += 9;
			break;
		default:
			break;
	}	
	return 1;
}

/******************************************************************************/

//static uint8_t load_security_param_struct(security_params_t* pToSecstuff, uint8_t** p_from )
uint8_t load_security_param_struct(security_params_t* pToSecstuff, uint8_t** p_from )
{
	memset((uint8_t*)pToSecstuff,0x0,sizeof(security_params_t));
	
	if( (mac_pib.mac_security_enabled == TRUE) && (pToSecstuff != NULL))
	{
		pToSecstuff->security_level = **p_from & 0x0f;
		pToSecstuff->key_id_mode = (**p_from & 0xf0) >> 0x04;

		(*p_from)++;

		switch( pToSecstuff->key_id_mode )
		{
			case 0:
				pToSecstuff->key_identifier[8] = *(*p_from)++;
				break;
			case 2:
				memcpy(pToSecstuff->key_identifier, *p_from,4);
				*p_from += 4;
		
				pToSecstuff->key_identifier[8] = **p_from;
				*p_from += 1;
				break;		
			case 1:
			case 3:		
				memcpy(pToSecstuff->key_identifier, *p_from,8);
				*p_from += 8;
		
				pToSecstuff->key_identifier[8] = **p_from;
				*p_from += 1;		
				break;
			default:
				break;
		}
	}
	else
	{
		pToSecstuff->security_level = (*(*p_from) & 0x0f);
		pToSecstuff->key_id_mode =  ((*(*p_from) & 0xf0 ) >> 4); 
		
		*(*p_from)++;		
	}
	return 1;
}

/******************************************************************************/

uint16_t get_mac_sec_table_attrib_len
( 
	uint8_t sec_table_attrib_id, 
	uint8_t* p_attrib_val 
)
{
	uint8_t element_count = 0;
	uint16_t len = 0;
	
	if( sec_table_attrib_id == macKeyTable )
	{
		/*store the number of key id look up descs count*/
		element_count = *p_attrib_val; 
	
		len = 1+ (element_count * ( KEY_SOURCE_LENGTH + 1 + 1 ));
	
		/*store the number of device desc count*/
		element_count = p_attrib_val[len];
	
		len += (1+ (element_count * KEY_DEVICE_DESC_LENGTH));
	
		/*store the number of key usage desc count*/
		element_count = p_attrib_val[len];
	
		len += ((1 + (element_count * KEY_USAGE_DESC_LENGTH)) + KEY_LENGTH );
		
	}
	else if ( sec_table_attrib_id == macDeviceTable )
	{
		len = 17;
	}
	else if ( sec_table_attrib_id == macSecurityLevelTable )
	{
		len = 4;
	}
	return len;	
}

/******************************************************************************/

#endif

//static void mac_interface_address_parser(uchar **p, mac_address_t *addr)
void mac_interface_address_parser(uchar **p, mac_address_t *addr)
{
	int i = 0;
	*(*p)++ = addr->address_mode;
	if(addr->address_mode > 1)
	{
		*(*p)++ = addr->pan_id % 256;
		*(*p)++ = addr->pan_id / 256;
	}

	switch( addr->address_mode )
	{
		case ADDR_MODE_NONE:			
		case ADDR_MODE_RESVD:
			break;

		case ADDR_MODE_SHORT:			
			*(*p)++ = addr->address.short_address % 256;
			*(*p)++ = addr->address.short_address / 256;
			break;

		case ADDR_MODE_EXTENDED:			
			for(; i < IEEE_ADDRESS_LENGTH; i++ )
			{
				*(*p)++ = addr->address.ieee_address[i];
			}
			break;
	}
}

//static void mac_interface_addr_parse_From_Buff(mac_address_t *addr, uchar **p)
void mac_interface_addr_parse_From_Buff(mac_address_t *addr, uchar **p)
{
    uchar address_none[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    uchar *p_address_none = address_none;
    addr->address_mode = **p;
    *p += 0x01;

    if(addr->address_mode > 1)
    {
      addr->pan_id = get_ushort(*p);
      *p += 0x02; 	
    }

    switch( addr->address_mode )
    {
      case ADDR_MODE_NONE:
        memcpy(&addr->address.ieee_address, &p_address_none, 8);
        break;
        
      case ADDR_MODE_RESVD:
        break;

      case ADDR_MODE_SHORT:				
        addr->address.short_address =  get_ushort(*p);
        *p += 0x02; 			
        break;

      case ADDR_MODE_EXTENDED:					
        //memcpy(Srcaddr.address.ieee_address,data,IEEE_ADDRESS_LENGTH );
        addr->address.ieee_address = *p;
        *p += IEEE_ADDRESS_LENGTH;
        break;
      
      default:
        break;
    }
}

//static uint8_t find_length(uint8_t mode)
//{
//	if(mode == ADDR_MODE_SHORT)
//	{
//		return ADDR_MODE_SHORT_LENGTH;
//	}
//	else if(mode == ADDR_MODE_EXTENDED)
//	{
//		return ADDR_MODE_EXTENDED_LENGTH;
//	}
//return ADDR_MODE_RESVD_LENGTH;
//}


//static void parse_IEs(uchar **p,uint8_t *IElist,uint8_t listLen)
void parse_IEs(uchar **p,uint8_t *IElist,uint8_t listLen)
{
	uint8_t short_subIdsCnt = 0;
	uint8_t long_subIdsCnt = 0;
	while(listLen)
	{
//		if( *IElist  != PIE_MLME_NESTED/*MLME IE ID*/)
//		{
//			*(*p)++ = *IElist++;			
//		}
/*Uemsh : 11-01-2018 this thing goes to modified after sepration of mac and fan mac*/
#ifdef WISUN_FAN_MAC          
                if(*IElist  != WP_IE/*wisun payload ID*/)
                {
                  *(*p)++ = *IElist++;
                }
#else
                if( *IElist  != PIE_MLME_NESTED/*MLME IE ID*/)
		{
			*(*p)++ = *IElist++;			
		}
#endif                
		else
		{
			*(*p)++ = *IElist++;
			/*p_attrib_val is pointing to sub IDs count field*/
			short_subIdsCnt = *IElist++;
			long_subIdsCnt = *IElist++;

			*(*p)++ = short_subIdsCnt;
			*(*p)++ = long_subIdsCnt;

			memcpy( *p, IElist, short_subIdsCnt );
			IElist += short_subIdsCnt;
			
			*p += short_subIdsCnt;
			
			memcpy( *p, IElist, long_subIdsCnt );
			
			IElist += long_subIdsCnt;
			
			*p += long_subIdsCnt;
			
		}
		listLen--;
	}	
}

static void free_data_msgs( queue_t* p_queue )
{
	msg_t* p_msg = NULL;
	/* clear any tx messages queued for encryption*/
    while( (p_msg = (msg_t*) queue_item_get( p_queue )) != NULL )
    {
        free_mac_2_nhle_msg( p_msg );
        p_msg = NULL;
    }
}

uint8_t* get_pairing_id(void)
{
#ifdef WISUN_ENET_PROFILE
    return(&mac_pib.PairingIeIdContent[0]);
#else
    return NULL;
#endif
}  
/*need to be cleaned after sepration of fan-mac and genaric mac*/
#ifndef WISUN_FAN_MAC
static uint16_t data_tx_triggered_failed = 0;
void MCPS_DATA_Request
	(
	 mac_address_t	*SrcAddr, /* Source Addressing Mode	*/
	 mac_address_t *DstAddr, /* Dst Addressing Mode	*/		
	 uint16_t msduLength,					
	 uint8_t *msdu,
	 uint8_t msduHandle,								          		 
	 uint8_t TxOptions,
	 ushort	 TxChannel,
	 bool PPDUCoding, 
	 uint8_t FCSLength,
	 bool  ModeSwitch,
	 uint8_t NewModeSUNPage,
	 uint8_t  ModeSwitchParameterEntry,
	 uint8_t  frameCtrlOptions,
	 uint8_t  headerIElistLen,
	 uint8_t* headerIElist,
	 uint8_t payloadIElistLen,
	 uint8_t* payloadIElist,   
	 bool sendMultiPurpose,
	 security_params_t* pSecstuff
	)
{
	//uchar *mac_wrp = NULL_POINTER;			/* pointer to the packet to write to the MAC */
	uchar *p;// = NULL_POINTER;
	msg_t *msg = NULL;
        
        //sizeof(security_params_t)  :: 11
	int allocation_length = sizeof(security_params_t) + 52 + msduLength;//+100;/*Added 25 for HAN*/;
        //alloc_length_copy = allocation_length;
	/* allocate some memory */
	if((msg = allocate_nhle_2_mac_msg( allocation_length  ))==NULL)
	{
          data_tx_triggered_failed++;
		return;
	}

	
	p = msg->data;

	*p++ = MAC_MCPS_DATA_REQUEST;

	*p++ = SrcAddr->address_mode;

	mac_interface_address_parser(&p, DstAddr);

	*p++ = msduLength % 256;
	*p++ = msduLength / 256;

	memcpy( p, msdu,msduLength );
	p += msduLength;

	*p++ = msduHandle;
	*p++ = TxOptions;
	*p++ = TxChannel % 256;
	*p++ = TxChannel / 256;
	*p++ = PPDUCoding;
	*p++ = FCSLength;
	*p++ = ModeSwitch;
	*p++ = NewModeSUNPage;
	*p++ = ModeSwitchParameterEntry;
	*p++ = frameCtrlOptions;	
	*p++ = headerIElistLen;
	if( headerIElistLen )
	{		
		memcpy(p,headerIElist,headerIElistLen);
		p += headerIElistLen;
	}
	
	*p++ = payloadIElistLen;	
	if( payloadIElistLen )
	{
		parse_IEs(&p, payloadIElist, payloadIElistLen);
	}
	*p++ = sendMultiPurpose;
	
#ifdef MAC_CFG_SECURITY_ENABLED		
	append_security_parameters(&p,pSecstuff);
#endif	
        //data_tx_triggered++;//santosh: no where we are using
	/* send it to MAC Layer*/
	send_nhle_2_mcps( msg );
	event_set(MCPS_EVENT);
	//signal_event_to_mac_task();
} /* end MCPS_DATA_Request */

#endif




/********************* FAN Related primitives ***************************/

#ifdef WISUN_FAN_MAC
//static void process_ws_async_frame_confirm( uint8_t* data );
void process_ws_async_frame_confirm( uint8_t* data );
//static void process_ws_async_frame_indication( uint8_t *data );
void process_ws_async_frame_indication( uint8_t *data );
#endif

/********************* End of FAN Related primitives ********************/

#ifdef WISUN_FAN_MAC
//static void process_fan_mcps_data_confirm( uint8_t* data );
void process_fan_mcps_data_confirm( uint8_t* data );
//static void process_fan_mcps_data_indication( uint8_t* data );
void process_fan_mcps_data_indication( uint8_t* data );
void process_fan_mac_ack_indication(uint8_t* data );
void process_fan_mac_no_ack_indication(uint8_t* data );
extern void App_FAN_ack_Ind_cb( mac_address_t*  pSrcaddr,
    mac_address_t*  pDstaddr,
    uint8_t DSN,
    uint8_t rsl_value,
    uint8_t security_status
    );
void App_FAN_no_ack_Ind_cb (void);
#endif

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

extern mac_pib_t mac_pib;

/*
** ============================================================================
** External Function Declarations
** ============================================================================
*/

extern void mac_interface_address_parser(uchar **p, mac_address_t *address);
extern void parse_IEs(uchar **p,uint8_t *payloadIElist,uint8_t payloadIElistLen);
extern void mac_interface_addr_parse_From_Buff(mac_address_t *addr, uchar **p);
#ifdef MAC_CFG_SECURITY_ENABLED
extern uint8_t load_security_param_struct(security_params_t* pToSecstuff, uint8_t** p_to );
extern uint8_t append_security_parameters(uint8_t** p_to, security_params_t* pSecstuff );
extern void App_FAN_MCPS_Data_Conf_cb
(
	uint8_t , 
	uint8_t , 
	uint8_t ,
	uint32_t
);
extern void App_FAN_MCPS_Data_Conf_cb
(
	uint8_t , 
	uint8_t , 
	uint8_t ,
	uint32_t 
);
extern void  App_MLME_ws_async_frame_Conf_cb
(
	uint8_t ,
        uint8_t 
);
void App_MLME_WS_ASYNC_FRAME_Ind_cb
(
     uint8_t ,
     uint8_t ,
     mac_address_t * // Src address details
	
);
extern void App_FAN_MCPS_Data_Ind_cb
(
	mac_address_t*  pSrcaddr,
	mac_address_t*  pDstaddr,
	uint16_t msduLength,
	uint8_t* pMsdu,
	uint8_t mpduLinkQuality,
	uint8_t DSN,
        uint8_t pld_ies_present,
	uint32_t Timestamp,
	security_params_t* pSec 
);

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

#ifdef WISUN_FAN_MAC
void FAN_MCPS_DATA_Request (
                            mac_address_t *SrcAddr, /*Source Addressing Mode*/
                            mac_address_t *DstAddr, /*Dst Addressing Mode*/
                            uint16_t msduLength,
                            uint8_t *msdu,
                            uint8_t msduHandle,
                            uint8_t TxOptions,
                            ushort TxChannel,
                            bool PPDUCoding, 
                            uint8_t FCSLength,
                            bool  ModeSwitch,
                            uint8_t NewModeSUNPage,
                            uint8_t  ModeSwitchParameterEntry,
                            uint8_t  frameCtrlOptions,
                            uint32_t sub_hdr_bitmap,   
                            uint32_t sub_pld_bitmap,
                            bool sendMultiPurpose,
                            security_params_t* pSecstuff
                            )
{
  uchar *p = NULL_POINTER;
  msg_t *msg = NULL;
  uint16_t allocation_length = (sizeof(security_params_t) + msduLength + 17 + 20);
  
  /* allocate some memory */
  if((msg = allocate_nhle_2_mac_msg (allocation_length)) == NULL)
  {
#if ((PRINT_DEBUG_LEVEL==ERROR_DEBUG)||(PRINT_DEBUG_LEVEL==CONSOLE_ERROR_DEBUG))
    stack_print_debug ("### allocate_nhle_2_mac_msg FAILED\n");
#endif
    return;
  }
  
  p = msg->data;
  *p++ = FAN_MAC_MCPS_DATA_REQUEST;
  *p++ = SrcAddr->address_mode;
  mac_interface_address_parser(&p, DstAddr);
  
  *p++ = msduLength % 256;
  *p++ = msduLength / 256;
  memcpy (p, msdu, msduLength);
  p += msduLength;
  
  *p++ = msduHandle;
  *p++ = TxOptions;
  *p++ = TxChannel % 256;
  *p++ = TxChannel / 256;
  *p++ = PPDUCoding;
  *p++ = FCSLength;
  *p++ = ModeSwitch;
  *p++ = NewModeSUNPage;
  *p++ = ModeSwitchParameterEntry;
  *p++ = frameCtrlOptions;	
  memcpy (p, (uint8_t *)&sub_hdr_bitmap, sizeof (sub_hdr_bitmap));
  p += sizeof (sub_hdr_bitmap);
  memcpy (p, (uint8_t *)&sub_pld_bitmap, sizeof (sub_pld_bitmap));
  p += sizeof (sub_pld_bitmap);
  *p++ = sendMultiPurpose;
  
#ifdef MAC_CFG_SECURITY_ENABLED		
  append_security_parameters(&p,pSecstuff);
#endif	

  /* send it to MAC Layer*/
  send_nhle_2_mcps( msg );
  event_set(MCPS_EVENT);
}

#else
//
//void MCPS_DATA_Request
//	(
//	 mac_address_t	*SrcAddr, /* Source Addressing Mode	*/
//	 mac_address_t *DstAddr, /* Dst Addressing Mode	*/		
//	 uint16_t msduLength,					
//	 uint8_t *msdu,
//	 uint8_t msduHandle,								          		 
//	 uint8_t TxOptions,
//	 ushort	 TxChannel,
//	 bool PPDUCoding, 
//	 uint8_t FCSLength,
//	 bool  ModeSwitch,
//	 uint8_t NewModeSUNPage,
//	 uint8_t  ModeSwitchParameterEntry,
//	 uint8_t  frameCtrlOptions,
//	 uint8_t  headerIElistLen,
//	 uint8_t* headerIElist,
//	 uint8_t payloadIElistLen,
//	 uint8_t* payloadIElist,   
//	 bool sendMultiPurpose,
//	 security_params_t* pSecstuff
//	)
//{
//	//uchar *mac_wrp = NULL_POINTER;			/* pointer to the packet to write to the MAC */
//	uchar *p;// = NULL_POINTER;
//	msg_t *msg = NULL;
//        
//        //sizeof(security_params_t)  :: 11
//	int allocation_length = sizeof(security_params_t) + 52 + msduLength;//+100;/*Added 25 for HAN*/;
//        //alloc_length_copy = allocation_length;
//	/* allocate some memory */
//	if((msg = allocate_nhle_2_mac_msg( allocation_length  ))==NULL)
//	{
//          data_tx_triggered_failed++;
//		return;
//	}
//
//	
//	p = msg->data;
//
//	*p++ = MAC_MCPS_DATA_REQUEST;
//
//	*p++ = SrcAddr->address_mode;
//
//	mac_interface_address_parser(&p, DstAddr);
//
//	*p++ = msduLength % 256;
//	*p++ = msduLength / 256;
//
//	memcpy( p, msdu,msduLength );
//	p += msduLength;
//
//	*p++ = msduHandle;
//	*p++ = TxOptions;
//	*p++ = TxChannel % 256;
//	*p++ = TxChannel / 256;
//	*p++ = PPDUCoding;
//	*p++ = FCSLength;
//	*p++ = ModeSwitch;
//	*p++ = NewModeSUNPage;
//	*p++ = ModeSwitchParameterEntry;
//	*p++ = frameCtrlOptions;	
//	*p++ = headerIElistLen;
//	if( headerIElistLen )
//	{		
//		memcpy(p,headerIElist,headerIElistLen);
//		p += headerIElistLen;
//	}
//	
//	*p++ = payloadIElistLen;	
//	if( payloadIElistLen )
//	{
//		parse_IEs(&p, payloadIElist, payloadIElistLen);
//	}
//	*p++ = sendMultiPurpose;
//	
//#ifdef MAC_CFG_SECURITY_ENABLED		
//	append_security_parameters(&p,pSecstuff);
//#endif	
//        //data_tx_triggered++;//santosh: no where we are using
//	/* send it to MAC Layer*/
//	send_nhle_2_mcps( msg );
//	event_set(MCPS_EVENT);
//	//signal_event_to_mac_task();
//} /* end MCPS_DATA_Request */
#endif

/******************************************************************************/

#ifdef WISUN_FAN_MAC
void FAN_MAC_MLME_SET_Request
(
  uint8_t ie_identifier,		            /* header ie or payload ie */
  uint8_t ie_subid, 			/* subid for each ie */	        
  uint16_t length,		  	/* length of value */
  void *ie_value	          	/* pointer to first element*/
)
{
    msg_t *msg = NULL;
    uchar *buffer = NULL;
    uint16_t buffer_len = length;

	/* allocate a message buffer */
    msg = allocate_nhle_2_mac_msg( buffer_len + 4 ); /*2 byte for ie id and subid*/

    if( msg == NULL_POINTER )
    {		
      return;
    }

    msg->data_length = buffer_len + 2;

    buffer = msg->data;

    buffer[0] = FAN_MAC_MLME_SET_REQUEST;
    buffer[1] = ie_identifier;
    buffer[2] = ie_subid;

    buffer[3] = buffer_len;
    memcpy( &buffer[4],ie_value,buffer_len );
    /* write to the MAC interface */
    send_nhle_2_mlme( msg );
    event_set(MLME_EVENT);
}  
    
#else
#if(CFG_MLME_START_REQ_CONF == 1)
void MLME_START_Request
	(
	uint16_t PANId,  		
	uint8_t  LogicalChannel,
	uint8_t ChannelPage,
	uint32_t startTime,			
	uint8_t BeaconOrder,		
	uint8_t SuperframeOrder, 
	uint8_t PANCoordinator,	
	bool  BatteryLifeExtension, 
	uint8_t CoordRealignment,		
	uint8_t* pDSMESuperframeSpecification,
	uint8_t* pBeaconBitMap,
	uint8_t* hoppingDescriptor,
	uint8_t headerIEListLen,
	uint8_t* headerIEList,
	uint8_t payloadIEListLen,
	uint8_t* payloadIEList,
#if(0)
    uint8_t enhancedBeaconOrder,
	uint8_t OffsetTimeOrder,
	uint8_t NBPANEnhancedBeaconOrder,
#else
	uint8_t MPM_EB_IEListLen,
	uint8_t* MPM_EB_IEList,
	uint8_t EnhancedBeaconOrder,
	uint8_t  OffsetTimeSlot,
	uint16_t NBPANEnhancedBeaconOrder,

#endif
	security_params_t* pCRSecstuff ,
	security_params_t* pBCNSecstuff 
	)
{
	//uchar *mac_wrp = NULL_POINTER;			/* pointer to the data part of the message buffer */
	uchar *p = NULL_POINTER;
	msg_t *msg = NULL;
	//uint8_t short_subIdsCnt;
	//uint8_t long_subIdsCnt;
	int allocation_length = 50 +  sizeof(security_params_t);	/* length of memory to allocate */
	
	/* allocate a message buffer */
	msg = allocate_nhle_2_mac_msg( allocation_length );
	
	if( msg == NULL_POINTER )
	{
		return;
	}
	
	p = msg->data;

	msg->data_length = allocation_length;

	//p = mac_wrp;

	*p++ = MAC_MLME_START_REQUEST;

	/* PANID MSB first */
	*p++ = PANId / 256;
	/* now PANId LSB */
	*p++ = PANId % 256;
	
	*p++ = LogicalChannel;
	*p++ = ChannelPage;
	
	/*send it in BIG endian fashion*/
	*p++ = ( startTime >> 16 ) & 0xFF;
	*p++ = ( startTime >> 8 ) & 0xFF;
	*p++ = ( startTime >> 0 ) & 0xFF;
	
	/*Beacon order and superframe order each are of 4 bit*/
	*p =  BeaconOrder;

	*p++ |= SuperframeOrder << 4;
	
	*p = 0x00;
	
	/*copy the pan coordinator bit information*/                                            
	*p = ( PANCoordinator )? 0x01 : 0x00;

	/*copy the Battery life extension bit(1) */
	*p |= (BatteryLifeExtension )? 0x02 : 0x00;

	/* copy the Coordinator realignment bit */
	*p++ |= ( CoordRealignment )? 0x04 : 0x00;

	/**p++ = 0;//pDSMESuperframeSpecification is being set to 0 for now
	*p++ = 0;//pBeaconBitMap is being set to 0 for now
	*p++ = 0; //hoppingDescriptor is being set to 0 for now*/

	memset(p,0x0,3);
	p += 3;

	*p++ = headerIEListLen;
	memcpy(p,headerIEList,headerIEListLen);
	
	p += headerIEListLen;
	
	*p++ = payloadIEListLen;

	parse_IEs(&p, payloadIEList, payloadIEListLen);
	
	/*while(payloadIEListLen)
	{
		if( *payloadIEList != 0x09)//MLME IE ID
		{
			*p++ = *payloadIEList++;		
		}
		else
		{
			*p++ = *payloadIEList++;
			//p_attrib_val is pointing to sub IDs count field
			short_subIdsCnt = *payloadIEList++;
			long_subIdsCnt = *payloadIEList++;

			*p++ = short_subIdsCnt;
			*p++ = long_subIdsCnt;

			memcpy( p, payloadIEList, short_subIdsCnt );
			payloadIEList += short_subIdsCnt;
			
			p += short_subIdsCnt;
			
			memcpy( p, payloadIEList, long_subIdsCnt );
			
			payloadIEList += long_subIdsCnt;
			
			p += long_subIdsCnt;		
		}
		payloadIEListLen--;
	}
	*/

#if(0)
	*p++ = enhancedBeaconOrder;
	*p++ = OffsetTimeOrder;
	*p++ = NBPANEnhancedBeaconOrder;
#else

	*p++ = MPM_EB_IEListLen;
	parse_IEs(&p, MPM_EB_IEList, MPM_EB_IEListLen);
	
	/*while(MPM_EB_IEListLen)
	{
		if( *MPM_EB_IEList != 0x09)//MLME IE ID
		{
			*p++ = *MPM_EB_IEList++;	
		}
		else
		{
			*p++ = *MPM_EB_IEList++;
			//p_attrib_val is pointing to sub IDs count field
			short_subIdsCnt = *MPM_EB_IEList++;
			long_subIdsCnt = *MPM_EB_IEList++;

			*p++ = short_subIdsCnt;
			*p++ = long_subIdsCnt;

			memcpy( p, MPM_EB_IEList, short_subIdsCnt );
			MPM_EB_IEList += short_subIdsCnt;
			
			p += short_subIdsCnt;
			
			memcpy( p, MPM_EB_IEList, long_subIdsCnt );
			
			MPM_EB_IEList += long_subIdsCnt;
			
			p += long_subIdsCnt;		
		}
		MPM_EB_IEListLen--;
	}
	*/
	*p++ = EnhancedBeaconOrder;
	*p++ = OffsetTimeSlot;

	*p++ = NBPANEnhancedBeaconOrder % 256;
	*p++ = NBPANEnhancedBeaconOrder / 256;	
#endif

#ifdef MAC_CFG_SECURITY_ENABLED		
	append_security_parameters(&p,pCRSecstuff);	
	append_security_parameters(&p,pBCNSecstuff);
#endif

	/* write to the MAC interface*/
	send_nhle_2_mlme( msg );
	event_set(MLME_EVENT);
	//signal_event_to_mac_task();
}
#endif	/*(CFG_MLME_START_REQ_CONF == 1)*/

#endif

/******************************************************************************/

#ifdef WISUN_FAN_MAC
//static void process_fan_mcps_data_confirm( uint8_t* data )
void process_fan_mcps_data_confirm( uint8_t* data )
{
    App_FAN_MCPS_Data_Conf_cb
    (
    data[0], 
    data[1], 
    data[2],
    (uint32_t)data[3]
    );	
}
/******************************************************************************/
void process_fan_mac_ack_indication(uint8_t* data )
{
  mac_address_t  Srcaddr = {0};
  mac_address_t  Dstaddr = {0};
  uint8_t dsn = 0;
  uint8_t rsl_value = 0;
  uint8_t security_status = 0;
  mac_interface_addr_parse_From_Buff(&Srcaddr, &data);
  mac_interface_addr_parse_From_Buff(&Dstaddr, &data);
  dsn = *data++;
  rsl_value = *data++;
  security_status = *data++;
  App_FAN_ack_Ind_cb
    (
     &Srcaddr,
     &Dstaddr,
     dsn,//DSN
     rsl_value,// rsl value
     security_status
       );
}

void process_fan_mac_no_ack_indication(uint8_t* data )
{
  App_FAN_no_ack_Ind_cb ();
}

//static void process_fan_mcps_data_indication( uint8_t* data )
void process_fan_mcps_data_indication( uint8_t* data )
{
          mac_address_t  Srcaddr = {0};
          mac_address_t  Dstaddr = {0};
          security_params_t*  p_sec_params = NULL;
          uint16_t msduLength = 0;
          uint32_t rx_time_stamp = 0;

#ifdef MAC_CFG_SECURITY_ENABLED		
	security_params_t sec_params = {0};	
	uint8_t* p_sec = NULL;
	p_sec_params = &sec_params;
#endif	

	
	
	if( !( mac_pib.PromiscuousMode ) )
	{
		 

		mac_interface_addr_parse_From_Buff(&Srcaddr, &data);
		/*Srcaddr.address_mode = data[0];
		data += 0x01;

		if(Srcaddr.address_mode > 1)
		{
			Srcaddr.pan_id = get_ushort(data);
			data += 0x02; 	
		}
			
		
		switch( Srcaddr.address_mode )
		{
			case ADDR_MODE_NONE:
			case ADDR_MODE_RESVD:
				break;

			case ADDR_MODE_SHORT:				
				Srcaddr.address.short_address =  get_ushort(data);
				data += 0x02; 			
				break;

			case ADDR_MODE_EXTENDED:					
				//memcpy(Srcaddr.address.ieee_address,data,IEEE_ADDRESS_LENGTH );
				Srcaddr.address.ieee_address = data;
				data += IEEE_ADDRESS_LENGTH;
				break;
		}
		*/

		mac_interface_addr_parse_From_Buff(&Dstaddr, &data);
		/*Dstaddr.address_mode = data[0];
		data += 0x01;
		if(Dstaddr.address_mode > 1)
		{
			Dstaddr.pan_id = get_ushort(data);
			data += 0x02; 
		}
		switch( Dstaddr.address_mode )
		{
			case ADDR_MODE_NONE:
			case ADDR_MODE_RESVD:
				break;

			case ADDR_MODE_SHORT:							
				Dstaddr.address.short_address =  get_ushort(data);
				data += 0x02; 			
				break;

			case ADDR_MODE_EXTENDED:
				//memcpy(Dstaddr.address.ieee_address,data,IEEE_ADDRESS_LENGTH );
				Dstaddr.address.ieee_address = data;
				data += IEEE_ADDRESS_LENGTH;
				break;
		}
		*/
		msduLength =  get_ushort(data);
		data += 0x02;
	
		memcpy((uint8_t*)&rx_time_stamp,( data + msduLength + 2 ),3);
		
	#ifdef MAC_CFG_SECURITY_ENABLED		
		p_sec = data + msduLength + 5 + 1/*Sagar: 1 is added as one byt of pld_ie_present flag is added to this buffer*/;
	
		load_security_param_struct( &sec_params,&p_sec );
	#endif

		App_FAN_MCPS_Data_Ind_cb
		(
			&Srcaddr,
			&Dstaddr,
			msduLength,
			data,
			*( data + msduLength ),
			*( data + msduLength + 1 ),//DSN
                        *( data + msduLength + 2 ),// pld_ie_present
			rx_time_stamp,
			p_sec_params 
		);
		
	}
	else
	{
		uint32_t lqi = 0;
		uint8_t dsn = 0;
		uint8_t time_stamp[3] = {0,0,0};
		
		/* Promiscuous Mode */	
		Srcaddr.address_mode = ADDR_MODE_NONE;
		Dstaddr.address_mode = ADDR_MODE_NONE;
		
		msduLength = get_ushort(data+4);

		memcpy((uint8_t*)&time_stamp[0],( data + msduLength + 17 ),3);
		memcpy((uint8_t*)&lqi,( data + 8 ),4);
		dsn = *( data + msduLength + 16);
		
		//data += 16;	
		
		App_FAN_MCPS_Data_Ind_cb
		(
			&Srcaddr,
			&Dstaddr,
			msduLength,
			data,
			lqi,
			dsn,
                        *( data + msduLength + 17),// pld_ie_present                        
			rx_time_stamp,
			p_sec_params 
		);
	}		
	
}

#endif

/******************************************************************************/

#ifdef WISUN_FAN_MAC 
//static void process_ws_async_frame_confirm( uint8_t* data)
void process_ws_async_frame_confirm( uint8_t* data)
{  
//    App_MLME_ws_async_frame_Conf_cb
//    (
//    data[0],		//uint8_t status
//    data[1]  
//    );
        
/*add send event to the fan_mac_interface*/
//        if(data[0] != 0x00)
//        {
//          send_error_ws_async_pkt();
//        } 
}
/******************************************************************************/
#ifdef WISUN_FAN_MAC
//static void process_ws_async_frame_indication( uint8_t *data)
void process_ws_async_frame_indication( uint8_t *data)
{	
  mac_address_t src_addr;
  
  src_addr.address.ieee_address = &data[2];
  //suneet :: imp when required 
//  App_MLME_WS_ASYNC_FRAME_Ind_cb
//    (
//     data[0],//status   
//     data[1],//frame_type
//     &src_addr//Src address details
//       );
}
#endif
/******************************************************************************/
void WS_ASYNC_FRAME_request(
                             uint8_t operation, 
                             uint8_t frame_type,                              
                             uint8_t* channel_list,
                             uint8_t length
                           )
{
    uchar *p = NULL;
    msg_t *msg = NULL;

    int allocation_length = length;//sizeof(security_params_t) + 52 + msduLength+100;/*Added 25 for HAN*/;
    //alloc_length_copy = allocation_length+3;
    //Suneet :: multiply by two because channel is two byte in  size +3 byte 
    if((msg = allocate_nhle_2_mac_msg( (allocation_length*2)+3 ))==NULL)
    {
      //data_tx_triggered_failed++;
      return;
    }

    p = msg->data;

    *p++ = MLME_WS_ASYNC_FRAME_REQ;
    *p++ = operation;
    *p++ = frame_type;   
    if(channel_list != NULL)
      memcpy(p, channel_list, length*2);   
    send_nhle_2_mlme( msg );
    event_set(MLME_EVENT);
}
/******************************************************************************/

#if(FAN_EAPOL_FEATURE_ENABLED == 1)
void fan_eapol_request(void)  // check the param require and send them and 
                              //according allocate buffer
{
   uchar *p;
   msg_t *msg;
  if((msg = allocate_nhle_2_mac_msg( 3))==NULL)    
  {
    // unable to allocate buffer 
  }
  p = msg->data;
  *p++ = MLME_EAPOL_FRAME_REQ;  
  send_nhle_2_mlme( msg );
  event_set(MLME_EVENT);
}     //Debdeep
#endif


#endif

/******************************************************************************/