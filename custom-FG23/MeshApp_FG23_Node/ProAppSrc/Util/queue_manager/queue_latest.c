/** \file queue_latest.c
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

/*
********************************************************************************
* File inclusion
********************************************************************************
*/

#include "StackPHYConf.h"
#include "common.h"
#include "queue_latest.h"
#include "buff_mgmt.h"

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
** =============================================================================
** Private Variable Definitions
** =============================================================================
*/

/* None */

/*
** =============================================================================
** Private Function Prototypes
** =============================================================================
*/

/* None */

/*
** =============================================================================
** Private Function Definitions
** =============================================================================
*/

/* None */

/*
** =============================================================================
** External Variable Declarations
** =============================================================================
*/

/*Umesh :01-02-2018*/
extern uint8_t heap[];
/*this is not used anywher in this file*/

/*
** =============================================================================
** External Function Prototypes
** =============================================================================
*/

extern void app_bm_free(
    uint8_t *pMem      
    );

/*
** =============================================================================
** Public Variables Definitions
** =============================================================================
**/

/* None */

/*
** =============================================================================
** Public Function Prototypes
** =============================================================================
*/

/* None */

/*
** =============================================================================
** Public Function Definitions
** =============================================================================
*/

void queue_initialise(
    queue_t * q 
    )
{
    q->count = 0;
    q->start = q->end = NULL;
}

/*----------------------------------------------------------------------------*/

void queue_item_put(
    queue_t * q,        
    queue_item_t *item  
    )
{
//    irq_state_t flags = __get_interrupt_state();//Umesh
//    flags = irq_disable();
    item->link = NULL;

    /* if this is the first item set as queue start */
    if ( q->start == NULL )
    {
        q->start = item;
    }
    else
    {
        /* not the first item */
        q->end->link = item;
    }
    q->end = item;
    q->count++;
//    irq_enable( flags );
}

/*----------------------------------------------------------------------------*/

void queue_front_put(
    queue_t * q,      
    queue_item_t *item 
    )

{
//    irq_state_t flags = irq_disable();

    if(item == NULL)
    {
//	irq_enable( flags );    
    	return;
    }

    /* if there is nothing on the queue just set the queue start */
    /*if ( q->start == NULL )
    {
        q->start = item;
    }*/
     if ( q->start != NULL )
    {
        /* not the first item, so make it the first item, as follows */

        /* first make the new item point at the current first item */
        item->link = q->start;
	q->start = item;
	q->count++;

    }
    else
    {
	 queue_item_put(q,item);   
    }
	/* now make the queue point at the new item */
	//q->start = item;
    //q->count++;
//    irq_enable( flags );

}

/*----------------------------------------------------------------------------*/

queue_item_t *queue_item_get(
     queue_t * q 
     )
{
    queue_item_t *item = NULL;
//    irq_state_t flags = irq_disable();

    /* get first item on queue */
    item = q->start;

    if ( item != NULL )
    {
        /* one less item on queue */
        q->count--;
        /* queue points at next item */
        q->start = item->link;
        /* item is no longer linked */
        item->link = NULL;
    }
//    irq_enable( flags );

    return item;
}

/*----------------------------------------------------------------------------*/

queue_item_t *queue_peek(
     queue_t * q
     )
{
    queue_item_t *item = NULL;
//    irq_state_t flags = irq_disable();

    /* get first item on queue */
    item = q->start;

//    irq_enable( flags );
    return item;
	//return q->start;
}

/*----------------------------------------------------------------------------*/
uint16_t queue_count_get(
      queue_t * q 
      )
{
    uint16_t count = 0;
//    irq_state_t flags  = irq_disable();
     
    count = q->count;
    
//    irq_enable(flags);
    
    return count;
}

/*----------------------------------------------------------------------------*/

queue_item_t *queue_item_scan_next(
   queue_t * q, /*queue to process */
   queue_item_t *item /* place to start */
  )
{
    queue_item_t *next_item = NULL;
//    irq_state_t flags = irq_disable();

    if ( item != NULL )
    {
        next_item = item->link;
//        irq_enable(flags);
		//return item->link;
    }
    else if ( q != NULL )
    {
        next_item = q->start;
//        irq_enable(flags);
		//return q->start;
    }
    else
    {
        next_item = NULL;
//        irq_enable(flags);
		//return NULL;
    }
//    irq_enable(flags);
    return next_item;
}

/*----------------------------------------------------------------------------*/

void queue_item_remove(
						queue_t *q, 
						queue_item_t *item 
					  )    
{
    queue_item_t *current_item = NULL;
//    irq_state_t flags = irq_disable();

    if ( item != NULL )
    {

        /* are we attempting to remove the item at the start of the queue? */
        if (item == q->start)
        {
            ( void )queue_item_get(q);
            //result = 1;
        }
        else
        {
            /* init the position in the queue */
            current_item = NULL;

            /* now loop until either the end of queue, OR we find the item */
            for (;;)
            {
                current_item = queue_item_scan_next(q, current_item);
                if (current_item == NULL)
                {
                    /* end of the queue, so exit result still 0 */
                    break;
                }
                if (current_item->link == item)
                {
                    /* we found it, so check if it was the end of the queue */
                    if (q->end == current_item->link)
                    {
                        /* yes, so adjust q->end to point at the current item */
                        q->end = current_item;
                    }
                    /* unlink it! */
                    current_item->link = current_item->link->link;
                    q->count--;
                    //result = 1;
                    break;
                }
            }
        }
        
    }
//        irq_enable( flags );
    return;
}

