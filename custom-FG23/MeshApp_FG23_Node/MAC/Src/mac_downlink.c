/** \file mac_downlink.c
 *******************************************************************************
 ** \brief Processes the packets down to the MAC Layer
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
#include "hw_tmr.h"
#include "sw_timer.h"
#include "phy.h"
#include "mac_config.h"
#include "mac.h"
#include "mac_queue_manager.h"
#include "mac_defs_sec.h"
#include "ie_element_info.h"
#include "mac_defs.h"
#include "mac_pib.h"
#include "fan_mac_ie.h"
#include "mac_frame_build.h"
#include "mac_mem.h"
#include "macutils.h"

#if(CFG_MAC_PENDADDR_ENABLED == 1)
#include "pendaddr.h"
#endif

#if(CFG_MAC_PENDADDR_ENABLED == 1)
#include "pandesc.h"
#endif

#include "sm.h"
#if(CFG_MAC_SFTSM_ENABLED == 1)
#include "sftsm.h"
#endif

#include "ccasm.h"

#if(CFG_MAC_MPMSM_ENABLED == 1)
#include "mpmsm.h"
#endif

#include "trxsm.h"

#if(CFG_MAC_STARTSM_ENABLED ==1)
#include "startsm.h"
#endif

#if(CFG_MAC_SYNCSM_ENABLED == 1)
#include "syncsm.h"
#endif
#if(CFG_MAC_SCANSM_ENABLED == 1)  
#include "scansm.h"
#endif

#if(CFG_MAC_DRSM_ENABLED == 1)
#include "drsm.h"
#endif
#if(CFG_ASSOCSM_ENABLED == 1)
#include "assocsm.h"
#endif

#include "mac_uplink.h"
#include "event_manager.h"
#include "mac_security.h"


#if ( ( MAC_CFG_LE_RIT_CAPABILITY == 1 ) || ( MAC_CFG_LE_CSL_CAPABILITY == 1 ) )
#include "mac_le.h"
#endif

#ifdef UTEST
#include "utest_utils.h"
#endif

#include "mac_interface_layer.h"

#ifdef WISUN_FAN_MAC

#include "fan_mac_interface.h" 
#include "tri_tmr.h"
#include "fan_sm.h"

#if ((FAN_EAPOL_FEATURE_ENABLED == 1) || (APP_GTK_HARD_CODE_KEY == 1))
  #include "sha256.h"
#endif

#endif

/*
********************************************************************************
* File inclusion
********************************************************************************
*/



/*
** =============================================================================
** Private Macro definitions
** =============================================================================
*/
#define MAX_ALLOC_SIZE                                  150	
#define AVG_MAX_PAYLOAD_SIZE                            20
#define PARENT_DEST					0
#define NON_PARENT_DEST					1

#define COORD_WISHES_DEV_TO_LEAVE			1					
#define DEV_WISHES_TO_LEAVE				2
#define DEV_WISHES_TO_LEAVE_SILENT			3

/*
** =============================================================================
** Private Structures, Unions & enums Type Definitions
** =============================================================================
**/

typedef struct nwk_to_mac_process_table_struct
{
    uchar primitive;
    uchar (* action)( uchar *args, uint16_t length );
} nwk_to_mac_process_table_t;

/*
** =============================================================================
** Private Variables Definitions
** =============================================================================
**/

/*None*/

/*
** ============================================================================
** External Variable Declarations
** ============================================================================
*/

extern uint8_t force_ie_list_in_frame_control;
extern mac_pib_t mac_pib;
extern uchar aExtendedAddress[8];
extern uchar heap[];
extern trxsm_t *trxsm_p;

#if ( ( MAC_CFG_LE_RIT_CAPABILITY == 1 ) || ( MAC_CFG_LE_CSL_CAPABILITY == 1 ) )
	extern low_energy_t low_energy;
#endif

/* state machines */
#if(CFG_MAC_SFTSM_ENABLED == 1)        
extern sftsm_t sftsm_in, sftsm_out;
extern sftsm_t *sftsm_in_p;
#endif
extern trxsm_t trxsm;
#if (CFG_MAC_STARTSM_ENABLED == 1)  
extern startsm_t *startsm_p;
#endif
#if(CFG_MLME_SYNC_REQ == 1)
      extern syncsm_t syncsm;
#endif
#if(CFG_MAC_SCANSM_ENABLED == 1)        
extern scansm_t scansm;
#endif
#if(CFG_MAC_DRSM_ENABLED == 1)
extern drsm_t drsm;
#endif

#if(CFG_ASSOCSM_ENABLED ==1)
extern assocsm_t assocsm;
#endif

#if(CFG_MAC_MPMSM_ENABLED == 1)
extern mpmsm_t mpmsm;
#endif
/*
** ============================================================================
** External Function Prototypes
** ============================================================================
*/


extern void app_bm_free(uint8_t *pMem  );
extern void * app_bm_alloc( uint16_t length  );
#if(APP_LBR_ROUTER == 1)
extern uint8_t get_node_type( void );
#endif

extern uint16_t get_current_pan_id( void );

/*Umesh : 16-01-2018*/
extern uchar process_mcps_data_request( uchar *buf, uint16_t length );

#ifdef WISUN_FAN_MAC

extern uchar process_fan_mcps_data_request( uchar *buf, uint16_t length );
extern uchar process_ws_async_frame_request( uchar *buf, uint16_t length );

#if(FAN_EAPOL_FEATURE_ENABLED == 1)
extern uchar process_eapol_frame_request(uchar *buf, uint16_t length);
extern void send_eapol_request(void);
#endif

#if(CFG_MLME_START_REQ_CONF == 1)
  extern uchar process_fan_mac_mlme_set_req(uchar *buf, uint16_t length);
#endif

void set_mac_self_info_from_lbr(uint8_t *rec_ptr,uint16_t rec_len);
uint16_t build_ie (uint8_t *buf, uint8_t type, uint32_t ie_bitmap, 
                   uint32_t sub_ie_bitmap, void *data);
extern void update_seq_number_l2_attr(uint8_t seq_number);

extern uint8_t is_key_request_triggered (void);
//extern uint8_t gtk_hash_need_to_be_updated (void);
uint8_t get_mac_active_key_index (void);
mac_status_t mac_cca_event_do(mac_tx_t *txd);
#endif
/*
** ============================================================================
** Public Function Prototypes
** ============================================================================
*/

#ifdef MAC_CFG_SECURITY_ENABLED
  void cleanup_security_queues(uchar SetDefaultPIBValue);
#endif

/*
** ============================================================================
** External Variable Declarations
** ============================================================================
*/

#ifdef WISUN_FAN_MAC
extern fan_mac_param_t fan_mac_params;
extern fan_pkt_tmr_t pas_timer;  
extern fan_pkt_tmr_t pa_timer;  
extern fan_pkt_tmr_t pcs_timer;  
extern fan_pkt_tmr_t pc_timer; 
extern valid_channel_list_t usable_channel;
#endif

#ifdef WISUN_FAN_MAC  
extern self_info_fan_mac_t mac_self_fan_info;
extern phy_pib_t phy_pib;
#if(FAN_EDFE_FEATURE_ENABLED == 1)
extern edfe_info_t edfe_information;  
#endif
#endif
#if(CFG_MAC_SFTSM_ENABLED == 1)  
extern startsm_t *startsm_p;/*Umesh  : 15-01-2018*/
#endif
extern mac_pib_t mac_pib;
extern uchar aExtendedAddress[8];

/*
** ============================================================================
** External Function Declarations
** ============================================================================
*/
extern mac_status_t mac_prim_parse_address( uchar **, mac_address_t * );
extern mac_status_t mac_prim_parse_security( uchar **, security_params_t *, uchar );
extern mac_status_t mac_frame_build( mac_tx_t**, uchar, uchar, mac_address_t*, mac_address_t*,
                                     uchar, uchar,uchar*, uint16_t, uchar, uint32_t/*IE_ids_list_t**/, phy_data_params_t*,security_params_t*, uint8_t );
extern void mac_mem_free_tx(mac_tx_t *);
extern uchar send_mcps_data_confirm( uchar, uchar , ulong ,uchar);

#ifdef WISUN_FAN_MAC

#if(APP_LBR_ROUTER == 1)
extern uint8_t get_node_type( void );
extern uint16_t get_current_pan_id( void );
#endif
extern void send_mlme_set_fan_mac_confirm( uchar status,uchar subie_val );

#endif


/*
** ============================================================================
** Private Function Prototypes
** ============================================================================
*/
#ifndef MAC_CFG_SECURITY_ENABLED
	static void init_sec_params(security_params_t *sec_params_t);
#endif

#if(CFG_MCPS_PURGE_REQ_CONF == 1)
static uchar process_mcps_purge_request( uchar *buf, uint16_t length );
#endif	/*(CFG_MCPS_PURGE_REQ_CONF == 1)*/

#if(CFG_MLME_ASSOCIATE_REQ_CONF == 1)
static uchar process_mlme_associate_request( uchar *buf, uint16_t length );
#endif	/*(CFG_MLME_ASSOCIATE_REQ_CONF == 1)*/

#if(CFG_MLME_ASSOCIATE_IND_RESP == 1)
static uchar process_mlme_associate_response( uchar *buf, uint16_t length );
#endif	/*(CFG_MLME_ASSOCIATE_IND_RESP == 1)*/

#if(CFG_MLME_DISASSOCIATE_REQ_CONF == 1)
static uchar process_mlme_disassociate_request( uchar *buf, uint16_t length );
#endif	/*(CFG_MLME_DISASSOCIATE_REQ_CONF == 1)*/
       
#ifdef MAC_CFG_GTS_ENABLED
static uchar process_mlme_gts_request( uchar *buf, uint16_t length );
#endif

#if(CFG_MLME_ORPHAN_IND_RESP == 1)
static uchar process_mlme_orphan_response( uchar *buf, uint16_t length );
#endif	/*(CFG_MLME_ORPHAN_IND_RESP == 1)*/

#if(CGF_MLME_RESET_REQ_CONF == 1)
static uchar process_mlme_reset_request( uchar *buf, uint16_t length );
#endif	/*(CGF_MLME_RESET_REQ_CONF == 1)*/

#if(CFG_MLME_RX_ENABLE_REQ_CONF == 1)
static uchar process_mlme_rx_enable_request( uchar *buf, uint16_t length );
#endif	/*(CFG_MLME_RX_ENABLE_REQ_CONF == 1)*/

#if(CFG_MLME_SCAN_REQ_CONF==1)
static uchar process_mlme_scan_request( uchar *buf, uint16_t length );
#endif

#if (CFG_MLME_SYNC_REQ == 1)
	static uchar process_mlme_sync_request( uchar *buf, uint16_t length );
#endif

#if(CFG_MLME_POLL_REQ_CONF == 1)
static uchar process_mlme_poll_request( uchar *buf, uint16_t length );
#endif	/*(CFG_MLME_POLL_REQ_CONF == 1)*/

#if(CFG_MLME_START_REQ_CONF == 1)
static uchar process_mlme_start_request( uchar *buf, uint16_t length );
#endif	/*(CFG_MLME_START_REQ_CONF == 1)*/

#if(CGF_MLME_BEACON_REQ_CONF == 1)
static uchar process_mlme_beacon_request( uchar *buf, uint16_t length );
#endif	/*(CGF_MLME_BEACON_REQ_CONF == 1)*/

//static mac_status_t mac_prim_parse_address( uchar **, mac_address_t * );
mac_status_t mac_prim_parse_address( uchar **, mac_address_t * );
#ifdef MAC_CFG_SECURITY_ENABLED

//static mac_status_t mac_prim_parse_security( uchar **, security_params_t *, uchar );
mac_status_t mac_prim_parse_security( uchar **, security_params_t *, uchar );
        
        
static mac_status_t mac_prim_parse_security_for_data(
                                                uchar **buf, /* pointer to pointer to the data to parse */
                                                security_params_t *sec, /* location to store the result */
                                                uchar bitmask2003 /* mask for v.2003 SecurityEnabled bit */
                                           );
#endif

/*
** ============================================================================
** Private Variable Definitions
** ============================================================================
*/

/**************************************************************
 * table of MAC interface primitives and associated functions *
 **************************************************************/

static const nwk_to_mac_process_table_t nwk_to_mac_process_table[] =
{
    { MAC_MCPS_DATA_REQUEST             , process_mcps_data_request},
/*Uemsh : 10-01-2018 this thing goes to modified after sepration of mac and fan mac*/
#ifdef WISUN_FAN_MAC
    { FAN_MAC_MCPS_DATA_REQUEST         ,process_fan_mcps_data_request},
    { MLME_WS_ASYNC_FRAME_REQ           ,process_ws_async_frame_request},

#if(FAN_EAPOL_FEATURE_ENABLED == 1)    
    { MLME_EAPOL_FRAME_REQ              ,process_eapol_frame_request}, /*Debdeep::17-sep-2018*/
#endif // #if(FAN_EAPOL_FEATURE_ENABLED == 1)
    
#endif
#if(CFG_MCPS_PURGE_REQ_CONF == 1)   
    { MAC_MCPS_PURGE_REQUEST        , process_mcps_purge_request            },
#endif	/*(CFG_MCPS_PURGE_REQ_CONF == 1) */

#if(CFG_MLME_ASSOCIATE_REQ_CONF == 1)
    { MAC_MLME_ASSOCIATE_REQUEST    , process_mlme_associate_request        },
#endif	/*(CFG_MLME_ASSOCIATE_REQ_CONF == 1)*/

#if(CFG_MLME_ASSOCIATE_IND_RESP == 1)
    { MAC_MLME_ASSOCIATE_RESPONSE   , process_mlme_associate_response       },
#endif	/*(CFG_MLME_ASSOCIATE_IND_RESP == 1)*/

#if(CFG_MLME_DISASSOCIATE_REQ_CONF == 1)
    { MAC_MLME_DISASSOCIATE_REQUEST , process_mlme_disassociate_request     },
#endif	/*(CFG_MLME_DISASSOCIATE_REQ_CONF == 1)*/

#if(CFG_MLME_GET_REQ_CONF == 1)
    { MAC_MLME_GET_REQUEST          , process_mlme_get_request              },
#endif	/*(CFG_MLME_GET_REQ_CONF == 1)*/
    
#ifdef MAC_CFG_GTS_ENABLED
    { MAC_MLME_GTS_REQUEST          , process_mlme_gts_request              },
#endif
#if(CFG_MLME_POLL_REQ_CONF == 1)
    { MAC_MLME_POLL_REQUEST         , process_mlme_poll_request             },
#endif	/*(CFG_MLME_POLL_REQ_CONF == 1)*/

#if(CFG_MLME_ORPHAN_IND_RESP == 1)
    { MAC_MLME_ORPHAN_RESPONSE      , process_mlme_orphan_response          },
#endif	/*(CFG_MLME_ORPHAN_IND_RESP == 1)*/

#if(CGF_MLME_RESET_REQ_CONF == 1)
    { MAC_MLME_RESET_REQUEST        , process_mlme_reset_request            },
#endif	/*(CGF_MLME_RESET_REQ_CONF == 1)*/

#if(CFG_MLME_RX_ENABLE_REQ_CONF == 1)
    { MAC_MLME_RX_ENABLE_REQUEST    , process_mlme_rx_enable_request        },
#endif	/*(CFG_MLME_RX_ENABLE_REQ_CONF == 1)*/

#if(CFG_MLME_SCAN_REQ_CONF==1)    
    { MAC_MLME_SCAN_REQUEST         , process_mlme_scan_request             },
#endif 

#if(CFG_MLME_SET_REQ_CONF == 1)
    { MAC_MLME_SET_REQUEST          , process_mlme_set_request              },
#endif	/*(CFG_MLME_SET_REQ_CONF == 1)*/

#if(CFG_MLME_START_REQ_CONF == 1)
    { MAC_MLME_START_REQUEST        , process_mlme_start_request            },
/*Uemsh : 10-01-2018 this thing goes to modified after sepration of mac and fan mac*/
#ifdef WISUN_FAN_MAC 
    { FAN_MAC_MLME_SET_REQUEST   ,          process_fan_mac_mlme_set_req },
#endif  /*(end of WISUN_FAN_MAC) */  
    
#endif	/*(CFG_MLME_START_REQ_CONF == 1)*/

#if (CFG_MLME_SYNC_REQ == 1)
    { MAC_MLME_SYNC_REQUEST         , process_mlme_sync_request             },
#endif

#if(CGF_MLME_BEACON_REQ_CONF == 1)
    { MAC_MLME_BEACON_REQUEST 		, process_mlme_beacon_request			},
#endif	/*(CGF_MLME_BEACON_REQ_CONF == 1)*/

#ifdef PROCESS_PHY_PRIMITIVE_MESSAGES_ENABLED
    { PHY_PD_DATA_REQUEST            , process_pd_data_request              },
    { PHY_PLME_CCA_REQUEST           , process_plme_cca_request             },
    { PHY_PLME_ED_REQUEST            , process_plme_ed_request              },
    { PHY_PLME_SET_TRX_STATE_REQUEST , process_plme_set_trx_state_request   },
#endif

#ifdef PROCESS_APP_PRIMITIVE_MESSAGES_ENABLED
    { APP_CLOCK_REQUEST	             , process_app_clock_request            },
    { APP_TEST_REQUEST	             , process_app_test_request             },
    { APP_TIMER_REQUEST	             , process_app_timer_request            },
#endif
    { 0,                             NULL_POINTER                           }
};


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

