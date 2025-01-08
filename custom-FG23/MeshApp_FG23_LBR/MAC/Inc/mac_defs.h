/** \file mac_defs.h
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

#ifndef MAC_DEFS_H
#define MAC_DEFS_H

#ifdef __cplusplus
extern "C" {
#endif

/*
** ============================================================================
** Public Macro definitions
** ============================================================================
*/
#ifndef bit
#define bit(x) (0x01<<(x))
#endif

/* definitions for MAC frame header fields */
/* MAC frame types */
/*! Defines the BEACON frame type value*/
#define MAC_FRAME_TYPE_BEACON				0
/*! Defines the DATA frame type value*/
#define MAC_FRAME_TYPE_DATA					1
/*! Defines the ACK frame type value*/
#define MAC_FRAME_TYPE_ACK					2
/*! Defines the COMMAND frame type value*/
#define MAC_FRAME_TYPE_MAC_COMMAND			3
/*! Defines the MASK value for frame identifying the frame type*/
#define MAC_FRAME_TYPE_MASK                 0x07    /* frame type is 3 LSBs */
/*! Defines that the frame is Regular beacon sub type for the frame type beacon*/
#define MAC_FRAME_BCN_SUB_TYPE_RB			0x01
/*! Defines that the frame is enhanced beacon sub type for the frame type beacon*/
#define MAC_FRAME_BCN_SUB_TYPE_EB			0x02
/*! Defines that the frame is MPM beacon sub type for the frame type beacon*/
#define MAC_FRAME_BCN_SUB_TYPE_MPM_EB		0x03
/*! Defines that the frame sub type for the frame type beacon,need not worry*/
#define MAC_FRAME_BCN_SUB_TYPE_DONT_CARE    0xFF	

/* Control bits in the first byte of Frame control field */
/*! Indicates the MAC_SECURITY_ENABLED bit in the first byte of Frame control field*/
#define MAC_SECURITY_ENABLED				bit(3)
/*! Indicates the MAC_FRAME_PENDING bit in the first byte of Frame control field*/
#define MAC_FRAME_PENDING					bit(4)
/*! Indicates the MAC_ACK_REQUIRED bit in the first byte of Frame control field*/
#define MAC_ACK_REQUIRED					bit(5)
/*! Indicates the MAC_INTRA_PAN bit in the first byte of Frame control field*/
#define MAC_INTRA_PAN						bit(6)

/* Control bits in the second byte (MSB) of Frame control field */
/*! Indicates the MAC_SEQ_NUM_SUPPRESSION bit in the second byte of Frame control field*/
#define MAC_SEQ_NUM_SUPPRESSION				bit(0)
/*! Indicates the MAC_IE_LIST_PRESENT bit in the second byte of Frame control field*/
#define MAC_IE_LIST_PRESENT					bit(1)

/* Definitions for MAC Frame address fields */
/*! Defines the value for no addressing mode as 0*/
#define MAC_NO_ADDRESS						0
/*! Defines the value for short addressing mode as 2*/
#define MAC_SHORT_ADDRESS					2
/*! Defines the value for extended addressing mode as 3*/
#define MAC_IEEE_ADDRESS					3
/*! Defines the shifting value for source address*/
#define MAC_SRC_ADDRESS_SHIFT				6
/*! Defines the shifting value for destination address*/
#define MAC_DST_ADDRESS_SHIFT				2
/*! Defines the masking value for destination address*/
#define MAC_DST_ADDRESS_MASK			(3<<MAC_DST_ADDRESS_SHIFT)
/*! Defines the shifting value when destination address is not present*/
#define MAC_NO_DST_ADDRESS				(0<<MAC_DST_ADDRESS_SHIFT)
/*! Defines the shifting value for short destination address*/
#define MAC_SHORT_DST_ADDRESS			(2<<MAC_DST_ADDRESS_SHIFT)
/*! Defines the shifting value for extended destination address*/
#define MAC_IEEE_DST_ADDRESS			(3<<MAC_DST_ADDRESS_SHIFT)

/*! Defines the masking value for source address*/
#define MAC_SRC_ADDRESS_MASK			(3<<MAC_SRC_ADDRESS_SHIFT)
/*! Defines the shifting value when source address is not present*/
#define MAC_NO_SRC_ADDRESS				(0<<MAC_SRC_ADDRESS_SHIFT)
/*! Defines the shifting value for short source address*/
#define MAC_SHORT_SRC_ADDRESS			(2<<MAC_SRC_ADDRESS_SHIFT)
/*! Defines the shifting value for extended source address*/
#define MAC_IEEE_SRC_ADDRESS			(3<<MAC_SRC_ADDRESS_SHIFT)
/*! Defines the value for Invalid Short Address*/
#define INVALID_SHORT_ADDRESS			0xffff

/*************************************************************
 * field identifier tokens passed to mac_calc_field_offset() *
 *************************************************************/
/*! Defines value for MAC_FRAME_FIELD_CONTROL_FIELD0*/
#define MAC_FRAME_FIELD_CONTROL_FIELD0		0
/*! Defines value for MAC_FRAME_FIELD_CONTROL_FIELD1*/
#define MAC_FRAME_FIELD_CONTROL_FIELD1		1
/*! Defines value for MAC_FRAME_FIELD_DSN*/
#define MAC_FRAME_FIELD_DSN					2
/*! Defines value for MAC_FRAME_FIELD_BSN*/
#define MAC_FRAME_FIELD_BSN					2
/*! Defines value for MAC_FRAME_FIELD_DST_PAN*/
#define MAC_FRAME_FIELD_DST_PAN				3
/*! Defines value for MAC_FRAME_FIELD_DST_ADDRESS*/
#define MAC_FRAME_FIELD_DST_ADDRESS			4
/*! Defines value for MAC_FRAME_FIELD_SRC_PAN*/
#define MAC_FRAME_FIELD_SRC_PAN				5
/*! Defines value for MAC_FRAME_FIELD_SRC_ADDRESS*/
#define MAC_FRAME_FIELD_SRC_ADDRESS			6
/*! Defines value for MAC_FRAME_FIELD_MAC_COMMAND*/
#define MAC_FRAME_FIELD_MAC_COMMAND			7
/*! Defines value for MAC_FRAME_FIELD_DATA*/
#define MAC_FRAME_FIELD_DATA				8

/*definitions for beacon*/
/*! Defines the masking value for pending short address*/
#define SHORT_ADDRESSES_PENDING_MASK    0x07
/*! Defines the masking value for pending extended address*/
#define IEEE_ADDRESSES_PENDING_MASK     0x70

/*=================================================
 * defines for ACK frame
 *=================================================
 */
 #if OLD
/*! Macro defining value for MAC_ACK_FRAME_CONTROL*/
#define MAC_ACK_FRAME_CONTROL					0
/*! Macro defining value for MAC_ACK_FRAME_CONTROL0*/
#define MAC_ACK_FRAME_CONTROL0					0
/*! Macro defining value for MAC_ACK_FRAME_CONTROL1*/
#define MAC_ACK_FRAME_CONTROL1					1
/*! Macro defining value for MAC_ACK_DSN*/
#define MAC_ACK_DSN								2
/*! Macro defining value for MAC_ACK_CRC1*/
#define MAC_ACK_CRC1							3
/*! Macro defining value for MAC_ACK_CRC2*/
#define MAC_ACK_CRC2							4
/*! Macro defining value for MAC_ACK_FRAME_SIZE*/
#define MAC_ACK_FRAME_SIZE                      5

