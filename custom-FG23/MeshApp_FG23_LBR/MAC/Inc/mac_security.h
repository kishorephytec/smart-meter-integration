/** \file mac_security.h
 *******************************************************************************
 ** \brief Provides different structure definitions required for MAC Security
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

#ifndef MAC_SECURITY_H
#define MAC_SECURITY_H

#ifdef __cplusplus
extern "C" {
#endif

/*
** ============================================================================
** Public Macro definitions
** ============================================================================
*/

/*! Defines the Security Level mask bits */
#define SEC_LEVEL_MASK 0x07  //(bit(0)| bit(1)| bit(2))
/*! Defines Key ID mask bits */
#define KEY_ID_MASK 0x30     //(bit(4)| bit(5))

#ifdef MAC_CFG_SECURITY_ENABLED

/*
** ============================================================================
** Public Structures, Unions & enums Type Definitions
** ============================================================================
*/

/* None */

/*
** ============================================================================
** Public Variable Declarations
** ============================================================================
*/

extern uint8_t send_wrong_mic;

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

/**
 *******************************************************************************
 ** \brief Initialises the queues(mac_key_table, mac_device_table and
 **		   mac_security_level_table)
 ** \param - None 
 ** \retval - None
 ******************************************************************************/
void sec_list_initialise( void );

/**
 *******************************************************************************
 ** \brief Function to look for the appropriate Key_Descriptor for the received 
 **        Key_lookup_data and updating the Key_Descriptor. 
 ** \param *key_lookup_data- Pointer to the Key_Lookup_Data
 ** \param key_lookup_size- Indicates the size of the Key_Lookup_Data
 ** \param **pKD - [Out] pointer in which the KeyDescriptor to be updated
 ** \retval - TRUE or FALSE
 ******************************************************************************/
uchar key_descriptor_lookup_procedure
(
	uchar *key_lookup_data,
	uchar key_lookup_size,
	key_descriptor_t **pKD
);

/**
 *******************************************************************************
 ** \brief Function to check for the Black listed device
 ** \param *pKD - Pointer to the KeyDescriptor 
 ** \param *lookup_data - Pointer to localy generated lookup data 
 ** \param lookup_size - Indicates the Lookup Data Size
 ** \param **pKDD -[Out] Pointer to the Key Device Descriptor 
 ** \param **pDD - [Out] Pointer to the DeviceDescriptor 
 ** \retval - TRUE or FALSE
 ******************************************************************************/
uchar blacklist_checking_procedure
(
	key_descriptor_t *pKD,
	uchar *lookup_data,
	uchar lookup_size,
	key_device_descriptor_t **pKDD, /* KeyDeviceDescriptor to be returned */
	device_descriptor_t **pDD		/* DeviceDescriptor to be returned */
);

device_descriptor_t *get_device_descriptior_from_pib (uchar *lookup_data);      //Debdeep :: 16-jan-2019

/**
 *******************************************************************************
 ** \brief Function for the Security Level Checking procedure
 ** \param incoming_security_level - Indicates the security level of the 
 **		   received frame.
 ** \param frame_type - Indicates the type of the frame received.
 ** \param command_frame_identifier - Indicates the command frame Id of the 
 **        received frame.
 ** \retval - TRUE or FALSE
 ******************************************************************************/
uchar security_level_check( uchar incoming_security_level, uchar frame_type, uchar command_frame_identifier );

/**
 *******************************************************************************
 ** \brief Function for device descriptor matching procedure.
 ** \param *pDD - Pointer to the device descriptor entry present in the 
 **        macKeyTable 
 ** \param *lookup_data -Pointer to the locally generated lookupdata for match 
 **        procedure
 ** \param lookup_size - Indicates the lookup data size
 ** \retval - TRUE or FALSE
 ******************************************************************************/
uchar device_descriptor_match_procedure( device_descriptor_t *pDD, uchar *lookup_data, uchar lookup_size );

/**
 *******************************************************************************
 ** \brief Function to generate a key for the outgoing packet
 ** \param *src_address - pointer to source address 
 ** \param *dst_address - pointer to destination address
 ** \param key_id_mode - Indicates the KeyIdMode to be used for generating the 
 **        Key
 ** \param *key_identifier - Pointer to the KeySource and KeyIndex.
 ** \param *key - [Out] pointer which should be updated with the Key being 
 **        generated by this procedure
 ** \retval - TRUE or FALSE
 ******************************************************************************/
uchar outgoing_key_retrieval_procedure
(
    mac_address_t *src_address,
    mac_address_t *dst_address,
    uchar key_id_mode,
    uchar *key_identifier, /* Key Source and Key Index */
    uchar *key             /* The Key to be Returned if Sucessful */
);

