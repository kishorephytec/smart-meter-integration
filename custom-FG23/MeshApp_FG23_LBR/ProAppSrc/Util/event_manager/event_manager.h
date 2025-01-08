/** \file event_manager.h
 *******************************************************************************
 ** \brief Provides supporting APIs and macros for Non-RTOS task scheduling 
 ** mechanism 
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
 
#ifndef EVENT_MANAGER_H
#define EVENT_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

/*
** ============================================================================
** Public Macro definitions
** ============================================================================
*/

#define MAC_EVENT_NONE					0xFF
#define RF_INT_OCCURED				        0x00
#define TIMER_EXPIRY_EVENT				RF_INT_OCCURED + 1
#define PHY_2_MAC_EVENT                                 TIMER_EXPIRY_EVENT + 1
//#define RF_INT_OCCURED					TIMER_EXPIRY_EVENT + 1
//#define RF_TX_DONE_EVENT				RF_INT_OCCURED + 1
//#define RF_RX_PENDING_EVENT				RF_TX_DONE_EVENT + 1
#if ((RADIO_VALIDATION || SNIFFER) || (APP_HIF_PROCESS_FEATURE_ENABLED == 0) )
    #define HIF_RX_EVENT				PHY_2_MAC_EVENT + 1
    #define HIF_TX_EVENT				HIF_RX_EVENT + 1
    #define RX_SEC_FRAME_EVENT				HIF_TX_EVENT + 1	
#else
    #define RX_SEC_FRAME_EVENT				PHY_2_MAC_EVENT + 1	
#endif
#define UNSECURE_EVENT					RX_SEC_FRAME_EVENT + 1		
#define BCN_RX_EVENT					UNSECURE_EVENT + 1
#define DATA_REQ_RX_EVENT				BCN_RX_EVENT + 1
#define CMD_OR_DATA_FRAME_RX_EVENT		        DATA_REQ_RX_EVENT + 1
#define SECURE_EVENT					CMD_OR_DATA_FRAME_RX_EVENT + 1
#define PENDING_TX_EVENT_UCAST				SECURE_EVENT + 1
#define PENDING_TX_EVENT_BCAST                          PENDING_TX_EVENT_UCAST+1
#define FRAME_TX_DONE_EVENT				PENDING_TX_EVENT_BCAST + 1
#define MLME_EVENT					FRAME_TX_DONE_EVENT + 1
#define MCPS_EVENT					MLME_EVENT + 1
//#define MAC_2_NHLE_EVENT				MCPS_EVENT + 1
#define MAX_EVENT_PRIO					MCPS_EVENT + 1

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

/**
 *******************************************************************************
 ** \brief Function to get the highest priority event
 ** \param - None
 ** \retval - priority 
 ******************************************************************************/
uint32_t highest_prio_event_get( void );

/**
 *******************************************************************************
 ** \brief Function to set event for given priority
 ** \param prio - priority 
 ** \retval - None
 ******************************************************************************/
void event_set( uint32_t prio );

//void set_event_signal_to_mask(uint32_t prio);
/**
 *******************************************************************************
 ** \brief Function to clear event for given priority
 ** \param prio - priority 
 ** \retval - None
 ******************************************************************************/
void event_clear( uint32_t prio );

/**
 *******************************************************************************
 ** \brief Function to signel event to mac layer
 ** \param prio - None
 ** \retval - None
 ******************************************************************************/
//extern void signal_event_to_mac_task( uint8_t event );

#ifdef __cplusplus
}
#endif
#endif		//EVENT_MANAGER_H

