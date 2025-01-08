/** \file mac_interface_layer.h
 *******************************************************************************
 ** \brief Provides different structure definitions required for MAC_Interface
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

#ifndef MAC_INTERFACE_LAYER_H
#define MAC_INTERFACE_LAYER_H


#include "fan_mac_ie.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
** =============================================================================
** Public Macro definitions
** =============================================================================
*/

/* None */

/**
 * \defgroup mac_interface MAC Interface Layer
 */

/**
 ** \defgroup mac_defs  MAC Interface Layer Definitions
 ** \ingroup mac_interface
 */

/*@{*/

/*
** =============================================================================
** Public Structures, Unions & enums Type Definitions
** =============================================================================
**/

/**
 *******************************************************************************
 ** \struct mac_address_struct
 ** Structure to indicate address of Source and Destination
 *******************************************************************************
 **/
#ifndef MAC_ADDR_STRUCT
#define MAC_ADDR_STRUCT
typedef struct mac_address_struct
{
    uint8_t address_mode;         /* addressing mode */
    ushort pan_id;                /* pan id (if present)*/
    union
    {
        ushort short_address;     /* short address */
        uint8_t *ieee_address;    /* IEEE address */
    } address;
} mac_address_t;
#endif

/**
 *******************************************************************************
 ** \struct mac_app_address_struct
 ** Structure to indicate address of Source and Destination
 *******************************************************************************
 **/
typedef struct mac_app_address_struct
{
	uint8_t address_mode;         /* addressing mode          */
    ushort pan_id;                /* pan id (if present)      */

    union
    {
        ushort short_address;     /* short address */
        uint64_t ieee_address;    /* IEEE address  */
    } address;

}mac_app_address_t;

/**
 *******************************************************************************
 ** \struct security_params
 ** Structure to indicate security features
 *******************************************************************************
 **/
#ifndef SEC_PARAMS_STRUCT
#define SEC_PARAMS_STRUCT
typedef struct security_params
{
    uint8_t security_level;
    uint8_t key_id_mode;
    uint8_t key_identifier[9];
} security_params_t;
#endif

/**
 *******************************************************************************
 ** \struct pandesc_t
 ** Structure to indicate parameters of Pandescriptor
 *******************************************************************************
 **/
typedef struct
{
    mac_address_t address;
    uint8_t channel;
    uint8_t page;
    uint16_t   SuperframeSpec;
    uint8_t gts_permit;
    uint8_t link_quality;
    uint32_t timestamp;
    uint8_t sec_status;
    security_params_t* secparms;
} pandesc_t;

/**
 *******************************************************************************
 ** \struct mac_app_pandesc_struct
 ** Structure to indicate parameters of Pandescriptor
 *******************************************************************************
 **/
typedef struct mac_app_pandesc_struct
{
    mac_app_address_t address;
    uint8_t channel;
    uint8_t page;
    uint16_t   SuperframeSpec;
    uint8_t gts_permit;
    uint8_t link_quality;
    uint32_t timestamp;
    uint8_t sec_status;
	security_params_t secparms;
} app_pandesc_t;

/**
 *******************************************************************************
 ** \struct mac_to_nhle_process_table_struct
 ** Structure to indicate MAC to NHL processing table
 *******************************************************************************
 **/
typedef struct mac_to_nhle_process_table_struct
{
	uchar primitive;
	void (* action)( uchar *buffer );
} mac_to_nhle_process_table_t;

/*@}*/

/*
** ============================================================================
** Public Function Prototypes
** ============================================================================
*/
 
/** \defgroup mac_interface_functions MAC Interface Functions
 ** \ingroup mac_interface
 */

/** \defgroup mac_interface_required_functions MAC Required Interface Functions
 *  \ingroup mac_interface_functions
 */

/*@{*/

extern void mem_rev_cpy(uint8_t* dest, uint8_t* src, uint16_t len );

/**
 *******************************************************************************
 ** \brief Implemented to initialise to the MAC Interface Layer
 ** \param cold_start - if set to true loads all the mac PIBs to default 
 **                         values
 ** \retval - None
 ******************************************************************************/
void MIL_Init( uint8_t cold_start );

/**
 *******************************************************************************
 ** \brief Implemented to process all the events coming from MAC
 ** \param - None
 ** \retval - None
 ******************************************************************************/
