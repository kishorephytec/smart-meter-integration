/** \file mac_app_build_config.h
 *******************************************************************************
 ** \brief
 ** Brief Describes about the MAC Layers Configuarion
 **
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

#ifndef MAC_APP_BUILD_CONFIG_H
#define MAC_APP_BUILD_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

/*
** ============================================================================
** Public Macro definitions
** ============================================================================
*/

#define DEBUG_FLAG		0 // used for enabling or disabling the debug code

//#define MAC_CFG_RFD
#if !defined(MAC_CFG_RFD) && !defined(MAC_CFG_FFD)
#define MAC_CFG_FFD	
#endif

/*! Define a Full Function Device - FIXME: Must be defined at present */
#ifdef MAC_CFG_FFD
  
#ifndef WISUN_ENET_PROFILE
  
  // USED for FAN Stack..
  
#define CFG_MCPS_PURGE_REQ_CONF				0////
#define CFG_MLME_ASSOCIATE_REQ_CONF			0////
#define CFG_MLME_ASSOCIATE_IND_RESP			0////
#define CFG_MLME_DISASSOCIATE_REQ_CONF                  0//// 	// This should be 1 always for both RFD and FFD
#define CFG_MLME_DISASSOCIATE_IND			0////	// This should be 1 always for both RFD and FFD
#define CGF_MLME_BEACON_NOTIFY_IND			0////
#define CGF_MLME_BEACON_REQ_CONF			0////
#define CGF_MLME_BEACON_REQUEST_IND			0////
#define CFG_MLME_GET_REQ_CONF				1////
#define CFG_MLME_SET_REQ_CONF				1////
#define CGF_MAC_FUNCTIONAL_ORGANIZATION                 0

#ifdef MAC_CFG_GTS_ENABLED
	#ifdef MAC_CFG_BEACONING_ENABLED
		#define CFG_MLME_GTS_REQ_CONF		1
		#define CFG_MLME_GTS_IND		1
	#else
		#define CFG_MLME_GTS_REQ_CONF		0
		#define CFG_MLME_GTS_IND		0
	#endif
#else
  // Raka :: This condtion is used in FAN MAC
	#define CFG_MLME_GTS_REQ_CONF			0
	#define CFG_MLME_GTS_IND			0
#endif

#define CFG_MLME_ORPHAN_IND_RESP			0///
#define CGF_MLME_RESET_REQ_CONF				1////    //not required as it should be present no matter it is RFD or FFD
#define CFG_MLME_RX_ENABLE_REQ_CONF			0////
#define CFG_MLME_START_REQ_CONF				1////
#define CFG_MLME_SYNC_LOSS_IND				1////   // this should be always present on both FFD and RFD
#define CFG_MLME_POLL_REQ_CONF				0////
#define CFG_PANID_CD_BY_PANC				0////
#define CFG_ENERGY_DETECTION_SCAN			1////
#define CFG_ACTIVE_SCAN					0////
#define CFG_ORPHAN_SCAN					0////
#define CFG_ENHANCED_ACTIVE_SCAN			0
#define CFG_MPM_EB_PASSIVE_SCAN				0
#define CFG_ASK_CHILD_TO_DISASSOCIATE		        0 // capability of making child to disassociate from the PAN
#define MAC_CFG_LE_RIT_CAPABILITY			0
#define WISUN_ENET_EACK					1
#define WISUN_ENET_FRAME_FORMAT				0
#define WISUN_ENET_LBT					0

#define CGF_MLME_RESET_REQ_CONF				1////    //not required as it should be present no matter it is RFD or FFD 
  
#else

  // wi-sun enet profile config for FFD
#define CFG_MCPS_PURGE_REQ_CONF				0////
#define CFG_MLME_ASSOCIATE_REQ_CONF			0////
#define CFG_MLME_ASSOCIATE_IND_RESP			0////
#define CFG_MLME_DISASSOCIATE_REQ_CONF                  0//// 	// This should be 1 always for both RFD and FFD
#define CFG_MLME_DISASSOCIATE_IND			0////	// This should be 1 always for both RFD and FFD
#define CGF_MLME_BEACON_NOTIFY_IND			1////
#define CGF_MLME_BEACON_REQ_CONF			1////
#define CGF_MLME_BEACON_REQUEST_IND			1////
#define CFG_MLME_GET_REQ_CONF				1////
#define CFG_MLME_SET_REQ_CONF				1////
#define CGF_MAC_FUNCTIONAL_ORGANIZATION                 1  

