/** \file plme.c
 *******************************************************************************
 ** \brief
 ** Implements the PHY Layer management features.
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
#include "sm.h"
//#include "fec.h"
#include "phy.h"

/*
** ============================================================================
** Private Macro definitions
** ============================================================================
*/


/*
** ============================================================================
** Private Structures, Unions & enums Type Definitions
** ============================================================================
*/

typedef struct attribute_info_t
{
	uint8_t id;
	uint16_t length;
	void* value;
	uint32_t min_value;
	uint32_t max_value;
} attribute_info_t;

/*
** ============================================================================
** Public Variable Definitions
** ============================================================================
*/

volatile uint8_t for_cca = 0;
int8_t peak_rssi_value = 0;
phy_pib_t phy_pib;

/*
** ============================================================================
** Private Variable Definitions
** ============================================================================
*/


static attribute_info_t phy_pib_attrib_info[ TOTAL_PHY_PIBS + 1] =
{
    { phyCurrentChannel,           	2, (uint8_t*) &phy_pib.CurrentChannel, 0, 65535 },
    { phyTransmitPower,                 1, (uint8_t*) &phy_pib.TransmitPower, 0, 0xFFFFFFFF },
    { phyCCAMode,               	1, (uint8_t*) &phy_pib.CCAMode, 1, 3 },
    { phyCurrentPage,        		1, (uint8_t*) &phy_pib.CurrentPage, 9, 9 },
    { phyMaxFrameDuration,              READ_ONLY|2, (uint8_t*) &phy_pib.MaxFrameDuration, 0, 0 },
    { phySHRDuration,       		READ_ONLY|1, (uint8_t*) &phy_pib.SHRDuration, 0, 0 },
    { phySymbolsPerOctet,               READ_ONLY|1, (uint8_t*) &phy_pib.SymbolsPerOctet, 0, 0 },
    { phyCCADuration,              	READ_ONLY|2, (uint8_t*) &phy_pib.CCADuration, 8, 1000 },  
    { phyCurrentSUNPageEntry,           4, (uint8_t*) &phy_pib.CurrentSUNPageEntry, 0, 0xFFFFFFFF },
    { phyFSKFECScheme,      		1, (uint8_t*) &phy_pib.FSKFECScheme, 0, 1 },
    { phyFSKFECInterleaving,            1, (uint8_t*) &phy_pib.FSKFECInterleaving, 0, 1 },
    { phyMaxSUNChannelSupported,        READ_ONLY|2, (uint8_t*) &phy_pib.MaxSUNChannelSupported, 0, 65535},
    { phyMRFSKSFD,                      1, (uint8_t*) &phy_pib.MRFSKSFD, 0, 1 },
    { phyNumSUNPageEntriesSupported,    1, (uint8_t*) &phy_pib.NumSUNPageEntriesSupported, 1, MAX_SUN_PAGE_ENTRIES_CAPABILITY },
    { phySUNChannelsSupported,          READ_ONLY|8, (uint8_t*) &phy_pib.SUNChannelsSupported, 0xFFFFFFFF , 0xFFFFFFFF },
    { phySUNPageEntriesSupported,       4,(uint8_t*) phy_pib.SUNPageEntriesSupported, 0, 0xFFFFFFFF },
    { phyFSKPreambleRepetitions,        2, (uint8_t*) &phy_pib.FSKPreambleRepetitions, 4, 1000 },
    { phyFSKScramblePSDU,               1, (uint8_t*) &phy_pib.FSKScramblePSDU, 0, 1 },
    { phyFECEnabled, 		        1, (uint8_t*) &phy_pib.FECEnabled,  0, 1 },    
    { PhyTRXState,                      1, (uint8_t*) &phy_pib.TRXState, 0, 0 },
    { phySUNPageEntriesIndex,           1, (uint8_t*) &phy_pib.SUNPageEntryIndex,0,MAX_SUN_PAGE_ENTRIES_CAPABILITY-1 },
    { phyExplicit_Channel_plan,         1, (uint8_t*) &phy_pib.explicit_channel_plan,0,1},
    { phyExplicit_canter_freq,          4, (uint8_t*) &phy_pib.explicit_center_ch_freq,0,0xFFFFFFFF},
    { phyExplicit_total_ch_number,      1, (uint8_t*) &phy_pib.total_explicit_number_of_channel,0,0x81},
    { 0,0,0 }
};


/*
** ============================================================================
** External Variable Declarations
** ============================================================================
*/

#if 0  // Raka : 07-Sept-2016 .. Changing to universal PHY Config...
extern phy_mode_config_t* gp_phy_mode_config;
extern phy_band_config_t phy_band_config[];
#endif

#ifdef RF_DEBUG
extern signed char rssival[];
extern uint16_t rssival_index;
#endif // #ifdef RF_DEBUG

/*
** ============================================================================
** Extern Function Prototypes
** ============================================================================
*/
extern void R_INTC1_Start(void);
extern void R_INTC1_Stop(void);
//extern uint8_t TRX_Config_CRC_for_RX_pkt();
//int Disable_RX_SPORT(void);
phy_status_t phy_prepare_channel_list( uint16_t channel );
/*
** ============================================================================
** Private Function Prototypes
** ============================================================================
*/

static bool phy_is_channel_supported( uint16_t chan_num );
static phy_status_t phy_validate_attribute
		( 
			uint8_t action, 
			uint8_t attrib_id, 
			uint16_t* size,
			uint8_t* p_index 
		);
static phy_status_t phy_set_transmit_power( int8_t pwr_dbm );
static phy_status_t phy_set_cca_mode( uint8_t cca_mode );
static phy_status_t phy_set_sun_page_entry(uint32_t sun_current_page, bool set_current );
static phy_status_t phy_validate_sun_page_entry( uint32_t sun_page );
//static phy_status_t phy_cca_sample_and_evaluate( void );

/*
** ============================================================================
** Public Function Prototypes
** ============================================================================
*/

int8_t phy_get_transmit_power( void );
void restore_trx_state( phy_status_t  state );
phy_status_t backup_trx_state( void );

/*
** ============================================================================
** Public Function Definitions
** ============================================================================
*/

/*Obtain channel freq from channel no*/
uint32_t get_channel_freq(uint16_t channel_num)
{
	//phy_mode_config_t* p_phy_config = gp_phy_mode_config;
	uint32_t chanFreq = 1000 ;
	//chanFreq = chanFreq * ( gp_operating_phy_mode->chan_center_freq0 + ( channel_num * gp_global_phy_mode_config->channel_spacing_kHz));
	return chanFreq;
}

/*****************************************************************************/

