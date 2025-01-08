/** \file network-manager.h
 *******************************************************************************
 ** \brief Provides information about the MAC Layer
 **
 ** \cond STD_FILE_HEADER
 **
 ** COPYRIGHT(c) 2023-24 Procubed Innovations Pvt Ltd. 
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


#ifndef FAN_NWK_MANAGER_H
#define FAN_NWK_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "sw_timer.h"
  /*Suneet :: Set COUNTRY_CODE_VAL 
0-USA/CANADA
1-USA/MAXICO
2-BRAZIL
3-ANZ
4-EU#3
5-PHILIPPINES
6-MALAYSIA
7-HONGKONG , SINGAPORE 1, THAILAND, VIETNAM
8-KOREA
9-CHINA
10-JAPAN
11-EU#1
12-EU#2
13-INDIA
14-SINGAPORE #2
*/
#define COUNTRY_CODE_VAL 0x0D  // INDIA 

typedef struct opearating_zone
{
  
   uint8_t current_phy_band_ID;
    uint8_t current_ext_phy_band_ID;
    uint32_t SunPage;
}opearating_zone_t;


extern opearating_zone_t opearating_zone_list[];


/*! Node is of type PAN Coordinator*/
#define PAN_COORD_NODE_TYPE					0x00
/*! Node is of type Coordinator*/
#define COORD_NODE_TYPE						0x01
/*! Node is of type Simple Device*/
#define DEVICE_NODE_TYPE					0x02

/*! Total number of Channels(Each bit indicates a channel)*/
#define DEFAULT_CHANNEL_BIT_MAP				(uint64_t)(0x000000000000FFFF)

/*! Total number of Channels(Each bit indicates a channel)*/
#define DEFAULT_CHANNEL_BIT_MAP_ARRAY			0xFF,0xFF,0xFF,0x1F,0x00,0x00,0x00,0x00

/*! Defines the Short Address of the Coordinator*/
#define SELF_SHORT_ADDRESS                	(0xA1B1)

/*! Defines the Extended Address of the Device*/

#define CMD_NOT_SUPPORTED 0x6E
#define   CMD_INVALID     0x20

// LBR Address
//#define SELF_LONG_ADDRESS	                    0x11,0x22,0x33,0x44,0xAA,0xBB,0xCC,0xDD
//Router Address
//#define SELF_LONG_ADDRESS	                    0x11,0x22,0x33,0x44,0x00,0x00,0x00,0x01

//#define SELF_LONG_ADDRESS	                    0x00,0x00,0x00,0x00,0x00,0xAA,0x11,0x22

#define SELF_LONG_ADDRESS	                    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01


#define LBR_ADDRESS                                 0x11,0x22,0x33,0x44,0xAA,0xBB,0xCC,0xDD
#define ROUTER_ADDRESS                              0x11,0x22,0x33,0x44,0x00,0x00,0x00,0x01

#define MAXIMUM_EXCLUDE_CHANNEL_SUPPORT   5 



/**
 *******************************************************************************
 ** \enum el_nwk_manager_sm_trigger_t
 **  Specific triggers for Coordinator state machine
 *******************************************************************************
 **/
typedef enum
{
    TRIGGER_ENTRY = SM_TRIGGER_ENTRY, /**< Event to enter into a new state*/
    TRIGGER_EXIT  = SM_TRIGGER_EXIT,  /**< Event to exit from the current state*/
    TRIGGER_START_NODE,               /**< Event to indicate that start request is received*/
    TRIGGER_RESET_CONF,  	   /**< Event to indicate that Reset Conf is received*/
    TRIGGER_SET_CONF,    	   /**< Event to indicate that Set Conf is received*/
    TRIGGER_NWK_START_CONF,    /**< Event to indicate that Start Conf is received*/
    TRIGGER_SET_FAN_MAC_IE_CONF,
    TRIGGER_FAN_MAC_INIT,
    TRIGGER_CHANGE_JOIN_STATE,
    TSM_TRIGGER_PACKETS,
    TSM_TRIGGER_TX_COMPLETE,
    TSM_TRIGGER_STOP_TX,
    TSM_TRIGGER_START_RECEIVE,
    TSM_TRIGGER_RX_COMPLETE,
    TSM_TRIGGER_STOP_RX,
    TSM_TRIGGER_STREAM,
}fan_nwk_manager_sm_trigger_t;