uchar mac_process_primitive
(
	 uchar *bufp,   
	 uint16_t length   
)
{
	nwk_to_mac_process_table_t const *pt ;
	//uchar primitive; Sagar

	/* check the primitive, then point at the first parameter */
	//primitive = *bufp++; Sagar
	for( pt = nwk_to_mac_process_table; pt->primitive != 0; pt++ )
	{
		//if( pt->primitive == primitive ) Sagar
		if(pt->primitive == *bufp)
		{
			*bufp++;
			return pt->action( bufp, length - 1 );
		} /* end of if */
	} /* end of for */

	/* didn't find primitive - so indicate this to the caller */
	return 2;
}

/******************************************************************************/
#ifdef MAC_CFG_SECURITY_ENABLED
void cleanup_security_queues(uchar SetDefaultPIBValue)
{

    mac_rx_t *rxmsg = NULL;
    security_struct_t *pstx = NULL;
    security_struct_t *psrx = NULL;

	if ( mac_pib.mac_security_enabled == TRUE)
        {
        /* clear any tx messages queued for encryption */
        while( (pstx = (security_struct_t *) queue_item_get( (queue_t *) &mac_data.security->hallin_tx_queue )) != NULL_POINTER )
        {
            /* Put message buffer back on to free queue */
		    mac_mem_free_tx( pstx->private_msg_data );
		    /* release the memory used by security buffer */
		    app_bm_free((uint8_t*)pstx );

			event_clear( SECURE_EVENT );/*posting secure event*/
        }

	    if( mac_data.security->encrypt_msg != NULL_POINTER )
        {
            /* Put message buffer back on to free queue */
		    mac_mem_free_tx( mac_data.security->encrypt_msg->private_msg_data );
		    /* release the memory used by security buffer */
		    app_bm_free( (uint8_t*)mac_data.security->encrypt_msg );/*free mem*/
            mac_data.security->encrypt_msg = NULL_POINTER;
        }

        /* clear any rx messages queue for decryption */
	    while( (psrx = (security_struct_t *)queue_item_get( (queue_t *) &mac_data.security->hallin_rx_queue )) != NULL_POINTER )
	    {
            /* Put message buffer back on to free queue */
            mac_free_rcv_buffer( psrx->private_msg_data );
		    /* release the memory used by security buffer */
		    app_bm_free( (uint8_t*)pstx );/*free mem*/

			event_clear( UNSECURE_EVENT );/*posting unsecure event*/
        }

	    if( mac_data.security->decrypt_msg != NULL_POINTER )
        {
            /* Put message buffer back on to free queue */
            mac_free_rcv_buffer( mac_data.security->decrypt_msg->private_msg_data );
		    /* release the memory used by security buffer */
		    app_bm_free((uint8_t*) mac_data.security->encrypt_msg );/*free mem*/
            mac_data.security->encrypt_msg = NULL_POINTER;
        }

        /* clear any secured received messages that haven't been sent for decryption yet */
	    while( (rxmsg = (mac_rx_t *)queue_item_get( (queue_t *) &mac_data.security->rx_security_queue )) != NULL_POINTER )
	    {
                rxmsg->security_data = NULL_POINTER;
                /* Put message buffer back on to free queue */
                mac_free_rcv_buffer( rxmsg );/*free mem*/
        }

        /* now check if we must reset the PIB (LSB set) */
        if( (SetDefaultPIBValue & 0x01)!= 0 )
        {
            mac_pib_init_security();
        }
    }

    /* Initialise all other security data */
    mac_data.security->coord_realign_sec_param.security_level = 0;
    mac_data.security->coord_realign_sec_param.key_id_mode = 0;
    memset( mac_data.security->coord_realign_sec_param.key_identifier, 0xff,KEY_IDENTIFIER_MAX_LENGTH);

    mac_data.security->beacon_sec_param.security_level = 0;
    mac_data.security->beacon_sec_param.key_id_mode = 0;
    memset( mac_data.security->beacon_sec_param.key_identifier, 0xff, KEY_IDENTIFIER_MAX_LENGTH);
    mac_data.security->security_flags = 0;
}
#endif
/*@}*/

/******************************************************************************/

/*
** ============================================================================
** Private Function Definitions
** ============================================================================
*/
#if(CFG_MLME_START_REQ_CONF == 1)
static uchar process_mlme_start_request(
                                         uchar *buf,  
                                         uint16_t length 
                                       )

{
#if (CFG_MAC_STARTSM_ENABLED == 1)  
  mac_status_t status = MAC_SUCCESS;
    sm_event_t event;
#endif
#if (CFG_MAC_STARTSM_ENABLED == 1)       
    startsm_param_t param = {0};
    uint32_t temp = 0;
    uint16_t len=0;
    uint8_t short_subIdsCnt = 0, long_subIdsCnt = 0;
    uint8_t* p_mpm_eb_ie_list = param.mpm_ie_list;
#ifdef MAC_CFG_SECURITY_ENABLED
    security_params_t sec_params1 = {0}, sec_params2 = {0};
#endif

#define PAN_COORDINATOR_FLAG            bit(0)
#define BATTERY_LIFE_EXTENSION_FLAG     bit(1)
#define COORDINATOR_REALIGNMENT_FLAG    bit(2)
#define SECURITY_ENABLED_FLAG           bit(3)

#if(CFG_MLME_START_REQ_CONF == 0)
	/*validation to see if MLME-START is disabled. RFD does not support MLME-START*/
	status = MAC_INVALID_REQUEST;
	goto exit;
#endif	/*	CFG_MLME_START_REQ_CONF*/

    param.pan_id = get_ushort_BE(buf);// buf[1] | ( buf[0] << 8 );
    buf += 2;

    param.logical_channel = *buf++;

    /* ChannelPage and StartTime is in v.2006 only */
    if(  ( mac_pib.ConfigFlags & (USE_2006_PRIMITIVES|USE_2011_PRIMITIVES) ) )
    {
        param.channel_page = *buf++ & 0x1F;
        param.start_time = (ulong)buf[0] + ((ulong)buf[1] << 8) + ((ulong)buf[2] << 16);
        buf += 3;
    }
    else
    {
	PLME_get_request( phyCurrentPage, &len, &temp );
        param.channel_page = temp & 0xff;
        param.start_time = 0;
    }

    param.beacon_order = *buf & 0x0f;
    param.superframe_order = (*buf++ & 0xf0) >> 0x04;

    /* security info starts at next octet in v.2006 */
    if( mac_pib.ConfigFlags & USE_2006_PRIMITIVES )
    {
        buf += 1;
    }

    param.pan_coordinator = *buf & PAN_COORDINATOR_FLAG ? 1 : 0;
    param.coord_realignment = *buf & COORDINATOR_REALIGNMENT_FLAG ? 1 : 0;
    param.battery_life_ext = *buf & BATTERY_LIFE_EXTENSION_FLAG ? 1 : 0;

    /*Now buf is pounting to the pDSMESuperframeSpecification*/
    buf += 0x04;
    
    /*Now buf is pounting to the headerIEListLen*/
    len = *buf++;
    
    /*skip over the header IE list*/
    buf += len;
    
    /*Now buf is pounting to the payloadIEListLen*/
    len = *buf++;
    
    /*skip over the Payload IE list*/
    buf += len;
    
#if(0)    
    /*Now buf is pounting to the enhanced bcn order*/	
	param.enhanced_bcn_order = *buf++;
	param.offset_time_slot = *buf++;
	param.NBPAN_enhanced_bcn_order = get_ushort(buf); 
#else	
        
    /*Now buf is pointing to the MPM EB IE list len*/
    len = *buf++;
    
	param.mpm_ie_list_len = len;
	
	while(len)
	{
		if( *buf != PIE_MLME_NESTED )
		{
			*p_mpm_eb_ie_list++ = *buf++;			
		}
		else
		{
			*p_mpm_eb_ie_list++ = *buf++;
			/*p_attrib_val is pointing to sub IDs count field*/
			short_subIdsCnt = *buf++;
			long_subIdsCnt = *buf++;

			*p_mpm_eb_ie_list++ = short_subIdsCnt;
			*p_mpm_eb_ie_list++ = long_subIdsCnt;

			memcpy( p_mpm_eb_ie_list, buf, short_subIdsCnt );
			buf += short_subIdsCnt;
			
			p_mpm_eb_ie_list += short_subIdsCnt;
			
			memcpy( p_mpm_eb_ie_list, buf, long_subIdsCnt );
			
			buf += long_subIdsCnt;
			
			p_mpm_eb_ie_list += long_subIdsCnt;
			
		}
		len--;
	}
		
	//memcpy(param.mpm_ie_list,buf,len);
	
	//buf += len;
	
	/*Now buf is pointing to the enhanced bcn order*/	
	param.enhanced_bcn_order = *buf++;
	param.offset_time_slot = *buf++;
	param.NBPAN_enhanced_bcn_order = get_ushort(buf);
	buf += sizeof(ushort);  
#endif

 /* TBD We shouldn't use these if parsing fails */
    /* parse security fields */
    //if( (mac_pib.mac_security_enabled) && ( mac_data.security ))
#ifdef MAC_CFG_SECURITY_ENABLED
    {
        //buf += 1;
    	if( ( status = mac_prim_parse_security( &buf,
                                                &sec_params1,//&mac_data.security->coord_realign_sec_param,
                                                SECURITY_ENABLED_FLAG )
            ) != MAC_SUCCESS
          )
        {
            if ( param.coord_realignment )
            {
            	goto exit;
            }        	
        }
        /*ANAND: Enable this while implementing beacon security*/
       
        if( ( status = mac_prim_parse_security( &buf,&sec_params2,/*&mac_data.security->beacon_sec_param*/0 )) != MAC_SUCCESS)
        {
            goto exit;
        }        
    }
    
    if( (mac_pib.mac_security_enabled) && ( mac_data.security ))
    {
    	memcpy((uint8_t*)&(mac_data.security->coord_realign_sec_param),(uint8_t*)&(sec_params1),sizeof( security_params_t ));    
    	memcpy((uint8_t*)&(mac_data.security->beacon_sec_param),(uint8_t*)&(sec_params2),sizeof( security_params_t ));    
    	memcpy((uint8_t*)&(mac_data.security->mpm_eb_sec_param),(uint8_t*)&(mac_data.security->beacon_sec_param),sizeof( security_params_t ));    
    }
#else
	//init_sec_params(&mac_data.security->coord_realign_sec_param);
	//init_sec_params(&mac_data.security->beacon_sec_param);
	//init_sec_params(&mac_data.security->mpm_eb_sec_param);
#endif 
   
    event.trigger = (sm_trigger_t) STARTSM_TRIGGER_START_REQ;
    event.param.vector = &param;
    SM_DISPATCH( (sm_t*) startsm_p, &event );

    return 1;

 exit:
    if( status != MAC_SUCCESS )
    {
        send_mlme_start_confirm( status );
    }
#endif      
    return 1; 
}
#endif	/*(CFG_MLME_START_REQ_CONF == 1)*/
/******************************************************************************/

/**
 *
 * When v.2006 primitives are used, the value of *buf increases as security
 * fields are being parsed.
 * After successful completition, *buf points to the next octet to be parsed.
 *
 * Note, that when v.2003 primitives are used, the value of *buf remains
 * intact.
 */
#ifdef MAC_CFG_SECURITY_ENABLED
//static mac_status_t mac_prim_parse_security(
//                                                uchar **buf, /* pointer to pointer to the data to parse */
//                                                security_params_t *sec, /* location to store the result */
//                                                uchar bitmask2003 /* mask for v.2003 SecurityEnabled bit */
//                                           )
mac_status_t mac_prim_parse_security(
                                                uchar **buf, /* pointer to pointer to the data to parse */
                                                security_params_t *sec, /* location to store the result */
                                                uchar bitmask2003 /* mask for v.2003 SecurityEnabled bit */
                                           )
{
    if( !sec )
    {
        return MAC_SUCCESS;
    }

    if( mac_pib.ConfigFlags & ( USE_2006_PRIMITIVES|USE_2011_PRIMITIVES ) )
    {
		/* parsing v.2006 security fields */
		sec->security_level = **buf & SEC_LEVEL_MASK;
		sec->key_id_mode = (**buf & KEY_ID_MASK) >> 4;
		/*following line of code introduced to by pass this function and
		return as MAC SUCCESS. Need to be changed when integrating the
		security*/

		*buf += 1;

#ifndef MAC_CFG_SECURITY_ENABLED
		/*load the security level member with MAC_SECURITY_NONE* so that this
		function returns MAC_SUCCESS for no security build */
		sec->security_level = MAC_SECURITY_NONE;
#endif
		
		/* parse Key Identifier */
		switch( sec->key_id_mode )
		{
			case 0:
				/*do nothing as no more sec information needs to be extracted
				from the buffer*/
				break;

			case 1:
				/* Key Identifier - 1 octet Key Index */
				sec->key_identifier[0] = **buf;
				*buf += 1;
				break;

			case 2:
				/* Key Identifier - 4 octet Key Source + 1 octet Key Index */
				memcpy( sec->key_identifier, *buf, 4 );
				*buf += 4;
				sec->key_identifier[4] = **buf;
				*buf += 1;
				break;

			case 3:
				/* Key Identifier - 8 octet Key Source + 1 octet Key Index */
				memcpy( sec->key_identifier, *buf, 8 );
				*buf += 8;
				sec->key_identifier[8] = **buf;
				*buf += 1;
				break;

			default:
				return MAC_INVALID_PARAMETER;
		}
	// Raka ... [16-Nov-2017] .. Stack should support Unsecured packet transmission 
                // even if the security is enabled
                
                
//		if( sec->security_level != MAC_SECURITY_NONE )
//		{
//			if( mac_pib.mac_security_enabled )
//			{
//				/*everything is done*/
//			}
//			else
//			{
//				return MAC_UNSUPPORTED_SECURITY;
//			}
//		}
//		else
//		{
//			if( mac_pib.mac_security_enabled )
//			{
//				return MAC_UNSUPPORTED_SECURITY;
//			}
//
//		}
    }
    else
    {
        /* v.2003 security is not supported */
        if( **buf & bitmask2003 )
        {
            return MAC_UNSUPPORTED_SECURITY;
        }
    }
    return MAC_SUCCESS;
}

static mac_status_t mac_prim_parse_security_for_data(
                                                uchar **buf, /* pointer to pointer to the data to parse */
                                                security_params_t *sec, /* location to store the result */
                                                uchar bitmask2003 /* mask for v.2003 SecurityEnabled bit */
                                           )
{
    if( !sec )
    {
        return MAC_SUCCESS;
    }

    if( mac_pib.ConfigFlags & ( USE_2006_PRIMITIVES|USE_2011_PRIMITIVES ) )
    {
		/* parsing v.2006 security fields */
		sec->security_level = **buf & SEC_LEVEL_MASK;
		sec->key_id_mode = (**buf & KEY_ID_MASK) >> 4;
		/*following line of code introduced to by pass this function and
		return as MAC SUCCESS. Need to be changed when integrating the
		security*/

		*buf += 1;

#ifndef MAC_CFG_SECURITY_ENABLED
		/*load the security level member with MAC_SECURITY_NONE* so that this
		function returns MAC_SUCCESS for no security build */
		sec->security_level = MAC_SECURITY_NONE;
#endif
		
		/* parse Key Identifier */
		switch( sec->key_id_mode )
		{
			case 0:
				/*do nothing as no more sec information needs to be extracted
				from the buffer*/
				break;

			case 1:
				/* Key Identifier - 1 octet Key Index */
				sec->key_identifier[0] = **buf;
				*buf += 1;
				break;

			case 2:
				/* Key Identifier - 4 octet Key Source + 1 octet Key Index */
				memcpy( sec->key_identifier, *buf, 4 );
				*buf += 4;
				sec->key_identifier[4] = **buf;
				*buf += 1;
				break;

			case 3:
				/* Key Identifier - 8 octet Key Source + 1 octet Key Index */
				memcpy( sec->key_identifier, *buf, 8 );
				*buf += 8;
				sec->key_identifier[8] = **buf;
				*buf += 1;
				break;

			default:
				return MAC_INVALID_PARAMETER;
		}
						
		if( sec->security_level != MAC_SECURITY_NONE )
		{
			if( mac_pib.mac_security_enabled )
			{
				/*everything is done*/
			}
			else
			{
				return MAC_UNSUPPORTED_SECURITY;
			}
		}
//                else
//		{
//			if( mac_pib.mac_security_enabled )
//			{
//				return MAC_UNSUPPORTED_SECURITY;
//			}
//
//		}
    }
    else
    {
        /* v.2003 security is not supported */
        if( **buf & bitmask2003 )
        {
            return MAC_UNSUPPORTED_SECURITY;
        }
    }
    return MAC_SUCCESS;
}
#endif
/******************************************************************************/