/*setting phy channel based on channel*/
phy_status_t phy_set_current_channel( uint16_t channel )
{
    phy_status_t  original_state = PHY_SUCCESS;
//    uint32_t chanFreq = 0;
   // phy_mode_config_t* p_phy_config = gp_phy_mode_config;
//    uint32_t delay = 200;

    if( (channel >= phy_pib.MaxSUNChannelSupported) 
    || ( !phy_is_channel_supported( channel ) ))
    {
    	/*MaxSUNChannelSupported holds the number of channels calculated using calculate_mrfsk()*/
    	return PHY_INVALID_PARAMETER;
    }
    else
    {
	   	original_state = backup_trx_state();
		if(( original_state != PHY_BUSY_RX ) && ( original_state != PHY_BUSY_TX  ))
		{
		    	phy_pib.CurrentChannel = channel;
		    	
		    	/* ChanCenterFreq = BandEdge+GL+(NumChan+1/2) * chanspacing */
		    	//chanFreq = gp_operating_phy_mode->chan_center_freq0 + ( channel * gp_global_phy_mode_config->channel_spacing_kHz);
		    	//TRX_Set_Channel_Frequency( chanFreq*1000 );		    	
		    	//while(--delay);
			//restore_trx_state( original_state );
                        
			PLME_Set_TRX_State( original_state ); 
		}
		else
		{
			return PHY_INVALID_PARAMETER;
		}

    }
    
    return PHY_SUCCESS;
}

/*******************************************/
/*setting channel frequency according explicit*/
phy_status_t phy_set_frequncy_on_explicit()
{
  phy_status_t  original_state = PHY_SUCCESS;
//  uint32_t chanFreq = 0;
//  uint32_t end_freq = 0;
  // phy_mode_config_t* p_phy_config = gp_phy_mode_config;
//  uint32_t delay = 200;
  
    original_state = backup_trx_state();
    if(( original_state != PHY_BUSY_RX ) && ( original_state != PHY_BUSY_TX  ))
    { 
      /*Suneet = For cal end freuency here total_explicit_number_of_channel is total avilable channel but in array index is start from 0
      example  = if total number channel is 64 then array index is (0 to 63)  thats why (phy_pib.total_explicit_number_of_channel - 1)
      */
//      end_freq = phy_pib.explicit_center_ch_freq + ((phy_pib.total_explicit_number_of_channel - 1)*gp_global_phy_mode_config->channel_spacing_kHz);
//      /* ChanCenterFreq = BandEdge+GL+(NumChan+1/2) * chanspacing */
//      chanFreq = phy_pib.explicit_center_ch_freq + ( phy_pib.CurrentChannel * gp_global_phy_mode_config->channel_spacing_kHz);
//      
//      if((chanFreq > end_freq) || (gp_operating_phy_mode->chan_end_freq < end_freq))
//      {
//        chanFreq = gp_operating_phy_mode->chan_center_freq0 + (0 * gp_global_phy_mode_config->channel_spacing_kHz);
//      }
//      
//      TRX_Set_Channel_Frequency( chanFreq*1000 );
//      
//      while(--delay);
//      //restore_trx_state( original_state );
//      PLME_Set_TRX_State( original_state ); 
    }
    else
    {
      return PHY_INVALID_PARAMETER;
    }
  return PHY_SUCCESS;
  
}


/*****************************************************************************/

phy_status_t phy_prepare_channel_list( uint16_t channel )
{
    
//  uint32_t chanFreq = 0;
  uint16_t channel_cnt = channel;
   
    if( (channel_cnt >= phy_pib.MaxSUNChannelSupported) 
    || ( !phy_is_channel_supported( channel_cnt ) ))
    {
    	/*MaxSUNChannelSupported holds the number of channels calculated using calculate_mrfsk()*/
    	return PHY_INVALID_PARAMETER;
    }
    else
    {
      
      for (channel_cnt = 0 ;channel_cnt <= phy_pib.MaxSUNChannelSupported; channel_cnt++)
      {
	      /* ChanCenterFreq = BandEdge+GL+(NumChan+1/2) * chanspacing */
              //chanFreq = gp_operating_phy_mode->chan_center_freq0 + ( channel_cnt * gp_global_phy_mode_config->channel_spacing_kHz);
              //TRX_calc_Channel_Frequency_list( chanFreq*1000, channel_cnt );
      }
          	

    }
    
    return PHY_SUCCESS;
}



/********************* RAka Change ends *************************************/

/*****************************************************************************/

int32_t get_rssi_min_threshold (void)
{
  return MIN_RSSI_THRESHOLD;
}

/*****************************************************************************/

int32_t get_rssi_max_threshold (void)
{
  return MAX_RSSI_THRESHOLD;
}

/*****************************************************************************/

