/** \file mac_pib.c
 *******************************************************************************
 ** \brief Provided MAC Layer PIBs definitions
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
#include "list_latest.h"
#include "hw_tmr.h"
#include "sw_timer.h"
#include "timer_service.h"
#include "phy.h"
#include "mac_config.h"
#include "mac.h"
//#include "phy_timing_pib.h"
#include "mac_defs_sec.h"
#include "ie_element_info.h"
#include "mac_defs.h"

#include "mac_mem.h"
#include "sm.h"

#if(CFG_MAC_SFTSM_ENABLED == 1)  
#include "sftsm.h"
#include "pandesc.h"
#include "lpmsm.h"
#endif

#if(CFG_MAC_MPMSM_ENABLED == 1)
#include "mpmsm.h"
#endif

#include "ccasm.h"
#include "trxsm.h"

#if(CFG_MAC_LBTSM_ENABLED == 1)    
#include "lbtsm.h"
#else
#include "ccasm.h"
#endif

#include "mac_uplink.h"
#include "mac_pib.h"

#if ( ( MAC_CFG_LE_RIT_CAPABILITY == 1 ) || ( MAC_CFG_LE_CSL_CAPABILITY == 1 ) )
#include "mac_le.h"
#endif

#ifdef MAC_CFG_SECURITY_ENABLED
#include "mac_security.h"
#endif

/*
** =============================================================================
** Private Macro definitions
** =============================================================================
*/
#ifdef MAC_CFG_SECURITY_ENABLED
	#define MAC_DEFAULT_KEY_SOURCE_VALUE {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF}
#endif

/*
** =============================================================================
** Private Structures, Unions & enums Type Definitions
** =============================================================================
**/

typedef struct mac_attribute_table_struct
{
	uchar id;
	uint16_t length;
	uchar *value;
	uchar min_value;
	uint32_t max_value;
} mac_attribute_table_t;

/*
** ============================================================================
** External Variable Declarations
** ============================================================================
*/

extern trxsm_t trxsm;
#if(CFG_MAC_LBTSM_ENABLED == 1)    
extern lbtsm_t lbtsm;
#elseif(CFG_MAC_CCA_ENABLED == 1)
extern ccasm_t ccasm;
#endif
//extern const phy_timing_pib_t phy_timing_pib_default;

#ifdef MAC_CFG_SECURITY_ENABLED
	extern mac_security_data_t     mac_security_data;
#endif

extern const char mac_version_info[];
/*
** ============================================================================
** Private Variable Definitions
** ============================================================================
*/
//static const uchar def_aExtendedAddress[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0xAA, 0xBB, 0xCC };

static const mac_pib_t default_pib =
{
	0,			/* ulong BeaconTxTime*/
	DEFAULT_SHORT_ADDRESS,  /* ushort CoordShortAddress */ 
	DEFAULT_PAN_ID,		/* ushort PANId*/
	DEFAULT_SHORT_ADDRESS,	/* ushort ShortAddress*/
	0x01f4, 		/* ushort TransactionPersistenceTime*/
	{0,0,0,0,0,0,0,0},	/* uchar CoordExtendedAddress[8]*/
	0x0104,			/* uint16_t AckWaitDuration*/
	0,			/* uchar AssociationPermit*/
	1,			/* uchar AutoRequest*/
	0,			/* uchar BattLifeExt*/
	6,			/* uchar BattLifeExtPeriods*/
	0,			/* ushort BeaconPayloadLength*/
	15,			/* uchar BeaconOrder*/
	0,			/* uchar BSN*/
	0, 			/* uchar DSN*/
	1,			/* uchar GTSPermit*/
	4,			/* uchar MaxCSMABackoffs*/
	0,			/* uchar MinBE*/
	0,			/* uchar PromiscuousMode*/
	0,			/* uchar RxOnWhenIdle*/
	15,			/* uchar SuperframeOrder*/
	0,			/* uchar mac_security_enabled*/    
	DEFAULT_CONFIG_FLAGS,	/* ushort ConfigFlags*/
	DEFAULT_ACK_DELAY,	/* ushort ack_delay*/
	21460,			/* ushort MaxFrameTotalWaitTime*/
	0,			/* uchar AssociatePANCoord*/
	3,			/* uchar MaxBE*/
	0,			/* uchar TimestampSupported*/
	5,			/* uchar MaxFrameRetries*/ 	/* Debdeep :: MAC retry is strictly 5 */
	32,			/* uchar ResponseWaitTime*/
	0,			/* ushort SyncSymbolOffset*/
	50000,       /* ulong LBTActiveDuration     == 1000 ms*/
	50000/10,    /* ulong LBTSuspendedDuration  == 100 ms       */	
	20,                     /* ushort LBTSamplingDuration                  */
	50,			/* ushort LBTSamplingPeriod*/
	50000/100,   /* ushort LBTCCADuration       == 10 ms        */
	7,                      /* uchar LBTFlags LBTSM_FLAG_USE_BACKOFF|LBTSM_FLAG_KEEP_RX_ON|LBTSM_FLAG_ENABLE_TOT_TX_DUR_THRESHOLD*/
	0,			/*uint32_t LBTPrevHrTotalSendingDurUS */
	0,			/* uchar EBRPermitJoining*/ /* 1 byte-- True or False */
	0,			/* uchar EBRFilters;*/		/*b0-indicates if the Link Quality Filter Enable bit, b1:Percent Filter Enable bit*/
	0,			/* uchar EBRLinkQuality */	/* 1 byte-- 0x00 to 0x FF */
	0,			/* uchar EBRPercentFilter*/ /* 1 byte-- 0x00 to 0x64 */
	{0x00,0xFF,0xFF,0xFF,0xFF,0xFF},			/* 1 byte-- 0x00 to 0xFF */
	1,			/* uchar BeaconAutoRespond*/
	0x9C40,			/* ushort EnhAckWaitDuration*/ /*by default kept as 4000 us*/
	0,			/* uchar ImplicitBroadcast*/
	0,			/* uchar SimpleAddress*/
	0,			/* uchar UseEnhancedBeacon*/
	0,			/* uchar EBIEList*/
#if(CGF_MAC_FUNCTIONAL_ORGANIZATION==1)         
	0,			/* uchar TSCHcapable*/
	0,			/* uchar LLcapable*/
	0,			/* uchar DSMEcapable*/
#endif        
#if ( ( MAC_CFG_LE_RIT_CAPABILITY == 1 ) || ( MAC_CFG_LE_CSL_CAPABILITY == 1 ) )
	1,			/* uchar LEcapable is TRUE*/
#else
	0,			/* uchar LEcapable is FALSE*/
#endif
#if(CGF_MAC_FUNCTIONAL_ORGANIZATION==1)
	0,			/* uchar RFIDcapable*/
	0,			/* uchar HoppingCapable*/
	0,			/* uchar AMCACapable*/
	0,			/* uchar MetricsCapable*/
	0,			/* uchar TSCHenabled*/
	0,			/* uchar LLenabled*/
	0,			/* uchar DSMEenabled*/
#endif        
	0,			/* uchar LEenabled*/
#if(CGF_MAC_FUNCTIONAL_ORGANIZATION==1)	
	0,			/* uchar RFIDenabled*/
	0,			/* uchar HoppingEnabled*/
	0,			/* uchar AMCAenabled*/
	0,			/* uchar MetricsEnabled*/ 
#endif        
	0,			/* uchar EBFilteringEnabled*/
	0,			/* uchar EBSN*/
	0,			/* uchar EnhancedBeaconOrder*/
	1,			/* uchar MPMIE*/
	0x4000,		/* ushort NBPANEnhancedBeaconOrder*/
	0,			/* uchar OffsetTimeSlot*/
/* Raka ::: The size of the FCS in the passed PPDU. True [1] for 16-bit CRC, false [0] for 32-bit CRC 
            Changing 1 to 0 for Enabling 4 Byte FCS.        
*/         
	0,			/* uchar FCSType*/ 
//	1,			/* uchar FCSType*/
        
#ifdef WISUN_ENET_PROFILE        
        //{0x43, 0x43, 0x44, 0x44, 0x45, 0x45, 0x46, 0x46}, // Pairing_ie contents "CCDDEEFF" string
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // Pairing_ie contents "CCDDEEFF" string
        0, //for Capability Notification  0x79
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
#endif        
	0,			/* uchar EnhancedAck*/
#if ( ( MAC_CFG_LE_RIT_CAPABILITY == 1 ) || ( MAC_CFG_LE_CSL_CAPABILITY == 1 ) )
	0x10,0x00000200,0x00000000,1,//macRITDataWaitDuration,macRITTxWaitDuration,macRITPeriod,macRITDataReqCmdConfig
#endif
	{0},		/* uchar BeaconPayload[aMaxBeaconPayloadLength]; */ /* must be last (due to initialisation using default_pib */
};