/* defines for Beacon frame */
/*! Macro defining value for BEACON_FRAME_CONTROL*/
#define BEACON_FRAME_CONTROL					0
/*! Macro defining value for BEACON_FRAME_SRC_PAN_ID*/
#define BEACON_FRAME_SRC_PAN_ID					2
/*! Macro defining value for BEACON_FRAME_SRC_ADDR*/
#define BEACON_FRAME_SRC_ADDR					4
/*! Macro defining value for BEACON_FRAME_BSN*/
#define BEACON_FRAME_BSN						5
/*! Macro defining value for BEACON_FRAME_SUPERFRAME_SPEC*/
#define BEACON_FRAME_SUPERFRAME_SPEC			6

/* bit defns for superframe spec field */
/*! Macro defining bit definition for BEACON_FRAME_PAN_COORDINATOR*/
#define BEACON_FRAME_PAN_COORDINATOR			0x01
/*! Macro defining bit definition for BEACON_FRAME_ASSOCIATION_PERMIT*/
#define BEACON_FRAME_ASSOCIATION_PERMIT		(0x01<<1)

/* address spec field */
/*! Macro defining value for BEACON_FRAME_ADDRESS_SPECIFICATION*/
#define BEACON_FRAME_ADDRESS_SPECIFICATION			8
/*! Macro defining value for BEACON_ADDRESS_LIST*/
#define BEACON_ADDRESS_LIST							9
/*! Macro defining value for BEACON_HEADER_LENGTH*/
#define BEACON_HEADER_LENGTH						9
/*! Macro defining length of beacon frame control feild*/
#define BEACON_FRAME_CONTROL_LENGTH					2
/*! Macro defining length of beacon sequence number*/
#define BEACON_SEQ_NUM_LENGTH						1
/*! Macro defining maximum length of beacon address feild*/
#define BEACON_ADDRESS_FIELD_MAX_LENGTH				10
/*! Macro defining length of beacon superframe specification feild*/
#define BEACON_SUPERFRAME_SPEC_LENGTH				2
/*! Macro defining length of pending address speciication feild*/
#define BEACON_PENDING_ADDR_SPEC_LENGTH				1
/*! Macro defining maximum length of beacon address list*/
#define BEACON_ADDR_LIST_MAX_LENGTH				(7 * 8) /* 7 IEEE addresses */

/*! Defines the maximum length for beacon frame control*/
#define BEACON_MAX_LENGTH BEACON_FRAME_CONTROL_LENGTH + \
								  BEACON_SEQ_NUM_LENGTH			+ \
								  BEACON_SUPERFRAME_SPEC_LENGTH	+ \
								  BEACON_PENDING_ADDR_SPEC_LENGTH + \
								  BEACON_ADDR_LIST_MAX_LENGTH

#endif

/*********************************
 * define for DATA request frame *
 *********************************/

/*! Defines maximum length for data request frame*/
#define DATA_REQUEST_MAX_LENGTH		18
/*! Defines maximum length for orphan notification frame( 2 for CRC )*/
#define ORPHAN_NOTIFICATION_LENGTH	(18+2)
/*! Defines maximum length for beacon request frame( 2 for CRC )*/
#define BEACON_REQUEST_LENGTH		 (8+2)  
/*! Defines maximum length for GTS request frame( 2 for CRC )*/
#define GTS_REQUEST_LENGTH			(9+2)   /* include space for CRC */
/*! Defines maximum length for panid conflict frame( 2 for CRC )*/
#define PANID_CONFLICT_NOTIFICATION_LENGTH	(23+1+2)
/*! Defines the bit indicating RX_ON_WHEN_IDLE in the capability information parameter during assoc*/
#define RX_ON_WHEN_IDLE bit(3)

/* MAC Commands */
/*! Defines command id for ASSOCIATION_REQUEST*/
#define ASSOCIATION_REQUEST						0x01
/*! Defines command id for ASSOCIATION_RESPONSE*/
#define ASSOCIATION_RESPONSE					0x02
/*! Defines command id for DISASSOCIATION_NOTIFICATION*/
#define DISASSOCIATION_NOTIFICATION				0x03
/*! Defines command id for DATA_REQUEST*/
#define DATA_REQUEST							0x04
/*! Defines command id for PAN_ID_CONFLICT_NOTIFICATION*/
#define PAN_ID_CONFLICT_NOTIFICATION			0x05
/*! Defines command id for ORPHAN_NOTIFICATION*/
#define ORPHAN_NOTIFICATION						0x06
/*! Defines command id for BEACON_REQUEST*/
#define BEACON_REQUEST							0x07
/*! Defines command id for COORDINATOR_REALIGNMENT*/
#define COORDINATOR_REALIGNMENT					0x08
/*! Defines command id for GTS_REQUEST*/
#define GTS_REQUEST								0x09
/*! Defines command id for GTS_CONFIRM*/
#define GTS_CONFIRM								0x0a
/*! Defines command id for LE RIT Data request*/
#define LE_RIT_DATA_REQUEST						0x20

/* values stored in mac_data.dtx->tx_options  */
/*! Defines the values to be stored in mac_data.dtx->tx_options if data tx is intiated*/
#define TX_OPTION_MAC_INITIATED_DATA			0x80
/*! Defines the values to be stored in mac_data.dtx->tx_options if data tx is pending*/
#define TX_OPTION_FRAME_PENDING_IN				0x40

/*! Defines length of assocaition request command frame*/
#define MAC_ASSOCIATE_REQUEST_LENGTH (23+1+1+2) /* 17 or 23 octets of header, command field
                                                   and capability information + 2 bytes CRC */
/*! Defines length of association response command frame*/
#define MAC_ASSOCIATE_RESPONSE_LENGTH (23+1+2+1+2)  /* 23 octets header, command, short address and status, CRC */
/*! Defines length of disassociation notification command frame*/
#define MAC_DISASSOCIATE_NOTIFICATION_LENGTH (23+1+1+2) /* 23 octets header,  command, reason, CRC */
/*! Defines length of coordinator realignment command frame with extended address*/
#define MAC_IEEE_COORDINATOR_REALIGNMENT_LENGTH (23+1+2+1+2+2+2)    /* 23 octets header, command, PAN, Coordinator short address, channel, Short Address, CRC */
/*! Defines length of coordinator realignment command frame with short address*/
#define MAC_SHORT_COORDINATOR_REALIGNMENT_LENGTH (17+1+2+1+2+2+2)   /* 17 octets header, command, PAN, Coordinator short address, channel, Short Address, CRC */

/* some internal error codes */
/* success is 0 and defined elsewhere */
/*! Macro defining value for unspecified fail*/
#define MAC_UNSPECIFIED_FAIL			1
/*! Macro defining value when an ack is received, but not expected*/
#define MAC_ACK_NOT_EXPECTED			2   
/*! Macro defining value when an extended address match failed*/
#define IEEE_ADDRESS_MATCH_FAILED		3
/*! Macro defining value when an short address match failed*/
#define SHORT_ADDRESS_MATCH_FAILED		4
/*! Macro defining value when an PANID match failed*/
#define PANID_MATCH_FAILED				5

/* various timeout values */
/*! Macro defining value for ack timeout*/
#define ACK_TIMEOUT						6   /* number of 20 symbol periods to wait for an ACK */