/*gets a mac address from the buffer
 * get the MAC address from the serialized data @ buf and store in addr
  retval status of the operation
 */
//static mac_status_t mac_prim_parse_address
mac_status_t mac_prim_parse_address(
                                           uchar **buf, /* pointer to pointer to the data to parse */
                                           mac_address_t *addr /* location to store the result */
                                           )
{
    addr->address_mode = **buf;
    *buf += 1;


    switch( addr->address_mode )
    {
    case ADDR_MODE_NONE:/*No Address*/
        break;

    case ADDR_MODE_SHORT:/*For Short Address*/
    	addr->pan_id = (*buf)[0] | (((uint16_t)((*buf)[1])) << 8);
    	*buf += 2;  
        addr->address.short_address = (*buf)[0] | (((uint16_t)((*buf)[1])) << 8);
        *buf += 2;
        break;

    case ADDR_MODE_EXTENDED:/*For Extended Address*/
    	addr->pan_id = (*buf)[0] | (((uint16_t)((*buf)[1])) << 8);
    	*buf += 2;        
        addr->address.ieee_address = *buf;
        *buf += 8;
        break;
	
    case ADDR_MODE_RESVD:/*fall through*/
    default:/*Not A Valid Address*/
        return MAC_INVALID_ADDRESS;
    }
    return MAC_SUCCESS;
}

/******************************************************************************/
#if (CFG_MLME_SYNC_REQ == 1)
/*processes the MLME-SYNC.request primitive received from the NHL*/
static uchar process_mlme_sync_request(
										   uchar *buf, /* buffer to process */
										   uint16_t length /* length of data in buffer */
                                       )
{
    uint32_t channel_page = 0;
    uint32_t current_channel = 0;   
	sm_event_t event;
	
#if(CFG_MLME_SYNC_REQ == 0) 		
	return 2;
#else	

    event.trigger = (sm_trigger_t) SYNCSM_TRIGGER_SYNC_REQ;
    event.param.scalar = aMaxLostBeacons;

    current_channel = *buf++;

    /* Check if we are using 802.15.4-2006 primtives and above */
     if(  ( mac_pib.ConfigFlags & (USE_2006_PRIMITIVES|USE_2011_PRIMITIVES) ) )
    {
        channel_page = *buf++;
    }
    /* tracking mode is the parameter of the event */
    event.param.scalar |= *buf++ & 1 ? SYNCSM_FLAG_TRACK : 0;
#if(CFG_MLME_SYNC_LOSS_IND == 1)  
    if( ( mac_pib.BeaconOrder == 15 ) || (mac_pib.PANId == 0xffff))
    {      
	/* cannot sync to a beacon on a beaconless network ! */
       // send_mlme_sync_loss_indication( MAC_BEACON_LOST, NULL_POINTER );

        //return 1;
    }
#endif	/*(CFG_MLME_SYNC_LOSS_IND == 1)*/


    if( PLME_set_request( phyCurrentPage, 1, &channel_page ) != PHY_SUCCESS )
    {
#if(CFG_MLME_SYNC_LOSS_IND == 1)
        /* invalid channel, so go no further */
        send_mlme_sync_loss_indication( MAC_BEACON_LOST, NULL_POINTER );
#endif	/*(CFG_MLME_SYNC_LOSS_IND == 1)*/
        return 1;
    }

    /* set the channel */
    if( PLME_set_request( phyCurrentChannel, 2, &current_channel ) != PHY_SUCCESS )
    {
#if(CFG_MLME_SYNC_LOSS_IND == 1)        
		/* invalid channel, so go no further */
        send_mlme_sync_loss_indication( MAC_BEACON_LOST, NULL_POINTER );
#endif	/*(CFG_MLME_SYNC_LOSS_IND == 1)*/
        /* TBD Shouldn't we set back original channel page here? */
        return 1;
    }

    /* notify SYNCSM about MLME-SYNC.request */
    SM_DISPATCH( (sm_t *) &syncsm, &event );

    return 1;
    
#endif 	/*CFG_MLME_SYNC_REQ    */
}
#endif

/******************************************************************************/
#if(CFG_MLME_POLL_REQ_CONF == 1)
/* processes the MLME-POLL.request primitive received from the NHL
 a Poll request causes a slave to issue a Data request mac command*/
static uchar process_mlme_poll_request(
										   uchar *buf,  /* buffer to process */
										   uint16_t length /* length of data in buffer */
                                       )
{
    mac_status_t status = MAC_SUCCESS;
    mac_address_t coord = 0, src = 0;
    mac_tx_t *dtx = NULL;
    security_params_t sec_params = 0;
    sm_event_t event;
#if(CFG_MAC_DRSM_ENABLED == 1)
    /* check if DR-SM is not busy */
    if( drsm_get_state( &drsm ) != DRSM_STATE_NONE )
    {
        status = MAC_TRANSACTION_OVERFLOW;
        goto exit;
    }
#endif
    /* parse coordinator address */
    if( ( status = mac_prim_parse_address( &buf, &coord ) ) != MAC_SUCCESS )
    {
        /* Invalid parameter if the address parsing fails */
        status = MAC_INVALID_PARAMETER;
        goto exit;
    }

#ifdef MAC_CFG_SECURITY_ENABLED
    /* parse security fields */
    if( ( status = mac_prim_parse_security( &buf, &sec_params, 1 ) ) != MAC_SUCCESS )
    {
        goto exit;
    }
#else
	init_sec_params(&sec_params);
#endif

    if( ( ( coord.pan_id == PAN_ID_UNKNOWN ) ||
        ( coord.address.short_address == INVALID_SHORT_ADDRESS ) ) 
		|| ( ( coord.address_mode != MAC_SHORT_ADDRESS ) &&
        ( coord.address_mode != MAC_IEEE_ADDRESS ) ) )
    {
        status = MAC_INVALID_PARAMETER;
        goto exit;
    }

    if( ( mac_pib.ShortAddress == USE_IEEE_ADDRESS ) ||
        ( mac_pib.ShortAddress == INVALID_SHORT_ADDRESS ) )
    {
        src.address_mode = MAC_IEEE_ADDRESS;
        src.address.ieee_address = aExtendedAddress; 
    }
    else
    {
        src.address_mode = MAC_SHORT_ADDRESS;
        src.address.short_address = mac_pib.ShortAddress;
    }
	src.pan_id = mac_pib.PANId;

    /* build packet */
	status = mac_frame_build_data_request( &dtx, &src, &coord, &sec_params, DATA_REQUEST );
	
    /* queue packet for transmission */
    if( dtx != NULL_POINTER )
    {
        /* indicate to DR-SM */
        event.trigger = (sm_trigger_t) DRSM_TRIGGER_START;
        event.param.scalar = DRSM_FLAG_POLLING;
        SM_DISPATCH( (sm_t *) &drsm, &event );

        /* queue packet */
        if( ( status = mac_queue_direct_transmission( dtx ) ) != MAC_SUCCESS )
        {
            /* cancel DR-SM if queuing fails */
            event.trigger = (sm_trigger_t) DRSM_TRIGGER_CANCEL;
            event.param.scalar = 0;
            SM_DISPATCH( (sm_t *) &drsm, &event );
        }
    }

 exit:
    if( status != MAC_SUCCESS )
    {
        send_mlme_poll_confirm( status );
    }
    return 1;
}
#endif	/*(CFG_MLME_POLL_REQ_CONF == 1)*/

/******************************************************************************/
#if(CGF_MLME_BEACON_REQ_CONF == 1)
/* processes the MLME-BEACON.request primitive received from the NHL
  a beacon request causes a node to send the requested beacon/enhanced beacon*/
static uchar process_mlme_beacon_request(
                                       uchar *buf,  /* buffer to process */
                                       uint16_t length /* length of data in buffer */
                                       )
{
	mac_tx_t *p = NULL;
	mac_address_t DestAddr = {0};
	mac_status_t status = MAC_SUCCESS;
	uchar* header_ie_ids = NULL;
	uchar* payload_ie_ids = NULL;	
	sm_event_t e = { (sm_trigger_t) TRXSM_TRIGGER_BCN_TX_REQUIRED, { 0 } };
	uchar bcnType = *buf++;
	uchar channel = *buf++;
	uchar channelPage = *buf++;
	uchar superFrameOrder = *buf++;
	//uchar BSNSuppression;
#if (CFG_MAC_STARTSM_ENABLED == 1)          
	startsm_param_t* p_pending = &(startsm_p->pending);
#endif	
	 /* test if i am capable of sending beacons and test if the channel,page and 
	 SO are sent properly */
                                 
	if( ((
#if (CFG_MAC_STARTSM_ENABLED == 1)              
              !( startsm_get_flags( startsm_p ) & STARTSM_FLAG_COORD_MASK )
#endif                
                )
	|| ( p_pending->logical_channel != channel)
	|| (p_pending->channel_page != channelPage)
	|| ( p_pending->superframe_order !=superFrameOrder ))
	|| ( mac_pib.BeaconAutoRespond)
	|| ( mac_prim_parse_address( &buf, &DestAddr ) != MAC_SUCCESS)
	)
	{
		status = MAC_INVALID_PARAMETER; 
		goto exit;
	}
	/*
	The MLME-BEACON.request primitive requests the generation of a 
	beacon or enhanced beacon in a non-beacon enabled PAN, 
	either in response to a beacon request command when 
	macBeaconAutoRespond is set to FALSE. If it is set 
	to TRUE, this usage of this request primitive is invalid.
	
	if( mac_pib.BeaconAutoRespond )
	{
		status = MAC_INVALID_REQUEST;
		goto exit;
	}
	*/
	/* destination address 
    if( mac_prim_parse_address( &buf, &DestAddr ) != MAC_SUCCESS )
    {
         Invalid parameter if the address parsing fails 
        status = MAC_INVALID_PARAMETER;
        goto exit;
    }*/
    
    *buf++;
    if(*buf)
    {
    	header_ie_ids = buf;
    }
    
    buf += (( *buf ) + 1 );
    if( *buf )
    {
    	payload_ie_ids = buf;
    }
    
#ifndef WISUN_ENET_PROFILE    
	if( bcnType == 0x01 )
	{
		if( ( payload_ie_ids == NULL ) && ( header_ie_ids == NULL ) )
	    {
	    	status = MAC_INVALID_PARAMETER;
	    	goto exit;
	    }
	}
#endif
	status = mac_frame_build_beacon
	    	( 
	    		&p, 
	    		( bcnType + 1 ),
				&DestAddr,
	    		header_ie_ids, 
	    		payload_ie_ids 
	    	);
    	
	/* construct beacon/enahanced beacon*/
    if(  status == MAC_SUCCESS )
    {
        /* queue beacon/enhanced beacon */
    	mac_queue_beacon_transmission( p, bcnType + 1 );
    	
    	e.param.scalar = bcnType + 1;
    	
	    SM_DISPATCH( (sm_t *) trxsm_p, &e );

    }
    
   exit:
    if( status != MAC_SUCCESS )
    {
        send_mlme_beacon_confirm( status );
    }
    
    return 1;
}  
#endif	/*(CGF_MLME_BEACON_REQ_CONF == 1)*/

/******************************************************************************/
#if(CFG_MLME_ORPHAN_IND_RESP == 1)
/* processes the MLME-ORPHAN.response primitive received from the NHL
**  orphan response has IEEE address, short address,
**  and flags for Associated Member and security enabled
*/
static uchar process_mlme_orphan_response(
											  uchar *buf,  /* buffer to process */
											  uint16_t length /* length of data in buffer */
                                         )

{
    mac_status_t status = MAC_SUCCESS;
    mac_tx_t *dtx = NULL;
    mac_address_t src = {0}, dst = {0};
    uchar *ieee_addr = NULL;
    ushort short_addr = 0;
    security_params_t sec_params = 0;
    uint32_t attr_value = 0;
    uint16_t len = 0;
    uint8_t channel_page = 0, channel = 0;

    /* if AssociatedMember field is false, we discard this primitive */
    if( (buf[8+2] & 0x01) == 0 )
    {
        status = MAC_SUCCESS;
        goto exit;
    }

    /* get OrphanAddress field */
    ieee_addr = buf;
    buf += 8;

    /* get ShortAddress field */

    short_addr = buf[0] + (((uint16_t)(buf[1])) << 8);
    
    buf += 2;

    /* set source address */
    dst.address_mode = src.address_mode = MAC_IEEE_ADDRESS;
    src.pan_id = mac_pib.PANId;
    src.address.ieee_address = aExtendedAddress;

    /* set destination address */
    dst.pan_id = BROADCAST_PAN_ID;
    dst.address.ieee_address = ieee_addr;

    /* skip AssociatedMember field for v.2006 primitives */
    if( mac_pib.ConfigFlags & ( USE_2006_PRIMITIVES|USE_2011_PRIMITIVES ) )
    {
        buf += 1;
    }
#ifdef MAC_CFG_SECURITY_ENABLED
    /* parse security fields */
    if( ( status = mac_prim_parse_security( &buf, &sec_params, 2 ) ) != MAC_SUCCESS )
    {
        goto exit;
    }
#else
	init_sec_params(&sec_params);
#endif
    /* check the short address */
    if( short_addr == BROADCAST_SHORT_ADDRESS )
    {
        /* invalid short address */
        status = MAC_INVALID_PARAMETER;
        goto exit;
    }

    PLME_get_request( phyCurrentPage, &len, &attr_value );
    channel_page = attr_value;
    attr_value = 0;
    PLME_get_request( phyCurrentChannel, &len, &attr_value );
    channel = attr_value;

    status = mac_frame_build_coordinator_realignment( &dtx,
                                                      &src,
                                                      &dst,
                                                      ieee_addr,
                                                      short_addr,
                                                      channel,
                                                      channel_page,
                                                      &sec_params );

    if( dtx != NULL_POINTER )
    {
        /* and transmit it */
        status = mac_queue_direct_transmission( dtx );
    }

 exit:
    if( status != MAC_SUCCESS )
    {
        /* indicate the error */
        send_mlme_comm_status_indication( &src, &dst, status, &sec_params );
    }
    return 1;
}
#endif	/*(CFG_MLME_ORPHAN_IND_RESP == 1)*/

/******************************************************************************/
#if(CGF_MLME_RESET_REQ_CONF == 1)
/* processes the MLME-RESET.request primitive received from NHL*/
static uchar process_mlme_reset_request(
											uchar *buf, /* buffer to process */
											uint16_t length /* length of data in buffer */
                                        )