phy_status_t PLME_set_request
			(
              uint8_t pib_attribute, 
              uint16_t size, 
              uint32_t *val 
            )
{ 
  phy_status_t status = PHY_SUCCESS;
  
  uint8_t attrib_info_index = 0;
  int8_t rssi_threshold = 0;
  bool init_fec_sm = false;
//  uint16_t fec_capability = 0x0000 ; //CONFIG_FEC_CAPABILITY;
  
  status = phy_validate_attribute( 0, pib_attribute, &size, &attrib_info_index );
  //chack_val=*val;
  if( status == PHY_SUCCESS )
  {
    uint32_t	min = phy_pib_attrib_info[attrib_info_index].min_value;
    uint32_t	max = phy_pib_attrib_info[attrib_info_index].max_value;
    
    /*check range and set the value*/
    if( ( min != 0x0 ) || ( max != 0x0 ) )
    {
      /*check if the value being set falls within the range or not*/
      if( ( *((uint32_t*)val) >= min ) && ( *((uint32_t*)val) <= max ) ) 
      {
        switch( pib_attribute )
        {
        case phyCurrentChannel:
          /* Sets the phyCurrentChannel and returns status as PHY_SUCCESS 
          if it is successful else returns PHY_INVALID_PARAMETER as the status */
          status = phy_set_current_channel(*((uint16_t*)val));
          break;
          
        case phyTransmitPower:
          /* Sets the phyTransmitPower and returns status as PHY_SUCCESS 
          if it is successful else returns PHY_INVALID_PARAMETER as the status*/
          status =  phy_set_transmit_power(*((int8_t*)val));
          break;
          
        case phyCCAMode:
          /* Sets the phyCCAMode and returns status as PHY_SUCCESS 
          if it is successful else returns PHY_INVALID_PARAMETER as the status*/
          status =  phy_set_cca_mode(*((uint8_t*)val));
          break;
          
        case phyCurrentPage:
          /* Breaks here if the value specified falls within the valid range 
          and Sets the phyCurrentPage */
          phy_pib.CurrentPage = *((uint8_t*)val);
          break;
          
        case phyMaxFrameDuration://will not hit here as this is read only
          break;
          
        case phySHRDuration://will not hit here as this is read only
          break;
          
        case phySymbolsPerOctet://will not hit here as this is read only
          break;
          
        case phyCCADuration:
          
          // phy_pib.CCADuration= *((uint16_t*)val);	
          //will not hit here as this is read only
          
          break;
          
        case phyCurrentSUNPageEntry:
          /* Sets the phyCurrentSUNPageEntry and returns status as PHY_SUCCESS 
          if it is successful else returns PHY_INVALID_PARAMETER as the status*/
          status = phy_set_sun_page_entry(*((uint32_t*)val), true);
          break;
          
        case phyFSKFECScheme:
//          /* Breaks here if the value specified falls within the valid range 
//          and Sets the phyFSKFECScheme */
//          if( *((uint8_t*)val) == 0x01 )
//          {
//            /*Attempt to enable RSC scheme. Check if the local node is capable and 
//            take actions accordingly */
//            if( fec_capability & FEC_RSC_CAPABLE )
//            {
//              if( phy_pib.FSKFECScheme != *((uint8_t*)val) )
//              {
//                phy_pib.FSKFECScheme = *((uint8_t*)val);
//                fec_sm.scheme = phy_pib.FSKFECScheme;
//                init_fec_sm = true;									
//              }
//            }
//            else
//            {
//              status = PHY_INVALID_PARAMETER;
//            }
//            
//          }
//          else
//          {
//            /*Attempt to enable NRRSC scheme. Check if the local node is capable and 
//            take actions accordingly */
//            if( fec_capability & FEC_NRNSC_CAPABLE )
//            {
//              if( phy_pib.FSKFECScheme != *((uint8_t*)val) )
//              {
//                phy_pib.FSKFECScheme = *((uint8_t*)val);
//                fec_sm.scheme = phy_pib.FSKFECScheme;
//                init_fec_sm = true;									
//              }
//            }
//            else
//            {
//              status = PHY_INVALID_PARAMETER;
//            }
//            
//          }
          
          break;
          
        case phyFSKFECInterleaving:
//          if( *((uint8_t*)val) == 0x01 )
//          {
//            /* Attempt to enable interleaving. Check if the local node has the 
//            capability */
//            if( fec_capability & FEC_INTERLEAVING )
//            {
//              if( phy_pib.FSKFECInterleaving != *((uint8_t*)val) )
//              {
//                /*since the FEC interleaving is being enabled, check if the FEC is enabled*/
//                if( phy_pib.FECEnabled )
//                {
//                  phy_pib.FSKFECInterleaving= *((uint8_t*)val);
//                  fec_sm.interleave = phy_pib.FSKFECInterleaving;
//                  init_fec_sm = true;
//                }
//                else
//                {
//                  status = PHY_INVALID_PARAMETER;
//                }
//              }							
//            }
//            else
//            {
//              status = PHY_INVALID_PARAMETER;
//            }
//          }
//          else
//          {
//            /* attempt to disable interleaving */
//            if( fec_capability & FEC_INTERLEAVING )
//            {
//              if( phy_pib.FSKFECInterleaving != *((uint8_t*)val) )
//              {
//                phy_pib.FSKFECInterleaving= *((uint8_t*)val);
//                fec_sm.interleave = phy_pib.FSKFECInterleaving;
//                init_fec_sm = true;
//              }							
//            }
//          }
          
          break;
          
        case phyMaxSUNChannelSupported: //will not hit here as this is read only
          break;
          
        case phyMRFSKSFD:
          /* Breaks here if the value specified falls within the valid range 
          and Sets the phyMRFSKSFD */
          phy_pib.MRFSKSFD= *((uint8_t*)val);
          break;
          
        case phyNumSUNPageEntriesSupported:
          /* Breaks here if the value specified falls within the valid range 
          and Sets the phyNumSUNPageEntriesSupported */
          phy_pib.NumSUNPageEntriesSupported= *((uint8_t*)val);
          break;
          
        case phySUNChannelsSupported://will not hit here as this is read only
          break;
          
        case phySUNPageEntriesSupported://setting of an array element
          status = phy_set_sun_page_entry( *((uint32_t*)val), false );
          break;
          
        case phyFSKPreambleRepetitions:
          /* Breaks here if the value specified falls within the valid range 
          and Sets the phyFSKPreambleRepetitions */
          phy_pib.FSKPreambleRepetitions= *((uint16_t*)val);
          
          phy_pib.SHRDuration =  
            (/*gp_phy_mode_config->symbols_per_octet*/ SYMBOLS_PER_OCTATE ) * ( phy_pib.FSKPreambleRepetitions + aMRFSKSFDLength );
          
          phy_pib.MaxFrameDuration = phy_pib.SHRDuration +
            ( (aMRFSKPHRLength + aMaxPHYPacketSize) * phy_pib.SymbolsPerOctet );
          
          break;
          
        case phyFSKScramblePSDU:
          /* Breaks here if the value specified falls within the valid range 
          and Sets the phyFSKScramblePSDU */
          phy_pib.FSKScramblePSDU = *((uint8_t*)val);
//          if(phy_pib.FSKScramblePSDU)
//          {
//            Enable_Data_Whitening();
//          }
//          else
//          {
//            Disable_Data_Whitening();
//          }
          break;
          
        case phyFECEnabled:
//          if( *((uint8_t*)val) == 0x01 )
//          {
//            if( (fec_capability & ( FEC_NRNSC_CAPABLE | FEC_RSC_CAPABLE )))
//            {
//              /* FEC capability exists */
//              /* Attempt to enable FEC */
//              if( phy_pib.FECEnabled != *((uint8_t*)val))
//              {
//                phy_pib.FECEnabled = *((uint8_t*)val);
//                fec_sm.enable = phy_pib.FECEnabled;
//                init_fec_sm = true;
//              }
//            }
//            else
//            {
//              /*trying to enable FEC when the FEC feature is not available*/
//              status = PHY_INVALID_PARAMETER;
//            }
//          }
//          else
//          {
//            if( (fec_capability & ( FEC_NRNSC_CAPABLE | FEC_RSC_CAPABLE )))
//            {
//              if( phy_pib.FECEnabled != *((uint8_t*)val))
//              {
//                phy_pib.FECEnabled = *((uint8_t*)val);
//                fec_sm.enable = phy_pib.FECEnabled;
//                init_fec_sm = true;
//              }
//            }
//          }
          
          break;
          
        case phySUNPageEntriesIndex:
          /* Breaks here if the value specified falls within the valid range 
          and Sets the phySUNPageEntriesIndex */
          phy_pib.SUNPageEntryIndex = *((uint8_t*)val);
          break;
          
        case phyExplicit_Channel_plan:
          phy_pib.explicit_channel_plan = *((uint8_t*)val);
            break;
          
        case phyExplicit_canter_freq: 
          phy_pib.explicit_center_ch_freq = *val;
            break;
          
        case phyExplicit_total_ch_number:
          phy_pib.total_explicit_number_of_channel = *((uint8_t*)val);
            break;
          
        default:
          break; 
        }
      }
      else
      {
        status = PHY_INVALID_PARAMETER;
      }
      
    }
    
  }
  else if ( status == PHY_UNSUPPORTED_ATTRIBUTE ) 
  {
    if ( pib_attribute == phyRSSIThreshold )
    {
      rssi_threshold = *((int8_t*)val);
      
      /* check if the value being set falls within the range or not and check the size */
      if(  ( rssi_threshold >= MIN_RSSI_THRESHOLD )  && ( rssi_threshold <= MAX_RSSI_THRESHOLD )  && ( size == 1 ) )
      {     
        phy_pib.RSSIThreshold = rssi_threshold;
        status = PHY_SUCCESS;
      }
      else
      {
        status = PHY_INVALID_PARAMETER;
      }
    }
    else if (pib_attribute == phyRSSIOffset  )
    {
      rssi_threshold = *((uint8_t*)val);
      /* check if the value being set falls within the range or not and check the size */
      if(  ( rssi_threshold >= MIN_RSSI_OFFSET )  && ( rssi_threshold <= MAX_RSSI_OFFSET )  && ( size == 1 ) )
      {
        phy_pib.RSSIOffset = rssi_threshold;
        status = PHY_SUCCESS;
      }
      else
      {
        status = PHY_INVALID_PARAMETER;
      }
    }
  }
  
  if( ( status ==  PHY_SUCCESS ) && init_fec_sm )
  {
    //Disable_RX_SPORT();
    
    //TRX_Set_RX_Buffer();
//    reset_sports_driver();
  }
  
  return status;
}