/**
 *******************************************************************************
 ** \enum el_nwk_mgnr_state_t
 ** The state indicator is used to refer to a state outside of the state machine
 *******************************************************************************
 **/
typedef enum {
	IDLE_STATE,            /**< Indicates that the machine is in IDLE state*/
        STARTUP_STATE,         /* start up state */
	RESETTING_STATE,       /**< Indicates that the machine is in RESET state*/
	INITIALIZING_STATE,    /**< Indicates that the machine is in INITIALISING state*/
	ED_SCANNING_STATE,     /**< Indicates that the machine is in ED SCAN state*/
	ACTIVE_SCANNING_STATE, /**< Indicates that the machine is in ACTIVE SCAN state*/
        EB_ACTIVE_SCANNING_STATE, /**< Indicates that the machine is in ACTIVE SCAN state*/
	STARTING_NETWORK_STATE,/**< Indicates that the machine is in START state*/
	ASSOCIATING_STATE,     /**< Indicates that the machine is in ASSOCIATE state*/
	NODE_MAC_READY_STATE,      /**< Indicates that the machine is in MAC READY state*/
        NODE_EL_READY_STATE,      /**< Indicates that the machine is in EL READY state after pana authentication and echonet lite layer start*/
        NODE_EL_DISABLE_STATE,  /*el layer is disabled temporarily*/
        /*NODE_SET_SECURITY_PARAM_FROM_PANA_STATE,*/      /**< Indicates that the security credentials are set from PANA*/
        NODE_SET_SECURITY_PARAM_STATE,
        DEV_READY_WITHOUT_SCANNING_STATE,
        NODE_MAC_READY_STATE_AFTER_SET_MAC_SECURITY,
        NODE_FACTORY_MODE_STATE,
        NODE_SENDIND_PACKET,
        NODE_RECEIVEING_PACKET,
        NODE_CONTINIOUS_UNMOD_STREAM_STATE,
        NODE_CONTINIOUS_MOD_STREAM_STATE,
} fan_nwk_mgnr_state_t;


/**
 *******************************************************************************
 ** \struct el_nwk_manager_sm_struct
 ** Structure holding information about State Machine
 *******************************************************************************
 **/

/**
 *******************************************************************************
 ** \struct neighbor_table_tag
 **  Structure that can store the utt ie element a
 **  device
 *******************************************************************************
 **/
typedef struct utt_ie_element_struct
{
  uint8_t frame_type_id;  //bit 0-3 only used 4-7 is reserved
  uint32_t unicast_fraction_seq_interval; // this is 24 bit value 
}utt_ie_t;
/*----------------------------------------------------------------------------*/
/**
 *******************************************************************************
 ** \struct neighbor_table_tag
 **  Structure that can store the bt ie element a
 **  device
 *******************************************************************************
 **/
typedef struct bt_ie_element_struct
{
  uint8_t length;
  uint16_t broadcast_time_interval;
  uint32_t broadcast_fraction_interval_offset;

}bt_ie_t;
/*----------------------------------------------------------------------------*/
/**
 *******************************************************************************
 ** \struct neighbor_table_tag
 **  Structure that can store the fc ie element a
 **  device
 *******************************************************************************
 **/
typedef struct fc_ie_element_struct
{
  uint8_t transmit_flow_control;
  uint8_t receive_flow_control;
}fc_ie_t;
/*----------------------------------------------------------------------------*/
/**
 *******************************************************************************
 ** \struct neighbor_table_tag
 **  Structure that can store the rsl ie element a
 **  device
 *******************************************************************************
 **/