/**
 *******************************************************************************
 ** \brief Function to extract the key of an incoming packet
 ** \param *src_address - pointer to source address 
 ** \param *dst_address - pointer to destination address
 ** \param key_id_mode - Indicates the KeyIdMode to be used for extracting the 
 **        Key
 ** \param *key_identifier - Pointer to the KeySource and KeyIndex.
 ** \param **pKD -[Out] Pointer to the KeyDescriptor present in the received frame.
 ** \param **pDD -[Out] Pointer to the DeviceDescriptor present in the received frame.
 ** \retval - TRUE or FALSE
 ******************************************************************************/
uchar incoming_key_retrieval_procedure
(
    mac_address_t *src_address,
    mac_address_t *dst_address,
	uchar key_id_mode,
	uchar *key_identifier,
    key_descriptor_t **pKD,
    key_device_descriptor_t **pKDD,
	device_descriptor_t **pDD
);

/**
 *******************************************************************************
 ** \brief Function for incoming key usage checking procedure.
 ** \param **pKD- Pointer to the keyDescriptor present in the macKeyTable 
 ** \param frame_type- Inidicates the frame type present in the received packet
 ** \param command_frame_identifier- Indicates the command Id present in the 
 **        received packet.
 ** \retval - TRUE or FALSE
 ******************************************************************************/
uchar incoming_key_usage_check( key_descriptor_t **pKD, uchar frame_type, uchar command_frame_identifier );

/**
 *******************************************************************************
 ** \brief Function to clear event for given priority
 ** \param prio - priority 
 ** \retval - None
 ******************************************************************************/
void init_nonce( uchar *nonce, ulong frame_counter, uchar security_level, uchar *IEEE_addr );

/**
 *******************************************************************************
 ** \brief Returns the length of the MIC depending on the SecurityLevel
 ** \param security_level - Holds the value from 0x00 - 0x07 
 ** \retval - The length of the MIC to be added or removed
 ******************************************************************************/
uchar integrity_code_length( uchar security_level );

/**
 *******************************************************************************
 ** \brief Function extract the security material from the incoming packet
 ** \param *rxmsg - Pointer to the received packet 
 ** \retval - None
 ******************************************************************************/
void mac_get_security_data( mac_rx_t *rxmsg );

/**
 *******************************************************************************
 ** \brief Function to seperate the header and data information for 
 **        encryption/decryption.
 ** \param *mac_buffer - pointer to the received beacon 
 ** \retval - None
 ******************************************************************************/
uchar get_bcn_payload_offset( uchar *mac_buffer );

/**
 *******************************************************************************
 ** \brief Function to copy and arrange the key in a proper format
 ** \param *d - Pointer to the destination location	
 ** \param *s - Pointer to the Source location
 ** \retval - None
 ******************************************************************************/
void copy_key( uchar *d, uchar *s);

/**
 *******************************************************************************
 ** \brief Function to copy and arrange the address in a proper format
 ** \param *d - Pointer to the destination location	
 ** \param *s - Pointer to the Source location
 ** \retval - None
 ******************************************************************************/
void copy_IEEE_address( uchar *d, uchar *s);

/**
 *******************************************************************************
 ** \brief Function to clear event for given priority
 ** \param prio - priority 
 ** \retval - None
 ******************************************************************************/
uchar restore_security_data( uchar *dst );

/**
 *******************************************************************************
 ** \brief Function to clear event for given priority
 ** \param prio - priority 
 ** \retval - None
 ******************************************************************************/
uchar save_security_data( uchar *dst );
/**
 *******************************************************************************
 ** \brief Function to initialise the lists(key_device_list, key_id_lookup_list 
 **        and key_usage_list)
 ** \param *p - pointer to the KeyDescriptor which contains the above specified 
 **        lists to be initialised 
 ** \retval - None
 ******************************************************************************/
void initialise_key_descriptor( key_descriptor_t *p );

/**
 *******************************************************************************
 ** \brief Function to check the authentication status of the received frame
 ** \param *rxmsg - Pointer to the received frame 
 ** \retval - None
 ******************************************************************************/
void check_authentication_status( mac_rx_t *rxmsg );

/**
 *******************************************************************************
 ** \brief Function to make an entry into the macKeyTable
 ** \param *data -  Pointer to the data for creating the table entry
 ** \param table_entry - Specifies the index in the table
 ** \param restore - Contains TRUE or FALSE
 ** \retval - returns the offset
 ******************************************************************************/
uchar store_key_table_entry_in_pib( uchar *data, uchar table_entry );

/**
 *******************************************************************************
 ** \brief Function to make an entry into the macDeviceTable
 ** \param *data -  Pointer to the data for creating the table entry
 ** \param table_entry - Specifies the index in the table
 ** \param restore - Contains TRUE or FALSE
 ** \retval - returns the offset
 ******************************************************************************/