{
    mac_tx_t *dtx = NULL;
    mac_rx_t *rxmsg = NULL;
    sm_event_t event;

#ifdef UTEST
    /* see if unit test output is requested */
    if( *buf & 0x02 )
    {
        send_mlme_reset_confirm( MAC_SUCCESS );

        utu_print( );

        return 1;
    }
#endif

//    disable_interrupt();
#if (CFG_MAC_CCA_ENABLED == 1) 
    /* cancel any ongoing CCA */
    event.trigger = (sm_trigger_t) CCASM_TRIGGER_CANCEL;
    event.param.scalar = 0;
    SM_DISPATCH( trxsm.ccasm, &event );
#endif    

    /* turn off the transceiver */
    event.trigger = (sm_trigger_t) TRXSM_TRIGGER_CANCEL;
    event.param.scalar = TRXSM_PARAM_OFF;
    SM_DISPATCH( (sm_t*) &trxsm, &event );
#if(CFG_MAC_MPMSM_ENABLED == 1)    
    event.trigger = (sm_trigger_t) MPMSM_TRIGGER_RESET;
#endif    
    //event.param.scalar = TRXSM_PARAM_OFF;

#if(CFG_MAC_MPMSM_ENABLED == 1)    
    SM_DISPATCH( (sm_t*) &mpmsm, &event );
#endif
    
    /* clear received message queue */
    while( (rxmsg = (mac_rx_t *)queue_manager_pop_front( QUEUE_RX_MSG )) != NULL_POINTER )
    {
#ifdef MAC_CFG_SECURITY_ENABLED
        rxmsg->security_data = NULL_POINTER;
#endif
        /* Put message buffer back on to free queue */
        mac_free_rcv_buffer( rxmsg );
    }

    /* clear any dtx messages still queued */
    while( (dtx = (mac_tx_t *) queue_manager_pop_front( QUEUE_CAP_TX ))!= NULL_POINTER )
    {
		mac_mem_free_tx( dtx );
    }

    /* clear any messages due for confirmation still queued */
    while( (dtx = (mac_tx_t *) queue_manager_pop_front( QUEUE_TX_DONE ))!= NULL_POINTER )
    {
		mac_mem_free_tx( dtx );
    }

    /* clear any messages in the beacon queue */
    while( (dtx = (mac_tx_t *) queue_manager_pop_front( QUEUE_BCN_TX_CURR ))!= NULL_POINTER )
    {
		mac_mem_free_tx( dtx );
    }

	/* clear any messages in the beacon queue */
    while( (dtx = (mac_tx_t *) queue_manager_pop_front( QUEUE_EB_TX_CURR ))!= NULL_POINTER )
    {
		mac_mem_free_tx( dtx );
    }

	/* clear any messages in the beacon queue */
    while( (dtx = (mac_tx_t *) queue_manager_pop_front( QUEUE_MPM_EB_TX_CURR ))!= NULL_POINTER )
    {
		mac_mem_free_tx( dtx );
    }

#ifdef MAC_CFG_GTS_ENABLED
    /* clear any GTS's */
    for( i = 0; i < MAX_GTS_DESCRIPTORS; i++ )
    {
		mac_data.gts_params[i].gts_state = GTS_UNUSED;
		mac_data.gts_params[i].expiration_timer = 0;
		mac_data.gts_params[i].desc_timer = 0;
    }

    /* clear current GTS message */
    if( mac_data.gts_msg != NULL_POINTER )
    {
        mac_mem_free_tx( mac_data.gts_msg );
        mac_data.gts_msg = NULL_POINTER;
    }

    /* clear any GTS messages still queued */
    while( (dtx = (mac_tx_t *) queue_manager_pop_front( QUEUE_GTS_TX ))!= NULL_POINTER )
    {
		mac_mem_free_tx( dtx );
    }

    mac_data.gts_count = 0;
#endif
#if(CFG_MAC_PENDADDR_ENABLED == 1)
    /* clear pending messages */
    pendaddr_initialise();
#endif    
    while( (dtx = (mac_tx_t *) queue_manager_pop_front( QUEUE_INDIRECT_TX )) != NULL_POINTER )
    {
        mac_mem_free_tx(dtx);
    }

#ifdef ENHANCED_ACK_SUPPORT
	while( (dtx = (mac_tx_t *) queue_manager_pop_front( QUEUE_ENACKWAIT )) != NULL_POINTER )
    {
        mac_mem_free_tx(dtx);
    }
#endif

	


    /* check if we must reset the PIB */
    if( (*buf & 0x01) != 0 )
    {
        mac_pib_init( );
		
        //phy_reset_sports_driver();
        /* set default channel */
        PHY_Reset( ); 
    }

    /*  reset the mac state machine */
    mac_data.coordinator = 0;
#if(CFG_ASSOCSM_ENABLED == 1)
    /* disassociate device */
    event.trigger = (sm_trigger_t) ASSOCSM_TRIGGER_DISASSOCIATE;
    event.param.scalar = 0;
    SM_DISPATCH( (sm_t *) &assocsm, &event );
#endif
    
#ifdef MAC_CFG_GTS_ENABLED
    mac_data.gts_rx_request_pending = 0;    /* set when we request a GTS slot, cleared when we send the confirm */
    mac_data.gts_tx_request_pending = 0;    /* set when we request a GTS slot, cleared when we send the confirm */
#endif

    mac_data.in_gts_receive_period =  /* set if we are in a GTS receive period */
    mac_data.rx_enable_active =       /* set if we are currently in the active part of rx enable */
    mac_data.ieee_address_pending =   /* there is a message pending for our IEEE address */
    mac_data.short_address_pending =  /* there is a message pending for our short address */
    mac_data.gts_tx_start_slot =
    mac_data.gts_rx_start_slot =
    mac_data.gts_tx_length =
    mac_data.gts_rx_length = 0;

		
    mac_data.final_cap_slot = 15;
    mac_data.gts_start_slot = 16;
    mac_data.panid_conflict_state = PANID_CONFLICT_NOT_DETECTED;

    /*TBD  It should be reviewed if 'confirm' primitives are
             sent for ongoing 'request' primitives or not. */

    /* cancel ongoing start process */
#if (CFG_MAC_STARTSM_ENABLED == 1)      
    event.trigger = (sm_trigger_t) STARTSM_TRIGGER_RESET;
    //event.param.scalar = 0; 
    SM_DISPATCH( (sm_t*) startsm_p, &event );
#endif
#if(CFG_MAC_SCANSM_ENABLED == 1)      
    /* cancel ongoing scan process */
    event.trigger = (sm_trigger_t) SCANSM_TRIGGER_CANCEL;
    //event.param.scalar = 0;
    SM_DISPATCH( (sm_t*) &scansm, &event );
#endif    

#if(CFG_MLME_SYNC_REQ == 1)
    /*  cancel ongoing sync process */
    event.trigger = (sm_trigger_t) SYNCSM_TRIGGER_CANCEL;
    //event.param.scalar = 0;
    SM_DISPATCH( (sm_t*) &syncsm, &event );
#endif
    
#if(CFG_MAC_DRSM_ENABLED == 1)
    /* cancel ongoing data request */
    event.trigger = (sm_trigger_t) DRSM_TRIGGER_CANCEL;
    //event.param.scalar = 0;
    SM_DISPATCH( (sm_t*) &drsm, &event );
#endif
     
#ifdef MAC_CFG_BEACONING_ENABLED     
    /* reset superframe */
    event.trigger = (sm_trigger_t) SFTSM_TRIGGER_RESET;
    //event.param.scalar = 0;
    SM_DISPATCH( (sm_t*) &sftsm_in, &event );

    event.trigger = (sm_trigger_t) SFTSM_TRIGGER_RESET;
    //event.param.scalar = 0;
    SM_DISPATCH( (sm_t*) &sftsm_out, &event );

#endif

#if(CFG_MAC_SFTSM_ENABLED == 1)
    sftsm_in_p = NULL_POINTER;
#endif    

    /* reset transmitter */
    event.trigger = (sm_trigger_t) TRXSM_TRIGGER_CANCEL;
    event.param.scalar = TRXSM_PARAM_IDLE;
    //event.param.scalar = TRXSM_PARAM_IDLE;
    SM_DISPATCH( (sm_t*) &trxsm, &event );

#if ( ( MAC_CFG_LE_RIT_CAPABILITY == 1 ) || ( MAC_CFG_LE_CSL_CAPABILITY == 1 ) )
    /*reset LE-SM*/
	event.trigger = (sm_trigger_t) LE_TRIGGER_CANCEL;
	SM_DISPATCH( (sm_t *) &low_energy, &event );
#endif 

//    enable_interrupt();

    send_mlme_reset_confirm( MAC_SUCCESS );

    return 1;
}
#endif	/*(CGF_MLME_RESET_REQ_CONF == 1)*/

/******************************************************************************/
#if(CFG_MLME_RX_ENABLE_REQ_CONF == 1)
/*processes the MLME-RX-ENABLE.request primitive received from the NHL*/
static uchar process_mlme_rx_enable_request(
                                            uchar *buf, /* buffer to process */
                                            uint16_t length /* length of data in buffer */
                                           )
{
    uchar defer_permit = 0;
    ulong rx_on_time = 0;
    ulong rx_on_duration = 0;

    defer_permit = *buf++;

    rx_on_time = (ulong) buf[0] + ((ulong)buf[1] << 8) + ((ulong)buf[2]<< 16);
    buf += 3;
    rx_on_duration = (ulong)buf[0] + ((ulong)buf[1]<<8) + ((ulong)buf[2] <<16);

    /* adjust delay and enable times to be 20 symbol counts */

    if( mac_pib.BeaconOrder != 15 )
    {
        /* TBD should be the superframe duration in symbols */
        if( (rx_on_time + rx_on_duration) > (0xffffffff) )
        {
			#if(CFG_MLME_RX_ENABLE_REQ_CONF == 1)
				send_mlme_rx_enable_confirm( MAC_INVALID_PARAMETER );
			#endif
            return 1;
        }

        /* test if we have already passed the rx on time */
        if( mac_pib.BeaconTxTime > rx_on_time )
        {
            /* yes, so check if we can defer to the next superframe */
            if( defer_permit == 0 )
            {
                /* this is the correct error, despite being misleading */
				#if(CFG_MLME_RX_ENABLE_REQ_CONF == 1)
					send_mlme_rx_enable_confirm( MAC_OUT_OF_CAP );
				#endif
                return 1;
            }
            else
            {
                mac_data.rx_enable_state = RX_ENABLE_DEFERRED;
            }
        }
        else
        {
            mac_data.rx_enable_state = RX_ENABLE_DELAYING;
        }
    }
    else /* not beaconing */
    {
        rx_on_time = 0;
        mac_data.rx_enable_state = RX_ENABLE_DELAYING;
    }

    set_process_activity( AF_RX_ENABLE_ACTIVE );
    mac_data.rx_enable_delay_timer = rx_on_time ;       /* delay before receiver is enabled (in backoff slots) */
    mac_data.rx_enable_timer = rx_on_duration;          /* period to leave rx enabled (in backoff slots)*/

    return 1;
}
#endif	/*(CFG_MLME_RX_ENABLE_REQ_CONF == 1)*/

/******************************************************************************/
#if(CFG_MLME_SCAN_REQ_CONF==1)
/* processes the MLME-SCAN.request primitive received from the NHL*/
static uchar process_mlme_scan_request(
										   uchar *buf,  /* buffer to process */
										   uint16_t length /* length of data in buffer */
                                       )
{
    mac_status_t status = MAC_SUCCESS;
    sm_event_t event;
    scan_param_t param = {0};
    uint64_t channels = 0;
    uint64_t supported_channels = 0;
    //uint64_t invalid_channels;	
    uint16_t len = 0;
    uint8_t* p = NULL;
    uint8_t short_subIdsCnt = 0, long_subIdsCnt = 0;
    
    memset((uint8_t*)&channels,0x00,8);
    memset((uint8_t*)&supported_channels,0x00,8);
    //memset((uint8_t*)&invalid_channels,0x00,8);
    
    /* scan type */
    param.type = *buf++;
    
    if( ( ( param.type ) > MPM_EB_PASSIVE_SCAN ) ||
    ( ( ( param.type ) > ORPHAN_SCAN) && ( ( param.type ) < EB_ACTIVE_SCAN ) )    
    )
    {
        status =  MAC_INVALID_PARAMETER;
        goto exit;
    }
    
#if(CFG_ENERGY_DETECTION_SCAN == 0)    
    if( param.type == ENERGY_DETECT_SCAN )
    {
    	status =  MAC_INVALID_REQUEST;
    	goto exit;
    }
#endif	/*(CFG_ENERGY_DETECTION_SCAN == 0)    */
    

    /* scan channels */
    param.channels[7] = *buf++;
    param.channels[6] = *buf++;
    param.channels[5] = *buf++;
    param.channels[4] = *buf++;
    param.channels[3] = *buf++;
    param.channels[2] = *buf++;
    param.channels[1] = *buf++;
    param.channels[0] = *buf++;

    /*TODO: consider the 8 byte entity. here only 4 bytes are considerred
    channels = (((((((((((((param.channels[7]  << 8)
    			  + param.channels[6]) << 8)
    			  + param.channels[5]) << 8)
        		  + param.channels[4]) << 8)
        		  + param.channels[3]) << 8)
        		  + param.channels[2]) << 8)
        		  + param.channels[1]) << 8)
        		  + param.channels[0];*/

    memcpy((uint8_t*)&channels,param.channels,sizeof(uint64_t));

    /* scan duration */
    if( param.type == ORPHAN_SCAN )
    {
        /* ignore duration parameter for orphan scan */
        param.duration = 5;
    }
    else
    {
        if( ( param.duration = *buf ) > 14 )
        {
            status =  MAC_INVALID_PARAMETER;
            goto exit;
        }
    }
	buf++;
    /* check if we are using 802.15.4-2006 primtives */
    if( mac_pib.ConfigFlags & ( USE_2006_PRIMITIVES|USE_2011_PRIMITIVES ) )
    {
        param.page = *buf & 0x1f;

        /* TBD Check if channel page is valid */
        if( param.page != 0x09 )
        {
            /* channel page is invalid, so report and leave */
            status = MAC_INVALID_PARAMETER;
            goto exit;
        }

        /*Increment by 1 after reading channel page and skip 1 byte link quality scan field*/
        buf+=2;

        /* Load frame control options*/
        param.fc_options = *buf++;       
        
	    param.hdr_ies_cnt = *buf++;
	    memcpy( param.hdr_ie_list, buf, param.hdr_ies_cnt );
	    
	    buf += param.hdr_ies_cnt;
	    
	    len = param.pld_ies_cnt = *buf++;
	    
	    p = param.pld_ie_list;
	    
	    while(len)
		{
			if( *buf != PIE_MLME_NESTED )
			{
				*p++ = *buf++;			
			}
			else
			{
				*p++ = *buf++;
				/*p_attrib_val is pointing to sub IDs count field*/
				short_subIdsCnt = *buf++;
				long_subIdsCnt = *buf++;

				*p++ = short_subIdsCnt;
				*p++ = long_subIdsCnt;

				memcpy( p, buf, short_subIdsCnt );
				buf += short_subIdsCnt;			
				p += short_subIdsCnt;
				
				memcpy( p, buf, long_subIdsCnt );			
				buf += long_subIdsCnt;			
				p += long_subIdsCnt;			
			}
			len--;
		}
		
//		if( ( param.pld_ies_cnt == 0 ) 
//		&& (  param.hdr_ies_cnt == 0 )
//		&& ( param.type == EB_ACTIVE_SCAN ))
//		{
//			status =  MAC_INVALID_PARAMETER;
//                      goto exit;
//		}
	
        param.mpm_scan_duration_bpan = *buf++;
        param.mpm_scan_duration_nbpan = get_ushort(buf);
        
        buf += sizeof(ushort);
#ifdef MAC_CFG_SECURITY_ENABLED              
        if( param.type == ORPHAN_SCAN )
        {
			/* parse security fields */
            if(
				//(mac_pib.mac_security_enabled) &&
				( mac_data.security ) &&
				(( status = mac_prim_parse_security( &buf,&mac_data.security->coord_realign_sec_param,0 ) ) != MAC_SUCCESS)
			)
            {
                goto exit;
            }
        }
#else
		//init_sec_params(&mac_data.security->coord_realign_sec_param);
#endif
    }
    else
    {
        uint32_t page=0;
		//uint16_t len;
        PLME_get_request( phyCurrentPage, &len, &page );
        param.page = page;
    }

    /* Check if scan channels are valid */

    PLME_get_request( phySUNChannelsSupported, &len, (uint32_t*)&supported_channels );

/*TODO: check if the channel map requested is suported or not*/
//Amarjeet: Changes donoe for IAR porting

    /*indicate INVALID_PARAMETER when we receive MPMEB_PASSIVE_SCAN and the following conditions are met
    1) scan duration BPAN is <15 indicating that we are supposed to perform scanning on a BPAN 
    and if the NBPAN scan duration is less than 0x3fff
    2) scan duration BPAN is 15 and scan duration NBPAN is greater than 0x3FFF
    */
    if( ( param.type == MPM_EB_PASSIVE_SCAN ) && 
    ( ( (param.mpm_scan_duration_bpan == 15) && (param.mpm_scan_duration_nbpan > 0x3FFF)  )||
    (  (param.mpm_scan_duration_bpan < 15) && ((param.mpm_scan_duration_nbpan <= 0x3FFF)) ) ) )
    {
      /* invalid channels in scanChannels list, so report and leave */
      status = MAC_INVALID_PARAMETER;
      goto exit;
    }  

    /* start scanning */
    event.trigger = (sm_trigger_t)SCANSM_TRIGGER_SCAN_REQ;
    event.param.vector = &param;
    SM_DISPATCH( (sm_t*) &scansm, &event );

    return 1;

 exit:
    if( status != MAC_SUCCESS )
    {
        send_mlme_scan_confirm( status, param );
    }
    return 1;
}

#endif/*(CFG_MLME_SCAN_REQ_CONF==1)*/

