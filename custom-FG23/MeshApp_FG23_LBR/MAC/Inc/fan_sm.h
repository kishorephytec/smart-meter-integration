/** \file fan_sm.h
 *******************************************************************************
 ** \brief Provides APIs for the Start State Machine
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

#ifndef FAN_SM_H
#define FAN_SM_H

#ifdef __cplusplus
extern "C" {
#endif

/*
** ============================================================================
** Public Macro definitions
** ============================================================================
*/

/* None */

/*
** ============================================================================
** Public Structures, Unions & enums Type Definitions
** ============================================================================
*/

/**
 *******************************************************************************
 ** \struct startsm_t
 ** Representation of Start FAN SM
 *******************************************************************************
 **/

   
#define                 MAXIMUM_SLOT_NUMBER                     65535



/**
 *******************************************************************************
 ** \enum startsm_trigger_t
 ** Specific triggers for STAR TSM.
 *******************************************************************************
 **/
typedef enum
{
    FAN_SM_TRIGGER_ENTRY = SM_TRIGGER_ENTRY,
    FAN_SM_TRIGGER_EXIT = SM_TRIGGER_EXIT,
    FAN_SM_TRIGGER_START_REQ,
    FAN_SM_TRIGGER_PAS_PKT,
    FAN_SM_TRIGGER_PCS_PKT,
    FAN_SM_TRIGGER_PA_PKT,
    FAN_SM_TRIGGER_PC_PKT,
} fan_sm_trigger_t;

/**
 *******************************************************************************
 ** \struct startsm_param_t
 ** Parameters from a FAN PARAM to START SM
 *******************************************************************************
 **/
typedef struct
{
  uint8_t length_channel_pas;
  uint8_t length_channel_pa;
  uint8_t length_channel_pcs;
  uint8_t length_channel_pc;
  uint8_t no_pkt_tx_on_trickle_timer;
  uint8_t pas_channel_index;
  uint8_t pa_channel_index;
   uint8_t pcs_channel_index;
  uint8_t pc_channel_index;
  uint8_t* pas_channel_list;
  uint8_t* pa_channel_list;
  uint8_t* pcs_channel_list;
  uint8_t* pc_channel_list;
} fan_sm_param_t;

/**
 *******************************************************************************
 ** \enum startsm_flag_t
 ** Enumeration to indicate different state of node
 *******************************************************************************
 **/
typedef enum {
    FAN_STARTSM_FLAG_DEVICE = 0,			 /**< running as a device */
    FAN_STARTSM_FLAG_COORD = 1,				 /**< running as a coordinator */
    FAN_STARTSM_FLAG_PANCOORD = 3,			 /**< running as a PAN coordinator */
    FAN_STARTSM_FLAG_COORD_MASK = 3			 /**< mask to determine coordinator status */
} fan_sm_flag_t;

/**
 *******************************************************************************
 ** \enum startsm_state_ind_t
 ** Enumeration to indicate different state of node
 *******************************************************************************
 **/
typedef enum {
              FAN_SM_STATE_NONE,
              FAN_SM_STATE_IDLE,
    
} fan_startsm_state_ind_t;

/**
 *******************************************************************************
 ** \enum startsm
 ** Data structure to store a START SM 
 *******************************************************************************
 **/
typedef struct fan_sm
{
    sm_t super;							
    fan_startsm_state_ind_t state_ind;		
    fan_sm_param_t fan_param;			
    fan_sm_flag_t flags;
}fan_sm_t;

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

/* None */

/*
**=========================================================================
**  Public Function Prototypes
**=========================================================================
*/
void add_ie_list_in_pan_adv_solicite_pkt(void);
void add_ie_list_in_pan_adv_pkt(void);
void add_ie_list_in_pan_config_pkt(void);
void add_ie_list_in_pan_solicite_pkt(void);
void add_ie_list_in_ulad_pkt(void);

#if(FAN_EAPOL_FEATURE_ENABLED == 1)
void add_ie_list_in_eapol_pkt(void);
#endif

void ws_send_pkt(uint8_t pkt_type, uint8_t* channal_list,uint8_t length_channel_list);
/**
 *******************************************************************************
 ** \brief Function to initialise the start state machine
 ** \param *s - start state machine
 ** \retval - None
 ******************************************************************************/
void fan_startsm_initialise(void);
#ifdef __cplusplus
}
#endif
#endif /* STARTSM_H */