void MIL_Task( void );
 
/**
 *******************************************************************************
 ** \brief Implemented to process all MIL messages
 ** \param - None
 ** \retval - None
 ******************************************************************************/
void process_mil_msgs( void );

/*@}*/

/*
** ============================================================================
**  MCPS SAP Primitives
** ============================================================================
*/

/** \defgroup mac_mcps_interface_functions MAC MCPS Interface Functions
 *  \ingroup mac_interface_functions
 */

/*@{*/

/**
 *******************************************************************************
 ** \brief Implemented to process Data Request
 ** \param *SrcAddr - pointer to the addressing mode of source
 ** \param *DstAddr - pointer to the addressing mode, PANID, Addr of destination
 ** \param msduLength - length of the payload to be transmitted
 ** \param *msdu - pointer to the payload to be transmitted 
 ** \param TxOptions - indicates the transmission option for eg indirect/direct 
 **                    or withack/withoutack or withGTS/withoutGTS
 ** \param TxChannel - the channel on which to send the PPDU
 ** \param PPDUCoding - indicates if coding needs to be applied to 
 ** \param FCSLength - indicates the length of FCS appended by MAC
 ** \param ModeSwitch - indicates if MAC has to send a mode switch PPDU before
 **                       sending the actual MPDU 
 ** \param NewModeSUNPage - the channel page number to be used in case mode 
 **                         switch is requested.
 ** \param ModeSwitchParameterEntry - specifies index in the 
 **                     phyModeSwitchParameterEntries table which needs to be
 **                         used for sending the MPDU.
 ** \param frameCtrlOptions - indicates a combination of enumerations used for
 **                    building the frame control field for multipurpose frame
 ** \param headerIElistLen - indicates the number of IEs sent in the header of 
 **                       the frame
 ** \param *headerIElist - pointer to which Header Information Elements are sent
 **                in the frame. IES_INCLUDED must be set in frameControlOptions 
 **                 if present
 ** \param payloadIElistLen - indicates the number of IEs sent in the payload of 
 **                        the frame
 ** \param *payloadIElist - pointer to which Payload Information Elements are sent
 **       in the frame. IES_INCLUDED must be set in frameControlOptions if present
 ** \param sendMultiPurpose - indicates if a multipurpose frame needs to be sent
 ** \param *pSecstuff - material used for security processing of the MAC frame 
 ** \retval - None
 ******************************************************************************/
void MCPS_DATA_Request
(
	mac_address_t	*SrcAddr, /* Source Addressing Mode*/
	mac_address_t *DstAddr, /* Dst Addressing Mode, DstPANID,Dst addr*/		
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
);


void FAN_MCPS_DATA_Request
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
 uint32_t sub_hdr_bitmap,  
 uint32_t sub_pld_bitmap,
 bool sendMultiPurpose,
 security_params_t* pSecstuff
);

#if(CFG_MCPS_PURGE_REQ_CONF == 1)
/**
 *******************************************************************************
 ** \brief This function is used to cancel a previously requested indirect data 
 **                  transmission 
 ** \param msduHandle - handle of the MSDU to be purged from
 **            the transaction queue
 ** \retval - None
 ******************************************************************************/
void MCPS_PURGE_Request
(
  uchar	msduHandle		              /* Handle associated with payload	*/
);

#endif	/*(CFG_MCPS_PURGE_REQ_CONF == 1)*/
/*@}*/

/*
** ============================================================================
**  MLME SAP Primitives
** ============================================================================
*/

/** \defgroup mac_mlme_interface_functions MAC MLME Interface Functions
 *  \ingroup mac_interface_functions
 */
   
   
#ifdef WISUN_FAN_MAC   
void WS_ASYNC_FRAME_request(
                             uint8_t operation, 
                             uint8_t frametype,                             
                             uint8_t* channellist,
                             uint8_t length
                           );

#if(FAN_EAPOL_FEATURE_ENABLED == 1)
void fan_eapol_request(void);
#endif

#endif

/*@{*/

