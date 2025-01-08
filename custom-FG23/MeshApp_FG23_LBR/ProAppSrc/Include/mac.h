/** \file mac.h
 *******************************************************************************
 ** \brief Provides different structure definitions required for MAC
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

#ifndef MAC_H
#define MAC_H

#ifdef __cplusplus
extern "C" {
#endif

/*
** =============================================================================
** Public Macro definitions
** =============================================================================
*/

/**
 * \defgroup mac MAC Layer 
 */

/**
 ** \defgroup mac_defs MAC Layer Definitions
 ** \ingroup mac
 */

/*@{*/

/* Macro defined for the IOT6, defines the ENET_MAC_FOR_GU */  
//#define ENET_MAC_FOR_GU
  

/* values for address modes, as passed in primitives */
/*! No Address Mode */
#define ADDR_MODE_NONE          0
/*! Address Mode Reserved */
#define ADDR_MODE_RESVD         1
/*! Short Address Mode */
#define ADDR_MODE_SHORT         2
/*! Extened Address Mode */
#define ADDR_MODE_EXTENDED      3
/*! Length of Short Address Mode */
#define SHORT_ADDRESS_LENGTH            2
/*! Length of Extened Address Mode */
#define IEEE_ADDRESS_LENGTH             8
/*! Length of PAN ID */
#define PANID_LENGTH                    2

//#define MAC_ADDRESS_FILTERING_ENABLE
  
/*! Key ID Mode 0 */
#define KEY_ID_MODE0							 0
/*! Key ID Mode 1 */
#define KEY_ID_MODE1							 1
/*! Key ID Mode 0 */
#define KEY_ID_MODE2							 2
/*! Key ID Mode 0 */
#define KEY_ID_MODE3							 3

/*Lengths (including length of PANID field)*/
/*! Length of No Address Mode */
#define ADDR_MODE_NONE_LENGTH           0
/*! Length of Address Mode Reserved */
#define ADDR_MODE_RESVD_LENGTH          0
/*! Length of Short Address Mode */
#define ADDR_MODE_SHORT_LENGTH          ( SHORT_ADDRESS_LENGTH + 2 )
/*! Length of Extened Address Mode */
#define ADDR_MODE_EXTENDED_LENGTH       ( IEEE_ADDRESS_LENGTH + 2 )

/*! NWK to MAC - Receive Data Enable */
#define MAC_REQUEST_RXEN                        0x01
/*! NWK to MAC - Transmit Data Request */
#define MAC_REQUEST_TXDR                        0x02	
/*! MAC to NWK - Receive Data Available	*/
#define MAC_STATUS_RXDA                         0x01
/*! MAC to NWK - Transmit Data OK */
#define MAC_STATUS_TXOK                         0x02

/*! Channel Page 7 to be used for specifying the channel page */
#define CHANNEL_PAGE_7							0x07

/*! Scan channels to be specified if all the channels to be scanned */
#define SCAN_ALL_CHANNELS						(uint64_t)0x00000001FFFFFFFF	
#define MAC_BOARD_RESET_REQUEST                        0x99

/* Tokens for MAC primitives */
#ifdef WISUN_FAN_MAC 
 /*   user specific macro not in IEEE 802.15.4 spec. Only used to set fan mac channel
  info in phy layer*/
#define MLME_SET_FAN_MAC_CHANNEL_INFO_REQ               (uchar)0xDC 
#define MAC_MLME_SET_MAC_CHANNEL_INFO_CONFIRM           (uchar)0xDD  
    
#define FAN_MAC_MLME_SET_REQUEST                        (uchar)0xDE  
#define FAN_MAC_MLME_SET_CONFIRM                        (uchar)0xDF 
  
  
#define MLME_WS_ASYNC_FRAME_REQ                         0x78
#define WS_ASYNC_FRAME_REQ_CONF                         0xEE
#define WS_ASYNC_FRAME_INDICATION                       0xEF

    
#if(FAN_EAPOL_FEATURE_ENABLED == 1) 
#define MLME_EAPOL_FRAME_REQ                            0x88
#define MLME_EAPOL_INDICATION                           0x89
#endif
  
#define MLME_REQ_CONF                                   0x8A    
  
#define FAN_MAC_MCPS_DATA_REQUEST                       0x7B
#define FAN_MAC_MCPS_DATA_CONFIRM                       0x7C
#define FAN_MAC_MCPS_DATA_INDICATION                    0x7D
#define FAN_MAC_ACK_INDICATION                          0x7E  
#define FAN_MAC_NO_ACK_INDICATION                          0x7F
  
#define WISUN_INFO_HEADER_IE_ID                         0x2A
#define WISUN_INFO_PAYLOAD_IE_ID                        0x04
    
#define WISUN_IE_SUBID_UTT_IE                            0x01
#define WISUN_IE_SUBID_BT_IE                             0x02
#define WISUN_IE_SUBID_FC_IE                             0x03
#define WISUN_IE_SUBID_RSL_IE                            0x04
#define WISUN_IE_SUBID_MHDS_IE                           0x05
    
#define WISUN_IE_SUBID_US_IE                            0x06 /* it is 0x01*/
#define WISUN_IE_SUBID_BS_IE                            0x07 /* it is 0x02*/
#define WISUN_IE_SUBID_VP_IE                            0x08 /* it is 0x03*/
#define WISUN_IE_SUBID_PAN_IE                           0x09 /* it is 0x04*/
#define WISUN_IE_SUBID_NETNAME_IE                       0x0A /* it is 0x05*/
#define WISUN_IE_SUBID_PANVER_IE                        0x0B /* it is 0x06*/
#define WISUN_IE_SUBID_GTKHASH_IE                       0x0C/* it is 0x07*/
//#define WISUN_IE_SUBID_MP_IE                       0x07
    
#endif  
  
