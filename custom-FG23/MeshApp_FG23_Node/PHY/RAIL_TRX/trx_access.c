
/** \file trx_access.c
**
** \brief Functions for accessing the transeiver functionality
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
#include "em_core.h"
#include "hw_tmr.h"
#include "trx_access.h"
#include "phy.h"
#include "rail.h"
#include "rail_config.h"
#include "rail_types.h"
#include "rail_ieee802154.h"
#include "sli_rail_util_callbacks.h"
#include "pa_conversions_efr32.h"
#include "app_log.h"
#include "em_timer.h"
/*
** ============================================================================
** Private Macro definitions
** ============================================================================
*/




//#define RF_DEBUG

#ifdef RF_DEBUG
#define PRIVATE
#else
#define PRIVATE static
#endif

#ifdef MAX_FRAME_SIZE_CAPABILITY
#define MAX_PHY_FRAME_SIZE			MAX_FRAME_SIZE_CAPABILITY
#else
#define MAX_PHY_FRAME_SIZE			2047
#endif

#define MAX_PPDU_SIZE				MAX_PHY_FRAME_SIZE
#define RX_BUFFER_LEN				MAX_PPDU_SIZE + 8 


#define RX_FRAME_INFO_RESERVED_SIZE		24
#define MAX_TX_FIFIO_BUFF_SIZE                  512

typedef enum rf_packet_trx_state_tag
{
  RF_IDLE				= 0x00,
  RF_DETECTING_ENERGY			= 0x01,
  RF_PACKET_READY_TO_RX			= 0x02,
  RF_PACKET_SYNC_DETECTED		= 0x03,
  RF_PACKET_RX_UNDER_PROGRESS		= 0x04,
  RF_PACKET_RX_COMPLETE			= 0x05,
  RF_PACKET_TX_UNDER_PROGRESS		= 0x06,
  RF_PACKET_TX_COMPLETE			= 0x07,
  RF_PACKET_RX_ERROR			= 0x08
}rf_packet_trx_state_t; 

rf_packet_trx_state_t rf_pkt_trx_state = RF_IDLE;

p3time_t trx_complete_timestamp = 0;
typedef uint32_t ( *crc_engine_t )( uint8_t* inbuf, int32_t n, uint32_t* lfsr );
uint16_t gpsdulen = 0;
uint16_t wReadPointer = 0;
uint16_t wFifoReadCounter = 0;
uint16_t wReadBytesLeft = 0;

static uint16_t g_msg_len = 0;


#ifdef RF_DEBUG 
uint16_t rssi_problem = 0;
uint16_t tx_complete_irq_missed = 0;
uint16_t rx_is_off_while_rssi_read = 0;
uint16_t incorrect_packet_len_count = 0;
uint16_t incorrect_packet_len = 0;
uint16_t max_incorrect_packet_len = 0;
#endif //RF_DEBUG 

uint8_t fcs_error = 0;
uint32_t g_crc = 0;
crc_engine_t crc_engine = NULL;
uint8_t* gp_frame;
uint16_t gtx_frame_length = 0;
uint8_t gtx_is_crc_16 = 0;

//void RAILCb_RxPacketReceived(void);
#ifndef RF_DEBUG
volatile PRIVATE uint32_t sync_detect_irq = 0;
volatile PRIVATE uint32_t rx_irq_count = 0;
volatile PRIVATE uint32_t tx_irq_count = 0;
volatile PRIVATE uint32_t crc32_failures = 0;
volatile PRIVATE uint32_t crc16_failures = 0;
#else
volatile uint32_t sync_detect_irq = 0;
volatile uint32_t rx_irq_count = 0;
volatile uint32_t tx_irq_count = 0;
volatile uint32_t crc32_failures = 0;
volatile uint32_t crc16_failures = 0;
volatile uint32_t dont_care_packets = 0;
uint32_t tx_triggered = 0;
uint16_t RevB1A_workaround_misfired = 0;
#endif

volatile uint8_t tx_try_index = 0;
uint32_t g_center_freq;
uint8_t g_channel_num = 0;
uint32_t g_last_packet_rx_time_H = 0;
//volatile uint8_t exit_cont_mode = 0;
//PRIVATE uint8_t def_rssi_val;

uint8_t txBuffer[MAX_TX_FIFIO_BUFF_SIZE];
int currentConfig;
int reset=0;
uint16_t fifoSize;
RAIL_Handle_t app_init(void);
uint16_t rXdcount;

/*
** ============================================================================
** Private Structures, Unions & enums Type Definitions
** ============================================================================
*/

uint32_t return_rx_irq_count(void);   
uint32_t return_tx_irq_count(void); 

typedef struct rx_frame_info_tag
{
  uint8_t* link; 
  uint32_t sfd_rx_time;
  uint64_t rx_current_hw_time;	    /**<Packet current time at reception *///Suneet
  uint64_t complete_rx_hw_time;            /*Suneet :: this is when complete packet read */ 
  uint16_t psduLength;
  uint16_t FCSLength;
 // int32_t psduLinkQuality;//this actually holds RSSI and not LQI which gets converted to LQI in PD_si.c
  int8_t rssi;
  uint8_t lqi;
  uint32_t channel;
  uint8_t reserved[RX_FRAME_INFO_RESERVED_SIZE];
}rx_frame_info_t;

typedef struct crc_data_tag
{	
  uint8_t g_is_crc_16;
  uint16_t msg_len;
  uint32_t crc;
}crc_data_t;

typedef struct rx_data_tag
{	
  uint16_t wReadPointer;
  uint16_t wFifoReadCounter;
  uint16_t wReadBytesLeft;
}rx_data_t;

typedef struct modem_status_tag
{
  uint8_t modem_pend;
  uint8_t modem_status;
  uint8_t curr_rssi;
  uint8_t latch_rssi;
  uint8_t ant1_rssi;
  uint8_t ant2_rssi;
}modem_status_t;

/*
** ============================================================================
** Private Variable Definitions
** ============================================================================
*/


#ifdef RF_DEBUG

signed char rssival[10];
uint16_t rssival_index = 0;
signed char max_rssi = -127;
signed char min_rssi = 0;

#endif