/*! Defines default value for beacon header length with extended address*/
#define DEFAULT_BEACON_HEADER_LENGTH (3+2+8)    /* cf0, cf1, BSN, pan id, ieee address */
/*! Defines default value for beacon header length with short address*/
#define SHORT_BEACON_HEADER_LENGTH (3+2+2)  /* cf0, cf1, BSN, pan id, short address */

/* a maximum  of 7 GTS allocations are permitted */
/*! Defines maximum value for GTS slot allocations*/
#define MAX_GTS_DESCRIPTORS		7   /* maximum of 7 GTS slot allocations */
/*! Defines maximum value for pending addresses in GTS frame*/
#define MAX_PENDING_ADDRESSES	7   /* max of 7 pending addresses (may be short or ieee) */
/*! Defines GTS permit value*/
#define GTS_PERMIT 0x80

/*! Macro defining maximum number of indirect transactions*/
#define MAX_INDIRECT_MAC_TRANSACTIONS 7 /* maximum pending transactions */

/*! Macro defining default value when short address is still not assigned by PAN Coord*/
#define SHORT_ADDRESS_UNKNOWN	0xffff
/*! Macro defining default value when the child device wishes to use only extended address*/
#define USE_IEEE_ADDRESS		0xfffe
/*! Macro defining default value to be used when broadcasting a frame*/
#define BROADCAST_SHORT_ADDRESS	0xffff
/*! Macro defining default value of PANID*/
#define PAN_ID_UNKNOWN			0xffff
/*! Macro defining default value of Broadcast PANID*/
#define BROADCAST_PAN_ID		0xffff
/*! Macro defining default short address of the node as 0xFFFF*/
#define DEFAULT_SHORT_ADDRESS	SHORT_ADDRESS_UNKNOWN
/*! Macro defining default PANID of the node as 0xFFFF*/
#define DEFAULT_PAN_ID			PAN_ID_UNKNOWN
/*! Macro defining value when security is not in ACL*/
#define MAC_SECURITY_NOT_IN_ACL         8
/*! Macro defining value for SYNC_SINGLE*/
#define SYNC_SINGLE	 0
/*! Macro defining value for SYNC_TRACKING*/
#define SYNC_TRACKING 1
/*! Macro defining value for SECURITY_ENABLED_TRANSMISSION*/
#define SECURITY_ENABLED_TRANSMISSION           0x08
/*! Macro defining frame version field value for the frames introduced in the 802.15.4eg spec*/
#define FRAME_VERSION_2006                      0x10
/*! Macro defining frame version field value for the frames introduced in the 802.15.4eg spec*/
#define FRAME_VERSION_2011                      0x20
/*! Macro defining masking value for frame version feild*/
#define FRAME_VERSION_MASK                      0x30

/*! Macro defining value successfull status*/
#define PASSED                                  0
/*! Macro defining value for conditionally passed status*/
#define CONDITIONALLY_PASSED                    1
/*! Macro defining value for failure status*/
#define FAILED                                  2
/*! Macro defining status of battery as good*/
#define BATTERY_STATUS_OK          0
/*! Macro defining status of battery as low*/
#define BATTERY_STATUS_LOW         1
/*! Macro defining status of battery as unavailable*/
#define BATTERY_STATUS_UNAVAILABLE 2

                                                     

                                                    
/*
** ============================================================================
** Public Structures, Unions & enums Type Definitions
** ============================================================================
*/
/**
 *******************************************************************************
 ** \struct mac_address_t
 ** Structure to store the address details of the node
 *******************************************************************************
 **/
#ifndef MAC_ADDR_STRUCT
#define MAC_ADDR_STRUCT
typedef struct mac_address_struct
{
    uchar address_mode;         /* addressing mode */
    ushort pan_id;              /* pan id (if present) */
    union
    {
        ushort short_address;    /* short address */
        uchar *ieee_address;     /* IEEE address */
    } address;
} mac_address_t;
#endif

/* structure used in the beacon */
/**
 *******************************************************************************
 ** \struct gts_descriptor_t
 ** Structure to store the GTS details of the node
 *******************************************************************************
 **/
typedef struct gts_descriptor_struct
{
    uchar device_short_address[2];  /* short address of GTS device */
    uchar length_and_start_slot;
} gts_descriptor_t;

/*Information about the GTS,is held in the mac_data structure*/
/**
 *******************************************************************************
 ** \struct gts_data_t
 ** Structure to store the GTS data details
 *******************************************************************************
 **/
typedef struct gts_data_struct
{
    gts_descriptor_t gts_desc;  /* the GTS descriptor, as found in the beacon */
    ushort expiration_timer;    /* idle timer for expiration */
    uchar gts_state;            /* current state */

#define GTS_UNUSED		0
#define GTS_ALLOCATED	        1
#define GTS_DEALLOCATED	        2

    bitfield desc_timer:3;      /* time remaining for descriptor to be in beacon */
    bitfield direction:1;
} gts_data_t;


/* MAC message structure - all messages received from the PHY by the MAC are of this type*/
/**
 *******************************************************************************
 ** \struct mac_rx_t
 ** Structure to store details of the packet received from PHY
 *******************************************************************************
 **/

#include "fan_mac_ie.h"

typedef struct mac_rx_struct
{
    struct mac_rx_struct *link;			/**< linked list */
    phy_rx_t *pd_rxp;					/**< receive message structure */
    uchar frame_type;							/**< message type */
    uchar frame_ver;					/**< frame version info */
    gboolean      security_enable;
    gboolean    pan_id_compression;
    gboolean    seqno_suppression;
    gboolean    ie_present;
    gboolean    ack_request;
    gboolean    frame_pending;
    uchar sn;							/**< beacon or data sequence number */
    uchar *payload;						/**< start of MAC payload */
    uint16_t payload_length;			/**< payload length */
    uchar frame_pending_out;			/**< if ACK to this frame was sent with the frame pending bit set */
    uchar frame_pending_in;				/**< if frame pending bit was set in this frame */
    mac_address_t src;					/**< source address */
    mac_address_t dst;					/**< destination address */
    uint8_t pld_ies_present;
    uchar headerIEListLen;				/**< number of formatted header IEs in the received frame */
    uchar *headerIEList;				/**< start of formatted header IEs in the received frame */
    ushort headerIEFieldLen;			/**< total length of the header IE feild */
    uchar payloadIEListLen;				/**< number of formatted payload IEs in the received frame */
    uchar *payloadIEList;				/**< start of formatted payload IEs in the received frame */
    ushort payloadIEFieldLen;			/**< total length of the payload IE feild */
    uchar recived_frame_type;                     //For fan_mac_payload and hedr ie
    uchar auxiliary_secoffset_index;              //For track auxiliary_sec_index
    uint16_t header_ie_offset;                    //for track header-ie offset
/* FIXME PEB */
    uchar gts;							/**< indicates if msg was received in a GTS slot */
/* FIXME: code does not compile correctly if these elements are missing */
#ifdef MAC_CFG_SECURITY_ENABLED
    security_struct_t *security_data;   /**< Pointer to the security data structure */
    mac_status_t security_status;		/**< security status */
    ulong frame_counter;				/**< frame counter */ 
#endif
    security_params_t sec_param;		/**< security material details */
    uint32_t receive_frame_sfd;
    struct ieee802154_ies *wisun_fan_ies;
    uint8_t      command_id;
} mac_rx_t;