/*! Primitive id for MCPS_DATA_REQUEST */
#define MAC_MCPS_DATA_REQUEST                           (uchar)0x40
/*! Primitive id for MCPS_DATA_CONFIRM */
#define MAC_MCPS_DATA_CONFIRM                           (uchar)0x41
/*! Primitive id for MCPS_DATA_INDICATION */
#define MAC_MCPS_DATA_INDICATION                        (uchar)0x42
/*! Primitive id for MCPS_PURGE_REQUEST */
#define MAC_MCPS_PURGE_REQUEST                          (uchar)0x43
/*! Primitive id for MCPS_PURGE_CONFIRM */
#define MAC_MCPS_PURGE_CONFIRM                          (uchar)0x44
/*! Primitive id for MLME_ASSOCIATE_REQUEST */
#define MAC_MLME_ASSOCIATE_REQUEST                      (uchar)0x45
/*! Primitive id for MLME_ASSOCIATE_CONFIRM */
#define MAC_MLME_ASSOCIATE_CONFIRM                      (uchar)0x46
/*! Primitive id for MLME_ASSOCIATE_INDICATION */
#define MAC_MLME_ASSOCIATE_INDICATION                   (uchar)0x47
/*! Primitive id for MLME_ASSOCIATE_RESPONSE */
#define MAC_MLME_ASSOCIATE_RESPONSE                     (uchar)0x48
/*! Primitive id for MLME_DISASSOCIATE_REQUEST */
#define MAC_MLME_DISASSOCIATE_REQUEST                   (uchar)0x49
/*! Primitive id for MLME_DISASSOCIATE_CONFIRM */
#define MAC_MLME_DISASSOCIATE_CONFIRM                   (uchar)0x4a
/*! Primitive id for MLME_DISASSOCIATE_INDICATION */
#define MAC_MLME_DISASSOCIATE_INDICATION                (uchar)0x4b
/*! Primitive id for MLME_BEACON_NOTIFY_INDICATION */
#define MAC_MLME_BEACON_NOTIFY_INDICATION               (uchar)0x4c
/*! Primitive id for MLME_GET_REQUEST */
#define MAC_MLME_GET_REQUEST                            (uchar)0x4d
/*! Primitive id for MLME_GET_CONFIRM */
#define MAC_MLME_GET_CONFIRM                            (uchar)0x4e
/*! Primitive id for MLME_GTS_REQUEST */
#define MAC_MLME_GTS_REQUEST                            (uchar)0x4f
/*! Primitive id for MLME_GTS_CONFIRM */
#define MAC_MLME_GTS_CONFIRM                            (uchar)0x50
/*! Primitive id for MLME_GTS_INDICATION */
#define MAC_MLME_GTS_INDICATION                         (uchar)0x51
/*! Primitive id for MLME_ORPHAN_INDICATION */
#define MAC_MLME_ORPHAN_INDICATION                      (uchar)0x52
/*! Primitive id for MLME_ORPHAN_RESPONSE */
#define MAC_MLME_ORPHAN_RESPONSE                        (uchar)0x53
/*! Primitive id for MLME_RESET_REQUEST */
#define MAC_MLME_RESET_REQUEST                          (uchar)0x54
/*! Primitive id for MLME_RESET_CONFIRM */
#define MAC_MLME_RESET_CONFIRM                          (uchar)0x55
/*! Primitive id for MLME_RX_ENABLE_REQUEST */
#define MAC_MLME_RX_ENABLE_REQUEST                      (uchar)0x56
/*! Primitive id for MLME_RX_ENABLE_CONFIRM */
#define MAC_MLME_RX_ENABLE_CONFIRM                      (uchar)0x57
/*! Primitive id for MLME_SCAN_REQUEST */
#define MAC_MLME_SCAN_REQUEST                           (uchar)0x58
/*! Primitive id for MLME_SCAN_CONFIRM */
#define MAC_MLME_SCAN_CONFIRM                           (uchar)0x59
/*! Primitive id for MLME_COMM_STATUS_INDICATION */
#define MAC_MLME_COMM_STATUS_INDICATION                 (uchar)0x5a
/*! Primitive id for MLME_SET_REQUEST */
#define MAC_MLME_SET_REQUEST                            (uchar)0x5b
/*! Primitive id for MLME_SET_CONFIRM */
#define MAC_MLME_SET_CONFIRM                            (uchar)0x5c
/*! Primitive id for MLME_START_REQUEST */
#define MAC_MLME_START_REQUEST                          (uchar)0x5d
/*! Primitive id for MLME_START_CONFIRM */
#define MAC_MLME_START_CONFIRM                          (uchar)0x5e
/*! Primitive id for MLME_SYNC_REQUEST */
#define MAC_MLME_SYNC_REQUEST                           (uchar)0x5f
/*! Primitive id for MLME_SYNC_LOSS_INDICATION */
#define MAC_MLME_SYNC_LOSS_INDICATION                   (uchar)0x60
/*! Primitive id for MLME_POLL_REQUEST */
#define MAC_MLME_POLL_REQUEST                           (uchar)0x61
/*! Primitive id for MLME_POLL_CONFIRM */
#define MAC_MLME_POLL_CONFIRM                           (uchar)0x62
/*! Primitive id for Error Indication */
#define  MAC_SET_MAC_KEY_TABLE_REQUEST   				(uchar)0xAA
/*! Primitive id for Error Indication */
#define  MAC_SET_DEVICE_TABLE_REQUEST					(uchar)0xBB
/*! Primitive id for Error Indication */
#define  MAC_SET_SECURITY_LEVEL_TABLE_REQUEST 			(uchar)0xCC

/*! Primitive id for MLME_BEACON_REQUEST */
#define MAC_MLME_BEACON_REQUEST							(uchar)0x71

/*! Primitive id for MLME_BEACON_REQUEST */
#define MAC_MLME_BEACON_REQUEST_INDICATION				(uchar)0x72

/*! Primitive id for MLME_BEACON_REQUEST */
#define MAC_MLME_BEACON_CONFIRM							(uchar)0x73

/*! Primitive id for getting the buffer details of the buffers which are unused still */
#define GET_BUFF_CONFIG_DETAILS							(uchar)0x91

/*! Primitive id for getting the buffer details of the buffers which are unused still */
#define GET_BUFF_CONFIG_DETAILS_CONF					(uchar)0x92


#ifdef ENET_MAC_FOR_GU
/*! Primitive id for getting the SEND_CONTINUOUS_DATA_REQ */
#define SEND_CONTINUOUS_DATA_REQ					(uchar)0x93

/*! Primitive id for getting the SEND_CONTINUOUS_DATA_CONF */
#define SEND_CONTINUOUS_DATA_CONF					(uchar)0x94  

/*! Primitive id for getting the STOP_CONT_DATA_TX_REQ */
#define STOP_CONT_DATA_TX_REQ					        (uchar)0xA1

/*! Primitive id for getting the STOP_CONT_DATA_TX_CONF */
#define STOP_CONT_DATA_TX_CONF					        (uchar)0xA2  

/*! Primitive id for getting the CONFIG_TE_BEHAVIOUR_REQ */
#define CONFIG_TE_BEHAVIOUR_REQ 					(uchar)0x95

/*! Primitive id for getting the CONFIG_TE_BEHAVIOUR_CONF */
#define CONFIG_TE_BEHAVIOUR_CONF                                       (uchar)0x96 
    
#endif  
   
   
//   
///*! Primitive id for MAC_WS_ASYNC_FRAME_REQUEST */   
//#define MAC_WS_ASYNC_FRAME_REQUEST                           (uchar)0x97
//   
///*! Primitive id for MAC_WS_ASYNC_FRAME_CONF */
//#define MAC_WS_ASYNC_FRAME_CONF                               (uchar)0x98
//
///*! Primitive id for MAC_WS_ASYNC_FRAME_IND */
//#define MAC_WS_ASYNC_FRAME_IND                                (uchar)0x99 
//   
//   
//
//   
  
/*! Primitive id for Error Indication */
#define MAC_MLME_ERROR_INDICATION                       (uchar)0xff

/*  MSDU transmission options */
/*! MSDU Transmission with Acknowledgement */
#define ACKNOWLEDGED_TRANSMISSION     0x01
/*! MSDU Transmission without Acknowledgement */  
#define NO_ACKNOWLEDGED_TRANSMISSION     0x00   
/*! MSDU Transmission with GTS */
#define GTS_TRANSMISSION              0x02
/*! MSDU Transmission with Indirect Mode */
#define INDIRECT_TRANSMISSION         0x04
/*! MSDU Transmission with Security Enabled */
//#define SECURITY_ENABLED_TRANSMISSION 0x08

