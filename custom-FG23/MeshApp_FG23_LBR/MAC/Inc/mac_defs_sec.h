/** \file mac_defs_sec.h
 *******************************************************************************
 ** \brief Provides information about the MAC Layer Definitions for Security
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

#ifndef MAC_DEFS_SEC_H
#define MAC_DEFS_SEC_H

#ifdef __cplusplus
extern "C" {
#endif

/*
** ============================================================================
** Public Macro definitions
** ============================================================================
*/

/*! Macro defining value for MAC_DTX_STATE_NOT_PROCESSED*/
#define MAC_DTX_STATE_NOT_PROCESSED                 0
/*! Macro defining value for MAC_DTX_STATE_ENCRYPTION_COMPLETE*/
#define MAC_DTX_STATE_ENCRYPTION_COMPLETE           1
/*! Macro defining value for MAC_RX_STATE_NOT_PROCESSED*/
#define MAC_RX_STATE_NOT_PROCESSED                  2
/*! Macro defining value for MAC_RX_STATE_DECRYPTION_COMPLETE*/
#define MAC_RX_STATE_DECRYPTION_COMPLETE            3
/*! Macro defining value for MAC_RX_STATE_FAILED_AUTHENTICATION*/
#define MAC_RX_STATE_FAILED_AUTHENTICATION          4
/*! Macro defining value for MAC_DTX_STATE_FAILED_AUTHENTICATION*/
#define MAC_DTX_STATE_FAILED_AUTHENTICATION         5

/*! Macro defining value for SF_NONCE_IEEE_ADDRESS*/
#define SF_NONCE_IEEE_ADDRESS                       1
/*! Macro defining value for SF_NONCE_FRAME_COUNTER*/
#define SF_NONCE_FRAME_COUNTER                      2
/*! Macro defining value for SF_FRAME_COUNTER*/
#define SF_FRAME_COUNTER                            4
/*! Macro defining value for SF_KEY*/
#define SF_KEY                                      8
/*! Macro defining value for SF_IEEE_ADDR*/
#define SF_IEEE_ADDR                               16
/*! Macro defining mask value for INCOMING_FRAME_SECURITY_LEVEL_MASK*/
#define INCOMING_FRAME_SECURITY_LEVEL_MASK       0x07

/*
** ============================================================================
** Public Structures, Unions & enums Type Definitions
** ============================================================================
*/

/**
 *******************************************************************************
 ** \struct security_params_t
 ** Structure to store secuirty parameter
 *******************************************************************************
 **/
#ifndef SEC_PARAMS_STRUCT
#define SEC_PARAMS_STRUCT
typedef struct security_params
{
    uchar security_level;
    uchar key_id_mode;
    uchar key_identifier[9];
} security_params_t;
#endif

/**
 *******************************************************************************
 ** \struct security_struct_t
 ** Structure to store secuirty materials
 *******************************************************************************
 **/
typedef struct security_struct
{
    queue_item_t q_item;        /**<  make this a queuable item */
    uchar header_length;        /**<  length of the data */
    uchar *header;              /**<  pointer to start of header - a data */
    uint16_t payload_length;    /**<  length in bytes of payload  */
    uchar *payload;             /**<  pointer to start of payload - m data */
    uchar *raw_payload_data;    /**<  Suneet :: pointer to hold of payload - m data update mic when tetranmit packet */
    uchar *output_data;         /**<  pointer to the location for the encrpyted data */
    uchar state;                /**<  security processing state */
    uchar key[16];              /**<  we copy the key incase software tries to change it before msg is encrypted */
    uchar nonce[13];            
    uchar security_level;       /**<  security level to use to encrypt/authenticate the message */
    void *private_msg_data;     /**<  pointer to the original dtx structure containing the message */
    queue_t *return_queue;      /**<  queue to put message on after encryption */
    uint32_t event_prio;        /**<  MAC Event to be raised after putting a msg into the queue pointed by return_queue*/ 
    uint8_t *frame_counter_ptr; /* Debdeep :: 22-jan-2019 :: for updating frame counter in mac retry */
} security_struct_t;

// KEY DEVICE DESCRIPTOR
/**
 *******************************************************************************
 ** \struct key_device_descriptor_t
 ** Structure to store key device descriptor details
 *******************************************************************************
 **/
typedef struct key_device_descriptor_struct
{
    struct key_device_descriptor_struct *next;
    uchar device_table_entry_handle;
    uchar unique_device;
    uchar black_listed;
} key_device_descriptor_t;

// SECURITY LEVEL DESCRIPTOR
/**
 *******************************************************************************
 ** \struct security_level_descriptor_t
 ** Structure to store security level descriptor details
 *******************************************************************************
 **/
typedef struct security_level_descriptor_struct
{
    struct security_level_descriptor_struct *next;
    uchar index;
    uchar frame_type;
    uchar command_frame_identifier;
    uchar security_minimum;
    uchar device_overide_security_minimum;
} security_level_descriptor_t;

// KEY USAGE DESCRIPTOR
/**
 *******************************************************************************
 ** \struct key_usage_descriptor_t
 ** Structure to store security key usage details
 *******************************************************************************
 **/