/******************************************************************************/
/*processes the MCPS-DATA.request received from the NHL*/
uchar process_mcps_data_request(
                                           uchar *buf,  
                                           uint16_t length 
                                       )
{
    mac_status_t status = MAC_SUCCESS;
    mac_tx_t *dtx = NULL;
    mac_address_t src = {0}, dst = {0};
    uint16_t msdu_length = 0;
    uchar *msdu = NULL;
    uchar *t_msdu = NULL;
    uchar msdu_handle = 0;
    uchar tx_options = 0;
    security_params_t sec_params = {0};
    phy_data_params_t phy_params = {0};
    uchar fc_options = 0;
    //Raka ..
    //IE_ids_list_t* p_hdr_ie_ids = NULL;
    IE_ids_list_t* p_pld_ie_ids = NULL;
    uint16_t ie_field_len = 0;
    //uint16_t old_msdu_len = 0;
    uint16_t buf_len = 0;
    uchar* pld_ie = NULL; 
    uint32_t attr_value = 0;
    uint16_t len = 0;
    uchar *buf_end;
    uint32_t tempRakA = 0;

    buf_end = buf + length;

    if(mac_pib.PANId == 0xFFFF)
    {
      status = MAC_INVALID_REQUEST;
      goto exit;
    }
    /* Check if we are using 802.15.4-2006 primtives */
    if( mac_pib.ConfigFlags & ( USE_2006_PRIMITIVES|USE_2011_PRIMITIVES ) )
    {
        /* get the source address */
        src.address_mode = *buf++;
        src.pan_id = mac_pib.PANId;
        switch( src.address_mode )
        {
        case MAC_SHORT_ADDRESS:
            src.address.short_address = mac_pib.ShortAddress;
            break;

        case MAC_IEEE_ADDRESS:
            src.address.ieee_address = aExtendedAddress;
            break;

        case MAC_NO_ADDRESS:
#if (CFG_MAC_STARTSM_ENABLED == 1)          
            /* OK, if we are PAN coordinator */
            if( ( startsm_get_flags( startsm_p ) & STARTSM_FLAG_COORD_MASK ) == STARTSM_FLAG_PANCOORD )
            {
                break;
            }
            /* fallthrough to error reporting */
#endif
        default:
            status = MAC_INVALID_PARAMETER;
            goto exit;
        }
    }
    else /* 2003 Primitives */
    {
        /* get the source address */
        if( mac_prim_parse_address( &buf, &src ) != MAC_SUCCESS )
        {
            /* Invalid parameter if the address parsing fails */
            status = MAC_INVALID_PARAMETER;
            goto exit;
        }
    }

    /* destination address */
    if( mac_prim_parse_address( &buf, &dst ) != MAC_SUCCESS )
    {
        /* Invalid parameter if the address parsing fails */
        status = MAC_INVALID_PARAMETER;
        goto exit;
    }
    
	if(( src.address_mode == ADDR_MODE_NONE ) && ( dst.address_mode == ADDR_MODE_NONE ))
	{
		status = MAC_INVALID_ADDRESS;
                goto exit;
	}
	
    /* get MSDU length */
    /*old_msdu_len =*/ msdu_length = get_ushort(buf);

    if(!msdu_length)
    {
    	status = MAC_INVALID_PARAMETER;
        goto exit;
    }
    buf += 0x02;

    /* set the MSDU pointer */
    msdu = buf;

    /* step over the MSDU */
    buf += msdu_length;

    /* at least two more bytes to read */
    if( buf >= buf_end - 1 )
    {
        status = MAC_INVALID_PARAMETER;
        goto exit;
    }

    /* get msduHandle and TxOptions */
    msdu_handle = *buf++;
    tx_options = *buf++;
    
    phy_params.tx_channel = get_ushort(buf);

    buf += 0x02;

	phy_params.PPDUCoding = *buf++;
	phy_params.FCSLength = *buf++;
	phy_params.ModeSwitch = *buf++;
	phy_params.NewModeSUNPage = *buf++;
	phy_params.ModeSwitchParameterEntry = *buf++;
	
	PLME_get_request( phyFECEnabled, &len, &attr_value );

	if(((phy_params.PPDUCoding == FALSE) && (attr_value == 0x01)) ||
	((phy_params.PPDUCoding == TRUE) && (attr_value == 0x00)))
	{
		status = MAC_INVALID_PARAMETER;
                goto exit;
	}
	
	/* check if everything OK so far */
    if( status != MAC_SUCCESS )
    {
        goto exit;
    }

	/*skip the frame control field*/
	buf++;

	/*skip the IE lists if any*/
	if(*buf)
	{
              // Add Header IE Packet....
		buf += *buf;
	}

	buf++;

	if(*buf)
	{
           // Payload IE
          
          // Raka ..
	     
                p_pld_ie_ids = (IE_ids_list_t* )buf;
                
                p_pld_ie_ids->ie_flags = INCLUDE_TIE;
                
                buf += 0x04;//*buf;               
              
	              
        /*Check if header ie list has to be constructed*/
	if( ( p_pld_ie_ids != NULL ) && (p_pld_ie_ids->ie_list_len))
	{
		pld_ie = (uchar*)app_bm_alloc(50);
		if( pld_ie == NULL )
		{			
                        status = MAC_TRANSACTION_OVERFLOW;
                        goto exit;		
		}



                /*build the IEs with content as well*/
                  
                  // Raka :: to do need to find the code for putting payload IE,
                  // currently same path is taken which is used for header IE elemnet.
#ifdef WISUN_FAN_MAC          
                ie_field_len  =  build_ie_list
                                                 (
                                                        pld_ie,
                                                        IE_TYPE_PLD,
                                                        p_pld_ie_ids->ie_list_len, 
                                                        p_pld_ie_ids->ie_list,
                                                        p_pld_ie_ids->ie_flags 
                                                 );
#endif

		if( ie_field_len )
		{
			msdu_length += ie_field_len;
			//enc_offset += ie_field_len;
			/*ensure that the IE list present bit in the frame control is set*/ 
			fc_options |= MAC_IE_LIST_PRESENT;
			//tx_options |= FRAME_VERSION_2011;
		}
		else
		{
			/* free the buffer allocated for constructing the header IE list */		
			if( pld_ie != NULL )
			{
                                
				app_bm_free( pld_ie );/*free mem*/
				pld_ie = NULL;	
			}				

		}
	}
        buf_len = buf_end-msdu;
	/* allocate some memory */        
	//if((t_msdu = ((uchar*)app_bm_alloc(msdu_length)))==NULL)
        if((t_msdu = ((uchar*)app_bm_alloc(buf_len)))==NULL)
	{
          app_bm_free( pld_ie );/*free mem*/
          //data_tx_triggered_failed++;
	  return 0;
	}               
        //memcpy(t_msdu, msdu, old_msdu_len);
        memcpy(t_msdu, msdu, buf_len);
        memcpy(msdu, pld_ie, ie_field_len);
        memcpy(msdu+ie_field_len, t_msdu, buf_len);    
        buf +=  ie_field_len;
       
        app_bm_free(t_msdu);/*free mem*/
        app_bm_free(pld_ie);/*free mem*/
        }
        
#ifdef PANA_FOR_HAN_NWK
        
        if (force_ie_list_in_frame_control == 1)
        {
            // Raka :: this flag has to be removed with proper code flow for relay device.
            //We need to enable IE list present in frame control filed for realy device without
            // putting IE elemnt data 
            force_ie_list_in_frame_control = 0;
            fc_options |= MAC_IE_LIST_PRESENT;

        }        
        
#endif        

	/*Increment by 1 byte after skipping Ies and then skip 1 byte multipurpose field*/
	buf+=2;

    /* Set frame version if packet length is to long for 802.15.4-2003*/
    if( msdu_length > aMaxMACFrameSize )
    {
        tx_options |= FRAME_VERSION_2006;
    }

    
#ifdef MAC_CFG_SECURITY_ENABLED
    /* parse security fields */
    //if(mac_pib.mac_security_enabled == TRUE)
    {
        //if( ( status = mac_prim_parse_security( &buf, &sec_params, 0 ) ) != MAC_SUCCESS )
        if( ( status = mac_prim_parse_security_for_data( &buf, &sec_params, 0 ) ) != MAC_SUCCESS )
        {
            goto exit;
        }
    }
#else
	init_sec_params(&sec_params);
#endif
    /* verify addresses */
    if( ((src.address_mode == MAC_SHORT_ADDRESS) && (src.address.short_address == USE_IEEE_ADDRESS))
        || ((dst.address_mode == MAC_SHORT_ADDRESS) && (dst.address.short_address == USE_IEEE_ADDRESS))
        || ((dst.address_mode == MAC_SHORT_ADDRESS) && (dst.address.short_address == BROADCAST_SHORT_ADDRESS)
            && (
#if ( ( MAC_CFG_LE_RIT_CAPABILITY == 1 ) || ( MAC_CFG_LE_CSL_CAPABILITY == 1 ) )
				((tx_options & ACKNOWLEDGED_TRANSMISSION) && ( low_energy_get_state(&low_energy) == LE_STATE_INIT ))
#else
				(tx_options & ACKNOWLEDGED_TRANSMISSION)
#endif            
            
#if ( ( MAC_CFG_LE_RIT_CAPABILITY == 1 ) || ( MAC_CFG_LE_CSL_CAPABILITY == 1 ) )
            /* When RIT is enabled, broadcast is not supported. request can be 
            issued as if it is supposed to be sent as broadcast to all PANs, 
            which eventually be sent as a unicast frame*/
			||((low_energy_get_state(&low_energy) != LE_STATE_INIT) && (dst.pan_id != BROADCAST_PAN_ID))
#endif            
            ))

#if ( ( MAC_CFG_LE_RIT_CAPABILITY == 1 ) || ( MAC_CFG_LE_CSL_CAPABILITY == 1 ) )
		/*When RIT is enabled, discard MCPS-DATA.request with other than "indirect" tx options */
		|| (low_energy_get_state(&low_energy) != LE_STATE_INIT) && ( !(tx_options & INDIRECT_TRANSMISSION) )
#endif                        
#ifndef MAC_CFG_GTS_ENABLED
        || ((tx_options & GTS_TRANSMISSION) != 0)
#endif
      )
    {
        status = MAC_INVALID_PARAMETER;
        goto exit;
    }
	
#if defined( MAC_CFG_FFD )

#if (CFG_MAC_STARTSM_ENABLED == 1)       
	 /* check if we are the PAN coordinator and trying to send data to the PAN coordinator! */
	if((( startsm_get_flags( startsm_p ) & STARTSM_FLAG_COORD_MASK ) == STARTSM_FLAG_PANCOORD) &&          
	 dst.address_mode == MAC_NO_ADDRESS)
	{
		status = MAC_INVALID_PARAMETER;
                goto exit;
	} 
#endif
    
#endif	/*MAC_CFG_RFD*/

    /* create the packet to transmit */				   
		status = mac_frame_build( &dtx,//
					MAC_FRAME_TYPE_DATA,
					MAC_FRAME_BCN_SUB_TYPE_DONT_CARE,
					&src,//
					&dst,//
					tx_options,//
					fc_options,//fc_options,
					msdu,//
					msdu_length,//
					0,//enc_offset,//
					//(uint32_t) p_hdr_ie_ids,//  // Raka TO DO  [ 19 Sept 2022 ]
                                        tempRakA,
					&phy_params,//
					&sec_params,0 );
			     

    /* There was an error creating the packet, so report it */
    if( dtx == NULL_POINTER )
    {
        goto exit;
    }

    /* set the handle and the length */
    dtx->msdu_handle = msdu_handle;

    /*  7.1.1.1.3 para 5:
     *  GTS flag has priority over indirect.
     *  We only send indirectly if we're a coordinator. */
#ifdef MAC_CFG_GTS_ENABLED
    if( tx_options & GTS_TRANSMISSION )
    {
        status = mac_add_gts_transmission( &dst, dtx );
    }
    else
#endif

 
    /*TBD  How to determine which superframe (TRXSM) the primitive belongs to? */
    /* check if indirect transmission and acting as a coordinator */
    if( tx_options & INDIRECT_TRANSMISSION  )
    {
#if ( ( MAC_CFG_LE_RIT_CAPABILITY == 1 ) || ( MAC_CFG_LE_CSL_CAPABILITY == 1 ) )
		/*During this period (macRITPeriod equals to zero), all transactions shall be 
		handled as those of normal non beacon-enabled PAN (macRxOnWhenIdle: False). 
		RIT is ON or OFF, if LE is enabled, put it into indirect queue to be pulled 
		by dest nodes */
		if( mac_pib.LEenabled )
		{
			status = mac_add_indirect_transmission( &src, &dst, dtx );
			goto exit;
		}else
#endif
#if (CFG_MAC_STARTSM_ENABLED == 1)                  
		if( ( startsm_get_flags( startsm_p ) & STARTSM_FLAG_COORD_MASK ) )
		{
        
    		if( dst.address_mode == MAC_SHORT_ADDRESS &&
				dst.address.short_address == BROADCAST_SHORT_ADDRESS )
			{
				status = mac_queue_bcast_transmission( dtx );
			}
			else
			{
				status = mac_add_indirect_transmission( &src, &dst, dtx );
			}
		}
		else
		{
			status = mac_queue_direct_transmission( dtx );
		}
#else
                        status = mac_queue_direct_transmission( dtx );
#endif                
    }
    else
    {
      if( dst.address_mode == MAC_NO_ADDRESS &&
              dst.address.short_address == BROADCAST_SHORT_ADDRESS )
      {
              status = mac_queue_bcast_transmission( dtx );
      }
      else
      {
              status = mac_queue_direct_transmission( dtx );
      }
    }

 exit:
    if( status != MAC_SUCCESS )
    {
        if( dtx != NULL_POINTER )
        {
            mac_mem_free_tx( dtx );/*free mem*/
        }
        send_mcps_data_confirm( msdu_handle, status, 0 ,0);
    }
    return 1;
}

/******************************************************************************/

/* processes the MCPS-PURGE.request received from the NHL.Purges the DATA message specified by the handle*/
#if(CFG_MCPS_PURGE_REQ_CONF == 1)
static uchar process_mcps_purge_request(
                                        uchar *buf, 
                                        uint16_t length 
                                 )
{
    
    uchar status = MAC_SUCCESS;
    /* get the handle */
    uchar msdu_handle = *buf;

#if(CFG_MLME_PURGE_REQ_CONF == 0)
	/*validation to see if MLME-START is disabled. RFD does not support MLME-START*/
	status = MAC_INVALID_REQUEST;
#endif	/*	CFG_MLME_PURGE_REQ_CONF*/    

    /* now attempt to find the transaction in the direct, gts and indirect lists */
    status = mac_purge_direct_transmission( MAC_FRAME_TYPE_DATA, msdu_handle );

#ifdef MAC_CFG_GTS_ENABLED
    if( status != MAC_SUCCESS )
    {
        status = mac_purge_gts_transmission( msdu_handle );
    }
#endif

    if( status != MAC_SUCCESS )
    {
        status = mac_purge_indirect_transmission( MAC_FRAME_TYPE_DATA, msdu_handle );
    }
    
    send_mcps_purge_confirm( msdu_handle, status );
    return 1;
}
#endif	/*(CFG_MCPS_PURGE_REQ_CONF == 1)*/

/******************************************************************************/
#if(CFG_MLME_ASSOCIATE_REQ_CONF == 1)
/*processes the MLME-ASSOCIATE.request received from the NHL*/
static uchar process_mlme_associate_request(
												uchar *buf, /* buffer to process */
												uint16_t length /* length of data in buffer */
                                            )
{
    mac_status_t status = MAC_SUCCESS;
    mac_tx_t *dtx = NULL;
    mac_address_t dst = {0};
    uchar capability_info = 0;
    uint32_t logical_channel = 0;
    uint32_t channel_page = 0;
    security_params_t sec_params = {0};
    phy_data_params_t phy_params = {0};

	uint8_t adv_prim_2011 = mac_pib.ConfigFlags & ( USE_2006_PRIMITIVES|USE_2011_PRIMITIVES);
    //status = MAC_SUCCESS;

    /* get (and save) the logical channel */
    logical_channel = *buf++;
    phy_params.tx_channel = logical_channel;

    /* Check if we are using 802.15.4-2006 primitives */
    if( adv_prim_2011 )
    {
        channel_page = (*buf++ & 0x1F);
    }

    /* get the address of the coordinator to associate with */
    if( ( status = mac_prim_parse_address( &buf, &dst ) ) != MAC_SUCCESS )
    {
        /* Invalid parameter if the address parsing fails */
        status = MAC_INVALID_PARAMETER;
        goto exit;
    }

    capability_info = *buf++;

#define CAPABILITY_INFO_RESERVED_BITS 0x30 /* 2 reserved bits */

    if( capability_info & CAPABILITY_INFO_RESERVED_BITS )
    {
        status = MAC_INVALID_PARAMETER;
        goto exit;
    }

    /*skip the low latency(1byte), channel offset(2 bytes), hopping seq id(1 byte)*/
    buf += 0x04;

#ifdef MAC_CFG_SECURITY_ENABLED
    /* parse security fields */
    if( ( status = mac_prim_parse_security( &buf, &sec_params, 1 ) ) != MAC_SUCCESS )
    {
        goto exit;
    }
#else
	init_sec_params(&sec_params);
#endif
    /* select correct channel page */
    if( ( adv_prim_2011 )  &&
        PLME_set_request( phyCurrentPage, 1, &channel_page ) != PHY_SUCCESS )
    {
        status = MAC_INVALID_PARAMETER;
        goto exit;
    }

    /* try switching to the requested channel */
    /*TBD  Should be done with TRX-SM */
    if( PLME_set_request( phyCurrentChannel, 2, &logical_channel ) != PHY_SUCCESS )
    {
        status = MAC_INVALID_PARAMETER;
        goto exit;
    }

    /* validate coordinator addressing mode */
    if( ( (dst.address_mode == MAC_SHORT_ADDRESS )
          && ( ( dst.address.short_address == USE_IEEE_ADDRESS )
               || ( dst.address.short_address == 0xffff ) ) )
        || ( ( dst.address_mode != MAC_SHORT_ADDRESS )
             && ( dst.address_mode != MAC_IEEE_ADDRESS ) ) )
    {
        /* invalid coordinator address */
        status = MAC_INVALID_PARAMETER;
        goto exit;
    }

    /*TBD  Do we really have to update macRxOnWhenIdle? */
    mac_pib.RxOnWhenIdle = capability_info & RX_ON_WHEN_IDLE ? 1 : 0;

    /* update PIB (see 15.4-2006 7.5.3.1) */
    mac_pib.PANId = dst.pan_id;
    if (dst.address_mode == MAC_SHORT_ADDRESS)
    {
        mac_pib.CoordShortAddress = dst.address.short_address;
    }
    else if (dst.address_mode == MAC_IEEE_ADDRESS)
    {
        mac_pib.CoordShortAddress = USE_IEEE_ADDRESS;
        memcpy( mac_pib.CoordExtendedAddress, dst.address.ieee_address, 8 );
    }

    /* build packet */
    status = mac_frame_build_association_request( &dtx,
                                                  capability_info,
                                                  &dst,
                                                  &phy_params,
                                                  &sec_params );

    /* queue packet for transmission */
    if( dtx != NULL_POINTER )
    {
        status = mac_queue_direct_transmission( dtx );
    }

 exit:
    if( status != MAC_SUCCESS )
    {
	    mac_pib.RxOnWhenIdle = 0x00;
	    mac_pib.PANId = DEFAULT_PAN_ID;
            send_mlme_associate_confirm( INVALID_SHORT_ADDRESS, status, NULL_POINTER );
    }
    return 1;
}
#endif	/*(CFG_MLME_ASSOCIATE_REQ_CONF == 1)*/