#if(CFG_MLME_ASSOCIATE_REQ_CONF == 1)
/**
 *******************************************************************************
 ** \brief This function is used by the NHLE for requesting an association with   
 **                  a PAN through a PAN coordinator
 ** \param LogicalChannel - indicates the channel on which the association should 
 **                            be attempted by the MLME
 ** \param channelPage - indicates the channel page on which the association should
 **                         be attempted by the MLME
 ** \param *coordAddr - pointer to object holding coordinator address mode,
 **                     coordinator address and the coordinator PAN ID information.
 ** \param CapabilityInfo - specifies different kinds of operational capabilities 
 **                         of the device attempting association
 ** \param *pLowLatencyNwkInfo - pointer to the low latency information that should 
 **                       be used by the MAC Layer for joining a LL network
 ** \param channelOffset - indicates the offset value of the channel Hopping sequence
 ** \param HoppingSequenceID - indicates the ID of channel hopping sequence 
 ** \param *pSecstuff - material used for security processing of the MAC frame 
 ** \retval - None
 ******************************************************************************/
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
);

#endif	/*(CFG_MLME_ASSOCIATE_REQ_CONF == 1)*/

#if(CFG_MLME_ASSOCIATE_IND_RESP == 1)
/**
 *******************************************************************************
 ** \brief This function is used for responding to the MLME Associate Indication.
 **        This function is used by the NHLE as a response to association request
 **            sent by a remote device.
 ** \param *DeviceExtAddress - pointer to the device’s extended address which is 
 **              attempting to associate with the PAN
 ** \param AssocShortAddress - indicates the 16-bit short address allocated by 
 **                 the coordinator to the device attempting to associate
 ** \param Status - indicates the status of the association attempt
 ** \param *pLowLatencyNwkInfo - Upper layer specific information on LLDN. 
 **                  Significant only if it is a low latency network
 ** \param channelOffset - indicates the channel offset value of the channel
 **     hopping sequence. Significance only if channel hopping is being used
 ** \param channelHoppingSeqLength - length of the channel hopping sequence
 ** \param *pchannelHoppingSeq - pointer to the memory which holds the channel 
 **          hopping sequence
 ** \param *pSecstuff - material used for security processing of the MAC frame 
 ** \retval - None
 ******************************************************************************/
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
);
#endif	/*(CFG_MLME_ASSOCIATE_IND_RESP == 1)*/

#if(CFG_MLME_GET_REQ_CONF == 1)
/**
 *******************************************************************************
 ** \brief A synchronous function which returns the attribute value through the 
 **    output pointer
 ** \param PIBAttribute - ID indicating the attribute to be read from the MAC
 ** \param PIBAttributeIndex - index to the table entry which is being read, 
 **          if the PIBAttribute indicates a PIB Table
 ** \retval - None
 ******************************************************************************/
void MLME_GET_Request
(
	uint8_t PIBAttribute,
	uint8_t PIBAttributeIndex
);
#endif	/*(CFG_MLME_GET_REQ_CONF == 1)*/
#if(CGF_MLME_RESET_REQ_CONF == 1)
/**
 *******************************************************************************
 ** \brief This function is used for resetting the MAC Layer.
 ** \param SetDefaultPIB - indicates whether the MAC layer has to set all its 
 **                     PIBs to their default values.
 ** \retval - None
 ******************************************************************************/
void MLME_RESET_Request
(
  uchar	SetDefaultPIB	              /* Also Reset PIB */
);
#endif	/*(CGF_MLME_RESET_REQ_CONF == 1)*/

#if (CFG_MLME_SCAN_REQ_CONF == 1)
/**
 *******************************************************************************
 ** \brief This API is used if the upper layer wishes to perform the network 
 **                      scanning. 
 ** \param ScanType - indicates the type of scan to be performed
 ** \param scanChannels - indicates the list of channels which need to be scanned
 ** \param ScanDuration - duration to be spent in a particular channel while scanning,
 **                      ignored for orphaned scan
 ** \param channelPage - channel page to be used for scanning
 ** \param LinkQualityScan - When set to TRUE continues channel scan on other channels 
 **          till the PANdescriptor reaches the limit or scanduration is elapsed
 ** \param frameCtrlOptions - indicates a combination of enumerations used for
 **                    building the frame control field for multipurpose frame
 ** \param headerIElistLen - indicates the number of IEs sent in the header of 
 **                       the frame
 ** \param *headerIElist - pointer to which Header Information Elements are sent
 **                in the frame. IES_INCLUDED must be set in frameControlOptions 
 **                 if present
 ** \param payloadIElistLen - indicates the number of IEs sent in the payload of 
 **                        the frame
 ** \param *payloadIElist - pointer to which Payload Information Elements are sent
 **       in the frame. IES_INCLUDED must be set in frameControlOptions if present
 ** \param *pSecstuff - material used for security processing of the MAC frame 
 ** \retval - None
 ******************************************************************************/
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
#if(1)
	uint8_t mpm_scanduration_bpan,
	uint16_t mpm_scanduration_nbpan,