/*
** =============================================================================
** Public Variables Definitions
** =============================================================================
**/

//uchar* p_pld_ie_sdu = NULL;
mac_pib_t mac_pib;          /* the parameter information block for the MAC */
uchar aExtendedAddress[8];  /* our own extended address */

const mac_attribute_table_t mac_attribute_table[] =
    {
        { macAckWaitDuration,           2, (uchar*) &mac_pib.AckWaitDuration, 0x54, 0x0104 },
        { macAssociationPermit,         1, (uchar*) &mac_pib.AssociationPermit, 0, 1 },
        { macAutoRequest,               1, (uchar*) &mac_pib.AutoRequest, 0, 1 },
        { macBattLifeExt,               1, (uchar*) &mac_pib.BattLifeExt, 0, 1 },
        { macBattLifeExtPeriods,        1, (uchar*) &mac_pib.BattLifeExtPeriods, 6, 41 },
        { macBeaconPayload,             1, (uchar*) &mac_pib.BeaconPayload, 0, aMaxBeaconPayloadLength },
        { macBeaconPayloadLength,       2, (uchar*) &mac_pib.BeaconPayloadLength, 0, aMaxBeaconPayloadLength },
        { macBeaconOrder,               1, (uchar*) &mac_pib.BeaconOrder, 0, 15 },
        { macBeaconTxTime,              3, (uchar*) &mac_pib.BeaconTxTime, 0, 0 },
        { macBSN,                       1, (uchar*) &mac_pib.BSN, 0, 255 },
        { macCoordExtendedAddress,      8, (uchar*) &mac_pib.CoordExtendedAddress, 0, 0 },
        { macCoordShortAddress,         2, (uchar*) &mac_pib.CoordShortAddress, 0, 0 },
        { macDSN,                       1, (uchar*) &mac_pib.DSN, 0, 255 },
        { macGTSPermit,                 1, (uchar*) &mac_pib.GTSPermit, 0, 1 },
        { macMaxCSMABackoffs,           1, (uchar*) &mac_pib.MaxCSMABackoffs, 0, 5 },
        { macMinBE,                     1, (uchar*) &mac_pib.MinBE, 0, 8 },
        { macPANId,                     2, (uchar*) &mac_pib.PANId, 0, 0xFFFE },
        { macPromiscuousMode,           1, (uchar*) &mac_pib.PromiscuousMode, 0, 1 },
        { macRxOnWhenIdle,              1, (uchar*) &mac_pib.RxOnWhenIdle, 0 , 1 },
        { macShortAddress,              2, (uchar*) &mac_pib.ShortAddress, 0, 0xFFFE },
        { macSuperframeOrder,           1, (uchar*) &mac_pib.SuperframeOrder, 0, 15 },
        { macConfigFlags,               2, (uchar*) &mac_pib.ConfigFlags, 0, 0xff },
#if(CFG_MAC_VERSION_ENABLED == 1)        
        { macVersionInfo,               16,(uchar*) mac_version_info, 0, 0 },
#endif        
        { macTransactionPersistenceTime,2, (uchar*) &mac_pib.TransactionPersistenceTime, 0, 0xFFFF },
        { macAssociatedPANCoord,        1, (uchar*) &mac_pib.AssociatePANCoord, 0, 1 },
        { macMaxBE,                     1, (uchar*) &mac_pib.MaxBE, 3, 8 },
        { macMaxFrameTotalWaitTime,     2, (uchar*) &mac_pib.MaxFrameTotalWaitTime, 0, 0xffff }, /* FIXME - max value */
        { macMaxFrameRetries,           1, (uchar*) &mac_pib.MaxFrameRetries, 0, 7 },
        { macResponseWaitTime,          1, (uchar*) &mac_pib.ResponseWaitTime, 2, 64 }, /* FIXME - max value */
        { macSyncSymbolOffset,          2, (uchar*) &mac_pib.SyncSymbolOffset, 0, 0xffff }, /* READ ONLY */
        { macTimestampSupported,        1, (uchar*) &mac_pib.TimestampSupported, 0, 1 },
        { macIEEEAddress,               8, (uchar*) aExtendedAddress, 0, 0 },
        { macTxByteCount,               2, (uchar*) &mac_data.TxByteCount, 0, 0/*&mac_pib.IEEEAddress*/ },
        { macAckTime,                   2, (uchar*) &mac_pib.ack_delay, 0, 0xff },
        { macSecurityEnabled,           1, (uchar*) &mac_pib.mac_security_enabled, 0, 1 }, /* READ ONLY */
        
     
#ifdef MAC_CFG_SECURITY_ENABLED
        { macKeyTable,                  1, (uchar*) &mac_security_data.pib.mac_key_table, 0, 0 },
        { macKeyTableEntries,           1, (uchar*) &mac_security_data.pib.mac_key_table_entries, 0, 255 },
        { macDeviceTable,               1, (uchar*) &mac_security_data.pib.mac_device_table, 0, 0 },
        { macDeviceTableEntries,        1, (uchar*) &mac_security_data.pib.mac_device_table_entries, 0, 255 },
        { macSecurityLevelTable,        1, (uchar*) &mac_security_data.pib.mac_security_level_table, 0, 0 },
        { macSecurityLevelTableEntries, 1, (uchar*) &mac_security_data.pib.mac_security_level_table_entries, 0, 255 },
        { macFrameCounter,              4, (uchar*) &mac_security_data.pib.mac_frame_counter, 0, 0 },
        { macAutoRequestSecurityLevel,  1, (uchar*) &mac_security_data.pib.auto_request_security_level, 0, 7 },
        { macAutoRequestKeyIdMode,      1, (uchar*) &mac_security_data.pib.auto_request_keyid_mode, 0, 3 },
        { macAutoRequestKeySource,      8, (uchar*) &mac_security_data.pib.auto_request_key_source, 0, 0 },
        { macAutoRequestKeyIndex,       1, (uchar*) &mac_security_data.pib.auto_request_key_index, 0, 255 },
        { macDefaultKeySource,          8, (uchar*) &mac_security_data.pib.mac_default_key_source, 0, 0 },
        { macPANCoordExtendedAddress,   8, (uchar*) &mac_security_data.pib.mac_PAN_coord_extended_address, 0, 0 },
        { macPANCoordShortAddress,      2, (uchar*) &mac_security_data.pib.mac_PAN_coord_short_address, 0, 0xFFFE },
        { secFrameCounterPerk,          1, (uchar*) &mac_security_data.pib.mac_sec_frame_counter_perkey, 0, 0 },
#endif

        { macLBTActiveDuration,         4, (uchar*) &mac_pib.LBTActiveDuration, 0, 0 },
        { macLBTSuspendedDuration,      4, (uchar*) &mac_pib.LBTSuspendedDuration, 0, 0 },
        { macLBTSamplingDuration,       2, (uchar*) &mac_pib.LBTSamplingDuration, 0, 0 },
        { macLBTSamplingPeriod,         2, (uchar*) &mac_pib.LBTSamplingPeriod, 0, 0 },
        { macLBTCCADuration,            2, (uchar*) &mac_pib.LBTCCADuration, 0, 0 },
        { macLBTFlags,                  1, (uchar*) &mac_pib.LBTFlags, 0, 0 },
		{ macLBTPrevHrTotalSendingDurUS,4, (uchar*) &mac_pib.LBTPrevHrTotalSendingDurUS, 0, 0 },
        { macEBRPermitJoining, 			1, (uchar*) &mac_pib.EBRPermitJoining, 0, 1 },
        { macEBRFilters, 			1, (uchar*) &mac_pib.EBRFilters, 0, 3},
        { macEBRLinkQuality, 			1, (uchar*) &mac_pib.EBRLinkQuality, 0, 0xFF },
        { macEBRPercentFilter,			1, (uchar*) &mac_pib.EBRPercentFilter, 0, 0x64 },
        { macEBRattributeList,			1, (uchar*) &mac_pib.EBRattributeList, 0, 0 },
        { macBeaconAutoRespond, 		1, (uchar*) &mac_pib.BeaconAutoRespond, 0, 1 },
        { macUseEnhancedBeacon,			1, (uchar*) &mac_pib.UseEnhancedBeacon, 0, 1 },
#if(CGF_MAC_FUNCTIONAL_ORGANIZATION==1)          
        { macTSCHcapable, 			1, (uchar*) &mac_pib.TSCHcapable, 0, 1 },
        { macLLcapable, 			1, (uchar*) &mac_pib.LLcapable, 0, 1 },
        { macDSMEcapable, 			1, (uchar*) &mac_pib.DSMEcapable, 0, 1 },
#endif        
        { macLEcapable, 			1, (uchar*) &mac_pib.LEcapable, 0, 1 },
#if(CGF_MAC_FUNCTIONAL_ORGANIZATION==1)          
        { macRFIDcapable, 			1, (uchar*) &mac_pib.RFIDcapable, 0, 1 },
        { macHoppingCapable, 			1, (uchar*) &mac_pib.HoppingCapable, 0, 1 },
        { macAMCACapable, 			1, (uchar*) &mac_pib.AMCACapable, 0, 1 },
        { macMetricsCapable, 			1, (uchar*) &mac_pib.MetricsCapable, 0, 1 },
        { macTSCHenabled, 			1, (uchar*) &mac_pib.TSCHenabled, 0, 1 },
        { macLLenabled, 			1, (uchar*) &mac_pib.LLenabled, 0, 1 },
        { macDSMEenabled, 			1, (uchar*) &mac_pib.DSMEenabled, 0, 1 },
#endif        
        { macLEenabled, 			1, (uchar*) &mac_pib.LEenabled, 0, 1 },
#if(CGF_MAC_FUNCTIONAL_ORGANIZATION==1)         
        { macRFIDenabled, 			1, (uchar*) &mac_pib.RFIDenabled, 0, 1 },
        { macHoppingEnabled, 			1, (uchar*) &mac_pib.HoppingEnabled, 0, 1 },
        { macAMCAenabled, 			1, (uchar*) &mac_pib.AMCAenabled, 0, 1 },
        { macMetricsEnabled, 			1, (uchar*) &mac_pib.MetricsEnabled, 0, 1 },
#endif        
        { macEBFilteringEnabled,		1, (uchar*) &mac_pib.EBFilteringEnabled, 0, 1 },
        { macEBSN,				1, (uchar*) &mac_pib.EBSN, 0, 255 },
        { macEnhancedBeaconOrder,		1, (uchar*) &mac_pib.EnhancedBeaconOrder, 0, 15 },
        { macMPMIE,				1, (uchar*) &mac_pib.MPMIE, 0, 1 },
        { macNBPANEnhancedBeaconOrder,          2, (uchar*) &mac_pib.NBPANEnhancedBeaconOrder, 0, 0x4000 },
        { macOffsetTimeSlot,			1, (uchar*) &mac_pib.OffsetTimeSlot, 1, 15 },
        { macFCSType,				1, (uchar*) &mac_pib.FCSType, 0, 1 },
#ifdef WISUN_ENET_PROFILE           
        { macPairingIEId,			8, (uchar*) &mac_pib.PairingIeIdContent, 0, 0 },
        { macCapabilityNotificationId,		1, (uchar*) &mac_pib.Capability_Notification_Val, 0, 0 },
        { enetHANSRNIEId,			11, (uchar*) &mac_pib.SrnIeIdContent, 0, 0 },
        { enetHANRoutingIEId,			27, (uchar*) &mac_pib.RoutingIeIdContent, 0, 0 },
#endif
        
        { macEnhancedAck,			1, (uchar*) &mac_pib.EnhancedAck, 0, 1 },
        { macRITDataWaitDuration,		1, (uchar*) &mac_pib.RITDataWaitDuration, 0, 0xFF},
        { macRITTxWaitDuration,			3, (uchar*) &mac_pib.RITTxWaitDuration, 0, 0xFFFFFF},
        { macRITPeriod,				3, (uchar*) &mac_pib.RITPeriod, 0, 0xFFFFFF},
        { macRITDataReqCmdConfig,		1, (uchar*)&mac_pib.RITDataReqCmdConfig, 0, 2 },

        /* end of attributes */
        { 0, 0, 0, 0, 0 }
    };

