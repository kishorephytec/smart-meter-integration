/** \file phy.h
 *******************************************************************************
 ** \brief  Provides the different structure definitions required for MR-FSK Phy
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
#ifndef _PHY_
#define _PHY_

#include "phy_config_for_regulatory_uses.h"

#ifdef __cplusplus
extern "C" {
#endif


  
#define PHY_MODE_SELECT_ID              1
  
  
#define ENABLE_DATA_WHITENING           0x00000001  
  
#define  LONG_FCS       4
#define  SHORT_FCS      2  

#define CRC_Byte LONG_FCS
  
/*
** =============================================================================
** Public Macro definitions
** =============================================================================
*/

/**
 * \defgroup phy PHY Layer Interface
 */

/**
 ** \defgroup phy_defs  PHY Layer Definitions
 ** \ingroup phy
 */

/**
 ** \addtogroup phy_defs  PHY Layer Definitions
 */

/*@{*/
#ifdef EFM32_TARGET_IAR
  #define PHY_RX_BUFF_RESVD_SIZE				28      //Debdeep:: 24
#else
    #define PHY_RX_BUFF_RESVD_SIZE				22
#endif  

/*! Primitive id for PD_DATA_REQUEST*/
#define PHY_PD_DATA_REQUEST                             (uint8_t) 0x01

/*! Primitive id for PD_DATA_CONFIRM*/
#define PHY_PD_DATA_CONFIRM                             (uint8_t) 0x02

/*! Primitive id for PD_DATA_INDICATION*/
#define PHY_PD_DATA_INDICATION                          (uint8_t) 0x03

/*! Primitive id for PLME_CCA_REQUEST*/
#define PHY_PLME_CCA_REQUEST                            (uint8_t) 0x04

/*! Primitive id for PLME_CCA_CONFIRM*/
#define PHY_PLME_CCA_CONFIRM                            (uint8_t) 0x05

/*! Primitive id for PLME_ED_REQUEST*/
#define PHY_PLME_ED_REQUEST                             (uint8_t) 0x06

/*! Primitive id for PLME_ED_CONFIRM*/
#define PHY_PLME_ED_CONFIRM                             (uint8_t) 0x07

/*! Primitive id for PLME_SET_TRX_STATE_REQUEST*/
#define PHY_PLME_SET_TRX_STATE_REQUEST                  (uint8_t) 0x08

/*! Primitive id for PLME_SET_TRX_STATE_CONFIRM*/
#define PHY_PLME_SET_TRX_STATE_CONFIRM                  (uint8_t) 0x09

/*! Primitive id for PLME_GET_REQUEST*/
#define PHY_PLME_GET_REQUEST				(uint8_t) 0x0a

/*! Primitive id for PLME_GET_CONFIRM*/
#define PHY_PLME_GET_CONFIRM				(uint8_t) 0x0b

/*! Primitive id for PLME_SET_REQUEST*/
#define PHY_PLME_SET_REQUEST				(uint8_t) 0x0c

/*! Primitive id for PLME_SET_CONFIRM*/
#define PHY_PLME_SET_CONFIRM				(uint8_t) 0x0d

#define SYMBOLS_PER_OCTATE              8
#define FREQ_BAND_ID_SHIFT_VAL          23  // for SunPage Creation 
#define EXT_FREQ_BAND_ID_SHIFT_VAL      19  // for SunPage Creation 
 
/******************************************************************************/

  
  /******************************************************************************/

/* PHY Constants */
#ifdef MAX_FRAME_SIZE_CAPABILITY
/*!< Max SUN phy packet size */
#define aMaxPHYPacketSize			MAX_FRAME_SIZE_CAPABILITY
#else
#define aMaxPHYPacketSize			2047
#endif

/*!< Turnaround time is 1 ms for SUN PHYs (50 symbols) */
#define aTurnaroundTime				1000/HWTIMER_SYMBOL_LENGTH

/*!< CCA time in symbols for MR-FSK SUN phy */
#define aCCATime				8

/*!< Size of MR-FSK SUN phy SFD*/
#define aMRFSKSFDLength				2

/*!< Size of MR-FSK SUN phy PHR*/
#define aMRFSKPHRLength				2

/*! Defines duration of transmission in symbols*/
#define phy_tx_duration( length ) phy_tx_duration_calculate(length)

/*! Defines safety margin added to time to cover reading and writing fifo's*/
#define phySafetyMargin 300  

/******************************************************************************/

/* Macros for PHY PIB Attribute ids */

/*!< Attribute ID for phyCurrentChannel*/
#define phyCurrentChannel       		0x00	/* 2 byte  */
/*!< Attribute ID for phyChannelsSupported*/
//#define phyChannelsSupported    		0x01	/* 4 bytes not required for SUN PHYs*/
/*!< Attribute ID for phyTransmitPower*/
#define phyTransmitPower        		0x02	/* 1 byte  */