/*****************************************************************************/

phy_status_t PLME_get_request
			(
				uint8_t pib_attribute,
				uint16_t *length, 
				uint32_t *value 
            )
{

    phy_status_t status = PHY_SUCCESS;
    uint8_t attrib_info_index;
    

    status = phy_validate_attribute( 1, pib_attribute, length, &attrib_info_index );
    if( status == PHY_SUCCESS )
    {
    	*length = (( phy_pib_attrib_info[attrib_info_index].length) & ( ~READ_ONLY ));
    	
    	if( phyTransmitPower == pib_attribute)
    	{
    		*value = (uint32_t)phy_get_transmit_power();
       	}
    	else if( phySUNPageEntriesSupported == pib_attribute )
    	{
    		memcpy
	    	(
		    	(uint8_t*)value,
		    	(uint8_t*)&(((uint32_t*)(phy_pib_attrib_info[attrib_info_index].value))[phy_pib.SUNPageEntryIndex]), 
		    	*length
	    	);
    	}
    	else
    	{
	    	memcpy
	    	(
		    	(uint8_t*)value,
		    	(uint8_t*)phy_pib_attrib_info[attrib_info_index].value,
		    	*length 
	    	);
    		
    	}
    	
    }
    else if( ( status == PHY_UNSUPPORTED_ATTRIBUTE ) )
    {
      if(  pib_attribute == phyRSSIThreshold )
      {
          *value = phy_pib.RSSIThreshold;
          *length = sizeof( phy_pib.RSSIThreshold ); 
      }
      else if ( pib_attribute == phyRSSIOffset )
      {
          *value = phy_pib.RSSIOffset;
          *length = sizeof( phy_pib.RSSIOffset );
      }
       status = PHY_SUCCESS; 
    	
    }

    return status;
}

/*****************************************************************************/

phy_status_t PLME_Set_TRX_State( phy_status_t state )
{
     phy_status_t status = phy_pib.TRXState;;
     //irq_state_t flags = irq_disable();

	switch( state )
	{
		case PHY_RX_ON:
		    switch( phy_pib.TRXState )
		    {
					
				/* Breaks because TRX is already configured to PHY_RX_ON state.*/
				case PHY_BUSY_RX://fallthrough					
		    		//case PHY_RX_ON:	
					
					break;
				case PHY_BUSY_TX:
					/*change the state when the TX is complete*/
					phy_pib.pendingStateChange = PHY_RX_ON;
					break;
					
				case PHY_TRX_OFF://fallthrough
						
				case PHY_TX_ON://fallthrough
				  			
		    	/* Changes transceiver state to PHY_RX_ON if it is in any undesired state */	
		    	default:
		    		//TRX_Set_RX_Buffer();
//                                TRX_Config_CRC_for_RX_pkt( );
		    		TRX_Rx_On();
		        	phy_pib.TRXState = PHY_RX_ON;
		        	status = PHY_SUCCESS;
				
		    		break;	
		    }
		    break;

		case PHY_TRX_OFF://fallthrough
		case PHY_DEV_ON:
		    switch( phy_pib.TRXState )
		    {
		    	case PHY_BUSY_RX:
		    		/* change the state when the RX is complete */

		    	case PHY_BUSY_TX:
		    		/* change the state when the TX is complete */
		    		phy_pib.pendingStateChange = PHY_TRX_OFF;
		    		break;

		    	/* Breaks because transceiver is already configured to TRX_OFF state */	
		    	case PHY_TRX_OFF:
		    	case PHY_DEV_ON:
				
		    		break;
		    	case PHY_RX_ON://fallthrough
		    			
		    	case PHY_TX_ON://fallthrough
		    	/* Changes transceiver state to PHY_TRX_OFF if it is in any undesired state */		
		    	default:
		    		TRX_On();
		        	phy_pib.TRXState = PHY_TRX_OFF;
		        	status = PHY_SUCCESS;
				
		    		break;	
		    }
		        
		    break;
		        
		case PHY_TX_ON:
		    /* If this primitive is issued with TX_ON, the PHY will cause the PHY to 
		    go to the TX_ON state irrespective of the state the PHY is in */
		    	
		    switch( phy_pib.TRXState )
		    {
				case PHY_TX_ON:
				
		    		break;
		    				
		    		/* Breaks because transceiver is already configured to TX_ON state */	
				case PHY_BUSY_TX://fallthrough
					/*need to reset the SPORT driver which might be busy transmitting a frame */
				case PHY_BUSY_RX:
					/*need to reset the SPORT driver which might be busy capturing a frame */
		    			
		
			    	case PHY_RX_ON://fallthrough
			    	case PHY_TRX_OFF:
					/*change the transceiver to TX_ON state irrespective of the state the PHY is in */	    	         	 
			    	default: 
			    	/* need to reset the SPORT driver which might be busy either transmitting or receiving a frame */  	         
			        	TRX_TX_On();
			        	phy_pib.TRXState = PHY_TX_ON;
			        	status = PHY_SUCCESS;
					
			        	break;
		    }
		    break;
		    
		/*change the transceiver to TRX_OFF state irrespective of the state the PHY is in */	   
		case PHY_FORCE_TRX_OFF:
		    
		    /* If this primitive is issued with FORCE_TRX_OFF, the PHY will cause the PHY to 
		    go to the TRX_OFF state irrespective of the state the PHY is in */
		    	
		    switch( phy_pib.TRXState )
		    {
				case PHY_TX_ON://fallthrough	    				
		    		/* Breaks because transceiver is already configured to TX_ON state */	
				case PHY_BUSY_TX://fallthrough
					/*need to reset the SPORT driver which might be busy transmitting a frame */
				case PHY_BUSY_RX://fallthrough
					/*need to reset the SPORT driver which might be busy capturing a frame */
		    			
		
			    	case PHY_RX_ON://fallthrough
			    	case PHY_TRX_OFF://fallthrough
					/*change the transceiver to TX_ON state irrespective of the state the PHY is in */	    	         	 
			    	default: 
			    	/* need to reset the SPORT driver which might be busy either transmitting or receiving a frame */  	         
			        	TRX_On();
			        	phy_pib.TRXState = PHY_TRX_OFF;
			        	status = PHY_SUCCESS;
					
			        	break;
		    }
		    break;
		    	
		default:
		   	break;
	}
		
    //irq_enable(flags);
    return status;
}



