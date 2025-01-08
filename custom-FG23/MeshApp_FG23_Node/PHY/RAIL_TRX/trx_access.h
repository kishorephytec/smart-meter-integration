/** \file trx_access.h
 *******************************************************************************
 ** \brief provides APIs to access the RF driver functionality
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


#ifndef _TRX_ACCESS_H_
#define _TRX_ACCESS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "rail_types.h"

/**
********************************************************************************
* @ingroup sysdoc
*
* @{
********************************************************************************/

 

 /**
 *******************************************************************************
 * 
 * @}   
 ******************************************************************************/


/*
** =============================================================================
** Public Macro definitions
** =============================================================================
*/

/**
 ** \defgroup trx TRX Access Layer Interface
 */

/**
 ** \defgroup trx_access_defs  TRX Layer Definitions
 ** \ingroup trx
 */

/*@{*/
//
////#define RF_DEBUG
///*! Preamble length in bytes */
//#define PREAMBLE_LEN_IN_BYTES			8
//
///*! Preamble length in nibbles */
//#define PREAMBLE_LEN_IN_NIBBLES		(PREAMBLE_LEN_IN_BYTES<<1)
//
///*! Sync word length in bytes */
//#define SYNC_WORD_LEN_IN_BYTES			2
//
//#if(SYNC_WORD_LEN_IN_BYTES==1)
//#define SYNC_LENGTH_CONFIG		0
//#elif(SYNC_WORD_LEN_IN_BYTES==2)
//#define SYNC_LENGTH_CONFIG		1
//#elif(SYNC_WORD_LEN_IN_BYTES==3)
//#define SYNC_LENGTH_CONFIG		2
//#elif(SYNC_WORD_LEN_IN_BYTES==4)
//#define SYNC_LENGTH_CONFIG		3
//#else
//#error "Not supported"
//#endif
//
///*! Minimum out power supported (in dBm) */
//#define MIN_OUTPUT_POWER				-10// as per datasheet and not as per the components used 
//
///*! Maximum out power supported (in dBm) */
//#define MAX_OUTPUT_POWER				13 // as per datasheet and not as per the components used
//
///*! Enables PN9 data whitening in the Radio */
//#define Enable_Data_Whitening()			TRX_Set_DW_Prop(1)
//
///*! Disables PN9 data whitening in the Radio */
//#define Disable_Data_Whitening()		TRX_Set_DW_Prop(0)		
//	
///*! API reading the RSSI value */
//#ifdef USE_RSSI_METHOD_2
//	#define TRX_Get_RSSI(x) TRX_Get_RSSI_Method2(x)
//#else
//	#define TRX_Get_RSSI(x) TRX_Get_RSSI_Current(x)
//  
//#endif

/*
** =============================================================================
** Public Structures, Unions & enums Type Definitions
** =============================================================================
*/


//typedef void (*trx_data_event_cb_t)( volatile uint16_t*  nEvent,uint8_t* p_buff );
typedef void (*trx_data_event_cb_t)( volatile uint8_t*  nEvent );


uint16_t get_TRX_curr_PHY_total_channel_supported ( void );

/*!
 *******************************************************************************
 ** \struct mr_fsk_shr_t
 ** structure to hold all the information needed by TRX access module for 
 ** building the MR-FSK SHR. 
 ******************************************************************************/
typedef struct mr_fsk_shr_tag
{
	unsigned char preamble_byte; /**< holds MR_FSK Preamble byte*/
	unsigned char preamble_rep; /**< holds MR_FSK Preamble length in bytes*/
	unsigned short sfd;			/**< holds sfd*/
	unsigned char fec_coding;	/**< b0: for fec enable/disble, b1: for NRNSC or RSC, b2: Interleave enabled/disabled */		
}mr_fsk_shr_t;

/*!
 *******************************************************************************
 ** \union shr_info_t
 **     shr union to hold all the inforamtion needed by TRX access module to 
 **     build the SHR. 
 ******************************************************************************/
typedef union shr_info
{
	mr_fsk_shr_t	mr_fsk_shr;	/**< Holds information about MR_FSK_SHR*/
}shr_info_t;


/*!
 *******************************************************************************
 ** \struct SHR_t
 **		SHR structure to hold all the inforamtion needed by TRX access module to 
 **	    build the SHR. 
 ******************************************************************************/
typedef struct shr_tag
{
	unsigned char shr_type; /**< holds the type of the SHR to be built*/
	shr_info_t shr_info;	/**< holds the information for building the indicated SHR type*/
}SHR_t;