/*!< Attribute ID for phyCCAMode*/
#define phyCCAMode              		0x03	/* 1 byte  */

/*!< Attribute ID for phyCurrentPage*/
#define phyCurrentPage          		0x04    /* 1 byte  */

/*!< Attribute ID for phyMaxFrameDuration*/
#define phyMaxFrameDuration				0x05	/*RO*/

/*!< Attribute ID for phySHRDuration*/
#define phySHRDuration					0x06	/*RO*/

/*!< Attribute ID for phySymbolsPerOctet*/
#define phySymbolsPerOctet				0x07	/*RO*/

/*!< Attribute ID for phyCCADuration*/
#define phyCCADuration					0x21

/*!< Attribute ID for phyCurrentSUNPageEntry*/
#define phyCurrentSUNPageEntry			0x22

/*!< Attribute ID for phyFSKFECScheme*/
#define phyFSKFECScheme					0x23

/*!< Attribute ID for phyFSKFECInterleaving*/
#define phyFSKFECInterleaving			0x24

/*!< Attribute ID for phyMaxSUNChannelSupported*/
#define phyMaxSUNChannelSupported		0x26	/*RO*/

/*!< Attribute ID for phyModeSwitchParameterEntries*/
//#define phyModeSwitchParameterEntries	0x27

/*!< Attribute ID for phyMRFSKSFD*/
#define phyMRFSKSFD						0x28

/*!< Attribute ID for phySUNNumGenericPHYDescriptors*/
//#define phySUNNumGenericPHYDescriptors	0x29		// not supported

/*!< Attribute ID for phyNumSUNPageEntriesSupported*/
#define phyNumSUNPageEntriesSupported	0x2a

/*!< Attribute ID for phySUNChannelsSupported*/
#define phySUNChannelsSupported			0x2b	/*RO*/

/*!< Attribute ID for phySUNPageEntriesSupported*/
#define phySUNPageEntriesSupported		0x2c

/*!< Attribute ID for phyFSKPreambleRepetitions*/
#define phyFSKPreambleRepetitions		0x2d

/*!< Attribute ID for phyFSKScramblePSDU*/
#define phyFSKScramblePSDU				0x2e

/*!< Attribute ID for phyFECEnabled*/
#define phyFECEnabled   				0x2f	/*propritery phy pib*/

/*!< Attribute ID for PhyTRXState*/
#define PhyTRXState						0x30	/*propritery phy pib*/

/*!< Attribute ID for phySUNPageEntriesIndex*/
#define phySUNPageEntriesIndex			0x31	/*propritery phy pib*/

/*!< Attribute ID for phyRSSIThreshold*/
#define phyRSSIThreshold 				0x32	/*propritery phy pib*/

/*!< Attribute ID for MAX_PHY_PIB*/
#define MAX_PHY_PIB						0x33	/* Not used anywhere */

/*!< Attribute ID for phyRSSIOffset*/
#define phyRSSIOffset			                0x33	/*propritery phy pib*/
  
  /*Suneet :: added for when operate on explicit channel plan not using channel plan*/
#define phyExplicit_Channel_plan                        0x34  
#define phyExplicit_canter_freq                         0x35
#define phyExplicit_total_ch_number                     0x36
  
/*!< Default total PHY_PIB*/
#define TOTAL_PHY_PIBS					26

/******************************************************************************/
  
  
/**
 *******************************************************************************
 ** \enum Frequency Bands
 **  Phy frequency Bands supported
 *******************************************************************************
 **/
 

/**
 *******************************************************************************
 ** \enum FSK_Phy_Mode
 **  IDs for different modes in the SUN Page entry
 *******************************************************************************
 **/
typedef enum phy_mode_tag
{
	FSK_PHY_MODE_NONE	= 0x00000000,
	FSK_PHY_MODE_1 		= 0x00000001,
	FSK_PHY_MODE_2 		= 0x00000002,
	FSK_PHY_MODE_3 		= 0x00000004,
        FSK_PHY_MODE_4 		= 0x00000008,
	FSK_PHY_MODE_5 		= 0x00000010,
        FSK_PHY_MODE_6 		= 0x00000020,
	FSK_PHY_MODE_7 		= 0x00000040,
	FSK_PHY_MODE_8 		= 0x00000080,
	FSK_PHY_MODE_9 		= 0x00000100,
        FSK_PHY_MODE_10 	= 0x00000200,
        FSK_PHY_MODE_11 	= 0x00000400,
        FSK_PHY_MODE_12 	= 0x00000800,
        FSK_PHY_MODE_13 	= 0x00001000,
        FSK_PHY_MODE_14 	= 0x00002000,
        FSK_PHY_MODE_15 	= 0x00004000,
        FSK_PHY_MODE_16 	= 0x00008000,
        FSK_PHY_MODE_17 	= 0x00010000,
        FSK_PHY_MODE_18 	= 0x00020000,
        FSK_PHY_MODE_19 	= 0x00040000,
	FSK_PHY_MODE_MASK 	= 0xFFFFFFFF
}phy_mode_t; 	