#ifdef MAC_CFG_GTS_ENABLED
	#ifdef MAC_CFG_BEACONING_ENABLED
		#define CFG_MLME_GTS_REQ_CONF		1
		#define CFG_MLME_GTS_IND		1
	#else
		#define CFG_MLME_GTS_REQ_CONF		0
		#define CFG_MLME_GTS_IND		0
	#endif
#else
	#define CFG_MLME_GTS_REQ_CONF			0
	#define CFG_MLME_GTS_IND			0
#endif

#define CFG_MLME_ORPHAN_IND_RESP			0////
#define CGF_MLME_RESET_REQ_CONF				1////    //not required as it should be present no matter it is RFD or FFD
#define CFG_MLME_RX_ENABLE_REQ_CONF			0////
#define CFG_MLME_START_REQ_CONF				1////
#define CFG_MLME_SYNC_LOSS_IND				0////   // this should be always present on both FFD and RFD
#define CFG_MLME_POLL_REQ_CONF				0////
#define CFG_PANID_CD_BY_PANC				0////
#define CFG_ENERGY_DETECTION_SCAN			1////
#define CFG_ACTIVE_SCAN					0////
#define CFG_ORPHAN_SCAN					0////
#define CFG_ENHANCED_ACTIVE_SCAN			1
#define CFG_MPM_EB_PASSIVE_SCAN				0
#define CFG_ASK_CHILD_TO_DISASSOCIATE		        0 // capability of making child to disassociate from the PAN
#define MAC_CFG_LE_RIT_CAPABILITY			0
#define WISUN_ENET_EACK					1
#define WISUN_ENET_FRAME_FORMAT				1
#define WISUN_ENET_LBT					1


#endif  // for #ifndef WISUN_ENET_PROFILE
  
#else
  
#ifndef WISUN_ENET_PROFILE
    
/*! Define a Reduced Function Device - FIXME: Must be defined at present */
#define CFG_MCPS_PURGE_REQ_CONF				0
#define CFG_MLME_ASSOCIATE_REQ_CONF			1
#define CFG_MLME_ASSOCIATE_IND_RESP			0
#define CFG_MLME_DISASSOCIATE_REQ_CONF                  1
#define CFG_MLME_DISASSOCIATE_IND			1
#define CGF_MLME_BEACON_NOTIFY_IND			1
#define CGF_MLME_BEACON_REQ_CONF			0
#define CGF_MLME_BEACON_REQUEST_IND			0
#define CFG_MLME_GET_REQ_CONF				1
#define CFG_MLME_SET_REQ_CONF				1
#define CGF_MAC_FUNCTIONAL_ORGANIZATION                 0  

#ifdef MAC_CFG_GTS_ENABLED
	#ifdef MAC_CFG_BEACONING_ENABLED
		#define CFG_MLME_GTS_REQ_CONF		1
		#define CFG_MLME_GTS_IND		1
	#else
		#define CFG_MLME_GTS_REQ_CONF		0
		#define CFG_MLME_GTS_IND		0
	#endif
#else
	#define CFG_MLME_GTS_REQ_CONF			0
	#define CFG_MLME_GTS_IND			0
#endif

#define CFG_MLME_ORPHAN_IND_RESP			0
#define CGF_MLME_RESET_REQ_CONF				1 //not required as it should be present no matter it is RFD or FFD
#define CFG_MLME_RX_ENABLE_REQ_CONF			0	//
#define CFG_MLME_START_REQ_CONF				1
#define CFG_MLME_SYNC_LOSS_IND				1
#define CFG_MLME_POLL_REQ_CONF				1
#define CFG_PANID_CD_BY_PANC				0
#define CFG_ENERGY_DETECTION_SCAN			1
#define CFG_ACTIVE_SCAN					1
#define CFG_ORPHAN_SCAN					1
#define CFG_ENHANCED_ACTIVE_SCAN			1
#define CFG_MPM_EB_PASSIVE_SCAN				1 
#define CFG_ASK_CHILD_TO_DISASSOCIATE		        0 // capability of making child to disassociate from the PAN 
#define MAC_CFG_LE_RIT_CAPABILITY			1
#define WISUN_ENET_EACK					0
#define WISUN_ENET_FRAME_FORMAT				0
#define WISUN_ENET_LBT					0