typedef struct trx_details_tag
{
  uint16_t pkt_tx_irq;
  uint16_t pkt_rx_irq;
  uint16_t crc32_failures;
  uint16_t crc16_failures;
}trx_details_t;

/*!
 *******************************************************************************
 ** \enum event
 **      Enumeration for different TRX event types
 ******************************************************************************/
enum
{ 
	NO_EVENT 	= 0x00,   /**< Indicates that there is no event from the TRX */
	TX_COMPLETE = 0x02,   /**< Indicates that TRX successfully transmitted a frame */
	RX_COMPLETE = 0x04,    /**< Indicates that TRX successfully received a frame*/
	SFD_DETECTED = 0x08	  /**< Indicates that TRX has detected a SFD and is busy 
							in getting the whole packet*/
};

typedef enum mod_type_tag
{
	MOD_TYPE_UNMODULATED_CARRIER,
	MOD_TYPE_OOK,
	MOD_TYPE_2FSK,
	MOD_TYPE_2GFSK,
	MOD_TYPE_4FSK,
	MOD_TYPE_4GFSK
}mod_type_t;

//typedef enum mod_src_tag
//{
//	MOD_SRC_PKT_HANDLER_INFO,
//	MOD_SRC_DIRECT_MODE_PIN,
//	MOD_SRC_PN9
//}mod_src_t;


/*!
 *******************************************************************************
 ** \enum SHR_TYPES
 **		Enumeration for differnt types of SHR that can be built by the TRX Access 
 **		module 
 ******************************************************************************/
enum
{
	SHR_TYPE_MR_FSK_SHR,	/**< SHR type as MR_FSK_SHR */
	SHR_TYPE_MR_O_QPSK,		/**< SHR type as MR_O_QPSK */
	SHR_TYPE_MR_OFDM		/**< SHR type as MR_ODMK */
};

/*!
 *******************************************************************************
 ** \enum FEC_coding 
 ** Enumeration for values that can be specified for creating the MR-FSK SHR. 
 ******************************************************************************/

enum
{
	MASK_FEC_BIT	 			= 0x01,
	NRNSC_FEC_BIT				= 0x02,
	RSC_FEC_BIT					= 0x00,
	MASK_FEC_SCHEME_BIT			= 0x02,
	MASK_INTERLEAVING_BIT		= 0x04	
};


/*@}*/


/*
** =============================================================================
** Public Variable Declarations
** =============================================================================
*/

/*
** =============================================================================
** Public Function Prototypes
** =============================================================================
*/
extern uint32_t timer_current_time_get_high_32(void);
extern uint32_t Get_Chan_Center_Freq0( void );
extern uint16_t Get_Channel_Spacing_kHz( void );
void handle_sync_detec_event( void );
void handle_tx_done_event( uint8_t status, uint32_t tx_ts );
void handle_rx_pending_event(  uint16_t psdu_len,uint8_t fcs_error );
uint8_t Get_Preamble_len_nibbles( void );
void detection_callback(void);
extern void  trx_packet_rx_cb( void );
extern void trx_packet_sent_cb( void );

/** \defgroup trx_access_req_func TRX Required Functions
 ** \ingroup trx
 */

/*@{*/

/**
 *******************************************************************************
 ** \brief Initialises the radio with the configured settings and brings it into 
 **        PHY on state
 ** \param - None
 ** \return SUCCESS or FAILURE
 ** \note This function should be called once during the system initialization 
 ******************************************************************************/
//uint8_t TRX_Init( uint8_t phy_mode );
//uint8_t TRX_Init( uint32_t phy_mode , uint8_t** p_phy_mode_setings , uint8_t dataRate);

/**
 *******************************************************************************
 ** \brief Resets the TRX module and brings the RF to PHY ON state and 
 ** 	uninitialises all device resources
 ** \param - None
 ** \return SUCCESS or FAILURE
 ** \note Not being used. 
 ******************************************************************************/
//uint8_t TRX_Reset(void);

/**
 *******************************************************************************
 ** \brief registers call back for processing the data events 
 ** \param - cb - callback function to be called whenever there are events 
 *  from the TRX.
 ** \return SUCCESS or FAILURE
 ******************************************************************************/
uint8_t TRX_Register_Data_Event_cb(trx_data_event_cb_t cb);


/*@}*/