typedef struct security_key_usage_struct
{
    struct security_key_usage_struct *next;
    uchar frame_type;
    uchar command_frame_identifier;
} key_usage_descriptor_t;

// DEVICE DESCRIPTOR
/**
 *******************************************************************************
 ** \struct device_descriptor_t
 ** Structure to store device descriptor details
 *******************************************************************************
 **/
   // RAka  ...
   // secFrameCounterPerKey
#define  MAX_MAC_KEY_SUPPORTED  4
   
   
typedef struct device_descriptor_struct
{
    struct device_descriptor_struct *next;
//    uchar index;				//Debdeep :: 16-jan-2019  :: Not required
//    ushort pan_id;            //Debdeep :: 16-jan-2019  :: Not required
//    ushort short_addr;        //Debdeep :: 16-jan-2019  :: Not required
    uchar ieee_addr[IEEE_ADDRESS_LENGTH];
    ulong frame_count[MAX_MAC_KEY_SUPPORTED];
//    uchar exempt;             //Debdeep :: 16-jan-2019  :: Not required
} device_descriptor_t;

// KEY ID LOOKUP DESCRIPTOR
/**
 *******************************************************************************
 ** \struct key_id_lookup_descriptor_t
 ** Structure to store keyid lookup descriptor details
 *******************************************************************************
 **/
typedef struct key_id_lookup_descriptor_struct
{
    struct key_id_lookup_descriptor_struct *next;
    uchar lookup_data[KEY_IDENTIFIER_MAX_LENGTH];
    uchar lookup_data_size;
} key_id_lookup_descriptor_t;

// KEY DESCRIPTOR
/**
 *******************************************************************************
 ** \struct key_descriptor_t
 ** Structure to store key descriptor details
 *******************************************************************************
 **/
#if 0
typedef struct key_descriptor_struct
{
    struct key_descriptor_struct *next;
    uchar index;
    queue_t key_id_lookup_list;
    uchar key_id_lookup_list_entries;
    queue_t key_device_list;
    uchar key_device_list_entries;
    queue_t key_usage_list;
    uchar key_usage_list_entries;
    uchar key[KEY_LENGTH];
} key_descriptor_t;
#endif

typedef struct mac_key_descriptor_sruct
{
  struct key_id_lookup_descriptor_struct *next;
  uint8_t index;
  uint8_t mac_key[16];
}mac_key_descriptor_t;

typedef struct key_descriptor_struct
{
    struct key_descriptor_struct *next;
    queue_t key_id_lookup_list;
    uchar key_id_lookup_list_entries;
    uchar index;
    queue_t mac_key_list;
    uchar mac_key_list_entries;
} key_descriptor_t;
/**
 *******************************************************************************
 ** \struct hallin_queue_t
 ** Structure to store hallin queue details
 *******************************************************************************
 **/
typedef struct hallin_queue_struct
{
    security_struct_t *start;
    security_struct_t *end;
    uchar count;
} hallin_queue_t;

/**
 *******************************************************************************
 ** \struct security_pib_t
 ** Structure to store security PIB details
 *******************************************************************************
 **/
typedef struct security_pib
{
    queue_t mac_key_table;
    uchar mac_key_table_entries;
    queue_t mac_device_table;
    uchar mac_device_table_entries;
    queue_t mac_security_level_table;
    uchar mac_security_level_table_entries;
    ulong mac_frame_counter;
    uchar auto_request_security_level;
    uchar auto_request_keyid_mode;
    uchar auto_request_key_source[KEY_SOURCE_LENGTH];
    uchar auto_request_key_index;
    uchar mac_default_key_source[KEY_SOURCE_LENGTH];
    uchar mac_PAN_coord_extended_address[IEEE_ADDRESS_LENGTH];
    ushort mac_PAN_coord_short_address;
    ushort mac_sec_frame_counter_perkey;
} security_pib_t;

/**
 *******************************************************************************
 ** \struct mac_security_data_t
 ** Structure to store security data details
 *******************************************************************************
 **/
typedef struct mac_security_data_struct
{
    security_pib_t pib;
    security_params_t coord_realign_sec_param;
    security_params_t beacon_sec_param;
    security_params_t mpm_eb_sec_param;
    uchar security_flags;
    queue_t hallin_tx_queue; /* Queue of messages to be encrypted */
    queue_t hallin_rx_queue; /* Queue of messages to be decrypted */
    queue_t rx_security_queue;    /* Queue of secured messages to be processed */
    security_struct_t *encrypt_msg; /* Message currently being encrypted */
    security_struct_t *decrypt_msg; /* Message currently being decrypted */
    security_struct_t beacon_data;  /* Statically allocated Security Buffer for Beacons */
    security_struct_t mpm_eb_data;  /* Statically allocated Security Buffer for Beacons */
} mac_security_data_t;

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

/* None */

/*
**=========================================================================
**  Public Function Prototypes
**=========================================================================
*/

/* None */

#ifdef __cplusplus
}
#endif
#endif /* MAC_DEFS_SEC_H */