#define APP_DEFAULT_MAC_PIB_CSMA_MIN_BE                 3       /* RAIL range 0 - 8 */
#define APP_DEFAULT_MAC_PIB_CSMA_MAX_BE                 5       /* RAIL range 0 - 8 (must be greater than APP_DEFAULT_MAC_PIB_CSMA_MIN_BE) */
#define APP_DEFAULT_MAC_PIB_CSMA_TRIES                  1
#define APP_DEFAULT_MAC_PIB_CCA_THRESHOLD               -80     /* dB */
#define APP_DEFAULT_MAC_PIB_CCA_CCABACKOFF              1160
#define APP_DEFAULT_MAC_PIB_CCA_SAMPLING_DURATION       160      /* micro seconds */
#define APP_DEFAULT_MAC_PIB_CSMATimeout                 0

/******************************************************/

rx_frame_info_t rx_frame_info;
//RX_BUFFER_LEN
uint8_t rx_buffer[ 1024 ];
#define HEADER_LENGTH 2
#define RAIL_FIFO_SIZE (128U)

#define RX_THRESHOLD  RAIL_FIFO_SIZE - RAIL_FIFO_SIZE / 10

/// Contains the status of RAIL Calibration
static volatile RAIL_Status_t calibration_status = 0;
static volatile bool cal_error = false;
/// Receive and Send FIFO
static __ALIGNED(RAIL_FIFO_ALIGNMENT) uint8_t rx_fifo[RAIL_FIFO_SIZE];

static __ALIGNED(RAIL_FIFO_ALIGNMENT) uint8_t tx_fifo[RAIL_FIFO_SIZE];

uint32_t total_symbols_transmitted = 0;
uint16_t wWritePointer = 0;
extern uint8_t get_cca_required();
/*
** ============================================================================
** Public Variable Definitions
** ============================================================================
*/

const RAIL_ChannelConfig_t *gCurrChannelConfig = NULL;

RAIL_Handle_t grailHandlePHY = NULL;

extern uint16_t get_PHY_current_channel();
extern uint8_t get_PHY_current_state();
RAIL_CsmaConfig_t csma_config; //= RAIL_CSMA_CONFIG;

/* None */

/*
** ============================================================================
** External Variable Declarations
** ============================================================================
*/

/*
** ============================================================================
** Private Function Prototypes
** ============================================================================
*/


/* Driver structures */
typedef enum {
    RADIO_UNINIT,
    RADIO_INITING,
    RADIO_IDLE,
    RADIO_TX,
    RADIO_RX,
    RADIO_CALIBRATION
} siliconlabs_modem_state_t;

static volatile siliconlabs_modem_state_t radio_state = RADIO_UNINIT;
static void RAILCb_RfReady(RAIL_Handle_t railHandle);

void RAILCb_radioEventHandler(RAIL_Handle_t railHandle, RAIL_Events_t events);
//void RAILCb_TxPacketSent(void);
//void storeReceivedPackets(RAIL_Handle_t grailHandlePHY);
static RAIL_Config_t railCfg = {
  .eventsCallback = &RAILCb_radioEventHandler,
};

void phy_goto_recei_mode();
RAIL_MultiTimer_t phy_busy_trx_tm;
uint32_t event_tx_underflow_cnt = 0;
uint8_t temp_reset = 0;
uint16_t cca_start = 0;
uint16_t cca_retry = 0;
uint16_t crc_failure=0;
 void APP_LED_ON ();
 void APP_LED_OFF ();
uint64_t get_time_now_64 (void);

static void verfiy_trx_state(struct RAIL_MultiTimer *tmr,
                            RAIL_Time_t expectedTimeOfEvent,
                            void *cbArg );
/*
** ============================================================================
** Public Function Definitions
** ============================================================================
*/

//====================== RAIL-defined callbacks =========================
/**
 * Callback that lets the app know when the radio has finished init
 * and is ready.
 */
static void RAILCb_RfReady(RAIL_Handle_t handle)
{
    (void) handle;
    radio_state = RADIO_IDLE;
}

//=========================================================================
void phy_goto_recei_mode()
{
  uint16_t currCh = 0;
  currCh = get_PHY_current_channel();
  RAIL_Idle(grailHandlePHY, RAIL_IDLE, true);
  RAIL_StartRx(grailHandlePHY,currCh, NULL);
  PLME_Set_TRX_State(PHY_RX_ON);
  rf_pkt_trx_state = RF_PACKET_READY_TO_RX;
}
//=========================================================================

uint8_t are_radio_settings_intact( void )
{
  return (g_center_freq)?1:0;
}


//=========================================================================

void TRX_WakeUp( uint8_t switch_rx_on, uint32_t chan_freq )
{
  

}

/******************************************************************************/

void TRX_RAIL_PHY_Reset ( void )
{
   // To Do For RUN time PHY mode change..
   RAIL_Idle(grailHandlePHY, RAIL_IDLE_FORCE_SHUTDOWN_CLEAR_FLAGS, true);
   radio_state = RADIO_IDLE;
    
}



/******************************************************************************/
int8_t TRX_RAIL_PHY_Mode_Change (uint8_t phyModeID)
{
  
  
  // To Do For RUN time PHY mode change..
  // Channel Config Selection Variable
uint8_t configIndex = 0;
// Channel Variable
uint16_t channel = 0;
  
  uint8_t proposedIndex = phyModeID;

  // Be sure that the proposed index is valid. Scan through all possible
  // indexes and check for the last NULL parameter since you can't
  // use sizeof on an extern-ed array without an explicit index.
  for (uint8_t i = 0; i <= proposedIndex; i++) {
    if (channelConfigs[i] == NULL) {
//      responsePrintError(sl_cli_get_command_string(args, 0), 0x11,
//                         "Invalid radio config index '%d'",
//                         proposedIndex);
      return -1;
    }
  }

  configIndex = proposedIndex;
  RAIL_Idle(grailHandlePHY, RAIL_IDLE, true);
  // Load the channel configuration for the specified index.
  channel = RAIL_ConfigChannels(grailHandlePHY,
                                channelConfigs[configIndex],
                                &sli_rail_util_on_channel_config_change);
//  responsePrint(sl_cli_get_command_string(args, 0), "configIndex:%d,firstAvailableChannel:%d",
//                configIndex,
//                channel);
 
   (void) RAIL_StartRx(grailHandlePHY, channel, NULL);
  
   return 0;
}

/******************************************************************************/

/*
 * \brief Function initialises and registers the RF driver.
 *
 * \param none
 *
 * \return rf_radio_driver_id Driver ID given by NET library
 */

void set_fifo_thresholds(RAIL_Handle_t rail_handle)
{
  RAIL_SetTxFifoThreshold(rail_handle, RAIL_FIFO_SIZE - RAIL_FIFO_SIZE / 10);
  RAIL_SetRxFifoThreshold(rail_handle, HEADER_LENGTH);
}

