/** \file mac_queue_manager.c
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
#include "StackMACConf.h"
#include "common.h"
#include "mac.h"
#include "queue_latest.h" 
#include "mac_queue_manager.h" 

/*
** =============================================================================
** Private Macro definitions
** =============================================================================
*/

/* None */

/*
** =============================================================================
** Private Structures, Unions & enums Type Definitions
** =============================================================================
**/

/* None */

/*
** ============================================================================
** Private Variable Definitions
** ============================================================================
*/

queue_t queue_array[QUEUE_END];

/*
** =============================================================================
** Public Variable Definitions
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
** ============================================================================
** Private Function Prototypes
** ============================================================================
*/

/* None */

/*
** ============================================================================
** Public Function Definitions
** ============================================================================
*/

void queue_manager_push_back( uchar q_id, queue_item_t *item)
{
    if (q_id < QUEUE_END)
        queue_item_put(&queue_array[q_id], item);
}

/******************************************************************************/

void queue_manager_push_front(uchar q_id, queue_item_t *item)
{
    if (q_id < QUEUE_END)
        queue_front_put(&queue_array[q_id], item);
}

/******************************************************************************/

queue_item_t* queue_manager_pop_front(uchar q_id)
{
    if (q_id < QUEUE_END)
        return (queue_item_get(&queue_array[q_id]));
    return NULL_POINTER;
}

/******************************************************************************/

void queue_manager_remove(uchar q_id, queue_item_t *item )
{
    if (q_id < QUEUE_END)
        queue_item_remove( &queue_array[q_id], item );
    return;
}

/******************************************************************************/

queue_item_t* queue_manager_peek(uchar q_id)
{
    if (q_id < QUEUE_END)
        return (queue_peek(&queue_array[q_id]));
    return NULL_POINTER;
}

/******************************************************************************/

uchar queue_manager_size(uchar q_id)
{
    if (q_id < QUEUE_END)
        return queue_count_get(&queue_array[q_id]);
    return 0;	
}

/******************************************************************************/

queue_item_t* queue_manager_scan_next(uchar q_id, queue_item_t *item)
{
    if (q_id < QUEUE_END)
        return queue_item_scan_next(&queue_array[q_id], item );
    return NULL_POINTER;
}
 
/******************************************************************************/

void queue_manager_initialise(uchar q_id)
{
    if (q_id < QUEUE_END)
        queue_initialise(&queue_array[q_id]);
}
/******************************************************************************/
void queue_manager_initialise_all_queues(void)
{
  int i =0;
    for(i=0;i<QUEUE_END;i++)
    {
       queue_initialise(&queue_array[i]);
    }       
}
/******************************************************************************/

queue_t* queue_manager_get_list(uchar q_id)
{
    if (q_id < QUEUE_END)	
        return &queue_array[q_id];
    return NULL_POINTER;
}

/*
** ============================================================================
** Private Function Definitions
** ============================================================================
*/

/* None */