typedef struct rsl_ie_element_struct
{
  uint8_t rsl;
}rsl_ie_t;
/*----------------------------------------------------------------------------*/
/**
 *******************************************************************************
 ** \struct neighbor_table_tag
 **  Structure that can store the mhds ie element a
 **  device
 *******************************************************************************
 **/
typedef struct mhds_ie_element_struct
{
  uint8_t prefix_len;
  uint8_t prefix[12];
}mhds_ie_t;
/*----------------------------------------------------------------------------*/
/**
 *******************************************************************************
 ** \struct neighbor_table_tag
 **  Structure that can store the vh ie element a
 **  device
 *******************************************************************************
 **/
typedef struct vh_ie_element_struct
{
  uint16_t wisun_vendor_id;
  uint8_t* vendor_content;
}vh_ie_t;

/*----------------------------------------------------------------------------*/
/**
 *******************************************************************************
 ** \struct neighbor_table_tag
 **  Structure that can store the vp ie element a
 **  device
 *******************************************************************************
 **/
typedef struct vp_ie_element_struct
{
  uint8_t* vendor_content;
  uint16_t wisun_vendor_id;
  
}vp_ie_t;
/*----------------------------------------------------------------------------*/



typedef struct mp_ie_element_struct
{
  

  uint8_t transaction_control;
  uint8_t fragment_number;
  uint16_t length_type_id;
    /*
  0-10 bits --length
  11-14 bit --group id
  15- bit --- type
  */  
  
  uint16_t total_upper_layer_frame_size;
  uint16_t multiplex_id;
  uint8_t* upper_layer_fragment;
}mp_ie_t;
/*----------------------------------------------------------------------------*/
/**
 *******************************************************************************
 ** \struct neighbor_table_tag
 **  Structure that can store the heaader ie element of a
 **  device
 *******************************************************************************
 **/
typedef struct mac_header_information_element_struct
{
  utt_ie_t utt_e;   // Unicast Timing and Frame Type Information Element (UTT_IE)
  bt_ie_t bt_ie;    //Broadcast Timing Information Element (BT-IE)
  fc_ie_t fc_ie;    //Flow Control IE (FC-IE)
  rsl_ie_t rsl_ie;  //Received Signal Level Information Element (RSL-IE)
  mhds_ie_t mhds_ie; //MHDS Information Element (MHDS-IE) __ only use when l2 lyaer routing
  vh_ie_t vh_ie;    //Vendor Header Information Element (VH-IE)
}mac_header_information_element_t;
/*----------------------------------------------------------------------------*/
/**
 *******************************************************************************
 ** \struct neighbor_table_tag
 **  Structure that can store the and payload ie of a
 **  device
 *******************************************************************************
 **/
typedef struct mac_payload_information_element_struct
{
  vp_ie_t vp_ie;	        //Vendor Payload Information Element (VP-IE)
  mp_ie_t mp_ie ;                //MP Information Element (MP-IE)      
}mac_payload_information_element_t;
/*----------------------------------------------------------------------------*/

typedef struct packet_mode_struct
{
	uint32_t pkt_cnt;
	uint16_t pkt_len;
	uint8_t dw_enabled;
	uint8_t fcslength;//1:long,0 short. used while invoking MIL API for data tx
	uint8_t sending_cntrl; // 0x00 - 1 sec or less, 0x01 - 100 ms or less 
	uint32_t sending_ctrl_cycle_dur_us; //3600 * 10^6 us by default it will have
	uint32_t carrier_sense_dur;
	uint8_t is_cca_on;
	mac_address_t dest_addr;
        uint64_t dest_ieee_address; 
        uint16_t first_pkt_size; 
        uint16_t last_pkt_size;
	uint8_t test_bit_map;
        uint8_t random_packet_len;
}packet_mode_t;