/******************************************************************************/
#if(CFG_MLME_ASSOCIATE_IND_RESP == 1)
/*processes the MLME-ASSOCIATE.response primitive received from the NHL*/
static uchar process_mlme_associate_response(
                                                   uchar *buf,  
                                                   uint16_t length 
                                             )
{
    mac_status_t status = MAC_SUCCESS;
    mac_tx_t *dtx = NULL;
    mac_address_t src = {0}, dst = {0};
    uchar *ieee_addr = NULL, *short_addr = NULL, assoc_status = 0;
    security_params_t sec_params = {0};

    /* parse primitive */
    ieee_addr = buf;
    buf += 8;
    short_addr = buf;
    buf += 2;
    assoc_status = *buf++;

    /* set destination address */
    dst.address_mode = MAC_IEEE_ADDRESS;
    /*TBD  Make sure startsm_p is not NULL_POINTER */
#if (CFG_MAC_STARTSM_ENABLED == 1)      
    dst.pan_id = startsm_p->pending.pan_id;
#endif    
    dst.address.ieee_address = ieee_addr;

    /* set source addresses */
    src.address_mode = MAC_IEEE_ADDRESS;
    src.pan_id = dst.pan_id;
    src.address.ieee_address = aExtendedAddress;

    /*skip the low latency info(1), channel offset(2), hopping seq len(1),hopping seq(var)*/
    buf += 0x06;
#ifdef MAC_CFG_SECURITY_ENABLED
    /* parse security fields */
    if( ( status = mac_prim_parse_security( &buf, &sec_params, 1 ) ) != MAC_SUCCESS )
    {
        goto exit;
    }
#else
	init_sec_params(&sec_params);
#endif

#define PAN_AT_CAPACITY   1
#define PAN_ACCESS_DENIED 2

    /* check for invalid status */
    if( assoc_status > PAN_ACCESS_DENIED )
    {
        status = MAC_INVALID_PARAMETER;
        goto exit;
    }

    /* check for invalid short address */
    if ( ((assoc_status == PAN_ACCESS_DENIED) || (assoc_status == PAN_AT_CAPACITY)) &&
         ((short_addr[0] != 0xff) || (short_addr[1] != 0xff)) )
    {
        status = MAC_INVALID_PARAMETER;
        goto exit;
    }

    /* build packet */
    status = mac_frame_build_association_response( &dtx,
                                                   &src,
                                                   &dst,
                                                   short_addr,
                                                   assoc_status,
                                                   &sec_params );

    /* queue packet for transmission */
    if( dtx != NULL_POINTER )
    {
        dtx->msdu_handle = 0;
        status = mac_add_indirect_transmission( &src, &dst, dtx);
    }

 exit:
    if( status != MAC_SUCCESS )
    {
        send_mlme_comm_status_indication( &src, &dst, status, &sec_params );
    }
    return 1;
}
#endif	/*(CFG_MLME_ASSOCIATE_IND_RESP == 1)*/

/******************************************************************************/
#if(CFG_MLME_DISASSOCIATE_REQ_CONF == 1)
/*processes the MLME-DISASSOCIATE.request primitive received from the NHL*/
static uchar process_mlme_disassociate_request(
													uchar *buf, 
													uint16_t length 
                                              )
{
    mac_status_t status = MAC_SUCCESS;
    mac_tx_t *dtx = NULL;
    mac_address_t src = {0}, dst = {0};
#if(CFG_ASK_CHILD_TO_DISASSOCIATE == 1)
    uchar indirect = 0;
#endif	/*(CFG_ASK_CHILD_TO_DISASSOCIATE == 1)*/
    uchar reason = 0;
    uchar tx_indirect = 0;
    security_params_t sec_params = {0}; 
    uint8_t dest_address = 0;
    uint8_t adv_prim_2011 = mac_pib.ConfigFlags & ( USE_2006_PRIMITIVES|USE_2011_PRIMITIVES);
    if( adv_prim_2011 )
    {
        /* get the address of device to disassociate */
		status = mac_prim_parse_address( &buf, &dst );
        if( ( status != MAC_SUCCESS ) && (dst.pan_id == PAN_ID_UNKNOWN || dst.pan_id != mac_pib.PANId))
        {
            /* Invalid parameter if the address parsing fails */
            status = MAC_INVALID_PARAMETER;
            goto exit;
        }

		/* check PAN Id 
        if( dst.pan_id == PAN_ID_UNKNOWN || dst.pan_id != mac_pib.PANId )
        {
            status = MAC_INVALID_PARAMETER;
            goto exit;
        }*/

        /* disassociation reason */
        reason = *buf++;
     
        /* indirect flag */
        tx_indirect = *buf++;
    }
    else
    {
        /* IEEE destination address */
        dst.address_mode = MAC_IEEE_ADDRESS;
        dst.pan_id = mac_pib.PANId;
        dst.address.ieee_address = buf;
        buf += 8;

        /* disassociation reason */
        reason = *buf++;

        /* indirect flag */
        tx_indirect = 0;
    }
    
    /* validate disassociate reason */
#if(CFG_ASK_CHILD_TO_DISASSOCIATE == 1)
    if( reason == 0 || reason > DEV_WISHES_TO_LEAVE )
#else
	if( reason != DEV_WISHES_TO_LEAVE  )
#endif	/*(CFG_ASK_CHILD_TO_DISASSOCIATE == 1)*/
    {
        status = MAC_INVALID_PARAMETER;
        goto exit;
    }
    
    if( ( ( dst.address_mode == MAC_SHORT_ADDRESS )
              && ( dst.address.short_address == mac_pib.CoordShortAddress ) )
            || ( ( dst.address_mode == MAC_IEEE_ADDRESS )
                 && ( ieeeaddr_cmp( dst.address.ieee_address, mac_pib.CoordExtendedAddress ) == 0 ) ) )
    {
    	dest_address = PARENT_DEST;
    }
	else
	{
#if(CFG_ASK_CHILD_TO_DISASSOCIATE == 1)
		dest_address = NON_PARENT_DEST;
#else
		status = MAC_INVALID_PARAMETER;
    	goto exit;
#endif	/*(CFG_ASK_CHILD_TO_DISASSOCIATE == 1)*/
	}

#if(CFG_ASK_CHILD_TO_DISASSOCIATE == 1)
    
    /* When Device wishes to leave network it should send request only to its Coordinator and
    when Coord is asking any of its device to leave network dissoc reason should be 0x01 only*/        
    if ( (( reason == DEV_WISHES_TO_LEAVE ) && (dest_address == NON_PARENT_DEST)) 
     	|| (( reason == COORD_WISHES_DEV_TO_LEAVE)&&(dest_address == PARENT_DEST)))
    {                           
    	status = MAC_INVALID_PARAMETER;
    	goto exit;	
    
    }
#endif	/*(CFG_ASK_CHILD_TO_DISASSOCIATE == 1)*/

#ifdef MAC_CFG_SECURITY_ENABLED
    /* parse security fields */
    if( ( status = mac_prim_parse_security( &buf, &sec_params, 1 ) ) != MAC_SUCCESS )
    {
        goto exit;
    }
#else
	init_sec_params(&sec_params);
#endif

    if( adv_prim_2011 )
    {
        /* check PAN Id */
        if( dst.pan_id == PAN_ID_UNKNOWN || dst.pan_id != mac_pib.PANId )
        {
            status = MAC_INVALID_PARAMETER;
            goto exit;
        }

    }

#if(CFG_ASK_CHILD_TO_DISASSOCIATE == 1)   
    if( dest_address == PARENT_DEST )
    {
        /* this is for our coordinator, so send direct */
        indirect = 0;
    }
#if (CFG_MAC_STARTSM_ENABLED == 1)    
    else if( startsm_get_flags( startsm_p ) & STARTSM_FLAG_COORD )
    {
        
    	if( adv_prim_2011 )
	    {
	        indirect = tx_indirect;	    	
	    }
	    else
	    {
	    	/* In 2003, if we are a coordinator, send indirect */
	    	indirect = 1;	
	    }    
    }
#else
                indirect = 1;
#endif    
    else
    {
        status = MAC_INVALID_PARAMETER;
        goto exit;
    }
#endif	/*(CFG_ASK_CHILD_TO_DISASSOCIATE == 1)*/
    

    /* build packet */
    status = mac_frame_build_disassociation_notification( &dtx,
                                                          &src,
                                                          &dst,
                                                          reason,
                                                          &sec_params );
    /* queue packet for transmission */
    if( dtx != NULL_POINTER )
    {
#if(CFG_ASK_CHILD_TO_DISASSOCIATE == 1)        
		/* check if this is a message for a device, not for my coordinator */
        if( indirect )
        {
			dtx->msdu_handle = 0;
            /* this message should be pended, to be extracted by the device */
            status = mac_add_indirect_transmission( &src, &dst, dtx);
        }
        else
#endif	/*(CFG_ASK_CHILD_TO_DISASSOCIATE == 1)*/
        {
            /* transmit it now, direct to the device */
            status = mac_queue_direct_transmission( dtx );
        }
    }

 exit:
    if( status != MAC_SUCCESS )
    {
        send_mlme_disassociate_confirm( status, &dst );
    }
    return 1;
}
#endif	/*(CFG_MLME_DISASSOCIATE_REQ_CONF == 1)*/

/******************************************************************************/

/*processes the MLME-GTS.request primitive received from the NHL*/
#ifdef MAC_CFG_GTS_ENABLED
static uchar process_mlme_gts_request(
										  uchar *buf,  
										  uint16_t length 
                                      )
{
    uchar status;

    /* be optimistic */
    status = MAC_SUCCESS;

    /* only accept a GTS request if we have a short address */
    if( (mac_pib.ShortAddress == SHORT_ADDRESS_UNKNOWN )
        || ( mac_pib.ShortAddress == USE_IEEE_ADDRESS ))
    {
        status = NO_SHORT_ADDRESS;
    }
    /* test if we are the coordinator */
    else if( mac_data.coordinator)
    {
        status = DENIED;
    }
    /* we are a slave, so create a GTS request whether or not GTS permit is set */
    else  /* spec does not describe this so assume we send it anyway */
#if USE_GTS_PERMIT_VALUE
        if( mac_pib.GTSPermit )
#endif
        {
            /* send a GTS request to the coordinator (buf is pointing at the gts characteristics) */
            status = mac_create_gts_request( buf );
        }

        if( status != MAC_SUCCESS )
        {
            send_mlme_gts_confirm(  buf[0], status );
        }
        return 1;
} /* end of process_mlme_gts_request */
#endif

/******************************************************************************/
#ifndef MAC_CFG_SECURITY_ENABLED
static void init_sec_params(security_params_t *sec_params_t)
{
	sec_params_t->security_level = 0;
	sec_params_t->key_id_mode = 0;
}
#endif
/******************************************************************************/


#ifdef WISUN_FAN_MAC

uchar process_ws_async_frame_request( uchar *buf, uint16_t length )
{ 
//    sm_event_t event;
    uint8_t ws_async_frame_type =0xFF;

    if(*buf++)  /* operation */
    {
#if(CFG_MAC_SFTSM_ENABLED == 1)        
      event.trigger = (sm_trigger_t) SCANSM_TRIGGER_CANCEL;	
      SM_DISPATCH( (sm_t*) startsm_p, &event );
#endif      
      return 1;
    }

    ws_async_frame_type = (*buf++ );  
    switch(ws_async_frame_type)
    {
      case PAN_ADVERT_SOL: 
          ws_send_pkt(ws_async_frame_type,buf,usable_channel.total_usable_ch_unicast);
          break;

      case PAN_ADVERT:
          ws_send_pkt(ws_async_frame_type,buf,usable_channel.total_usable_ch_unicast);
          break;

      case PAN_CONFIG_SOL:
          ws_send_pkt(ws_async_frame_type,buf,usable_channel.total_usable_ch_unicast);
          break;

      case PAN_CONF:
          ws_send_pkt(ws_async_frame_type,buf,usable_channel.total_usable_ch_unicast);
          break;

      default:
          break;     
    } 

    return 1;
}
/******************************************************************************/

#if(FAN_EAPOL_FEATURE_ENABLED == 1)

uchar process_eapol_frame_request(uchar *buf, uint16_t length)
{
//#if (APP_LBR_ROUTER == 1) 
//#if !(WITHOUT_EAPOL)
//  if ((gtk_hash_need_to_be_updated () == 1) && (is_key_request_triggered () == 0))
//    send_eapol_request();
//#endif
//  return 1;
//#endif
  return 1;
}     //Debdeep

#endif // #if(FAN_EAPOL_FEATURE_ENABLED == 1)


#endif