int8_t App_RAIL_rf_device_register(void)
{
    // If we already exist, bail.
    if (radio_state != RADIO_UNINIT) {
        return -1;
    }

    RAIL_StateTransitions_t txTransitions;
    RAIL_StateTransitions_t rxTransitions;
 // RAIL_TxPower_t txPowerDeciDbm;
  //RAIL_Status_t paError;
  //RAIL_Status_t status;
  
    grailHandlePHY = NULL;
    
//    const RAIL_ChannelConfig_t *channel_config = NULL;
    
    RAIL_DataConfig_t data_config = {
      .txSource = TX_PACKET_DATA,
      .rxSource = RX_PACKET_DATA,
      .txMethod = FIFO_MODE,
      .rxMethod = FIFO_MODE,
    };

    // Set up RAIL
    // Initialize the RAIL library and any internal state it requires
    grailHandlePHY = RAIL_Init(&railCfg, &RAILCb_RfReady);
    
     //grailHandlePHY = RAIL_Init(&railCfg, NULL);

    // RAIL data management configuration
    RAIL_ConfigData(grailHandlePHY, &data_config);
    
    gCurrChannelConfig = channelConfigs[ PHY_MODE_SELECT_ID ];
    
    //channel_config = channelConfigs[0];

    // Set up library for IEEE802.15.4 PHY operation
    RAIL_ConfigChannels(grailHandlePHY, gCurrChannelConfig, &sli_rail_util_on_channel_config_change);
    
    // Configure calibration settings
    RAIL_ConfigCal(grailHandlePHY, RAIL_CAL_ALL);
    
    // Fire all events by default
    RAIL_ConfigEvents(grailHandlePHY, RAIL_EVENTS_ALL, RAIL_EVENTS_ALL);

    txTransitions.success = RAIL_RF_STATE_RX;
    txTransitions.error = RAIL_RF_STATE_RX;
    rxTransitions.success = RAIL_RF_STATE_RX;
    rxTransitions.error = RAIL_RF_STATE_RX;
    RAIL_SetTxTransitions(grailHandlePHY, &txTransitions);
    RAIL_SetRxTransitions(grailHandlePHY, &rxTransitions);

    
    uint16_t rxFifoSize = RAIL_FIFO_SIZE;
    RAIL_Status_t status = RAIL_SetRxFifo(grailHandlePHY, &rx_fifo[0], &rxFifoSize);
    if (rxFifoSize != RAIL_FIFO_SIZE) {
      // We set up an incorrect FIFO size
      return RAIL_STATUS_INVALID_PARAMETER;
    }
    if (status == RAIL_STATUS_INVALID_STATE) {
      // Allow failures due to multiprotocol
      return RAIL_STATUS_NO_ERROR;
    }
    
    uint16_t allocated_tx_fifo_size = 0;
    allocated_tx_fifo_size = RAIL_SetTxFifo(grailHandlePHY, tx_fifo, 0, RAIL_FIFO_SIZE);
    
    set_fifo_thresholds(grailHandlePHY);
//    // Setup the transmit buffer
//    uint16_t fifoSize = RAIL_SetTxFifo(grailHandlePHY, txBuffer, 0, sizeof(txBuffer));
//    if (fifoSize != sizeof(txBuffer)) {
//        while (1) ;
//    }

    // Setup the recieve buffer
//    uint16_t rxFifoSize = sizeof(rx_buffer);
//    if (RAIL_SetRxFifo(grailHandlePHY, rx_buffer, &rxFifoSize) != RAIL_STATUS_NO_ERROR) {
//        while (1) ;
//    }
//      uint16_t fifo_rxSize = RAIL_GetTxFifoSpaceAvailable(grailHandlePHY);
//      RAIL_SetRxFifoThreshold(grailHandlePHY, fifo_rxSize / 2);
    // Raka TO Do .. Verify the PA configuration ..
    
//    if (RAIL_ConfigTxPower(grailHandlePHY, &paInitSubGhz) != RAIL_STATUS_NO_ERROR) {
//        // Error: The PA could not be initialized due to an improper configuration.
//        // Please ensure your configuration is valid for the selected part.
//        while (1) ;
//    }

    // Set the output power to the maximum supported by this chip
    RAIL_SetTxPower(grailHandlePHY, 255);

    RAIL_ConfigMultiTimer(1);
    return 0;
}

/******************************************************************************/

//uint8_t TRX_Init( uint32_t phy_mode, uint8_t** p_phy_mode_setings,  uint8_t dataRate )
//{
//
//  return SUCCESS;
//}

/******************************************************************************/

uint16_t get_TRX_curr_PHY_total_channel_supported ( void )
{
  uint16_t totalNoCh = gCurrChannelConfig->configs->channelNumberEnd;
  
  return ( totalNoCh+1);
}


/******************************************************************************/

