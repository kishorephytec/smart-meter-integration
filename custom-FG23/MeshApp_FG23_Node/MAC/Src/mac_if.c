/** \file mac_if.c
 *******************************************************************************
 ** \brief Provides APIs for the packets to be transmitted 
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

#include "mac_uplink.h"
#include "macutils.h"

#if(CFG_MAC_PENDADDR_ENABLED == 1) 
#include "pendaddr.h"
#include "pandesc.h"
#endif

#include "sm.h"

#if(CFG_MAC_SFTSM_ENABLED == 1)
#include "sftsm.h"
#endif

#if(CFG_MAC_MPMSM_ENABLED == 1)
#include "mpmsm.h"
#endif

#if(CFG_MAC_LBTSM_ENABLED == 1)
#include "lbtsm.h"
#else
#include "ccasm.h"
#endif

#include "trxsm.h"
#include "mac_frame_parse.h"

#if(CFG_MAC_SYNCSM_ENABLED == 1)
#include "syncsm.h"
#endif

#if(CFG_MAC_SCANSM_ENABLED == 1)  
#include "scansm.h"
#endif
#if(CFG_MAC_DRSM_ENABLED == 1)
#include "drsm.h"
#endif

#include "ccasm.h"
#if(CFG_MAC_SFTSM_ENABLED == 1)  
#include "startsm.h"
#endif

#if(CFG_ASSOCSM_ENABLED ==1)
#include "assocsm.h"
#include "enackwaitsm.h"
#endif

#if(CFG_MAC_PTSM_ENABLED == 1)   
#include "ptsm.h"
#endif


#include "event_manager.h"
#include "fan_sm.h"

#ifdef UTEST_MACIF
#include "utest_support.h"
#endif

#ifdef MAC_CFG_SECURITY_ENABLED
#include "mac_security.h"
#endif

#if ( ( MAC_CFG_LE_RIT_CAPABILITY == 1 ) || ( MAC_CFG_LE_CSL_CAPABILITY == 1 ) )
#include "mac_le.h"
#endif

#include "tri_tmr.h"
#include "timer_service.h"
#include "mac_interface_layer.h"
#include "fan_mac_interface.h" 
#include "fan_config_param.h"
#include "mac_nbr_manager.h"
#include "fan_sm.h"
#include "fan_mac_nbr_info.h"
#include "l3_random_utility.h"


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

/*******************************************************************************
 ** \struct nwk_to_mac_process_table_t
 **  Structure to network to mac process APIs
 *******************************************************************************
 **/ 
/*Umesh : 28-12-2017*/
//typedef struct nwk_to_mac_process_table_struct
//{
//    uchar primitive;
//    uchar (* action)( uchar *args, uint16_t length );
//} nwk_to_mac_process_table_t;
/*this is not used any where in this file*/
/*******************************************************************************
 ** \struct queue_test_struct
 **  Structure to store id and flag of a test queue
 *******************************************************************************
 **/
struct queue_test_struct
{
    int id;
    int flag;
};

/*******************************************************************************
 ** \struct queue_test
 **  Structure to store different queues
 *******************************************************************************
 **/
struct queue_test_struct queue_test[] =
{
    { QUEUE_BCN_RX,      AF_BEACON_RECEIVED },       /* received beacon queue  */
    { QUEUE_RX_MSG,      AF_RCV_MSG_PENDING },       /* received message queue */
    { QUEUE_TX_DONE,     AF_TX_MSG_SENT_PENDING },   /* transmitted msg queue  */
    { QUEUE_INDIRECT_TX, AF_IND_MSG_PENDING },       /* Indirect message queue */
    { QUEUE_NHLE_2_MCPS, AF_MCPS_MSG_PENDING },      /* mac->nwk layer msg q   */
    { QUEUE_NHLE_2_MLME, AF_MLME_MSG_PENDING },      /* nwk -> mac layer msg q */
    { QUEUE_CAP_TX,      AF_CAP_MSG_PENDING },       /* message due for cap tx */
    { 0,                 0                  }        /* end of list */
};

/*
** ============================================================================
** Private Variable Definitions
** ============================================================================
*/
static mac_tx_t dtx_msg_buffer[MAX_DTX_MESSAGES];			/* queue of dtx messages */
#if(CFG_MAC_BEACON_ENABLED == 1)  
static mac_tx_t pool_tx_bcn[MAX_BCN_MESSAGES];				/* pool of beacons */
static mac_tx_t pool_tx_mpmeb[MAX_MPM_EB_MESSAGES];			/* pool of mpm enhanced beacons */
static mac_tx_t pool_tx_eb[MAX_EB_MESSAGES];				/* pool of enhanced beacons */
#endif
static mac_rx_t rx_msg_buf[MAX_RCV_MESSAGES];				/* buffers reserved for MAC receive messages */
/*Umesh : 28-12-2017*/
//static ushort last_active=0;
/*this variable is not used*/
static const uchar def_aExtendedAddress[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0xAA, 0xBB, 0xCC };
//static uint8_t fixed_frame_type=0;/*Umesh : 15-01-2018*//*this varriable is not requied here moved to fan_mac_if.c*/

/*
** =============================================================================
** Public Variables Definitions
** =============================================================================
**/
/* state machines */
        trxsm_t trxsm;
        trxsm_t *trxsm_p;
#if(CFG_MLME_SYNC_REQ == 1)
	syncsm_t syncsm;
	syncsm_t *syncsm_p;
#endif
#if(CFG_MAC_SCANSM_ENABLED == 1)          
        scansm_t scansm;
        scansm_t *scansm_p;
#endif
        
#if(CFG_MAC_DRSM_ENABLED == 1)        
        drsm_t drsm;
#endif 
#if(CFG_MAC_SFTSM_ENABLED == 1)         
        sftsm_t sftsm_in;
        sftsm_t sftsm_out;
        sftsm_t *sftsm_in_p;        
        startsm_t startsm;
        startsm_t *startsm_p;
#endif     
#if(CFG_ASSOCSM_ENABLED ==1)        
        assocsm_t assocsm;
        enacksm_t enacksm;
        enacksm_t *enacksm_p;
#endif
        
#if(CFG_MAC_PTSM_ENABLED == 1)           
        ptsm_t ptsm;
        ptsm_t *ptsm_p;
#endif        
        sm_t *ccasm_p;
/*
** ============================================================================
** External Variable Declarations
** ============================================================================
*/
        extern mac_pib_t mac_pib;
        extern uchar aExtendedAddress[8];
        extern void *saved_pc;
        extern int current_thread;
        extern uint8_t heap[];

#if(CFG_MAC_MPMSM_ENABLED == 1)        
        extern mpmsm_t mpmsm;
#endif
        
#if ( ( MAC_CFG_LE_RIT_CAPABILITY == 1 ) || ( MAC_CFG_LE_CSL_CAPABILITY == 1 ) )
	extern low_energy_t low_energy;
#endif 

#ifdef MAC_CFG_SECURITY_ENABLED
	extern mac_security_data_t     mac_security_data;
#endif        

        /*Umesh : 16-01-2018*/
#ifdef WISUN_FAN_MAC        
        extern mac_nbr_data_t mac_nbr_data;	
#endif 


/*
** ============================================================================
** Extern Function Prototypes
** ============================================================================
*/

#ifdef MAC_CFG_SECURITY_ENABLED
	extern void aes_init(void);
#endif           
        
#if (DEBUG_FLAG == 1)
	extern void log_item(uint8_t item);
#endif
        extern void * app_bm_alloc(
            uint16_t length//base_t length       
            );
            
        extern void app_bm_free(
            uint8_t *pMem      
            );

        extern void queue_manager_initialise_all_queues(void);
        extern uchar process_mcps_data_request( uchar *buf, uint16_t length );
        extern uchar process_fan_mcps_data_request( uchar *buf, uint16_t length );
#ifdef WISUN_FAN_MAC
        extern uchar send_ws_sync_result(mac_tx_t *dm);
#endif
          
/********************* FAN Related primitives ***************************/

        extern uchar process_ws_async_frame_request( uchar *buf, uint16_t length );

/********************* End of FAN Related primitives ********************/


#if(CFG_MCPS_PURGE_REQ_CONF == 1)
        extern uchar process_mcps_purge_request( uchar *buf, uint16_t length );
#endif


/*
** ============================================================================
** Private Function Prototypes
** ============================================================================
*/

#ifdef MAC_CFG_GTS_ENABLED
	static void mac_check_expired_gts(void);
	static void gts_purge( gts_data_t *gd );
#endif

#if SUPPORT_PROCESSOR_MODE
	static uchar deep_sleep_mode_ok( void );
	static uchar sleep_mode_ok( void );
#endif

        static void mac_if_msg_initialise(void);
        static msg_t *get_mlme_msg(void);
        static msg_t *get_mcps_msg(void);
        static void requeue_mlme_msg( msg_t * msgp );
        static void requeue_mcps_msg( msg_t * msgp );
//static void mac_process_spi( void );

/*
** ============================================================================
** Public Function Prototypes
** ============================================================================
*/
#if(CFG_MLME_RX_ENABLE_REQ_CONF == 1)
	uchar mac_process_receiver_enable_state(void);
#endif	/*(CFG_MLME_RX_ENABLE_REQ_CONF == 1)*/

        uchar mac_check_cap_message_ucast(void);
        uchar mac_check_cap_message_bcast();
        uchar check_pending_data(void);
        uchar process_completed_dtx_messages( void );
        uchar mac_check_expired_pending_messages( void );
        uchar mac_check_panid_conflict_state( void );
        /*Umesh : 28-12-2017*/
        void mac_check_scan_state( void );
        /*check this is used or not*/

#ifdef MAC_CFG_SECURITY_ENABLED
	uchar load_auto_request_sec_params( security_params_t* p_sec_params );
#endif

/*
** ============================================================================
** Public Function Definitions
** ============================================================================
*/