/*****************************************************************************/

phy_status_t plme_get_trx_state_request( void )
{
    return phy_pib.TRXState;
}

/*****************************************************************************/

phy_status_t phy_ed_on( void *p )
{ 
	phy_status_t status = PHY_SUCCESS;
        
//#if (PRINT_DEBUG_LEVEL==ERROR_DEBUG)
//        stack_print_debug ("K phy state = %d\n", plme_get_trx_state_request ());
//#endif 
        
	
//	irq_state_t flags;
//
//        flags = irq_disable();
       //R_INTC1_Stop();
	
#ifdef RF_DEBUG	
	memset(rssival,0,1000);
	rssival_index = 0;
#endif

#ifdef USE_RSSI_METHOD_2
	/*use this along with RSSI method2*/
	PLME_Set_TRX_State( PHY_DEV_ON );//TRX_On();
#else
	//PLME_Set_TRX_State( PHY_RX_ON );//TRX_Rx_On();
	
	status = PLME_Set_TRX_State( PHY_RX_ON );
	if( ( status == PHY_RX_ON ) || ( status == PHY_SUCCESS ) )
	{
		//if( status == PHY_RX_ON )
		{
			for_cca = 1;
			TRX_Rx_On();
			status = PHY_SUCCESS;
		}
			
		/*indicate the RF layer about the intention of behind making the RX on. It is for ED and not for packet reception. In this state the RF should ignore all teh packates benig received*/
		
		TRX_ED_On();
		
//		irq_enable(flags);
		return status;
	}

#endif

    //for_cca = 0;
    //R_INTC1_Start();
    //PMK1 = 0;//enable radio ints
    //R_INTC1_Start();
//    irq_enable(flags);
        
//#if (PRINT_DEBUG_LEVEL==ERROR_DEBUG)
//        stack_print_debug ("L phy state = %d\n", plme_get_trx_state_request ());
//#endif         
        
    return status;
}

/*****************************************************************************/

phy_status_t phy_ed_off( void *p )
{ 
	phy_status_t status = PHY_SUCCESS;
//	 irq_state_t flags;
//
//        flags = irq_disable();
	//R_INTC1_Start();
        
//#if (PRINT_DEBUG_LEVEL==ERROR_DEBUG)
//    stack_print_debug ("M phy state = %d\n", plme_get_trx_state_request ());
//#endif           
        
#ifdef USE_RSSI_METHOD_2
	/*use this along with RSSI method2*/
	PLME_Set_TRX_State( PHY_DEV_ON );//TRX_On();
#else
	//PLME_Set_TRX_State( PHY_RX_ON );//TRX_Rx_On();
	status = PLME_Set_TRX_State( PHY_DEV_ON );
	
	for_cca = 0;

	TRX_Stop_ED();
    	//PMK1 = 0;
	//R_INTC1_Start();

	#if(0)
	status = PLME_Set_TRX_State( PHY_DEV_ON );
	if( ( status == PHY_DEV_ON ) || ( status == PHY_TRX_OFF )  ||( status == PHY_SUCCESS ) )
	{
		/*indicate the RF layer about the intention of behnid making the RX on. It is for ED and not for packet reception. In this state the RF should ignore all teh packates benig received*/
		TRX_Stop_ED();
		status = PLME_Set_TRX_State( PHY_RX_ON );
	}
	#endif

#endif
//	irq_enable(flags);
        
//#if (PRINT_DEBUG_LEVEL==ERROR_DEBUG)
//    stack_print_debug ("N phy state = %d\n", plme_get_trx_state_request ());
//#endif           
        
	return status;
}


/*****************************************************************************/

        
//int8_t phy_ed_sample( void )
//{
//    signed char grssi;
////    TRX_Get_RSSI( &grssi );
//    return (int8_t)grssi;
//}

/*****************************************************************************/

phy_status_t PLME_ED_Request( uint8_t* p_ed_val )
{
    signed char rssi = 0;
    phy_status_t status = phy_pib.TRXState;
    
    
    switch( phy_pib.TRXState )
    {
		/* Receiver should be ON for the ED measurement.*/
	    case PHY_RX_ON:
	    
#ifndef USE_RSSI_METHOD_2	    
//	        TRX_Get_RSSI( &rssi );

	        *p_ed_val = convert_RSSI_To_ED((int8_t)rssi);
       
	        status = PHY_SUCCESS;
#endif	        
	        	
	        break;

	    case PHY_TRX_OFF://fallthrough
	    case PHY_TX_ON://fallthrough
	    case PHY_DEV_ON:
	    
#ifdef USE_RSSI_METHOD_2
			TRX_Get_RSSI( &rssi );
	        *p_ed_val = convert_RSSI_To_ED(rssi);
	        
	        status = PHY_SUCCESS;
#endif
	        break;
	        
	    default:
	    	break;
    }     
	
	return status;
}

/*****************************************************************************/


phy_status_t PLME_CCA_Request( void )
{
    phy_status_t status = phy_pib.TRXState;

    switch( phy_pib.TRXState )
    {
		
    	/* Receiver should be ON for the CCA.*/
#ifdef USE_RSSI_METHOD_2
		case PHY_RX_ON:
			break;
			
                case PHY_DEV_ON://fallthrough
                case PHY_TRX_OFF://fallthrough
#else 

		case PHY_DEV_ON://fallthrough
		case PHY_TRX_OFF:
			break;  	
	    

		case PHY_RX_ON:
#endif		
	    	//peak_rssi_value = -127;
                  peak_rssi_value = MIN_RSSI_THRESHOLD;
	    	
	    	/*call once the function as it takes more than 453 usecs in case we are 
	    	using method 3 or 147 usecs in case we are using Method 2 */

//	    	status = phy_cca_sample_and_evaluate();
	       
	        break;

	    /* Transceiver is in transmission mode returns status as PHY_BUSY */	
	    case PHY_TX_ON://fallthrough
	    case PHY_BUSY_RX:
			status = PHY_BUSY;
			break;
	        
	    default:
	    	break;
    }     
	
	return status;
}

