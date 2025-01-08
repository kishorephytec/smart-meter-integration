/** \file phy_si.c
 *******************************************************************************
 ** \brief
 ** Implements the PHY Layer required functions.
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
#include "StackPHYConf.h"
#include "common.h"
#include "trx_access.h"
#include "phy.h"

/*
** ============================================================================
** Private Macro definitions
** ============================================================================
*/

#if SNIFFER
#define BUSY_RX_MAX_TIME                2000000
#else
#define BUSY_RX_MAX_TIME                1000000
#endif
/*
** ============================================================================
** Private Structures, Unions & enums Type Definitions
** ============================================================================
*/

/* None */

/*
** ============================================================================
** Public Variable Definitions
** ============================================================================
*/

/*
** ============================================================================
** Private Variable Definitions
** ============================================================================
*/

const uchar phy_reg_max_pib = 20;

/*
** ============================================================================
** External Variable Declarations
** ============================================================================
*/

extern phy_pib_t phy_pib;
extern void restore_trx_state( phy_status_t  state );
extern phy_status_t backup_trx_state( void );
extern int8_t phy_get_transmit_power( void );
extern uint32_t get_channel_freq(uint16_t channel_num);


/*
** ============================================================================
** Public Function Prototypes
** ============================================================================
*/

uint8_t Get_Preamble_len_nibbles(void );

/*
** ============================================================================
** Private Function Prototypes
** ============================================================================
*/

static phy_status_t phy_set_defaults( void );

/*
** ============================================================================
** Public Function Definitions
** ============================================================================
*/

//uint8_t Bytes = 0xFF;

/*Initialise PHY*/
void PHY_Init( uint8_t cold_start )
{	
        //uint32_t chan_freq = 0;
	if ( cold_start )
	{
              phy_pib.TRXState = PHY_DEV_ON;	
              phy_pib.pendingStateChange = PHY_INVALID_PARAMETER;
                
              phy_set_defaults();
	}
	else
	{
		if( !(are_radio_settings_intact() ))
               {
                
                  /*power recycle has happened*/
                  plme_set_phy_mode_request( );
                
               }
               
                
	}

}

/*****************************************************************************/

void PHY_Reset ( void )
{
//	TRX_Reset();
	
	phy_pib.TRXState = PHY_DEV_ON;
	phy_pib.pendingStateChange = PHY_INVALID_PARAMETER;
	TRX_On();
	phy_set_defaults();
	
}

/*****************************************************************************/

uint64_t get_time_now_64 (void);
uint64_t get_last_sfd_detect_time (void);
void radio_state_cleanup (void);
uint32_t phy_rx_busy_error_handeled = 0;
void dummy_packet_send_to_trx (void);

void handle_phy_busy_rx_error (void)
{

}

/*****************************************************************************/
uint8_t PHY_rcv_in_progress( void )
{
  handle_phy_busy_rx_error ();
  return ( plme_get_trx_state_request() == PHY_BUSY_RX )?1:0;
}

/*****************************************************************************/

uint8_t PHY_Get_Symbol_Rate( void )
{
	//return gp_phy_mode_config->symbol_duration;
  // Raka to do .. from RAIL 
  return 8;  //gp_global_phy_mode_config->symbol_duration;
}

/*****************************************************************************/

void plme_set_phy_mode_request( void )
{

    uint32_t k = 0; 
    phy_pib.pendingStateChange = PHY_INVALID_PARAMETER;
    phy_pib.MaxSUNChannelSupported = get_TRX_curr_PHY_total_channel_supported();							
   
    memset(phy_pib.SUNChannelsSupported,0x0,MAX_CHANNELS_CAPABILITY);				
    for (k = 0 ;k< phy_pib.MaxSUNChannelSupported; k++ )
      {
          phy_pib.SUNChannelsSupported[k/8] |= (1<<(k%8));
      }
               
	
}