/* data structure for mac direct transmission */
/**
 *******************************************************************************
 ** \struct mac_tx_t
 ** Structure to store details of the MAC direct transmission
 *******************************************************************************
 **/
typedef struct mac_tx_struct
{
    struct mac_tx_struct *link;			/**< linked list */
    phy_tx_t *data;						/**< pointer to the data to transmit */
    gts_data_t *gts_data;				/**< pointer to the associated gts data structure */
    mac_address_t src;					/**< packet source address */
    mac_address_t dst;					/**< packet destination address */
    uint8_t dest_long_addr[8];			/**< extended address of the destination*/
    ushort persistence_timer;			/**< time to live timer - 0 means expired */
#ifdef ENHANCED_ACK_SUPPORT
    uint32_t enack_wait_timer;			/**< max absoulte time till , mac waits for an enhanced ack for this message */
#endif
    uchar type;							/**< type of message */
    uchar sub_type;						/**< used to differentiate EB v/s B v/s MPM EB and EBR vs BR */
    uchar tx_options;					/**< misc flags for this message */
    uchar msdu_handle;					/**< handle if direct MCPS */
    uchar cmd;							/**< command identifier for a mac command frame */
    uint16_t length;					/**< total length of message */
    uchar sn;							/**< sequence number */
    mac_status_t status;				/**< status */
    uchar missed_ack_count;				/**< number of acks missed for this message */
    uchar cap_retries;					/**< number of times transmission has failed */
    uchar num_csmaca_backoffs;
    ulong tx_timestamp;					/**< time packet is transmitted (in symbols) */
    security_struct_t *security_data;   /**< pointer to the security data structure */
    security_params_t sec_param;		/**< security material details*/
#ifdef WISUN_FAN_MAC
    uint8_t* p_ufsi;
//    uint8_t fan_pkt_type;     //Debdeep
    uint8_t* p_bfsi;
    uint8_t* broadcast_slot_no;
//#ifdef TEST_CHOP    
//    uint8_t* p_channel_holder;
//#endif
#endif
} mac_tx_t;


typedef struct l3_pkt_queue
{
  struct l3_pkt_queue *link;	
  uint16_t length;
  uint8_t *data_ptr;
  mac_address_t src;					/**< packet source address */
  mac_address_t dst;	
  uint8_t fan_packet_type;
  uint8_t msduHandle;
}l3_pkt_queue_t;

/*Parameters of a MAC superframe*/
/**
 *******************************************************************************
 ** \struct mac_sf_t
 ** Structure to store details of the MAC Superframe
 *******************************************************************************
 **/
typedef struct
{
    uchar beacon_order;
    uchar superframe_order;
    uchar final_cap_slot;
    uchar battery_life_ext;
    p3time_t time_reference;
} mac_sf_t;

/*Parameters from a MLME-SCAN.request primitive to SCAN-SM*/
/**
 *******************************************************************************
 ** \struct scan_param_t
 ** Structure to store details from a MLME-SCAN.request primitive to SCAN-SM
 *******************************************************************************
 **/
typedef struct
{
  uchar type;
  uchar channels[16];					/**< list of channels to scan */
  uchar duration;						/**< scan duration */
  uchar page;							/**< channel page to scan */
  uchar fc_options;					/**< frame control option*/
  uchar hdr_ies_cnt;					/**< number of header IEs*/
  uchar hdr_ie_list[ 20 ];			/**< header IE ids requested while performing EBActive or MPMEB_Passive scan*/
  uchar pld_ies_cnt;					/**< number of payload IEs */
  uchar pld_ie_list[ 20 ];			/**< payload IE ids requested while performing EBActive or MPMEB_Passive scan */
  uchar mpm_scan_duration_bpan;		/**< scan duration while perfroming MPMEB_Passive scan in a beacon enabled network */
  ushort mpm_scan_duration_nbpan;		/**< scan duration while perfroming MPMEB_Passive scan in a nonbeacon enabled network */
} scan_param_t;


typedef struct
{
  uchar type;
  uint16 multiplex_id;
  uint8_t transfer_type;
  uint8_t KMP_ID ;
  uint16 channels_list[200];					/**< Maximum channel support */
  uchar duration;						/**< scan duration */
  uchar fc_options;					/**< frame control option*/
  uchar hdr_ies_cnt;					/**< number of header IEs*/
  uchar hdr_ie_list[1];			/**< header IE ids requested while performing EBActive or MPMEB_Passive scan*/
  uchar pld_ies_cnt;					/**< number of payload IEs */
  uchar pld_ie_list[1];			/**< payload IE ids requested while performing EBActive or MPMEB_Passive scan */
  queue_t *l3_layer_unicast_queue;
  queue_t *l3_layer_broadcast_queue;
} fan_mac_param_t;

/**
 *******************************************************************************
 ** \struct beacon_t
 ** Structure to store details of beacon
 *******************************************************************************
 **/
#if OLD
typedef struct beacon_list_struct
{
  //  beacon_header_t beacon_header;  /* beacon header fields - control bytes 0, 1
                                       BSN and source address fields */
 //   superframe_t superframe;    /* superframe field - 2 bytes */
    beacon_payload_t beacon_payload;    /* beacon payload ends the linked list (2 copies 1 to write and 1 to send) */
} beacon_t;
#endif

/**
 *******************************************************************************
 ** \struct 
 ** Structure to store details FAN MAC parameters
 *******************************************************************************
 **/


/**
 *******************************************************************************
 ** \struct mac_data_t
 ** Structure to store MAC data in one Data structure
 *******************************************************************************
 **/