/*
** =============================================================================
** Public Structures, Unions & enums Type Definitions
** =============================================================================
**/


/**
 *******************************************************************************
 ** \enum Modulation
 **  Enumerations for different Modulations.
 *******************************************************************************
 **/
typedef enum modulation_type_tag
{
	FILTERED_2FSK_MODN,
	FILTERED_4FSK_MODN,
	OFDM_MODN,
	OQPSK_MODN	
}modulation_type_t;



/**
 *******************************************************************************
 ** \enum Freq_Ban_IDs 
 ** Frequency band ids used for setting the freq band field in the SUN Page entry
 *******************************************************************************
 **/
							


#define FREQ_BAND_902_928               0x03800000   
#define FREQ_BAND_917_923               0x04000000 
#define FREQ_BAND_920_928               0x04800000   
#define FREQ_BAND_863_870               0x02000000
#define FREQ_BAND_870_876               0x05800000
#define FREQ_BAND_865_867               0x07000000
   
#define FREQ_BAND_EXTENDED              0x07800000							
#define FREQ_BAND_MASK                  0x07800000



#define EXT_FREQ_BAND_915_MEXICO              0x00000000 													
#define EXT_FREQ_BAND_915_BRAZIL              0x00080000 													
#define EXT_FREQ_BAND_915_ANZ                 0x00100000 													
#define EXT_FREQ_BAND_915_EUROPE              0x00180000 													
#define EXT_FREQ_BAND_915_PHILIPPINES         0x00200000 													
#define EXT_FREQ_BAND_919_MALAYSIA            0x00280000 													
#define EXT_FREQ_BAND_920_CHINA               0x00300000 													
#define EXT_FREQ_BAND_920_HK_SIN_THAI_VIET    0x00380000 
#define EXT_FREQ_BAND_866_SINGAPORE           0x00400000 
#define EXT_FREQ_BAND_MASK                    0x00780000 													


/**
 *******************************************************************************
 ** \enum Mod_Type_IDs
 **	 Modulation type ids used for setting the Modulation type field in the SUN 
 **  Page entry
 *******************************************************************************
 **/


#define MOD_FSK			0x00000000
#define MOD_OFDM		0x00020000
#define MOD_O_QPSK		0x00040000
#define MOD_BIT_MASK	        0x00060000


/**
 *******************************************************************************
 ** \enum PHY_Status
 **  Enumerations for different Status of phy
 *******************************************************************************
 **/
typedef enum phy_status_tag
{
	PHY_BUSY					= 0,	/**< Phy layer is in busy state*/
	PHY_BUSY_RX					= 1,	/**< Transceiver is busy receiving data*/
	PHY_BUSY_TX					= 2,	/**< Transceiver is busy transmitting data*/
	PHY_FORCE_TRX_OFF			        = 3,	/**< Transceiver gets OFF irrespective of the state it is in*/
	PHY_IDLE					= 4,	/**< Phy layer is in idle state*/
	PHY_INVALID_PARAMETER		                = 5,	/**< Error while an invalid range or an supported parameter is entered */
	PHY_RX_ON					= 6,	/**< Transceiver is in receiving mode*/
	PHY_SUCCESS					= 7,	/**< Successful completion of call*/
	PHY_TRX_OFF					= 8,	/**< Transceiver is in OFF state*/
	PHY_TX_ON					= 9,	/**< Transceiver is in transmitting mode*/
	PHY_UNSUPPORTED_ATTRIBUTE	                = 0xa,  /**< Error while an supported attribute is entered */
	PHY_READ_ONLY			        	= 0xb,  /**< Attempt to set value for a read only pib*/
	PHY_DEV_ON					= 0xc,  /**< Default state of Phy*/
	UNSUPPORTED_MODE_SWITCH		                = 0x22, /**< Indicates that mode switching is not supported*/
	UNSUPPORTED_PPDU_FEC		                = 0x23, /**< Indicates that FEC is not supported*/
	UNSUPPORTED_TX_CHANNEL		                = 0x24, /**< Indicates that the specified channel is not supported*/
	UNSUPPORTED_MR_OQPSK_SPREADING_MODE             = 0x25, /**< Indicates that the O-QPSK spreading is not supported*/
        RAIL_PHY_TX_ERROR                               = 0x26, /**< Indicates that the PHY channel Busy Event come From Rail>**/
	PHY_HW_ERROR	                                = 0xFF
}phy_status_t;