#endif
	security_params_t* pSecstuff  
);

#endif	/*(CFG_MLME_SCAN_REQ_CONF == 1)*/

/**
 *******************************************************************************
 ** \brief This function is used for setting PIB attribute. 
 ** This is a synchronous function which does not need to be put in the MLME Q 
 **          for MAC Processing. 
 ** \param PIBAttribute - ID indicating the attribute to be read from the MAC
 ** \param PIBAttributeIndex - index to the table entry which is being read, 
 **          if the PIBAttribute indicates a PIB Table
 ** \param PIBAttributeLength - indicates the length of the attribute value
 ** \param PIBAttributeValue - pointer to memory holding the attribute value to set
 ** \retval - None
 ******************************************************************************/
void MLME_SET_Request
(
  uint8_t PIBAttribute,		            /* Attribute to update */
  uint8_t PIBAttributeIndex, 			/* index of the attribute to be updated */	        
  uint16_t  PIBAttributeLength,		  	/* length of value */
  void *PIBAttributeValue	          	/* pointer to Value	*/
);

#if(CFG_MLME_START_REQ_CONF == 1)
/**
 *******************************************************************************
 ** \brief This function is implemented to Start a network
 ** \param PANId - PAN Id to be used by the coordinator
 ** \param LogicalChannel - – the channel on which the new super frame configuration
 **            needs to be started
 ** \param ChannelPage - Indicates the channel page on which the new super frame 
 **                       configuration needs to be started
 ** \param startTime - Indicates the relative time at which the device should start
 **              transmitting the beacons
 ** \param BeaconOrder - Indicates how often the Beacon needs to be transmitted
 ** \param SuperframeOrder - Indicates the duration of the active portion of the 
 **             super frame
 ** \param PANCoordinator - Indicates if the device transmitting the beacons should 
 **                  act as a PAN coordinator
 ** \param BatteryLifeExtension - Indicates if the beaconing device should switch
 **     off its receiver for a known duration soon after transmission of the beacon
 ** \param CoordRealignment - Indicates if the device needs to transmit a coordinator
 **          realignment command before changing the super frame configuration 
 ** \param *pCRSecstuff - material used for security processing of the coordinator 
 **                    realignment command frame
 ** \param *pBCNSecstuff - material used for security processing of the beacon frame 
 ** \param *pDSMESuperframeSpecification - poiter to indicate the DSME superframe 
 **               structure 
 ** \param *pBeaconBitMap - poiter to describes the beacon scheduling 
 ** \param hoppingDescriptor - Specifies the information about the channel hopping
 ** \param headerIElistLen - indicates the number of IEs sent in the header of 
 **                       the frame
 ** \param *headerIElist - pointer to which Header Information Elements are sent
 **                in the frame. IES_INCLUDED must be set in frameControlOptions 
 **                 if present
 ** \param payloadIElistLen - indicates the number of IEs sent in the payload of 
 **                        the frame
 ** \param *payloadIElist - pointer to which Payload Information Elements are sent
 **       in the frame. IES_INCLUDED must be set in frameControlOptions if present
 ** \param CoexBeaconOrder - Indicates how often the coex-beacon is to
 **             be transmitted
 ** \param OffsetTimeOrder - Indicates the time difference between the
 **            coex-beacon and the preceding periodic beacon
 ** \param CoexBeaconNBPAN - Indicates how often the coex-beacon is to be 
 **        transmitted in a non-beacon-enabled  PAN (i.e. BeaconOrder=15)
 ** \retval - None
 ******************************************************************************/

#ifdef WISUN_FAN_MAC   
void FAN_MAC_MLME_SET_Request
(
  uint8_t ie_identifier,		            /* header ie or payload ie */
  uint8_t ie_subid, 			/* subid for each ie */	        
  uint16_t Length,		  	/* length of value */
  void *mac_value	          	/* */
);

