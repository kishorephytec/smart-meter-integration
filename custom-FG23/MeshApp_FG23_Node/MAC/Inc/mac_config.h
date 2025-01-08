/** \file mac_config.h
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

#ifndef MAC_CONFIG_H
#define MAC_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

/*
** ============================================================================
** Public Macro definitions
** ============================================================================
*/
//#define MAC_CFG_BEACONING_ENABLED
//#define MAC_CFG_GTS_ENABLED
//#define MAC_CFG_RFD

#if !defined(MAC_CFG_RFD) && !defined(MAC_CFG_FFD)
#define MAC_CFG_FFD 
#endif

/*! Defines size of beacon packet pool */
#define MAX_BCN_MESSAGES 2

/*! Defines size of MPM EB packet pool */
#define MAX_MPM_EB_MESSAGES 2

/*! Defines size of MPM EB packet pool */
#define MAX_EB_MESSAGES 2

/*! Defines sizes and quantity for the MAC buffers */
#ifndef MAX_RCV_MESSAGES
	#if MIN_MAC_BUFFERS
		#define MAX_RCV_MESSAGES                2
		#define MAX_DTX_MESSAGES                2
	#else
		#define MAX_RCV_MESSAGES                4 
		#define MAX_DTX_MESSAGES                20 

	#endif
#else
	#define MAX_DTX_MESSAGES  20
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
#define CFG_MAC_BEACON_ENABLED                          0 
#define CFG_MAC_DRSM_ENABLED                            0
#define CFG_MAC_LBTSM_ENABLED                           0
#define CFG_MAC_CCA_ENABLED                             0  
#define CFG_MAC_MPMSM_ENABLED                           0
#define CFG_MAC_PENDADDR_ENABLED                        0
#define CFG_MAC_PTSM_ENABLED                            0
#define CFG_MAC_SCANSM_ENABLED                          0 
#define CFG_MAC_SFTSM_ENABLED                           0
#define CFG_MAC_VERSION_ENABLED                         0
#define CFG_MAC_STARTSM_ENABLED                         0
#define CFG_MAC_SYNCSM_ENABLED                          0  
#define CFG_MAC_BLACK_LIST_IS_USE                       0 
#define UPDATE_UFSI_AFTER_CCA				0  // 1 : CCA is done and than Update the UFSI , 0: UFSI Update and after that CCA 
#define SUBSCRIBE_TX_PACKET                             0  /* 1 = Enable TX packet subscription;  0 = Disable TX packet subscription */
#define CFG_ASSOCSM_ENABLED                             0 

#define CGF_MLME_RESET_REQ_CONF				1////    //not required as it should be present no matter it is RFD or FFD

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

#define CFG_MLME_ORPHAN_IND_RESP			0////
#define CGF_MLME_RESET_REQ_CONF				1////    //not required as it should be present no matter it is RFD or FFD
#define CFG_MLME_RX_ENABLE_REQ_CONF			0////
#define CFG_MLME_START_REQ_CONF				1////
#define CFG_MLME_SYNC_LOSS_IND				1////   // this should be always present on both FFD and RFD
#define CFG_MLME_POLL_REQ_CONF				0////
#define CFG_PANID_CD_BY_PANC				0////
#define CFG_ENERGY_DETECTION_SCAN			0////
#define CFG_ACTIVE_SCAN					0////
#define CFG_ORPHAN_SCAN					0////
#define CFG_ENHANCED_ACTIVE_SCAN			0
#define CFG_MPM_EB_PASSIVE_SCAN				0
#define CFG_ASK_CHILD_TO_DISASSOCIATE		        0 // capability of making child to disassociate from the PAN
#define MAC_CFG_LE_RIT_CAPABILITY			0
#define WISUN_ENET_EACK					1
#define WISUN_ENET_FRAME_FORMAT				0
#define WISUN_ENET_LBT					0

#else
    
  // FFD configuration as per wi-sun ENET profile
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
#define CFG_MLME_START_REQ_CONF				0////
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


#endif
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
#define CFG_MLME_START_REQ_CONF				0
#define CFG_MLME_SYNC_LOSS_IND				1
#define CFG_MLME_POLL_REQ_CONF				1
#define CFG_PANID_CD_BY_PANC				0
#define CFG_ENERGY_DETECTION_SCAN			0
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
  // RFD configurations as per wi-sun enet profile
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
#define CFG_MLME_START_REQ_CONF				0
#define CFG_MLME_SYNC_LOSS_IND				0
#define CFG_MLME_POLL_REQ_CONF				0
#define CFG_PANID_CD_BY_PANC				0
#define CFG_ENERGY_DETECTION_SCAN			0
#define CFG_ACTIVE_SCAN					0
#define CFG_ORPHAN_SCAN					0
#define CFG_ENHANCED_ACTIVE_SCAN			1
#define CFG_MPM_EB_PASSIVE_SCAN				0 
#define CFG_ASK_CHILD_TO_DISASSOCIATE		        0 // capability of making child to disassociate from the PAN 
#define MAC_CFG_LE_RIT_CAPABILITY			0
#define WISUN_ENET_EACK					1
#define WISUN_ENET_FRAME_FORMAT				1
#define WISUN_ENET_LBT					1


#endif
#endif

#ifdef MAC_CFG_BEACONING_ENABLED
    #define CFG_MLME_SYNC_REQ			        1
	 #define CFG_PASSIVE_SCAN			1
#else
    #define CFG_MLME_SYNC_REQ			        0 //
	 #define CFG_PASSIVE_SCAN			0
#endif  /* MAC_CFG_BEACONING_ENABLED */

#if ((CFG_ENERGY_DETECTION_SCAN == 1)      || \
     (CFG_ACTIVE_SCAN == 1)  || \
     (CFG_ORPHAN_SCAN == 1) || \
     (CFG_PASSIVE_SCAN == 1) || \
	 (CFG_ENHANCED_ACTIVE_SCAN == 1) || \
	 (CFG_MPM_EB_PASSIVE_SCAN == 1))
    #define CFG_MLME_SCAN_REQ_CONF                      1
	
#else
    #define CFG_MLME_SCAN_REQ_CONF                      0
#endif

#if ((CFG_MLME_ORPHAN_IND_RESP == 1) || (CFG_MLME_ASSOCIATE_IND_RESP == 1))/*need to include SECURITY ALSO to define this*/
    #define CGF_MLME_COMM_STATUS_IND			1
#else
#ifdef MAC_CFG_SECURITY_ENABLED
    #define CGF_MLME_COMM_STATUS_IND			1
#else
	#define CGF_MLME_COMM_STATUS_IND		0
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
#endif /* MAC_CONFIG_H */