uint8_t TRX_Write
(
SHR_t* p_shr_info,
uint16_t phr, 
uint8_t* pd_data,  // excludes the FCS field
uint16_t pd_length // exludes the FCS field len
)
{
 
  g_msg_len = pd_length;
  wWritePointer = 0;  
  gtx_is_crc_16 = ( phr & 0x0008 )? 1:0;
  gtx_frame_length = ( pd_length + aMRFSKPHRLength + (gtx_is_crc_16?2:4));//includes the FCS length also // 4;// Add 2 Octet PHR and 4 Octet CRC Length
  
//  if(radio_status == RAIL_RF_STATE_ACTIVE)
//  {
//    RAIL_Idle(grailHandlePHY, RAIL_IDLE, true);
//  }
//  
  
  RAIL_RadioState_t radio_status = RAIL_GetRadioState(grailHandlePHY);
  
  if(radio_status != RAIL_RF_STATE_RX)
  {
    return FAILURE;
  }
  //RAIL_ResetFifo(grailHandlePHY, false, true);
  
//    RakaTest[1] =  __RBIT((pd_length + 4)  & 0xFF) >> 24; // Creating phr as per 15.4
//    RakaTest[0]  = __RBIT((pd_length + 4)   >> 8) >> 24;   // Adding 4 Octet FCS Length
//    RakaTest[0] = txBuffer[0] | 0x10;//Setting the Data whitening bit to TRUE
    
    
  txBuffer[0]              =  (phr & 0xFF);
  txBuffer[1]              = ((phr & 0xFF00) >> 8 );
  
  memcpy ((uint8_t*)&txBuffer[2], (uint8_t*)pd_data, pd_length);
  
  
   //RAIL_Idle(grailHandlePHY, RAIL_IDLE, true);
   RAIL_TxOptions_t txOpt = RAIL_TX_OPTIONS_DEFAULT;
   
    if(txBuffer[2] & (1 << 5)) {
      //txOpt |= RAIL_TX_OPTION_WAIT_FOR_ACK;
//      waiting_for_ack = true;
    } else {
//      waiting_for_ack = false;
    }
  // Raka ...
  
  g_msg_len = gtx_frame_length;
  if( (gtx_frame_length )>= RAIL_FIFO_SIZE )
  {
    wWritePointer = RAIL_WriteTxFifo(grailHandlePHY,txBuffer,gtx_frame_length,true);
  }
  else
  {
    RAIL_WriteTxFifo(grailHandlePHY,txBuffer,gtx_frame_length,true);
    wWritePointer = 0;
    g_msg_len = 0;
  }
  
  uint16_t currCh = get_PHY_current_channel();
  
  
  if(get_cca_required() == 0x01)
  { 
    csma_config.csmaMinBoExp = APP_DEFAULT_MAC_PIB_CSMA_MIN_BE;
    csma_config.csmaMaxBoExp = APP_DEFAULT_MAC_PIB_CSMA_MAX_BE;
    csma_config.csmaTries = APP_DEFAULT_MAC_PIB_CSMA_TRIES;
    csma_config.ccaThreshold = APP_DEFAULT_MAC_PIB_CCA_THRESHOLD;
    csma_config.ccaBackoff = APP_DEFAULT_MAC_PIB_CCA_CCABACKOFF;
    csma_config.ccaDuration = APP_DEFAULT_MAC_PIB_CCA_SAMPLING_DURATION;
    csma_config.csmaTimeout = APP_DEFAULT_MAC_PIB_CSMATimeout;
    if(RAIL_StartCcaCsmaTx(grailHandlePHY, currCh, txOpt, &csma_config, NULL) != 0) 
    {
      phy_goto_recei_mode();
      //        RAIL_Idle(grailHandlePHY, RAIL_IDLE, true);
      //        RAIL_StartRx(grailHandlePHY, currCh, NULL);
      return FAILURE;
    }
    else
    {
      rf_pkt_trx_state = RF_PACKET_TX_UNDER_PROGRESS;
    }
  }
  else
  {
    if(RAIL_StartTx(grailHandlePHY,currCh,txOpt,NULL) != 0)
    {
      //        RAIL_Idle(grailHandlePHY, RAIL_IDLE, true);
      //        RAIL_StartRx(grailHandlePHY, currCh, NULL);
      phy_goto_recei_mode();
      return FAILURE;
    }
    else
    {
      rf_pkt_trx_state = RF_PACKET_TX_UNDER_PROGRESS;
    }
  }
  
 
  return SUCCESS;
}


static void verfiy_trx_state(struct RAIL_MultiTimer *tmr,
                            RAIL_Time_t expectedTimeOfEvent,
                            void *cbArg )
{
  if(get_PHY_current_state ()== PHY_BUSY_RX)
  {
      temp_reset++;
      wReadBytesLeft = 0;
      memset(rx_buffer,0,1024);
      phy_goto_recei_mode();
      RAIL_CancelMultiTimer(&phy_busy_trx_tm);  
  }
}