#ifdef MAC_CFG_SECURITY_ENABLED
/* Default values for security elements of the MAC PIB */
const security_pib_t default_security_pib =
{
    {NULL,NULL,0},                  /* macKeyTable */
    0,                              /* macKeyTableEntries  */
    {NULL,NULL,0} ,                 /* macDeviceTable */
    0,                              /* macDeviceTableEntries  */
    {NULL,NULL,0} ,                 /* macSecurityLevelTable */
    0,                              /* macSecurityLevelTableEntries  */
    0,                              /* macFrameCounter */
    0,                              /* macAutoRequestSecurityLevel */
    3,                              /* macAutoRequestKeyIdMode */
    MAC_DEFAULT_KEY_SOURCE_VALUE,   /* macAutoRequestKeySource */
    0x01,                           /* macAutoRequestKeyIndex  */
    MAC_DEFAULT_KEY_SOURCE_VALUE,   /* macDefaultKeySource */
    {0,0,0,0,0,0,0,0},              /* macPANCoordExtendedAddress */
    0x0000                          /* macPANCoordShortAddress */
};
#endif /* MAC_CFG_SECURITY_ENABLED */

/*
** ============================================================================
** Private Function Prototypes
** ============================================================================
*/

static uint32_t calculate_MaxFrameTotalWaitTime( void );

