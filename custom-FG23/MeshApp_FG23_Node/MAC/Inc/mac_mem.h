/** \file mac_mem.h
 *******************************************************************************
 ** \brief Provides APIs for Memory Management
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

#ifndef MAC_MEM_H
#define MAC_MEM_H

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
 ** \brief Allocate a buffer and data structure for MAC transmit packets
 ** \param type - type of frame required
 ** \param length - length of data buffer
 ** \param secure - packet is to be secured
 ** \param sub_type - type of beacon 
 ** \retval - pointer to the structure or NULL_POINTER if it fails
 ******************************************************************************/
mac_tx_t* mac_mem_alloc_tx(
                           uchar type,      
                           uint16_t length,    
                           uchar secure,     
						   uchar sub_type	
                           );

/**
 *******************************************************************************
 ** \brief Function to free the mac_tx_t message
 ** \param *txd - tx data frame to free
 ** \retval - None
 ******************************************************************************/
void mac_mem_free_tx( mac_tx_t *txd );

#ifdef __cplusplus
}
#endif
#endif /* MAC_MEM_H */
