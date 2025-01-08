/** \file trxsm.h
 *******************************************************************************
 ** \brief Provides APIs for the Transceiver State Machine 
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

#ifndef TRXSM_H
#define TRXSM_H

#ifdef __cplusplus
extern "C" {
#endif

/*
** ============================================================================
** Public Macro definitions
** ============================================================================
*/

#define TRXSM_PARAM_IDLE		0
#define TRXSM_PARAM_OFF			1

#if RAIL_TIMER_INTERFACE_USED
#include "rail_timer_interface.h"
#endif  
  
/*
** ============================================================================
** Public Structures, Unions & enums Type Definitions
** ============================================================================
*/

/**
 *******************************************************************************
 ** \struct trxsm_t
 ** Representation of the TRX State Machine
 *******************************************************************************
 **/
typedef struct trxsm_struct trxsm_t;

/**
 *******************************************************************************
 ** \struct trxsm_channel_t
 ** Structure to store channel information
 *******************************************************************************
 **/
typedef struct {
    uchar page;
    uchar channel;
} trxsm_channel_t;

/**
 *******************************************************************************
 ** \struct trxsm_ack_t
 ** Structure to store acknowledgement information
 *******************************************************************************
 **/




typedef struct {
    uchar dsn;		/**< data sequence number */
    uchar fp;		/**< frame pending bit */
    uint32_t sfd_rx_time;

#if ( WISUN_ENET_EACK == 1 )
    mac_address_t dst;
    uint8_t dest_long_addr[8]; 
#ifdef WISUN_FAN_MAC
    mac_address_t src;
    uint8_t src_long_addr[8];  
#endif  
  
#endif
} trxsm_ack_t;

/**
 *******************************************************************************
 ** \enum trxsm_trigger_t
 ** Specific triggers for TRX state machine
 *******************************************************************************
 **/
typedef enum
{
    TRXSM_TRIGGER_ENTRY = SM_TRIGGER_ENTRY,
    TRXSM_TRIGGER_EXIT  = SM_TRIGGER_EXIT,
    TRXSM_TRIGGER_ALARM_BEACON,
    TRXSM_TRIGGER_ALARM_CFP,
    TRXSM_TRIGGER_ALARM_INACTIVE,
    TRXSM_TRIGGER_ALARM,
    TRXSM_TRIGGER_CCA_DONE,
    TRXSM_TRIGGER_CCA_MODE_1_REQ,
    TRXSM_TRIGGER_NEW_DIRECT_PACKET,
    TRXSM_TRIGGER_NEW_BCAST_PACKET,
    TRXSM_TRIGGER_PD_DATA_CONFIRM,
    TRXSM_TRIGGER_PD_DATA_INDICATION,
    TRXSM_TRIGGER_BCN_RECEIVED,
    TRXSM_TRIGGER_BCN_TX_REQUIRED,
    TRXSM_TRIGGER_ACK_RECEIVED,
    TRXSM_TRIGGER_ACK_REQUIRED,
    TRXSM_TRIGGER_BCN_EXPECTED,
    TRXSM_TRIGGER_RSSI,
    TRXSM_TRIGGER_UPDATE_RX,
    TRXSM_TRIGGER_CHANNEL,
    TRXSM_TRIGGER_CANCEL,
    TRXSM_TRIGGER_ALARM_MPM_EB,
    TRXSM_TRIGGER_CSM_LISTENING,
    TRXSM_TRIGGER_MPM_EB_EXPECTED,
    TRXSM_TRIGGER_EB_REQUIRED,
    TRXSM_TRIGGER_ED_SCAN_DONE,
    TRXSM_TRIGGER_AFTER_CCA_SUCESS,
//#ifdef WISUN_FAN_MAC        
//    TRXSM_START_CHANNEL_HOPPING,
//    TRXSM_HALT_CHANNEL_HOPPING,
//    TRXSM_RESTART_CHANNEL_HOPPING,
//    TRXSM_TRIGGER_HOP_ALARM,
//#endif        
#ifdef ENHANCED_ACK_SUPPORT
	TRXSM_TRIGGER_ENACK_TMR_EXPIRY
#endif
} trxsm_trigger_t;

/**
 *******************************************************************************
 ** \enum trxsm_result_t
 ** Specific result codes for TRX state machine
 *******************************************************************************
 **/