phy_status_t phy_cca_on( void *p )
{
       //check assiged value is ok or not(is it impact on code return value) if USE_RSSI_METHOD_2 was defined
	phy_status_t status = PHY_SUCCESS; 

//#if (PRINT_DEBUG_LEVEL==ERROR_DEBUG)
//    stack_print_debug ("O phy state = %d\n", plme_get_trx_state_request ());
//#endif                                                  
                                               

#ifdef USE_RSSI_METHOD_2
	/*use this along with RSSI method2*/
	PLME_Set_TRX_State( PHY_DEV_ON );
#else
	status = PLME_Set_TRX_State( PHY_RX_ON );
	if( ( status == PHY_RX_ON ) || ( status == PHY_SUCCESS ) )
	{
		
		//If the RX was alredy ON then the API returns PHY_RX_ON. But the RX should 
		//be made ON explicitly for CCA with an option of continuous RX on. IN normal 
		//case the RX will be made On with an option that the radio should be moved 
		//to PHY ON state after a valid rx which should not be the case with the RX on done 
		//before performing the ED scan or CCA. IN these cases, besides doing "special RX ON", 
		//we also disable the radio interrupt line so that we do not get ant rx interrupts. 
		//Even if they are received with a packet while doing ED or CCA, they were supposed 
		//to be dropped so instead getting and dropping, disable the getting itself.
		if( status == PHY_RX_ON )
		{
			for_cca = 1;
			TRX_Rx_On();
			status = PHY_SUCCESS;
		}
		
		/*indicate the RF layer about the intention of behnid making the RX on. 
		It is for ED and not for packet reception. In this state the RF 
		should ignore all teh packates benig received*/
		
		TRX_ED_On();
		peak_rssi_value = MIN_RSSI_THRESHOLD;
		
		return status;
		
	}
#endif
        
//#if (PRINT_DEBUG_LEVEL==ERROR_DEBUG)
//    stack_print_debug ("P phy state = %d\n", plme_get_trx_state_request ());
//#endif           
        
    return status;
}

/*****************************************************************************/

//void phy_cca_sample( void )
//{
//    int8_t rssi_sample;
//
//    rssi_sample = phy_ed_sample();
//
//    /* store the sample if it is the highest so far */
//    if( rssi_sample > peak_rssi_value )
//    {
//        peak_rssi_value = rssi_sample;
//    }
//}

//static phy_status_t phy_cca_sample_and_evaluate( void )
//{
//    int8_t rssi_sample;
//
//    rssi_sample = phy_ed_sample();
//
//    /* store the sample if it is the highest so far */
//    if( rssi_sample > peak_rssi_value )
//    {
//        peak_rssi_value = rssi_sample;
//    }
//    
//    /* evaluate result */  
//    return ( ( peak_rssi_value < phy_pib.RSSIThreshold ) ? PHY_IDLE: PHY_BUSY );
//}


/*****************************************************************************/

phy_status_t phy_cca_stop( void )
{
    phy_status_t ret;

    // do not worry for successful DEV_ON as a result of TX busy or RX busy, 
    //This wont happen as the radio ints are disabled
	PLME_Set_TRX_State(PHY_DEV_ON);
    
	for_cca = 0;

	TRX_Stop_ED();
    /* evaluate result */
    if( peak_rssi_value < phy_pib.RSSIThreshold )
    {
        /* clear channel */
        ret = PHY_IDLE;
    }
    else
    {
        /* busy channel */
        ret = PHY_BUSY;
    }

    return ret;
}

/*****************************************************************************/



//void phy_reset_sports_driver( int id, void *data )
//{
//	if( plme_get_trx_state_request( ) == PHY_BUSY_RX )
//	{
//		sport_reset_counter++;
//		reset_sports_driver();
//		
//	}	
//}

//void reset_sports_driver( void )
//{
//	Disable_RX_SPORT();
//	phy_pib.pendingStateChange = PHY_INVALID_PARAMETER;
//	TRX_On();
//	phy_pib.TRXState = PHY_TRX_OFF;
//	TRX_Set_RX_Buffer();
//	PD_Data_Indication_cb( NULL, 1 );
//}


/*
** ============================================================================
** Private Function Definitions
** ============================================================================
*/

static phy_status_t phy_validate_attribute
		( 
			uint8_t action, 
			uint8_t attrib_id, 
			uint16_t* size,
			uint8_t* p_index 
		)
{
	uint8_t i = 0;
	phy_status_t status = PHY_UNSUPPORTED_ATTRIBUTE;
	
	static attribute_info_t* attrib_info = NULL;
	
	for(i = 0;i< (sizeof(phy_pib_attrib_info)/sizeof(attribute_info_t) ); i++ )
	{
		attrib_info = &phy_pib_attrib_info[i];
		
		if( attrib_id == attrib_info->id )
		{
			if( action == 0 )
			{
				if( attrib_info->length & READ_ONLY )
				{
					status = PHY_READ_ONLY;
					break;
				}
					
				if( ( attrib_info->length & (~( READ_ONLY )) ) != *size )
				{
					status = PHY_INVALID_PARAMETER;
					break;
				}
				
			}
			
			*p_index = i;
			
			status = PHY_SUCCESS;
			break;
	
		}
	}
	
	return status;
}

/*****************************************************************************/
/*
0	3	-18.5
1	7	-16.5
2	11	-14.5
3	15	-12.5
4	19	-10.5
5	23	-8.5
6	27	-6.5
7	31	-4.5
8	35	-2.5
9	39	-0.5
10	43	1.5
11	47	3.5
12	51	5.5
13	55	7.5
14	59	9.5
15	63	11.5

or


0	3	-20
1	7	-18
2	11	-16
3	15	-14
4	19	-12
5	23	-10
6	27	-8
7	31	-6
8	35	-4
9	39	-2
10	43	0
11	47	2
12	51	4
13	55	6
14	59	8
15	63	10

*/


static phy_status_t phy_set_transmit_power( int8_t pwr_dbm )
{
//    uint8_t pa_mcr_level = 0;
	if ( pwr_dbm ) //>= MIN_OUTPUT_POWER  && pwr_dbm <= MAX_OUTPUT_POWER )
	{
	    //uint8_t pa_level = ( pwr_dbm - MIN_OUTPUT_POWER ) / 2;
    
//	    TRX_Set_Output_Power( pwr_dbm, &pa_mcr_level );
	    
	    phy_pib.TransmitPower = ( phy_pib.TransmitPower & (~TX_POWER_BITS ) );
    
	    phy_pib.TransmitPower |= (uint8_t)( pwr_dbm & TX_POWER_BITS );
    
	    return PHY_SUCCESS;
			
	}
	else
	{
		return PHY_INVALID_PARAMETER;
	}
	
	
}