#else    

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
	uint8_t CoexBeaconOrder,
	uint8_t  OffsetTimeOrder,
	uint8_t CoexBeaconNBPAN
#else
	uint8_t MPM_EB_IEListLen,
	uint8_t* MPM_EB_IEList,
	uint8_t EnhancedBeaconOrder,
	uint8_t  OffsetTimeSlot,
	uint16_t NBPANEnhancedBeaconOrder,
#endif
	security_params_t* pCRSecstuff ,
	security_params_t* pBCNSecstuff 
	);
#endif	/*(CFG_MLME_START_REQ_CONF == 1)*/
 
#endif

#if(CFG_MLME_SYNC_REQ==1)

/**
 *******************************************************************************
 ** \brief This function is used in a beacon enabled network for synchronizing
 **   with the coordinator 
 ** \param LogicalChannel - indicates the channel on which the synchronization
 **  should be attempted by the MLME
 ** \param ChannelPage - indicates the channel page on which the synchronization
 **  should be attempted by the MLME
 ** \param TrackBeacon - TRUE if the MLME is to synchronize with the
 **  next beacon and attempt to track all future
 **  beacons. 
 **  FALSE if the MLME is to synchronize with only the next beacon
 ** \retval - None
 ******************************************************************************/

void MLME_SYNC_Request
(
 uint8_t LogicalChannel, 
 uint8_t ChannelPage,
 uint8_t TrackBeacon         
 );
#endif /*#if(CFG_MLME_SYNC_REQ==1)*/
 
#if(CFG_MLME_POLL_REQ_CONF == 1)
/**
 *******************************************************************************
 ** \brief This function is used in a non beacon enabled network for synchronizing
 **   with the coordinator 
 ** \param *CoordAddress - pointer to the object holding the information about the 
 **   address mode, address and pan id of the coordinator to be polled
 ** \param *pSecstuff - material used for security processing of the MAC frame 
 ** \retval - None
 ******************************************************************************/
void MLME_POLL_Request
(
	mac_address_t *CoordAddress,	      
	security_params_t* pSecstuff  				
);
#endif	/*(CFG_MLME_POLL_REQ_CONF == 1)*/

#if(CFG_MLME_ORPHAN_IND_RESP == 1)
/**
 *******************************************************************************
 ** \brief This function is used in a non beacon enabled network for synchronizing
 **   the orphan device with its parent 
 ** \param *OrphanAddress - pointer to the address of the orphaned device
 ** \param ShortAddress - short address of the orphan device which was assigned to
 **                       it during association
 ** \param AssociatedMember - set to true indicating that it was its child earlier
 ** \param *pSecstuff - material used for security processing of the MAC frame 
 ** \retval - None
 ******************************************************************************/
void MLME_ORPHAN_Response
(
	 uint8_t*	OrphanAddress,			
	 ushort 	ShortAddress,			
	 uint8_t	AssociatedMember,	
	 security_params_t* pSecstuff  				
);

#endif	/*(CFG_MLME_ORPHAN_IND_RESP == 1)*/

#if(CFG_MLME_DISASSOCIATE_REQ_CONF == 1)
/**
 *******************************************************************************
 ** \brief This function is used in a non beacon enabled network for disaaociating
 **        from the network
 ** \param DeviceAddressMode - Address mode of the device for which to send the orphan
 **                            notification command
 ** \param DevPANID - PANID of the device for which to send the orphan
 **                            notification command 
 ** \param *DeviceAddress - pointer to address of the node for which to send the orphan
 **                            notification command
 ** \param DisassociateReason - Indicates device wants to leave network or the PAN Coordinator
 **                          decides the device to leave the network
 ** \param TxDirect - mode of tranmission of the request indirect or direct 
 ** \param *pSecstuff - material used for security processing of the MAC frame 
 ** \retval - None
 ******************************************************************************/
void MLME_DISASSOCIATE_Request
(
	uint8_t  DeviceAddressMode,
	ushort    DevPANID,
	uint8_t *DeviceAddress,			
	uint8_t  DisassociateReason,
	bool        TxDirect,	
	security_params_t* pSecstuff 
);

#endif	/*(CFG_MLME_DISASSOCIATE_REQ_CONF == 1)*/