/*
** ============================================================================
** Public Function Definitions
** ============================================================================
*/
#if(CFG_MLME_GET_REQ_CONF == 1)
uchar process_mlme_get_request(
								   uchar *buf, 
								   uint16_t length 
                               )
{
    uchar attribute = 0;
    uchar attribute_index = 0;
    const mac_attribute_table_t *at;
    //phy_status_t phy_status;
    ulong phy_value = 0;
    uint16_t phy_length = 0;
    uchar phy_buf[4] = {0};

    attribute = buf[0];

    /* is it a PHY attribute */
    if( attribute <= MAX_PHY_PIB )
    {
    	if(attribute == phySUNChannelsSupported )
    	{    		
                uint32_t chan_buff[2] = {0};
    		PLME_get_request( attribute, &phy_length, chan_buff );
    		//mem_rev_cpy(chan_buff, channelBuffer, 8);
    		return send_mlme_get_confirm( MAC_SUCCESS, attribute, 0, phy_length, chan_buff );    		
    	}
    	else
    	{
    		//phy_status = PLME_get_request( attribute, &phy_length, &phy_value );		
        
	        if( ( PLME_get_request( attribute, &phy_length, &phy_value ) )== PHY_SUCCESS )
	        {
			*phy_buf = phy_value;
        	        
	        	if( phy_length == 2)
	        	{
	        		phy_buf[0] = phy_value & 0xff;
	        		phy_buf[1] = (phy_value >>8l) & 0xff;
                
	        	}
	        	else if( phy_length == 4 )
	        	{
		        	phy_buf[0] = phy_value & 0xff;
		            phy_buf[1] = (phy_value >>8l) & 0xff;
		            phy_buf[2] = (phy_value >>16l) & 0xff;
		            phy_buf[3] = (phy_value >>24l) & 0xff;
		     	}
	     	
			 /*   else
		     	{
		     		*phy_buf = phy_value;
		     	}*/
            
	            send_mlme_get_confirm( MAC_SUCCESS, attribute, 0, phy_length, phy_buf );
	        }
	        else
	        {
	            send_mlme_get_confirm( MAC_UNSUPPORTED_ATTRIBUTE, attribute, 0, 0, 0 );
	        }
    	}   
        return 1;
    }

     /* special case to trap phyCCADuration to map it from the MAC pib */
    if (attribute == phyCCADuration)
    {
        phy_buf[0] = mac_pib.LBTCCADuration & 0xff;
        phy_buf[1] = (mac_pib.LBTCCADuration >> 8) & 0xff;
        return send_mlme_get_confirm( MAC_SUCCESS, attribute, 0, 2, phy_buf );
    }

    /* is it a MAC attribute ? */
    if( attribute < MAC_PIB_ID_BASE )
    {
        /* no, so fail for the moment */
        return send_mlme_get_confirm( MAC_UNSUPPORTED_ATTRIBUTE, attribute, attribute_index, 0, NULL_POINTER );
    }

    /* Check if we are using 802.15.4-2006 primitives */
    if( mac_pib.ConfigFlags & ( USE_2006_PRIMITIVES|USE_2011_PRIMITIVES ) )
    {
        attribute_index = buf[1];
        /* check this is a valid index (no arrays outside security PIB) */
        if( (attribute < macKeyTable) && ( attribute_index != 0 ))
        {
            return send_mlme_get_confirm( MAC_INVALID_INDEX, attribute, attribute_index, 0, NULL_POINTER );
        }
    }
    else
    {
        /* check valid range for 2003 attributes */
        if( (attribute > macTransactionPersistenceTime)
            && ((attribute<PIB_ID_BASE)||(attribute>macIEEEAddress)) )
        {
            /* unsupported attribute */
            return send_mlme_get_confirm( MAC_UNSUPPORTED_ATTRIBUTE, attribute, attribute_index, 0, NULL_POINTER );
        }
    }
#ifndef EFM32_TARGET_IAR
    /* Power Management (sleep) mode */
    if( attribute == macLowPowerMode )
    {

        uchar pmsm_flags = lpmsm_get_flags() &
            ( LPMSM_FLAG_DISABLE_SNOOZE|LPMSM_FLAG_DISABLE_SLEEP|LPMSM_FLAG_DISABLE_DEEP_SLEEP ); 

        return send_mlme_get_confirm( MAC_SUCCESS, macLowPowerMode,
                                      attribute_index,
                                      1,
                                      &pmsm_flags );
    }
#endif


    /*TBD macStatusFlags moved to PHY */
    /* status flags are a special case - currently only battery supported */
    //if( attribute == macStatusFlags )
    //{
    //    i = check_battery_status();
    //    send_mlme_get_confirm( MAC_SUCCESS, attribute, attribute_index, 1, &i );
    //}

    /* beacon payload is a special case */
    if( attribute == macBeaconPayload )
    {
        return send_mlme_get_confirm( MAC_SUCCESS, macBeaconPayload,
            attribute_index,
            mac_pib.BeaconPayloadLength,
            mac_pib.BeaconPayload);
    }
    
    /* special case for txtime */
    if( attribute == macEBRattributeList )
    {
        return send_mlme_get_confirm( MAC_SUCCESS, macEBRattributeList, attribute_index,
                                                   mac_pib.EBRattributeList[0], &(mac_pib.EBRattributeList[1]) );
    }

    /* special case for txtime */
    if( attribute == macBeaconTxTime )
    {
        /*uchar val[3];
        val[0] = mac_pib.BeaconTxTime & 0xff;
        val[1] = (mac_pib.BeaconTxTime >> 8 )& 0xff;
        val[2] = (mac_pib.BeaconTxTime >> 16 )& 0xff;*/
        phy_buf[0] = mac_pib.BeaconTxTime & 0xff;
        phy_buf[1] = (mac_pib.BeaconTxTime >> 8 )& 0xff;
        phy_buf[2] = (mac_pib.BeaconTxTime >> 16 )& 0xff;
        return send_mlme_get_confirm( MAC_SUCCESS, macBeaconTxTime, attribute_index, 3, phy_buf );
    }
    /* special case for total tx-time */
    if( attribute == macTxTotalDuration )
    {
        phy_value = phy_total_symbols_transmitted();
        phy_buf[3] = phy_value & 0xff;
        phy_buf[2] = (phy_value >> 8 )  & 0xff;
        phy_buf[1] = (phy_value >> 16 ) & 0xff;
        phy_buf[0] = (phy_value >> 24 ) & 0xff;
        return send_mlme_get_confirm( MAC_SUCCESS, macTxTotalDuration, attribute_index, 4, phy_buf );
    }

#ifdef MAC_CFG_COLLECT_STATS
    /* MAC stats are a special case */
    if( attribute == macStats )
    {
        /* FIXME: current free queue count now in PHY */
        mac_data.mac_stats.current_phy_free_rx_queue_cnt = 0;
        mac_data.mac_stats.current_phy_rx_queue_cnt = mac_data.rx_msg_queue.count;
        return send_mlme_get_confirm( MAC_SUCCESS, macStats, attribute_index, sizeof(struct mac_stats), &mac_data.mac_stats );
    }
#endif

#ifdef MAC_CFG_SECURITY_ENABLED
    if (mac_pib.mac_security_enabled == TRUE)
    {
        /* macKeyTable, MacDeviceTable, and macSecurityLevelTable are all special cases */
        if( attribute == macKeyTable )
        {
            return send_key_table_get_confirm( attribute_index );
        }
        if( attribute == macDeviceTable )
        {
            return send_device_table_get_confirm( attribute_index );
        }
        if( attribute == macSecurityLevelTable )
        {
            return send_security_level_table_get_confirm( attribute_index );
        }
    }
    else if(attribute == macKeyTable ||  attribute == macDeviceTable || attribute == macSecurityLevelTable)
    {
      return send_mlme_get_confirm( MAC_INVALID_PARAMETER, attribute, 00, 01, NULL_POINTER );
    }
#endif

    for( at = mac_attribute_table; at->length != 0; at++ )
    {
        /* find the attribute we are requesting */
        if( attribute == at->id )
        {
            /* we found it */
            /* short integers are stored as such, may need byte swapping, so take no chances */
            if( at->length == 2 )
            {
                /* the value is being sent in little endian 
                fashion to the NHLE. eg if shortAddress is 0x1234, first 0x34 is sent followed by 0x12*/
            	//uchar val[2];
                phy_buf[1] = (*(ushort *)at->value) >> 0x08;
                phy_buf[0] = (*(ushort *)at->value)  & 0xff;
                return send_mlme_get_confirm( MAC_SUCCESS, at->id, attribute_index, at->length, phy_buf );
            }
            /* Frane Counter is stored as a ulong  */
            else if( at->length == 4 )
            {
                /* the value is being sent in little endian 
                fashion to the NHLE. eg if we have an attribute value  of 0x12345678 
                first 0x78 will be sent and 0x12 will be sent as the last byte*/
            	//uchar val[4];
                phy_buf[3] = ((*(ulong *)at->value) >> 24) & 0xff;
                phy_buf[2] = ((*(ulong *)at->value) >> 16) & 0xff;
                phy_buf[1] = ((*(ulong *)at->value) >> 8) & 0xff;
                phy_buf[0] = (*(ulong *)at->value)  & 0xff;
                return send_mlme_get_confirm( MAC_SUCCESS, at->id, attribute_index, at->length, phy_buf );
            }
            else
            {
                return send_mlme_get_confirm( MAC_SUCCESS, at->id, attribute_index, at->length, at->value );
            }
        }
    }
    /* not found, so fail !! */
    return send_mlme_get_confirm( MAC_UNSUPPORTED_ATTRIBUTE, attribute, attribute_index, 0, NULL_POINTER );
}
#endif	/*(CFG_MLME_GET_REQ_CONF == 1)*/