/*****************************************************************************/

int8_t phy_get_transmit_power( void )
{
    // mask the 2 msbs which indicate the tolerance
	int8_t pwr_dBm = ( phy_pib.TransmitPower & TX_POWER_BITS ); 
	
	if ( pwr_dBm & 0x20 )
	{
		/* if b5 is set it indicates that the dbm is negative, so set all the 
		msbs afer the b5 */
		pwr_dBm |= 0xC0;
	}
	
	return pwr_dBm;
}

/*****************************************************************************/

static bool phy_is_channel_supported( uint16_t chan_num )
{
	return ( phy_pib.SUNChannelsSupported[chan_num/8] & (1<<(chan_num%8)))?true:false;
}

/*****************************************************************************/

static phy_status_t phy_set_cca_mode( uint8_t cca_mode )
{
    if( cca_mode == DEF_CCA_MODE )
    {
        phy_pib.CCAMode = cca_mode;
        return PHY_SUCCESS;
    }
    else
    {
        return PHY_INVALID_PARAMETER;
    }
}

/*****************************************************************************/

static phy_status_t phy_set_sun_page_entry(uint32_t sun_page, bool set_current )
{
	int i =0;
	uint32_t* p_sun_page_entry = NULL;

	if ( (phy_validate_sun_page_entry( sun_page )) == PHY_SUCCESS )
	{
		/*it is a valid SUN page entry which can be set now. Check if the entry 
		already exist in the table*/
			
		for(;i<MAX_SUN_PAGE_ENTRIES_CAPABILITY; i++)
		{
			p_sun_page_entry = &phy_pib.SUNPageEntriesSupported[ i ];
			
			if( (sun_page & 0xFFFF0000 ) == ( *p_sun_page_entry & 0xFFFF0000 ))
			{
				if ( !set_current )
				{
					/* SUN Page Entries are being set*/
					if( sun_page & 0x0000FFFF )
					{
						/* phySUNPageEntriesSupported table attribute is being set */
						*p_sun_page_entry = sun_page;
						return PHY_SUCCESS;	
						
					}
					else
					{
						/*there is no point in storing a SUN Page entry which indicates that no 
						PHY modes are supported*/
						return PHY_INVALID_PARAMETER;
					}
				}
				else
				{
					/* phyCurrentSUNPageEntry is being set*/
					if( *p_sun_page_entry >= sun_page )
					{
						uint32_t modes = sun_page & 0x0000FFFF;
						
						if ( ( modes>0 ) && ((modes &(modes-1))==0) )
						{
							/*set the phyCurrentSUNPageEntry attribute if only one bit is set in 
							the page entr being set indicating which phy mode need to be activated */
							
							if( phy_pib.CurrentSUNPageEntry != sun_page )
							{
								phy_pib.CurrentSUNPageEntry = sun_page;
								plme_set_phy_mode_request();
							}
																					
							return PHY_SUCCESS;	
						}
						else
						{
							return PHY_INVALID_PARAMETER;	
						}
					}
					else
					{
						/*current page being set indicates that it supports a mode which is 
						not being indicated in the SUN Page Entry table*/
						return PHY_INVALID_PARAMETER;
					}
				}
			}
		}
		
		
		if ( !set_current )
		{
			/* not found in the table, so set it */
			if ( phy_pib.SUNPageEntryIndex < MAX_SUN_PAGE_ENTRIES_CAPABILITY )
			{
				phy_pib.SUNPageEntriesSupported[ phy_pib.SUNPageEntryIndex ] = 
											 sun_page;
											 								 
				phy_pib.SUNPageEntryIndex += 0x01;
				phy_pib.NumSUNPageEntriesSupported += 0x01; 
		
				return PHY_SUCCESS;	
			}
			else
			{	
				/*no room for setting this entry in the table*/
				return PHY_INVALID_PARAMETER;	
			}
			
		}
		else
		{
			/*there is no corresponding entry in the Table, so cannot set the currrent 
			SUN page entry */
			return PHY_INVALID_PARAMETER;
		}
		
	}
	
	return PHY_INVALID_PARAMETER;	
}

/*****************************************************************************/


static phy_status_t phy_validate_sun_page_entry( uint32_t sun_page )
{
//	uint32_t freq_band_id = 0;
//        uint8_t ext_freq_band_id = 0xFF; // Raka :: size of extended_freq_band_id_t in unit8_t
//	uint32_t i = 0;
//	uint32_t j = 0;
//	
//	uint32_t phy_mode = sun_page & OPERATING_PHY_MODE_MASK;
//        
//	/*validate the page number field */
//         if( ( ( sun_page &  PAGE_NUMBER_MASK ) == PAGE_NUMBER_9 )  &&
//		( validate_phy_band_id (sun_page )) &&
//		(( sun_page & MOD_BIT_MASK ) == MOD_FSK ) &&
//		( phy_mode >0  )		
//	)
//	{
//		freq_band_id = ( sun_page & FREQ_BAND_MASK ) >> FREQ_BAND_ID_SHIFT_VAL;
//		
//                if ( freq_band_id == 15)
//                  ext_freq_band_id = ( sun_page & EXT_FREQ_BAND_MASK ) >> EXT_FREQ_BAND_ID_SHIFT_VAL;
//                  
//		/*look through the supported phy bands configuration table if this freq band is supported*/
//		for(i=0;i<  MAX_REGULATORY_DOMAIN_SUPPORTED;i++)
//		{
//				if((regulatory_domain_cfg[i].frq_band_ID == freq_band_id ) && 
//                                   (regulatory_domain_cfg[i].extd_frq_band_ID == ext_freq_band_id ))
//				{	
//                                  uint32_t curr_phy_mode = 0;
//					for(j=0;j< regulatory_domain_cfg[i].max_phy_band_supported;j++) 
//					{
//                                                curr_phy_mode |= (phy_mode & regulatory_domain_cfg[i].phy_operating_mode_cfg[j].OperatingPHYMode);
//                                                
//						//if( phy_mode & regulatory_domain_cfg[i].phy_operating_mode_cfg[j].OperatingPHYMode)
//                                                
//                                                if( phy_mode == curr_phy_mode)
//						{
//							return PHY_SUCCESS;
//						}
//						
//					}	
//				}
//				
//		}						
//	}
//	
//	return PHY_INVALID_PARAMETER;
  
  return PHY_SUCCESS;
}

phy_status_t backup_trx_state( void)
{
	//phy_status_t status;
	phy_status_t  original_state = phy_pib.TRXState;
    
    if( ( original_state != PHY_BUSY_RX ) && ( original_state != PHY_BUSY_TX  ))
    {
	  (void)PLME_Set_TRX_State(PHY_DEV_ON); 
    }
    
    return original_state;
}