PRIVATE uint16_t read_phr( void )
{
  wReadPointer += RAIL_ReadRxFifo(grailHandlePHY,rx_buffer, HEADER_LENGTH);
  gpsdulen = __RBIT(rx_buffer[1]) >> 24;
  gpsdulen |= (__RBIT(rx_buffer[0]) >> 16) & 0x0700;
  /*Suneet :: return recived byte length with fcs lenth */
  return gpsdulen;
}

 
void RAILCb_radioEventHandler(RAIL_Handle_t railHandle, RAIL_Events_t events)
{
  uint16_t phr = 0;


  //uint16_t data = 0;
  if (rf_pkt_trx_state == RF_PACKET_TX_UNDER_PROGRESS)
  {
    tx_try_index++;
    if((events & RAIL_EVENT_TX_FIFO_ALMOST_EMPTY) == RAIL_EVENT_TX_FIFO_ALMOST_EMPTY)
    {
      
      if (wWritePointer < g_msg_len){
        //add rest of the packet
        wWritePointer += RAIL_WriteTxFifo(grailHandlePHY, txBuffer + wWritePointer,  g_msg_len - wWritePointer, false);
      }
    }
    
    if((events & RAIL_EVENT_TX_UNDERFLOW) == RAIL_EVENT_TX_UNDERFLOW)
    {
      trx_complete_timestamp = (uint32_t) hw_tmr_get_time_int( NULL );
      rf_pkt_trx_state = RF_IDLE;
      wWritePointer = 0;
      
      g_msg_len = 0;
      //gtx_is_crc_16 = 0;
      gtx_frame_length = 0;
      
      handle_tx_done_event(RAIL_PHY_TX_ERROR,trx_complete_timestamp);
      phy_goto_recei_mode();
    }
    if((events & RAIL_EVENT_TX_ABORTED) == RAIL_EVENT_TX_ABORTED)
    {
        trx_complete_timestamp = (uint32_t) hw_tmr_get_time_int( NULL );
      rf_pkt_trx_state = RF_IDLE;
      wWritePointer = 0;
      
      g_msg_len = 0;
      //gtx_is_crc_16 = 0;
      gtx_frame_length = 0;
      
      handle_tx_done_event(RAIL_PHY_TX_ERROR,trx_complete_timestamp);
      phy_goto_recei_mode();
    }
    if((events & RAIL_EVENT_TX_CHANNEL_BUSY) == RAIL_EVENT_TX_CHANNEL_BUSY)
    {
      trx_complete_timestamp = (uint32_t) hw_tmr_get_time_int( NULL );
      rf_pkt_trx_state = RF_IDLE;
      wWritePointer = 0;
      
      g_msg_len = 0;
      //gtx_is_crc_16 = 0;
      gtx_frame_length = 0;
      
      handle_tx_done_event(RAIL_PHY_TX_ERROR,trx_complete_timestamp);
      phy_goto_recei_mode();
    }
    
    
    if((events & RAIL_EVENT_TX_CCA_RETRY) == RAIL_EVENT_TX_CCA_RETRY)
    {
      cca_retry++;
    }
    
    if( (events & RAIL_EVENT_TX_START_CCA) == RAIL_EVENT_TX_START_CCA)
    {
      cca_start++;
    }
    
    if( (events & RAIL_EVENT_TX_PACKET_SENT) == RAIL_EVENT_TX_PACKET_SENT )
    {
      APP_LED_OFF();
      //        rf_int_debug_log[rf_int_indx++] = 0xA2;
      tx_irq_count++;
      tx_try_index = 0;
      trx_complete_timestamp = (uint32_t) hw_tmr_get_time_int( NULL );
      rf_pkt_trx_state = RF_IDLE;
      wWritePointer = 0;
      
      g_msg_len = 0;
      //gtx_is_crc_16 = 0;
      gtx_frame_length = 0;
      
      handle_tx_done_event(PHY_SUCCESS,trx_complete_timestamp);
      uint16_t currCh = get_PHY_current_channel();
      RAIL_StartRx(grailHandlePHY,currCh, NULL);
    }
    if(tx_try_index == 50)
    {
      tx_try_index = 0;
      
      //device stuck in tx_busy state clear tx_busy state if came;
      trx_complete_timestamp = (uint32_t) hw_tmr_get_time_int( NULL );
      rf_pkt_trx_state = RF_IDLE;
      wWritePointer = 0;
      
      g_msg_len = 0;
      //gtx_is_crc_16 = 0;
      gtx_frame_length = 0;
      
      handle_tx_done_event(RAIL_PHY_TX_ERROR,trx_complete_timestamp);
      phy_goto_recei_mode();
    }
  }
  else
  {
    /* process this only  if we have a recceive interrupt*/
    if( rf_pkt_trx_state == RF_PACKET_TX_UNDER_PROGRESS )
    {
      /*may be an ack was transmitted and we have this interrupt. Do not process this here*/
      return;
    }
    
    if((events & RAIL_EVENT_RX_SYNC1_DETECT) == RAIL_EVENT_RX_SYNC1_DETECT )
    {
      if( rf_pkt_trx_state == RF_PACKET_READY_TO_RX )
      { 
        rx_frame_info.sfd_rx_time = (uint32_t) hw_tmr_get_time_int( NULL );
        rx_frame_info.rx_current_hw_time = get_time_now_64();
        sync_detect_irq++;
        timer_2_init();
        printf("Rx Timer Started Packect Started Reciving\r\n");
        handle_sync_detec_event();
        rf_pkt_trx_state = RF_PACKET_SYNC_DETECTED;
        RAIL_SetMultiTimer(&phy_busy_trx_tm,1000000,RAIL_TIME_DELAY,&verfiy_trx_state,NULL);
      }
    }
    
    if( (events & RAIL_EVENT_RX_FIFO_ALMOST_FULL) == RAIL_EVENT_RX_FIFO_ALMOST_FULL )
    {
      if( rf_pkt_trx_state == RF_PACKET_SYNC_DETECTED  )
      {
        if(!wReadBytesLeft)
        {
          wReadBytesLeft = read_phr()+2;
          if(wReadBytesLeft > MAX_FRAME_SIZE_CAPABILITY)
          {
            rf_pkt_trx_state = RF_PACKET_RX_ERROR;
            goto end;
          }
          else
          {
            RAIL_SetRxFifoThreshold(railHandle, RAIL_FIFO_SIZE - RAIL_FIFO_SIZE / 10);
            phr = (uint16_t)(((unsigned) rx_buffer[1] << 8) |  rx_buffer[0] );
            rx_frame_info.FCSLength = ( phr & 0x0008 )?2:4;
            rx_frame_info.psduLength = gpsdulen;
            rf_pkt_trx_state = RF_PACKET_RX_UNDER_PROGRESS;
          }
        }
      }
      else if ( rf_pkt_trx_state == RF_PACKET_RX_UNDER_PROGRESS )
      {
        if(wReadBytesLeft >RX_THRESHOLD )
        {
          RAIL_ReadRxFifo(grailHandlePHY,rx_buffer+wReadPointer, RX_THRESHOLD);
          g_msg_len -= ((g_msg_len <= RX_THRESHOLD)?g_msg_len:RX_THRESHOLD);
          wReadPointer += RX_THRESHOLD;
          wFifoReadCounter++;                            // Update FIFO read counter
          wReadBytesLeft -= RX_THRESHOLD;
        }
        else
        {
          RAIL_ReadRxFifo(grailHandlePHY,rx_buffer+wReadPointer, wReadBytesLeft);
          g_msg_len -= ((g_msg_len <= wReadBytesLeft)?g_msg_len:wReadBytesLeft);
          wReadPointer += wReadBytesLeft;	
          wFifoReadCounter++;                            // Update FIFO read counter
          wReadBytesLeft -= wReadBytesLeft;
        }
        
        if (wReadBytesLeft == 0)
        {
          rf_pkt_trx_state = RF_PACKET_RX_COMPLETE;
          //change the PHY state from RX_BUSY to DEV_ON
          trx_packet_rx_cb();
          goto end;
        }
      }
    }
    
    if ( (events & RAIL_EVENT_RX_PACKET_RECEIVED) == RAIL_EVENT_RX_PACKET_RECEIVED )             // If PACKET RX IT arrived - this should never occur
    {
      if ( rf_pkt_trx_state == RF_PACKET_RX_UNDER_PROGRESS )
      {
        if( wReadBytesLeft )
        {
          
          RAIL_ReadRxFifo(grailHandlePHY,rx_buffer+wReadPointer, wReadBytesLeft);
          if( !wReadPointer )
          {
            
            /*wReadPointer holds 0 which indicates that the 
            last read is the first and last chunk of the frame 
            read from the radio */
            phr = (uint16_t)(((unsigned) rx_buffer[1] << 8) |  rx_buffer[0] );
            rx_frame_info.FCSLength = ( phr & 0x0008 )?2:4;
          
//            rx_frame_info.FCSLength = ( rx_buffer[1] & 0x10 )?2:4;
            
            rx_frame_info.psduLength = gpsdulen;
            g_msg_len = rx_frame_info.psduLength;
          }
          g_msg_len -= ((g_msg_len <= wReadBytesLeft)?g_msg_len:wReadBytesLeft);
          // Update poiinter of abReceivedPacket[] array 
          wReadPointer += wReadBytesLeft;											 
          // Update FIFO read counter
          wFifoReadCounter++;                            
          wReadBytesLeft -= wReadBytesLeft;
        }
        rf_pkt_trx_state = RF_PACKET_RX_COMPLETE;
        trx_packet_rx_cb();//change the PHY state from RX_BUSY to DEV_ON		
        goto end;
      }
      if( rf_pkt_trx_state == RF_PACKET_SYNC_DETECTED  )
      {
        RAIL_ReadRxFifo(grailHandlePHY,rx_buffer, RX_THRESHOLD);
        gpsdulen = __RBIT(rx_buffer[1]) >> 24;
        gpsdulen |= (__RBIT(rx_buffer[0]) >> 16) & 0x0700;
        phr = (uint16_t)(((unsigned) rx_buffer[1] << 8) |  rx_buffer[0] );
        rx_frame_info.FCSLength = ( phr & 0x0008 )?2:4;
//        rx_frame_info.FCSLength = ( rx_buffer[1] & 0x10 )?2:4;
        rx_frame_info.psduLength = gpsdulen;
        rf_pkt_trx_state = RF_PACKET_RX_COMPLETE;
        if(rf_pkt_trx_state==RF_PACKET_RX_COMPLETE)
        {
           TIMER_IntDisable(TIMER2, TIMER_IF_OF ); 
           printf("Rx Timer Stopped,Packet Recived\r\n");
        }
        trx_packet_rx_cb(); 
      }
    }
    
    if(events & RAIL_RX_PACKET_ABORT_CRC_ERROR == RAIL_RX_PACKET_ABORT_CRC_ERROR)
    {
      crc_failure++;
      rf_pkt_trx_state = RF_PACKET_RX_ERROR;
      goto end;
    }
    
    
//    if( events & RAIL_EVENT_RX_PACKET_RECEIVED == RAIL_EVENT_RX_PACKET_RECEIVED )
//    {
//      if( rf_pkt_trx_state == RF_PACKET_SYNC_DETECTED  )
//      {
//        //          
//        if( wReadBytesLeft )
//        {
//          // read 2 bytes of PHR also
//          RAIL_ReadRxFifo(grailHandlePHY,rx_buffer, wReadBytesLeft);
//          RAIL_DelayUs(5);
//          rx_frame_info.FCSLength = ( rx_buffer[0] & 0x10 )?2:4;
//          rx_frame_info.psduLength = gpsdulen;
//          g_msg_len = rx_frame_info.psduLength;
//          rf_pkt_trx_state = RF_PACKET_RX_COMPLETE;
//          //change the PHY state from RX_BUSY to DEV_ON
//          trx_packet_rx_cb();            
//        }
//      }
//      else if( rf_pkt_trx_state == RF_PACKET_RX_UNDER_PROGRESS )
//      {
//        //          rf_int_debug_log[rf_int_indx++] = 0xC9;
//        if( wReadBytesLeft )
//        {
//          //            rf_int_debug_log[rf_int_indx++] = 0xCA;
//          // read 2 bytes of PHR also
//          RAIL_ReadRxFifo(grailHandlePHY,rx_buffer+wReadPointer, wReadBytesLeft);
//          RAIL_DelayUs(5);
//          g_msg_len -= ((g_msg_len <= wReadBytesLeft)?g_msg_len:wReadBytesLeft);
//          wReadPointer += wReadBytesLeft;
//          // Update FIFO read counter
//          wFifoReadCounter++;                            
//          wReadBytesLeft -= wReadBytesLeft; 
//        }
//        rf_pkt_trx_state = RF_PACKET_RX_COMPLETE;
//        //change the PHY state from RX_BUSY to DEV_ON
//        trx_packet_rx_cb();	
//      }
//    }
    
    if(events & RAIL_EVENT_RX_PACKET_ABORTED == RAIL_EVENT_RX_PACKET_ABORTED)
    {
      rf_pkt_trx_state = RF_PACKET_RX_ERROR;
      goto end;
    }
    
    if(events & RAIL_EVENT_RX_FRAME_ERROR == RAIL_EVENT_RX_FRAME_ERROR)
    {
      rf_pkt_trx_state = RF_PACKET_RX_ERROR;
      goto end;
    }
    
    if(events & RAIL_EVENT_RX_FIFO_OVERFLOW == RAIL_EVENT_RX_FIFO_OVERFLOW)
    {
      rf_pkt_trx_state = RF_PACKET_RX_ERROR;
      goto end;
    }
  }
  
end:
  if( ( rf_pkt_trx_state == RF_PACKET_RX_COMPLETE ) || (rf_pkt_trx_state == RF_PACKET_RX_ERROR))
  {
    //      rf_int_debug_log[rf_int_indx++] = 0xCB;
    if(rf_pkt_trx_state == RF_PACKET_RX_COMPLETE)
    {
      RAIL_CancelMultiTimer(&phy_busy_trx_tm); 
      // Now all fields of rxInfo and rxDetails have been populated correctly
      RAIL_RxPacketDetails_t packetDetails = {
        .isAck = false,
        .timeReceived = {
          .timePosition = RAIL_PACKET_TIME_AT_PACKET_END,
        },
      };
      RAIL_GetRxPacketDetails(grailHandlePHY, RAIL_RX_PACKET_HANDLE_OLDEST_COMPLETE, &packetDetails);
      
      rx_frame_info.rssi = packetDetails.rssi;
      rx_frame_info.lqi = packetDetails.lqi;
      rx_frame_info.complete_rx_hw_time = packetDetails.timeReceived.packetTime;
      // if we are here it means that the CRC is correct and we do not have to worry about anything.
      rx_irq_count++;
      APP_LED_ON ();
      //fcs_error = TRX_Is_Frame_Corrupted(g_crc,g_msg_len);
      fcs_error = 0;      
    }
    else
    {
      
      //        rf_int_debug_log[rf_int_indx++] = 0xCC;
      fcs_error = 1;		
    }      
    
    rf_pkt_trx_state = RF_IDLE;
    g_msg_len = 0;
    wReadPointer = 0;
    wFifoReadCounter = 0;
    wReadBytesLeft = 0;		
    handle_rx_pending_event(rx_frame_info.psduLength,fcs_error);      
    fcs_error = 0;
  }  
  
}
/******************************************************************************/