/** \defgroup trx_access_control_functions TRX Control Functions
 ** \ingroup trx
 */

/*@{*/

/**
 *******************************************************************************
 ** \brief Queries and sets the radio to phy ON state
 ** \param - None
 ** \return SUCCESS or FAILURE
 *******************************************************************************/
uint8_t TRX_On( void );

/**
 *******************************************************************************
 ** \brief Queries and sets the radio to phy TX_ON state
 ** \param - None
 ** \return SUCCESS or FAILURE
 ******************************************************************************/
uint8_t TRX_TX_On( void );

/**
 *******************************************************************************
 ** \brief Queries and sets the radio to phy RX_ON state
 ** \param - None
 ** \return SUCCESS or FAILURE
 ******************************************************************************/
uint8_t TRX_Rx_On( void );
void TRX_ED_On( void );
void TRX_Stop_ED( void );

/**
 *******************************************************************************
 ** \brief Queries and sets the radio to phy OFF state
 ** \param - None
 ** \return SUCCESS or FAILURE
 ******************************************************************************/
uint8_t TRX_Off( void );

/**
 *******************************************************************************
 ** \brief Provides RSSI reading
 ** \param[out] *pRSSIval - pointer to RSSI value
 ** \return SUCCESS or FAILURE
 ** \note 1)This function should be called in PHY_ON state\n
 **		  2)Reads the received input power in 2's complement dBm
 ******************************************************************************/
uint8_t TRX_Get_RSSI_Current( int8_t* pRSSIval );


/**
 *******************************************************************************
 ** \brief Provides RSSI reading using mode 3 
 ** \param[out] *pRSSIval - pointer to RSSI value.
 ** \return SUCCESS or FAILURE
 ** \note 1)This function should be called in PHY_RX state\n
 **		  2)Reads the received input power in 2's complement dBm
 ******************************************************************************/


/**
 *******************************************************************************
 ** \brief Sets the transmitter PA power level
 ** \param[in] pwr_dbm - transmit power value
 ** \param[out] *p_pa_mcr_level - pointer to pa_mcr_level  value.
 ** \return SUCCESS or FAILURE
 ** \note 1)This function calculates the PA level by using pa_level = ( *p_pa_mcr_level - 3)/4; 
 ******************************************************************************/
//uint8_t TRX_Set_Output_Power( int8_t pwr_dbm, uint8_t* p_pa_mcr_level );

/**
 *******************************************************************************
 ** \brief Sets the transmitter PA power level
 ** \param[in] level - transmit power value(0 to 63)
 ** \return SUCCESS or FAILURE
 ** \note 1)This function calculates the PA level by using pa_level = ( level - 3)/4; \n
 **       2) Valid range for level is 0 to 63
 ******************************************************************************/
uint8_t TRX_Set_PA_Level_MCR( uint8_t level );

/**
 *******************************************************************************
 ** \brief Sets the Channel Frequency
 ** \param[in] center_freq - frequency value in Hz
 ** \return SUCCESS or FAILURE
 ** \note Sets the frequency to the closest possible value.
 ******************************************************************************/
uint8_t TRX_Set_Channel_Frequency( uint32_t center_freq );

/**
 *******************************************************************************
 ** \brief Sets the Frequency Deviation
 ** \param[in] freq_dev - frequency deviation value
 ** \return SUCCESS or FAILURE
 ** \note The value passed in should be the desired frequency deviation in kHz*10
 *******************************************************************************/

//uint8_t TRX_Set_Freq_Deviation( uint16_t freq_dev, mod_type_t mod_type );

/**
 *******************************************************************************
 ** \brief Sets the data rate
 ** \param[in] data_rate - data_rate value
 ** \return SUCCESS or FAILURE
 ** \note The value passed in must be 10 times of the desired data rate in kbps
 ******************************************************************************/
//uint8_t TRX_Set_Data_Rate( uint32_t data_rate, mod_type_t  mod_type );

/**
 *******************************************************************************
 ** \brief Sets the modulation scheme
 ** \param[in] mod_scheme - mod_scheme value
 ** \return SUCCESS or FAILURE.
 ** \note MOD_2FSK = 0x0\n MOD_2GFSK = 0x1\n MOD_OOK = 0x2\n MOD_CARRIER = 0x3\n
 ******************************************************************************/
//uint8_t TRX_Mod_Scheme( uint8_t mod_scheme );


/**
 *******************************************************************************
 ** \brief Reads the PA level
 ** \param - None
 ** \return PA MCR value
 ******************************************************************************/