#if(CGF_MLME_BEACON_REQ_CONF == 1)
/**
 *******************************************************************************
 ** \brief This function is used in a non beacon enabled network for requesting
 **        the beacons transmission form the FFD to the requested Device.
 ** \param bcnType - 0x00 for beacon OR 0x01 for enhanced beacon
 ** \param channel - The channel on which the Beacon should be transmitted
 ** \param channelPage - Indicates the channel page on which the beacon should
 **                      be attempted by the MLME
 ** \param superFrameOrder - Indicates the duration of the active portion of the 
 **							 super frame
 ** \param *DstAddr -
 ** \param *pSecstuff - material used for security processing of the MAC frame 
 ** \retval - None
 ******************************************************************************/
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
);
#endif	/*(CGF_MLME_BEACON_REQ_CONF == 1)*/

#ifdef MAC_CFG_SECURITY_ENABLED
/**
 *******************************************************************************
 ** \brief Fuction to get the PIB attributes value length
 ** \param sec_table_attrib_id - PIB attribute ID
 ** \param *p_attrib_val - pointer to store the length of the PIB attributes value
 ** \retval - length of th PIB attribute value
 ******************************************************************************/
uint16_t get_mac_sec_table_attrib_len
( 
	uint8_t sec_table_attrib_id,
	uint8_t* p_attrib_val 
);

#endif

/*@}*/

/*
** ============================================================================
** MCPS application callback interface
** ============================================================================
*/
extern void App_MCPS_Data_Conf_cb
(
	uint8_t msduHandle, 
	uint8_t status, 
	uint8_t NumBackoffs,
	uint32_t Timestamp
);

/******************************************************************************/

extern void App_MCPS_Data_Ind_cb
(
	mac_address_t*  pSrcaddr,
	mac_address_t*  pDstaddr,
	uint16_t msduLength,
	uint8_t* pMsdu,
	uint8_t mpduLinkQuality,
	uint8_t DSN,
        uint8_t pld_ies_present,
	uint64_t Timestamp,
	security_params_t* pSec 
);

#ifdef WISUN_FAN_MAC

extern void App_FAN_MCPS_Data_Conf_cb
(
	uint8_t msduHandle, 
	uint8_t status, 
	uint8_t NumBackoffs,
	uint32_t Timestamp
);

/******************************************************************************/
#ifdef WISUN_FAN_MAC
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
#endif


/******************************************************************************/

extern void App_MCPS_Purge_Conf_cb
(
	uint8_t msdu_handle, 
	uint8_t status
);

/******************************************************************************/

/*
** ============================================================================
** MLME application callback interface
** ============================================================================
*/
extern void App_MLME_Associate_Conf_cb
(
	uint16_t short_address,
	uint8_t status,
	uint8_t* pLowLatencyNwkInfo,
	uint16 channelOffset,
	uint8_t  HoppingSequenceLength,
	uint8_t HoppingSequence,
	security_params_t *sec_params
);

/******************************************************************************/
#if(CFG_MLME_ASSOCIATE_IND_RESP == 1)
extern void App_MLME_Associate_Ind_cb
(
uint8_t* pChild_64_bit_addr,
uint8_t CapabilityInformation,
uint8_t* pLowLatencyNwkInfo,
uint16 channelOffset,
uint8_t  HoppingSequenceID,
security_params_t *sec_params
);
#endif	/*(CFG_MLME_ASSOCIATE_IND_RESP == 1)*/
/******************************************************************************/

extern void  App_MLME_Beacon_Notify_Ind_cb
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

/******************************************************************************/

extern void  App_MLME_Comm_Status_Ind_cb
(
	mac_address_t* pSrcaddr,
	mac_address_t * pDstaddr,
	uint8_t status,
	security_params_t *sec_param
);


/******************************************************************************/
#if(CFG_MLME_POLL_REQ_CONF == 1)
extern void App_MLME_Poll_Conf_cb
(
uint8_t status 
);
#endif	/*(CFG_MLME_POLL_REQ_CONF == 1)*/


/******************************************************************************/

#ifdef WISUN_FAN_MAC
extern void  App_MLME_ws_async_frame_Conf_cb
(
	uint8_t status,
        uint8_t async_frame_type
);
#endif

/******************************************************************************/