uint8_t TRX_On( void )
{
  return SUCCESS;
}

/******************************************************************************/

uint8_t TRX_TX_On( void )
{
  TRX_On();
  return SUCCESS;
}

/******************************************************************************/
void TRX_ED_On( void )
{
  rf_pkt_trx_state = RF_DETECTING_ENERGY;
}

/******************************************************************************/

void TRX_Stop_ED( void )
{
  rf_pkt_trx_state = RF_IDLE;//RF_PACKET_READY_TO_RX;
}

/******************************************************************************/

uint8_t TRX_Rx_On( void )
{
  
  uint16_t currCh = get_PHY_current_channel();
  
  if ( RAIL_IsInitialized() )
    RAIL_StartRx(grailHandlePHY,currCh, NULL);
  
  rf_pkt_trx_state = RF_PACKET_READY_TO_RX;
  
  return SUCCESS;
}

/******************************************************************************/

uint8_t TRX_Off( void )
{
  return SUCCESS;
}

/******************************************************************************/
uint8_t TRX_Get_RSSI_Current( int8_t* pRSSIval )
{  
  *pRSSIval = (int8_t)(RAIL_GetRssi(grailHandlePHY,false) >> 2);  
  return SUCCESS;
}

/******************************************************************************/
//
//uint8_t TRX_Get_RSSI_Method_3( int* pRSSIval )
//{
//  return SUCCESS;
//}