typedef struct mac_data_struct
{
    /* mac bit field flags */
    bitfield coordinator:1;						/**< set if a coordinator*/
    bitfield gts_rx_request_pending:1;			/**< set when we request a GTS slot, cleared when we send the confirm */
    bitfield gts_tx_request_pending:1;			/**< set when we request a GTS slot, cleared when we send the confirm */
    bitfield gts_update_in_progress:1;			/**< set when we change the GTS and cleared at the next beacon */
    bitfield power_save_mode:1;					/**< set when we are in power saving mode */
    bitfield in_gts_receive_period:1;			/**< set if we are in a GTS receive period */
    bitfield rx_enable_active:1;				/**< set if we are currently in the active part of rx enable */
    bitfield ieee_address_pending:1;			/**< there is a message pending for our IEEE address */
    bitfield short_address_pending:1;			/**< there is a message pending for our short address */
    bitfield check_gts_timers:1;                /**< set each beacon */
    uchar lifs_timer;							/**< number of backoffs before we are allowed to transmit again */
    uchar rx_enable_state;						/**< rx enable state */
	
#define RX_ENABLE_IDLE		0					/**< Macro defining value for RX_ENABLE_IDLE*/	
#define RX_ENABLE_DEFERRED	1					/**< Macro defining value for receiver to wait until next frame */  
#define RX_ENABLE_DELAYING	2					/**< Macro defining value for decrementing the delay timer */
#define RX_ENABLE_ACTIVE	3					/**< Macro defining value when rx is enabled, decrementing enabled timer */

    uchar panid_conflict_state;					/**< place holder indicating the state of PAN ID Conflict and resolution state */
#define PANID_CONFLICT_NOT_DETECTED	0		/**< Value indicating that no PAN ID conflict has been detected*/
#define PANID_CONFLICT_DETECTED			1		/**< Value indicating that PAN ID conflict is detected */
#define PANID_CONFLICT_SCAN_IN_PROGRESS	2		/**< Value indicatingthat coordinator is scanning for a new PAN */
#define PANID_CONFLICT_REPORTED			3		/**< Value indicating that the PAN ID conflict is detected by device and reported to PAN Coordinator*/

    /***************************
	 * Slot and GTS parameters *
     ***************************/
    uchar current_slot;							/**< current slot */
    uchar final_cap_slot;						/**< the final cap slot as transmitted in the beacon */
    uchar gts_start_slot;						/**< slot where GTS's start */
    uchar gts_tx_characteristics;				/**< characteristic requested by network layer while tx */
    uchar gts_tx_length;						/**< gts tx length */
    uchar gts_tx_start_slot;					/**< gts slave's start slot for tx */
    uchar gts_tx_timer;							/**< gts delay timer for slave to wait for allocation at tx */
    uchar gts_rx_characteristics;				/**< gts characteristic requested by network layer while rx */
    uchar gts_rx_length;						/**< gts rx length */
    uchar gts_rx_start_slot;					/**< gst slave's start slot of rx */
    uchar gts_rx_timer;							/**< gst delay timer for slave to wait for allocation at rx */
    mac_tx_t *gts_msg;							/**< pointer to current GTS tx message */
    /* coordinator must maintain the GTS information in a different format from that sent in the beacon */
    gts_data_t gts_params[MAX_GTS_DESCRIPTORS];	/**< gts parameters*/
    gts_data_t *current_gts;					/**< pointer to the currently active GTS in a coordinator */
    uchar gts_count;							/**< count of active GTS's NOT descriptor count in beacon ! */
    uchar gts_desc_count;						/**< count of GTS descriptor in beacon */

    /***************
     * misc timers *
     ***************/
    ulong rx_enable_delay_timer;				/**< delay before receiver is enabled (in backoff slots) */
    ulong rx_enable_timer;						/**< period to leave rx enabled (in backoff slots) */
    //uchar ack_timer;							/** timer for waiting for acks */
    uchar ble_expiry_timer;						/**< counts down 20 symbol periods after beacon if BattLifeExtension is TRUE */
    //ushort backoff_interrupt_interval;		/**  measured in symbols */
    //ulong current_symbol_count;				/** current count of symbols */
    //ulong beacon_rcv_symbol_count;			/** symbol count at start of received beacon */
    //ulong next_beacon_symbol_count;			/** symbol count when next beacon from parent is expected */
    /*TBD Delete beacon_tx */
    //mac_tx_t beacon_tx;						/** beacon transmit message structure */

/* Activity flag (AF_) definitions */
#define AF_RCV_MSG_PENDING				bit(0)  	
#define AF_TX_MSG_SENT_PENDING               bit(1)	
#define AF_CAP_MSG_PENDING                   bit(2)	
/* set if the coordinator has messages pended which we haven't dealt with yet */
#define AF_MESSAGES_PENDING                  bit(3)	 
#define AF_IND_MSG_PENDING                   bit(4)	
#define AF_SCAN_IN_PROGRESS                  bit(5)	
#define AF_SECURITY_IN_PROGRESS              bit(6)		
#define AF_ASSOCIATION_TIMER_EXPIRED	     bit(7)	
#define AF_GTS_ACTIVITY_PENDING              bit(8) 
#define AF_BEACON_RECEIVED                   bit(9) 
#define AF_RX_ENABLE_ACTIVE                  bit(10) 
#define AF_BEACON_UPDATE_FAILED              bit(11) 
#define AF_MCPS_MSG_PENDING                  bit(12) 
#define AF_MLME_MSG_PENDING                  bit(13)
#define AF_UART_MSG_TX_PENDING               bit(14)     
#define AF_INIT_VALUE					0xffff  

    /*TBS Remove active_beacon */
    uchar active_beacon;						/**< index of beacon data to transmit */
#ifdef MAC_CFG_SECURITY_ENABLED
    uchar beacon_data[2][127];     /*TBD Size! Can a beacon be this big? */
#endif
    uchar VersionInfo[16];						/**< frame version information*/
    ushort TxByteCount;							/**< private PIB value used for duty cycle calculation */

#ifdef MAC_CFG_COLLECT_STATS
    struct mac_stats
    {
        unsigned lowest_sp;						/**< Lowest stack pointer seen */
//        uchar lowest_spi_rx_queue_cnt;		/* Lowest number of free buffers on rx queue */
//        uchar lowest_spi_tx_queue_cnt;		/* Lowest number of free buffers on tx queue */
        uchar lowest_spi_small_tx_queue_cnt;    /**< Lowest number of free buffers on tx queue */
        uchar failed_memory_allocations;		/**< Number of times a call to mem_allocate failed */
        uchar current_phy_free_rx_queue_cnt;    /**< current number of free phy buffers on rx queue*/
        uchar current_phy_rx_queue_cnt;			/**< current number of phy buffers on rx queue*/
    } mac_stats;
#endif
    mac_security_data_t *security;				/**< security material details*/
#ifdef WISUN_FAN_MAC
    mac_nbr_data_t* p_nbr_data;
#endif    
} mac_data_t;

/*
** ============================================================================
** Public Variable Declarations
** ============================================================================
*/

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

/* the main MAC data structure */
extern mac_data_t mac_data;

/*
**=========================================================================
**  Public Function Prototypes
**=========================================================================
*/
/*prototypes for functions to send messages back to the network layer via SPI*/
/**
 *******************************************************************************
 ** \brief Adds an indirect transmission to the queue
 ** \param *src_address - pointer to source address   
 ** \param *dst_address - pointer to destination address
 ** \param *dtx - pointer to the dtx msg
 ** \param msdu_handle - handle passed by nwk
 ** \retval - status
 ******************************************************************************/
mac_status_t mac_add_indirect_transmission ( 
                                            mac_address_t *src_address, 
                                            mac_address_t *dst_address,   
                                            mac_tx_t *dtx                         
                                            );
#ifdef ENHANCED_ACK_SUPPORT
mac_status_t mac_wait_for_enack(  mac_tx_t *txp );
#endif

/**
 *******************************************************************************
 ** \brief Creates and add a data packet for transmission.Uses the GTS info in 
 **  the beacon to determine if this is allowed
 ** \param *dst_address - pointer to destination address
 ** \param *dtx - pointer to the dtx msg
 ** \retval - status
 ******************************************************************************/
mac_status_t mac_add_gts_transmission (
                                      mac_address_t *dst_address,  
                                      mac_tx_t *dtx   
                                      );

/**
 *******************************************************************************
 ** \brief  Queues a beacon packet for transmitting
 ** \param *txd - packet to queue for direct transmission
 ** \param sub_type - type of beacon 
 ** \retval - status
 ******************************************************************************/
mac_status_t mac_queue_beacon_transmission( mac_tx_t *txd , uchar sub_type );

/**
 *******************************************************************************
 ** \brief Queues a packet for transmitting in the CAP
 ** \param *txd - packet to queue for direct transmission
 ** \retval - status
 ******************************************************************************/
mac_status_t mac_queue_direct_transmission( mac_tx_t *txd );

/**
 *******************************************************************************
 ** \brief Queues a broadcast packet for transmitting in the CAP
 ** \param *txd - packet to queue for broadcast transmission
 ** \retval - status
 ******************************************************************************/
mac_status_t mac_queue_bcast_transmission( mac_tx_t *txd );