/*----------------------------------------------------------------------------*/
typedef unsigned char stream_mode_t;
typedef struct trx_params_struct
{
	uint32_t channel_freq;//Hz
	uint16_t freq_dev;//KHz
	uint32_t data_rate;//bps
	uint8_t ifbw;
	uint8_t dev_role;//TX or RX
	int8_t tx_pwr_dbm;//dBm
	uint8_t pa_level;//1 to 63
	uint8_t tx_mode;//continuous/stream or packet mode
	uint8_t mod_type;//2GFSK.2FSK,4GFSK,2GFSK, unmod carrier
	uint8_t mod_source;// packet handler, internall PN9 gen
        uint8_t display_rx_data;
        uint8_t promiscous_mode;
        packet_mode_t pkt_mode_param;
        stream_mode_t strm_mode_param;
}trx_params_t;
/*----------------------------------------------------------------------------*/
typedef struct  factory_mod_tag
{
  trx_params_t trx_pkt_param;
  uint8_t app_msdu_handle;    /**< holds the random number for the request*/
  sw_tmr_t send_dur_tmr; 
  sw_tmr_t ipdelay;
  sw_tmr_t delay_next_pkt;
  void (*fn_alarm)(void*);
  sw_tmr_t pause_tmr;  
  sw_tmr_t last_tx_timer;
  uint8_t *gmsduu;
  uint32_t tx_packet_counter;
  uint32_t rx_packet_num;
  int8_t rssi_threshold;
  uint8_t xtal_adjust;
  uint32_t serial_baudrate;
}factory_mod_tag;
/*----------------------------------------------------------------------------*/
typedef struct el_nwk_manager_sm_struct
{
    sm_t super;					/**< current state, represented as a general state machine */
    fan_nwk_mgnr_state_t state_ind;	        /**< current state indicator */
    fan_nwk_mgnr_state_t previous_state;
    node_basic_config_t node_basic_cfg;
    uint8_t result;				/**< result of a previous operation */
    uint8_t one_byte_value;		/**< used to set the PIBs*/
    uint16_t self_short_addr;         /**< holds it short address*/                                           
    uint64_t def_chan_bit_map;
    bool upper_layer_started;
    mac_header_information_element_t fan_mac_header_ie;
    mac_payload_information_element_t fan_mac_payload_ie;
    factory_mod_tag factory_mod_st;
#if(APP_NVM_FEATURE_ENABLED == 1)
    bool nvm_write_to_start;
#endif
}fan_nwk_manager_sm_t;

/*----------------------------------------------------------------------------*/
enum {
  FAN_NWK_MGR_START,
  FAN_NWK_MGR_START_UPPER_LAYER,
  SEND_MAC_ADDRESS_TO_LINUX,
  SEND_FAN_MAC_START_CMD,
  MAC_2_NHLE_CONFIRMATION
};


extern fan_nwk_manager_sm_t fan_nwk_manager_app;


typedef void ( *app_mcps_purge_conf_handler_t )( uint8_t msdu_handle, uint8_t status );

typedef void ( *app_mlme_assoc_conf_handler_t )( uint16_t short_address,
	uint8_t status,
	uint8_t* pLowLatencyNwkInfo,
	uint16 channelOffset,
	uint8_t HoppingSequenceLength,
	uint8_t HoppingSequence,
	security_params_t *sec_params);


typedef void ( *app_mlme_bcn_notify_ind_handler_t ) 
( 
	uint8_t bsn,
	pandesc_t* pPandesc,
	uint8_t PendAddrSpec,
	uint8_t* pPendaddrlist,
	uint16_t sdulen,
	uint8_t* pSdu,
	uint8_t ebsn,
	uint8_t beaconType,
	coex_spec_ie_t* coex_spec
);

typedef void ( *app_mlme_assoc_ind_handler_t )( uint8_t* pChild_64_bit_addr,
	uint8_t CapabilityInformation,
	uint8_t* pLowLatencyNwkInfo,
	uint16 channelOffset,
	uint8_t  HoppingSequenceID,
	security_params_t *sec_params );