void mac_initialise( uchar cold_start )
{
    uchar i = 0;

    if( cold_start )
    {
        //mac_data_init( );
		memset( (void *) &mac_data, 0, sizeof( mac_data_t ) );
        mac_pib_init( );
        //mac_pib_init_ieee( );
		memcpy( (void *) aExtendedAddress, (void *) def_aExtendedAddress, 8);
    }
    
    /*tmr_create_one_shot_timer
	( 
		&(mac_data.temp_timer),
		MAX_RX_INTERVAL, // usecs
		(sw_tmr_cb_t)phy_reset_sports_driver,
		&mac_data 
	); */
	
/*Uemsh : 10-01-2018 this thing goes to modified after sepration of mac and fan mac*/
#ifdef WISUN_FAN_MAC
    mac_data.p_nbr_data = &mac_nbr_data;
#endif
	
#ifdef MAC_CFG_SECURITY_ENABLED
    mac_data.security = &mac_security_data;
    

    /* Security Data that is ALWAYS Initialised */
    /* Initialise Security Queues */
    queue_initialise( (queue_t *) &mac_security_data.hallin_rx_queue );
    queue_initialise( (queue_t *) &mac_security_data.hallin_tx_queue );
    queue_initialise( (queue_t *) &mac_security_data.rx_security_queue );

    /* Create the links between the security Buffer and the beacon tx structure */
    //mac_security_data.beacon_data.private_msg_data = &mac_data.beacon_tx;
    //mac_data.beacon_tx.security_data = &mac_security_data.beacon_data;
    //mac_data.beacon_tx.security_data->q_item.link = NULL_POINTER;

    mac_security_data.encrypt_msg = NULL_POINTER;
    mac_security_data.decrypt_msg = NULL_POINTER;


//#ifdef BLACKFIN
//    aes_init();
//#endif

#endif
    queue_manager_initialise_all_queues();
    /*queue_manager_initialise(QUEUE_TX_FREE);
    queue_manager_initialise(QUEUE_RX_FREE);
    queue_manager_initialise(QUEUE_BCN_TX_FREE);
    queue_manager_initialise(QUEUE_BCN_TX_CURR);
    queue_manager_initialise(QUEUE_BCN_RX);
    queue_manager_initialise(QUEUE_DATA_REQUEST);
    queue_manager_initialise(QUEUE_BCAST);
    queue_manager_initialise(QUEUE_INDIRECT_TX);
    queue_manager_initialise(QUEUE_TX_DONE);
    queue_manager_initialise(QUEUE_CAP_TX);
    queue_manager_initialise(QUEUE_PHY_RX);
    queue_manager_initialise(QUEUE_RX_MSG);
    queue_manager_initialise(QUEUE_RX_FREE_MSG);
    queue_manager_initialise(QUEUE_GTS_TX);
    queue_manager_initialise(QUEUE_EB_TX_FREE);
    queue_manager_initialise(QUEUE_EB_TX_CURR);
    queue_manager_initialise(QUEUE_MPM_EB_TX_FREE);
    queue_manager_initialise(QUEUE_MPM_EB_TX_CURR);*/

    mac_if_msg_initialise();
    
#if(CFG_MAC_PENDADDR_ENABLED == 1)
      pendaddr_initialise();
#endif
    
    for(i=0; i < MAX_DTX_MESSAGES; i++ )
    {
        queue_manager_push_back( QUEUE_TX_FREE,(queue_item_t *) &dtx_msg_buffer[i]);
    }

    for( i = 0; i < MAX_RCV_MESSAGES; i++ )
    {
        queue_manager_push_back( QUEUE_RX_FREE_MSG, (queue_item_t *) &rx_msg_buf[i]);
    }
#if(CFG_MAC_BEACON_ENABLED == 1)  
    for( i = 0; i < MAX_BCN_MESSAGES; i++ )
    {
        queue_manager_push_back( QUEUE_BCN_TX_FREE, (queue_item_t *) &pool_tx_bcn[i]);
    }

	for( i = 0; i < MAX_EB_MESSAGES; i++ )
    {
        queue_manager_push_back( QUEUE_EB_TX_FREE, (queue_item_t *) &pool_tx_eb[i]);
    }
	
	for( i = 0; i < MAX_MPM_EB_MESSAGES; i++ )
    {
        queue_manager_push_back( QUEUE_MPM_EB_TX_FREE, (queue_item_t *) &pool_tx_mpmeb[i]);
    }
#endif
    mac_data.ble_expiry_timer = 1;
#if(CFG_MAC_LBTSM_ENABLED == 1)
    lbtsm_initialise_init(&ccasm_p);
#endif
#if(CFG_MAC_CCA_ENABLED == 1)    
    ccasm_initialise( &ccasm_p );
#endif    
#if(CFG_MAC_SFTSM_ENABLED == 1)      
    startsm_p = &startsm;
#endif    
    trxsm_p = NULL_POINTER;

    /* initialise MAC state machines */
    if( cold_start )
    {
        /* initialise MAC components
         *     note: these are saved during deep sleep, so initialise only
         *     during cold start */
#if(CFG_MLME_SYNC_REQ == 1)
        syncsm_initialise( &syncsm, &trxsm );
#endif
#if(CFG_MAC_SFTSM_ENABLED == 1)          
        startsm_initialise( startsm_p );
#endif
        
#ifdef MAC_CFG_BEACONING_ENABLED
        /* initialise incoming SFT-SM */
        sftsm_in_initialise( &sftsm_in );

        /* initialise outgoing SFT-SM */
        sftsm_out_initialise( &sftsm_out );
        
#endif

#if(CFG_MAC_MPMSM_ENABLED == 1)        
        mpmsm_initialise(&mpmsm);
#endif		
        /* initialise TRX-SM */
        trxsm_initialise( &trxsm, NULL );
        trxsm.curr_bcn_queue = queue_manager_get_list( QUEUE_BCN_TX_CURR );
        trxsm.curr_mpm_eb_queue = queue_manager_get_list( QUEUE_MPM_EB_TX_CURR );
        trxsm.curr_eb_queue = queue_manager_get_list( QUEUE_EB_TX_CURR );
		
        trxsm.bcast_queue = queue_manager_get_list( QUEUE_BCAST );
        trxsm.direct_queue = queue_manager_get_list( QUEUE_CAP_TX );
        trxsm.completed_queue = queue_manager_get_list( QUEUE_TX_DONE );
        trxsm.indirect_queue = queue_manager_get_list( QUEUE_INDIRECT_TX );

		
#ifdef ENHANCED_ACK_SUPPORT
        trxsm.enackwait_queue = queue_manager_get_list( QUEUE_ENACKWAIT );
#endif //#ifdef ENHANCED_ACK_SUPPORT
        trxsm_start( &trxsm );
        trxsm_p = &trxsm;
        
#if ( ( MAC_CFG_LE_RIT_CAPABILITY == 1 ) || ( MAC_CFG_LE_CSL_CAPABILITY == 1 ) )		
		low_energy_initialise(&low_energy);
#endif
#if(CFG_MAC_PTSM_ENABLED == 1)                
        /* initialise PM-SM */
        ptsm_p = &ptsm;
        ptsm_initialise( ptsm_p, &trxsm );
#endif //#if(CFG_MAC_PTSM_ENABLED == 1)          
#ifdef ENHANCED_ACK_SUPPORT
	enacksm_p = &enacksm;
        enacksm_initialise( enacksm_p, &trxsm );
#endif
    }
    else
    {
        
        trxsm_p = NULL_POINTER;
#if(CFG_MAC_SFTSM_ENABLED == 1)          
        startsm_p = &startsm;
        startsm_initialise( startsm_p );
#endif //#if(CFG_MAC_SFTSM_ENABLED == 1)  
#if(CFG_MAC_MPMSM_ENABLED == 1)        
        mpmsm_initialise(&mpmsm);
#endif        
        trxsm_initialise( &trxsm, NULL );
        trxsm.curr_bcn_queue = queue_manager_get_list( QUEUE_BCN_TX_CURR );
        trxsm.curr_mpm_eb_queue = queue_manager_get_list( QUEUE_MPM_EB_TX_CURR );
        trxsm.curr_eb_queue = queue_manager_get_list( QUEUE_EB_TX_CURR );
		
        trxsm.bcast_queue = queue_manager_get_list( QUEUE_BCAST );
        trxsm.direct_queue = queue_manager_get_list( QUEUE_CAP_TX );
        trxsm.completed_queue = queue_manager_get_list( QUEUE_TX_DONE );
        trxsm.indirect_queue = queue_manager_get_list( QUEUE_INDIRECT_TX );
#ifdef ENHANCED_ACK_SUPPORT
        trxsm.enackwait_queue = queue_manager_get_list( QUEUE_ENACKWAIT );
#endif //#ifdef ENHANCED_ACK_SUPPORT
        trxsm_start( &trxsm );
        trxsm_p = &trxsm;
#if ( ( MAC_CFG_LE_RIT_CAPABILITY == 1 ) || ( MAC_CFG_LE_CSL_CAPABILITY == 1 ) )		
		low_energy_initialise(&low_energy);
#endif	        
    }
#if(CFG_MAC_PTSM_ENABLED == 1)       
	ptsm_p = &ptsm;
#endif    
	trxsm_p = &trxsm;

    /* initialise MAC components
     *     note: these are not saved during deep sleep, initialisation is
     *     the same for both cold and warm start */
#if(CFG_MAC_SCANSM_ENABLED == 1)          
        scansm_initialise( &scansm, &trxsm );
        scansm_p = &scansm;
#endif
        
#if(CFG_MAC_DRSM_ENABLED == 1)  
        drsm_initialise( &drsm, cold_start );
#endif
#if(CFG_MLME_ASSOCIATE_REQ_CONF == 1)
        assocsm_initialise( &assocsm, cold_start );
#endif        
#if(CFG_MAC_PTSM_ENABLED == 1)           
        ptsm_p = &ptsm;
        ptsm_initialise( ptsm_p, &trxsm );
#endif
        
#if(CFG_MLME_SYNC_REQ == 1)
        syncsm_p = &syncsm;
#endif

    /* initialise SFT-SM pointers */
#if(CFG_MAC_SFTSM_ENABLED == 1)        
    sftsm_in_p = NULL_POINTER;
#endif    
}

/*****************************************************************************/

/* checks if we have detected pending data in our coordinator
 *
 * When a slave device processes a beacon it will set the short address and/or
 * ieee address pending flags dependent on the pending address fields in the
 * beacon.  This function is called from the foreground to create a data request
 * if required
 * return 0 if no messages to process, 1 otherwise
 */
uchar check_pending_data(void)
{
    mac_address_t dst_address;
    mac_address_t mac_address;
    mac_tx_t *dtx = NULL_POINTER;
    mac_status_t error;
#if(CFG_MAC_DRSM_ENABLED == 1)      
    sm_event_t event;
#endif

#ifdef MAC_CFG_SECURITY_ENABLED
    security_params_t sec_params;
#endif
#if(CFG_MAC_DRSM_ENABLED == 1)
    if( drsm_get_state( &drsm ) != DRSM_STATE_NONE )
    {
        /* there is already a data request in progress, so nothing to do*/
        return 1; /* need to keep processor running though */
    }
#endif
    /* check if we already have a data request queued */
    //dtx = NULL_POINTER;

    /* scan the queue, looking for the item */
    while( (dtx = (mac_tx_t *) queue_manager_scan_next( QUEUE_CAP_TX, (queue_item_t *)dtx )) != NULL_POINTER )
    {
        if( (dtx->type == MAC_FRAME_TYPE_MAC_COMMAND) && (dtx->cmd == DATA_REQUEST))
        {
            return 1;
        }
    }

#ifdef MAC_CFG_SECURITY_ENABLED
	load_auto_request_sec_params(&sec_params);
#endif

    /* assume there are messages pending */
        set_process_activity( AF_MESSAGES_PENDING );
	dst_address.pan_id = mac_address.pan_id = mac_pib.PANId;
    if( mac_data.short_address_pending )
    {
        /* create a data request */
        mac_address.address_mode = MAC_SHORT_ADDRESS;
        mac_address.address.short_address = mac_pib.ShortAddress;
        //mac_address.pan_id = mac_pib.PANId;
    }
    else if( mac_data.ieee_address_pending )
    {
		#if (DEBUG_FLAG == 1)
			log_item(0xAD);
		#endif
		/* create a data request */
        mac_address.address_mode = MAC_IEEE_ADDRESS;
        mac_address.address.ieee_address = aExtendedAddress;
		//mac_address.pan_id = mac_pib.PANId;
    }
    else
    {
        /* no messages pending, so we can sleep here */
        clear_process_activity( AF_MESSAGES_PENDING );
        return 0;
    }

    /* pending data, so set the coordinator address */
    //dst_address.pan_id = mac_pib.PANId;
    if( mac_pib.CoordShortAddress < USE_IEEE_ADDRESS )
    {
        /* short address mode */
        dst_address.address_mode = MAC_SHORT_ADDRESS;
        dst_address.address.short_address = mac_pib.CoordShortAddress;
    }
    else
    {
        /* IEEE address mode */
        dst_address.address_mode = MAC_IEEE_ADDRESS;
        dst_address.address.ieee_address = mac_pib.CoordExtendedAddress;
    }

#ifdef MAC_CFG_SECURITY_ENABLED
    /* Check if we are trying to secure a frame when security is not enabled */
    if ( ( mac_pib.ConfigFlags & ( USE_2006_PRIMITIVES|USE_2011_PRIMITIVES ) ) 
        && (mac_security_data.pib.auto_request_security_level != MAC_SECURITY_NONE)
        && (mac_pib.mac_security_enabled == FALSE))
    {
        sec_params.key_id_mode = mac_security_data.pib.auto_request_keyid_mode;
        sec_params.security_level = mac_security_data.pib.auto_request_security_level;

        /* Report Error to NHL */
        send_mlme_comm_status_indication(&mac_address, &dst_address,
                                         MAC_UNSUPPORTED_SECURITY,
                                         &sec_params );

        /* now clear the cause of the data request */
        if (mac_address.address_mode == MAC_SHORT_ADDRESS)
        {
            mac_data.short_address_pending = 0;
        }
        else if (mac_address.address_mode == MAC_IEEE_ADDRESS)
        {
            mac_data.ieee_address_pending = 0;
        }
        return 1;
    }
#endif

    /* Now create the direct message */
    error = mac_frame_build_data_request( &dtx,
                                          &mac_address, &dst_address, /* source and dst address */
#ifdef MAC_CFG_SECURITY_ENABLED
                                          &sec_params,
#else
                                          NULL_POINTER,
#endif
                                          DATA_REQUEST
        );

    if( dtx == NULL_POINTER )
    {
		#if (DEBUG_FLAG == 1)
			log_item(0xAE);
		#endif
		if ( error == MAC_TRANSACTION_OVERFLOW )
        {
            /* failed to create a packet, so leave ready to try again next time around */
			#if (DEBUG_FLAG == 1)
				log_item(0xAF);
			#endif
            return 1;
        }
#ifdef MAC_CFG_SECURITY_ENABLED
        else if ( error != SUCCESS )
        {
            /* Report Error to NHL */
            send_mlme_comm_status_indication(&mac_address, &dst_address,
                                             error,
                                             &sec_params);

            /* Clear the Pending Flag as we do not want to retry sending this */
            if( mac_address.address_mode == MAC_IEEE_ADDRESS )
            {
                mac_data.ieee_address_pending = 0;
            }
            else
            {
                mac_data.short_address_pending = 0;
            }
            return 1;
        }
#endif
    }

    dtx->msdu_handle = 0;

    if( mac_address.address_mode == MAC_IEEE_ADDRESS )
    {
        mac_data.ieee_address_pending = 0;
    }
    else
    {
        mac_data.short_address_pending = 0;
    }

    /* indicate to DR-SM */
#if(CFG_MAC_DRSM_ENABLED == 1)      
    event.trigger = (sm_trigger_t) DRSM_TRIGGER_START;
    event.param.scalar = 0;
    SM_DISPATCH( (sm_t *) &drsm, &event );
#endif
    
    /* now queue the data request */
    if( mac_queue_direct_transmission( dtx ) != MAC_SUCCESS )
    {
        /*TBD Cancel DR-SM if queuing fails */
		#if (DEBUG_FLAG == 1)
			log_item(0xB0);
		#endif
        //return 1;
    }

    return 1;
}

/*****************************************************************************/