/*----------------------------------------------------------------------------*/ 
queue_item_t* queue_item_get_last(
                                            queue_t * q 
								 )
{
    queue_item_t *end = NULL, *item = NULL;
//    irq_state_t flags = irq_disable();
    //item = NULL;//Umesh commented

    /* get last item on list */
    end = q->end;
    /* get first item on list */
    item = q->start;

    if (item != NULL)
    {
        /* Is there only one entry on the list? */
        if (item == end)
        {
            /* queue will be empty, reset start and end pointers */
            q->start = NULL;
            q->end = NULL;
        }
        else 
		{
            /* More than one element in the queue */
            while (item->link != end)
            {
                item = item->link;
            }
            q->end = item;
            item = item->link;
            q->end->link = NULL;
        }
        /* Decrement the number of items in the queue */
        q->count--;
    }    
//    irq_enable( flags );
    return item;
}

/*----------------------------------------------------------------------------*/
// Sagar: Not Used
#if (0)
void queue_item_insert(queue_t * q, queue_item_t* item, uint8_t iterator)
{
    uint8_t i = 0;
//    irq_state_t flags = 0;
    queue_item_t *current = NULL;
    
//    flags = irq_disable();

	if (item != NULL)
	{
		current = q->start;
		/* Inserts item at front */ 
		if (iterator == 0)
		{
			queue_front_put( q, item );
		}
		/* Inserts item at end */
		else if (iterator == (q->count))
        {
            queue_item_put(q, item);
        }
		else
		{
			if ( iterator < q->count )
			{
				/* Find the location in the table */
				while( i++ != iterator-1 )  // We want to find the entry before the one we want to insert
				{
					current = current->link;
				}
				/* Insert the item in the given position */
				item->link = current->link;
				current->link = item;
				/* Update the queue counter */
				q->count++;	
			}
		}	
	}
//    irq_enable( flags );
}
#endif

/*----------------------------------------------------------------------------*/

queue_item_t* queue_item_read_from(queue_t *q, uint8_t iterator)
{
    queue_item_t *current = NULL;
//    irq_state_t flags = irq_disable();
    //current = NULL;//Umesh commented

    if ((q->start != NULL) && (iterator < q->count ))
    {	
        current = q->start;
	
        /* Find the location in the table */
        while( iterator-- != 0 )
        {
            current = current->link;
        }	
    }
//    irq_enable( flags );
    return current;
}


/*----------------------------------------------------------------------------*/

void queue_item_delete(queue_t *q, uint8_t iterator)
{
    queue_item_t* item = NULL;
    queue_item_t *current = NULL;
//    irq_state_t flags = irq_disable();
    //item = NULL;//Umesh commented
    //current = NULL;//Umesh commented
	
    if (q->count > 0)
    {		
        /* Are we trying to delete the first entry */
        if (iterator == 0)
        {
            item = queue_item_get(q);
        }
		/* Are we trying to remove the last entry */
        else if ( iterator == (q->count -1) )
        {
            item  = queue_item_get_last(q);
        }
        else	
        {
			if ( iterator < q->count )
			{
				/* Find the location before the one we want to remove */
				current = q->start;		

				while( iterator-- != 1 )
				{
					current = current->link;
				}
			
				/* Get the one we want to remove */
				item = current->link;
			
				/* Unlink the unwanted item */
				current->link = item->link;
				/* Update the queue counter */
				q->count--;
			}
        }
    }
        
    /* free the memory allocated for the item */
    if ( item != NULL_POINTER )
    {
        app_bm_free((void *) item);
    }
    
//   irq_enable( flags );
}
/*----------------------------------------------------------------------------*/

void dhcpv6_queue_item_delete(queue_t *q, uint8_t iterator)
{
 queue_item_t* item = NULL;
    queue_item_t *current = NULL;
//    irq_state_t flags = irq_disable();
    //item = NULL;//Umesh commented
    //current = NULL;//Umesh commented
	
    if (q->count > 0)
    {		
        /* Are we trying to delete the first entry */
        if (iterator == 0)
        {
            item = queue_item_get(q);
        }
		/* Are we trying to remove the last entry */
        else if ( iterator == (q->count -1) )
        {
            item  = queue_item_get_last(q);
        }
        else	
        {
			if ( iterator < q->count )
			{
				/* Find the location before the one we want to remove */
				current = q->start;		

				while( iterator-- != 1 )
				{
					current = current->link;
				}
			
				/* Get the one we want to remove */
				item = current->link;
                                
				/* Unlink the unwanted item */
				current->link = item->link;
				/* Update the queue counter */
				q->count--;
			}
        }
    }
        
    /* free the memory allocated for the item */
    if ( item != NULL_POINTER )
    {
        app_bm_free((void *) item);
    }
    
//   irq_enable( flags );

}
/*----------------------------------------------------------------------------*/ 

void queue_delete(queue_t * q)
{
    queue_item_t *current = NULL, *tmp = NULL;
//    irq_state_t flags = irq_disable();

    current = q->start;

    if (current != NULL_POINTER)
	{	
        while( current->link != NULL_POINTER )
        {
            tmp = current->link;
            app_bm_free( (void *) current);
            current = tmp;
        }
        app_bm_free((void *) current);
        queue_initialise(q);	
    }    
//    irq_enable(flags);
}

/*----------------------------------------------------------------------------*/