uchar store_device_table_entry_in_pib( uchar *data, uchar table_entry );

/**
 *******************************************************************************
 ** \brief Function to make an entry into the macSecurityLevelTable
 ** \param *data -  Pointer to the data for creating the table entry
 ** \param table_entry - Specifies the index in the table
 ** \param restore - Contains TRUE or FALSE
 ** \retval - returns the offset
 ******************************************************************************/
uchar store_security_level_table_entry_in_pib( uchar *data, uchar table_entry );

/**
 *******************************************************************************
 ** \brief Function to retrieve the data from macKeyTable
 ** \param *buf - [Out] Pointer to be updated with the values
 ** \param *p_kd- Pointer to the table entry
 ** \param store - Contains TRUE or FALSE
 ** \retval - returns the offset
 ******************************************************************************/
uchar write_key_table_entry_to_buffer( uchar *buf, key_descriptor_t* p_kd, uchar store );

/**
 *******************************************************************************
 ** \brief Function to retrieve the data from macDeviceTable
 ** \param *buf - [Out] Pointer to be updated with the values
 ** \param *pdd- Pointer to the table entry
 ** \param store - Contains TRUE or FALSE
 ** \retval - returns the offset
 ******************************************************************************/
uchar write_device_table_entry_to_buffer( uchar *buf, device_descriptor_t *pdd, uchar store );

/**
 *******************************************************************************
 ** \brief Function to retrieve the data from macSecurityLevelTable
 ** \param *buf - [Out] Pointer to be updated with the values
 ** \param *p_sld- Pointer to the table entry
 ** \param store - Contains TRUE or FALSE
 ** \retval - returns the offset
 ******************************************************************************/
uchar write_security_level_table_entry_to_buffer(	uchar *buf, 
													security_level_descriptor_t *p_sld, 
													uchar store 
												 );

/**
 *******************************************************************************
 ** \brief Function to delete an entry from the SecurityLevelTable based on 
 **        the index
 ** \param table_entry - Index to the entry to be deleted
 ** \retval - TRUE or FALSE
 ******************************************************************************/
uchar delete_security_level_table_entry( uchar table_entry );

/**
 *******************************************************************************
 ** \brief Function to find an entry in the SecurityLevelTable based on 
 **        the index
 ** \param table_entry - Index for the entry to be searched
 ** \retval - returns the pointer to the searched entry
 ******************************************************************************/
security_level_descriptor_t* find_security_level_table_entry( uchar table_entry );

/**
 *******************************************************************************
 ** \brief Function to delete an entry from the DeviceTable based on the index
 ** \param table_entry - Index to the entry to be deleted
 ** \retval - TRUE or FALSE
 ******************************************************************************/
uchar delete_device_table_entry( uchar table_entry );

/**
 *******************************************************************************
 ** \brief Function to find an entry in the DeviceTable based on the index     
 ** \param table_entry - Index for the entry to be searched
 ** \retval - returns the pointer to the searched entry
 ******************************************************************************/
device_descriptor_t* find_device_table_entry( uchar table_entry );

/**
 *******************************************************************************
 ** \brief Function to delete an entry from the KeyTable based on the index
 ** \param table_entry - Index to the entry to be deleted
 ** \retval - TRUE or FALSE
 ******************************************************************************/
uchar delete_key_table_entry( uchar table_entry );

/**
 *******************************************************************************
 ** \brief Function to find an entry in the KeyTable based on the index     
 ** \param table_entry - Index for the entry to be searched
 ** \retval - returns the pointer to the searched entry
 ******************************************************************************/
key_descriptor_t* find_key_table_entry( uchar table_entry );

/**
 *******************************************************************************
 ** \brief Function to find the key identifier length based on the KeyIdMode
 ** \param key_id_mode -Holds the KeyIdMode(0x00 - 0x03)
 ** \retval - returns the length of the KeyIdentifier.
 ******************************************************************************/
/* required even when security is not included in build */
uchar get_key_identifier_length( uchar key_id_mode );

/**
 *******************************************************************************
 ** \brief Function to perform security processing on the received packet.
 ** \param -  None
 ** \retval - TRUE or FALSE
 ******************************************************************************/
uint8_t mac_incoming_security_processor( void );

/**
 *******************************************************************************
 ** \brief Function to perform security processing for the outgoing packet.
 ** \param -  None
 ** \retval - TRUE or FALSE
 ******************************************************************************/
uint8_t mac_outgoing_security_processor( void );

#endif /* MAC_CFG_SECURITY_ENABLED */

#ifdef __cplusplus
}
#endif
#endif /* MAC_SECURITY_H */