/******************************************************************************/
#if(CFG_MLME_SET_REQ_CONF == 1)
uchar process_mlme_set_request(
                                     uchar *buf,  
                                     uint16_t length 
                               )
{
    mac_status_t status = MAC_READ_ONLY;
    const mac_attribute_table_t *at;
    ushort phy_value = 0;
    uint32_t value = 0;
    uchar attribute = 0;
    uchar attribute_index = 0;
    uint16_t attribute_length = 0;
    uchar *attribute_value = NULL;
    uint8_t i = 0;
	//uint32_t att_val;
    sm_event_t event;
    mac_tx_t* packet = NULL;
//    uint8_t trigger = 0;
    
    uint8_t adv_prim_2011 = mac_pib.ConfigFlags & ( USE_2006_PRIMITIVES|USE_2011_PRIMITIVES);
    attribute = buf[0];

    /* check if we are using 802.15.4-2006 primtives */
    if( adv_prim_2011 )
    {
        attribute_index = buf[1];
    }
    
    attribute_length = get_ushort( ( buf + 2) ); 
    	
    attribute_value = &buf[4];
    
    /* check if read-only */
    if( attribute == macAckWaitDuration ||
        attribute == macBeaconTxTime ||
        attribute == macSuperframeOrder ||
        attribute == macSyncSymbolOffset ||
        attribute == macTimestampSupported ||
        attribute == macLBTPrevHrTotalSendingDurUS )
    {
        //status = MAC_READ_ONLY;
        goto exit;
    }
    
    if( attribute_length + 3 > length )
    {
    	status = MAC_INVALID_PARAMETER;
    	goto exit;
    }

    /* is it a PHY attribute? */
    if( attribute <= MAX_PHY_PIB )
    {
                value = *attribute_value;
	    
    	if(attribute_length == 2)
    	{
    		value = get_ushort(attribute_value);
    	}
    	else if(attribute_length == 4)
    	{
    		value = get_ulong(attribute_value);
    	}
    	
    	/*else 
    	{
    		value = *attribute_value;
    	}*/
    	switch( PLME_set_request( attribute, attribute_length, &value))//&value ) )
        {
        case PHY_SUCCESS:
            status = MAC_SUCCESS;
            event.trigger = (sm_trigger_t) TRXSM_TRIGGER_UPDATE_RX;
            /* notify TRXSM to update receiver status */
            SM_DISPATCH( (sm_t *) &trxsm, &event );
            
            /* special case to trap to map it from the MAC pib */
		    if ( attribute == phyFECEnabled )
		    {
		        if( attribute_value[0] )
		        {
		        	/*FEC is enabled in the PHY so indicate the same in the 
		        	configuration flags */
		        	mac_pib.ConfigFlags |= CONFIG_USE_PPDU_CODING;
		        }
		        else
		        {
		        	/*FEC disabled in the PHY.*/
		        	mac_pib.ConfigFlags &= ~CONFIG_USE_PPDU_CODING;
		        }
		    }
		    else if ( ( attribute == phyCurrentSUNPageEntry ) ||
					  ( attribute == phyMRFSKSFD ) ||
					  ( attribute == phyNumSUNPageEntriesSupported ) ||
					  ( attribute == phySUNPageEntriesSupported )
					)
		    {
		    	/*create a new MPM EB to reflect the new PHY PIB values used in creation 
		    	of IEs*/
		    	
		    	/*free up all the existing MPM EBs stored*/
		    	while( (packet = (mac_tx_t *) queue_item_get( trxsm.curr_mpm_eb_queue )) != NULL_POINTER )
		        {
		            mac_mem_free_tx( packet );
		        }
#if(CFG_MAC_BEACON_ENABLED == 1)           
		    	/* prepare a MPM Enhanced Beacon */
                if( mac_beacon_update( MAC_FRAME_BCN_SUB_TYPE_MPM_EB ) != MAC_SUCCESS )
                {
                    /*TBD: What to do here? */
                    //debug(("mpm bcn update err6\r\n"));
                }
#endif    //#if(CFG_MAC_BEACON_ENABLED == 1)              
		    }    
            break;

        case PHY_INVALID_PARAMETER:
            status = MAC_INVALID_PARAMETER;
            break;
        
        case PHY_READ_ONLY:
        	//status = MAC_READ_ONLY;
        	break;

        case PHY_UNSUPPORTED_ATTRIBUTE:
        default:
            status = MAC_UNSUPPORTED_ATTRIBUTE;
            break;
        }
        goto exit;
    }

     /* special case to trap phyCCADuration to map it from the MAC pib */
    if (attribute == phyCCADuration)
    {
        phy_value = ( (attribute_value[1] << 0x08) | attribute_value[0] );
        /* check if value is in range*/
        if (( phy_value >= 8) && (phy_value <= 1000))
        {
            mac_pib.LBTCCADuration = (ushort)phy_value;
            status = MAC_SUCCESS;
        }
        else
        {
            status = MAC_INVALID_PARAMETER;
        }
        goto exit;
    }

    /* is it a MAC attribute ? */
    if( attribute < MAC_PIB_ID_BASE )
    {
        status = MAC_UNSUPPORTED_ATTRIBUTE;
        goto exit;
    }

    /* Check if we are using 802.15.4-2006 primitives */
    if( adv_prim_2011 )
    {
        attribute_index = buf[1];

        /* check this is a valid index (no arrays outside security PIB) */
        if( (attribute < macKeyTable) && ( attribute_index != 0 ) )
        {
            status = MAC_INVALID_INDEX;
            goto exit;
        }
    }
    else
    {
        /* check valid range for 2003 attributes */
        if( (attribute > macTransactionPersistenceTime)
            && (attribute != macMaxFrameRetries)
            && ((attribute < PIB_ID_BASE) ||(attribute > macIEEEAddress)) )
        {
            status = MAC_UNSUPPORTED_ATTRIBUTE;
            goto exit;
        }
    }

    /* not supported */
    if( attribute == macBeaconTxTime )
    {
        status = MAC_INVALID_PARAMETER;
        goto exit;
    }

	status = MAC_SUCCESS;
    
	/* special case for macTxTotalDuration */
    if( attribute == macTxTotalDuration )
    {
        phy_reset_symbols_transmitted();
        //status = MAC_SUCCESS;
        goto exit;
    }
    /* power mode configuration*/
    if( attribute == macLowPowerMode )
    {
        
        /* --------------------------------------------
           b0 - b3: MCU low power mode config
           b4 - b5: Radio Low power mode config
           b6     : reserved
           b7     : "Autosave into NVM" config
           --------------------------------------------
           b3 b2 b1 b0
           0  0  0  0 -  MCU Always active
           0  0  0  1 -  STOP mode enabled
           0  0  1  0 - reserved for future use
           .......... - reserved for future use
           .......... - reserved for future use
           1  1  1  1 - reserved for future use
           --------------------------------------------
           b4 b5      
           0  0  - Radio always active state
           0  1  - SHUTDOWN state
           1  0  - Standby state
           1  1  - reserved for future use      
           --------------------------------------------
           b6    - reserved for future use
           --------------------------------------------
           b7    
           0 - Auto save OFF
           1 - Auto save ON
           --------------------------------------------  */
      
        status = MAC_SUCCESS;
#ifndef EFM32_TARGET_IAR
        lpmsm_set_flag( (lpmsm_flag_t) (LPMSM_FLAG_DISABLE_SNOOZE | LPMSM_FLAG_DISABLE_SLEEP | LPMSM_FLAG_DISABLE_DEEP_SLEEP |
                                          LPMSM_FLAG_DISABLE_TRX_DEEP_SLEEP | LPMSM_FLAG_DISABLE_TRX_SLEEP ) );
#endif
        /*check if user is trying to enable other than LPMSM_FLAG_DISABLE_DEEP_SLEEP and LPMSM_FLAG_TRX_DEEP_SLEEP OR
        LPMSM_FLAG_DISABLE_DEEP_SLEEP and LPMSM_FLAG_TRX_SLEEP
        */
        //perform validation TBD
       // if( attribute_value[0] & LPMSM_FLAG_DISABLE_DEEP_SLEEP | )
        
        /*before setting this let all the modes be disabled, may be during initialization of the lpmsm. ALl bits set to 1*/
        if( !attribute_value[0]  )
        {
#ifndef EFM32_TARGET_IAR 
           /* if the low power modes are being indicated as always active then set the required lpmsm flags to disable all the power modes*/
          lpmsm_set_flag(LPMSM_FLAG_DISABLE_SNOOZE | LPMSM_FLAG_DISABLE_SLEEP | LPMSM_FLAG_DISABLE_DEEP_SLEEP | LPMSM_FLAG_DISABLE_TRX_DEEP_SLEEP | LPMSM_FLAG_DISABLE_TRX_SLEEP | LPMSM_FLAG_DISABLE_SAVE_BEFORE_SLEEP);
#endif
        }
        else
        {
#ifndef EFM32_TARGET_IAR
            /*before setting this let all the modes be disabled, may be during initialization of the lpmsm. ALl bits set to 1*/
          /*TBD: need to use macros for the following hard coded values*/
           lpmsm_clear_flag( (attribute_value[0] & 0x0f) ); // lpm sm made aware of the mcu low pwoer  modes 
           
           lpmsm_clear_flag( (attribute_value[0] & 0x30) ); // lpm sm made aware of the RF Low pwoer modes
           
           lpmsm_clear_flag((attribute_value[0] & 0x80));    // lpm sm made aware of the auto save into nvm config
#endif                      
           /*above three statements can be replaced by the following 
           lpmsm_clear_flag(0xBF); This 0xBF needs to be arrived by bitwise ORing all the required lpmsm flag macros
            */
           
        }                  
        goto exit;
    }


    /* loop to find attribute */
    for( at = mac_attribute_table; at->length != 0; at++ )
    {
        /* find the attribute we are requesting */
        if( attribute != at->id )
        {
            continue;
        }

        /* check special cases first */
        if( attribute == macBeaconPayload )
        {
            if( attribute_length > aMaxBeaconPayloadLength )
            {
                status = MAC_INVALID_PARAMETER;
                goto exit;
            }

            /* update PIB */
            mac_pib.BeaconPayloadLength = attribute_length;
            if( attribute_length != 0 )
            {
                memcpy( mac_pib.BeaconPayload, attribute_value, attribute_length );
            }
#if(CFG_MAC_BEACON_ENABLED == 1) 
            /* update beacon */
            mac_beacon_update( MAC_FRAME_BCN_SUB_TYPE_RB );
#endif    //#if(CFG_MAC_BEACON_ENABLED == 1)  
            //status = MAC_SUCCESS;
            goto exit;
        }
        else if( attribute == macBeaconPayloadLength )
        {
            if( attribute_length > aMaxBeaconPayloadLength )
            {
                status = MAC_INVALID_PARAMETER;
                goto exit;
            }

            /* update PIB */
            mac_pib.BeaconPayloadLength = get_ushort(attribute_value); 
#if(CFG_MAC_BEACON_ENABLED == 1) 
            /* update beacon */
            mac_beacon_update( MAC_FRAME_BCN_SUB_TYPE_RB );
#endif  //#if(CFG_MAC_BEACON_ENABLED == 1)  
            //status = MAC_SUCCESS;
            goto exit;
        }
        else if( attribute == macLBTFlags )
        {
          
        	event.trigger = (sm_trigger_t) CCASM_TRIGGER_TRACK_TX_DURATION;        	
#if(CFG_MAC_LBTSM_ENABLED == 1)          	
        	if( *attribute_value & LBTSM_FLAG_ENABLE_TOT_TX_DUR_THRESHOLD)
        	{
        		/*trying to enable. Do nothing if it is already enabled*/
        		if( !(mac_pib.LBTFlags & LBTSM_FLAG_ENABLE_TOT_TX_DUR_THRESHOLD) )
        		{
        			//mac_pib.LBTFlags |= LBTSM_FLAG_ENABLE_TOT_TX_DUR_THRESHOLD;
                   
        			trigger = 1;
        		}        		
                        mac_pib.LBTFlags = (*attribute_value & 0x07);        		
        	}
        	else
        	{
        		/*trying to disable. Do nothing is it is already disabled*/
        		if( mac_pib.LBTFlags & LBTSM_FLAG_ENABLE_TOT_TX_DUR_THRESHOLD )
        		{
        			//mac_pib.LBTFlags &= ~LBTSM_FLAG_ENABLE_TOT_TX_DUR_THRESHOLD;
        			trigger = 1;
        		}        		        	        		        		
                        mac_pib.LBTFlags = (*attribute_value & 0x07); 
               
        	}
      	
        	lbtsm.flags = (lbtsm_flag_t) mac_pib.LBTFlags;
                if( trigger )
        	{	        	
      		
    	    	/* notify lbtsm to start tracking and counting the total tx duration*/
        	    SM_DISPATCH( (sm_t *) &lbtsm, &event );
                  
        	}
#else
                if( *attribute_value & CCASM_FLAG_ENABLE_TOT_TX_DUR_THRESHOLD)
        	{
        		/*trying to enable. Do nothing if it is already enabled*/
        		if( !(mac_pib.LBTFlags & CCASM_FLAG_ENABLE_TOT_TX_DUR_THRESHOLD) )
        		{
        			//trigger = 1;
        		}        		
                        mac_pib.LBTFlags = (*attribute_value & 0x07);        		
        	}
        	else
        	{
        		/*trying to disable. Do nothing is it is already disabled*/
        		if( mac_pib.LBTFlags & CCASM_FLAG_ENABLE_TOT_TX_DUR_THRESHOLD )
        		{
        			//trigger = 1;
        		}        		        	        		        		
                        mac_pib.LBTFlags = (*attribute_value & 0x07); 
               
        	}
#if (CFG_MAC_CCA_ENABLED == 1)                
                ccasm.flags = (ccasm_flag_t) mac_pib.LBTFlags;
                
                if( trigger )
        	{	        	
      		
    	    	/* notify ccasm to start tracking and counting the total tx duration*/
        	    SM_DISPATCH( (sm_t *) &ccasm, &event );
                  
        	}
#endif                
#endif                  
           	//status = MAC_SUCCESS;
            goto exit;
        }
        
        /*Sagar: Special case for macMaxFrameTotalWaitTime*/
        
        /*if( attribute == macMaxFrameTotalWaitTime )
        {
        	mac_pib.MaxFrameTotalWaitTime = (32 + 31 * 3) * aUnitBackoffPeriod + phyMaxFrameDuration;
        	
        	status = MAC_SUCCESS;
            goto exit;
        }*/
               
#ifdef MAC_CFG_SECURITY_ENABLED
        else if( (attribute == macKeyTable) && (mac_pib.mac_security_enabled == TRUE) )
        {
            /* Are we trying to clear an Entry */
            if( attribute_length == 0)
            {
                if ( delete_key_table_entry( attribute_index ) == TRUE )
                {
                    status = MAC_SUCCESS;
                }
                else
                {
                    status = MAC_INVALID_INDEX;
                }
                goto exit;
            }

            /* Add a New Entry */
            if ( store_key_table_entry_in_pib( attribute_value, attribute_index) == 0 )
            {
                status = MAC_TRANSACTION_OVERFLOW;                
            }
            else
            {
            	 status = MAC_SUCCESS;
            }
            
            goto exit;

            /*TBD: What if it gets here? */
        }
        else if( (attribute == macDeviceTable) && (mac_pib.mac_security_enabled == TRUE) )
        {
            /* Are we trying to clear an Entry */
            if( attribute_length == 0)
            {
                if ( delete_device_table_entry( attribute_index ) == TRUE )
                {
                    status = MAC_SUCCESS;
                }
                else
                {
                    status = MAC_INVALID_INDEX;
                }
                goto exit;
            }

            /* Add a New Entry */
            if ( store_device_table_entry_in_pib(attribute_value, attribute_index) == 0 )
            {
                status = MAC_TRANSACTION_OVERFLOW;                
            }
            else
            {
            	status = MAC_SUCCESS;
            }
            
            goto exit;

            /*TBD: What if it gets here? */
        }
        else if( (attribute == macSecurityLevelTable)  && (mac_pib.mac_security_enabled == TRUE) )
        {
            /* Are we trying to clear an Entry */
            if( attribute_length == 0)
            {
                if ( delete_security_level_table_entry( attribute_index ) == TRUE )
                {
                    status = MAC_SUCCESS;
                }
                else
                {
                    status = MAC_INVALID_INDEX;
                }
                goto exit;
            }

            /* Add a New Entry */
            if ( store_security_level_table_entry_in_pib(attribute_value, attribute_index) == 0 )
            {
                status = MAC_TRANSACTION_OVERFLOW;                
            }
            else
            {
            	status = MAC_SUCCESS;
            }            
            goto exit;

            /*TBD: What if it gets here? */
        }
#endif
        else if( attribute == macConfigFlags )
        {
            /* Special Case - We are changing which set of primitives we are using */

            /* So Send confirm before processing */
            send_mlme_set_confirm( MAC_SUCCESS , attribute, attribute_index );
            mac_pib.ConfigFlags = attribute_value[0] + (attribute_value[1] << 8);
            event.trigger = (sm_trigger_t) TRXSM_TRIGGER_UPDATE_RX;
            /* notify TRXSM to update receiver status */
            SM_DISPATCH( (sm_t *) &trxsm, &event );

            /* return PROCESSED */
            return 1;
        }
        else if ( attribute == macEBRattributeList )
        {
        	if( attribute_length > ( MAX_EBR_PIB_IDS_LIST_SIZE - 1 ))
            {
            	status = MAC_INVALID_PARAMETER;
                goto exit;
            }            
            for( i = 0; i<attribute_length; i++ )
            {
            	uint8_t pib = attribute_value[i];
            	
            	/* 0x9F,0xA1,0xA2,0xA5,0xAE */
            	if( ( pib != macTSCHenabled ) && ( pib != macDSMEenabled )
            	&& ( pib != macLEenabled ) && ( pib != macHoppingEnabled )
            	&& ( pib != macMPMIE ))
            	{
            		status = MAC_INVALID_PARAMETER;
            		goto exit;
            	}
            }           
        	memcpy(&( mac_pib.EBRattributeList[1]),attribute_value,attribute_length );
        	mac_pib.EBRattributeList[0] = attribute_length;
        	//status = MAC_SUCCESS;
        	goto exit;
        }
        /*ajay*/
		 else if ( attribute == macRITPeriod ) 
		{
		//	if(attribute_length == 0x03)
			//{
				if(mac_pib.LEenabled && attribute_length == 0x03)
				{
					memcpy((uint8_t*)&mac_pib.RITPeriod, attribute_value, attribute_length);
					
#if ( ( MAC_CFG_LE_RIT_CAPABILITY == 1 ) || ( MAC_CFG_LE_CSL_CAPABILITY == 1 ) )					
					if(low_energy_get_state(&low_energy) == LE_STATE_INIT )
					{
						trigger_rit_cmd_transmission();	
					}
					else if ( !( mac_pib.RITPeriod ) )
					{
						/* queueing failed, so reset LE-SM */
						event.trigger = (sm_trigger_t) LE_TRIGGER_CANCEL;
						SM_DISPATCH( (sm_t *) &low_energy, &event );
						
						event.trigger = (sm_trigger_t) TRXSM_TRIGGER_UPDATE_RX;
						SM_DISPATCH( (sm_t *) &trxsm, &event );
					}
					
#endif					
	            	//status = MAC_SUCCESS;
					goto exit;
				}
			//}
			
			status = MAC_INVALID_PARAMETER;
    		goto exit;
			
		}
     	else if (attribute == macRITDataWaitDuration)
     	{
     		if(mac_pib.LEenabled && attribute_length == 0x01)
     		{
     			mac_pib.RITDataWaitDuration =  *attribute_value;     			     			
				if (mac_pib.RITDataWaitDuration < mac_pib.RITPeriod)
				{
					//status = MAC_SUCCESS;
					goto exit;	
				}
			/*	else
				{
					status = MAC_INVALID_PARAMETER;
				}*/
     		}
     	/*	else
     		{
     			status = MAC_INVALID_PARAMETER;
     		}*/
     		status = MAC_INVALID_PARAMETER;
     		goto exit;
     	}

		else if (attribute == macRITTxWaitDuration)
		{
			memcpy((uint8_t*)&value, attribute_value, attribute_length);	
			
			if ( (value < mac_pib.RITPeriod) || (attribute_length != 0x03)|| (!mac_pib.LEenabled) )
			{
				status = MAC_INVALID_PARAMETER;
				goto exit;
			}	
			memcpy((uint8_t*)&(mac_pib.RITTxWaitDuration), &value, attribute_length);
			status = MAC_SUCCESS;
			goto exit;

		}
		

        /*
         * not a special attribute
         */

    /*Sagar: Added this exception for validating the length of an enetHANRoutingIEId attribute 
      after discussing with Rakesh and Anand as this is a special case as this attribute 
      will get variable in lenth of data to be stored*/
#ifdef WISUN_ENET_PROFILE          
    if (attribute != enetHANRoutingIEId) 
#endif      
        {

          /* check if length is OK */
          if( attribute_length != at->length )
          {
              status = MAC_INVALID_PARAMETER;
              goto exit;
          }
        }
        

        /* check if attribute is in valid range
         * (fortunately only single byte attributes have ranges) */
        if( (at->min_value != 0) || (at->max_value != 0) )
        {
        	
        	if( attribute_length == 0x01 )
        	{
        		
	            if( (*attribute_value > at->max_value) || (*attribute_value < at->min_value) )
	            {
	                status = MAC_INVALID_PARAMETER;
	                goto exit;
	            }
        	}
        	else if (attribute_length == 0x02)
        	{
        		uint16_t temp = 0;
        		temp = get_ushort(attribute_value);
        		if( ( temp > at->max_value) || ( temp < at->min_value) )
	            {
	                status = MAC_INVALID_PARAMETER;
	                goto exit;
	            } 
        	}
        }

        /* location is guaranteed to have sufficient space */
        if( at->length == 2 )
        {
            /* Note: short integers must be stored as short integers, not byte sequences
             * location will be aligned as a short integer */
            //*((ushort *)(at->value)) = (ushort)( (attribute_value[1] << 0x08) |
                                         //attribute_value[0] );
		*((ushort *)(at->value)) = get_ushort(attribute_value);
        }
#ifdef MAC_CFG_SECURITY_ENABLED
        /* this must be a Frame Counter */
        else if(( at->length == 4 ) && (mac_pib.mac_security_enabled == TRUE))
        {
            /* Note: Long integers must be stored as short integers, not byte sequences
             * location will be aligned as a long integer */
            *((ulong *)(at->value)) = ( (((ulong)attribute_value[3]) << 24) |
                                        (((ulong)attribute_value[2]) << 16) |
                                        (((ulong)attribute_value[1]) << 8) |
                                        (ulong)attribute_value[0] );
        }
#endif
        else
        {
        	if((at->id == macKeyTable) || (at->id == macDeviceTable) || (at->id == macSecurityLevelTable) )
        	{
        		status = MAC_INVALID_PARAMETER;
        		goto exit;
        	}
            /* update PIB */
            memcpy( at->value, attribute_value, at->length );
#if(CFG_MAC_BEACON_ENABLED == 1) 
            if( attribute == macAssociationPermit || attribute == macBattLifeExt )
            {
                /* update beacon */
                mac_beacon_update( MAC_FRAME_BCN_SUB_TYPE_RB );
            }
#endif
            if( attribute == macRxOnWhenIdle )
            {
                event.trigger = (sm_trigger_t) TRXSM_TRIGGER_UPDATE_RX;
            	/* notify TRXSM to update receiver status */
                SM_DISPATCH( (sm_t *) &trxsm, &event );
            }
        }

        // status = MAC_SUCCESS;
        goto exit;
    }

    status = MAC_UNSUPPORTED_ATTRIBUTE;

 exit:
    return send_mlme_set_confirm( status, attribute, attribute_index);
}
#endif	/*(CFG_MLME_SET_REQ_CONF == 1)*/