/*called from the foreground main loop to handle transmit messages*/
uchar process_completed_dtx_messages( void )
{
    mac_tx_t *dm = NULL_POINTER;
    mac_address_t dst_addr = {0};
    mac_address_t src_addr = {0};
    uchar processed = 0 ;
#if(CGF_MLME_BEACON_REQ_CONF == 1)
    uchar sub_type = 0; //Umesh : 03-01-2018 sub_type uncommented
#endif

#if(CFG_MLME_ASSOCIATE_REQ_CONF == 1)    
    sm_event_t event;
    sm_event_t event2;
#endif
       
    processed = 0;

    /* get a transmitted message from the queue */
    dm = (mac_tx_t *) queue_manager_pop_front( QUEUE_TX_DONE );

    if( dm == NULL_POINTER )
    {
        /* no messages, so exit */
		event_clear(FRAME_TX_DONE_EVENT);
		clear_process_activity( AF_TX_MSG_SENT_PENDING );
        return 0;
    }

	if(!queue_manager_size(QUEUE_TX_DONE))
	{
		event_clear(FRAME_TX_DONE_EVENT);
		clear_process_activity( AF_TX_MSG_SENT_PENDING );
	}

    /* check type of message */
    if((dm->type == MAC_FRAME_TYPE_DATA)
#ifdef MAC_CFG_GTS_ENABLED
       || (dm->type==TX_TYPE_GTS_DATA)
#endif
        )
    {
#ifdef MAC_CFG_GTS_ENABLED
      /* check if it was a GTS, and reload the expiration timer */
      if( dm->gts_data != NULL_POINTER )
      {
        dm->gts_data->expiration_timer = mac_get_gts_expiration_time();
      }
#endif
      /*Uemsh : 10-01-2018 this thing goes to modified after sepration of mac and fan mac*/
#ifdef WISUN_FAN_MAC
#if(APP_LBR_ROUTER == 1)      
      if((dm->sub_type != FAN_DATA_PKT) && 
         ((dm->sub_type == PAN_ADVERT_SOLICIT) || 
          (dm->sub_type == PAN_CONFIG) || 
            (dm->sub_type == PAN_ADVERT_FRAME) || 
              (dm->sub_type == PAN_ADVERT_FRAME) || 
                (dm->sub_type == PAN_CONFIG_SOLICIT)))  //extract these value in funciton --tbd
      {
        processed = send_ws_sync_result(dm);
        mac_mem_free_tx( dm );
        return 1;
      }
      else         
        processed = send_mcps_data_confirm( dm->msdu_handle, dm->status, dm->tx_timestamp,dm->num_csmaca_backoffs );
#else
        processed = send_mcps_data_confirm( dm->msdu_handle, dm->status, dm->tx_timestamp,dm->num_csmaca_backoffs );
#endif  //#if(APP_LBR_ROUTER == 1)     
#else
      processed = send_mcps_data_confirm( dm->msdu_handle, dm->status, dm->tx_timestamp,dm->num_csmaca_backoffs );
#endif  
      
    }
    else if( dm->type == MAC_FRAME_TYPE_MAC_COMMAND )
    {
        /* get addresses */
        //mac_frame_parse_addresses( dm->data->psdu, &dst_addr, &src_addr );

        /* assume this message gets processed */
        processed = 1;

        /* process command */
        switch( dm->cmd )
        {
#if(CFG_MLME_ASSOCIATE_REQ_CONF == 1)
        case ASSOCIATION_REQUEST:
#ifdef ENABLE_DEBUG_EVENTS
        	Indicate_Debug_Event( ASSOCIATION_REQUEST_SENT );
#endif

            /* indicate request sent to ASSOC-SM */
            event.trigger = (sm_trigger_t) ASSOCSM_TRIGGER_REQ_SENT;
            event.param.scalar = dm->status;
            SM_DISPATCH( (sm_t *) &assocsm, &event );
            break;
#endif	/*(CFG_MLME_ASSOCIATE_REQ_CONF == 1)*/
            
/*Uemsh : 10-01-2018 this thing goes to modified after sepration of mac and fan mac*/
#ifdef WISUN_FAN_MAC
        case FAN_PAN_ADV_SOLCIT:
          
        case FAN_PAN_CONF_SOLCIT:
          /* indicate request sent to SCAN-SM */
 #if(CFG_MAC_SCANSM_ENABLED == 1)           
            event.trigger = (sm_trigger_t) SCANSM_TRIGGER_REQ_SENT;
            event.param.scalar = 0;
            SM_DISPATCH( (sm_t *) &scansm, &event );
#endif            
          break;
#endif
#if( (CFG_ACTIVE_SCAN == 1) || (CFG_ENHANCED_ACTIVE_SCAN == 1) )            
        case BEACON_REQUEST:
#ifdef ENABLE_DEBUG_EVENTS
        	Indicate_Debug_Event( BEACON_REQUEST_SENT );
#endif
#endif	/*( CFG_ACTIVE_SCAN == 1 )*/

#if( CFG_ORPHAN_SCAN == 1 )
        case ORPHAN_NOTIFICATION:
#ifdef ENABLE_DEBUG_EVENTS
        	Indicate_Debug_Event( ORPHAN_NOTIFICATION_SENT );
#endif
#endif	/*( CFG_ORPHAN_SCAN == 1 )*/

#if( ( CFG_ACTIVE_SCAN == 1 ) || (CFG_ORPHAN_SCAN == 1 ) || (CFG_ENHANCED_ACTIVE_SCAN == 1) )
            /* indicate request sent to SCAN-SM */
            event.trigger = (sm_trigger_t) SCANSM_TRIGGER_REQ_SENT;
            event.param.scalar = 0;
            SM_DISPATCH( (sm_t *) &scansm, &event );
            break;
#endif	/*( ( CFG_ACTIVE_SCAN == 1 ) || (CFG_ORPHAN_SCAN == 1 ))*/

#if(CFG_MLME_ASSOCIATE_IND_RESP == 1)
        case ASSOCIATION_RESPONSE:
#ifdef ENABLE_DEBUG_EVENTS
        	Indicate_Debug_Event( ASSOCIATION_RESPONSE_SENT );
#endif

            /* send COMM-STATUS.indication */
            send_mlme_comm_status_indication( &src_addr, &dst_addr,
                                              dm->status,
                                              &dm->sec_param );
            break;
#endif	/*(CFG_MLME_ASSOCIATE_IND_RESP == 1)*/

#if((CFG_MLME_ORPHAN_IND_RESP == 1) || (CFG_MLME_START_REQ_CONF == 1))
        case COORDINATOR_REALIGNMENT:
#ifdef ENABLE_DEBUG_EVENTS
        	Indicate_Debug_Event( COORDINATOR_REALIGNMENT_SENT );
#endif

            if ( (dst_addr.address_mode == MAC_SHORT_ADDRESS) &&
                (dst_addr.address.short_address == BROADCAST_SHORT_ADDRESS) 
#if(CFG_MAC_SFTSM_ENABLED == 1)                  
                 && (startsm_p)
#endif                    
                    )
            {
#if(CFG_MAC_SFTSM_ENABLED == 1)               
                /* result of START.request, so indicate to START-SM */
                event.trigger = (sm_trigger_t) STARTSM_TRIGGER_CRA_COMPLETE;
                 event.param.scalar = dm->status;
//#endif                
//               
//#if(CFG_MAC_SFTSM_ENABLED == 1)                  
                SM_DISPATCH( (sm_t *) startsm_p, &event );
#endif                
            }
#if(CGF_MLME_COMM_STATUS_IND == 1)
            else
            {
                /* result of ORPHAN.response, so send COMM-STATUS.indication */
                send_mlme_comm_status_indication( &src_addr,
                                                  &dst_addr,
                                                  dm->status,
                                                  &dm->sec_param );
            }
#endif
            break;
#endif	/*((CFG_MLME_ORPHAN_IND_RESP == 1) || (CFG_MLME_START_REQ_CONF == 1))*/

#if(CFG_MLME_DISASSOCIATE_REQ_CONF == 1)
        case DISASSOCIATION_NOTIFICATION:
#ifdef ENABLE_DEBUG_EVENTS
        	Indicate_Debug_Event( DISASSOCIATION_NOTIFICATION_SENT );
#endif

            /* This message can either transmitted by a coordinator to a device, or by a device to
             * its coordinator. The behaviour is different dependeing on the scenario, which can be determined
             * by the destination address for the message.
            */
            if( (((dst_addr.address_mode == 2) && (dst_addr.address.short_address == mac_pib.CoordShortAddress)) )
                || ((dst_addr.address_mode == 3)
                && (ieeeaddr_cmp(dst_addr.address.ieee_address, mac_pib.CoordExtendedAddress) == 0) ))
            {
                /* sent to our coordinator (with short or IEEE address) */

#if(CFG_MLME_SYNC_REQ == 1)
                /* stop tracking beacons */
                event.trigger = (sm_trigger_t) SYNCSM_TRIGGER_CANCEL;
                event.param.scalar = 0;
                SM_DISPATCH( (sm_t *) &syncsm, &event );
#endif
                /* send event to ASSOC-SM */
                event.trigger = (sm_trigger_t) ASSOCSM_TRIGGER_DISASSOCIATE;
                //event.param.scalar = 0;
                SM_DISPATCH( (sm_t *) &assocsm, &event );

                /* send confirm primitive */
                send_mlme_disassociate_confirm( dm->status, &src_addr );
            }
            else
            {
                /* we are a coordinator, so send to the NHL */
                send_mlme_disassociate_confirm( dm->status, &dst_addr );
            }
            break;
#endif	/*(CFG_MLME_DISASSOCIATE_REQ_CONF == 1)*/

        case DATA_REQUEST:
#ifdef ENABLE_DEBUG_EVENTS
        	Indicate_Debug_Event( DATA_REQUEST_SENT );
#endif

            if( dm->status == MAC_SUCCESS &&
                dm->tx_options & TX_OPTION_FRAME_PENDING_IN )
            {
#if(CFG_MAC_DRSM_ENABLED == 1)              
                event.trigger = (sm_trigger_t) DRSM_TRIGGER_RESP_PENDING;
                event2.trigger = (sm_trigger_t) ASSOCSM_TRIGGER_RESP_PENDING;
#endif                
            }
            else
            {
#if(CFG_MAC_DRSM_ENABLED == 1)              
                event.trigger = (sm_trigger_t) DRSM_TRIGGER_RESP_NOPENDING;
                event2.trigger = (sm_trigger_t) ASSOCSM_TRIGGER_RESP_NOPENDING;
#endif                
            }
#if(CFG_MAC_DRSM_ENABLED == 1)
            /* indicate result to DR-SM */
            event.param.scalar = dm->status;
            SM_DISPATCH( (sm_t *) &drsm, &event );
             
#endif
           
           
#if(CFG_ASSOCSM_ENABLED == 1)
             /* indicate result to ASSOC-SM */
            event2.param.scalar = 0;            
            SM_DISPATCH( (sm_t *) &assocsm, &event2 );
#endif            
            break;
        
        case PAN_ID_CONFLICT_NOTIFICATION:
#ifdef ENABLE_DEBUG_EVENTS
        	Indicate_Debug_Event( PAN_ID_CONFLICT_NOTIFICATION_SENT );
#endif
        	/* After detecting the PANID Conflict by the FFD and transmitting
        	the PANID_CONFLICT_NOTIFY command to the Coordinator, on receiving 
        	the ack the MLME shall issue MLME_SYNC_LOSS_INDICATION with LossReason
        	set to PANID_CONFLICT 
        	*/
#if(CFG_MLME_SYNC_LOSS_IND == 1)
        	if(dm->status == MAC_SUCCESS)
        	{
        		send_mlme_sync_loss_indication( MAC_PAN_ID_CONFLICT, &dm->sec_param );	
        	}      	
#endif	/*(CFG_MLME_SYNC_LOSS_IND == 1)*/
        	break;

#if ( ( MAC_CFG_LE_RIT_CAPABILITY == 1 ) || ( MAC_CFG_LE_CSL_CAPABILITY == 1 ) )
        case LE_RIT_DATA_REQUEST:
        	/* indicate result to LE-SM */
        	event.trigger = (sm_trigger_t) LE_RIT_TRIGGER_RIT_CMD_TX_COMPLETE;
            event.param.scalar = dm->status;
            SM_DISPATCH( (sm_t *) &low_energy, &event );
        	break;
#endif
        default:
            /* it should have been one of the above commands! */
            break;

        }
    }   
#if(CGF_MLME_BEACON_REQ_CONF == 1)
    else if ( dm->type == MAC_FRAME_TYPE_BEACON )
    {
        /* it's a beacon */
        processed = 1;
        sub_type = dm->sub_type;

		send_mlme_beacon_confirm( dm->status );

		/* increment BSN or EBSN accordingly */                       
		if( sub_type == MAC_FRAME_BCN_SUB_TYPE_RB )
		{
			 mac_pib.BSN += 1;
		}
		else
		{
			mac_pib.EBSN += 1;
		}
		
		dm->cap_retries = 0;

		if( !(mac_pib.mac_security_enabled) )
		{
			/* requeue new beacon. Do not construct and keep 
			the Enhanced beacons, as the contents of EB are 
			determined by the EBR */
			if( sub_type == MAC_FRAME_BCN_SUB_TYPE_RB )
			{
				dm->data->psdu[2] = mac_pib.BSN;
				mac_queue_beacon_transmission( dm,sub_type );
			}
			else
			{
				/*After transmitting EBs should be freed. New 
				EBs should not constructed as the contents are 
				determined by the receiving EBR.*/
				mac_mem_free_tx(dm);
			}							
		}
		else
		{
			/*since secured beacons are being sent, they have 
			to be re constructed. So free up the sent beacon 
			and construct it newly. Contruct a new beacon and 
			not a EB*/
			mac_mem_free_tx(dm);
                    
#if(CFG_MAC_BEACON_ENABLED == 1) 			
			if( sub_type == MAC_FRAME_BCN_SUB_TYPE_RB )
			{
				mac_beacon_update( sub_type );	
			}
#endif  //#if(CFG_MAC_BEACON_ENABLED == 1)                      
		}
		
		return 1;
    }
#endif	/*(CGF_MLME_BEACON_REQ_CONF == 1)*/
    else
    {
    	/* something from Mars...*/
    	processed = 1;
    }

    if( processed )
    {
        mac_mem_free_tx( dm );
    }
    else
    {
        set_process_activity( AF_TX_MSG_SENT_PENDING );
        queue_manager_push_front( QUEUE_TX_DONE, (queue_item_t *) dm );
    }
    /* possibly more to do */
    return 1;
}
/*****************************************************************************/

