/** \file pd_si.c
 *******************************************************************************
 ** \brief Implements the PD-Data request primitive
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
#include "app_log.h"
/*
** ============================================================================
** Private Macro definitions
** ============================================================================
*/

#define PHR_FCS_LEN_BIT_RESET				0x0000
#define PHR_FCS_LEN_BIT_SET			        0x0008
#define PHR_DATA_WHITENING_ENABLED			0x0010
#define PHR_DATA_WHITENING_DISABLED			0x0000
#define MAX_RX_INTERVAL                                 ( (2047 * 32 * HWTIMER_SYMBOL_LENGTH ) + 1000 )

/*
** ============================================================================
** Private Structures, Unions & enums Type Definitions
** ============================================================================
*/

/*None*/

/*
** ============================================================================
** Public Variable Definitions
** ============================================================================
*/

void swapbits(uint8_t* data);
//static uint8_t bBitReverse(uint8_t bDataByte);
extern void * bm_alloc(    uint8_t *pHeap,    uint16_t length );
/*
** ============================================================================
** Private Variable Definitions
** ============================================================================
*/

static SHR_t shr = { SHR_TYPE_MR_FSK_SHR, { { 0x55, 0, 0x7209, 0x00 } } };
static void* pd_handle;
static uint32_t num_pkts_tx_fail = 0;
static uint16_t pendingStateChange_ctr = 0;


/*
** ============================================================================
** External Variable Declarations
** ============================================================================
*/
extern uint32_t total_symbols_transmitted;
extern phy_pib_t phy_pib;
extern uint8_t heap[];
extern uint8_t dev_state;
/*
** ============================================================================
** External Function Prototypes
** ============================================================================
*/

extern void mem_rev_cpy(uint8_t* dest, uint8_t* src, uint16_t len );
extern uint8_t get_LQI_from_RSSI( int8_t rssi_val );
extern void * app_bm_alloc( uint16_t length );
extern void app_bm_free(uint8_t *pMem);

/*
** ============================================================================
** Public Function Prototypes
** ============================================================================
*/

void handle_sync_detec_event( void );
void handle_tx_done_event( uint8_t status, uint32_t tx_ts );
void handle_rx_pending_event( uint16_t psdu_len,uint8_t fcs_error );
void process_packet_received(uint8_t* bRssi, uint16_t* p_psdulen, uint8_t* p_psdu );

/*
** ============================================================================
** Private Function Prototypes
** ============================================================================
*/

static uint16_t create_Normal_MR_FSK_PHR(       bool scramble_psdu,
						bool crc_16,
						uint16_t* p_psdu_len);

void set_cca_required(uint8_t cca_bit);
uint8_t get_cca_required();

/*
** ============================================================================
** Public Function Definitions
** ============================================================================
*/

uint8_t get_PHY_current_state()
{
  
  return phy_pib.TRXState;
}

uint16_t get_PHY_current_channel()
{
  
  return phy_pib.CurrentChannel;  
}

uint8_t get_cca_required()
{
  return phy_pib.CCAMode; 
}


void set_cca_required(uint8_t cca_bit)
{
  phy_pib.CCAMode = cca_bit;
}

