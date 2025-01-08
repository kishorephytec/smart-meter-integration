/** \file list_latest.c
 *******************************************************************************
 ** \brief This file provides the two list functionalities.
 **
 ** This file provides the functions like adding an entry at the end of the 
 ** list and deleting an entry from the list.
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
#include "list_latest.h"

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

//static void list_entry_add_end( 
//      p3_list_t *list,     
//      list_item_t* ptr  
//      );
//
//static list_item_t* list_entry_delete_next(
//    p3_list_t *list,    
//    list_item_t* ptr 
//    );

/*
** =============================================================================
** External Variable Declarations
** =============================================================================
*/

/* None */

/*
** =============================================================================
** External Function Prototypes
** =============================================================================
*/

/* None */

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

void p3list_init( p3_list_t *list )
{
    list->start = NULL;
}

/*----------------------------------------------------------------------------*/

void p3list_entry_add_start( 
      p3_list_t *list,     /**< the list    */
      list_item_t* ptr  /**< the item to be saved */
      )
{
//    irq_state_t flags = __get_interrupt_state();//Umesh

//    flags = irq_disable();
    
	ptr->next = NULL;

	if ( list->start != NULL)
    {
        // Add to front of the list
        ptr->next  = list->start;       
    }
	list->start = ptr;

    /*if ( list->start == NULL)
    {
        // This is the first
        list->start = ptr;
    }
    else
    {
        // Add to front of the list
        ptr->next  = list->start;
        list->start = ptr;
    }*/
    
//    irq_enable( flags );
}

/*----------------------------------------------------------------------------*/
 
 void list_entry_add_after( 
    p3_list_t *list,     
    list_item_t* ptr, 
    list_item_t* lptr 
    )
{
//    irq_state_t flags = __get_interrupt_state();//Umesh;

//    flags = irq_disable();
    
	if ( lptr == NULL )
    {
        p3list_entry_add_start( list, ptr );
    }
    else
    {
    	ptr->next = lptr->next;
    	lptr->next = ptr;	
    }        

//   irq_enable( flags );
}

/*----------------------------------------------------------------------------*/

 void list_concat( 
					p3_list_t *list1,
					p3_list_t *list2     
				  )
{
//    irq_state_t flags = __get_interrupt_state();//Umesh
    list_item_t *current = NULL;
//    flags = irq_disable();
    
    
    
    if ( list1->start == NULL )
    {
    	*list1 = *list2;
//    	irq_enable( flags );
    	return;
    }
    
    if( list2->start == NULL )
    {
//    	irq_enable( flags );
    	return;
    }
    	
    current = list1->start;
    
    while( current->next != NULL)
    {
        current = current->next;
    }
    current->next = list2->start;
    
//    irq_enable( flags );
}

/*----------------------------------------------------------------------------*/

 list_item_t* list_entry_get_start(
                                          p3_list_t*list     
								  )
{
    list_item_t* start_item = NULL;
//    irq_state_t flags = __get_interrupt_state();//Umesh

//    flags = irq_disable();
    
    start_item =  list->start;
    
//     irq_enable( flags );
     return start_item;
}


/*----------------------------------------------------------------------------*/

 list_item_t* list_entry_scan_next(
    p3_list_t* list,     
    list_item_t* ptr 
    )
{
     list_item_t* next_item = NULL;
//     irq_state_t flags = __get_interrupt_state();//Umesh

//    flags = irq_disable();
    
    next_item = ptr->next;
    
//    irq_enable( flags );
    return next_item;
}

/*----------------------------------------------------------------------------*/
  
  bool list_entry_is_start(
    p3_list_t*list,     
    list_item_t* ptr 
    )
{
//     irq_state_t flags = __get_interrupt_state();//Umesh
     bool is_start = false;

//    flags = irq_disable();
    is_start =  ( list->start == ptr )?true:false;
//     irq_enable( flags );
    return is_start;
}

/*----------------------------------------------------------------------------*/

void list_entry_delete( 
      p3_list_t *list,     
      list_item_t* ptr  
      )
{
    list_item_t *current;//, *ret = NULL;
//    irq_state_t flags = __get_interrupt_state();//Umesh

//    flags = irq_disable();
    
    current = list->start;
	
	if ( ptr == list->start )
	{
		list->start = ptr->next;
		//ret = ptr;
//		irq_enable( flags );  
		return;
	}
    while (( current != NULL ) && ( current->next != ptr ))
    {
        current = current->next;
    }
    if ( NULL != current )
    {
        //current->next = ptr; 
		current->next = ptr->next;   
        //ret = ptr;
    }              
//    irq_enable( flags );      	
    return;
}

/*----------------------------------------------------------------------------*/

 void list_entry_set_start(
    p3_list_t*list,     
    list_item_t* ptr 
    )
{
//      irq_state_t flags = __get_interrupt_state();//Umesh
 

//    flags = irq_disable();
    list->start = ptr;
//     irq_enable( flags );  
}

/*----------------------------------------------------------------------------*/

// Sagar: Not Used 
#if (0)
  list_item_t* list_entry_get_prev(
    p3_list_t*list,     
    list_item_t* ptr 
    )
{
//	irq_state_t flags = __get_interrupt_state();//Umesh
	list_item_t *current = NULL;

//        flags = irq_disable();
    
	if (ptr == list->start)
	{
		//If trying to get the previous item of start
//		irq_enable( flags );
		return NULL;
	}

    current = list->start;

    while(( current != NULL ) && ( current->next != ptr )) 
    {
        current = current->next;
    }
    
//    irq_enable( flags );
    
    return current;
}

/*----------------------------------------------------------------------------*/
 
/*
** =============================================================================
** Private Function Definitions
** =============================================================================
*/

static list_item_t* list_entry_delete_next(
    p3_list_t *list,    
    list_item_t* ptr 
    )
{
    list_item_t *a = NULL;
    //list_item_t *b;
//    irq_state_t flags = __get_interrupt_state();//Umesh

//    flags = irq_disable();
    
    a = list->start;

    if ( a == NULL )
    {
//    	irq_enable( flags );  
    	return NULL;
    }
        
    /* If we are trying to remove the item after first entry */
    if ( list->start == ptr )
    {
        list->start = a->next;
//        irq_enable( flags );                  
		return ptr->next;  
    }   
    
    /* Get the one we want to remove */
    a = ptr->next;

    if (ptr->next == NULL)
	{
		// If we are removing after last entry
//		irq_enable( flags );  
		return ptr->next;
	}
	else
	{
	    /* Remove the unwanted item */
	    ptr->next = a->next;
	}	
//	irq_enable( flags );  
    
    return ptr->next;
}

/*----------------------------------------------------------------------------*/

static void list_entry_add_end( 
      p3_list_t *list,     
      list_item_t* ptr  
      )
{
    list_item_t *current = NULL;
    
//    irq_state_t flags = __get_interrupt_state();//Umesh

//    flags = irq_disable();
    
    current = list->start;

    ptr->next = NULL;

    if ( list->start == NULL)
    {
        // This is the first
        list->start = ptr;
    }
    else
    {
        while( current->next != NULL)
        {
            current = current->next;
        }
        current->next = ptr;
    }
    
//    irq_enable( flags );
}
#endif


/*----------------------------------------------------------------------------*/