/******************************************************************************/

/**
 *******************************************************************************
 ** \enum SYMBOL RATES
 **  Enumerations for different SYMBOL rates
 *******************************************************************************
 **/


#define DEF_SUNPAGE_VAL                 0x4F000007  // India , 3 PHYBAND supporetd
/*! Default channel page */
#define DEF_CHANNEL_PAGE				0x9

/*! Default channel number used*/
#define DEF_CHANNEL					0x00

/*! Default 10dBm TX Power is used*/
#define DEF_TX_POWER				255 // 10 dBm

/*! Default CCA Mode1 is used to access the Channel is busy or free*/
#define DEF_CCA_MODE				0x01

/*! Default FEC Enable value*/
#define DEF_FSKFEC_SWITCH			0x00 // Disabled

/*! Default FEC used for FSK*/
#define DEF_FSKFEC_SCHEME			0x00 // NRNSC

/*! Default FEC interleaving used for FSK*/
#define DEF_FSKFEC_INTERLEAVING		0x00

/*! Default Multi-Rate and Multi-Regional used for FSK*/
#define DEF_MRFSKSFD				0x00

/*! Default Preamble Repitions used for FSK*/
#define	DEF_PREAMBLE_REPITITIONS	0x000F	// bytes

#ifdef WISUN_ENET_PROFILE
/*! Default scramble used for FSK*/
#define DEF_FSKSCRAMBLE				0x1
#else
/*! Default scramble used for FSK*/
#define DEF_FSKSCRAMBLE				0x0
#endif

#define READ_ONLY				0x8000
#define TX_POWER_BITS			        0x3F
#define MIN_RSSI_THRESHOLD		        -103    //Suneet :: change on 31jan 2019 
#define MAX_RSSI_THRESHOLD		        -50

#define MIN_RSSI_OFFSET		                -30
#define MAX_RSSI_OFFSET		                 30


#define MAX_ED_VAL				0xFF
#define MAX_LQI_VAL				0xFF

/*! Default -80dBm of RSSI Threshold is used for 920 MHz PHY*/
#define DEF_RSSI_THRESHOLD			-80                

#ifdef RL78_ADF7023_MURATA
/*! Defines receiver sensitivity value for phy mode 1[50 Kbps, GFSK(BT=0.5)]*/
#define RX_SENSITIVITY_PHY_MODE_1			(-109)  // TBD 

/*! Defines receiver sensitivity value for phy mode 2[100 Kbps, GFSK(BT=0.5)]*/
#define RX_SENSITIVITY_PHY_MODE_2			(-106)  // TBD 

/*! Defines RSSI's Upper limit for phy mode 1[50 Kbps, GFSK(BT=0.5)]*/
#define RSSI_UL_PHY_MODE_1			(-30)  // TBD 

/*! Defines  RSSI's Upper limit for phy mode 1[50 Kbps, GFSK(BT=0.5)]*/
#define RSSI_LL_PHY_MODE_1			(-109)  // TBD 

/*! Defines RSSI's Upper limit for phy mode 2[100 Kbps, GFSK(BT=0.5)]*/
#define RSSI_LL_PHY_MODE_2			(-106)  // TBD 

/*! Defines RSSI's Upper limit for phy mode 2[100 Kbps, GFSK(BT=0.5)]*/
#define RSSI_UL_PHY_MODE_2			(-30)  // TBD 

#else
// this is being used for EFM32_TARGET_IAR ( EFM32+Si4461)
/*! Defines receiver sensitivity value for phy mode 1[50 Kbps, GFSK(BT=0.5)]*/
#define RX_SENSITIVITY_PHY_MODE_1			(-109)  // TBD 

/*! Defines receiver sensitivity value for phy mode 2[100 Kbps, GFSK(BT=0.5)]*/
#define RX_SENSITIVITY_PHY_MODE_2			(-109)  // TBD 

/*! Defines RSSI's Upper limit for phy mode 1[50 Kbps, GFSK(BT=0.5)]*/
#define RSSI_UL_PHY_MODE_1			(-30)  // TBD 

/*! Defines  RSSI's Upper limit for phy mode 1[50 Kbps, GFSK(BT=0.5)]*/
#define RSSI_LL_PHY_MODE_1			(-109)  // TBD 

/*! Defines RSSI's Upper limit for phy mode 2[100 Kbps, GFSK(BT=0.5)]*/
#define RSSI_LL_PHY_MODE_2			(-109)  // TBD 