/*! Defines Success as the status */

#ifndef SUCCESS
#define SUCCESS	MAC_SUCCESS
#endif

/* Security Mode */
/*! Defines SecurityLevel as 0x00 */
#define MAC_SECURITY_NONE                       0x0

/*! Defines SecurityLevel as 0x01 */
#define MAC_SECURITY_AES_CCMSTAR_MIC_32         0x1

/*! Defines SecurityLevel as 0x02 */
#define MAC_SECURITY_AES_CCMSTAR_MIC_64         0x2

/*! Defines SecurityLevel as 0x03 */
#define MAC_SECURITY_AES_CCMSTAR_MIC_128        0x3

/*! Defines SecurityLevel as 0x04 */
#define MAC_SECURITY_AES_CCMSTAR_ENC            0x4

/*! Defines SecurityLevel as 0x05 */
#define MAC_SECURITY_AES_CCMSTAR_ENC_MIC_32     0x5

/*! Defines SecurityLevel as 0x06 */
#define MAC_SECURITY_AES_CCMSTAR_ENC_MIC_64     0x6

/*! Defines SecurityLevel as 0x07 */
#define MAC_SECURITY_AES_CCMSTAR_ENC_MIC_128    0x7


/*! Defines the Key Length ie 16 bytes */
#define KEY_LENGTH                              16

/*! Defines the Max length of the KeyIdentifier */
#define KEY_IDENTIFIER_MAX_LENGTH                9

/*! Defines the Max length of the LookUpData */
#define MAX_LOOKUP_LENGTH                        9
/*! Defines the Length of each SecurityLevelTable entry */
#define SEC_LEVEL_DESC_LENGTH                    4

/*! Defines the length of each DeviceDesc entry */
#define KEY_DEVICE_DESC_LENGTH                   3

/*! Defines length of DeviceDesc table entry */
#define DEVICE_DESC_LENGTH                      17

/*! Defines size of the lookup descriptor */
#define KEY_ID_LOOKUP_DESC_LENGTH               10

/*! Defines size of the KeyDescriptor */
#define KEY_DESC_LENGTH                         19

/*! Defines size of the KeyUsageDescriptor table entry */
#define KEY_USAGE_DESC_LENGTH                    2

/*! Defines max length of the KeySource */
#define KEY_SOURCE_LENGTH                        8

/*! Defines size of the Frame Counter */
#define FRAME_COUNTER_LENGTH					 4

/*! Defines size of the ack frame */
//#define TRXSM_ACK_FRAME_SIZE  (5+((mac_pib.FCSType)?2:4))
// we need to consider the max ack length as we do not know what ack length the destination will be using
#ifdef WISUN_ENET_PROFILE
#define TRXSM_MAX_ACK_FRAME_SIZE  17
#else   
#define TRXSM_MAX_ACK_FRAME_SIZE  7 // WITH 4 BYTES FCS//DOES not unclude PHR length. PHR length is taken into consideration while calculating the ack wait duration
#endif
  
#define TRXSM_ACK_FRAME_SIZE  TRXSM_MAX_ACK_FRAME_SIZE
/*
** =============================================================================
** Public Structures, Unions & enums Type Definitions
** =============================================================================
**/

/**
 *******************************************************************************
 ** \enum Enumeration for MAC Status
 *******************************************************************************
 **/
typedef enum
{
    MAC_SUCCESS                       = 0x00, /* temporarily removed */
    MAC_COUNTER_ERROR                 = 0xdb, 
    MAC_IMPROPER_KEY_TYPE             = 0xdc,
    MAC_IMPROPER_SECURITY_LEVEL       = 0xdd,
    MAC_UNSUPPORTED_LEGACY            = 0xde,
    MAC_UNSUPPORTED_SECURITY          = 0xdf,
    MAC_BEACON_LOST                   = 0xe0,
    MAC_CHANNEL_ACCESS_FAILURE        = 0xe1,
    MAC_DENIED                        = 0xe2,
    MAC_DISABLE_TRX_FAILURE           = 0xe3,
    MAC_SECURITY_ERROR                = 0xe4,
    MAC_FRAME_TOO_LONG                = 0xe5,
    MAC_INVALID_GTS                   = 0xe6,
    MAC_INVALID_HANDLE                = 0xe7,
    MAC_INVALID_PARAMETER             = 0xe8,
    MAC_NO_ACK                        = 0xe9,
    MAC_NO_BEACON                     = 0xea,
    MAC_NO_DATA                       = 0xeb,
    MAC_NO_SHORT_ADDRESS              = 0xec,
    MAC_OUT_OF_CAP                    = 0xed,
    MAC_PAN_ID_CONFLICT               = 0xee,
    MAC_REALIGNMENT                   = 0xef,
    MAC_TRANSACTION_EXPIRED           = 0xf0,
    MAC_TRANSACTION_OVERFLOW          = 0xf1,
    MAC_TX_ACTIVE                     = 0xf2,
    MAC_UNAVAILABLE_KEY               = 0xf3,
    MAC_UNSUPPORTED_ATTRIBUTE         = 0xf4,
    MAC_INVALID_ADDRESS               = 0xf5,
    MAC_ON_TIME_TOO_LONG              = 0xf6,
    MAC_PAST_TIME                     = 0xf7,
    MAC_TRACKING_OFF                  = 0xf8,
    MAC_INVALID_INDEX                 = 0xf9,
    MAC_LIMIT_REACHED                 = 0xfa,
    MAC_READ_ONLY                     = 0xfb,
    MAC_SCAN_IN_PROGRESS              = 0xfc,
    MAC_SUPERFRAME_OVERLAP            = 0xfd,
    MAC_INVALID_REQUEST		      = 0xfe			/*This was reserved status code. Being used for sending invalid request status. This is a propritary status value */
} mac_status_t;

/*! Defines macro value for PAN Capacity*/
#define MAC_PAN_AT_CAPACITY       		0x01
/*!  Defines macro value for PAN_ACCESS_DENIED*/
#define MAC_PAN_ACCESS_DENIED     		0x02

/*! Value used for enabling the LQI Filter bit in the macEBRFilters PIB */ 
#define MAC_EBR_LQI_FILTER_ENABLE		0x01

/*! Value used for enabling the Percent Filter bit in the macEBRFilters PIB */ 
#define MAC_EBR_PERCENT_FILTER_ENABLE	0x02

/*! Value used for enabling the both LQI Filter bit and Percent Filter bit 
in the macEBRFilters PIB */ 
#define MAC_EBR_ALL_FILTERS_ENABLE		0x03

