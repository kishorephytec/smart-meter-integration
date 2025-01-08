/** \file list_latest.h
 ******************************************************************************
 ** \brief Provides APIs for creating and maintaining singly linked list.
 **   
 ** This file provides the functionalities of the list module. It provides 
 ** different structures and function APIs of the list module to use by the 
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
 
#ifndef _LIST_LATEST_H_
#define _LIST_LATEST_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 ******************************************************************************
 * @ingroup sysdoc
 *
 * @{
 *****************************************************************************/

/**
 ******************************************************************************
 * @defgroup List List Management
 * @brief This section describes about the List_Latest.c functionality 
 *
 * This module provides the different APIs for differnt operations that can be 
 * performed on a singly linked list.
 * The user should create an instance of the list structure to initialise the 
 * list using the list_init() by providing the pointer to the list obj to 
 * be initialized. Once the list is initialised the user can perform the 
 * following operations on the list.
 *
 * (1) add an item at the start of the list \n
 * (2) add an item at the end of the list	\n
 * (3) add an item after the specified item \n
 * (4) get start item in the list \n
 * (5) get previous item of the specified item \n
 * (6) delete next item of the specified item in the list\n
 * (7) scan the next item in the list \n
 * (8) checks whether the given item is at the start of the list\n
 * (9) delete an item from the list\n
 * (10) sets the start item in the list\n
 * This modules provides APIs for all the above mentioned operations
******************************************************************************/

/**
 ******************************************************************************
 * 
 * @}     
 *****************************************************************************/

/*
** ============================================================================
** Public Macro definitions
** ============================================================================
*/

/**
 ** \defgroup list_defs  LIST Definitions
 ** \ingroup List
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
 * @brief Structure for adding the specified item.
 *     
 *****************************************************************************/
typedef struct list_item
{
    struct list_item *next; /**< Pointer to the item */
} list_item_t;

/**
 ******************************************************************************
 * @brief Structure for adding the specified item at the start of the list.
 *     
 *****************************************************************************/
typedef struct list
{
    list_item_t *start; /**<Pointer to the starting address of the list */
} p3_list_t;

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
 ** \defgroup list_req  LIST APIs
 ** \ingroup List
 */

/*@{*/

/**
 *******************************************************************************
 * @brief Initialises the specified List.
 * @param *list[in] -  pointer to list object to initialise.
 * @retval None
 ******************************************************************************/
void p3list_init( p3_list_t *list );

/**
 *******************************************************************************
 * @brief Adds an entry to the start of the specified list
 * @param *list[in] -  pointer to list object on which to save item. 
 * @param *ptr[in] - item to be added to the queue at the start
 * @retval None
 *******************************************************************************/
 void p3list_entry_add_start( 
							 p3_list_t *list,     
							 list_item_t* ptr  
						   );
      
/**
 ******************************************************************************
 * @brief Adds an item at the end of the specified list.
 * @param *list[in] - pointer to list object on which to save item.
 * @param *ptr[in] - item to be added to the queue at the end.
 * @retval None
 *****************************************************************************/
 // Sagar: Not Used
 /*void list_entry_add_end( 
							p3_list_t *list,     
							list_item_t* ptr  
						);
						*/
/**
 *******************************************************************************
 * @brief Adds an item into the list after the specified location.
 * @param *list[in] - pointer to list object on which to save item.
 * @param *ptr[in] - item to be added. 
 * @param *lptr[in] - pointer to the location after which the item is to be added
 * @retval None
 ******************************************************************************/
 void list_entry_add_after( 
							p3_list_t *list,     
							list_item_t* ptr, 
							list_item_t* lptr 
						   );

/**
 *******************************************************************************
 * @brief Gives the first item from the list.
 * @param *list[in] - pointer to list object from which it has to get.
 * @retval pointer to first item in the list.
 ******************************************************************************/
 list_item_t* list_entry_get_start(
									p3_list_t* list     
								   );

/**
 *******************************************************************************
 * @brief Gets the previous item from the specified item.\n
 * @param *list[in] - pointer to list object from which it has to get.
 * @param *ptr[out] - to get the previous item pointed by the ptr. 
 * @retval - pointer to the item.
 * @note If the item is the first item and trying to get the previous of it then
 *       it will return NULL.
 ******************************************************************************/
 // Sagar: Not Used 
 /*list_item_t* list_entry_get_prev(
									p3_list_t* list,     
									list_item_t* ptr 
								   );    
								   */

/**
 ******************************************************************************
 * @brief Deletes the specified item from a list. 
 * @param *list[in] - pointer to list from which to delete entry item.
 * @param *ptr[in] - entry item to delete.
 * @retval pointer of the deleted item.
 * @note If the item specified to delete is not a member of list then it will
 *        return NULL.
 *****************************************************************************/
void list_entry_delete(
								 p3_list_t *list,    
								 list_item_t* ptr 
								);
    

/**
 *******************************************************************************
 * @brief Deletes the next item of the specified item.
 * @param *list[in] - pointer to list object from which to delete entry.
 * @param *ptr[in] - pointer to the item whose next item is to be deleted.
 * @retval - pointer of the deleted item.
 * @note If the item specified is the last item whose next item has to be deleted
 *       then it will return NULL.
 ******************************************************************************/
// Sagar: Not Used 
/*list_item_t* list_entry_delete_next(
							  p3_list_t *list,    
							  list_item_t* ptr 
							 );
							 */	
/**
 *******************************************************************************
 * @brief Gives the pointer to the next entry in the list.
 * @param *list[in] - pointer to list object on which to save item.
 * @param *ptr[in]- pointer to the location from where to start scan for next entry.
 * @retval pointer to the next entry.
 ******************************************************************************/
list_item_t* list_entry_scan_next(
									p3_list_t* list,     
									list_item_t* ptr 
								   );

/**
 *******************************************************************************
 * @brief Checks whether the given item is the start item of the list.
 * @param *list[in] - pointer to list object on which it has to check.
 * @param *ptr[in] - item to be checked.
 * @retval True indicates that the item is the start item. 
 * @retval False indicates that the item is not the start item.
 ******************************************************************************/
 bool list_entry_is_start(
							p3_list_t*list,     
							list_item_t* ptr 
						 );
    
/**
 *******************************************************************************
 * @brief Sets the specified item as the start item.
 * @param *list[in] - pointer to list object on which it has to set.
 * @param *ptr - the item to be set as the start item.
 * @retval None
 ******************************************************************************/
 void list_entry_set_start(
							 p3_list_t*list,     
							 list_item_t* ptr 
						   );
/**
 *******************************************************************************
 * @ingroup Listmgmt
 * @brief joins two lists.
 * @param list1- List to which another list will be joined.
 * @param list2- List to be joined.. 
 ******************************************************************************/					
void list_concat( 
			    p3_list_t *list1,
			    p3_list_t *list2     
			    );

/*@}*/

#ifdef __cplusplus
}
#endif
#endif  /* _LIST_LATEST_H_ */