/******************************************************************************/

void mac_pib_init( void )
{
    uint32_t tx_dur_1_hr = 0;

#ifdef MAC_CFG_COLLECT_STATS
   mac_data.mac_stats.lowest_sp = -1;
#endif
	
    uint16_t ack_wait_duration = aUnitBackoffPeriod + aTurnaroundTime;

     
	uint16_t len = 0;
	uint32_t value = 0;
	
	PLME_get_request
	( 
		phySHRDuration, 
		&len, 
		&value 
	);
	
	
	ack_wait_duration += value;
	
	value = 0;
	
	PLME_get_request
	( 
		phySymbolsPerOctet, 
		&len, 
		&value 
	);
        tx_dur_1_hr = mac_pib.LBTPrevHrTotalSendingDurUS;
	/* set the default pib values and initialise the memory manager */
   	mac_pib =  default_pib;
	
	mac_pib.LBTPrevHrTotalSendingDurUS = tx_dur_1_hr;
   	
   	ack_wait_duration +=((TRXSM_ACK_FRAME_SIZE)*value);
   	
   	mac_pib.AckWaitDuration = ack_wait_duration;


   /* set the sequence numbers using a pseudo-random number generator */
   mac_pib.DSN = timer_rand_get();
   mac_pib.BSN = timer_rand_get();
   mac_pib.EBSN = timer_rand_get();
   
  mac_pib.MaxFrameTotalWaitTime = calculate_MaxFrameTotalWaitTime();

   /* init timing pib parameters */
   //phy_timing_pib_init( &phy_timing_pib_default );

#ifdef MAC_CFG_SECURITY_ENABLED
   /* Security Data that is NOT initialised when returning from Deep Sleep */
   mac_security_data.pib = default_security_pib;
   
   sec_list_initialise();

   mac_security_data.beacon_sec_param.security_level = 0;
   mac_security_data.beacon_sec_param.key_id_mode = 0;
   memset( mac_security_data.beacon_sec_param.key_identifier, 0xff, KEY_IDENTIFIER_MAX_LENGTH);

   mac_security_data.coord_realign_sec_param.security_level = 0;
   mac_security_data.coord_realign_sec_param.key_id_mode = 0;
   memset( mac_security_data.coord_realign_sec_param.key_identifier, 0xff, KEY_IDENTIFIER_MAX_LENGTH);

   mac_security_data.security_flags = 0;
#endif
}

