/** \file ccasm.h
 *******************************************************************************
 ** \brief This file breifly describes CCA state machine
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

#ifndef CCASM_H
#define CCASM_H

#ifdef __cplusplus
extern "C" {
#endif

/*
** =============================================================================
** Public Macro definitions
** =============================================================================
*/
	
/* None */

/*
** =============================================================================
** Public Structures, Unions & enums Type Definitions
** =============================================================================
**/
typedef struct ccasm_struct ccasm_t;
/**
 *******************************************************************************
 ** \enum ccasm_trigger_t
 ** Specific triggers for CCA state machines
 *******************************************************************************
 **/
typedef enum
{
    CCASM_TRIGGER_ENTRY = SM_TRIGGER_ENTRY,
    CCASM_TRIGGER_EXIT = SM_TRIGGER_EXIT,
    CCASM_TRIGGER_ALARM,
    CCASM_TRIGGER_TRANSMIT,
    CCASM_CCA_TRIGGER_REQUEST,
    CCASM_TRIGGER_SUSPEND,
    CCASM_TRIGGER_DEFER_TX,
    CCASM_TRIGGER_ONE_HR_EXPIRY,
    CCASM_TRIGGER_ONE_SLOT_EXPIRY,
    CCASM_TRIGGER_TRACK_TX_DURATION,
    CCASM_TRIGGER_CANCEL
} ccasm_trigger_t;


/**
 *******************************************************************************
 ** \enum ccasm_result_t
 ** Specific Result codes for CCA state machines
 *******************************************************************************
 **/
typedef enum {
    CCASM_NONE,
    CCASM_SUCCESS,
    CCASM_CHANNEL_BUSY,
    CCASM_CAP_SHORT,
    CCASM_BLE_SHORT,
    CCASM_PACKET_LONG,
    CCASM_CANCELLED,
    CCASM_SUSPENDED,
    CCASM_DEFERRED,
    CCASM_ERROR
} ccasm_result_t;

/**
 *******************************************************************************
 ** \struct ccasm_param_t
 ** Parameters for CCA
 *******************************************************************************
 **/
    typedef struct
    {
#if(CFG_MAC_SFTSM_ENABLED == 1)        
        sftsm_t *sftsm;
#endif        
        ulong trx_time;
//        freq_band_id_t freq_band_id;
//        phy_mode_t phy_mode;
    }ccasm_param_t;


/**
 *******************************************************************************
 ** \enum lbtsm_flag_t
 ** To indicate different operating modes of LBT-SM
 *******************************************************************************
 **/
typedef enum {
    CCASM_FLAG_USE_BACKOFF = 1,
    CCASM_FLAG_KEEP_RX_ON = 2,
    CCASM_FLAG_ENABLE_TOT_TX_DUR_THRESHOLD = 4,
} ccasm_flag_t;        
    
/**
 *******************************************************************************
 ** \enum lbtsm_state_ind_t
 ** Specific triggers for LBT-SM
 *******************************************************************************
 **/
typedef enum {
    CCASM_STATE_IDLE,
    CCASM_STATE_BACKOFF,
    CCASM_STATE_PERFORM_CCA,
//    CCASM_STATE_SUSPENDED
//    CCASM_STATE_ACTIVE,
//    CCASM_STATE_WARMUP,
//    CCASM_STATE_SAMPLE,
//    CCASM_STATE_SAMPLE_B,
//    CCASM_STATE_DELAY,
//    CCASM_STATE_TX_DEFERRED
} ccasm_state_ind_t;    
          
/**
 *******************************************************************************
 ** \brief Type for callback function of final state
 *******************************************************************************
 **/
typedef void (*ccasm_callback_t)(void *, ccasm_result_t);


/*******************************************************************************
 ** \struct drsm_struct
 ** Data structure to store a CCA State machine
 *******************************************************************************
 **/
struct ccasm_struct
{
    sm_t super; 					/**< current state, represented as a general state machine */
    ccasm_state_ind_t state_ind; 	/**< current state indicator */
    ccasm_flag_t flags;				/**< flag indication*/
    uchar nb; 						/**< number of backoffs */
    uchar cw; 						/**< contention window length */
    uchar be; 						/**< backoff exponent */
    ulong trx_time; 				/**< transaction time in symbols */
    ushort s, s0, p, l, t, w;
    p3time_t t0;
    ccasm_callback_t fn_cca_call_req; /**< function to call before cca request */
    ccasm_callback_t fn_final_call; /**< function to call when finished */
    void *hn_final_call; 			/**< handler for final call */
    sw_tmr_t cca_sw_timer;
    sw_tmr_t cca_sw_timer_suspend; 
     int alarm_delay_lut[3]; 		/**< lookup table of delays between CCAs */
    ulong duration_active;
    sw_tmr_t one_slot_tmr;			/**< slot timer for keeping track of the total sending duration of past cycle*/
}; 
/*
** =============================================================================
** Public Variable Declarations
** =============================================================================
*/

/* None */

/*
** ============================================================================
** Public Function Prototypes
** ============================================================================
*/

/**
 *******************************************************************************
 ** \brief Function initialising the CCA state machine
 ** \param **s - state machine
 ** \retval - None
 ******************************************************************************/
void ccasm_initialise( sm_t **s );


/**
 *******************************************************************************
 ** \brief Function initialising the CCA state machine
 ** \param **s - state machine
 ** \retval - None
 ******************************************************************************/
void cca_sm_initialise( ccasm_t *s,
                       ccasm_callback_t fn_final_call,ccasm_callback_t fn_cca_call_req,
                       void *hn_final_call );

/**
 *******************************************************************************
 ** \brief Check if any CCA is going on
 ** \param *s - state machine
 ** \retval - zero: if the channel is clear to use
 ** \retval - nonzero: if the channel is not clear to use
 ******************************************************************************/
uchar ccasm_is_idle( sm_t *s );

/**
 *******************************************************************************
 ** \brief Check if the any CCA operations should be avoided
 ** \param *s - state machine
 ** \retval - zero: CCA operations can be carried
 ** \retval - nonzero: CCA is in suspended state, no CCA operations can be done
 ******************************************************************************/
//uchar ccasm_is_suspended( sm_t *s );

/**
 *******************************************************************************
 ** \brief test if CCA algorithm is needed to transmit a packet
 ** \param *s - state machine
 ** \param *packet - packet for which the cca if needed to be checked 
 ** \retval - nonzero: if the ccasm cca is needed
 ******************************************************************************/
uchar ccasm_cca_needed( sm_t *s, mac_tx_t *packet );
void ccasm_go_to_idle (ccasm_t *s);


#ifdef __cplusplus
}
#endif
#endif /* CCASM_H */