typedef enum {
    TRXSM_NONE,
    TRXSM_SUCCESS,
    TRXSM_SUCCESS_QUEUE,
    TRXSM_FAILURE,
    TRXSM_FAILURE_NOACK,
    TRXSM_FAILURE_MAXRETRY
} trxsm_result_t;

/**
 *******************************************************************************
 ** \enum trxsm_state_ind_t
 ** Specific states for TRX state machine
 *******************************************************************************
 **/
typedef enum {
    TRXSM_STATE_NONE,
    TRXSM_STATE_IDLE,
    TRXSM_STATE_BCN_TX_SEND,
    TRXSM_STATE_BCAST_CSMA,
    TRXSM_STATE_BCAST_SEND,
    TRXSM_STATE_WAIT_BCN_WAIT,
    TRXSM_STATE_CAP_TX_CSMA,
    TRXSM_STATE_CAP_TX_SEND,
    TRXSM_STATE_CAP_TX_WAIT,
    TRXSM_STATE_CAP_ACK_TX_DELAY,
    TRXSM_STATE_CAP_ACK_TX_SEND,
    TRXSM_STATE_CFP,
    TRXSM_STATE_OFF,
    TRXSM_STATE_RSSI,
    TRXSM_STATE_MPM_EB_TX_SEND,
    TRXSM_STATE_WAIT_MPM_EB_WAIT
} trxsm_state_ind_t;

/**
 *******************************************************************************
 ** \enum trxsm_struct
 **  Data structure to store a TRX State Machine
 *******************************************************************************
 **/

/* None */
struct trxsm_struct
{
    sm_t super;							/**< current state, represented as a general state machine */
    trxsm_state_ind_t state_ind;			        /**< current state indicator */
    trxsm_result_t result;					/**< result of a previous operation */
#if(CFG_MAC_SFTSM_ENABLED == 1)      
    sftsm_t *sftsm;						/**< SFTSM to use for timing */
#endif 
    
#if(CFG_MAC_MPMSM_ENABLED == 1)    
    mpmsm_t* mpmsm;						/**< MPM SM to use for timing*/
#endif
#if (CFG_MAC_CCA_ENABLED == 1)     
    sm_t *ccasm;						/**< CCA state machine to use */
#endif    
    /* message queues */
    queue_t *curr_bcn_queue;				        /**< queue for the latest beacon */
    queue_t *curr_mpm_eb_queue;				        /**< queue for the latest mpm eb */
    queue_t *curr_eb_queue;					/**< queue for the latest eb */
    queue_t *bcast_queue;					/**< queue for broadcasts */
    queue_t *direct_queue;					/**< queue for direct transmission */
    queue_t *indirect_queue;				        /**< queue for indirect transmission */
#ifdef ENHANCED_ACK_SUPPORT
    queue_t *enackwait_queue;				        /**< queue for indirect transmission */
#endif
    queue_t *completed_queue;				        /**< queue for completed transmissions */
    mac_tx_t *packet;						/**< packet under processing */
#ifdef WISUN_FAN_MAC    
    sw_tmr_t dwell_interval_tmr;
    uint32_t current_us_channel;
    uint32_t next_us_channel;
    uint32_t current_us_slot_start_ts;
    uint32_t curr_us_slot_num;
        
    void (*fn_alarm_us_hop)(void*);
    uint64_t uc_chan_hop_seq_start_time;        //Debdeep
    uint64_t bc_chan_hop_seq_start_time;        //Debdeep
    uint64_t bc_chan_hop_seq_current_slot_time; //Debdeep
    uint32_t uc_chan_hop_dwell_start_time;
    uint32_t uc_chan_hop_dwell_halt_time;
    uint8_t* gp_ufsi;
    uint8_t self_ieee_addr[8];
    uint32_t current_max_channels;
#endif
#if RAIL_TIMER_INTERFACE_USED
    RAIL_MultiTimer_t sw_timer;
#else    
    sw_tmr_t sw_timer;						/**< sw timer for gseneral timeout usage */
#endif    
    void (*fn_alarm)(void*);					/**< function to signal timeout alarm */
    ulong delay_ack;						/**< delay before sending acknowledgement */
    ulong tmp;							/**< temporary storage for timeout*/
    uint8_t iUnitRadioChan;					/**< unit radio channel*/
    uint8_t CSMCurrTotalUnitRadioChanCount;                     /**< current total unit radio channel*/
    uint32_t orig_curr_sun_page;			        /**< original current sun page*/
    uint32_t orig_channel;					/**< original channel*/
    uint8_t CSMUnitRadioChanList[8];		                /**< unit radio channel list*/
};