/******************************************************************************/

#ifdef MAC_CFG_SECURITY_ENABLED

void mac_pib_init_security( void )
{
  
#if(CFG_MAC_BLACK_LIST_IS_USE == 1)   
    key_descriptor_t *pKeyList = NULL;
    int i = 0;
    
#endif

    //if ( mac_security_data.pib.mac_device_table != NULL_POINTER )
    {
        /* Delete All entries in Device Table */
        queue_delete((queue_t*) &mac_security_data.pib.mac_device_table );
    }
    mac_security_data.pib.mac_device_table_entries = 0;

    //if ( mac_security_data.pib.mac_security_level_table != NULL_POINTER )
    {
        /* Delete All entries in Security Level Table */
        queue_delete((queue_t*) &mac_security_data.pib.mac_security_level_table );
    }
    mac_security_data.pib.mac_security_level_table_entries = 0;

    //i = 0;
#if(CFG_MAC_BLACK_LIST_IS_USE == 1) 
    /* remove all entries and delete memory used by Key Table */
    while( (pKeyList = (key_descriptor_t *) queue_item_read_from( (queue_t*) &mac_security_data.pib.mac_key_table, i++ )) != NULL_POINTER)
    {
        if (pKeyList != NULL_POINTER)
        {
            pKeyList->key_device_list_entries = 
            pKeyList->key_id_lookup_list_entries = 
            pKeyList->key_usage_list_entries = 0;

            //if ( pKeyList->key_id_lookup_list != NULL_POINTER )
            {
                queue_delete((queue_t*) &pKeyList->key_id_lookup_list );
            }

            //if ( pKeyList->key_device_list != NULL_POINTER )
            {
                queue_delete((queue_t*) &pKeyList->key_device_list );
            }
            //if ( pKeyList->key_usage_list != NULL_POINTER )
            {
                queue_delete((queue_t*) &pKeyList->key_usage_list );
            }
        }
    }
#endif 
    //if ( mac_security_data.pib.mac_key_table != NULL_POINTER )
    {
        queue_delete((queue_t*) &mac_security_data.pib.mac_key_table );
    }
    mac_security_data.pib.mac_key_table_entries = 0;

    /* Reset PIB to Default values */
    mac_security_data.pib = default_security_pib;
}
#endif