/* Macros for MAC PIB Attribute */
/*!< Attribute ID for MAC_PIB_ID_BASE*/
#define MAC_PIB_ID_BASE				0x40
/*!< Attribute ID for macAckWaitDuration*/
#define macAckWaitDuration              0x40
/*!< Attribute ID for macAssociationPermit*/
#define macAssociationPermit            0x41
/*!< Attribute ID for macAutoRequest*/
#define macAutoRequest                  0x42
/*!< Attribute ID for macBattLifeExt*/
#define macBattLifeExt                  0x43
/*!< Attribute ID for macBattLifeExtPeriods*/
#define macBattLifeExtPeriods           0x44
/*!< Attribute ID for macBeaconPayload*/
#define macBeaconPayload                0x45
/*!< Attribute ID for macBeaconPayloadLength*/
#define macBeaconPayloadLength          0x46
/*!< Attribute ID for macBeaconOrder*/
#define macBeaconOrder                  0x47
/*!< Attribute ID for macBeaconTxTime*/
#define macBeaconTxTime                 0x48
/*!< Attribute ID for macBSN*/
#define macBSN                          0x49
/*!< Attribute ID for macCoordExtendedAddress*/
#define macCoordExtendedAddress         0x4a
/*!< Attribute ID for macCoordShortAddress*/
#define macCoordShortAddress            0x4b
/*!< Attribute ID for macDSN*/
#define macDSN                          0x4c
/*!< Attribute ID for macGTSPermit*/
#define macGTSPermit                    0x4d
/*!< Attribute ID for macMaxCSMABackoffs*/
#define macMaxCSMABackoffs              0x4e
/*!< Attribute ID for macMinBE*/
#define macMinBE                        0x4f
/*!< Attribute ID for macPANId*/
#define macPANId                        0x50
/*!< Attribute ID for macPromiscuousMode*/
#define macPromiscuousMode              0x51
/*!< Attribute ID for macRxOnWhenIdle*/
#define macRxOnWhenIdle                 0x52
/*!< Attribute ID for macShortAddress*/
#define macShortAddress                 0x53
/*!< Attribute ID for macSuperframeOrder*/
#define macSuperframeOrder              0x54
/*!< Attribute ID for macTransactionPersistenceTime*/
#define macTransactionPersistenceTime	0x55
/*!< Attribute ID for macAssociatedPANCoord*/
#define macAssociatedPANCoord           0x56
/*!< Attribute ID for macMaxBE*/
#define macMaxBE                        0x57
/*!< Attribute ID for macMaxFrameTotalWaitTime*/
#define macMaxFrameTotalWaitTime        0x58
/*!< Attribute ID for macMaxFrameRetries*/
#define macMaxFrameRetries              0x59
/*!< Attribute ID for macResponseWaitTime*/
#define macResponseWaitTime             0x5a
/*!< Attribute ID for macSyncSymbolOffset*/
#define macSyncSymbolOffset             0x5b
/*!< Attribute ID for macTimestampSupported*/
#define macTimestampSupported           0x5c
/*!< Attribute ID for macSecurityEnabled*/
#define macSecurityEnabled              0x5d
/*!< Attribute ID for PIB_ID_BASE */
#define PIB_ID_BASE                 0x61

/*Attribute ID for Listen Before Talk */
/*!< Attribute ID for macLBTActiveDuration*/
#define macLBTActiveDuration            0x61
/*!< Attribute ID for macLBTSuspendedDuration*/
#define macLBTSuspendedDuration         0x62
/*!< Attribute ID for macTxTotalDuration */
#define macTxTotalDuration              0x63
/*!< Attribute ID for macLBTSamplingDuration */
#define macLBTSamplingDuration          0x64
/*!< Attribute ID for macLBTSamplingPeriod */
#define macLBTSamplingPeriod            0x65
/*!< Attribute ID for macLBTCCADuration */
#define macLBTCCADuration               0x66
/*!< Attribute ID for macLBTFlags */
#define macLBTFlags                     0x67
/*!< Attribute ID for macLowPowerMode */
#define macLowPowerMode                   0x68
/*!< Attribute ID for macAckTime */
#define macAckTime                      0x69
/*!< Attribute ID for macStatusFlags */
#define macStatusFlags                  0x6a
/*!< Attribute ID for macTxByteCount */
#define macTxByteCount                  0x6b
/*!< Attribute ID for macVersionInfo */
#define macVersionInfo                  0x6c
/*!< Attribute ID for macConfigFlags */
#define macConfigFlags                  0x6d         
/*!< Attribute ID for macIEEEAddress */
#define macIEEEAddress                  0x6f
/* EBR-specific MAC PIB attributes */
/*!< Attribute ID for macEBRPermitJoining*/
#define macEBRPermitJoining                     0x91
/*!< Attribute ID for macEBRFilters*/
#define macEBRFilters                           0x92
/*!< Attribute ID for macEBRLinkQuality*/
#define macEBRLinkQuality                     	0x93
/*!< Attribute ID for macEBRPercentFilter*/
#define macEBRPercentFilter	                    0x94
/*!< Attribute ID for macEBRattributeList*/
#define macEBRattributeList	                    0x95
/*!< Attribute ID for macBeaconAutoRespond*/
#define macBeaconAutoRespond	                0x96

/*!< Attribute ID for macEnhAckWaitDuration*/
#define macEnhAckWaitDuration					0x8C
/*!< Attribute ID for macImplicitBroadcast*/
#define macImplicitBroadcast					0x8D
/*!< Attribute ID for macSimpleAddress*/
#define macSimpleAddress						0x8F

/* EB-specific MAC PIB attributes --- Conflicted ID's */

/*!< Attribute ID for macUseEnhancedBeacon*/
#define macUseEnhancedBeacon      	            0x97 // Conflicted 0x99-- True or False
/*!< Attribute ID for macEBIEList*/
#define macEBIEList	                            0x98 // Conflicted 0x9A-- Datatype is not clear

/*!< Attribute ID for macEBFilteringEnabled*/
#define macEBFilteringEnabled				    0xAB // Conflicted 0x96

/*!< Attribute ID for macEBSN */
#define macEBSN									0xAC // Confilcted 0x98
/*!< Attribute ID for macEnhancedBeaconOrder */
#define macEnhancedBeaconOrder					0xAD // Proprietary ID is given as it is not mention in d5-g spec
/*!< Attribute ID for macMPMIE */
#define macMPMIE								0xAE // Proprietary ID is given as it is not mention in d5-g spec
/*!< Attribute ID for macNBPANEnhancedBeaconOrder */
#define macNBPANEnhancedBeaconOrder				0xAF // Proprietary ID is given as it is not mention in d5-g spec
/*!< Attribute ID for macOffsetTimeSlot */
#define macOffsetTimeSlot						0xB0 // Proprietary ID is given as it is not mention in d5-g spec
/*!< Attribute ID for macFCSType */
#define macFCSType								0xB1 // Proprietary ID is given as it is not mention in d5-g spec
/*! Defines Enhanced acknowledgement */
#define macEnhancedAck							0xB2 // A proprietary attribute to indicate whether the node supports enhanced acks or not
/*! Defines LBTPrevHrTotalSendingDurUS */
#define macLBTPrevHrTotalSendingDurUS			0xAC // A proprietary attribute storing the previous hours total sending duration in us