#if SUPPORT_PROCESSOR_MODE

void mac_set_processor_mode( void )
{
    extern uint32_t processor_active;
    
    if( flags != processor_active )
    {
        flags = processor_active;
    }

    /* FIXME: disable DEEP SLEEP mode until we can make it work */
    //mac_pib.ConfigFlags |= DISABLE_DEEP_SLEEP ;// | DISABLE_POWER_SAVE_MODE;

    if( processor_active & AF_32KHZ_TICK )
    {
        clear_process_activity( AF_32KHZ_TICK );
    }

    /* there are 3 power save modes -
     * deep sleep  - where the clock is stopped and the RAM is saved to 1K
     * sleep - where the clock is stopped and...
     *  snooze - where the 12MHz continues to run, but the micro snoozes
     */
    if( processor_active == 0 )
    {
        /* test the conditions under which we can enter deep sleep mode - the least power consumption */
        if( deep_sleep_mode_ok() )
        {
            /* kill the 12MHz timer here */
            timer_12mhz_initialise();
            disable_12MHz_clock();
        }
        /* Sleep mode is not as low powered as deep sleep but is still better than idle */
        else if( sleep_mode_ok() )
        {
            /* throw a system call to set the processor into sleep mode right now! */
            disable_12MHz_clock();
            cpu_sleep();
        }
        else if( !(mac_pib.ConfigFlags & DISABLE_SNOOZE ) )
        {
            /* throw a system call to set the processor into sleep mode ensuring the clcok is running */
            enable_12MHz_clock();
            cpu_sleep();
        }
    }
    else
    {
        if( last_active !=  processor_active )
        {
            last_active = processor_active;
        }
    }
}

#endif

/*****************************************************************************/

#ifdef MAC_CFG_GTS_ENABLED
/*check for expired GTS's*/
static void mac_check_expired_gts(void)
{
    uchar i = 0;
    uchar gts = 0;
    gts_data_t *gd =  NULL;

    if( !mac_data.coordinator )
    {
        disable_interrupt();

        /* first check pending slave GTS allocations */
        if( mac_data.gts_rx_request_pending && (mac_data.gts_rx_timer == 0) )
        {
            mac_data.gts_rx_request_pending = 0;
            enable_interrupt();
            /* we were expecting a response by now so send the confirm */
            send_mlme_gts_confirm( mac_data.gts_rx_characteristics, MAC_NO_DATA );
            disable_interrupt();
        }

        if( mac_data.gts_tx_request_pending && (mac_data.gts_tx_timer == 0) )
        {
            mac_data.gts_tx_request_pending = 0;
            enable_interrupt();
            /* we were expecting a response by now so send the confirm */
            send_mlme_gts_confirm( mac_data.gts_tx_characteristics, MAC_NO_DATA );
            disable_interrupt();
        }
        enable_interrupt();

        if( (mac_data.gts_tx_queue.count == 0)
            && !mac_data.gts_rx_request_pending
            && !mac_data.gts_tx_request_pending )
        {
            clear_process_activity( AF_GTS_ACTIVITY_PENDING );
        }

        return;
    }

    /* coordinator actions */
    if( mac_data.check_gts_timers )
    {
        disable_interrupt();
        mac_data.check_gts_timers = 0;
        enable_interrupt();
        /* check for expired GTS's */
        for( i = 0, gd = mac_data.gts_params; i < MAX_GTS_DESCRIPTORS; i++, gd++ )
        {
            /* check if this is used */
            if( gd->gts_state != GTS_ALLOCATED )
            {
                /* no, go to the next one */
                continue;
            }
            /* it is used, so check the expiration timer */
            if( gd->expiration_timer != 0 )
            {
                disable_interrupt();
                --gd->expiration_timer;
                enable_interrupt();
                if( gd->expiration_timer == 0 )
                {
                    ushort short_address;
                    /* this has expired, so deallocate */
                    if( gd->direction ) /* check the direction */
                    {
                        /* Receive only */
                        gts = GTS_DIR_RX_ONLY | GTS_TYPE_DEALLOCATE;
                    }
                    else
                    {
                        gts = GTS_DIR_TX_ONLY | GTS_TYPE_DEALLOCATE;
                        /* there may be messages to purge from the GTS queue */
                        gts_purge( gd );
                    }

                    gts |= ((gd->gts_desc.length_and_start_slot>>4) & 0x0f);

                    short_address = gd->gts_desc.device_short_address[0] | (gd->gts_desc.device_short_address[1] << 8);
                    mac_gts_allocate( short_address, &gts );
                    /* FIXME: Spec is not clear as to what the first three arguments should be */
                    send_mlme_gts_indication( mac_pib.PANId,
                                              (address_t *)&mac_pib.ShortAddress,
                                              MAC_SHORT_ADDRESS,
                                              short_address,
                                              gts,
                                              0, 0, NULL_POINTER );
                } /* *gd->expiration_timer == 0 )*/
            } /* ( gd->expiration_timer != 0 ) */
        } /* for() */
        clear_process_activity( AF_GTS_ACTIVITY_PENDING );

    } /* check GTS timers */
}


/*****************************************************************************/

/*purges messages destined for an expired GTS */
static void gts_purge( gts_data_t *gd )
{
    mac_tx_t *gtx = NULL;
    mac_tx_t *next_gtx = NULL;

    /* loop testing for GTS messages which may be destined for the specified GTS */
    for( gtx = (mac_tx_t *) queue_manager_scan_next( QUEUE_GTS_TX, NULL_POINTER );
         gtx != NULL_POINTER;
         gtx =  (mac_tx_t *)queue_manager_scan_next( QUEUE_GTS_TX, (queue_item_t *)gtx))
    {
        /* check for a match */
        if( gtx->gts_data == gd )
        {
            /* found */
            next_gtx = (mac_tx_t *) gtx->q_item.link;
            /* remove it from the queue */
            mlist_manager_remove( QUEUE_GTS_TX, (queue_item_t *) gtx );
            /* and inform the network layer that we couldn't send it */
            mac_gts_msg_complete( gtx, MAC_INVALID_GTS );
            gtx = next_gtx;
        }
    }
}
#endif

/*****************************************************************************/

/*expires pending messages which may have timed out.
 * return 0 if no more messages, else 1*/
uchar mac_check_expired_pending_messages( void )
{
    mac_tx_t *txd = NULL; /* pointer for the indirect tx table entries */
    mac_tx_t *next_txd = NULL;
    uchar retval = 0;
    uchar expired = 0;

    /* now check pending messages - there won't be any IF this is a slave device */
    for( txd = (mac_tx_t*) queue_manager_scan_next( QUEUE_INDIRECT_TX, NULL_POINTER );
         txd != NULL_POINTER; )
    {
        /* find the next item BEFORE removing an old item */
        next_txd = (mac_tx_t*) queue_manager_scan_next( QUEUE_INDIRECT_TX, (queue_item_t*) txd);

        if( txd->persistence_timer == 0 )
        {
           queue_manager_remove( QUEUE_INDIRECT_TX, (queue_item_t *) txd );

            expired = 1;
#if(CFG_MAC_PENDADDR_ENABLED == 1)   
            /* update Pending Address List */
            if( pendaddr_remove( txd ) != PENDADDR_SUCCESS )
            {
                /*TBD What to do here? */
                //debug(("pendaddr err\r\n"));
            }
#endif
            mac_cap_msg_complete( txd, MAC_TRANSACTION_EXPIRED );
        }
        txd = next_txd;
        retval = 1;
    }

    /* update beacon */
    if( expired )
    {
#if(CFG_MAC_BEACON_ENABLED == 1)      
        if( mac_beacon_update(MAC_FRAME_BCN_SUB_TYPE_RB) != MAC_SUCCESS )
        {
            /*TBD What to do here? */
            //debug(("bcn update err\r\n"));
        }
#endif   //#if(CFG_MAC_BEACON_ENABLED == 1)       
    }
    return retval;
}

/*****************************************************************************/
uchar mac_check_cap_message_bcast(void)
{
  uchar msg_count = 0;
  sm_event_t event;
  if( (msg_count = queue_manager_size(QUEUE_BCAST)) != 0 )
  {   
    if (trxsm_get_state (&trxsm) == TRXSM_STATE_IDLE)
    {
#if (PRINT_DEBUG_LEVEL==ERROR_DEBUG)	
      if (ccasm_is_idle (trxsm.ccasm) == 0)
      {
        stack_print_debug ("2 CCASM state recovered");
      }
#endif 	  
      
      //ccasm_go_to_idle ((ccasm_t *)(trxsm.ccasm));
      event.trigger = (sm_trigger_t) TRXSM_TRIGGER_NEW_BCAST_PACKET;
      event.param.scalar = 0;
      SM_DISPATCH((sm_t *)&trxsm, &event);
    }
  }
   return msg_count;
}
/*called from the mac main loop to check for a CAP message,return number of CAP messages*/
uchar mac_check_cap_message_ucast(void)
{
  uchar msg_count = 0;
  sm_event_t event;
  
#if (CFG_MAC_DRSM_ENABLED == 1)
  /* don't send anything if some indirect data is pending */
  if(( drsm_get_state( &drsm ) == DRSM_STATE_PENDING )|| ( ccasm_is_suspended( ccasm_p )))  
    {
      return 0;
    }
#endif
  
  if( (msg_count = queue_manager_size(QUEUE_CAP_TX)) != 0 )
  {    
    if (trxsm_get_state (&trxsm) == TRXSM_STATE_IDLE)
    {
#if (PRINT_DEBUG_LEVEL==ERROR_DEBUG)
      if (ccasm_is_idle (trxsm.ccasm) == 0)
      {
        stack_print_debug ("1 CCASM state recovered");
      }
#endif 
      
      //ccasm_go_to_idle ((ccasm_t *)(trxsm.ccasm));
      event.trigger = (sm_trigger_t) TRXSM_TRIGGER_NEW_DIRECT_PACKET;
      event.param.scalar = 0;
      SM_DISPATCH((sm_t *)&trxsm, &event);
    }
    else
      return 0;
  }
  
//  if (msg_count == 0)
//  {
//    event_clear(PENDING_TX_EVENT);
//  }
  
  return msg_count;
}

/*****************************************************************************/
#if(CFG_MLME_RX_ENABLE_REQ_CONF == 1)
/*called from the mac main loop to check the rx enable state,return always returns 0*/
uchar mac_process_receiver_enable_state( void )
{
    ulong current_symbol = 0;

    if( mac_pib.BeaconOrder != 15 )
    {
        current_symbol = mac_pib.BeaconTxTime;
    }
    else
    {
        /* FIXME: current_symbol = mac_data.current_backoff_slot*20; */
    }

    switch( mac_data.rx_enable_state )
    {
        case RX_ENABLE_IDLE:
            /* nothing to do */
			clear_process_activity( AF_RX_ENABLE_ACTIVE );
            break;

        case RX_ENABLE_DEFERRED:
            /* handled in slot interrupt service */
            break;

        case RX_ENABLE_DELAYING:
        {
            /* check if we should turn the receiver on now */
            if( current_symbol >= mac_data.rx_enable_delay_timer )
            {
                /* adjust for absolute time to turn off */
                if( plme_get_trx_state_request() == PHY_TX_ON )
                {
                    mac_data.rx_enable_state = RX_ENABLE_IDLE;
					#if(CFG_MLME_RX_ENABLE_REQ_CONF == 1)
						send_mlme_rx_enable_confirm( MAC_TX_ACTIVE );
					#endif
                }
                else
                {
                    /*TBD replace with event to trx state machine */
                  
//#if (PRINT_DEBUG_LEVEL==ERROR_DEBUG)
//    stack_print_debug ("9 phy state = %d\n", plme_get_trx_state_request ());
//#endif                   
                  
                    PLME_Set_TRX_State(PHY_RX_ON);
                    
//#if (PRINT_DEBUG_LEVEL==ERROR_DEBUG)
//    stack_print_debug ("A phy state = %d\n", plme_get_trx_state_request ());
//#endif                       
                    
                    mac_data.rx_enable_timer += mac_data.rx_enable_delay_timer;
                    mac_data.rx_enable_active = 1;
                    mac_data.rx_enable_state = RX_ENABLE_ACTIVE;
					#if(CFG_MLME_RX_ENABLE_REQ_CONF == 1)
						send_mlme_rx_enable_confirm( MAC_SUCCESS );
					#endif
                }
            }
            break;
        }

        case RX_ENABLE_ACTIVE:
        {
            /* check if we should turn the receiver off now */
            if( current_symbol >= mac_data.rx_enable_timer )
            {
                mac_data.rx_enable_state = RX_ENABLE_IDLE;
            }
            break;
        }

        default:
        {
            mac_data.rx_enable_state = RX_ENABLE_IDLE;
            break;
        }
    }
    return 0;
}
#endif	/*(CFG_MLME_RX_ENABLE_REQ_CONF == 1)*/