/*
** ============================================================================
** Public Variable Declarations
** ============================================================================
*/

#ifdef ENET_MAC_FOR_GU
 
#define CAPTURE_START_TIME      1
#define CAPTURE_STOP_TIME      2
  typedef enum {
    ENET_MAC_FOR_TE_IDLE,
    ENET_MAC_FOR_TE_CONTINUOUS_TX,
    ENET_MAC_STATE_CARRIER_TX,
    ENET_MAC_FOR_TE_READY    
  }enet_mac_for_te_state_ind_t;

  typedef struct enet_mac_for_te_sm_tag
  {
    sm_t super;							/**< current state, represented as a general state machine */
    enet_mac_for_te_state_ind_t state_ind;		/**< current state indicator */	
    uint8_t noacks_started;
    p3time_t start_time_of_no_ack;
    p3time_t stop_time_of_cont_data_tx;
    //p3time_t time_difference;
    sw_tmr_t sw_timer;					/**< sw timer used for energy detect sampling */
  }enet_mac_for_te_sm_t; 
  

  extern uint8_t enable_abnormal_cmd_tx;  
  extern uint8_t enable_abnormal_assoc_resp_tx;
  extern uint8_t enable_coord_realign_cmd_tx;
  
#endif

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
 ** \brief Function to initialise the TRX state machine
 ** \param *s - TRX state machine
 ** \param *ccasm - CSMA state machine
 ** \param *scansm - SCAN state machine
 ** \retval - None
 ******************************************************************************/
void trxsm_initialise( trxsm_t *s, sm_t *ccasm );

/**
 *******************************************************************************
 ** \brief Function to start the TRX state machine
 ** \param *s - TRX state machine
 ** \retval - None
 ******************************************************************************/
void trxsm_start( trxsm_t *s );

/**
 *******************************************************************************
 ** \brief Function to link the TRX state machine with SFT state machine
 ** \param *s - TRX state machine
 ** \param *sftsm - SFT state machine
 ** \retval - None
 ******************************************************************************/
#if(CFG_MAC_SFTSM_ENABLED == 1)    
 void trxsm_use_sftsm( trxsm_t *s, sftsm_t *sftsm );
#endif
/**
 *******************************************************************************
 ** \brief Function to get the state of SFT sm from the TRX state machine
 ** \param *s - TRX state machine
 ** \retval - current state of the SFT sm
 ******************************************************************************/
#if(CFG_MAC_SFTSM_ENABLED == 1)   
sftsm_t* trxsm_get_sftsm( trxsm_t *s );
#endif
/**
 *******************************************************************************
 ** \brief Function to get the state of the TRX state machine
 ** \param *s - TRX state machine
 ** \retval - current state of the trx sm
 ******************************************************************************/
trxsm_state_ind_t trxsm_get_state( trxsm_t *s );

/**
 *******************************************************************************
 ** \brief Function to get result of the TRX state machine
 ** \param *s - TRX state machine
 ** \retval - result of the last operation
 ******************************************************************************/
trxsm_result_t trxsm_get_result( trxsm_t *s );

/**
 *******************************************************************************
 ** \brief Function called to indicate the completition of the CCA algorithm
 ** \param *s - TRX state machine
 ** \param result - result of the CCA algorithm
 ** \retval - None
 ******************************************************************************/
#if(CFG_MAC_CCA_ENABLED == 1)    
void trxsm_ntfy_cca_done( void *s, ccasm_result_t result );
void trxsm_ntfy_cca_request( void *s, ccasm_result_t result );
#endif
/**
 *******************************************************************************
 ** \brief Function to pack TRX SM for serialization
 ** \param *s - TRX state machine
 ** \param *dst - destination of the pack data
 ** \retval - size of packed data
 ******************************************************************************/
ushort trxsm_pack( void *s, uchar *dst );

/**
 *******************************************************************************
 ** \brief Function to unpack TRX SM from serialized data
 ** \param *s - TRX state machine
 ** \param *src - source to unpack data
 ** \retval - None
 ******************************************************************************/
void trxsm_unpack( void *s , uchar *src );

/**
 *******************************************************************************
 ** \brief Function to set pending data tx event for MIL Task depending on the 
 **  state of LBTSM
 ** \param s - TRX state machin
 ** \retval - None
 ******************************************************************************/
void trxsm_set_pending_tx_event( trxsm_t *s );

#ifdef __cplusplus
}
#endif
#endif /* TRXSM_H */