/******************************************************************************/

uint32_t TRX_Total_Symbols_Txed(void)
{
  return total_symbols_transmitted;
}

/******************************************************************************/

void TRX_Reset_Total_Symbols_Txed(void)
{
  total_symbols_transmitted = 0;
}
/******************************************************************************/

//uint8_t TRX_Set_RX_Buffer( void )
//{
//  return SUCCESS;
//}

/******************************************************************************/

uint8_t TRX_Read_RX_Buffer(uint8_t* pd_data, uint8_t* p_rx_pkt, uint16_t pd_length)
{
  
  //bool is_data_whitened = false;
  
  if( pd_data != NULL )
  {
    // this is because at the RX, the packet size is being reported 
    //as 2 bytes extra possibly because of the FCS bit issue in the Radio. 
    //So subtract the total length by 2 to arrive at the correct length of 
    //the received packet.
    
    
    memcpy( pd_data, (uint8_t*)&rx_frame_info, (sizeof(rx_frame_info_t)-RX_FRAME_INFO_RESERVED_SIZE) );
    //sizeofrx_frame_info = sizeof( rx_frame_info_t );
    pd_data += (sizeof( rx_frame_info_t ));
    
    memcpy( pd_data , &(rx_buffer[2]),(rx_frame_info.psduLength) );
    
    memset((uint8_t*)&rx_frame_info, 0, (sizeof(rx_frame_info_t)-RX_FRAME_INFO_RESERVED_SIZE));
    
    // Raka :::: resetting the PHY Rx buffer to 0x00 
    // once we have copyied to pd_data
    memset(&rx_buffer[0],0x00, RX_BUFFER_LEN );
    return SUCCESS;
  }
  
  return FAILURE; 
}

/******************************************************************************/

//uint8_t TRX_Set_Output_Power( int8_t pwr_dbm, uint8_t* p_pa_level )
//{	
////  uint8_t index = ((pwr_dbm) - (MIN_OUTPUT_POWER) ); 
////  *p_pa_level = pa_level_lkp[index];
////  TRX_Set_PA_Level(pwr_dbm);
//  return SUCCESS;  
//}

/******************************************************************************/


//uint8_t TRX_Set_PA_Level( uint8_t pa_level )
//{
// // uint8_t pa_dbm= pa_level*10;
//  // RAIL_SetTxPowerDbm(grailHandlePHY,pa_dbm);
//  return SUCCESS;
//}

/******************************************************************************/

uint8_t TRX_RAIL_setPowerConfig(RAIL_TxPowerMode_t mode, uint16_t voltage ,  uint16_t rampTime)
{
  
  RAIL_TxPowerConfig_t *txPowerConfigPtr;
   txPowerConfigPtr = sl_rail_util_pa_get_tx_power_config_subghz();

  // Make a backup of the TX Power Config before it's changed.
  RAIL_TxPowerConfig_t txPowerConfigBackup = {
    .mode = txPowerConfigPtr->mode,
    .voltage = txPowerConfigPtr->voltage,
    .rampTime = txPowerConfigPtr->rampTime
  };

 
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();
  txPowerConfigPtr->mode = mode;
  txPowerConfigPtr->voltage = voltage;
  txPowerConfigPtr->rampTime = rampTime;
  CORE_EXIT_CRITICAL();

  RAIL_Status_t status = RAIL_ConfigTxPower(grailHandlePHY, txPowerConfigPtr);
 
  // Restore the backup TX Power Config on error.
  if (status != RAIL_STATUS_NO_ERROR) {
    CORE_DECLARE_IRQ_STATE;
    CORE_ENTER_CRITICAL();
    txPowerConfigPtr->mode = txPowerConfigBackup.mode;
    txPowerConfigPtr->voltage = txPowerConfigBackup.voltage;
    txPowerConfigPtr->rampTime = txPowerConfigBackup.rampTime;
    CORE_EXIT_CRITICAL();
  }

  if (status == RAIL_STATUS_NO_ERROR) {
   return SUCCESS;
  } else {
    return FAILURE;
  }
}

/******************************************************************************/

uint8_t TRX_RAIL_getPower(uint8_t *powerLevel, int16_t *power )
{
    
  *powerLevel = RAIL_GetTxPower(grailHandlePHY);
  *power = RAIL_GetTxPowerDbm(grailHandlePHY);
  
  return SUCCESS;
}

/******************************************************************************/

uint8_t TRX_RAIL_setPower(uint8_t powerDbmVal)
{

  bool setPowerError = false;  
  RAIL_TxPower_t powerDbm = (RAIL_TxPower_t)powerDbmVal;

//  if (sl_cli_get_argument_count(args) >= 2 && strcmp(sl_cli_get_argument_string(args, 1), "raw") == 0) {
//    RAIL_TxPowerLevel_t rawLevel = sl_cli_get_argument_uint8(args, 0);
//
//    // Set the power and update the RAW level global
//    if (RAIL_SetTxPower(railHandle, rawLevel) != RAIL_STATUS_NO_ERROR) {
//      setPowerError = true;
//    }
//  } else {
  
    RAIL_TxPowerConfig_t tempCfg;
    

    // Set the power in dBm and figure out what RAW level to store based on what
    // was requested NOT what is actually applied to the hardware after limits.
    
    if ((RAIL_SetTxPowerDbm(grailHandlePHY, powerDbm)
         != RAIL_STATUS_NO_ERROR)
        || (RAIL_GetTxPowerConfig(grailHandlePHY, &tempCfg)
            != RAIL_STATUS_NO_ERROR)) {
      setPowerError = true;
    }
  

  if (setPowerError) {
    return FAILURE;
  } else {
    // Get and print out the actual applied power and power level
   return SUCCESS;
  }
}

/******************************************************************************/