#ifdef RUNTIME_SENDING_CONTROL_CONFIG
void mac_pib_conf_sending_control(uint16_t cycle_secs)
{
#if(CFG_MAC_LBTSM_ENABLED == 1)    
    configure_t108_sending_cotrol(&lbtsm ,cycle_secs);
#endif    
}
#endif

/*
** ============================================================================
** Private Function Definitions
** ============================================================================
*/

static uint32_t calculate_MaxFrameTotalWaitTime( void )
{
	uint8_t minBE = mac_pib.MinBE;
	uint8_t maxBE = mac_pib.MaxBE;
	uint8_t i = 0;
	uint32_t total = 0;
	uint16_t len = 0;
	uint32_t max_frame_dur = 0;
	uint8_t min = ((mac_pib.MaxCSMABackoffs)>(maxBE-minBE))?(maxBE-minBE):(mac_pib.MaxCSMABackoffs);

	for (;i<(min-1);i++)
	{
		total += (2<<(minBE+i));
	}

	total += (((2<<maxBE)-1)*(mac_pib.MaxCSMABackoffs-min));
	total *= aUnitBackoffPeriod;

	PLME_get_request( phyMaxFrameDuration, &len, &max_frame_dur );
	total += max_frame_dur;

	return total;

}
#ifdef WISUN_ENET_PROFILE
void get_self_pairing_id(uint8_t* p_pairing_id)
{
  if(p_pairing_id!=NULL)
  {
    memcpy(p_pairing_id,mac_pib.PairingIeIdContent,8);
  }  
}

void get_srnIE_Val(uint8_t* p_srnIEVal)
{
  if(p_srnIEVal!=NULL)
  {
    memcpy(p_srnIEVal,&mac_pib.SrnIeIdContent[0],11);
  }  
}
                
void update_rx(void)
{
     sm_event_t event;
     event.trigger = (sm_trigger_t) TRXSM_TRIGGER_UPDATE_RX;
     
     SM_DISPATCH( (sm_t *) &trxsm, &event );
}


#endif

/*None*/