void PD_Data_Request( void* pHandle, phy_tx_t* pTxpkt )
{	
    uint16_t phr = 0;
    phy_status_t status  = PHY_SUCCESS;
    uint32_t channel = 0;
    uint16_t psdu_len = 0;

    uint32_t sym_per_octet = 0;
    uint16_t attrib_data_len = 0;        

    pd_handle = pHandle;
      
    switch ( phy_pib.TRXState )
    {
        case PHY_DEV_ON:
        case PHY_BUSY_TX:        
        case PHY_RX_ON:
        case PHY_TRX_OFF:
            /* Sends confirmation with an error status w.r.t TRXState */
            status  = phy_pib.TRXState;
            //PD_Data_Confirmation_cb( pd_handle,status );
            break;

        case PHY_TX_ON:
            channel = pTxpkt->TxChannel;

            psdu_len = pTxpkt->psduLength;

            if(!psdu_len)
            {
              status = PHY_INVALID_PARAMETER;
              break;
            }
            
            if( phy_pib.CurrentChannel != pTxpkt->TxChannel )
            {
                            /* Set the channel with the value present in the TX_Channel param*/
              status = PLME_set_request( phyCurrentChannel,2,&channel );
            }
            if(phy_pib.explicit_channel_plan)
            {
              status = phy_set_frequncy_on_explicit();
            }

            if( status != PHY_SUCCESS )
            {
              status = UNSUPPORTED_TX_CHANNEL;
              break;
            }

            if( ( pTxpkt->PPDUCoding ) && (!(phy_pib.FECEnabled)) )
            {
              status = UNSUPPORTED_PPDU_FEC;
              break;
            }

            if( pTxpkt->ModeSwitch )
            {
              status = UNSUPPORTED_MODE_SWITCH;        	
              break;
            }

            // i = phy_pib.MRFSKSFD;

            /*TBD: need to test for different SFDs. Currently using SFD corresponding to phyMRFSKSFD=0 and uncoded for both coded and uncoded PPDUs*/

            //shr.shr_info.mr_fsk_shr.sfd = 
            //( pTxpkt->PPDUCoding )?codedSFD[ i ]: uncodedSFD[ i ];

            //		        shr.shr_info.mr_fsk_shr.sfd = uncodedSFD[ 0 ];
            //			
            //				shr.shr_info.mr_fsk_shr.fec_coding = pTxpkt->PPDUCoding;
            //			
            //				if ( shr.shr_info.mr_fsk_shr.fec_coding )
            //				{
            //					shr.shr_info.mr_fsk_shr.fec_coding |= ( phy_pib.FSKFECScheme<<1 );
            //					shr.shr_info.mr_fsk_shr.fec_coding |= ( phy_pib.FSKFECInterleaving<<2 );
            //				}
            //			
            //				shr.shr_info.mr_fsk_shr.preamble_rep = phy_pib.FSKPreambleRepetitions;

            phr = create_Normal_MR_FSK_PHR
                                        (
                                        phy_pib.FSKScramblePSDU,
                                        ( pTxpkt->FCSLength )?false:true,
                                        &( psdu_len ) // excluding the FCS
                                        );

            PLME_get_request( phySymbolsPerOctet, &attrib_data_len, &sym_per_octet );

            //total_symbols_transmitted += psdu_len * sym_per_octet;
            total_symbols_transmitted +=  phy_tx_duration_calculate( psdu_len + (( pTxpkt->FCSLength )?4:2) );

            /* update the phy status */
            phy_pib.TRXState  = PHY_BUSY_TX;

            /*for(int psdui = 0; psdui<psdu_len;)
            {
            pTxpkt->psdu[psdui] = psdui;
            psdui++;
            }*/
    
            if( 
                FAILURE == TRX_Write
                (
                    &shr,
                    phr, 
                    pTxpkt->psdu,  							
                    psdu_len // exluding teh FCS length
                )
            )
            {
                /*should not happen*/
                num_pkts_tx_fail += 0x01;
                status = PHY_HW_ERROR;
                phy_pib.TRXState  = PHY_TX_ON;
                total_symbols_transmitted -=  phy_tx_duration_calculate( psdu_len + (( pTxpkt->FCSLength )?4:2) );
            }
            break;
        
        default:
            break;
    }
    
    if( status != PHY_SUCCESS )
    {
        PD_Data_Confirmation_cb( pd_handle,status,0 );	
    }
}


uint8_t TRX_Write_error_handling ( void );

void dummy_packet_send_to_trx (void)
{
//  SHR_t shr = {SHR_TYPE_MR_FSK_SHR, {{0x11, 0, 0x1234, 0x00}}};
//  uint16_t phr = 0x1122;
//  uint8_t dummy_data[2] = {1, 2};
  
  TRX_Write_error_handling ( );
}

/******************************************************************************/

uint32_t phy_tx_duration_calculate( uint16_t length )
{
	uint8_t multiplier = (( phy_pib.FECEnabled )? 2:1);
	return (phy_pib.SHRDuration + ( ( length + aMRFSKPHRLength ) * (phy_pib.SymbolsPerOctet) * multiplier ) );
}

/******************************************************************************/

uint32_t phy_total_symbols_transmitted(void)
{/*Returns the Total no of symbols transmited*/
	return total_symbols_transmitted;
}

/******************************************************************************/

phy_status_t phy_reset_symbols_transmitted(void)
{/*Reset the Total no of symbols transmited*/
	total_symbols_transmitted = 0;
	return PHY_SUCCESS;
}


/******************************************************************************/

void trx_packet_rx_cb( void )
{
	TRX_On();
	phy_pib.TRXState = PHY_DEV_ON;
}

/******************************************************************************/

void trx_packet_sent_cb( void )
{
	phy_pib.TRXState = PHY_TX_ON;
	
	
}

/******************************************************************************/

/*
** ============================================================================
** Private Function Definitions
** ============================================================================
*/