/**
 *******************************************************************************
 ** \brief Function to purge a packet from direct transmission
 ** \param type - type of message to purge
 ** \param msdu_handle - handle of message to purge
 ** \retval - status
 ******************************************************************************/
mac_status_t mac_purge_direct_transmission(uchar type, uchar msdu_handle);

/**
 *******************************************************************************
 ** \brief Function to purge a packet from indirect transmission queue
 ** \param type - type of message to purge
 ** \param msdu_handle - handle of message to purge
 ** \retval - status
 ******************************************************************************/
mac_status_t mac_purge_indirect_transmission(uchar type, uchar msdu_handle);

/**
 *******************************************************************************
 ** \brief Function to purge the specified GTS message
 ** \param msdu_handle - handle of message to purge
 ** \retval - status
 ******************************************************************************/
mac_status_t mac_purge_gts_transmission(uchar msdu_handle);

/**
 *******************************************************************************
 ** \brief Processes messaged queued in the ISR called from the main loop. 
 ** Messages are MAC commands or data frames as beacons and acks
 ** have been processed in the ISR. Any ACK required has already been sent.
 ** \param - None
 ** \retval - 0 if all messages are processed
 ** \retval - 1 if all messages are not processed
 ******************************************************************************/
uchar mac_process_received_messages(void);

/* mac_if.c */
/**
 *******************************************************************************
 ** \brief Initialise the MAC (only init the PIB if requested )
 ** \param init_pib - if non zero value initialises the PIB
 ** \retval - None
 ******************************************************************************/
void mac_initialise(uchar init_pib);

/**
 *******************************************************************************
 ** \brief Function still not used
 ** \param - None
 ** \retval - None
 ******************************************************************************/
void mac_set_receiver_state(void);

/**
 *******************************************************************************
 ** \brief Function still not used
 ** \param - None
 ** \retval - None
 ******************************************************************************/
void mac_check_transmit_gts(void);

/**
 *******************************************************************************
 ** \brief Function to check all the MAC and SPI related queues for activity
 ** \param - None
 ** \retval - None
 ******************************************************************************/
void test_and_clear_queue_flags( void );

/**
 *******************************************************************************
 ** \brief Function to check the state of the different state machine
 ** \param - None
 ** \retval - 1 if all the state machines are in idle state 
 ** \ retval - 0 if any of the state machine in not in idle state
 ******************************************************************************/
uchar test_task_states( void );

/* PAN Descriptor */
/**
 *******************************************************************************
 ** \brief Function still not used
 ** \param - None 
 ** \param - None
 ** \param - None
 ** \retval - None
 ******************************************************************************/
uchar mac_pandesc_compare( uchar *, uchar *, uchar * );

/**
 *******************************************************************************
 ** \brief Function still not used
 ** \param - None
 ** \param - None
 ** \retval - None
 ******************************************************************************/
uchar mac_pandesc_create( uchar *, mac_rx_t * );

#ifdef MAC_CFG_SECURITY_ENABLED
/**
 *******************************************************************************
 ** \brief Function to get confirmation from the security key table for the 
 **       requested attribute index
 ** \param attribute_index - index to the key table
 ** \retval - 1 on successful extracting the data at the requested index
 ** \retval - 0 on failure
 ******************************************************************************/
uchar send_key_table_get_confirm(uchar attribute_index);

/**
 *******************************************************************************
 ** \brief Function to get confirmation from the security device table for the 
 **       requested attribute index
 ** \param attribute_index - index to the device table
 ** \retval - 1 on successful extracting the data at the requested index
 ** \retval - 0 on failure
 ******************************************************************************/
uchar send_device_table_get_confirm(uchar attribute_index);

/**
 *******************************************************************************
 ** \brief Function to get confirmation from the security level table for the 
 **       requested attribute index
 ** \param attribute_index - index to the security level table
 ** \retval - 1 on successful extracting the data at the requested index
 ** \retval - 0 on failure
 ******************************************************************************/
uchar send_security_level_table_get_confirm(uchar attribute_index);

/**
 *******************************************************************************
 ** \brief Function initialising the CCA state machine
 ** \param *p_sec_item - pointer to enqueue the security item
 ** \retval - None
 ******************************************************************************/
void Enqueue_Secure_Item( security_struct_t * p_sec_item );
#endif

#if(CFG_MLME_GET_REQ_CONF == 1)
/* mac_downlink.c */
/**
 *******************************************************************************
 ** \brief Processes the MLME-GET.request primitive received from the NHL
 ** \param *buf - pointer to buffer containing get request data
 ** \param length -  unused 
 ** \retval - Get confirm status 
 ******************************************************************************/
uchar process_mlme_get_request( uchar *buf, uint16_t length );
#endif	/*(CFG_MLME_GET_REQ_CONF == 1)*/
#if(CFG_MLME_SET_REQ_CONF == 1)
/**
 *******************************************************************************
 ** \brief Processes the MLME-SET.request primitive received from the NHL
 ** \param *buf - pointer to buffer containing set request data
 ** \param length -  unused 
 ** \retval - set confirm status 
 ******************************************************************************/
uchar process_mlme_set_request( uchar *buf, uint16_t length );
#endif	/*(CFG_MLME_SET_REQ_CONF == 1)*/

/**
 *******************************************************************************
 ** \brief Processes the mac primitive received from SPI port
 ** \param *bufp - pointer to buffer 
 ** \param length - length of received data
 **  \retval 0 if not processed due to insufficient resources
 **  \retval 1 if processed ok
 **  \retval 2 if invalid primitive
 ******************************************************************************/
uchar mac_process_primitive( uchar *bufp, uint16_t length );

/**
 *******************************************************************************
 ** \brief Function to set the mode of the processor
 ** There are 3 power save modes -
 **    1. deep sleep  - where the clock is stopped and the RAM is saved to 1K
 **    2. sleep - where the clock is stopped and
 **    3. snooze - where the 12MHz continues to run, but the micro snoozes
 ** \param - None
 ** \retval - None
 ******************************************************************************/
void mac_set_processor_mode(void);

/* mac_beacon.c */
/**
 *******************************************************************************
 ** \brief Creates a beacon request frame
 ** \param br_type - beacon type
 ** \retval - status
 ******************************************************************************/
uchar mac_create_beacon_request( uint8_t br_type );

/**
 *******************************************************************************
 ** \brief Processes an incoming beacon by extracting relevant BO and SO, 
 ** PAN ID and coordinator short address
 ** \param *rxb - buffer containing the beacon to process
 ** \retval 1 if this buffer is still needed
 ** \retval 0 if it can be reused
 ******************************************************************************/
uchar process_beacon(mac_rx_t *rxb);

/**
 *******************************************************************************
 ** \brief Function to update the beacon with the new values
 ** \param sub_type - type of beacon
 ** \retval - status
 ******************************************************************************/
mac_status_t mac_beacon_update( uchar sub_type );

/* macproc.c */
/**
 *******************************************************************************
 ** \brief Function to get the activity of the processor
 ** \param - None
 ** \retval - processor state
 ******************************************************************************/
uint32_t get_processor_activity( void );

/**
 *******************************************************************************
 ** \brief Function still not used
 ** \param backoff_interval - not used
 ** \retval - None
 ******************************************************************************/
void enable_backoff_timer(ushort backoff_interval);

/**
 *******************************************************************************
 ** \brief Function to find a pending tx data item from the indirect queue
 ** \param *dst - pointer to destination address to search for data item
 ** \param check_pan_id - indicates whether the pan id match is required
 ** \retval - SUCCESS if found else INVALID_HANDLE
 ******************************************************************************/