uint8_t TRX_Set_Channel_Frequency( uint32_t center_freq )
{
  uint32_t center_freq0 = Get_Chan_Center_Freq0();
  uint16_t chan_spacing_khz = Get_Channel_Spacing_kHz();
  
  if( !(( (center_freq/1000) - center_freq0 )% chan_spacing_khz ) )
  {
    g_channel_num = ( (center_freq/1000) - center_freq0 )/chan_spacing_khz;
  }
  else
  {
    //Raka :: logically you should not come here ..
    return FAILURE;
  }		
  return SUCCESS;
}

/******************************************************************************/
/*pass the freq deviation to be set in KHz*/
//uint8_t TRX_Set_Freq_Deviation( uint16_t freq_dev, mod_type_t mod_type )
//{
//  return 0;
//}

/******************************************************************************/

/*pass the data rate in bps*/
//uint8_t TRX_Set_Data_Rate( uint32_t data_rate, mod_type_t mod_type )
//{
//  return 0;
//}

/******************************************************************************/

//uint8_t TRX_Mod_Scheme( uint8_t mod_scheme )
//{
//  return SUCCESS;
//}

//int Disable_RX_SPORT(void)
//{
//  return SUCCESS;
//}

/******************************************************************************/

uint32_t TRX_get_last_packet_rx_time_H(void)
{
  return g_last_packet_rx_time_H;
}

/******************************************************************************/

//uint8_t TRX_Set_Mod_Source(RAIL_StreamMode_t mod_src )
//{
//  //RAIL_TxStreamStart(0,PN9_STREAM);
//  return SUCCESS;
//}

/******************************************************************************/

void TRX_Stop_Tx_Steram(void)
{
  
  RAIL_StopTxStream(grailHandlePHY);
  
}

/*****************************************************************************/

uint8_t TRX_Xmit_StreamModeOn( RAIL_StreamMode_t mod_src  )
{
  uint16_t channel = get_PHY_current_channel();
  
  if (mod_src  ==  RAIL_STREAM_CARRIER_WAVE)
    RAIL_StartTxStream(grailHandlePHY, channel, RAIL_STREAM_CARRIER_WAVE);
  else if (mod_src  ==  RAIL_STREAM_PN9_STREAM)
     RAIL_StartTxStream(grailHandlePHY, channel, RAIL_STREAM_PN9_STREAM);
  else if (mod_src  ==  RAIL_STREAM_10_STREAM)
     RAIL_StartTxStream(grailHandlePHY, channel, RAIL_STREAM_10_STREAM);
  else if (mod_src  ==  RAIL_STREAM_CARRIER_WAVE_PHASENOISE)
     RAIL_StartTxStream(grailHandlePHY, channel, RAIL_STREAM_CARRIER_WAVE_PHASENOISE);
  else if (mod_src  ==  RAIL_STREAM_RAMP_STREAM)
     RAIL_StartTxStream(grailHandlePHY, channel, RAIL_STREAM_RAMP_STREAM);
  else if (mod_src  ==  RAIL_STREAM_CARRIER_WAVE_SHIFTED)
     RAIL_StartTxStream(grailHandlePHY, channel, RAIL_STREAM_CARRIER_WAVE_SHIFTED);
  else 
     RAIL_StartTxStream(grailHandlePHY, channel, RAIL_STREAM_PN9_STREAM);
  
  return SUCCESS;
}

/*****************************************************************************/

//void set_cap_bank_val( uint8_t cap_bank )
//{
//  //RAIL_Status_t 	RAIL_SetTxPower (RAIL_Handle_t grailHandlePHY, RAIL_TxPowerLevel_t powerLevel)
//}

/*****************************************************************************/

//void TRX_Set_DW_Prop( uint8_t DW_enable )
//{
//  
//}

/******************************************************************************/

//uint8_t TRX_Config_CRC_for_RX_pkt( )
//{
//  
//  return 0;	
//}


/*****************************************************************************/
//
//void App_exit_continuous_mode( void )
//{
//  exit_cont_mode = 1;
//}

/*****************************************************************************/

//void App_enter_continuous_mode( void )
//{
//  exit_cont_mode = 0;
//}

/*****************************************************************************/

void get_trx_details(trx_details_t* p_trx_details, uint8_t reset_after_read )
{
  p_trx_details->pkt_tx_irq = tx_irq_count;
  p_trx_details->pkt_rx_irq = rx_irq_count;  
  p_trx_details->crc32_failures = crc32_failures;  
  p_trx_details->crc16_failures = crc16_failures;

  if( reset_after_read )
  {
    tx_irq_count = 0;
    rx_irq_count = 0;
    crc32_failures = 0;
    crc16_failures = 0;
  }
}

/*****************************************************************************/

//void configure_rssi_cal_val( uint8_t def_rssi )
//{
//  def_rssi_val = def_rssi;
//}
/******************************************************************************/
uint32_t return_rx_irq_count(void)
{
  return rx_irq_count;  // added for application - shubham
  
} 
/******************************************************************************/
uint32_t return_tx_irq_count(void)
{
  
  return tx_irq_count; // added for application - shubham  
} 


/******************************************************************************/

int chaneelConfigChange = 0;
void sl_rail_util_on_channel_config_change(RAIL_Handle_t rail_handle,
                                           const RAIL_ChannelConfigEntry_t *entry)
{
  (void) rail_handle;
  (void) entry;
  
  chaneelConfigChange++;
}
/******************************************************************************/
static volatile uint8_t EnergyDetectionCnt = 0;
uint32_t rfSensedEvent = 0;
static uint32_t rfUs = 100;
  
static void RAILCb_SensedRf(void)
{
  rfSensedEvent++;
  if (rfSensedEvent == 0) { // Wrap it to 1 not 0
    rfSensedEvent = 1;
  }
}

/******************************************************************************/

void TRX_RAIL_do_EnergyDetection_Scanning ()
{
  
  bool val = RAIL_SupportsRfSenseEnergyDetection(grailHandlePHY);
  
  if ( val == true )
    EnergyDetectionCnt++ ;
  
  (void) RAIL_StartRfSense(grailHandlePHY, RAIL_RFSENSE_OFF, 0, NULL);
  

  rfUs = RAIL_StartRfSense(grailHandlePHY, RAIL_RFSENSE_SUBGHZ_LOW_SENSITIVITY, rfUs, &RAILCb_SensedRf);
   
// uint32_t rfSensedEvent;
// void rfSensedCheck(void);
// RAIL_RFSENSE_MODE_ENERGY_DETECTION
    
  

}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