/******************************************************************************/
#ifdef WISUN_FAN_MAC
/*processes the MCPS-DATA.request received from the NHL*/
uchar process_fan_mcps_data_request(
                                    uchar *buf,  
                                    uint16_t length 
                                      )
{
  
  mac_status_t status = MAC_SUCCESS;
  mac_tx_t *dtx = NULL;
  mac_address_t src = {0}, dst = {0};
  uint16_t msdu_length = 0;
  uchar *msdu = NULL;
  uchar msdu_handle = 0;
  uchar tx_options = 0;
  security_params_t sec_params = {0};
  memset(&sec_params,0,sizeof(sec_params));
  phy_data_params_t phy_params = {0};
  uchar fc_options = 0;
  uint32_t sub_hdr_bitmap = 0;
  uint32_t sub_pld_bitmap = 0;
  struct mpx_data_st mpx_data;
  uint32_t ie_mask;
  uint16_t ie_field_len = 0;
  uint16_t total_allocated_len = 0;
  uchar* pld_ie = NULL; 
  uint32_t attr_value = 0;
  uint16_t len = 0;
  uchar *buf_end = buf + length;
  uint16_t multip_id = fan_mac_params.multiplex_id;
  uint8_t trensf_id = fan_mac_params.transfer_type;
  uint8_t kmp_id  = fan_mac_params.KMP_ID;
  
  memset ((uint8_t*)&sec_params, 0x00, sizeof(security_params_t));
  
  if(mac_pib.PANId == 0xFFFF)
  {
    status = MAC_INVALID_REQUEST;
    goto exit;
  }
  
  /* Check if we are using 802.15.4-2006 primtives */
  if( mac_pib.ConfigFlags & ( USE_2006_PRIMITIVES|USE_2011_PRIMITIVES ) )
  {
    /* get the source address */
    src.address_mode = *buf++;
    src.pan_id = mac_pib.PANId;
    switch( src.address_mode )
    {
    case MAC_SHORT_ADDRESS:
      src.address.short_address = mac_pib.ShortAddress;
      break;
    case MAC_IEEE_ADDRESS:
      src.address.ieee_address = aExtendedAddress;
      break;
    case MAC_NO_ADDRESS:
      /* OK, if we are PAN coordinator */
#if (CFG_MAC_STARTSM_ENABLED == 1)          
      if ((startsm_get_flags (startsm_p) & STARTSM_FLAG_COORD_MASK) == 
          STARTSM_FLAG_PANCOORD)
        break;
#endif
      break;
      /* fall through error reporting */
    default:
      status = MAC_INVALID_PARAMETER;
      goto exit;
    }
  }
  else /* 2003 Primitives */
  {
    /* get the source address */
    if( mac_prim_parse_address( &buf, &src ) != MAC_SUCCESS )
    {
      /* Invalid parameter if the address parsing fails */
      status = MAC_INVALID_PARAMETER;
      goto exit;
    }
  }
  
  /* destination address */
  if (mac_prim_parse_address (&buf, &dst) != MAC_SUCCESS )
  {
    /* Invalid parameter if the address parsing fails */
    status = MAC_INVALID_PARAMETER;
    goto exit;
  }
  
  if ((src.address_mode == ADDR_MODE_NONE) && 
      (dst.address_mode == ADDR_MODE_NONE))
  {
    status = MAC_INVALID_ADDRESS;
    goto exit;
  }
  
  /* get MSDU length */
  msdu_length = get_ushort(buf);
  if(!msdu_length)
  {
    status = MAC_INVALID_PARAMETER;
    goto exit;
  }
  
  /* Increment by 2 because msdulen is of 2 bytes*/
  buf += 0x02;
  
  /* set the MSDU pointer */
  msdu = buf;
  
  /* step over the MSDU */
  buf += msdu_length;
  
  /* at least two more bytes to read */
  if( buf >= buf_end - 1 )
  {
    status = MAC_INVALID_PARAMETER;
    goto exit;
  }
  
  /* get msduHandle and TxOptions */
  msdu_handle = *buf++;
  tx_options = *buf++;
  
  phy_params.tx_channel = get_ushort(buf);
  /* Increment by 2 because tx_channel is of 2 bytes*/
  buf += 0x02;
  
  /*Raka :: Extract the PHY parameters [29-05-2017]*/
  phy_params.PPDUCoding = *buf++;
  phy_params.FCSLength = *buf++;
  phy_params.ModeSwitch = *buf++;
  phy_params.NewModeSUNPage = *buf++;
  phy_params.ModeSwitchParameterEntry = *buf++;
  PLME_get_request (phyFECEnabled, &len, &attr_value);
  
  if (((phy_params.PPDUCoding == FALSE) && (attr_value == 0x01)) ||
     ((phy_params.PPDUCoding == TRUE) && (attr_value == 0x00)))
  {
    status = MAC_INVALID_PARAMETER;
    goto exit;
  }
  
  /* check if everything OK so far */
  if( status != MAC_SUCCESS )
    goto exit;
  
  /*skip the frame control field [ Raka :: 29-05-2017 {Why are we not using frame contol option}]*/
  buf++;
  /*skip the IE lists if any*/
#if (APP_LBR_ROUTER == 1)
    memcpy ((uint8_t *)&sub_hdr_bitmap, buf, sizeof (sub_hdr_bitmap));
    buf += sizeof (sub_hdr_bitmap);
    memcpy ((uint8_t *)&sub_pld_bitmap, buf, sizeof (sub_pld_bitmap));
    buf += sizeof (sub_pld_bitmap);
    
    if (sub_pld_bitmap != NO_IE_MASK)
    {
      uint8_t ii;
      uint8_t sub_pld_ie_count = 0;
      
      for (ii = 0; ii < 32; ii++)
      {
        if (sub_pld_bitmap & (1 << ii))
          sub_pld_ie_count++;
      }
      
      ie_mask = WP_IE_MASK | MPX_IE_MASK | PAYLOAD_TIE_MASK;
      total_allocated_len = (sub_pld_ie_count * AVG_MAX_PAYLOAD_SIZE) +
        msdu_length + AVG_MAX_PAYLOAD_SIZE;
    }
    else
    {
      ie_mask = MPX_IE_MASK | PAYLOAD_TIE_MASK;
      total_allocated_len = msdu_length + AVG_MAX_PAYLOAD_SIZE;
    }
    
    pld_ie = (uchar *)app_bm_alloc (total_allocated_len);
    if( pld_ie == NULL )
    {		
#if ((PRINT_DEBUG_LEVEL==ERROR_DEBUG)||(PRINT_DEBUG_LEVEL==CONSOLE_ERROR_DEBUG))
      stack_print_debug ("### Malloc fail for PLD IE\n");
#endif
      status = MAC_TRANSACTION_OVERFLOW;
      goto exit;		
    }
    
    mpx_data.msdu = msdu;
    mpx_data.msdu_len = msdu_length;
    mpx_data.multiplex_id = multip_id;
    mpx_data.transfer_type = trensf_id;
    mpx_data.kmp_id = kmp_id;
    
    ie_field_len = build_ie (pld_ie, IE_TYPE_PLD, ie_mask, sub_pld_bitmap, 
                             &mpx_data);
    
    if (ie_field_len)
    {
      msdu_length = ie_field_len;
      
      /*ensure that the IE list present bit in the frame control is set*/ 
      /*chack here for sequence number  is this EDFE or DFE Pkt for unicast 
      frame type MUST be set to 0 For dfe
      must be set for 1 For EDFE  and for broadcast 
      sequence number bradcast is set to zero*/
#if(FAN_EDFE_FEATURE_ENABLED == 1)
      if (( edfe_information.edfe_frame_enabled  == 0x01)
          &&(edfe_information.edfe_frame_tx_type != 0x99)
            && (dst.address_mode == ADDR_MODE_EXTENDED))
      {
        fc_options |= MAC_SEQ_NUM_SUPPRESSION; 
      }
#endif
    }
    else
    {
      /*Else we should drop the packet :: Raka  [29-05-2017]*/
#if ((PRINT_DEBUG_LEVEL==ERROR_DEBUG)||(PRINT_DEBUG_LEVEL==CONSOLE_ERROR_DEBUG))
      stack_print_debug ("### Build IE failure\n");
#endif
      app_bm_free(pld_ie);      //Debdeep
      pld_ie = NULL;            //Debdeep
      status = MAC_NO_DATA;
      goto exit;
    }
#else
      pld_ie = (uchar *)app_bm_alloc (msdu_length);
      memcpy(pld_ie,msdu,msdu_length);
#endif
  /*Skip 1 byte for multipurpose field*/
  buf += 1;
  
  /* Set frame version if packet length is to long for 802.15.4-2003*/
  if( msdu_length > aMaxMACFrameSize )
    tx_options |= FRAME_VERSION_2006;  
  
#ifdef MAC_CFG_SECURITY_ENABLED
  /* parse security fields */
  if(mac_pib.mac_security_enabled == TRUE)
  {
    if ((status = mac_prim_parse_security (&buf, &sec_params, 0)) != MAC_SUCCESS)
      goto exit;
  }
#else
  init_sec_params (&sec_params);
#endif
  
  /* verify addresses */
  if (((src.address_mode == MAC_SHORT_ADDRESS) && (src.address.short_address == USE_IEEE_ADDRESS))
     || ((dst.address_mode == MAC_SHORT_ADDRESS) && (dst.address.short_address == USE_IEEE_ADDRESS))
       || ((dst.address_mode == MAC_SHORT_ADDRESS) && (dst.address.short_address == BROADCAST_SHORT_ADDRESS)
           && (
#if ((MAC_CFG_LE_RIT_CAPABILITY == 1) || (MAC_CFG_LE_CSL_CAPABILITY == 1))
               ((tx_options & ACKNOWLEDGED_TRANSMISSION) && (low_energy_get_state (&low_energy) == LE_STATE_INIT))
#else
                 (tx_options & ACKNOWLEDGED_TRANSMISSION)
#endif                    
#if ((MAC_CFG_LE_RIT_CAPABILITY == 1) || (MAC_CFG_LE_CSL_CAPABILITY == 1))
                   /* When RIT is enabled, broadcast is not supported. request can be 
                   issued as if it is supposed to be sent as broadcast to all PANs, 
                   which eventually be sent as a unicast frame*/
                   || ((low_energy_get_state (&low_energy) != LE_STATE_INIT) && (dst.pan_id != BROADCAST_PAN_ID))
#endif            
                     ))
         
#if ((MAC_CFG_LE_RIT_CAPABILITY == 1) || (MAC_CFG_LE_CSL_CAPABILITY == 1))
         /*When RIT is enabled, discard MCPS-DATA.request with other than "indirect" tx options */
         || (low_energy_get_state (&low_energy) != LE_STATE_INIT) && (!(tx_options & INDIRECT_TRANSMISSION))
#endif                        
#ifndef MAC_CFG_GTS_ENABLED
           || ((tx_options & GTS_TRANSMISSION) != 0)
#endif
             )
  {
    status = MAC_INVALID_PARAMETER;
    goto exit;
  }
  
#if defined( MAC_CFG_FFD )
  /* check if we are the PAN coordinator and trying to send data to the PAN coordinator! */
   
#if (CFG_MAC_STARTSM_ENABLED == 1)           
   if(((startsm_get_flags (startsm_p ) & STARTSM_FLAG_COORD_MASK) == STARTSM_FLAG_PANCOORD) 
      && (dst.address_mode == MAC_NO_ADDRESS))
  {
    status = MAC_INVALID_PARAMETER;
    goto exit;
  }
#endif
  
#endif	/*MAC_CFG_RFD*/
  
  /* create the packet to transmit */				   
  status = mac_frame_build (&dtx,
                            MAC_FRAME_TYPE_DATA,
                            fan_mac_params.type,
                            &src,
                            &dst,
                            tx_options,
                            fc_options,
                            pld_ie,
                            msdu_length,
                            0,                   /*enc_offset*/
                            sub_hdr_bitmap,
                            &phy_params,
                            &sec_params,
                            0);
  
  /* There was an error creating the packet, so report it */
  if( dtx == NULL_POINTER )
    goto exit;

  /*Free buffer which is use for payload ie */
  app_bm_free(pld_ie);
  pld_ie = NULL;
  /* set the handle and the length */
  dtx->msdu_handle = msdu_handle;
  if(dtx->msdu_handle == 135)
  {
    update_seq_number_l2_attr(dtx->sn);  // suneet :: do it as requirement 
  }
  /*  7.1.1.1.3 para 5:
  *  GTS flag has priority over indirect.
  *  We only send indirectly if we're a coordinator. */
#ifdef MAC_CFG_GTS_ENABLED
  if (tx_options & GTS_TRANSMISSION)
    status = mac_add_gts_transmission( &dst, dtx );
  else
#endif    
    /*TBD  How to determine which superframe (TRXSM) the primitive belongs to? */
    /* check if indirect transmission and acting as a coordinator */
    if (tx_options & INDIRECT_TRANSMISSION)
    {
#if ((MAC_CFG_LE_RIT_CAPABILITY == 1) || (MAC_CFG_LE_CSL_CAPABILITY == 1))
      /*During this period (macRITPeriod equals to zero), all transactions shall be 
      handled as those of normal non beacon-enabled PAN (macRxOnWhenIdle: False). 
      RIT is ON or OFF, if LE is enabled, put it into indirect queue to be pulled 
      by dest nodes */
      if (mac_pib.LEenabled)
      {
        status = mac_add_indirect_transmission (&src, &dst, dtx);
        goto exit;
      }else
#endif
#if (CFG_MAC_STARTSM_ENABLED == 1)                   
        if (( startsm_get_flags (startsm_p) & STARTSM_FLAG_COORD_MASK))
        {          
          if( dst.address_mode == MAC_SHORT_ADDRESS &&
             dst.address.short_address == BROADCAST_SHORT_ADDRESS)
            status = mac_queue_bcast_transmission (dtx );
          else
            status = mac_add_indirect_transmission (&src, &dst, dtx);
        }
        else
          status = mac_queue_direct_transmission (dtx);
#else
      status = mac_queue_direct_transmission (dtx);
#endif
    }
    else
    {
      if( dst.address_mode == MAC_NO_ADDRESS)
        status = mac_queue_bcast_transmission (dtx);
      else
	  #if (UPDATE_UFSI_AFTER_CCA == 0)
           status = mac_queue_direct_transmission( dtx );
	  #else
           status = mac_cca_event_do(dtx);
	  #endif
    }

exit:
  if (status != MAC_SUCCESS)
  {
    if (dtx != NULL_POINTER)
      mac_mem_free_tx( dtx );
	if (pld_ie != NULL)
      app_bm_free(pld_ie); /*Free buffer which is use for payload ie */ 
    send_mcps_data_confirm( msdu_handle, status, 0 ,0);
  }
  
  return 1;     /*Success*/
}