void restore_trx_state( phy_status_t  original_state )
{
	//phy_status_t status;
	
    if(( original_state != PHY_BUSY_RX ) && ( original_state != PHY_BUSY_TX  ))
    {
    	 (void)PLME_Set_TRX_State( original_state );
    }
}


uint8_t convert_RSSI_To_ED( int8_t rssi )
{
	uint8_t ed_val = 0x00;
	
        
//        if( (rssi)>(RX_SENSITIVITY_PHY_MODE_2+LIMIT_OVER_SENSITIVITY_FOR_0_ED_IN_dBm) )
//          
//	{
//	/* Ed value is rssi in dbm above, 10db above rx sensitivity times ED equivalent of 1 dbm RSSI*/
//	
//          ed_val = ( (rssi) - ( RX_SENSITIVITY_PHY_MODE_2 + LIMIT_OVER_SENSITIVITY_FOR_0_ED_IN_dBm )) * (MAX_ED_VAL/(TOTAL_RSSI_RANGE));
//	}

	return ed_val;
}

char convert_ED_To_RSSI( uint8_t ed_val )
{
//	uint16_t val = 	ed_val;
//	val *= TOTAL_RSSI_RANGE;//gp_phy_mode_config->total_rssi_range;
//	
//	return (val/ MAX_ED_VAL) + (/*gp_phy_mode_config->rx_sensitivity*/RX_SENSITIVITY_PHY_MODE_2 + LIMIT_OVER_SENSITIVITY_FOR_0_ED_IN_dBm);  
  
  return ed_val;
}


uint8_t convert_RSSI_To_LQI( int8_t rssi_val )
{
	uint8_t lqi = 0xFF;
        //int8_t rssi_int = (int8_t)rssi;
	
	if ( rssi_val < /*gp_phy_mode_config->rssi_ul*/ RSSI_UL_PHY_MODE_2)
	{		
		lqi = (( rssi_val - /*gp_phy_mode_config->rssi_ll*/RSSI_LL_PHY_MODE_2 ) * MAX_LQI_VAL)/(TOTAL_RSSI_RANGE/*gp_phy_mode_config->total_rssi_range*/);
	}

	return lqi;
}

uint8_t get_LQI_from_RSSI( int8_t rssi_val )
{
uint8_t i=0;
int16_t rssi_table [255]={-174,-173,-172,-171,-170,-169,-168,-167,-166,-165,-164,-163,-162,-161,-160,-159,-158,-157,-156,-155,-154,-153,
                          -152,-151,-150,-149,-148,-147,-146,-145,-144,-143,-142,-141,-140,-139,-138,-137,-136,-135,-134,-133,-132,-131,
                          -130,-129,-128,-127,-126,-125,-124,-123,-122,-121,-120,-119,-118,-117,-116,-115,-114,-113,-112,-111,-110,-109,
                          -108,-107,-106,-105,-104,-103,-102,-101,-100,-99,-98,-97,-96,-95,-94,-93,-92,-91,-90,-89,-88,-87,-86,-85,-84,
                          -83,-82,-81,-80,-79,-78,-77,-76,-75,-74,-73,-72,-71,-70,-69,-68,-67,-66,-65,-64,-63,-62,-61,-60,-59,-58,-57,
                          -56,-55,-54,-53,-52,-51,-50,-49,-48,-47,-46,-45,-44,-43,-42,-41,-40,-39,-38,-37,-36,-35,-34,-33,-32,-31,-30,
                          -29,-28,-27,-26,-25,-24,-23,-22,-21,-20,-19,-18,-17,-16,-15,-14,-13,-12,-11,-10,-9,-8,-7,-6,-5,-4,-3,-2,-1,0,
                          1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,
                          40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,
                          76,77,78,79,80};
	for(i=0;i<255;i++)
	{
		if(rssi_table[i]==rssi_val)
		{
				return (i+1);
		}
	}
        return 0xFF;
}


char convert_LQI_To_RSSI( uint8_t lqi )
{
	char rssi = /*gp_phy_mode_config->rssi_ul*/(char)RSSI_UL_PHY_MODE_2;
	uint16_t val = lqi;
	
	if ( lqi < MAX_LQI_VAL )
	{
		rssi = (( val * ( TOTAL_RSSI_RANGE /*gp_phy_mode_config->total_rssi_range*/ ) )/MAX_LQI_VAL ) + /*gp_phy_mode_config->rssi_ll*/RSSI_LL_PHY_MODE_2;
	}
	
	return rssi;
}

void PLME_Get_CSM_Unit_Radio_Chan_List
( 
	uint8_t curr_chan,
	uint8_t* p_channel_cnt, 
	uint8_t* p_channels 
)
{
	//phy_mode_config_t* p_curr_phy = gp_phy_mode_config;
	
	//RAKA  .. TO DO AS per RAIL ....
  
  
//	uint32_t centerFreq = 0,upperFreq = 0,lowerFreq = 0;
//	uint32_t i = 0;
//	uint32_t channel_cnt = 0; 
//	uint32_t value = phy_pib.CurrentSUNPageEntry;
//	uint32_t* unit_radio_channel_list = NULL;
//	
//
//        if( ( freq_band_id_t )( ( value & FREQ_BAND_MASK ) >> 22 ) == FREQ_BAND_ID_920_928_JAPAN  )
//        {
//// #if defined(CFG_PHY_BAND_950_958)           
//          unit_radio_channel_list = CSM920MHzUnitRadioChannelFreqs;
////#endif          
//        }
//// #if defined(CFG_PHY_BAND_950_958)        
////        else
////        {
////          unit_radio_channel_list = CSM950MHzUnitRadioChannelFreqs;
////        }
////#endif        
//	
//	if( ( value & OPERATING_PHY_MODE_MASK ) == COMMON_SIGNALLING_MODE )
//	{
//		*p_channel_cnt = 1;
//		p_channels[i] = curr_chan;
//		
//	}
//	else
//	{							
//    	//centerFreq = p_curr_phy->chan_center_freq0 + ( curr_chan * p_curr_phy->channel_spacing_kHz);
////          lowerFreq = centerFreq - p_curr_phy->channel_spacing_kHz/2;
////        upperFreq = centerFreq + p_curr_phy->channel_spacing_kHz/2;
//        
//        centerFreq = gp_operating_phy_mode->chan_center_freq0 + ( curr_chan * gp_global_phy_mode_config->channel_spacing_kHz);
//        lowerFreq = centerFreq - gp_global_phy_mode_config->channel_spacing_kHz/2;
//        upperFreq = centerFreq + gp_global_phy_mode_config->channel_spacing_kHz/2;
//
//    	while( upperFreq >= unit_radio_channel_list[i] )
//    	{
//    		if( unit_radio_channel_list[i] >= lowerFreq )
//    		{
//    			p_channels[channel_cnt++] = i++;	
//    		}
//    		else
//    		{
//    			i++;
//    		}    		
//    	}
//		*p_channel_cnt = channel_cnt;
//	}	
}