/*! Defines RSSI's Upper limit for phy mode 2[100 Kbps, GFSK(BT=0.5)]*/
#define RSSI_UL_PHY_MODE_2			(-30)  // TBD

#endif
/*! Defines default common signalling mode*/
#define COMMON_SIGNALLING_MODE		( 0x00000001)

/*! Default number of channels for MR_FSK*/
#define MR_FSK_NUM_CHAN  	(( (BAND_WIDTH) - (GL) - (GH) )/ (CHANNEL_SPACING) )

/*! Default maximum channel capabilty */


///*! Default maximum channel capabilty */
#define MAX_CHANNELS_CAPABILITY					17

/*! Default maximum SUN Page entries capabilty */
#define MAX_SUN_PAGE_ENTRIES_CAPABILITY			MAX_REGULATORY_DOMAIN_SUPPORTED //0x03

#define TOTAL_RSSI_RANGE (RSSI_UL_PHY_MODE_2-RSSI_LL_PHY_MODE_2)
//////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 *******************************************************************************
 ** \struct PHY_PIB
 ** Storage structure for PHY PIBs 
 *******************************************************************************
 **/
typedef struct phy_pib_tag
{
	uint16_t 	CurrentChannel;  /**< Holds the current channel*/     		
	uint8_t 	TransmitPower;   /**< Holds the Tx power*/     		
	uint8_t 	CCAMode;         /**< Holds the type of CCA used*/     		
	uint8_t 	CurrentPage;     /**< Holds the current page*/    		
	uint16_t 	MaxFrameDuration;/**< Holds the Maximum frame duration value */				
	uint8_t 	SHRDuration;	 /**< Holds the duration value of the SHR in symbols*/					
	uint8_t 	SymbolsPerOctet; /**< indicates the number of symbols needed for transmitting 1 byte*/					
	uint16_t 	CCADuration;	 /**< Duration used for performing CCA*/					
	uint32_t 	CurrentSUNPageEntry; /**< Holds the currently active sun page*/			
	uint8_t 	FSKFECScheme;      /**< Holds the FEC scheme*/					
	uint8_t 	FSKFECInterleaving;	/**< Indicates if FEC inteleaving is supported or not*/				
	uint16_t 	MaxSUNChannelSupported;	/**< Stores the total number of channels supported in the currently active phy mode*/	
	uint8_t 	MRFSKSFD;				/**< Indicates if start of frame delimiter status*/								
	uint8_t 	NumSUNPageEntriesSupported;	/**< Number of stored sun page entries in the SUNPageEntriesSupported table*/
	uint8_t 	SUNChannelsSupported[ MAX_CHANNELS_CAPABILITY ];			/**< Stores the bit map indicating which all channels are supported for the active phy mode*/			
	uint32_t 	SUNPageEntriesSupported[ MAX_SUN_PAGE_ENTRIES_CAPABILITY ];	/**< Stores all the supported sun page entries*/	
	uint16_t 	FSKPreambleRepetitions;	/**< Stores the number of preamble bytes that should be sent as part of SHR*/	
	uint8_t 	FSKScramblePSDU;	/**< Indicates if Scrambling is enabled or disabled*/	
	uint8_t     FECEnabled;			/**< Indicates if FEC is enabled or disabled*/			
	phy_status_t TRXState;			/**< Stores the TRX state*/
        int8_t      RSSIThreshold;		/**< Stores the RSSI threshold to be used for CCA*/
        int8_t RSSIOffset;             /**<*/
	uint8_t     SUNPageEntryIndex;	/**< Stores the current sun page entry which can be used for storing a new entry*/	
	phy_status_t pendingStateChange;/**< Stores the pending state to be changed to if TRX state change is not allowed at the time of request*/
        uint8_t explicit_channel_plan;
        uint32_t explicit_center_ch_freq;
        uint8_t total_explicit_number_of_channel;
        
}phy_pib_t;

/**
 *******************************************************************************
 ** \struct phy_transmission(phy_tx_t)
 **			Used by MAC Layer(NHLE to PHY) for issuing PD-Data request
 *******************************************************************************
 **/
typedef struct phy_tx_struct
{
	struct phy_tx_struct *link;		/**< Ptr to next msg in list */
	uint16_t TxChannel;				/**< Holds the channel on which tx should be done */
	uint8_t PPDUCoding;				/**< Indicates if the PPDU should be FEC encoded or not*/
	uint8_t FCSLength;				/**< The size of the FCS in the passed PPDU. True for 32-bit CRC, false for 16-bit CRC */
	uint8_t ModeSwitch;				/**< Indicates if PPDU should be transmitted in a different mode */
	uint8_t NewModeSUNPage;			/**< The new SUN page if mode switching is required */
	uint8_t ModeSwitchParameterEntry;/**< The index to the table holding the mode switch info */
	uint16_t psduLength;			/**< Length of the psdu to be transmitted excluding CRC */
	uint8_t psdu[1];				/**< Place holder from where the psdu bytes are placed*/
}phy_tx_t;

