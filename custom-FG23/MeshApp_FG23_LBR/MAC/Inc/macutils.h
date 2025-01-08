/** \file macutils.h
 *******************************************************************************
 ** \brief Provides utilities for MAC Layer
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

#ifndef MACUTILS_H
#define MACUTILS_H

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
 ** \brief Function to compare two IEEE addresses
 ** \param *dst - compare destination address
 ** \param *src - compare source address
 ** \retval - 0 if equal, non-zero if not
 ******************************************************************************/
int ieeeaddr_cmp( uchar *dst, uchar *src );

/**
 *******************************************************************************
 ** \brief Function to compare two MAC addresses
 ** \param *a1 - compare address1
 ** \param *a2 - compare address2
 ** \param panid - whether to check PAN Id 
 ** \retval - 0 if equal, non-zero if not
 ******************************************************************************/
int macaddr_cmp(
					mac_address_t *a1,
					mac_address_t *a2, 
					uchar panid 
                );

/**
 *******************************************************************************
 ** \brief Function to reverse the content of the data
 ** \param *dest - pointer to the dest where the reversed data is to be stored
 ** \param *src - pointer to the source of which the data has to be reversed
 ** \param len - length of the data to be reversed
 ** \retval - None
 ******************************************************************************/
void mem_rev_cpy(uint8_t* dest, uint8_t* src, uint16_t len );

#ifdef __cplusplus
}
#endif
#endif /* MACUTILS_H */
