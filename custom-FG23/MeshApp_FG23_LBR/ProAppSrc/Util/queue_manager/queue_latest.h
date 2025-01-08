/** \file queue_latest.h
 ******************************************************************************
 ** \brief Host Queue Header.
 **   
 ** This file provides the functionalities of the queue module. It provides 
 ** different structures and function APIs of the queue module to use by the 
 ** timer application
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
 ******************************************************************************
 **  \endcond
 */

#ifndef _QUEUE_H_
#define _QUEUE_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 *****************************************************************************
 * @ingroup sysdoc
 *
 * @{
 *****************************************************************************/

/**
 *****************************************************************************
 * @defgroup Qmgmt Queue Management
 * @brief This section describes about the Queue_Latest.c functionality 
 *
 * This module provides the different operations to be performed on queues.
 * The user should create the instance of the queue structure and to initialise 
 * the queue the queue_initialise() is called by providing the pointer to the 
 * queue structure to be initialized. Once the queue is initialised the user can 
 * perform various operations on the queue like,
 *
 * (1) put an item into the queue \n
 * (2) putting an item to the front of the queue \n
 * (3) get an item from the queue\n
 * (4) peek into the queue\n
 * (5) get the number of items in the queue\n
 * (6) scan next item in the queue and \n
 * (7) remove an item from the queue.\n
*******************************************************************************/

/**
 *******************************************************************************
 * 
 * @}     
 ******************************************************************************/

/*
** ============================================================================
** Public Macro definitions
** ============================================================================
*/

/**
 ** \defgroup q_defs  QUEUE Definitions
 ** \ingroup Qmgmt
 */

/*@{*/

/* None */

/*
** ============================================================================
** Public Structures, Unions & enums Type Definitions
** ============================================================================
*/

/**
 ******************************************************************************
 * @struct Structure for queueing the items.
 *     Structure definition  to create the specified Queue_item.
 *    
 *****************************************************************************/
typedef struct queue_item_struct
{
	struct queue_item_struct *link; /**< Pointer to the item */
} queue_item_t;

/**
 *******************************************************************************
 * @struct Structure for queue. 
 *    Structure definition to create Queue_struct for getting the count,
 *     and pointer to start of the queue and end of the queue.
 ******************************************************************************/
typedef struct queue_struct
{
	queue_item_t *start; /**< Pointer to point to the starting location of the 
                             queue*/
	queue_item_t *end; /**< Pointer to point to the end of the queue */
	uint16_t count; /**< Counts the number of items in the queue */
} queue_t;

/*@}*/

/*
** ============================================================================
** Public Variable Declarations
** ============================================================================
*/

/*None*/

/*
**=========================================================================
**  Public Function Prototypes
**=========================================================================
*/

/**
 ** \defgroup q_req  QUEUE APIs
 ** \ingroup Qmgmt
 */

/*@{*/

/**
 *******************************************************************************
 * @brief Initialises the queue(pointer to the queue object).
 * @param *q[in]- pointer to queue structure to initialise. 
 * @retval None
 * @note - This function should be called atleast once before using any of the 
 *          queues.
 ******************************************************************************/
void queue_initialise(
    queue_t * q 
    );

/**
 *******************************************************************************
 * @brief Puts an item on a queue.
 * @param *q[in] - queue on which to add the items.
 * @param *item[in] - item to be added into the queue.
 * @retval None
 ******************************************************************************/
void queue_item_put(
    queue_t * q,        
    queue_item_t *item  
    );

/**
 *******************************************************************************
 * @brief Puts an item at the front of a queue.
 * @param *q[in] - queue on which to save the item.
 * @param *item[in] - item to be added at front in the queue.
 * @retval None
 ******************************************************************************/
void queue_front_put(
    queue_t * q,        
    queue_item_t *item  
    );

/**
 *******************************************************************************
 * @brief Gets an item from the specified queue.
 * @param *q[in] - queue from which to get the item.
 * @retval pointer to an item.
 ******************************************************************************/
queue_item_t *queue_item_get(
     queue_t * q 
     );

/**
 *******************************************************************************
 * @brief Peeks the first item on the specified queue.
 * @param *q[in] - pointer to the queue to peek for the first item.
 * @retval pointer of the first item on a queue.
 ******************************************************************************/
queue_item_t *queue_peek(
     queue_t * q 
     );

/**
 *******************************************************************************
 * @brief  counts the number of items on a queue.
 * @param *q[in] - queue from which to get the count.
 * @retval number of items on the queue.
 ******************************************************************************/
uint16_t queue_count_get(
      queue_t * q 
      );

/**
 *******************************************************************************
 * @brief Find the next item on a queue.
 * @param *q[in] - pointer to the queue from where the item needs to be scanned.
 * @param *item[in]- pointer to the item from where the scan to start.
 * @retval pointer of the next scanned item.
 ******************************************************************************/
queue_item_t *queue_item_scan_next(
   queue_t * q, 
   queue_item_t *item 
  );
  
/**
 *******************************************************************************
 * @brief Find the specified item on a queue and remove it.
 * @param *q[in]- pointer to the queue from which the item needs to be removed.
 * @param *item[in] - pointer to the item to be removed.
 * @retval 1 - On Success.
 * @retval 0 - On Failure.
 ******************************************************************************/  
void queue_item_remove(
    queue_t * q, 
    queue_item_t *item 
    );

/**
 *******************************************************************************
 * @brief Gets the last item from the specified queue and removes it from queue.
 * @param *q[in] - queue from which to get the item.
 * @retval pointer to an item.
 ******************************************************************************/  
queue_item_t* queue_item_get_last(
    queue_t * q 
    );

/********************************************************************************
 * @brief Inserts an item at the specified index in the specified queue.
 * @param *q[in] - queue from which to get the item.
 * @param iterator - index at which the item is to be inserted.
 * @retval - None
 * @note - The iterator value begins from 0
 ******************************************************************************/ 
// Sagar: Not Used
//void queue_item_insert(queue_t * q, queue_item_t* item, uint8_t iterator);

/**
 *******************************************************************************
 * @brief Reads an item from the specified index and the specified queue.
 * @param *q[in] - queue from which to read the item.
 * @param iterator - index from which to read the item.
 * @retval pointer to an item.
 * @note - This fuction just reads the specified item it does not deletes.
 ******************************************************************************/  
queue_item_t* queue_item_read_from(queue_t * q, uint8_t iterator);

/**
 *******************************************************************************
 * @brief Deletes the specified index item from the specified queue.
 * @param *q[in] - queue from which to get the item.
 * @param iterator - index from which the item is to be deleted.
 * @retval - None
 ******************************************************************************/  
void queue_item_delete(queue_t * q, uint8_t iterator);

/**
 *******************************************************************************
 * @brief Deletes all the items from the specified queue.
 * @param *q[in] - queue from which to delete all the items.
 * @retval - None
 * @note - This function it re-initialises the specified queue.
 ******************************************************************************/  
void queue_delete(queue_t * q);

void dhcpv6_queue_item_delete(queue_t * q, uint8_t iterator);
/*@}*/

#ifdef __cplusplus
}
#endif
#endif /* _QUEUE_H_ */