uchar mac_find_indirect_transmission( mac_address_t *dst, uchar check_pan_id );

/**
 *******************************************************************************
 ** \brief Function called from the main loop.Processes MAC command Data request
 **  messages received from the PHY and subsequently queued.Beacons and acks are 
 **  processed in the ISR.
 ** \param - None
 ** \retval - 1 on SUCCESS 
 ** \retval - 0 on FAIL
 ******************************************************************************/
uchar mac_process_data_requests(void);

/**
 *******************************************************************************
 ** \brief Function still not used
 ** \param - None
 ** \retval - None
 ******************************************************************************/
void mac_check_clock_adjust(void);

/**
 *******************************************************************************
 ** \brief Function still not used
 ** \param - None
 ** \retval - None
 ******************************************************************************/
void mac_sync_clock_adjust(void);

/**
 *******************************************************************************
 ** \brief Function to create and queue a broadcast coordinator realignment 
 ** message prior to changing beacon characteristics
 ** \param - None
 ** \retval - status
 ******************************************************************************/
//uchar mac_create_coordinator_realignment(void);

/**
 *******************************************************************************
 ** \brief Function called by the ISR (or foreground) when a CAP message has
 ** been dealt with.
 ** \param *dm - pointer to completed message
 ** \param reason - mac status code
 ** \retval - None
 ******************************************************************************/
void mac_cap_msg_complete(mac_tx_t *dm, mac_status_t reason);

/**
 *******************************************************************************
 ** \brief Function still not used
 ** \param - None
 ** \retval - None
 ******************************************************************************/
void mac_clear_cap_msg(void);

/**
 *******************************************************************************
 ** \brief Function called by the ISR (or foreground) when a GTS message has
 **  been dealt with
 ** \param *dm - pointer to completed message
 ** \param reason - mac status code
 ** \retval - None
 ******************************************************************************/
void mac_gts_msg_complete(mac_tx_t *dm, mac_status_t reason);

/**
 *******************************************************************************
 ** \brief Function to create a panid conflict MAC command message
 ** \param *src - pointer to the source addess
 ** \param *dst - pointer to the destination addess
 ** \param *sec_param - pointer to the security material
 ** \retval - status
 ******************************************************************************/
uchar mac_create_panid_conflict_notification(void);

/**
 *******************************************************************************
 ** \brief Function to set specific processor activity flag(s)
 ** \param activity_flag - bit field for current tasks 
 ** \retval - None
 ******************************************************************************/
void set_process_activity(ushort activity_flag);

/**
 *******************************************************************************
 ** \brief Function to clear specific processor activity flag(s)
 ** \param activity_flag - bit field for current tasks 
 ** \retval - None
 ******************************************************************************/
void clear_process_activity(ushort activity_flag);

void set_uart_tx_activity(uint8_t status);

#if	(CFG_ORPHAN_SCAN == 1)
/**
 *******************************************************************************
 ** \brief Function called from scan request(Orphan scan)
 ** \param - None
 ** \retval - status
 ** \note Interrupts are disabled around this call
 ******************************************************************************/
uchar mac_create_orphan_notification(void);
#endif	/*(CFG_ORPHAN_SCAN == 1)*/


#ifdef MAC_CFG_GTS_ENABLED
/**
 *******************************************************************************
 ** \brief Function to create a GTS request message, queues it and is ready to send
 ** \param *buf - pointer to serial buffer from nwk layer
 ** \retval - status
 ******************************************************************************/
uchar mac_create_gts_request(uchar *buf);

/**
 *******************************************************************************
 ** \brief Function allocates the GTS structure for the slot(s) in the main 
 **  mac_data structure.This is copied into the beacon at the start of each frame
 ** \param s_address - address
 ** \param *params - pointer to the gts parameters  
 ** \retval - status
 ******************************************************************************/
uchar mac_gts_allocate(ushort s_address, uchar *params);
#endif

/**
 *******************************************************************************
 ** \brief Function processes received beacons from the queue in the main loop
 ** \param - None
 ** \retval - 1 on SUCCESS
 ** \retval - 0 on FAIL
 ******************************************************************************/
uchar mac_process_beacons(void);

/**
 *******************************************************************************
 ** \brief Function to process the beacon request
 ** \param *mrp - pointer to the beacon packet 
 ** \retval - 0 0n successful processing 
 ** \retval - 1 on unsuccessful processing
 ******************************************************************************/
int process_beacon_request( mac_rx_t *mrp );
#ifdef WISUN_FAN_MAC
int process_async_frame( mac_rx_t *mrp );
#endif

#ifdef MAC_CFG_GTS_ENABLED
/**
 *******************************************************************************
 ** \brief Function still not used
 ** \param - None
 ** \retval - None
 ******************************************************************************/
void macsm_wait_for_gts_sent(void);

/**
 *******************************************************************************
 ** \brief Function still not used
 ** \param - None
 ** \retval - None
 ******************************************************************************/
void macsm_tx_gts(void);

/**
 *******************************************************************************
 ** \brief Function still not used
 ** \param - None
 ** \retval - None
 ******************************************************************************/
ushort mac_get_gts_expiration_time(void);
#endif

/* mac_slot.c */
/**
 *******************************************************************************
 ** \brief Function still not used
 ** \param - None
 ** \retval - None
 ******************************************************************************/
ulong number_of_backoffs_in_slot(void);

/**
 *******************************************************************************
 ** \brief Function not used
 ** \param - None
 ** \retval - None
 ******************************************************************************/
void mac_buffer_available(void);

/* phyprimif.c */
/* received from NHL */
/**
 *******************************************************************************
 ** \brief Processes the PHY data primitive received from SPI port
 ** \param *buf - pointer to the buffer
 ** \param length - length of the buffer 
 ** \retval - 1 if success
 ** \retval - 0 if fail
 ******************************************************************************/
uchar process_pd_data_request(uchar *buf, uchar length);

/**
 *******************************************************************************
 ** \brief Processes the PHY management primitive received from SPI port
 ** \param *buf - pointer to the buffer
 ** \param length - length of the buffer 
 ** \retval - 1 if success
 ** \retval - 0 if fail
 ******************************************************************************/
uchar process_plme_ed_request(uchar *buf, uchar length);

/**
 *******************************************************************************
 ** \brief Processes the PHY CCA primitive received from SPI port
 ** \param *buf - pointer to the buffer
 ** \param length - length of the buffer 
 ** \retval - 1 if success
 ** \retval - 0 if fail
 ******************************************************************************/
uchar process_plme_cca_request(uchar *buf, uchar length);

/**
 *******************************************************************************
 ** \brief Processes the SET-TRX_STATE.request primitive received from SPI port
 ** \param *buf - pointer to the buffer
 ** \param length - length of the buffer 
 ** \retval - 1 if success
 ** \retval - 0 if fail
 ******************************************************************************/
uchar process_plme_set_trx_state_request(uchar *buf, uchar length);

/* sent to NHL */
/**
 *******************************************************************************
 ** \brief Creates and send the pd data confirmation primitive to the SPI port
 ** \param status - value of data confirmation
 ** \retval - 1 if success
 ** \retval - 0 if fail
 ******************************************************************************/
uchar send_pd_data_confirm(uchar status);