/*****************************************************************************/

/*checks panid conflict status*/
uchar mac_check_panid_conflict_state( void )
{
    //mac_address_t src, dst;
    //security_params_t sec_param;
    uchar status = MAC_SUCCESS;

    /* deal with a pan ID conflict if detected */
    if( mac_data.panid_conflict_state == PANID_CONFLICT_DETECTED )
    {
        
#if( (CFG_PANID_CD_BY_PANC == 1 )|| (CFG_MAC_STARTSM_ENABLED == 1)  )
        if( ( startsm_get_flags( startsm_p ) & STARTSM_FLAG_COORD_MASK ) == STARTSM_FLAG_PANCOORD )
        {
#if(CFG_MLME_SYNC_LOSS_IND == 1)
			/* We Detected the Conflict - Security Params are NULL */
            send_mlme_sync_loss_indication( MAC_PAN_ID_CONFLICT, NULL_POINTER );
#endif	/*(CFG_MLME_SYNC_LOSS_IND == 1)*/

			mac_data.panid_conflict_state = PANID_CONFLICT_REPORTED;
        }
        else
#endif	/*(CFG_PANID_CD_BY_PANC == 1)*/
        {
#if(CFG_MAC_SCANSM_ENABLED == 1)           
            if( scansm_get_state( &scansm ) == SCANSM_STATE_NONE )
			{
				status = mac_create_panid_conflict_notification();

				mac_data.panid_conflict_state = PANID_CONFLICT_REPORTED;

			}
#endif //#if(CFG_MAC_SCANSM_ENABLED == 1)             
        }
    }
    return status;
}

/*****************************************************************************/

#ifdef MAC_CFG_SECURITY_ENABLED
uchar load_auto_request_sec_params( security_params_t* p_sec_params )
{

	security_pib_t* lp_security_data = &(mac_security_data.pib);
	
	if ( (lp_security_data->auto_request_security_level != MAC_SECURITY_NONE)
         && ( mac_pib.ConfigFlags & ( USE_2006_PRIMITIVES|USE_2011_PRIMITIVES ) ) )
    {
        p_sec_params->security_level = lp_security_data->auto_request_security_level;
        p_sec_params->key_id_mode = lp_security_data->auto_request_keyid_mode;
        
    	if (lp_security_data->auto_request_keyid_mode == 1)
        {
            /* KEY_IDENTIFIER - 1 octet Key Index */
            p_sec_params->key_identifier[0] = lp_security_data->auto_request_key_index;
        }
        else if (lp_security_data->auto_request_keyid_mode == 2)
        {
            /* KEY_IDENTIFIER - 4 octet Key Source + 1 octet Key Index */
            memcpy(p_sec_params->key_identifier, lp_security_data->auto_request_key_source, 4);
            p_sec_params->key_identifier[4] = lp_security_data->auto_request_key_index;
        }
        else if (mac_security_data.pib.auto_request_keyid_mode == 3)
        {
            /* KEY_IDENTIFIER - 8 octet Key Source + 1 octet Key Index */
            memcpy(p_sec_params->key_identifier, lp_security_data->auto_request_key_source,8);
            p_sec_params->key_identifier[8] = lp_security_data->auto_request_key_index;
        }

		return 1;
    }

	return 0;
}
#endif

/*****************************************************************************/

/*processes mcps messages coming into the MAC*/
void process_mcps_msgs(void)
{
  msg_t *mac_msg = NULL;
  uchar result = 2; /*so that msg gets freed if the primitive is not supported*/
  
  /* loop processing MAC messages while there are any */
  for (;;)
  {
    mac_msg = get_mcps_msg();
    
    if (mac_msg == NULL_POINTER)
    {
      /* no more pending messages */
      event_clear(MCPS_EVENT);
      clear_process_activity(AF_MCPS_MSG_PENDING);
      return;
    }
    
    if(!queue_manager_size(QUEUE_NHLE_2_MCPS))
      event_clear(MCPS_EVENT);    
    
    if (MAC_MCPS_DATA_REQUEST == mac_msg->data[0])
      result = process_mcps_data_request (&mac_msg->data[1], 
                                          mac_msg->data_length - 1);
#ifdef WISUN_FAN_MAC        
    else if (FAN_MAC_MCPS_DATA_REQUEST == mac_msg->data[0])
      result = process_fan_mcps_data_request (&mac_msg->data[1], 
                                              mac_msg->data_length - 1);
#endif
#if (CFG_MCPS_PURGE_REQ_CONF == 1)        
    else
      result = process_mcps_purge_request (&mac_msg->data[1], 
                                           mac_msg->data_length - 1);
#endif        
    
    switch ( result )
    {
    case 0:
      /* not processed due to insufficient resources, so save it for later */
      requeue_mcps_msg(mac_msg);
      return;
    case 1:
      /* processed OK */
    case 2:
      free_nhle_2_mac_msg(mac_msg);
      break;
    }           /* end of switch */
  }             /* end of for(;;) */
}               /* end of process_mcps_msgs */

/*****************************************************************************/

/*processes mlme messages coming into the MAC*/
void process_mlme_msgs(void)
{
    msg_t *mac_msg = NULL;
    uchar result = 0;

    /* loop processing MAC messages while there are any */
    for (;;)
    {
        mac_msg = get_mlme_msg();

        if (mac_msg == NULL_POINTER)
        {
			/* no more pending messages */
			event_clear(MLME_EVENT);
			clear_process_activity(AF_MLME_MSG_PENDING);
			return;
        }

		if(!queue_manager_size(QUEUE_NHLE_2_MLME))
		{
			event_clear(MLME_EVENT);
		}


        /* process the primitive and check if successful */
        result = mac_process_primitive(mac_msg->data, mac_msg->data_length);

        switch ( result )
        {
        case 0:
            /* not processed due to insufficient resources, so save it for later */
            requeue_mlme_msg(mac_msg);
            return;

        case 1:
            /* processed OK */
        case 2:
            free_nhle_2_mac_msg(mac_msg);
            break;
        } /* end of switch */
    } /* end of for(;;) */
} /* end of process_mlme_msgs */

/*****************************************************************************/

msg_t* allocate_nhle_2_mac_msg( uint16_t length )
{
    msg_t *msg = NULL;

    msg = (msg_t *) app_bm_alloc( length + 6 );// 5 for including next and data_length members

    if (msg != NULL)
    {
        msg->data_length = length;
    }
    return msg;
}

/*****************************************************************************/

void free_nhle_2_mac_msg( msg_t * msgp )
{
    app_bm_free((uint8_t*)msgp);
}

/*****************************************************************************/

void send_nhle_2_mcps( msg_t * msgp )
{
    set_process_activity(AF_MCPS_MSG_PENDING);
    queue_manager_push_back(QUEUE_NHLE_2_MCPS, (queue_item_t *) msgp);
    //signal_mac_layer_event();
}

/*****************************************************************************/

void send_nhle_2_mlme( msg_t * msgp )
{
    set_process_activity(AF_MLME_MSG_PENDING);
    queue_manager_push_back(QUEUE_NHLE_2_MLME, (queue_item_t *) msgp);
    //signal_mac_layer_event();
}

/*****************************************************************************/

/*
** ============================================================================
** Private Function Definitions
** ============================================================================
*/

#if SUPPORT_PROCESSOR_MODE