/******************************************************************************/ 
uchar process_fan_mac_mlme_set_req(uchar *buf, uint16_t length)
{
  mac_status_t status = MAC_SUCCESS;
  uint8_t* ie_ptr=NULL;
  uint8_t sub_ie_value=0x00;
  ie_ptr = buf;
  uint8_t ie_length = *(buf+2);
  sub_ie_value = *(buf+1);
  if(*ie_ptr == WISUN_INFO_HEADER_IE_ID)
  {
    ie_ptr++;
    if(*ie_ptr == WISUN_IE_SUBID_UTT_IE)
    {
      
    } 
    else if(*ie_ptr == WISUN_IE_SUBID_BT_IE)
    {
      ie_ptr+=2; //+subie+length
      mem_rev_cpy((uint8_t *)&mac_self_fan_info.bcast_slot_no,ie_ptr,2);
      ie_ptr+=2;
      mem_rev_cpy((uint8_t *)&mac_self_fan_info.bcast_frac_inter_offset,ie_ptr,4);
      goto exit;
    } 
    else if(*ie_ptr == WISUN_IE_SUBID_FC_IE)
    {
      
    } 
    else
    {
      status = MAC_INVALID_PARAMETER;
      goto exit;
      //invalid sub-ie
    }  
    
  }
  else if(*ie_ptr == WISUN_INFO_PAYLOAD_IE_ID)
  {
    ie_ptr++;
    if(*ie_ptr == WISUN_IE_SUBID_US_IE)
    {
      ie_ptr+=2; //+subie+length
      mac_self_fan_info.unicast_listening_sched.us_schedule.dwell_interval=*ie_ptr++;
      mac_self_fan_info.unicast_listening_sched.us_schedule.clock_drift=*ie_ptr++;
      mac_self_fan_info.unicast_listening_sched.us_schedule.timing_accuracy=*ie_ptr++; 
      
      mac_self_fan_info.unicast_listening_sched.us_schedule.channel_plan= *ie_ptr++; // need to change --shubham
      mac_self_fan_info.unicast_listening_sched.us_schedule.channel_function= *ie_ptr++;
      mac_self_fan_info.bcast_sched.bs_schedule.channel_function = 
        mac_self_fan_info.unicast_listening_sched.us_schedule.channel_function;
      mac_self_fan_info.unicast_listening_sched.us_schedule.excludded_channel_control= *ie_ptr++;
      mac_self_fan_info.unicast_listening_sched.us_schedule.chan_hop_count = *ie_ptr++;
      mac_self_fan_info.unicast_listening_sched.us_schedule.length= *ie_ptr++;
      
      ie_ptr+=4;  //Arjun: Incrementing pointer by 100 to bypass chan_hop_list
      
      if(mac_self_fan_info.unicast_listening_sched.us_schedule.excludded_channel_control == EXCLUDED_CHANNEL_PRESENT)                      //Arjun: checking excludded_channel_control value to deceide whether this uninon contains ex_channel_mask/range
      {
        mac_self_fan_info.unicast_listening_sched.us_schedule.excluded_channels.excluded_channel_ranges.num_of_ranges = *ie_ptr++;
        uint8_t no_exclude_channel_range = mac_self_fan_info.unicast_listening_sched.us_schedule.excluded_channels.excluded_channel_ranges.num_of_ranges;
        uint8_t i=0;
        if(no_exclude_channel_range <= MAX_EXCLUDED_CHANNEL_RANGES_SUPPORTED)
        {
          for(i = 0; i < no_exclude_channel_range; i++){
            memcpy(&mac_self_fan_info.unicast_listening_sched.us_schedule.excluded_channels.excluded_channel_ranges.ex_ch_range[i].start_ch, ie_ptr, 2);
            
            ie_ptr+=2;
            memcpy(&mac_self_fan_info.unicast_listening_sched.us_schedule.excluded_channels.excluded_channel_ranges.ex_ch_range[i].end_ch, ie_ptr, 2);
            ie_ptr+=2;
          }
        }
        uint8_t incPointer = (MAX_EXCLUDED_CHANNEL_RANGES_SUPPORTED-no_exclude_channel_range)*4;
        incPointer += 11;
        ie_ptr+=incPointer;
      }
      else if(mac_self_fan_info.unicast_listening_sched.us_schedule.excludded_channel_control == EXCLUDED_CHANNEL_MASK_PRESENT)
      {
        memcpy((uint8_t *)& mac_self_fan_info.unicast_listening_sched.us_schedule.excluded_channels.excluded_channel_mask,ie_ptr,32);
        ie_ptr+=32;
      }
      else{
        ie_ptr+=32;
      }
      
      ie_ptr+=4; //Arjun: bypassing the local_time_node;
      
      if(mac_self_fan_info.unicast_listening_sched.us_schedule.channel_plan == CH_REG_OP_PRESENT)           //Arjun: checking whether reg_op is present
      {
        mac_self_fan_info.unicast_listening_sched.un_channel_plan.ch_reg_op.reg_domain= *ie_ptr++;
        mac_self_fan_info.unicast_listening_sched.un_channel_plan.ch_reg_op.op_class = *ie_ptr++;
        ie_ptr+=5; //Arjun: to point the next data we need to jump the extra memomry of union
      }
      else if(mac_self_fan_info.unicast_listening_sched.us_schedule.channel_plan == CH_EXPLICIT_PRESENT)
      {
        memcpy((uint8_t *)& mac_self_fan_info.unicast_listening_sched.un_channel_plan.ch_explicit.ch0,ie_ptr,4);
        ie_ptr+=4;
        mac_self_fan_info.unicast_listening_sched.un_channel_plan.ch_explicit.channel_spacing = *ie_ptr++;
        memcpy((uint8_t *)& mac_self_fan_info.unicast_listening_sched.un_channel_plan.ch_explicit.num_chans,ie_ptr,2);
        ie_ptr+=2;
      }
      memcpy((uint8_t *)& mac_self_fan_info.unicast_listening_sched.channel_fixed.fixed_chan,ie_ptr,2);
      ie_ptr+=2;
      
    } 
    else if(*ie_ptr == WISUN_IE_SUBID_BS_IE)
      
    {
      
      ie_ptr+=2; //+subie+length
      memcpy((uint8_t *)& mac_self_fan_info.bcast_sched.bcast_interval,ie_ptr,4);
      
      /* Debdeep :: 11-aug-2018 :: default broadcast interval should be set if it is zero */
      if (mac_self_fan_info.bcast_sched.bcast_interval == 0)
        mac_self_fan_info.bcast_sched.bcast_interval = 1020;
      
      ie_ptr+=4;
      memcpy((uint8_t *)& mac_self_fan_info.bcast_sched.bcast_sched_id,ie_ptr,2);
      ie_ptr+=2;
      mac_self_fan_info.bcast_sched.bs_schedule.dwell_interval=*ie_ptr++;
      
      /* Debdeep :: 11-aug-2018 :: default broadcast dwell interval should be set if it is zero */
      if (mac_self_fan_info.bcast_sched.bs_schedule.dwell_interval == 0)
        mac_self_fan_info.bcast_sched.bs_schedule.dwell_interval = 255;
      
      mac_self_fan_info.bcast_sched.bs_schedule.clock_drift=*ie_ptr++;
      mac_self_fan_info.bcast_sched.bs_schedule.timing_accuracy=*ie_ptr++; 
      
      mac_self_fan_info.bcast_sched.bs_schedule.channel_plan= *ie_ptr++; // need to change --shubham
      mac_self_fan_info.bcast_sched.bs_schedule.channel_function= *ie_ptr++;
      mac_self_fan_info.bcast_sched.bs_schedule.excludded_channel_control= *ie_ptr++;
      mac_self_fan_info.bcast_sched.bs_schedule.chan_hop_count = *ie_ptr++;
      mac_self_fan_info.bcast_sched.bs_schedule.length= *ie_ptr++;
      
      ie_ptr+=4;  //Arjun: Incrementing pointer by 100 to bypass chan_hop_list
      
      if(mac_self_fan_info.bcast_sched.bs_schedule.excludded_channel_control == EXCLUDED_CHANNEL_PRESENT)                      //Arjun: checking excludded_channel_control value to deceide whether this uninon contains ex_channel_mask/range
      {
        mac_self_fan_info.bcast_sched.bs_schedule.excluded_channels.excluded_channel_ranges.num_of_ranges = *ie_ptr++;
        uint8_t no_exclude_channel_range = mac_self_fan_info.bcast_sched.bs_schedule.excluded_channels.excluded_channel_ranges.num_of_ranges;
        uint8_t i=0;
        if(no_exclude_channel_range <= MAX_EXCLUDED_CHANNEL_RANGES_SUPPORTED)
        {
          for(i = 0; i < no_exclude_channel_range; i++){
            memcpy(&mac_self_fan_info.bcast_sched.bs_schedule.excluded_channels.excluded_channel_ranges.ex_ch_range[i].start_ch, ie_ptr, 2);
            
            ie_ptr+=2;
            memcpy(&mac_self_fan_info.bcast_sched.bs_schedule.excluded_channels.excluded_channel_ranges.ex_ch_range[i].end_ch, ie_ptr, 2);
            ie_ptr+=2;
          }
        }
        uint8_t incPointer = (MAX_EXCLUDED_CHANNEL_RANGES_SUPPORTED-no_exclude_channel_range)*4;
        incPointer += 11;
        ie_ptr+=incPointer;
      }
      else if(mac_self_fan_info.bcast_sched.bs_schedule.excludded_channel_control == EXCLUDED_CHANNEL_MASK_PRESENT)
      {
        memcpy((uint8_t *)& mac_self_fan_info.bcast_sched.bs_schedule.excluded_channels.excluded_channel_mask,ie_ptr,32);
        ie_ptr+=32;
      }
      else{
        ie_ptr+=32;
      }
      
      ie_ptr+=4; //Arjun: bypassing the local_time_node;
      
      if(mac_self_fan_info.bcast_sched.bs_schedule.channel_plan == CH_REG_OP_PRESENT)           //Arjun: checking whether reg_op is present
      {
        mac_self_fan_info.bcast_sched.un_channel_plan.ch_reg_op.reg_domain= *ie_ptr++;
        mac_self_fan_info.bcast_sched.un_channel_plan.ch_reg_op.op_class = *ie_ptr++;
        ie_ptr+=5; //Arjun: to point the next data we need to jump the extra memomry of union
      }
      else if(mac_self_fan_info.bcast_sched.bs_schedule.channel_plan == CH_EXPLICIT_PRESENT)
      {
        memcpy((uint8_t *)& mac_self_fan_info.bcast_sched.un_channel_plan.ch_explicit.ch0,ie_ptr,4);
        ie_ptr+=4;
        mac_self_fan_info.bcast_sched.un_channel_plan.ch_explicit.channel_spacing = *ie_ptr++;
        memcpy((uint8_t *)& mac_self_fan_info.bcast_sched.un_channel_plan.ch_explicit.num_chans,ie_ptr,2);
        ie_ptr+=2;
      }
      memcpy((uint8_t *)& mac_self_fan_info.bcast_sched.channel_fixed.fixed_chan,ie_ptr,2);
      ie_ptr+=2;
    } 
    else if(*ie_ptr == WISUN_IE_SUBID_VP_IE)
    {
      
    } 
    else if(*ie_ptr == WISUN_IE_SUBID_PAN_IE)
    {
      ie_ptr+=2; //+subie+length
      memcpy((uint8_t *)&mac_self_fan_info.pan_metrics.pan_size,ie_ptr,2);
      ie_ptr+=2;
      memcpy((uint8_t *)&mac_self_fan_info.pan_metrics.routing_cost,ie_ptr,2);
      ie_ptr+=2;
      mac_self_fan_info.pan_metrics.parent_bs_ie_use= *ie_ptr++;
      mac_self_fan_info.pan_metrics.routing_methood= *ie_ptr++;
      mac_self_fan_info.pan_metrics.fan_tps_version= *ie_ptr++;
      ie_ptr++; //length
#if(APP_LBR_ROUTER == 1)        
      if(get_node_type() == 0x00);
        mac_self_fan_info.pan_metrics.pan_id = get_current_pan_id();
#endif
      goto exit;
    } 
    else if(*ie_ptr == WISUN_IE_SUBID_NETNAME_IE)
    {
      ie_ptr+=2; //+subie+length
      memcpy((uint8_t *)mac_self_fan_info.net_name,ie_ptr,ie_length);
      ie_ptr+=ie_length;
      mac_self_fan_info.net_name_length=ie_length;
      goto exit;
    } 
    else if(*ie_ptr == WISUN_IE_SUBID_PANVER_IE)
    {
      ie_ptr+=2; //+subie+length
      if(get_node_type() == 0x00)
        memcpy((uint8_t *)&mac_self_fan_info.pan_ver,ie_ptr,2);
    } 
    else if(*ie_ptr == WISUN_IE_SUBID_GTKHASH_IE)
    {
#if ((FAN_EAPOL_FEATURE_ENABLED == 1) || (APP_GTK_HARD_CODE_KEY == 1))     
      MAC_SHA256_CTX ctx;
      uint8_t hash[32] = {0};
      
      ie_ptr+=2; //+subie+length
      mac_self_fan_info.mac_gtk_hash_ele.gtkl = *ie_ptr++;
      
      if((!memcmp((uint8_t *)mac_self_fan_info.mac_gtk_hash_ele.MAC_GTK0_Key,ie_ptr,16)) ||
         (get_mac_active_key_index() == 1))
        ie_ptr+=16;
      else
      {
        memcpy((uint8_t *)mac_self_fan_info.mac_gtk_hash_ele.MAC_GTK0_Key,ie_ptr,16);
        ie_ptr+=16;
        if (mac_self_fan_info.mac_gtk_hash_ele.gtkl & 0x01)
        {
          // Hash of GTK0
          MAC_sha256_init(&ctx);
          MAC_sha256_update(&ctx,mac_self_fan_info.mac_gtk_hash_ele.MAC_GTK0_Key,GTK_KEY_LEN);
          MAC_sha256_final(&ctx,hash);
          Truncate_64 ( hash , (uint8_t *)mac_self_fan_info.gtk_hash_ele.gtk0_hash);
        }
        else
          memset ((uint8_t *)mac_self_fan_info.gtk_hash_ele.gtk0_hash, 0, 8);
      }
      
      /*********************************************************************/
      if((!memcmp((uint8_t *)mac_self_fan_info.mac_gtk_hash_ele.MAC_GTK1_Key,ie_ptr,16)) ||
        (get_mac_active_key_index() == 2))
        ie_ptr+=16;
      else
      {
        memcpy((uint8_t *)mac_self_fan_info.mac_gtk_hash_ele.MAC_GTK1_Key,ie_ptr,16);
        ie_ptr+=16;
        if (mac_self_fan_info.mac_gtk_hash_ele.gtkl & 0x02)
        {
          // Hash of GTK1
          MAC_sha256_init(&ctx);
          MAC_sha256_update(&ctx,mac_self_fan_info.mac_gtk_hash_ele.MAC_GTK1_Key,GTK_KEY_LEN);
          MAC_sha256_final(&ctx,hash);
          Truncate_64 ( hash , (uint8_t *)mac_self_fan_info.gtk_hash_ele.gtk1_hash);
        }
        else
          memset ((uint8_t *)mac_self_fan_info.gtk_hash_ele.gtk1_hash, 0, 8);
      }
      
      /*********************************************************************/
      if((!memcmp((uint8_t *)mac_self_fan_info.mac_gtk_hash_ele.MAC_GTK2_Key,ie_ptr,16)) ||
        (get_mac_active_key_index() == 3))
        ie_ptr+=16;
      else
      {
        memcpy((uint8_t *)mac_self_fan_info.mac_gtk_hash_ele.MAC_GTK2_Key,ie_ptr,16);
        ie_ptr+=16;
        if (mac_self_fan_info.mac_gtk_hash_ele.gtkl & 0x04)
        {
          // Hash of GTK2
          MAC_sha256_init(&ctx);
          MAC_sha256_update(&ctx,mac_self_fan_info.mac_gtk_hash_ele.MAC_GTK2_Key,GTK_KEY_LEN);
          MAC_sha256_final(&ctx,hash);
          Truncate_64 ( hash , (uint8_t *)mac_self_fan_info.gtk_hash_ele.gtk2_hash);
        }
        else
          memset ((uint8_t *)mac_self_fan_info.gtk_hash_ele.gtk2_hash, 0, 8);
        }
      
      /*********************************************************************/
      if((!memcmp((uint8_t *)mac_self_fan_info.mac_gtk_hash_ele.MAC_GTK3_Key,ie_ptr,16)) ||
        (get_mac_active_key_index() == 4))
      {
      }
      else
      {
        memcpy((uint8_t *)mac_self_fan_info.mac_gtk_hash_ele.MAC_GTK3_Key,ie_ptr,16);
        if (mac_self_fan_info.mac_gtk_hash_ele.gtkl & 0x08)
        {
          // Hash of GTK3
          MAC_sha256_init(&ctx);
          MAC_sha256_update(&ctx,mac_self_fan_info.mac_gtk_hash_ele.MAC_GTK3_Key,GTK_KEY_LEN);
          MAC_sha256_final(&ctx,hash);
          Truncate_64 ( hash , (uint8_t *)mac_self_fan_info.gtk_hash_ele.gtk3_hash);
        }
        else
          memset ((uint8_t *)mac_self_fan_info.gtk_hash_ele.gtk3_hash, 0, 8);
      }
#endif
      
    } 
    else
    {
      status = MAC_INVALID_PARAMETER;
      goto exit;
      //invalid sub-ie
    }  
  }
  else
  {
    status = MAC_INVALID_PARAMETER;
    goto exit;
    //invalid ie
  }  
  
exit:
  send_mlme_set_fan_mac_confirm((uchar)status,sub_ie_value);
  
  return 1;
  
}

/******************************************************************************/ 
void set_mac_self_info_from_lbr(uint8_t *rec_ptr,uint16_t rec_len)
{

#if ((FAN_EAPOL_FEATURE_ENABLED == 1) || (APP_GTK_HARD_CODE_KEY == 1))
  
  MAC_SHA256_CTX ctx = {0};
  uint8_t hash[32] = {0};
  
  memcpy((uint8_t *)mac_self_fan_info.mac_gtk_hash_ele.MAC_GTK0_Key,rec_ptr,16);
  rec_ptr+=16;
  if (mac_self_fan_info.mac_gtk_hash_ele.gtkl & 0x01)
  {
    // Hash of GTK0
    MAC_sha256_init(&ctx);
    MAC_sha256_update(&ctx,mac_self_fan_info.mac_gtk_hash_ele.MAC_GTK0_Key,GTK_KEY_LEN);
    MAC_sha256_final(&ctx,hash);
    Truncate_64 ( hash , (uint8_t *)mac_self_fan_info.gtk_hash_ele.gtk0_hash);
  }
  else
    memset ((uint8_t *)mac_self_fan_info.gtk_hash_ele.gtk0_hash, 0, 8);
  
  /*********************************************************************/
  memcpy((uint8_t *)mac_self_fan_info.mac_gtk_hash_ele.MAC_GTK1_Key,rec_ptr,16);
  rec_ptr+=16;
  if (mac_self_fan_info.mac_gtk_hash_ele.gtkl & 0x02)
  {
    // Hash of GTK1
    MAC_sha256_init(&ctx);
    MAC_sha256_update(&ctx,mac_self_fan_info.mac_gtk_hash_ele.MAC_GTK1_Key,GTK_KEY_LEN);
    MAC_sha256_final(&ctx,hash);
    Truncate_64 ( hash , (uint8_t *)mac_self_fan_info.gtk_hash_ele.gtk1_hash);
  }
  else
    memset ((uint8_t *)mac_self_fan_info.gtk_hash_ele.gtk1_hash, 0, 8);
  
  /*********************************************************************/
  memcpy((uint8_t *)mac_self_fan_info.mac_gtk_hash_ele.MAC_GTK2_Key,rec_ptr,16);
  rec_ptr+=16;
  if (mac_self_fan_info.mac_gtk_hash_ele.gtkl & 0x04)
  {
    // Hash of GTK2
    MAC_sha256_init(&ctx);
    MAC_sha256_update(&ctx,mac_self_fan_info.mac_gtk_hash_ele.MAC_GTK2_Key,GTK_KEY_LEN);
    MAC_sha256_final(&ctx,hash);
    Truncate_64 ( hash , (uint8_t *)mac_self_fan_info.gtk_hash_ele.gtk2_hash);     
  }
  else
    memset ((uint8_t *)mac_self_fan_info.gtk_hash_ele.gtk2_hash, 0, 8);
  
  /*********************************************************************/
  memcpy((uint8_t *)mac_self_fan_info.mac_gtk_hash_ele.MAC_GTK3_Key,rec_ptr,16);
  if (mac_self_fan_info.mac_gtk_hash_ele.gtkl & 0x08)
  {
    // Hash of GTK3
    MAC_sha256_init(&ctx);
    MAC_sha256_update(&ctx,mac_self_fan_info.mac_gtk_hash_ele.MAC_GTK3_Key,GTK_KEY_LEN);
    MAC_sha256_final(&ctx,hash);
    Truncate_64 ( hash , (uint8_t *)mac_self_fan_info.gtk_hash_ele.gtk3_hash);       
  }
  else
    memset ((uint8_t *)mac_self_fan_info.gtk_hash_ele.gtk3_hash, 0, 8);
  
#endif
  
}

#endif
/******************************************************************************/ 