/*
** ============================================================================
** Private Function Definitions
** ============================================================================
*/
/*Setting PHY default configuration*/
static phy_status_t phy_set_defaults( void )
{
	uint32_t i = 0,val = DEF_TX_POWER; //,j = 0; 
        uint8_t preamble_bytes = 0;
        
	
        phy_pib.MaxSUNChannelSupported = get_TRX_curr_PHY_total_channel_supported();
	/*set the read only paramaters */
	phy_pib.SymbolsPerOctet = SYMBOLS_PER_OCTATE; //p_phy_config->symbols_per_octet;        
        preamble_bytes = Get_Preamble_len_nibbles();  //Get_Preamble_len_nibbles()/2;	      
        phy_pib.SHRDuration = phy_pib.SymbolsPerOctet * ( preamble_bytes + aMRFSKSFDLength );    	
        phy_pib.MaxFrameDuration = phy_pib.SHRDuration +( (aMRFSKPHRLength + aMaxPHYPacketSize) * phy_pib.SymbolsPerOctet );  
        for (i = 0 ;i< phy_pib.MaxSUNChannelSupported; i++ )
        {
            phy_pib.SUNChannelsSupported[i/8] |= (1<<(i%8));
        }

        val = DEF_CHANNEL;
	PLME_set_request(phyCurrentChannel,2, &val);
	
	val = DEF_TX_POWER;
	PLME_set_request(phyTransmitPower,1,&val);
	
	val = DEF_CCA_MODE;
	PLME_set_request(phyCCAMode,1,&val); 
	
	phy_pib.CCADuration = aCCATime;

	val = DEF_CHANNEL_PAGE;
	PLME_set_request(phyCurrentPage,1,&val);
	
         val = DEF_SUNPAGE_VAL;
         PLME_set_request(phySUNPageEntriesSupported,4,&val);
         
         
	val = DEF_FSKFEC_SWITCH;
	PLME_set_request(phyFECEnabled,1,&val);

	val = DEF_FSKFEC_SCHEME;
	PLME_set_request(phyFSKFECScheme,1,&val);
	
	val = DEF_FSKFEC_INTERLEAVING;
	PLME_set_request(phyFSKFECInterleaving,1,&val);
	
	val = DEF_MRFSKSFD;
	PLME_set_request(phyMRFSKSFD,1,&val);
	
	val = preamble_bytes;
	PLME_set_request(phyFSKPreambleRepetitions,2,&val);
	
	val = DEF_FSKSCRAMBLE;
	PLME_set_request(phyFSKScramblePSDU,1,&val);
	
        // Raka need to check if -80 is saved  ??? debug 7-sept-2016
        // This is not getting set...
	val = (uint32_t)DEF_RSSI_THRESHOLD;
	PLME_set_request(phyRSSIThreshold,1,&val);
	
	return PHY_SUCCESS;
}

/*****************************************************************************/

uint32_t phy_get_total_symbols_txed(void)
{
    return TRX_Total_Symbols_Txed();

}

/*****************************************************************************/
void phy_reset_total_symbols_txed(void)
{/*Resetting total symbol transmitted*/
    TRX_Reset_Total_Symbols_Txed();
    return;
}	

/*****************************************************************************/
uint32_t Get_Chan_Center_Freq0( void )
{/*Return channel center freqency*/
    return  1; //gp_operating_phy_mode->chan_center_freq0;	
}

/*****************************************************************************/
uint16_t Get_Channel_Spacing_kHz( void )
{/*Return channel spcing in KHz*/
    return 1 ; //gp_global_phy_mode_config->channel_spacing_kHz;	
}

/*****************************************************************************/
uint8_t Get_Preamble_len_nibbles(void )
{
  /* Raka :  This function returns the number of bytes
              Initiaaly this function was suppose to return the number of nibbles.
              But it has been changed to return the number of Bytes . Please 
  refer Trx_Init () function in trx_access.c file
  */
  uint8_t num_nibbles = 8;
  

  
//  switch(gp_global_phy_mode_config->data_rate_kbps)
//  {
//    
//#if (PROFILE_CFG_WISUN_FAN == 1)     
//      case DR_50_KBPS:
//          num_nibbles = 8; 
//          break;
//      case DR_100_KBPS:
//         num_nibbles = 8; 
//          break;
//      case DR_150_KBPS:
//         num_nibbles = 12; 
//          break;
//      case DR_200_KBPS:
//          num_nibbles = 12; 
//          break;
//      case DR_300_KBPS:
//           num_nibbles = 24; 
//          break;
//        
//#elif (PROFILE_CFG_WISUN_ENET_INTERFACE == 1)  
//     case DR_50_KBPS:
//           num_nibbles = 8; 
//           break;
//           
//    case DR_100_KBPS:
//           num_nibbles = 15; 
//           break;
//         
//#else
//       
//      case DR_50_KBPS:
//          num_nibbles = 8; 
//          break;
//      case DR_100_KBPS:
//        num_nibbles = 8; 
//          break; 
//      case DR_150_KBPS:
//         num_nibbles = 12; 
//          break;               
//      case DR_200_KBPS:
//         num_nibbles = 12; 
//          break;
//      case DR_300_KBPS:
//           num_nibbles = 24; 
//          break;
//      
//#endif
//  default:
//    
//     num_nibbles = 8; 
//    break;
//    
//  }
  
  
  return num_nibbles;
}

/*****************************************************************************/

uint8_t get_max_supported_channel_on_phy_selected()
{/*It rturn MAX no of channel supported by selcted PHY Mode*/
  // Raka  to do with RAIL Config
   return 15 ; //gp_operating_phy_mode->num_of_channels;
}
/*****************************************************************************/