/**
 *******************************************************************************
 ** \struct phy_reception(phy_rx_t)
 **		Used by the PHY Layer to receive a packet,which is then passed to the 
 **		MAC Layer for further processing
 *******************************************************************************
 **/
typedef struct phy_rx_struct
{
    struct phy_rx_struct *link;			/**< Ptr to next msg */    	
    uint32_t sfd_rx_time;				/**< Packet reception timestamp */\
    uint64_t rx_current_hw_time;	    /**<Packet current time at reception *///Suneet
    uint64_t complete_rx_hw_time;            /*Suneet :: this is when complete packet read */   
    uint16_t psduLength;				/**< Length of the psdu in the received packet*/
    uint16_t FCSLength;					/**< Length of FCS recieved along with this packet */	
    //int32_t psduLinkQuality;			/**< LQI of the received packet */
    int8_t rssi;
    uint8_t lqi;
    uint32_t channel;					/**< The channel on which packet is received*/
    uint8_t reserved[PHY_RX_BUFF_RESVD_SIZE];
    uint8_t psdu[1];					/**< Place holder from where the psdu bytes are placed*/	
}phy_rx_t;


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



extern const uchar phy_reg_max_pib;

extern int8_t peak_rssi_value;

/*
** ============================================================================
** Public Function Prototypes
** ============================================================================
*/
 
/** \defgroup pd_functions PHY Data Functions
 ** \ingroup phy
 */

/**
 ** \addtogroup pd_functions PHY Data Functions
 */

/*@{*/

/**
 *******************************************************************************
 ** \brief Implements the PD-Data-Request primitive
 ** \param *pHandle[in] - pointer to handle
 ** \param *pTxpkt[in] - pointer from where the payload starts
 ** \retval - None
 ******************************************************************************/
void PD_Data_Request( void* pHandle, phy_tx_t* pTxpkt );

/* call back functions implemented and provided by MAC*/

/**
 *******************************************************************************
 ** \brief Entry point to MAC Layer from PHY for sending PD-Data-confirm 
 **  This function is send to MAC layer upon completion of a data transmission
 ** \param *pHandle[in] - pointer to handle
 ** \param status[in] - SUCCESS on successful transmission of request\n ERROR 
 **        status upon unsuccessful transmission
 ** \note This is implemented by the MAC Layer
 ******************************************************************************/
extern void PD_Data_Confirmation_cb( void* pHandle,uint8_t status, uint32_t tx_ts_us );

/**
 *******************************************************************************
 ** \brief Entry point to MAC Layer from PHY when a packet is received
 **        when needs to be conveyed to MAC layer
 ** \param *pRxpkt[in] - pointer from where the payload starts\
 ** \param state_change_req - indicates if the NHLE has to perform any TRX state change
 ** \note This is implemented by the MAC Layer
 ******************************************************************************/
extern void PD_Data_Indication_cb(phy_rx_t* pRxpkt, bool state_change_req);

/*@}*/

/******************************************************************************/

/** \defgroup phy_required_functions PHY Required Functions
 * \ingroup phy
 */

/** \addtogroup phy_required_functions PHY Required Functions
 */

/*@{*/


/**
 *******************************************************************************
 ** \brief Function for initializing the PHY Layer. Cold start indicates if the 
 **		PHY needs to be started cold,i.e all the phy PIBs should be loaded with 
 **		default settings
 ** \param cold_start[in] - if set to true loads all the phy PIBs to default 
 **                         values
 ** \retval None
 ******************************************************************************/
void PHY_Init( uint8_t cold_start );

/**
 *******************************************************************************
 ** \brief Function for resetting the PHY Layer. 
 ** \param - None
 ** \retval - None
 ******************************************************************************/
void PHY_Reset ( void );


/**
 *******************************************************************************
 ** \brief Function to indicate that the phy receive is in progress
 ** \param - None
 ** \retval - 1 if the receiver is busy
 ** \retval - 0 if idle
 ******************************************************************************/
uint8_t PHY_rcv_in_progress( void );

/**
 *******************************************************************************
 ** \brief Function to calculate the phy packets transmission duartion
 ** \param length - length of the packet to transmit
 ** \retval - time for packet transmission
 ******************************************************************************/
uint32_t phy_tx_duration_calculate( uint16_t length );