/* Attribute ID for macRITDataWaitDuration */
#define macRITDataWaitDuration					0x8e 
/* Attribute ID for macRITTxWaitDuration */
#define macRITTxWaitDuration					0x8f
/* Attribute ID for macRITPeriod */
#define macRITPeriod							0x8d

/* Attribute ID for macRITDataReqCmdConfig */
#define macRITDataReqCmdConfig					0x90 // A proprietary attribute to decide the dest addressing fields of the RIT data request commands.

/* MAC PIB attributes for functional organization All the following PIB 
	attributes are of 1 byte */
/*!< Attribute ID for macTSCHcapable */
#define macTSCHcapable	                        0x99
/*!< Attribute ID for macLLcapable */
#define macLLcapable	                        0x9A
/*!< Attribute ID for macDSMEcapable */
#define macDSMEcapable	                        0x9B
/*!< Attribute ID for macLEcapable*/
#define macLEcapable	                        0x9C
/*!< Attribute ID for macRFIDcapable*/
#define macRFIDcapable                          0x9E
/*!< Attribute ID for macTSCHenabled*/
#define macTSCHenabled                          0x9F
/*!< Attribute ID for macLLenabled*/
#define macLLenabled	                        0xA0
/*!< Attribute ID for macDSMEenabled*/
#define macDSMEenabled	                        0xA1
/*!< Attribute ID for macLEenabled*/
#define macLEenabled                            0xA2
/*!< Attribute ID for macRFIDenabled*/
#define macRFIDenabled	                        0xA4
/*!< Attribute ID for macHoppingEnabled*/
#define macHoppingEnabled	                    0xA5
/*!< Attribute ID for macHoppingCapable*/
#define macHoppingCapable	                    0xA6
/*!< Attribute ID for macAMCACapable*/
#define macAMCACapable                          0xA7
/*!< Attribute ID for macAMCAenabled*/
#define macAMCAenabled                          0xA8
/*!< Attribute ID for macMetricsCapable*/
#define macMetricsCapable                       0xA9
/*!< Attribute ID for macMetricsEnabled*/
#define macMetricsEnabled                       0xAA
/*!< Proprietary Attribute ID for storing the PLD IE, SDU IE*/
#define macPLDIEsdu                       		0xAB


#ifdef WISUN_ENET_PROFILE
  /*!< Attribute ID for Pairing id*/
 #define macPairingIEId                               0x88
 #define enetHANSRNIEId                               0x89
 #define enetHANRoutingIEId                           0x8A
 #define macCapabilityNotificationId                  0x8B
#endif
                   

/* MAC constants */
/*!<  Duration of aUnitBackoffPeriod, it includes both aTurnaroundTime and aCCATime */
#define aUnitBackoffPeriod ( aTurnaroundTime + aCCATime )

#ifdef MAC_SECURITY_2003_ENABLED
/*!< Attribute ID for macDefaultSecurity*/
#define macDefaultSecurity               0x72
/*!< Attribute ID for macDefaultSecurityMaterialLength*/
#define macDefaultSecurityMaterialLength 0x73
/*!< Attribute ID for macDefaultSecurityMaterial*/
#define macDefaultSecurityMaterial       0x74
/*!< Attribute ID for macDefaultSecuritySuite*/
#define macDefaultSecuritySuite          0x75
/*!< Attribute ID for macSecurityMode*/
#define macSecurityMode                  0x76
#endif


//#define ENHANCED_ACK_SUPPORT			0x01

/* status flag values */
/*!< Flag to indicate LOW_BATTERY_INDICATOR*/
#define LOW_BATTERY_INDICATOR            0x01

/* config flag values */
/*!< Flag to indicate BEACON_NOTIFY_DISABLE*/
#define BEACON_NOTIFY_DISABLE         0x0001		/* bit set to disable beacon notifies */
/*!< Flag to indicate USE_2011_PRIMITIVES*/		
#define USE_2011_PRIMITIVES           0x0002		/* bit set to use the 802.15.4e/g Primitives */
/*!< Flag to indicate USE_PIB_PANID_FOR_ASSOCIATION*/
#define USE_PIB_PANID_FOR_ASSOCIATION 0x0004         /* bit set to override 0xffff as source PAN id */
/*!< Flag to indicate USE_2006_PRIMITIVES*/
#define USE_2006_PRIMITIVES           0x0008         /* bit set to use the 802.15.4-2006 Primitives */
/*!< Flag to indicate MAC_DISABLED*/
#define MAC_DISABLED                  0x0080         /* bit set disables the MAC */
/*!< Flag to indicate CONFORMANT_MODE*/
#define CONFORMANT_MODE               0x0040         /* bit set if we adhere to strict 15.4 timing (for conformance testing) */
/*!< Flag to indicate DISABLE_PACKET_FILTER*/
#define DISABLE_PACKET_FILTER         0x0020         /* disable CRC and other invalid packet checks - sent to NHL using PD-DATA.indication */
/*!< Flag to indicate STRICT_2003_START_PARSE*/
#define STRICT_2003_START_PARSE       0x0010         /* parse start request to 2003 spec */
/*!< Flag to indicate CONFIG_USE_PPDU_CODING*/
#define CONFIG_USE_PPDU_CODING						0x0200         /* bit set to indicate usage of FEC */

/*!< Flag to indicate use of enhanced ack only when required*/
#define CONFIG_USE_ENHANCED_ACK_WHEN_NEEDED			0x0400         /* bit set to indicate usage of enhanced ack */

/*!< Flag to indicate enhanced ack only*/
#define CONFIG_MODE_SW								0x0800         /* bit set to indicate support of mode switch feature */

/*!< Flag to indicate support of mode switch feature*/
#define CONFIG_USE_ENHANCED_ACK_ALWAYS				0x0100         /* bit set to indicate support of mode switch feature */

/*!< Flag to indicate support of data whitening feature*/
#define CONFIG_DATA_WHITENING							0x1000         /* bit set to indicate support of mode switch feature */

/*!< Flag to indicate support of repeated listen after sending RIT data request command in RIT mode*/
#define CONFIG_RIT_REPEATED_LISTENING_ENABLE			0x2000

/*!< Flag to indicate support of repeated listen after sending RIT data request command in RIT mode*/
#define CONFIG_RIT_DATA_EXCHANGE_ENABLE					0x4000

/* sleep modes */
//#define DISABLE_DEEP_SLEEP            0x1000         /* bit set to disable deep sleep (stop) */
//#define DISABLE_SLEEP                 0x2000         /* bit to disable "sleep mode" */
//#define DISABLE_SNOOZE                0x4000         /* disable snooze mode */

#ifdef ENHANCED_ACK_SUPPORT
/*!< API to indicate DEFAULT_CONFIG_FLAGS for USE_2011_PRIMITIVES*/
#define DEFAULT_CONFIG_FLAGS         (CONFORMANT_MODE|USE_2011_PRIMITIVES|CONFIG_DATA_WHITENING|CONFIG_USE_ENHANCED_ACK_ALWAYS)
#else
#define DEFAULT_CONFIG_FLAGS         (CONFORMANT_MODE|USE_2011_PRIMITIVES|CONFIG_DATA_WHITENING)
#endif