typedef void ( *app_mlme_comm_status_ind_handler_t )( mac_address_t* pSrcaddr,
	mac_address_t * pDstaddr,
	uint8_t status,
	security_params_t *sec_param );
typedef void ( *app_mlme_poll_conf_handler_t )( uint8_t status );
typedef void ( *app_mlme_scan_conf_handler_t )( uint8_t status,
	uint8_t ScanType,
	uint8_t ChannelPage,
	uint8_t* p_unscannedChannels,
	uint8_t ResultListSize,
	void *ResultList);


typedef void ( *app_mlme_reset_conf_handler_t )( uint8_t status );
typedef void ( *app_mlme_start_conf_handler_t )( uint8_t status );

typedef void ( *app_mlme_set_conf_handler_t )( uint8_t status,
	 uint8_t PIBAttribute,
	 uint8_t PIBAttributeIndex  );


typedef void ( *app_mlme_get_conf_handler_t )( uint8_t status,
	 uint8_t PIBAttribute,
	 uint8_t PIBAttributeIndex,
	 uint16_t PIBAttributeLength,
	 void *PIBAttributeValue  );

typedef void ( *app_mlme_dissoc_ind_handler_t )( uint8_t* DeviceAddress,
	uint8_t DisassociateReason,
	security_params_t *sec_params  );
       

typedef void ( *app_mlme_dissoc_conf_handler_t )( uint8_t status,
	mac_address_t* Deviceaddr  );

typedef void ( *app_mlme_orphan_ind_handler_t )( uint8_t* pOrphan64bitAddress, // 64bit address of orphan device
	security_params_t *sec_params );


typedef void ( *app_mlme_sync_loss_ind_handler_t )( uint8_t LossReason,			//loss reason
	uint16_t PANId,				//PANID
	uint8_t LogicalChannel, 		// LogicalChannel
	uint8_t ChannelPage,			// ChannelPage
	security_params_t *sec_params	 );

typedef void ( *app_mlme_bcn_conf_handler_t )( uchar status );

typedef void ( *app_mlme_bcn_req_ind_handler_t )( uchar bcn_type,
	mac_address_t* src_addr,
	ushort dest_pan_id,
	ushort ie_list_fld_size,
	uchar* p_ie_list );

typedef void ( *mac_data_ind_handler_t )( mac_address_t*  pSrcaddr,
	mac_address_t*  pDstaddr,
	uint16_t msduLength,
	uint8_t* pMsdu,
	uint8_t mpduLinkQuality,
	uint8_t DSN,
        uint8_t pld_ies_present,
	uint32_t Timestamp,
	security_params_t* pSec);

typedef void ( *mac_data_eapol_ind_handler_t )
(
		mac_address_t*  pSrcaddr,
		mac_address_t*  pDstaddr,
		uint8_t transfer_cont,
		uint16_t multiplex_id,
		uint8_t kmp_id,
		uint16_t msduLength,
		uint8_t* pMsdu);

typedef void (* mac_ack_ind_handler_t)
(
  mac_address_t*  pSrcaddr,
  mac_address_t*  pDstaddr,
  uint8_t DSN,
  uint8_t rsl_val,
  uint8_t security_status
);

typedef void (* mac_no_ack_ind_handler_t) (void);

typedef void ( *mac_data_conf_handler_t )( uint8_t msduHandle, 
	uint8_t status, 
	uint8_t NumBackoffs,
	uint32_t Timestamp );

void enet_nwk_mgr_poll( void );

#if(APP_NWK_MANAGER_PROCESS_FEATURE_ENABLED == 1)
void fan_nwk_mgr_post_event( uint8_t ev, uint8_t* data);
#endif

void  get_self_extended_address (uint8_t *macAddr);
void  get_self_extended_address_reverse (uint8_t *macAddr);
void node_start_upper_layer_ready(void);
void send_mac_address_to_linux(void);
void add_security_key_descriptor_on_MAC();
/*-------------------------------------------------------------------*/

#ifdef __cplusplus
}
#endif
#endif /* FAN_NWK_MANAGER_H */