/**
 *******************************************************************************
 ** \brief Function to indicates the total amount of data symbols that has 
 ** been transmitted
 ** \param - None
 ** \retval - total symbols transmitted
 ******************************************************************************/
uint32_t phy_total_symbols_transmitted(void);
/**
 *******************************************************************************
 ** \brief Function to resets the counter for the total amount of symbols 
 ** tranmitted
 ** \param - None
 ** \retval - status
 ******************************************************************************/
phy_status_t phy_reset_symbols_transmitted( void );

/*@}*/

/******************************************************************************/

/** \defgroup plme_apis PHY Management APIs 
 * \ingroup phy
 */

/** \addtogroup plme_apis PHY Management APIs 
 */

/*@{*/

/**
 *******************************************************************************
 ** \brief Implements the PLME-CCA-Request primitive
 **   This function does cca_sample and assess the channel is busy or idle
 ** \param - None
 ** \retval -  IDLE or BUSY indicating a successful CCA
 ** \retval - TRX_OFF indicating an error code
 ** \note The receiver should be ON while performing PLME-CCA-Request  
 ******************************************************************************/
phy_status_t PLME_CCA_Request(void);

/**
 *******************************************************************************
 ** \brief Implements the PLME-SET-TRX-STATE-Request primitive
 **   This function set's state of the transceiver.If this function requests a 
 **   state that the transceiver is already configured,then returns with a 
 **   status indicating the current state, i.e., RX_ON, TRX_OFF,or TX_ON
 ** \param state[in] - state of the transceiver to be changed
 ** \retval - SUCCESS if the transceiver is changed from one state to other state
 ** \retval - RX_ON or TRX_OFF or TX_ON indicating the current state of transceiver
 ******************************************************************************/
phy_status_t PLME_Set_TRX_State( phy_status_t state );

/**
 *******************************************************************************
 ** \brief Implements the PLME-SET-Request primitive
 **		Sets the values for the PHY-PIB attributes
 ** \param pib_attribute - Contains the PIB attribute ID for which the value is 
 ** to be set
 ** \param size[in] - specifies the size of the PIB attribute
 ** \param val[in] -  contains the requested value to be set onto the requested 
 **					PIB attribute
 ** \retval - PHY_SUCCESS if the value requested is set successfully onto the 
 **				requested PIB attribute
 ** \retval - PHY_INVALID_PARAMETER if the value requested to be set is out of 
 **				valid range or the size specified is out of valid range
 ** \retval - PHY_READ_ONLY if the requested attribute is a READ_ONLY attribute
 ** \retval - PHY_UNSUPPORTED_ATTRIBUTE if the attribute ID is invalid
 ******************************************************************************/
phy_status_t PLME_set_request( uint8_t pib_attribute, uint16_t size, uint32_t *val);

/**
 *******************************************************************************
 ** \brief Implements the PLME-GET-Request primitive
 **		Retrieves the value for the requested PIB attribute which has been set by 
 **		default or explicitely set using the plme_set_request primitive
 ** \param pib_attribute - Specifies the PIB attribute ID
 ** \param length[in] - Indicates how many bytes to be read from the value buffer
 ** \param value[in] - Contains the value of the requested PIB attribute
 ** \retval - PHY_SUCCESS if the value for the requested PIB attribute is 
 **			  retrieved successfully
 ** \retval - PHY_UNSUPPORTED_ATTRIBUTE if the attribute ID is invalid
 ******************************************************************************/
phy_status_t PLME_get_request( uint8_t pib_attribute, uint16_t *length, uint32_t *value );

/**
 *******************************************************************************
 ** \brief Function for triggering the CCA by switcing on the receiver
 ** \param *p[in] - pointer to handle
 ** \retval - None
 ******************************************************************************/
phy_status_t plme_get_trx_state_request(void);

/**
 *******************************************************************************
 ** \brief Function for triggering the CCA by switcing on the receiver
 ** \param *p[in] - pointer to handle
 ** \retval - None
 ******************************************************************************/
phy_status_t phy_cca_on( void *p );

/**
 *******************************************************************************
 ** \brief Function for getting a CCA sample
 ** \param - None
 ** \retval - None 
 ******************************************************************************/
//void phy_cca_sample( void );

/**
 *******************************************************************************
 ** \brief Function for getting the result of the CCA after the CCA duration
 ** \param - None
 ** \retval - IDLE or BUSY based on the RSSI value
 ******************************************************************************/
phy_status_t phy_cca_stop( void );

/**
 *******************************************************************************
 ** \brief Function for switching on the receiver for performing ED measurement
 ** \param *p[in] - pointer to handle
 ** \retval - None
 ******************************************************************************/
phy_status_t phy_ed_on( void *p );
phy_status_t phy_ed_off( void *p );