/*! Default Acknowledgement Delay*/
#define DEFAULT_ACK_DELAY            (12*25)  /* 12 symbols in microseconds */

/* FIXME PEB - these are valid whether or not they are used by the MAC */
//#ifdef MAC_CFG_SECURITY_ENABLED
/*!< Attribute ID for macKeyTable*/
#define macKeyTable                     0x71
/*!< Attribute ID for macKeyTableEntries*/
#define macKeyTableEntries              0x72
/*!< Attribute ID for macDeviceTable*/
#define macDeviceTable                  0x73
/*!< Attribute ID for macDeviceTableEntries*/
#define macDeviceTableEntries           0x74
/*!< Attribute ID for macSecurityLevelTable*/
#define macSecurityLevelTable           0x75
/*!< Attribute ID for macSecurityLevelTableEntries*/
#define macSecurityLevelTableEntries    0x76
/*!< Attribute ID for macFrameCounter*/
#define macFrameCounter                 0x77
/*!< Attribute ID for macAutoRequestSecurityLevel*/
#define macAutoRequestSecurityLevel     0x78
/*!< Attribute ID for macAutoRequestKeyIdMode*/
#define macAutoRequestKeyIdMode         0x79
/*!< Attribute ID for macAutoRequestKeySource*/
#define macAutoRequestKeySource         0x7a
/*!< Attribute ID for macAutoRequestKeyIndex*/
#define macAutoRequestKeyIndex          0x7b
/*!< Attribute ID for macDefaultKeySource*/
#define macDefaultKeySource             0x7c
/*!< Attribute ID for macPANCoordExtendedAddress*/
#define macPANCoordExtendedAddress      0x7d
/*!< Attribute ID for macPANCoordShortAddress*/
#define macPANCoordShortAddress         0x7e
/*!< Attribute ID for MAC_SECURITY_PIB_BASE*/
#define MAC_SECURITY_PIB_BASE           macKeyTable
/*!< Attribute ID for secFrameCounterPerk*/
#define secFrameCounterPerk             0x7f   //Suneet ::added for enable secFrameCounterPerk
//#endif /* MAC_CFG_SECURITY_ENABLED */

/* MacDefaultSecuritySuite values */
/*! Default SecuritySuite value for MAC_AES_CCM_128*/
#define MAC_AES_CCM_128                 0x02
/*! Default SecuritySuite value for MAC_AES_CCM_64*/
#define MAC_AES_CCM_64                  0x03

/* Scan type parameters */
/*! Default value for ENERGY_DETECT_SCAN*/
#define ENERGY_DETECT_SCAN				0x00
/*! Default value for ACTIVE_SCAN*/
#define ACTIVE_SCAN                     0x01
/*! Default value for PASSIVE_SCAN*/
#define PASSIVE_SCAN                    0x02
/*! Default value for ORPHAN_SCAN*/
#define ORPHAN_SCAN						0x03

/*! Scan type value for EB Active Scanning*/
#define EB_ACTIVE_SCAN						0x07

/*! Scan type value for MPM EB Passive Scanning*/
#define MPM_EB_PASSIVE_SCAN						0x08

/*Disassociation reasons */
/*! Default value for DISASSOCIATION_COORDINATOR_INITIATED*/
#define DISASSOCIATION_COORDINATOR_INITIATED    0x01
/*! Default value for DISASSOCIATION_SLAVE_INITIATED*/
#define DISASSOCIATION_SLAVE_INITIATED          0x02

#ifdef WISUN_FAN_MAC
#define UNICAST_SCHED_TYPE             0x00          
#define BROADCAST_SCHED_TYPE             0x01
          
#define CHANNEL_PLAN_REG_DOMAIN         0x00
#define CHANNEL_PLAN_APP_SPEC           0x01

#define CF_FIXED_CHANNEL                0x00

#if(FAN_FRQ_HOPPING_FEATURE_ENABLED == 1)          
#define CF_TR51                         0x01
#define CF_DH1                          0x02
#endif
          
#define CF_VENDOR_SPECIFIC              0x03

#define EXC_CHANNEL_CTRL_NIL                    0x00
#define EXC_CHANNEL_CTRL_RANGE                  0x01
#define EXC_CHANNEL_CTRL_BITMASK                0x02
#endif



/*Mac Constants*/


/* MAC constants - described in 802.15.4 specification */
/*! Macro defining value for aNumSuperframeSlots*/
#define aNumSuperframeSlots          16
/*! Macro defining value for aBaseSlotDuration*/
#define aBaseSlotDuration            60
/*! Macro defining value for aBaseSuperframeDuration*/
#define aBaseSuperframeDuration      ( aBaseSlotDuration * aNumSuperframeSlots )
/*! Macro defining value for aMaxResponseWaitTime*/
#define aMaxResponseWaitTime         ( 32 * aBaseSuperframeDuration )
/*! Macro defining value for aMaxFrameOverhead*/
#define aMaxFrameOverhead            25
/*! Macro defining value for aMaxMACFrameSize*/
#define aMaxMACFrameSize             ( aMaxPHYPacketSize - aMaxFrameOverhead )
/*! Macro defining value for aMaxLostBeacons*/
#define aMaxLostBeacons              4
/*! Macro defining value for aMaxFrameRetries*/
#define aMaxFrameRetries             3
/*! Macro defining value for aMaxCommunicationsFailures*/
#define aMaxCommunicationsFailures   3
/*! Macro defining value for aMaxFrameResponseTime*/
#define aMaxFrameResponseTime        1220L
/*! Macro defining value for aGTSDescPersistenceTime*/
#define aGTSDescPersistenceTime      4
/*! Macro defining value for aMaxSIFSFrameSize*/
#define aMaxSIFSFrameSize            18
/*! Macro defining value for aMaxBeaconOverhead*/
#define aMaxBeaconOverhead           75
/*! Macro defining value for aMaxBeaconPayloadLength*/
#define aMaxBeaconPayloadLength      ( aMaxPHYPacketSize-aMaxBeaconOverhead )
/*! Macro defining value for aMinCAPLength*/
#define aMinCAPLength                440
/*! Macro defining value for MAX_EBR_PIB_IDS_LIST_SIZE*/
#define MAX_EBR_PIB_IDS_LIST_SIZE	 6

/**
 *******************************************************************************
 ** \union addr_union
 ** Union for Addressing
 *******************************************************************************
 **/
#if !defined(ADDRESS_T_DEFINED)
typedef union addr_union
{
	ushort   ShortAddress;
	uchar	IEEEAddress[IEEE_ADDRESS_LENGTH];
} address_t;
/*! Defines the address defined */
#define ADDRESS_T_DEFINED 1
#endif