uint8_t TRX_Read_pa_level( void );

/**
 *******************************************************************************
 ** \brief Converts pa_mcr_level value to corresponding dBm value
 ** \param[in] pa_mcr_level - value to be converted
 ** \return index value
 ** \note Valid range for pa_mcr_level is 0 to 63
 ******************************************************************************/
int8_t TRX_Convert_pa_mcr_level_to_dBm(uint8_t pa_mcr_level );


/*@}*/

/** \defgroup trx_access_data_functions TRX Data Functions 
 ** \ingroup trx
 */

/*@{*/

/**
 *******************************************************************************
 ** \brief Builds and writes the PHY packet into the TX buffer 
 ** and enables the SPOTRTS interface for transmission
 ** \param[in] p_shr_info - contains all the info for building the SHR,
 ** \param[in] phr - holds the 2 byte PHR to be put in the frame, 
 ** \param[in] *pd_data - pointer where the payload including the FCS is present
 ** \param[in] pd_length - payload length including the FCS field length
 ** \return SUCCESS or FAILURE.
 ** \note Creates the frame in the rx buffer of the SPORTS interface and 
 **			enables the SPORTS data flow.
 ******************************************************************************/
uint8_t TRX_Write
		(
			SHR_t* p_shr_info,
			uint16_t phr, 
			uint8_t* pd_data,  
			uint16_t pd_length 
		);
			

/**
 *******************************************************************************
 ** \brief Sets the rx buffer for reception and also enables the SPORTS interface 
 **  for data reception 
 ** \param - None
 ** \return SUCCESS or FAILURE.
 ** \note None.
 ******************************************************************************/
//uint8_t TRX_Set_RX_Buffer( void );


/**
 *******************************************************************************
 ** \brief Reads the received frame from the rx buffer and copies to 
 **			the application buffer
 ** \param[out] *pd_data - pointer where the frame needs to be copied
 ** \param[in]  pd_length - length of the frame to be copied
 ** \return SUCCESS or FAILURE.
 ** \note Applciation has to allocate a buffer and call this function to 
 **			get the frame recieved. Applciation needs to use 
 **			TRX_get_RX_packet_len() to know the length of the frame being read 
 **			before allocating a buffer
 ******************************************************************************/
uint8_t TRX_Read_RX_Buffer( uint8_t* pd_data, uint8_t* p_rx_pkt,  uint16_t pd_length );


extern int get_pld_len( void );
void Reset_packet_Rx( void );

/**
 *******************************************************************************
 ** \brief Function get the total number of symbols transmitted count
 ** \param - None
 ** \return - Total number of symbols transmitted
 ******************************************************************************/
uint32_t TRX_Total_Symbols_Txed(void);

/**
 *******************************************************************************
 ** \brief Function to reset the total number of symbols transmitted count
 ** \param - None
 ** \return - None
 ******************************************************************************/
void TRX_Stop_Tx_Steram(void);
void TRX_Reset_Total_Symbols_Txed(void);
//uint8_t TRX_Set_PA_Level( uint8_t pa_level );
//uint8_t TRX_Set_Freq_Deviation( uint16_t freq_dev, mod_type_t mod_type );
//uint8_t TRX_Set_Mod_Source(RAIL_StreamMode_t mod_src );

uint8_t TRX_Xmit_StreamModeOn( RAIL_StreamMode_t mod_src  );

//void set_cap_bank_val( uint8_t cap_bank );
void set_get_rf_properties(uint8_t set_or_get,uint8_t grp, uint8_t start_prop_id, uint8_t num_prop,uint8_t* p_properties );
void TRX_Set_DW_Prop( uint8_t DW_enable );
void TRX_WakeUp( uint8_t switch_rx_on, uint32_t chan_freq);

//void TRX_WakeUpFromSleep( uint8_t switch_rx_on  );
uint32_t TRX_get_last_packet_rx_time_H(void);
uint8_t are_radio_settings_intact( void );
void set_rssi_comp( uint8_t offset );
void get_trx_details(trx_details_t* p_trx_details, uint8_t reset_after_read );
//uint8_t TRX_Set_Output_Power_mW( uint8_t mw, uint8_t* p_pa_level );
//void configure_rssi_cal_val( uint8_t rssi_cal );

/*@}*/



#ifdef __cplusplus
  }
#endif

#endif /*_TRX_ACCESS_H_*/

