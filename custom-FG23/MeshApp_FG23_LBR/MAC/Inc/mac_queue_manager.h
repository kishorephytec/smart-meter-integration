/** \file mac_queue_manager.h
 *******************************************************************************
 ** \brief This file provides the various queue functionalities.
 **
 ** This file provides the functions like initialize, put an item, putting an 
 ** item at the front, getting an item from the queue, peeking into the queue, 
 ** get the count of items, scanning through the next item in the queue and 
 ** removing an item from the queue.
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

#ifndef QUEUE_MANAGER_H
#define QUEUE_MANAGER_H 

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
 ** \enum mlist_identifier_e
 ** Enumerations for different Queue events
 *******************************************************************************
 **/
typedef enum {
    QUEUE_TX_FREE = 0,
    QUEUE_RX_FREE,
    QUEUE_BCN_TX_FREE,
    QUEUE_BCN_TX_CURR,
    QUEUE_BCN_RX,
    QUEUE_DATA_REQUEST,
    QUEUE_BCAST,
    QUEUE_INDIRECT_TX,
    QUEUE_TX_DONE,
    QUEUE_CAP_TX,
    QUEUE_PHY_RX,
    QUEUE_RX_MSG,
    QUEUE_RX_FREE_MSG,
    QUEUE_GTS_TX,		
    QUEUE_SPI_RX,
    QUEUE_NHLE_2_MLME,
    QUEUE_NHLE_2_MCPS,
    QUEUE_L3_UNICAST_PKT,
    QUEUE_L3_BROADCAST_PKT,
#if defined( ZB_INCLUDED )
    QUEUE_USER2ZB,
    QUEUE_USER2ZB_FREE,
    QUEUE_ZB2USER,
    QUEUE_ZB2USER_FREE,
# endif
#ifdef MAC_CFG_SECURITY_ENABLED
    QUEUE_RX_SECURITY,
#endif
    QUEUE_PHY_DATA,

    QUEUE_EB_TX_FREE,
    QUEUE_EB_TX_CURR,

    QUEUE_MPM_EB_TX_FREE,
    QUEUE_MPM_EB_TX_CURR,
#ifdef ENHANCED_ACK_SUPPORT
      QUEUE_ENACKWAIT,
#endif
    QUEUE_END
} mlist_identifier_e;

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
 ** \brief Function to put an item at the end of the list specified by q_id
 ** \param[in] q_id - list identifier
 ** \param[in] *item - the element to insert in the list
 ** \retval - None
 ******************************************************************************/
void queue_manager_push_back( uchar q_id, queue_item_t *item);

/**
 *******************************************************************************
 ** \brief Function to put an item at the start of the list specified by q_id
 ** \param[in] q_id - list identifier
 ** \param[in] *item - the element to insert in the list
 ** \retval - None
 ******************************************************************************/
void queue_manager_push_front(uchar q_id, queue_item_t *item);

/**
 *******************************************************************************
 ** \brief Function to get an item at the begining of the list specified by q_id
 ** \param[in] q_id - list identifier
 ** \retval - the element at the begining of the list
 ******************************************************************************/
queue_item_t *queue_manager_pop_front(uchar q_id);

/**
 *******************************************************************************
 ** \brief Function to removes the given item from the list specified by q_id
 ** \param[in] q_id - list identifier
 ** \param[in] *item - element to be removed
 ** \retval - Success or failure (0)
 ******************************************************************************/
void queue_manager_remove(uchar q_id, queue_item_t *item );

/**
 *******************************************************************************
 ** \brief Function to see the first element of the list
 ** \param[in] q_id - list identifier
 ** \return first element of the list
 ******************************************************************************/
queue_item_t* queue_manager_peek(uchar q_id);

/**
 *******************************************************************************
 ** \brief Function to return the number of elements in the list
 ** \param[in] q_id - list identifier
 ** \retval = number of elements in the list
 ******************************************************************************/
uchar queue_manager_size(uchar q_id);

/**
 *******************************************************************************
 ** \brief Function to find the next item in the list
 ** \param[in] q_id - list identifier
 ** \param[in] *item - the item from where to begin to scan. If NULL begin from 
 **             begining of list
 ** \retval = the next item on the list
 ******************************************************************************/
queue_item_t *queue_manager_scan_next(uchar q_id, queue_item_t *item);

/**
 *******************************************************************************
 ** \brief Function to initialise the list
 ** \param[in] q_id - list identifier
 ** \retval - None
 ******************************************************************************/
void queue_manager_initialise(uchar q_id);

/**
 *******************************************************************************
 ** \brief Function to get the list pointer identified by q_id
 ** \param[in] q_id - list identifier
 ** \retval - the list pointer
 ******************************************************************************/
queue_t* queue_manager_get_list(uchar q_id);

#ifdef MAC_CFG_SECURITY_ENABLED

/**
 *******************************************************************************
 ** \brief Adds an entry to a specific place in the linked list
 ** \param[in] q_id - list identifier
 ** \param[in] *item - pointer to the list item to be inserted in the list
 ** \param iterator - should be inside the boundaries of the list
 ** \retval - None
 ******************************************************************************/
void queue_manager_insert(uchar q_id, queue_item_t* item, uchar iterator);

/**
 *******************************************************************************
 ** \brief reads an entry from a linked list
 ** \param[in] q_id - list identifier
 ** \param[in] iterator - the iterator of the element to be read
 ** \retval - the respective item
 ******************************************************************************/
//queue_item_t* queue_manager_get(uchar q_id, uchar iterator);

/**
 *******************************************************************************
 ** \brief Deletes a list entry from a linked list, and frees up the memory
 ** \param[in] q_id - identifier
 ** \param[in] iterator - the list iterator to the element we want to delete
 ** \retval - None
 ******************************************************************************/
void queue_manager_delete_entry(uchar q_id, uchar iterator);

/**
 *******************************************************************************
 ** \brief Deletes all entries from a linked list, and frees up the memory
 ** \param q_id - list identifier
 ** \retval - None
 ******************************************************************************/
//void queue_manager_delete(uchar q_id);

#endif

#ifdef __cplusplus
}
#endif
#endif /* QUEUE_MANAGER_H */