/*! Defines shuffling of the bits */
#ifndef bit
#define bit(x)  ((0x01) << (x) )
#endif
/*! Default bit to indicate CAP_INFO_ALTERNATE_PAN_COORDINATOR*/
#define CAP_INFO_ALTERNATE_PAN_COORDINATOR              bit(0)
/*! Default bit to indicate CAP_INFO_DEVICE_TYPE*/
#define CAP_INFO_DEVICE_TYPE                            bit(1)
/*! Default bit to indicate CAP_INFO_POWER_SOURCE*/
#define CAP_INFO_POWER_SOURCE                           bit(2)
/*! Default bit to indicate CAP_INFO_RECEIVER_ON_WHEN_IDLE*/
#define CAP_INFO_RECEIVER_ON_WHEN_IDLE                  bit(3)
/*! Default bit to indicate CAP_INFO_RESERVED1*/
#define CAP_INFO_RESERVED1                              bit(4)
/*! Default bit to indicate CAP_INFO_RESERVED2*/
#define CAP_INFO_RESERVED2                              bit(5)
/*! Default bit to indicate CAP_INFO_SECURITY_CAPABILITY*/
#define CAP_INFO_SECURITY_CAPABILITY                    bit(6)
/*! Default bit to indicate CAP_INFO_ALLOCATE_ADDRESS*/
#define CAP_INFO_ALLOCATE_ADDRESS                       bit(7)

/* bit masks for the superframe */
/*! Default bit to indicate SF0_BEACON_ORDER_MASK*/
#define SF0_BEACON_ORDER_MASK           0x0f
/*! Default bit to indicate SF0_SUPERFRAME_ORDER_MASK*/
#define SF0_SUPERFRAME_ORDER_MASK       0xf0
/*! Default bit to indicate SF1_ASSOCIATION_PERMIT*/
#define SF1_ASSOCIATION_PERMIT          0x80
/*! Default bit to indicate SF1_PAN_COORDINATOR*/
#define SF1_PAN_COORDINATOR             0x40
/*! Default bit to indicate SF1_FINAL_CAP_SLOT*/
#define SF1_FINAL_CAP_SLOT              0x0f
/*! Default bit to indicate SF1_BATTERY_LIFE_EXTENSION*/
#define SF1_BATTERY_LIFE_EXTENSION      0x10

/* Bit manipulation struct */
/**
 *******************************************************************************
 ** \struct sframe_bits_struct_h8
 ** Structure to indicate number of bits for superframe parameters for TARGET_H8
 *******************************************************************************
 **/
#if defined( TARGET_H8 )
typedef struct sframe_bits_struct
{
	uchar SuperframeOrder       : 4;
	uchar BeaconOrder           : 4;
	uchar AssociationPermit     : 1;
	uchar PANCoordinator        : 1;
	uchar Reserved              : 1;
	uchar BatteryLifeExtension  : 1;
	uchar FinalCAPSlot          : 4;
} sframe_bits_t;

#else
/**
 *******************************************************************************
 ** \struct sframe_bits_struct
 ** Structure to indicate number of bits for superframe parameters
 *******************************************************************************
 **/
typedef struct sframe_bits_struct
{
	uchar BeaconOrder           : 4;
	uchar SuperframeOrder       : 4;
	uchar FinalCAPSlot          : 4;
	uchar BatteryLifeExtension  : 1;
	uchar Reserved              : 1;
	uchar PANCoordinator        : 1;
	uchar AssociationPermit     : 1;
} sframe_bits_t;
#endif

/**
 *******************************************************************************
 ** \union sframe_struct
 ** Union for superframe structure
 *******************************************************************************
 **/
typedef union sframe_struct
{
	sframe_bits_t     bits;
	ushort            num;
} sframe_t;

/**
 *******************************************************************************
 ** \struct pan_descriptor_struct
 ** Structure to indicate parameters of pandescriptor
 *******************************************************************************
 **/
typedef struct  pan_descriptor_struct
{
	uchar           AddrMode;
	ushort          PANId;
	address_t       Address;
	uchar           LogicalChannel;
	sframe_t        SuperframeSpec;
	uchar           GtsPermit;
	uchar           LinkQuality;
	uchar           TimeStamp[3];
	uchar           Security;
} pan_descriptor_t;


/**
 *******************************************************************************
 ** \struct Rx_Frame_stats
 ** Structure used to track the number of packet received and the CRC failuer
 ** 
 *******************************************************************************
 **/
typedef struct Rx_Frame_stats
{
	uint16_t no_of_pkts_rcv;	
	uint32_t no_of_crc_failures;
}rx_Frame_stats_t;

/**
 *******************************************************************************
 ** \struct msg_struct
 ** Structure used for messages between layers e.g. nwk to mac
 ** NB-length must be declared before data,as the SPI hardware depends on this!
 *******************************************************************************
 **/
typedef struct msg_struct
{
	struct msg_struct *next;   /**< make this queueable    */
	uint16_t data_length;      /**< length of this message */
	uchar data[1];  		   /**< serialised primitive, then parameters */
} msg_t;

/*! Default value for PD_ADDRMODE*/
#define PD_ADDRMODE     0
/*! Default value for PD_PANIDLO*/
#define PD_PANIDLO      1
/*! Default value for PD_PANIDHI*/
#define PD_PANIDHI      2
/*! Default value for SPD_ADDRLO*/
#define SPD_ADDRLO      3
/*! Default value for SPD_ADDRHI*/
#define SPD_ADDRHI      4
/*! Default value for SPD_CHANNEL*/
#define SPD_CHANNEL     5
/*! Default value for SPD_SFRAMELO*/
#define SPD_SFRAMELO    6
/*! Default value for SPD_SFRAMEHI*/
#define SPD_SFRAMEHI    7
/*! Default value for SPD_GTSPERMIT*/
#define SPD_GTSPERMIT   8
/*! Default value for SPD_LINKQ*/
#define SPD_LINKQ       9
/*! Default value for SPD_TIMESTAMPLO*/
#define SPD_TIMESTAMPLO 10
/*! Default value for SPD_TIMESTAMPMD*/
#define SPD_TIMESTAMPMD 11
/*! Default value for SPD_TIMESTAMPHI*/
#define SPD_TIMESTAMPHI 12
/*! Default value for SPD_SEC*/
#define SPD_SEC         13
/*! Default value for IPD_ADDR*/
#define IPD_ADDR        3
/*! Default value for IPD_CHANNEL*/
#define IPD_CHANNEL     (IPD_ADDR+8)
/*! Default value for IPD_SFRAMELO*/
#define IPD_SFRAMELO    (IPD_CHANNEL+1)
/*! Default value for IPD_SFRAMEHI*/
#define IPD_SFRAMEHI    (SPD_SFRAMELO+1)
/*! Default value for IPD_GTSPERMIT*/
#define IPD_GTSPERMIT   (SPD_SFRAMEHI+1)
/*! Default value for IPD_LINKQ*/
#define IPD_LINKQ       (SPD_GTSPERMIT+1)
/*! Default value for IPD_TIMESTAMPLO*/
#define IPD_TIMESTAMPLO (SPD_LINKQ+1)
/*! Default value for IPD_TIMESTAMPMD*/
#define IPD_TIMESTAMPMD (SPD_TIMESTAMPLO+1)
/*! Default value for IPD_TIMESTAMPHI*/
#define IPD_TIMESTAMPHI (SPD_TIMESTAMPMD+1)
/*! Default value for IPD_SEC*/
#define IPD_SEC         (SPD_TIMESTAMPHI+1)