static uint16_t create_Normal_MR_FSK_PHR
					(
						bool scramble_psdu,
						bool crc_16,
						uint16_t* p_psdu_len // excluding FCS
					)
{
uint16_t phr;
	
	phr = *p_psdu_len; 
        
        if( crc_16 )
        {
           phr += 2;
        }
        else
        {
           phr += 4;
        }
        
	
	//phr(b15 to b5) = *p_psdu_len(b0 to b10)

	phr = (phr & 0x00FF) << 8 | (phr & 0xFF00) >> 8;
	phr = (phr & 0x0F0F) << 4 | (phr & 0xF0F0) >> 4;
	phr = (phr & 0x3333) << 2 | (phr & 0xCCCC) >> 2;
	phr = (phr & 0x5555) << 1 | (phr & 0xAAAA) >> 1;

	/*set the FCS len bit in the PHR*/
	phr |= ( crc_16 )?PHR_FCS_LEN_BIT_SET:PHR_FCS_LEN_BIT_RESET; //0 :32-bit, 1 :16 -bit
	
	/*set the Data whitening bit accordingly */
	phr |= ( scramble_psdu)?PHR_DATA_WHITENING_ENABLED:PHR_DATA_WHITENING_DISABLED;
	
	return phr;
}	
 
/******************************************************************************/

void handle_tx_done_event( uint8_t status, uint32_t tx_ts )
{
    phy_pib.TRXState  = PHY_TX_ON;
//    if( status == PHY_SUCCESS)
//    {
//      status = PHY_SUCCESS;
//    }
//    else
//    {
//      status = PHY_SUCCESS;
//    }

    if( phy_pib.pendingStateChange == PHY_RX_ON  )
    {
      TRX_Rx_On();
      phy_pib.TRXState = PHY_RX_ON;
      pendingStateChange_ctr++;
      phy_pib.pendingStateChange = PHY_INVALID_PARAMETER;
    }
    PD_Data_Confirmation_cb( pd_handle,status,tx_ts );      
}
/******************************************************************************/

void handle_sync_detec_event( void )
{
    phy_pib.TRXState  = PHY_BUSY_RX;
}

void phyPIB_state_cleanup (void)
{
  phy_pib.TRXState  = PHY_DEV_ON;
}

/******************************************************************************/

void handle_rx_pending_event( uint16_t psdu_len, uint8_t fcs_error )
{
    //int8_t temp_val = 0;
    phy_rx_t* p_rx_frame = NULL;
    phy_pib.TRXState = PHY_DEV_ON;
    //stop_radio_cleanup_timer ();
    if( fcs_error == 0 )
    {
      p_rx_frame = (phy_rx_t*)bm_alloc( heap, sizeof(phy_rx_t) +  psdu_len );
      
//#if (PRINT_DEBUG_LEVEL==ERROR_DEBUG)
//        stack_print_debug ("1M %d %p\n", sizeof(phy_rx_t)+psdu_len, p_rx_frame);
//#endif 

      /*since the radio has switched to ON state after the packet rx, 
      indicate the same in the PHY PIB*/
      if( p_rx_frame != NULL )
      {
        TRX_Read_RX_Buffer( (uint8_t*)p_rx_frame, NULL, psdu_len );
        p_rx_frame->channel = phy_pib.CurrentChannel;
        //temp_val = (int8_t)p_rx_frame->psduLinkQuality;
        //p_rx_frame->psduLinkQuality  = convert_RSSI_To_LQI(temp_val);
        //p_rx_frame->psduLinkQuality = get_LQI_from_RSSI(temp_val);
      }
    }
    else if(fcs_error == 1)
    {
      //memset_rx_buffer();//suneet :: when fcs error is true memset the rx_buffer 
      PLME_Set_TRX_State( PHY_RX_ON );
      return;
    }    
    PD_Data_Indication_cb( p_rx_frame, true );
}
/******************************************************************************/
void swapbits(uint8_t* data)
{
   *data = (*data & 0x0F) << 4 | (*data & 0xF0) >> 4;
   *data = (*data & 0x33) << 2 | (*data & 0xCC) >> 2;
   *data = (*data & 0x55) << 1 | (*data & 0xAA) >> 1;
}
/******************************************************************************/
//static  uint8_t bBitReverse(uint8_t bDataByte)
//{
//  bDataByte = (bDataByte & 0xF0) >> 4 | (bDataByte & 0x0F) << 4;
//  bDataByte = (bDataByte & 0xCC) >> 2 | (bDataByte & 0x33) << 2;
//  bDataByte = (bDataByte & 0xAA) >> 1 | (bDataByte & 0x55) << 1;
//  return bDataByte;
//}
/******************************************************************************/