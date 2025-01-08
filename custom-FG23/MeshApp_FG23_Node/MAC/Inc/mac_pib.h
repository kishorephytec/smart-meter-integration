/** \file mac_pib.h
 *******************************************************************************
 ** \brief Provides details about the MAC PIBs
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

#ifndef MAC_PIB_H
#define MAC_PIB_H

#ifdef __cplusplus
extern "C" {
#endif

/*
** ============================================================================
** Public Macro definitions
** ============================================================================
*/


/*
** ============================================================================
** Public Structures, Unions & enums Type Definitions
** ============================================================================
*/
   

/* MAC PIB parameters - described in 802.15.4 specification */
/**
 *******************************************************************************
 ** \stuct mac_pib_t
 ** Structure to store all the MAC PIBs
 *******************************************************************************
 **/
typedef struct mac_pib_struct
{
    ulong BeaconTxTime;
    ushort CoordShortAddress;
    ushort PANId;
    ushort ShortAddress;
    ushort TransactionPersistenceTime;
    uchar CoordExtendedAddress[8];
    uint16_t AckWaitDuration;
    uchar AssociationPermit;
    uchar AutoRequest;
    uchar BattLifeExt;
    uchar BattLifeExtPeriods;
    ushort BeaconPayloadLength;
    uchar BeaconOrder;
    uchar BSN;
    uchar DSN;
    uchar GTSPermit;
    uchar MaxCSMABackoffs;
    uchar MinBE;
    uchar PromiscuousMode;
    uchar RxOnWhenIdle;
    uchar SuperframeOrder;
    uchar mac_security_enabled;
    ushort ConfigFlags;         /* flags for mac configuration enhancements */
    ushort ack_delay;           /* delay time in microseconds before ack */
    ushort MaxFrameTotalWaitTime;
    uchar AssociatePANCoord;
    uchar MaxBE;
    uchar TimestampSupported;
    uchar MaxFrameRetries;
    uchar ResponseWaitTime;
    ushort SyncSymbolOffset;
    ulong LBTActiveDuration;
    ulong LBTSuspendedDuration;
    ushort LBTSamplingDuration;
    ushort LBTSamplingPeriod;
    ushort LBTCCADuration;
    uchar LBTFlags;
	uint32_t LBTPrevHrTotalSendingDurUS;
    /* EBR-specific MAC PIB attributes */
    uchar EBRPermitJoining;					/* 1 byte-- True or False */
    uchar EBRFilters;						/*b0-indicates if the Link Quality Filter Enable bit, b1:Percent Filter Enable bit*/
    uchar EBRLinkQuality;					/* 1 byte-- 0x00 to 0x FF */
    uchar EBRPercentFilter;					/* 1 byte-- 0x00 to 0x64 */
    uchar EBRattributeList[ MAX_EBR_PIB_IDS_LIST_SIZE ]; /* 1 byte-- 0x00 to 0xFF */
    uchar BeaconAutoRespond;				/* 1 byte-- True or False */
	ushort EnhAckWaitDuration; 
	uchar ImplicitBroadcast;
	uchar SimpleAddress;
	uchar UseEnhancedBeacon; 
	uchar EBIEList; 
#if( CGF_MAC_FUNCTIONAL_ORGANIZATION==1)
    /* MAC PIB attributes for functional organization */
    uchar TSCHcapable;
    uchar LLcapable;
    uchar DSMEcapable;
#endif    
    uchar LEcapable;
#if(CGF_MAC_FUNCTIONAL_ORGANIZATION==1)     
    uchar RFIDcapable;
    uchar HoppingCapable;
    uchar AMCACapable;
    uchar MetricsCapable;
    uchar TSCHenabled;
    uchar LLenabled;
    uchar DSMEenabled;
#endif    
    uchar LEenabled;
#if(CGF_MAC_FUNCTIONAL_ORGANIZATION==1)    
    uchar RFIDenabled;
    uchar HoppingEnabled;
    uchar AMCAenabled;
    uchar MetricsEnabled; 
#endif    
	uchar EBFilteringEnabled;
	uchar EBSN;
	uchar EnhancedBeaconOrder;
	uchar MPMIE;
	ushort NBPANEnhancedBeaconOrder;
	uchar OffsetTimeSlot;
	uchar FCSType;
#ifdef WISUN_ENET_PROFILE       
        uchar PairingIeIdContent[8];
        uchar Capability_Notification_Val;
        uchar SrnIeIdContent[11]; // will need to change base on max number of Routing device supported in network
        uchar RoutingIeIdContent[27]; // Previous was RoutingIeIdContent[19]
#endif
	uchar EnhancedAck; 
	uchar RITDataWaitDuration;
	uint32_t RITTxWaitDuration;
	uint32_t RITPeriod;
	uint8_t RITDataReqCmdConfig;
	uchar BeaconPayload[aMaxBeaconPayloadLength]; /* must be last (due to initialisation using default_pib */
        
} mac_pib_t;

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

/**
 *******************************************************************************
 ** \brief Function to initialise all the MAC PIBs
 ** \param - None
 ** \retval - None
 ******************************************************************************/
void mac_pib_init( void );

/**
 *******************************************************************************
 ** \brief Function to initialise all the MAC Security PIBs
 ** \param - None
 ** \retval - None
 ******************************************************************************/
void mac_pib_init_security( void );

/**
 *******************************************************************************
 ** \brief Function to initialise extended address of the node to default value 
 ** \param - None
 ** \retval - None
 ******************************************************************************/
//void mac_pib_init_ieee( void );

/**
 *******************************************************************************
 ** \brief Function to pack MAC PIB and aExtendedAddress for serialization.
 ** \param  *s - 
 ** \param *dst - pointer to the destination buffer to pack with the aExtendedAddress
 ** \retval - sizeof the pack
 ******************************************************************************/
//ushort mac_pib_pack( void *s, uchar *dst );

/**
 *******************************************************************************
 ** \brief Function to initialise all the MAC PIBs
 ** \param *s -
 ** \param *src - pointer to the source buffer to unpack the aExtendedAddress 
 ** \retval - None
 ******************************************************************************/
//void mac_pib_unpack( void *s, uchar *src );

#ifdef __cplusplus
}
#endif
#endif /* MAC_PIB_H */