/*! Default size of PAN_DESCRIPTOR_NO_ADDRESS_SIZE*/
#define PAN_DESCRIPTOR_NO_ADDRESS_SIZE (1+1+2+1+1+3+1) /* address mode, channel, superframe spec(2), GTS Permit, link quality, time stamp, security */
/*! Default size of PAN_DESCRIPTOR_SHORT_ADDRESS_SIZE*/
#define PAN_DESCRIPTOR_SHORT_ADDRESS_SIZE (PAN_DESCRIPTOR_NO_ADDRESS_SIZE+4) /* pan id + short address */
/*! Default size of PAN_DESCRIPTOR_IEEE_ADDRESS_SIZE*/
#define PAN_DESCRIPTOR_IEEE_ADDRESS_SIZE (PAN_DESCRIPTOR_NO_ADDRESS_SIZE+10) /* pan id + ieee address */
/*! Default size of SHORT_PAN_DESCRIPTOR_SIZE*/
#define SHORT_PAN_DESCRIPTOR_SIZE (PAN_DESCRIPTOR_NO_ADDRESS_SIZE+4) /* pan id + short address */
/*! Default size of  IEEE_PAN_DESCRIPTOR_SIZE*/
#define IEEE_PAN_DESCRIPTOR_SIZE (PAN_DESCRIPTOR_NO_ADDRESS_SIZE+10) /* pan id + ieee address */

/* The embedded MAC code use masks to ensure portability */
/*! Default bitmask of GTS_LENGTH_MASK to ensure portability*/
#define GTS_LENGTH_MASK			0x0f		/* 4 LSBs */
/*! Default bitmask of GTS_DIRECTION_MASK to ensure portability*/
#define GTS_DIRECTION_MASK		bit(4)		/* direction is bit 4 */
/*! Default bit for GTS_DIR_RX_ONLY to set if GTS is to be received*/
#define GTS_DIR_RX_ONLY			bit(4)		/* set if receive GTS */
/*! Default bit for GTS_DIR_TX_ONLY to clear if GTS is to be transmitted*/
#define GTS_DIR_TX_ONLY			0			/* clear if transmit GTS */

/*! Default bitmask of GTS_TYPE_MASK to allocate/deallocate*/
#define GTS_TYPE_MASK			bit(5)		/* bit 5 is the allocate / deallocate mask */
/*! Default bitmask of GTS_TYPE_ALLOCATE to set for allocate*/
#define GTS_TYPE_ALLOCATE		bit(5)		/* set for allocate */
/*! Default bitmask of GTS_TYPE_DEALLOCATE to clear for deallocate*/
#define GTS_TYPE_DEALLOCATE             0		/* clear for deallocate */

/* values for Power States ML7065v3 */
/*! Default value for MAC_IDLE_LOW in 2MHz Operation*/
#define MAC_IDLE_LOW        0                 // 2MHz Operation
/*! Default value for MAC_SUSPEND */ 
#define MAC_SUSPEND         1                 // Suspend (SCEN Blip to awake)
/*! Default value for MAC_SLEEP */
#define MAC_SLEEP           2                 // Sleep (SRESET Blip to awake)
/*! Default value for MAC_IDLE_HIGH in 16MHz Operation*/
#define MAC_IDLE_HIGH       3                 // 16MHz Operation

#if(FAN_EDFE_FEATURE_ENABLED == 1)
#define macEDFEframeEnabled                     0x80
#endif

/*@}*/

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

extern void mac_set_state( uchar state );

/*
** ============================================================================
** Public Function Prototypes
** ============================================================================
*/
 
/** \defgroup mac_functions MAC Functions
 ** \ingroup mac
 */

/** \defgroup mac_mlme_functions MAC MLME Interface
 * \ingroup mac_functions
 */

/*@{*/

/*APIs used by the MAC Layer for sending conf/ind to NHLE */

/**
 *******************************************************************************
 ** \brief Implemented by NHL for MLME primitives
 ** \param *msg_t - pointer from where the payload starts 
 ** \retval - None
 ******************************************************************************/
extern void send_mlme_2_nhle(msg_t*);//call back implemented at NHLE

/**
 *******************************************************************************
 ** \brief This function sends data from NHL to MAC_MLME
 ** \param *msg_t - pointer to the payload
 ** \retval - None
 ******************************************************************************/
void send_nhle_2_mlme( msg_t* );

/*@}*/

/** \defgroup mac_mcps_functions MAC MCPS Interface
 * \ingroup mac_functions
 */

/*@{*/

/**
 *******************************************************************************
 ** \brief Implemented by NHL for MCPS primitives
 ** \param *msg_t - pointer from where the payload starts 
 ** \retval - None
 ******************************************************************************/
extern void send_mcps_2_nhle(msg_t*);//call back implemented at NHLE

/**
 *******************************************************************************
 ** \brief This function sends data from NHL to MAC_MCPS
 ** \param *msg_t - pointer to the payload
 ** \retval - None
 ******************************************************************************/
void send_nhle_2_mcps( msg_t* );

/*@}*/

/** \defgroup mac_required_functions MAC Required Functions
 * \ingroup mac_functions
 */

/*@{*/

/*APIs used by the NHLE for accessing MAC service*/

/**
 *******************************************************************************
 ** \brief This function allocates a buffer from MAC to NHL
 ** \param length - length of the buffer to allocate
 ** \retval - pointer to the allocated buffer
 ******************************************************************************/
extern msg_t* allocate_mac_2_nhle_msg( uint16_t length );

/**
 *******************************************************************************
 ** \brief This function frees buffer allocated for MAC to NHL
 ** \param *msgp - pointer to the buffer to be freed
 ** \retval - None
 ******************************************************************************/
extern void free_mac_2_nhle_msg( msg_t * msgp );

/**
 *******************************************************************************
 ** \brief This function allocates a buffer from NHL to MAC
 ** \param length - length of the buffer to allocate
 ** \retval - pointer to the allocated buffer
 ******************************************************************************/
msg_t* allocate_nhle_2_mac_msg( uint16_t length );

/**
 *******************************************************************************
 ** \brief This function frees buffer allocated for NHL to MAC
 ** \param *msg_t - pointer to the buffer to be freed
 ** \retval - None
 ******************************************************************************/
void free_nhle_2_mac_msg(msg_t*);

/**
 *******************************************************************************
 ** \brief This function processes MCPS messages coming into the MAC
 ** \param - None
 ** \retval - None
 ******************************************************************************/
void process_mcps_msgs( void );

/**
 *******************************************************************************
 ** \brief This function processes MLME messages coming into the MAC
 ** \param - None
 ** \retval - None
 ******************************************************************************/
void process_mlme_msgs( void );

/**
 *******************************************************************************
 ** \brief This function used to intialise the PIBs
 ** \param init_pib - intialises all the pib's to default values
 ** \retval - None
 ******************************************************************************/
void mac_initialise(uchar init_pib);

/*@}*/

/*@}*/

#ifdef __cplusplus
}
#endif
#endif /* MAC_H */