static uchar deep_sleep_mode_ok( void )
{
    if( (mac_pib.BeaconOrder == 0x0f) /* beaconless */
        && ( plme_get_trx_state_request() == PHY_TRX_OFF ) /* not tx or waiting for incoming message */
        && !mac_data.coordinator /* this is a not a coordinator */
        && !app_clock_active()  /* the application clock is not running */
        && !(mac_pib.ConfigFlags & DISABLE_DEEP_SLEEP)/* disable deep_sleep */
        && sleep_delay_expired()
        )
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

/*****************************************************************************/

static uchar sleep_mode_ok( void )
{
    if( (mac_pib.BeaconOrder == 0x0f) /* beaconless */
        && ( plme_get_trx_state_request() == PHY_TRX_OFF ) /* not tx or waiting for incoming message */
        && !mac_data.coordinator /* this is a not a coordinator */
        && !app_clock_active()  /* the application clock is not running */
        && !(mac_pib.ConfigFlags & DISABLE_SLEEP)/* disable deep_sleep */
        && sleep_delay_expired()
        )
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

#endif

/*****************************************************************************/

static void mac_if_msg_initialise(void)
{
    //uchar i;
    queue_manager_initialise( QUEUE_NHLE_2_MLME );
    queue_manager_initialise( QUEUE_NHLE_2_MCPS );
}

/*****************************************************************************/

static msg_t *get_mlme_msg(void)
{
    return (msg_t *) queue_manager_pop_front(QUEUE_NHLE_2_MLME);
}

/*****************************************************************************/

static msg_t *get_mcps_msg(void)
{
	return (msg_t *) queue_manager_pop_front(QUEUE_NHLE_2_MCPS);
}

/*****************************************************************************/

static void requeue_mlme_msg( msg_t * msgp )
{
    queue_manager_push_front(QUEUE_NHLE_2_MLME, (queue_item_t *) msgp);

	event_set(MLME_EVENT);
	set_process_activity(AF_MLME_MSG_PENDING);
	//signal_event_to_mac_task();
}

/*****************************************************************************/

static void requeue_mcps_msg( msg_t * msgp )
{
        queue_manager_push_front(QUEUE_NHLE_2_MCPS, (queue_item_t *) msgp);
	event_set(MCPS_EVENT);
	set_process_activity(AF_MCPS_MSG_PENDING);
	//signal_event_to_mac_task();
}

/******************************************************************************/

uint32_t get_the_frame_counter_from_mac ()
{
  
#ifdef MAC_CFG_SECURITY_ENABLED
  return mac_security_data.pib.mac_frame_counter;
#else
  
  return 0;
#endif
  
  
}


/*
** ============================================================================
** Public Function Definitions
** ============================================================================
*/

#ifdef WISUN_FAN_MAC
void start_async_pa_timer (void);
void start_async_pc_timer (void);
void start_async_pas_timer (void);
void start_async_pcs_timer (void);
uchar send_ws_async_confirm( uchar , uchar );
void send_another_pkt_on_channel_hop(uint8_t pkt_type);
uint8_t get_async_sent_count (uint8_t sub_type);

uchar send_ws_sync_result(mac_tx_t *dm)
{
  if(dm->sub_type==PAN_ADVERT_SOLICIT)  // pas indication
  {
    send_ws_async_confirm(dm->status,PAN_ADVERT_SOLICIT);
    if (((1 + get_async_sent_count (PAN_ADVERT_SOLICIT)) % 5) == 0)
      start_async_pas_timer ();
    else
      send_another_pkt_on_channel_hop (PAN_ADVERT_SOLICIT);
  }
  else if(dm->sub_type==PAN_ADVERT_FRAME)  // pa indication
  {
    send_ws_async_confirm(dm->status,PAN_ADVERT_FRAME);
    if (((1 + get_async_sent_count (PAN_ADVERT_FRAME)) % 5) == 0)
      start_async_pa_timer ();
    else
      send_another_pkt_on_channel_hop (PAN_ADVERT_FRAME);
  } 
  else if(dm->sub_type==PAN_CONFIG) // pc indication
  {
    send_ws_async_confirm(dm->status,PAN_CONFIG);
    if (((1 + get_async_sent_count (PAN_CONFIG)) % 5) == 0)
      start_async_pc_timer ();
    else
      send_another_pkt_on_channel_hop (PAN_CONFIG);
  } 
  else if(dm->sub_type==PAN_CONFIG_SOLICIT) // pcs indication
  {
    send_ws_async_confirm(dm->status,PAN_CONFIG_SOLICIT);
    if (((1 + get_async_sent_count (PAN_CONFIG_SOLICIT)) % 5) == 0)
      start_async_pcs_timer ();
    else
      send_another_pkt_on_channel_hop (PAN_CONFIG_SOLICIT);
  }   
  return 1;
}

void trigger_pa_frame_on_timer (void *a)
{
  send_another_pkt_on_channel_hop(PAN_ADVERT_FRAME);
}
void trigger_pc_frame_on_timer (void *a)
{
  send_another_pkt_on_channel_hop(PAN_CONFIG);
}
void trigger_pas_frame_on_timer (void *a)
{
  send_another_pkt_on_channel_hop(PAN_ADVERT_SOLICIT);
}
void trigger_pcs_frame_on_timer (void *a)
{
  send_another_pkt_on_channel_hop(PAN_CONFIG_SOLICIT);
}

#endif  


/*
FAN_MAC_INTERFACE.c file content
*/


#ifdef WISUN_FAN_MAC

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

static bool fan_mac_started =0;
/*
** =============================================================================
** Public Variable Definitions
** =============================================================================
**/

fan_mac_information_sm_t fan_mac_information_data;


/*
** ============================================================================
** External Variable Declarations
** ============================================================================
*/
extern fan_pkt_tmr_t pcs_timer;
extern fan_pkt_tmr_t pa_timer;
extern fan_pkt_tmr_t pas_timer;
extern fan_pkt_tmr_t pc_timer;
extern void find_lowest_pancost_from_nbr_table_for_mac_address(uint8_t *src_addr);

#if WITH_SECURITY
//extern uint8_t router_addr[8]; //Umesh for temp
extern uint8_t neighbour_addr[8];
#endif

/*
** ============================================================================
** Private Function Prototypes
** ============================================================================
*/
static sm_result_t fan_node_idle(fan_mac_information_sm_t *s, const sm_event_t *e);
static sm_result_t fan_node_startup(fan_mac_information_sm_t *s, const sm_event_t *e);
static sm_result_t node_state_1(fan_mac_information_sm_t *s, const sm_event_t *e);
static sm_result_t node_state_2(fan_mac_information_sm_t *s, const sm_event_t *e);
static sm_result_t node_state_3(fan_mac_information_sm_t *s, const sm_event_t *e);
static sm_result_t node_state_4(fan_mac_information_sm_t *s, const sm_event_t *e);
static sm_result_t node_state_5(fan_mac_information_sm_t *s, const sm_event_t *e);
static uint8_t process_pas_msg(mac_rx_t* rx_pas_msg);
static uint8_t process_pa_msg(mac_rx_t* rx_pa_msg);
static uint8_t process_pc_msg(mac_rx_t* rx_pc_msg);
static uint8_t process_pcs_msg(mac_rx_t* rx_pcs_msg);
int get_join_state (void) ;
void kill_process_and_clean_rpl_nbr();
        
#if(FAN_EAPOL_FEATURE_ENABLED == 1)
void get_eapol_parent(uint8_t *eapol_parent);
void process_eapol_msg(mac_rx_t* rx_eapol_msg);
#if APP_LBR_ROUTER
parent_child_info_tag eapol_parent_child_info;
#endif
void set_eapol_parent_unresponsive (uint8_t *addr);
#endif
/*
** ============================================================================
** Public Function Declarations
** ============================================================================
*/

uint8_t get_dev_mac_join_state(void);
void send_trickle_timer_pkt(fan_pkt_tmr_t trickle_timer);
void process_ws_async_frame(mac_rx_t* mac_rec_msg);
uint8_t process_fan_mac_data(mac_rx_t* rx_data_msg);
void create_rand_delay_in_us();
uint8_t hw_tmr_rand( void *hw_tmr_ins );
uint8_t find_best_pan();
void reset_to_join_state_1 (void);

/*----------------------------------------------------------------------------*/
/*
** ============================================================================
** Extern Function Declarations
** ============================================================================
*/

extern void node_start_upper_layer_ready(void);
extern unsigned short l3_random_rand(void);
extern void update_pan_id_from_border_router(uint16_t pan_id);
extern void process_schedule_end_pas();
extern unsigned short l3_random_rand(void);
extern void broadcast_shedule_start(void);
extern void app_bm_free(uint8_t *pMem );
extern void send_mlme_async_frame_indicaiton(uint8_t* src_mac_addr , uint8_t frame_type , uint8_t status );
extern uint8_t total_channel_lenth_uicast;
extern uint16_t find_lowest_pancostfrom_nbr_table();
extern void chack_node_goes_in_consis_inconsi(mac_rx_t *mrp);
extern void add_dev_desc_on_MAC_for_security(uint8_t* macAddrOfNeighbour);

#if(APP_NVM_FEATURE_ENABLED == 1)
extern void nvm_store_mac_white_list_info( void );
#endif


/*
** ============================================================================
** Private Function Definitions
** ============================================================================
*/


static sm_result_t fan_node_idle(fan_mac_information_sm_t *s, const sm_event_t *e)
{
    switch((fan_mac_manager_sm_trigger_t) e->trigger)
    {
      case FAN_MAC_TRIGGER_ENTRY:
        s->state_ind = FAN_MAC_IDLE_STATE;
      break;
      
      case FAN_MAC_TRIGGER_EXIT:
        s->previous_state =  s->state_ind;
      break;
      
      default:
      break;
    } 
    return NULL;  
}

/*----------------------------------------------------------------------------*/
void initialise_nbr_table (void);
extern void set_mac_security_on_LBR(uint8_t *rec_buff, uint16_t len);
extern void set_mac_security_on_router_node(uint8_t *rec_buff, uint16_t len);
/*----------------------------------------------------------------------------*/

static sm_result_t fan_node_startup(fan_mac_information_sm_t *s, const sm_event_t *e)
{
    switch((fan_mac_manager_sm_trigger_t) e->trigger)
    {
      case FAN_MAC_TRIGGER_ENTRY:
        initialise_nbr_table ();
#if (EFR32FG13P_LBR == 0x00)          
        if(fan_mac_information_data.state_ind != JOIN_STATE_5)
        {
          s->state_ind = FAN_MAC_STARTUP_STATE;
        }
#else
          s->state_ind = FAN_MAC_STARTUP_STATE;
#endif        
        if(fan_mac_information_data.fan_node_type == BORDER_ROUTER_NODE)
        {
          /*set tickkle timmer for PA pkt*/
#if WITH_SECURITY 
#if(APP_NVM_FEATURE_ENABLED == 1)
          if(fan_mac_information_data.is_start_from_nvm != 1)
#endif
              set_mac_security_on_LBR(&neighbour_addr[0],8);
#endif
          trickle_timer_config_pa_send();  
        } 
#if WITH_SECURITY 
        else
        {
           set_mac_security_on_router_node(&neighbour_addr[0],8);
        }
#endif
        /*changing state to node state 1*/
         sm_transit((sm_t *)s, (sm_state_t)&node_state_1); 
         
      break;

      case SEND_PAS_WS_SYNC_PKT:/* pas message will send by the joining node 1*/
        //send_ws_async_frame(ASYNC_START,PAN_ADVERT_SOL,e->param.vector,total_channel_lenth_uicast);
        //sm_transit((sm_t *)s, (sm_state_t)&node_state_1); 
      break; 

      case FAN_MAC_TRIGGER_EXIT:
        s->previous_state =  s->state_ind;
      break;

      default:
      break; 
    } 
    return NULL;  
} 

/*----------------------------------------------------------------------------*/
/*PAS & PA exchange is done by this state*//*state 1*/
static sm_result_t node_state_1(fan_mac_information_sm_t *s, const sm_event_t *e)
{
    switch((fan_mac_manager_sm_trigger_t) e->trigger)
    {  
      case FAN_MAC_TRIGGER_ENTRY:
#if (EFR32FG13P_LBR == 0x00)        
        if((fan_mac_information_data.state_ind == JOIN_STATE_5)
#if(APP_NVM_FEATURE_ENABLED == 1)
           &&(fan_mac_information_data.is_start_from_nvm == true)
#endif
             )
        {
          sm_transit((sm_t *)s, (sm_state_t)&node_state_5);
        }
        else
        {
          s->state_ind = JOIN_STATE_1;     
#if(APP_NVM_FEATURE_ENABLED == 1)
          nvm_store_write_fan_join_info(); //suneet :: store the join state status  
#endif
          if(fan_mac_information_data.fan_node_type == LEAF_NODE ||\
            fan_mac_information_data.fan_node_type == ROUTER_NODE)
          {
            /* pas message will send by the joining node 1*/
            /*tickel timmer is set for pas pkt*/
#if (PRINT_DEBUG_LEVEL==RUNTIME_LOG_FILE)          
            send_runtime_log ("Join State 1");
#endif
            trickle_timer_config_pas_send();
          }
        }
#else        
        s->state_ind = JOIN_STATE_1;
        if(fan_mac_information_data.fan_node_type == LEAF_NODE ||\
          fan_mac_information_data.fan_node_type == ROUTER_NODE)
        {
          /* pas message will send by the joining node 1*/
          /*tickel timmer is set for pas pkt*/
#if (PRINT_DEBUG_LEVEL==RUNTIME_LOG_FILE)          
          send_runtime_log ("Join State 1");
#endif
          trickle_timer_config_pas_send();
        }
#endif        //(EFR32FG13P_LBR == 0x00)  
        /*Need to update here*////NO EAPOL so directly jumping to state-3  //Umesh for temp
#if ENABLE_FAN_MAC_WITHOUT_SECURITY   
        if(fan_mac_information_data.fan_node_type == BORDER_ROUTER_NODE)
        {
           /* if node is border router and security is disabled then it will directly go to the state 5*/         
           sm_transit((sm_t *)s, (sm_state_t)&node_state_5);     
        } 
#elif ( FAN_EAPOL_FEATURE_ENABLED == 0) //WITHOUT_EAPOL        
        if(fan_mac_information_data.fan_node_type == BORDER_ROUTER_NODE)
        {
           /* if node is border router and security is disabled then it will directly go to the state 5*/         
           sm_transit((sm_t *)s, (sm_state_t)&node_state_5);     
        } 
#endif        
        
        
      break;

      case SEND_PAS_WS_SYNC_PKT:/*when pas doesn't get pa - retransmission*/
           send_ws_async_frame(ASYNC_START,PAN_ADVERT_SOL,e->param.vector,total_channel_lenth_uicast);
               
      break;

      case SEND_PA_WS_SYNC_PKT:
             send_ws_async_frame(ASYNC_START,PAN_ADVERT,e->param.vector,total_channel_lenth_uicast);
      break;
      case RECEIVED_PA_PKT:        
          if(!(process_pa_msg(e->param.vector)))
            return NULL;
          break;
    
        case RECEIVED_PAS_PKT:/*rceived PAS pkt */  
          if(!(process_pas_msg(e->param.vector)))
            return NULL;
          
         break;  
       
    case RECEIVED_MAX_NBR_PA_PACKET:
         {
              trickle_timer_pas_stop();
              sm_transit((sm_t *)s, (sm_state_t)&node_state_2);
         }
         break;
        
      case FAN_MAC_TRIGGER_EXIT:
        s->previous_state =  s->state_ind;
      break;

      case FAN_SYN_PKT_SEND_ERROR:/*if there is an error in sync pkt change state to Idle state*/
        sm_transit((sm_t *)s, (sm_state_t)&fan_node_idle );
      break;

      default:
      break;
    }    
    return NULL;
}

/*----------------------------------------------------------------------------*/
/*eapol message will get exchange in this state*//*state 2*/
extern void fan_eapol_request (void);
static sm_result_t node_state_2(fan_mac_information_sm_t *s, const sm_event_t *e)
{
    switch((fan_mac_manager_sm_trigger_t) e->trigger)
    {
      case FAN_MAC_TRIGGER_ENTRY:
        s->state_ind = JOIN_STATE_2;
        
#if ((EFR32FG13P_LBR == 0x00)  && (APP_NVM_FEATURE_ENABLED == 1) )
        
         nvm_store_write_fan_join_info(); //suneet :: store the join state status
#endif         
        
#if ENABLE_FAN_MAC_WITHOUT_SECURITY  //NO EAPOL so directly jumping to state-3  //Umesh for temp 
        sm_transit((sm_t *)s, (sm_state_t)&node_state_3);
#elif (FAN_EAPOL_FEATURE_ENABLED == 0)   //WITHOUT_EAPOL
        sm_transit((sm_t *)s, (sm_state_t)&node_state_3);
#else        
#if (PRINT_DEBUG_LEVEL==RUNTIME_LOG_FILE)        
        send_runtime_log ("Join State 2");
#endif

#if(FAN_EAPOL_FEATURE_ENABLED == 1)
        fan_eapol_request();
#endif
        
#endif            
      break;

      case FAN_MAC_TRIGGER_EXIT:
        s->previous_state =  s->state_ind;
      break;

      case FAN_SYN_PKT_SEND_ERROR:/*if there is an error in sync pkt change state to Idle state*/
        sm_transit((sm_t *)s, (sm_state_t)&fan_node_idle );
      break;

      default:
      break;
    }
    return NULL;
}

/*----------------------------------------------------------------------------*/
/*state 3*/
static sm_result_t node_state_3(fan_mac_information_sm_t *s, const sm_event_t *e)
{
    switch((fan_mac_manager_sm_trigger_t) e->trigger)
    {
      case FAN_MAC_TRIGGER_ENTRY:
        s->state_ind = JOIN_STATE_3;
#if ((EFR32FG13P_LBR == 0x00)     && (APP_NVM_FEATURE_ENABLED == 1))    
         nvm_store_write_fan_join_info(); //suneet :: store the join state status
#endif            
        /*after exchanging of EAPOL messages and set the security*/
#if (PRINT_DEBUG_LEVEL==RUNTIME_LOG_FILE)        
        send_runtime_log ("Join State 3");
#endif
        trickle_timer_config_pcs_send(); // start pas timer once got                                
      break;

      case FAN_MAC_TRIGGER_EXIT:
        s->previous_state =  s->state_ind;
//        trickle_timer_pcs_stop ();
//        sm_transit((sm_t *)s, (sm_state_t)&node_state_4 );
      break;
      
      case SEND_PCS_WS_SYNC_PKT:
        send_ws_async_frame(ASYNC_START,PAN_CONFIG_SOL,e->param.vector,total_channel_lenth_uicast);
//        sm_transit((sm_t *)s, (sm_state_t)&node_state_4);
      break;

      case RECEIVED_PC_PKT:/*Receives PC packet*/
            process_pc_msg(e->param.vector);
      break;
      case RECEIVED_PCS_PKT:/*Received PCS pkt*/ 
            if(process_pcs_msg(e->param.vector))//if parsing done successfully
      break;
    
      case FAN_SYN_PKT_SEND_ERROR:/*if there is an error in sync pkt change state to Idle state*/
        sm_transit((sm_t *)s, (sm_state_t)&fan_node_idle );
      break;

      default:
      break;
    }
    return NULL;
}  
/*----------------------------------------------------------------------------*/
/*state 4*/
static sm_result_t node_state_4(fan_mac_information_sm_t *s, const sm_event_t *e)
{
    switch((fan_mac_manager_sm_trigger_t) e->trigger)
    {
      case FAN_MAC_TRIGGER_ENTRY:
        s->state_ind = JOIN_STATE_4;
#if (PRINT_DEBUG_LEVEL==RUNTIME_LOG_FILE)        
        send_runtime_log ("Join State 4");
#endif
        // Raka :: This should be only the case of Router not for LBR
        if(!fan_mac_information_data.upper_layer_started )    
          /*once everything is ok then start upper layer*/
          node_start_upper_layer_ready();
        /*changing upper_layer_started flag to 1*/
        fan_mac_information_data.upper_layer_started = 1;
#if ((EFR32FG13P_LBR == 0x00)  && (APP_NVM_FEATURE_ENABLED == 1))       
         nvm_store_write_fan_join_info(); //suneet :: store the join state status
#endif            
      break;

      case FAN_MAC_TRIGGER_EXIT:
        s->previous_state =  s->state_ind;
      break;

      case FAN_SYN_PKT_SEND_ERROR:/*if there is an error in sync pkt change state to Idle state*/
        sm_transit((sm_t *)s, (sm_state_t)&fan_node_idle );
      break;
      
      default:
      break;
    }   
    return NULL;
}  
/*----------------------------------------------------------------------------*/
/*state 5*/
static sm_result_t node_state_5(fan_mac_information_sm_t *s, const sm_event_t *e)
{
    switch((fan_mac_manager_sm_trigger_t) e->trigger)     
    {
      case FAN_MAC_TRIGGER_ENTRY:
        {
#if (EFR32FG13P_LBR == 0x01)            
          s->state_ind = JOIN_STATE_5;
          
          if(fan_mac_information_data.fan_node_type == BORDER_ROUTER_NODE)
          {
            if((fan_mac_information_data.state_ind == JOIN_STATE_5)
#if(APP_NVM_FEATURE_ENABLED == 1)               
               &&(fan_mac_information_data.is_start_from_nvm == true)
#endif
             )
            {
              trickle_timer_config_pc_send();
#if(APP_NVM_FEATURE_ENABLED == 1)
              upload_parameter_from_nvm();     
#endif
              if(!fan_mac_information_data.upper_layer_started )     
                node_start_upper_layer_ready();
              fan_mac_information_data.upper_layer_started = 1; 
            }
            else
            {
#if(APP_NVM_FEATURE_ENABLED == 1)
              update_nvm_parameter();  //update nvm l3 parameter when jump to  join state 5;
#endif
              /*In state 5 we send PA and PC messages by border router and router*/        
              trickle_timer_config_pc_send();
              //Arjun :26-11-17 move to state_5
              if(!fan_mac_information_data.upper_layer_started )     
                node_start_upper_layer_ready();
              fan_mac_information_data.upper_layer_started = 1;  
#if(APP_NVM_FEATURE_ENABLED == 1)
              nvm_store_write_fan_join_info();
              nvm_store_write_fan_macself_info();
              nvm_store_write_fan_macsecurity_info();
              nvm_store_mac_white_list_info(); 
#endif
            }
          }
          else if(fan_mac_information_data.fan_node_type == ROUTER_NODE)
          {
            
#if(APP_NVM_FEATURE_ENABLED == 1)
            nvm_store_write_fan_join_info(); //suneet :: store the join state status  
            nvm_store_write_fan_macself_info();
            nvm_store_write_fan_macsecurity_info();     
#endif
            
              /*In state 5 we send PA and PC messages by border router and router*/    
#if (PRINT_DEBUG_LEVEL==RUNTIME_LOG_FILE)            
              send_runtime_log ("Join State 5");
#endif
              trickle_timer_config_pa_send();          
              trickle_timer_config_pc_send(); 
#if(APP_NVM_FEATURE_ENABLED == 1)
              update_nvm_parameter();  //update nvm l3 parameter when jump to  join state 5;
#endif
          }
#else
          if(fan_mac_information_data.state_ind != JOIN_STATE_5)
          {
            s->state_ind = JOIN_STATE_5;
            if(fan_mac_information_data.fan_node_type == BORDER_ROUTER_NODE)
            {
              fan_mac_information_data.upper_layer_started = false;
              /*In state 5 we send PA and PC messages by border router and router*/        
              trickle_timer_config_pc_send();
              //Arjun :26-11-17 move to state_5
              if(!fan_mac_information_data.upper_layer_started )     
                node_start_upper_layer_ready();
              fan_mac_information_data.upper_layer_started = 1;
            }
            else if(fan_mac_information_data.fan_node_type == ROUTER_NODE)
            {       
#if(APP_NVM_FEATURE_ENABLED == 1)
              nvm_store_write_fan_join_info(); //suneet :: store the join state status
              nvm_store_write_fan_macself_info();
              nvm_store_write_fan_macsecurity_info();   
#endif
              /*In state 5 we send PA and PC messages by border router and router*/    
#if (PRINT_DEBUG_LEVEL==RUNTIME_LOG_FILE)            
              send_runtime_log ("Join State 5");
#endif
              trickle_timer_config_pa_send();          
              trickle_timer_config_pc_send(); 

#if(APP_NVM_FEATURE_ENABLED == 1)
              update_nvm_parameter();  //update nvm l3 parameter when jump to  join state 5;
#endif
            }
          }
          else
          {
            if((fan_mac_information_data.state_ind == JOIN_STATE_5)
               &&(fan_mac_information_data.upper_layer_started == TRUE)
#if(APP_NVM_FEATURE_ENABLED == 1)                 
                 &&(fan_mac_information_data.is_start_from_nvm == true)
#endif
              )
            {     
#if(APP_NVM_FEATURE_ENABLED == 1)  
              upload_parameter_from_nvm();    
#endif
              fan_mac_information_data.upper_layer_started = 1;
              trickle_timer_config_pa_send();          
              trickle_timer_config_pc_send();
            }
          }
#endif          
        }
      break;

      case FAN_MAC_TRIGGER_EXIT:
        s->previous_state =  s->state_ind;
      break;

      case SEND_PA_WS_SYNC_PKT:
        //create_rand_delay_in_us();//since for broadcast PAS, PA is triggered at exact same time for every devices
        send_ws_async_frame(ASYNC_START,PAN_ADVERT,e->param.vector,\
                                                    total_channel_lenth_uicast);
      break;

      case SEND_PC_WS_SYNC_PKT:
        //create_rand_delay_in_us();//since for broadcast PCS, PC is triggered at exact same time for every devices
        send_ws_async_frame(ASYNC_START,PAN_CONF,e->param.vector,\
                                                    total_channel_lenth_uicast);
      break;

      case STOP_ASYNC_STOP_PKT:
        /* stop to send async packet */
      break;  

      case RECEIVED_PAS_PKT:/*Received PAS pkt*/  
        if(process_pas_msg(e->param.vector));//if parsing done successfully
        trickle_timer_send_pa_pkt_immediately();
        //trickle_timer_consistency_pa();
      break;

      case RECEIVED_PCS_PKT:/*Received PCS pkt*/ 
        if(process_pcs_msg(e->param.vector))//if parsing done successfully
          trickle_timer_send_pc_pkt_immediately();
        //trickle_timer_consistency_pc();
      break;
      case RECEIVED_PA_PKT:
    /* process of packet is done now changing to state 2*/
      if(!(process_pa_msg(e->param.vector)))
        return NULL;
      break ;
      case RECEIVED_PC_PKT:/*Receives PC packet*/
        process_pc_msg(e->param.vector); 
        break ;
      /* this state is dummy to just send dummy eapol pkt */

      case FAN_SYN_PKT_SEND_ERROR:/*if there is an error in sync pkt change state to Idle state*/
        sm_transit((sm_t *)s, (sm_state_t)&fan_node_idle );
      break;
      
      default:
        break; 
    }
    return NULL;
}


/*----------------------------------------------------------------------------*/


/*
** ============================================================================
** Public Function Definitions
** ============================================================================
*/

void fan_mac_timer_init_value(void)
{
    pcs_timer.operation = ASYNC_START;
    pcs_timer.frame_type = PAN_CONFIG_SOL;
    //pcs_timer.channel_list =NULL; 

    pa_timer.operation = ASYNC_START;
    pa_timer.frame_type = PAN_ADVERT;
    //pa_timer.channel_list =NULL; 

    pas_timer.operation = ASYNC_START;
    pas_timer.frame_type = PAN_ADVERT_SOL;
    //pas_timer.channel_list =NULL;

    pc_timer.operation =  ASYNC_START;
    pc_timer.frame_type = PAN_CONFIG;
    //pc_timer.channel_list =NULL; 
}
/*----------------------------------------------------------------------------*/
void fan_mac_init(uint8_t fan_node_type)
{
    fan_mac_information_data.fan_node_type = fan_node_type;/*seting node type*/
    fan_mac_timer_init_value();/*setting timer values*/
    fan_mac_information_sm_t *app = &fan_mac_information_data;   
    sm_event_t e = { (sm_trigger_t) FAN_MAC_TRIGGER_ENTRY, { 0 } };
    app->super.state = (sm_state_t)&fan_node_startup;
    SM_DISPATCH((sm_t *)app, &e);/*cahnging state to fan_node startup state*/
    fan_mac_started =1;
    fan_startsm_initialise();/*starting and initialising Fan state machine */
}
 
/*----------------------------------------------------------------------------*/


void send_error_ws_async_pkt(void)
{
//   sm_event_t e = { (sm_trigger_t) FAN_SYN_PKT_SEND_ERROR, { 0 } };   
//   e.param.scalar = 0xFF;   
//   SM_DISPATCH((sm_t *)&fan_mac_information_data, &e);
} 
/*----------------------------------------------------------------------------*/
void send_trickle_timer_pkt(fan_pkt_tmr_t trickle_timer)
{
    if(trickle_timer.operation == ASYNC_STOP)
    {
      sm_event_t e1 = { (sm_trigger_t) STOP_ASYNC_STOP_PKT, { 0 } };   
      e1.param.scalar = 0xFF;   
      SM_DISPATCH((sm_t *)&fan_mac_information_data, &e1);
      return;
    }  
    else
    {
      switch (trickle_timer.frame_type)/*taking dicision on frame type which pkt is to send*/
      {
        case PAN_ADVERT_SOL: 
          {
          sm_event_t e2 = { (sm_trigger_t) SEND_PAS_WS_SYNC_PKT, { 0 } };   
          //e2.param.vector = trickle_timer.channel_list;   
          SM_DISPATCH((sm_t *)&fan_mac_information_data, &e2);
          }
        break;
        
        case PAN_ADVERT:
          {
          sm_event_t e3 = { (sm_trigger_t) SEND_PA_WS_SYNC_PKT, { 0 } };   
          //e3.param.vector = trickle_timer.channel_list;   
          SM_DISPATCH((sm_t *)&fan_mac_information_data, &e3);
          }
        break;

        case PAN_CONFIG_SOL:
          {
          sm_event_t e4 = { (sm_trigger_t) SEND_PCS_WS_SYNC_PKT, { 0 } };   
          //e4.param.vector = trickle_timer.channel_list;  
          SM_DISPATCH((sm_t *)&fan_mac_information_data, &e4);
          }
        break;
        
        case PAN_CONFIG:
          {
          sm_event_t e5 = { (sm_trigger_t) SEND_PC_WS_SYNC_PKT, { 0 } };   
          //e5.param.vector = trickle_timer.channel_list;   
          SM_DISPATCH((sm_t *)&fan_mac_information_data, &e5);
          }
        break;
        
        case STOP_ASYNC_STOP_PKT:
          /* stop to send async packet */
        break;  

        default:
        break;   
      }        
    }
}
/*----------------------------------------------------------------------------*/
void send_ws_async_frame(uint8_t op_type, uint8_t frame_type, uint8_t* channel_list,uint8_t length)
{
    WS_ASYNC_FRAME_request(
                          op_type,//Operation
                          frame_type,//Frame type                           
                          channel_list,
                          length 
                          );
}  
/*----------------------------------------------------------------------------*/
 
/*----------------------------------------------------------------------------*/
/*process received pkt*/
void process_ws_async_frame(mac_rx_t* mac_rec_msg)
{
    if(fan_mac_started == 1)
    {
      if((mac_rec_msg->recived_frame_type)==PAN_ADVERT_FRAME)
      {
        if(fan_mac_information_data.state_ind == JOIN_STATE_1
           || fan_mac_information_data.state_ind == JOIN_STATE_5)
        {
          sm_event_t e = { (sm_trigger_t) RECEIVED_PA_PKT, { 0 } };   
          e.param.vector = mac_rec_msg;  
          SM_DISPATCH((sm_t *)&fan_mac_information_data, &e);
        }
        else
          app_bm_free((uint8_t*)mac_rec_msg->pd_rxp);  /*if mac not started free the memory*/
      } 
      else if((mac_rec_msg->recived_frame_type)==PAN_ADVERT_SOLICIT)
      {
        if(fan_mac_information_data.state_ind == JOIN_STATE_1
           || fan_mac_information_data.state_ind == JOIN_STATE_5)
        {
          sm_event_t e = { (sm_trigger_t) RECEIVED_PAS_PKT, { 0 } };   
          e.param.vector = mac_rec_msg;  
          SM_DISPATCH((sm_t *)&fan_mac_information_data, &e);
        }
        else
          app_bm_free((uint8_t*)mac_rec_msg->pd_rxp);
      } 
      else if((mac_rec_msg->recived_frame_type)==PAN_CONFIG)
      {
        if(fan_mac_information_data.state_ind == JOIN_STATE_3
           || fan_mac_information_data.state_ind == JOIN_STATE_5)
        {
          sm_event_t e = { (sm_trigger_t) RECEIVED_PC_PKT, { 0 } };   
          e.param.vector = mac_rec_msg;  
          SM_DISPATCH((sm_t *)&fan_mac_information_data, &e);
        }
        else
          app_bm_free((uint8_t*)mac_rec_msg->pd_rxp);
      }   
      else if((mac_rec_msg->recived_frame_type)==PAN_CONFIG_SOLICIT)
      {
        if(fan_mac_information_data.state_ind == JOIN_STATE_5
           || fan_mac_information_data.state_ind == JOIN_STATE_3 )
        {
          sm_event_t e = { (sm_trigger_t) RECEIVED_PCS_PKT, { 0 } };   
          e.param.vector = mac_rec_msg;  
          SM_DISPATCH((sm_t *)&fan_mac_information_data, &e);  
        }
        else
          app_bm_free((uint8_t*)mac_rec_msg->pd_rxp);
      } 
      else
        return ;
    }
    else
    {
      app_bm_free((uint8_t*)mac_rec_msg->pd_rxp);  /*if mac not started free the memory*/
    }
    return;   
}  
/*----------------------------------------------------------------------------*/

uint8_t is_mac_addr_zero (uint8_t *addr)
{
  uint8_t zero[] = {0, 0, 0, 0, 0, 0, 0, 0};
  if (memcmp (addr, zero, IEEE_ADDRESS_LENGTH))
    return 0;
  else
    return 1;
}

uint8_t find_best_pan()
{
  /*finding best parrent on lowest path cost*/
  uint8_t rev_addr[8]= {0,0,0,0,0,0,0,0};
  
  find_lowest_pancost_from_nbr_table_for_mac_address(rev_addr);
  
  if (is_mac_addr_zero(rev_addr))
    return 0;   //Debdeep :: NO PAN Found  08-March-18

#if(FAN_EAPOL_FEATURE_ENABLED == 1)
#if APP_LBR_ROUTER
  memcpy(eapol_parent_child_info.sle_eapol_parent,rev_addr,8);
#endif
#endif
  
  sm_event_t e = { (sm_trigger_t) RECEIVED_MAX_NBR_PA_PACKET, { 0 } };
  SM_DISPATCH((sm_t *)&fan_mac_information_data, &e);/*changing state*/
  return 1;   
}

#if(FAN_EAPOL_FEATURE_ENABLED == 1)
void eapol_parent_unresponsive_callback (void)
{
  uint8_t rev_addr[8]= {0,0,0,0,0,0,0,0};
  
  #if APP_LBR_ROUTER
  set_eapol_parent_unresponsive (eapol_parent_child_info.sle_eapol_parent);
#endif
  
  find_lowest_pancost_from_nbr_table_for_mac_address(rev_addr);
  if (is_mac_addr_zero(rev_addr))
  {
    if (get_join_state () == 5)
      kill_process_and_clean_rpl_nbr ();
    else
      reset_to_join_state_1 ();
    return;
  }
  
  #if APP_LBR_ROUTER
  memcpy (eapol_parent_child_info.sle_eapol_parent, rev_addr, 8);
#endif
  
  fan_eapol_request ();    
}


void get_eapol_parent(uint8_t *eapol_parent)
{
  #if APP_LBR_ROUTER
  memcpy(eapol_parent,eapol_parent_child_info.sle_eapol_parent,8);
#endif
}

void get_eapol_parent_address(uint8_t *eapol_parent)
{
  #if APP_LBR_ROUTER
  mem_rev_cpy(eapol_parent,eapol_parent_child_info.sle_eapol_parent,8);
#endif
}


#endif // #if(FAN_EAPOL_FEATURE_ENABLED == 1)

/*----------------------------------------------------------------------------*/
/*processing PA pkt here*/
static uint8_t process_pa_msg(mac_rx_t* rx_pa_msg)
{ 
  uint8_t packet_type = PAN_ADVERT_FRAME;
  if(fan_mac_information_data.state_ind == JOIN_STATE_1)
  {
      if(rx_pa_msg->src.address_mode == ADDR_MODE_EXTENDED)
      {
        send_mlme_async_frame_indicaiton(rx_pa_msg->src.address.ieee_address,packet_type,1);
      }
  }
  else if(fan_mac_information_data.state_ind == JOIN_STATE_5)
  {
      chack_node_goes_in_consis_inconsi (rx_pa_msg); 
      send_mlme_async_frame_indicaiton(rx_pa_msg->src.address.ieee_address,packet_type,1);
  } 
    app_bm_free((uint8_t*)rx_pa_msg->pd_rxp);/*free mem*/
    
    
    /*checking ie elements matching*/
    /* calculate path cost and decide the prefered parent */
    return 1;
}  
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/*processing PAS pkt here*/
static uint8_t process_pas_msg(mac_rx_t* rx_pas_msg)
{
    uint8_t packet_type = PAN_ADVERT_SOLICIT;
    chack_node_goes_in_consis_inconsi (rx_pas_msg);  

    if(rx_pas_msg->src.address_mode == ADDR_MODE_EXTENDED)
    {
      send_mlme_async_frame_indicaiton(rx_pas_msg->src.address.ieee_address,packet_type,1);
    }     
    app_bm_free((uint8_t*)rx_pas_msg->pd_rxp);/*free mem*/
    //if(status==PARSE_SUCCESS)
      //create_rand_delay_in_us();
      //send_trickle_timer_pkt(pa_timer);//sending immediate PA packet once received PAS packet
    return 1;
}  
/*----------------------------------------------------------------------------*/
/*processing PCS pkt here*/
static uint8_t process_pcs_msg(mac_rx_t* rx_pcs_msg)
{
    uint8_t packet_type = PAN_CONFIG_SOLICIT;
    if(fan_mac_information_data.state_ind == JOIN_STATE_3)
    {
      chack_node_goes_in_consis_inconsi(rx_pcs_msg);
    }
    else 
    {
          chack_node_goes_in_consis_inconsi(rx_pcs_msg);  
          if(rx_pcs_msg->src.address_mode == ADDR_MODE_EXTENDED)
          {
            send_mlme_async_frame_indicaiton(rx_pcs_msg->src.address.ieee_address,packet_type,1);
          }
    }   
     app_bm_free((uint8_t*)rx_pcs_msg->pd_rxp);
      
    return 1;
}  
/*----------------------------------------------------------------------------*/
/*processing PC pkt here*/
static uint8_t process_pc_msg(mac_rx_t* rx_pc_msg)
{
    uint8_t packet_type = PAN_CONFIG;
    if(fan_mac_information_data.state_ind == JOIN_STATE_3)
    {
        /* Processing and checking ie elements matching*/
        if(rx_pc_msg->src.address_mode == ADDR_MODE_EXTENDED)
        {
          pcs_timer.ready_to_change_state = 1;
          if(recv_pcs_attement_count() > 0)
          {
            change_join_state (0);
          }
          send_mlme_async_frame_indicaiton(rx_pc_msg->src.address.ieee_address,packet_type,1);
        }
    }
    else if(fan_mac_information_data.state_ind == JOIN_STATE_5)
    {
        chack_node_goes_in_consis_inconsi( rx_pc_msg);  
    }    
    app_bm_free((uint8_t*)rx_pc_msg->pd_rxp);/*free mem*/
    return 1;
}  
/*----------------------------------------------------------------------------*/
////after receiving of RPL-DAO-ACK
//void change_to_join_state_05(void)   
//{
//    fan_mac_information_sm_t *app = &fan_mac_information_data;   
//    sm_event_t e = { (sm_trigger_t) FAN_MAC_TRIGGER_ENTRY, { 0 } };
//    app->super.state = (sm_state_t)&node_state_5;
//    SM_DISPATCH((sm_t *)app, &e);/*changing state to state 5*/
//}
/*----------------------------------------------------------------------------*/
uint8_t get_dev_mac_join5_status(void)
{
  return ((fan_mac_information_data.state_ind == JOIN_STATE_5) ? 1 : 0);
}
/*----------------------------------------------------------------------------*/
/*creating random delay in us sec for pa and pc*/
void create_rand_delay_in_us()
{
  unsigned short rand_val = 0;
  uint8_t randSeed =  hw_tmr_rand(NULL);
  l3_random_init((randSeed << 8));
  l3_random_init((randSeed << 8));
  rand_val = l3_random_rand();
  tmr_delay(rand_val);
}

int complete_count = 0;
int reset_count = 0;

void change_join_state(uint8_t attemt_index)
{
  fan_mac_information_sm_t *app = &fan_mac_information_data;   

  if(fan_mac_information_data.fan_node_type == BORDER_ROUTER_NODE)
  {  
      sm_transit((sm_t *)app, (sm_state_t)&node_state_5);
  }
  else if (pcs_timer.ready_to_change_state)
  {
      trickle_timer_pcs_stop ();
      pcs_timer.ready_to_change_state = 0;
      sm_transit((sm_t *)app, (sm_state_t)&node_state_4);
      reset_count++;
//      reset_to_join_state_1 ();
  }
  else if(attemt_index == 0x05)//MAX_ATTEMT_PCS_PKT is 5
  {
    sm_transit((sm_t *)app, (sm_state_t)&node_state_1);
  }
  else if(fan_mac_information_data.fan_node_type == ROUTER_NODE &&
          fan_mac_information_data.state_ind == JOIN_STATE_4)
  {
         ////after receiving of RPL-DAO-ACK
          sm_transit((sm_t *)app, (sm_state_t)&node_state_5);
  }
  else
  {
    if (fan_mac_information_data.state_ind == JOIN_STATE_2)
    {
      uint16_t best_pan =  find_lowest_pancostfrom_nbr_table();
      /*update nbr table based on best parrent*/
      update_pan_id_from_border_router(best_pan);
      complete_count++;
#if(APP_NVM_FEATURE_ENABLED == 1)
      nvm_store_write_fan_join_info(); //suneet :: store the join state status 
#endif
      sm_transit((sm_t *)app, (sm_state_t)&node_state_3);
    }
  }     
}

int get_join_state (void)       //Debdeep
{
  if (fan_mac_information_data.state_ind == JOIN_STATE_1)
    return 1;
  if (fan_mac_information_data.state_ind == JOIN_STATE_2)
    return 2;
  if (fan_mac_information_data.state_ind == JOIN_STATE_3)
    return 3;
  if (fan_mac_information_data.state_ind == JOIN_STATE_4)
    return 4;
  if (fan_mac_information_data.state_ind == JOIN_STATE_5)
    return 5;
  
  return 1;
}

#if(APP_NVM_FEATURE_ENABLED == 1)
void change_join_state_for_nvm()
{
  memset(&fan_mac_information_data,0,sizeof(fan_mac_information_data));
  nvm_store_write_fan_join_info();
  
}
#endif

#endif