#if(CGF_MLME_RESET_REQ_CONF == 1)
extern void  App_MLME_Reset_Conf_cb
(
	uint8_t status
);
#endif	/*(CGF_MLME_RESET_REQ_CONF == 1)*/

/******************************************************************************/
#if (CFG_MLME_SCAN_REQ_CONF == 1)
extern void App_MLME_Scan_Conf_cb
(
	uint8_t status,
	uint8_t ScanType,
	uint8_t ChannelPage,
	uint8_t* UnscannedChannels,
	uint8_t ResultListSize,
	void *ResultList
);
#endif /*(CFG_MLME_SCAN_REQ_CONF == 1)*/

/******************************************************************************/
#if(CFG_MLME_START_REQ_CONF == 1)
extern void App_MLME_Start_Conf_cb
(
	uint8_t status
);
#ifdef WISUN_FAN_MAC 
extern void App_MLME_Set_Mac_Channel_Info_Conf_cb
(
	uint8_t status
);
#if(APP_LBR_ROUTER == 1)
extern void App_MLME_Fan_Mac_Set_Conf_cb
(
	uint8_t status,
        uint8_t sub_ie_value
);
#endif//APP_LBR_ROUTER
#endif /*(end of WISUN_FAN_MAC)*/

#endif	/*(CFG_MLME_START_REQ_CONF == 1)*/

/******************************************************************************/

#if(CFG_MLME_SET_REQ_CONF == 1) 
extern void App_MLME_SET_Conf_cb
(
 uint8_t status,
 uint8_t PIBAttribute,
 uint8_t PIBAttributeIndex
);
#endif	/*(CFG_MLME_SET_REQ_CONF == 1) */

/******************************************************************************/
#if(CFG_MLME_GET_REQ_CONF == 1)
extern void App_MLME_GET_Conf_cb
(
 uint8_t status,
 uint8_t PIBAttribute,
 uint8_t PIBAttributeIndex,
 uint16_t PIBAttributeLength,
 void *PIBAttributeValue
);
#endif	/*(CFG_MLME_GET_REQ_CONF == 1)*/

/******************************************************************************/
#if(CFG_MLME_DISASSOCIATE_REQ_CONF == 1)
extern void App_MLME_Dissociate_Ind_cb
(
	uint8_t* DeviceAddress,
	uint8_t DisassociateReason,
	security_params_t *sec_params
);
#endif	/*(CFG_MLME_DISASSOCIATE_REQ_CONF == 1)*/

/******************************************************************************/

extern void App_MLME_Dissociate_Conf_cb
(

	uint8_t status,
	mac_address_t* Deviceaddr 
);

/******************************************************************************/
#if(CFG_MLME_ORPHAN_IND_RESP == 1)

extern void App_MLME_Orphan_Ind_cb
(
	uint8_t* pOrphan64bitAddress, 
	security_params_t *sec_params
 
);
#endif	/*(CFG_MLME_ORPHAN_IND_RESP == 1)*/


/******************************************************************************/

extern void App_MLME_Sync_Loss_Ind_cb
(
	uint8_t LossReason,			
	uint16_t PANId,					
	uint8_t LogicalChannel, 		
	uint8_t ChannelPage,			
	security_params_t *sec_params			
);

/******************************************************************************/

#ifdef WISUN_FAN_MAC
void App_MLME_WS_ASYNC_FRAME_Ind_cb
(
     uint8_t status,
     uint8_t frame_type,
     mac_address_t *p_Srcaddr // Src address details
);
#endif

#if(CGF_MLME_BEACON_REQUEST_IND == 1)
extern void App_MLME_Beacon_Request_Ind_cb
(
	uchar bcn_type,
	mac_address_t* src_addr,
	ushort dest_pan_id,
	ushort ie_list_fld_size,
	uchar* p_ie_list	
);
#endif	/*(CGF_MLME_BEACON_REQUEST_IND == 1)*/
/******************************************************************************/
#if(CGF_MLME_BEACON_REQ_CONF == 1)
extern void App_MLME_Beacon_Conf_cb( uchar status );
#endif	/*(CGF_MLME_BEACON_REQ_CONF == 1)*/

uint8_t* get_pairing_id(void);
/******************************************************************************/

#ifdef __cplusplus
}
#endif
#endif /* MAC_INTERFACE_LAYER_H */