#else
  
  //RFD config as per wi-sun ENET profile
  /*! Define a Reduced Function Device - FIXME: Must be defined at present */
#define CFG_MCPS_PURGE_REQ_CONF				0
#define CFG_MLME_ASSOCIATE_REQ_CONF			0
#define CFG_MLME_ASSOCIATE_IND_RESP			0
#define CFG_MLME_DISASSOCIATE_REQ_CONF                  0
#define CFG_MLME_DISASSOCIATE_IND			0
#define CGF_MLME_BEACON_NOTIFY_IND			1
#define CGF_MLME_BEACON_REQ_CONF			0
#define CGF_MLME_BEACON_REQUEST_IND			0
#define CFG_MLME_GET_REQ_CONF				1
#define CFG_MLME_SET_REQ_CONF				1
#define CGF_MAC_FUNCTIONAL_ORGANIZATION                 1  

#ifdef MAC_CFG_GTS_ENABLED
	#ifdef MAC_CFG_BEACONING_ENABLED
		#define CFG_MLME_GTS_REQ_CONF		1
		#define CFG_MLME_GTS_IND		1
	#else
		#define CFG_MLME_GTS_REQ_CONF		0
		#define CFG_MLME_GTS_IND		0
	#endif
#else
	#define CFG_MLME_GTS_REQ_CONF			0
	#define CFG_MLME_GTS_IND			0
#endif

#define CFG_MLME_ORPHAN_IND_RESP			0
#define CGF_MLME_RESET_REQ_CONF				1 //not required as it should be present no matter it is RFD or FFD
#define CFG_MLME_RX_ENABLE_REQ_CONF			0	//
#define CFG_MLME_START_REQ_CONF				1
#define CFG_MLME_SYNC_LOSS_IND				0
#define CFG_MLME_POLL_REQ_CONF				0
#define CFG_PANID_CD_BY_PANC				0
#define CFG_ENERGY_DETECTION_SCAN			1
#define CFG_ACTIVE_SCAN					0
#define CFG_ORPHAN_SCAN					0
#define CFG_ENHANCED_ACTIVE_SCAN			1
#define CFG_MPM_EB_PASSIVE_SCAN				0 
#define CFG_ASK_CHILD_TO_DISASSOCIATE		        0 // capability of making child to disassociate from the PAN 
#define MAC_CFG_LE_RIT_CAPABILITY			0
#define WISUN_ENET_EACK					1
#define WISUN_ENET_FRAME_FORMAT				1
#define WISUN_ENET_LBT					1

#endif  // for #ifndef WISUN_ENET_PROFILE



#endif

#ifdef MAC_CFG_BEACONING_ENABLED
    #define CFG_MLME_SYNC_REQ			        1
    #define CFG_PASSIVE_SCAN			        1
#else
    #define CFG_MLME_SYNC_REQ			        0 //
    #define CFG_PASSIVE_SCAN			        0
#endif  /* MAC_CFG_BEACONING_ENABLED */

#if ((CFG_ENERGY_DETECTION_SCAN == 1)      || \
     (CFG_ACTIVE_SCAN == 1)  || \
     (CFG_ORPHAN_SCAN == 1) || \
     (CFG_PASSIVE_SCAN == 1) || \
	 (CFG_ENHANCED_ACTIVE_SCAN == 1) || \
	 (CFG_MPM_EB_PASSIVE_SCAN == 1))
    #define CFG_MLME_SCAN_REQ_CONF                  1
	
#else
    #define CFG_MLME_SCAN_REQ_CONF                  0
#endif

/*need to include SECURITY ALSO to define this*/
#if ((CFG_MLME_ORPHAN_IND_RESP == 1) || (CFG_MLME_ASSOCIATE_IND_RESP == 1))
    #define CGF_MLME_COMM_STATUS_IND				1
#else

#ifdef MAC_CFG_SECURITY_ENABLED
    #define CGF_MLME_COMM_STATUS_IND				1
#else
    #define CGF_MLME_COMM_STATUS_IND			        0
#endif

#endif

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
	
/* None*/

/*
**=========================================================================
**  Public Function Prototypes
**=========================================================================
*/

/* None*/

#ifdef __cplusplus
}
#endif
#endif /* MAC_APP_BUILD_CONFIG_H */