/**
 *******************************************************************************
 ** \brief Function for getting an ED sample
 ** \param - None
 ** \retval - RSSI value
 ******************************************************************************/
//int8_t phy_ed_sample( void );

/**
 *******************************************************************************
 ** \brief Implements the PLME-ED-Request primitive
 **        This function performs the ED measurement
 ** \param  *p_ed_val[out] - pointer to store the RSSI value
 ** \retval - IDLE or BUSY based on the RSSI value
 ** \note The receiver should be ON while performing PLME_ED_Request
 *******************************************************************************/
phy_status_t PLME_ED_Request( uint8_t* p_ed_val );

/**
 *******************************************************************************
 ** \brief Implements the RSSI to ED conversion
 **        This function performs the conversion of RSSI to ED value
 ** \param  rssi - holds the RSSI value which is to be converted
 ** \retval - The calculated ED value
 *******************************************************************************/
uint8_t convert_RSSI_To_ED( int8_t rssi );

/**
 *******************************************************************************
 ** \brief Implements the ED to RSSI conversion
 **        This function performs the conversion of ED to RSSI value
 ** \param  rssi[in] - holds the ed value which is to be converted
 ** \retval - The calculated RSSI value
 *******************************************************************************/
char convert_ED_To_RSSI( uint8_t ed_val );

/**
 *******************************************************************************
 ** \brief Implements the RSSI to LQI conversion
 **        This function performs the conversion of RSSI to LQI value
 ** \param  rssi[in] - holds the RSSI value which is to be converted
 ** \retval - The calculated LQI value
 *******************************************************************************/
uint8_t convert_RSSI_To_LQI( int8_t rssi_val );

/**
 *******************************************************************************
 ** \brief Implements the LQI to RSSI conversion
 **        This function performs the conversion of LQI to RSSI value
 ** \param  rssi[in] - holds the LQI value which is to be converted
 ** \retval - The calculated RSSI value
 *******************************************************************************/
char convert_LQI_To_RSSI( uint8_t lqi );

/**
 *******************************************************************************
 ** \brief Implements setting of phy mode
 **        This function performs the setting of the PHY to different mode
 ** \param  - None
 ** \retval - None
 *******************************************************************************/
void plme_set_phy_mode_request( void );

/**
 *******************************************************************************
 ** \brief Implements to get the Symbol Rate
 **        This function is used to get the symbol rate of the phy
 ** \param  - None
 ** \retval - returns the symbol rate of the current phy
 *******************************************************************************/
uint8_t PHY_Get_Symbol_Rate( void );

/**
 *******************************************************************************
 ** \brief Implements to reset the PHY sports driver
 **        This function is used to forcefully reset the PHY sports driver 
 ** \param id - index value
 ** \param *data - pointer to the data to be placed in the desired index
 ** \retval - None
 *******************************************************************************/
//void phy_reset_sports_driver(int id, void *data );

/**
 *******************************************************************************
 ** \brief Implements to reset sports driver
 **        This function is used to reset sports driver
 ** \param  - None
 ** \retval - None
 *******************************************************************************/
//void reset_sports_driver( void );

/**
 *******************************************************************************
 ** \brief Implements to get the CSM Unit Radio Channel list
 **        This function is used to get the channel list
 ** \param curr_chan - channel on which the csm radio value is read
 ** \param *p_channel_cnt - pointer to count the channel
 ** \param *p_channels - pointer to the channels
 ** \retval - None
 *******************************************************************************/
void PLME_Get_CSM_Unit_Radio_Chan_List
( 
	uint8_t curr_chan,
	uint8_t* p_channel_cnt, 
	uint8_t* p_channels 
);

/******************************************************************************/

/**
 *******************************************************************************
 ** \brief Implements to set Radio to the requested Channel 
 **        This function is used to set the channel 
 ** \param channel - Channel value
 ** \retval - status
 *******************************************************************************/
phy_status_t phy_set_current_channel( uint16_t channel );

/**
 *******************************************************************************
 ** \brief Gets the total number of symbols transmitted count
 ** \param None
 ** \retval - total number of symbols transmitted count
 *******************************************************************************/
uint32_t phy_get_total_symbols_txed(void);

/**
 *******************************************************************************
 ** \brief resets the total number of symbols transmitted 
 ** \param None
 ** \retval - None
 *******************************************************************************/
void phy_reset_total_symbols_txed(void);


typedef struct ch_change_tag
{
uint8_t intC_ch_set;
uint32_t frac_val;
} ch_change_set;

phy_status_t phy_set_frequncy_on_explicit();

uint8_t get_max_supported_channel_on_phy_selected();
/*@}*/

#ifdef __cplusplus
}
#endif
#endif /* _PHY_*/