/**
 *******************************************************************************
 ** \brief Creates and send the pd data indication primitive to the NHL,usually 
 ** the SPI
 ** \param *buffer - pointer to the buffer
 ** \param length - length of the buffer 
 ** \retval - 1 if success
 ** \retval - 0 if fail
 ******************************************************************************/
uchar send_pd_data_indication(uchar *buffer, uchar length);

/**
 *******************************************************************************
 ** \brief Entry point for plme ed confirm from PHY
 ** \param status - PHY status
 ** \param value - ed value
 ** \retval - 1 if success
 ** \retval - 0 if fail
 ******************************************************************************/
uchar send_plme_ed_confirm(uchar status, uchar value);

/**
 *******************************************************************************
 ** \brief Entry point for plme cca confirm from PHY
 ** \param status - PHY status
 ** \retval - 1 if success
 ** \retval - 0 if fail
 ******************************************************************************/
uchar send_plme_cca_confirm(uchar status);

/**
 *******************************************************************************
 ** \brief Entry point for plme set trx state confirm from PHY
 ** \param status - PHY status
 ** \retval - 1 if success
 ** \retval - 0 if fail
 ******************************************************************************/
uchar send_plme_set_trx_state_confirm(uchar status);


/* appprimif.c */
/**
 *******************************************************************************
 ** \brief Function still not used
 ** \param *buf - pointer to the buffer
 ** \param length - length of the buffer 
 ** \retval - 1 if success
 ** \retval - 0 if fail
 ******************************************************************************/
uchar process_app_clock_request(uchar *buf, uchar length);

/**
 *******************************************************************************
 ** \brief Function still not used
 ** \param *buf - pointer to the buffer
 ** \param length - length of the buffer 
 ** \retval - 1 if success
 ** \retval - 0 if fail
 ******************************************************************************/
uchar process_app_test_request(uchar *buf, uchar length);

/**
 *******************************************************************************
 ** \brief Function still not used
 ** \param *buf - pointer to the buffer
 ** \param length - length of the buffer 
 ** \retval - 1 if success
 ** \retval - 0 if fail
 ******************************************************************************/
uchar process_app_timer_request(uchar *buf, uchar length);

/* sent to NHL */

/**
 *******************************************************************************
 ** \brief Function still not used
 ** \param status - 
 ** \param test - 
 ** \param resultsize -
 ** \param *resultlist -
 ** \retval - 
 ******************************************************************************/
uchar send_app_test_confirm(uchar status, uchar test, uchar resultsize, uchar *resultlist);

/**
 *******************************************************************************
 ** \brief Function still not used
 ** \param status -
 ** \retval - 
 ******************************************************************************/
uchar send_app_timer_confirm(uchar status);

/**
 *******************************************************************************
 ** \brief Function still not used
 ** \param - None
 ** \retval - 
 ******************************************************************************/
uchar send_app_timer_indication(void);

/**
 *******************************************************************************
 ** \brief Function to append frame counter to message
 ** \param *bp - location where the frame counter needs to be appended
 ** \retval - None
 ******************************************************************************/
void append_frame_counter( uchar *bp );

/**
 *******************************************************************************
 ** \brief Function to free a mac buffer
 ** \param *mac_rxp - pointer to a buffer
 ** \retval - None
 ******************************************************************************/
void mac_free_rcv_buffer( mac_rx_t *mac_rxp );

#ifdef WISUN_FAN_MAC
uchar Create_MLME_WS_ASYNC_FRAME_Request( uint32_t Start_Channel, 
                                          uint8_t fan_frame_type, 
                                          uint32_t hdr_bitmap,//uint8_t *hdr_ie_list, 
                                          uint32_t pld_bitmap);//uint8_t *payload_ie_list);


#if(FAN_EAPOL_FEATURE_ENABLED == 1)
uchar Create_MCPS_EAPOL_FRAME_Request( uint32_t Start_Channel, 
                                          uint8_t fan_frame_type, 
                                          uint8_t *hdr_ie_list, 
                                          uint8_t *payload_ie_list);
#endif

#if(FAN_EDFE_FEATURE_ENABLED == 1)
uchar create_edfe_frame(uint8_t *dest_addr,uint32_t sub_hdr_bitmap, uint32_t sub_pld_bitmap);
#endif

#endif

/*Suneet :: these macro use for parse fcf option*/
/*  Special IEEE802.15.4 Addresses */
#define IEEE802154_NO_ADDR16                0xFFFE
#define IEEE802154_BCAST_ADDR               0xFFFF
#define IEEE802154_BCAST_PAN                0xFFFF

/* Frame version definitions. */
#define IEEE802154_VERSION_2003                0x0
#define IEEE802154_VERSION_2006                0x1
#define IEEE802154_VERSION_2015                0x2
#define IEEE802154_VERSION_RESERVED            0x3

/*  Bit-masks for the FCF */
#define IEEE802154_FCF_TYPE_MASK            0x0007  /* Frame Type Mask */
#define IEEE802154_FCF_SEC_EN               0x0008
#define IEEE802154_FCF_FRAME_PND            0x0010
#define IEEE802154_FCF_ACK_REQ              0x0020
#define IEEE802154_FCF_PAN_ID_COMPRESSION   0x0040  /* known as Intra PAN prior to IEEE 802.15.4-2006 */
#define IEEE802154_FCF_SEQNO_SUPPRESSION    0x0100
#define IEEE802154_FCF_IE_PRESENT           0x0200
#define IEEE802154_FCF_DADDR_MASK           0x0C00  /* destination addressing mask */
#define IEEE802154_FCF_VERSION              0x3000
#define IEEE802154_FCF_SADDR_MASK           0xC000  /* source addressing mask */

/* Frame Type Definitions */
#define IEEE802154_FCF_BEACON                  0x0  /* Beacon Frame */
#define IEEE802154_FCF_DATA                    0x1  /* Data Frame */
#define IEEE802154_FCF_ACK                     0x2  /* Acknowlegement Frame */
#define IEEE802154_FCF_CMD                     0x3  /* MAC Command Frame */
#define IEEE802154_FCF_RESERVED                0x4  /* reserved */
#define IEEE802154_FCF_MULTIPURPOSE            0x5  /* Multipurpose */
#define IEEE802154_FCF_FRAGMENT                0x6  /* Fragment or Frak */
#define IEEE802154_FCF_EXTENDED                0x7  /* Extended */

/* Address Mode Definitions */
#define IEEE802154_FCF_ADDR_NONE               0x0
#define IEEE802154_FCF_ADDR_RESERVED           0x1
#define IEEE802154_FCF_ADDR_SHORT              0x2
#define IEEE802154_FCF_ADDR_EXT                0x3

/* Auxiliary Security Header */
#define IEEE802154_AUX_SEC_LEVEL_MASK                 0x07  /* Security Level */
#define IEEE802154_AUX_KEY_ID_MODE_MASK               0x18  /* Key Identifier Mode */
#define IEEE802154_AUX_KEY_ID_MODE_SHIFT              3
#define IEEE802154_AUX_FRAME_COUNTER_SUPPRESSION_MASK 0x20  /* 802.15.4-2015 */
#define IEEE802154_AUX_ASN_IN_NONCE_MASK              0x40  /* 802.15.4-2015 */
/* Note: 802.15.4-2015 specifies bits 6-7 as reserved, but 6 is used for ASN */
#define IEEE802154_AUX_CTRL_RESERVED_MASK             0x80  /* Reserved */


#ifdef __cplusplus
}
#endif
#endif /* MAC_DEFS_H */

