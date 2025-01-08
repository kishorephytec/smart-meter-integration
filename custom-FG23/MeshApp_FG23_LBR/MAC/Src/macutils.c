/** \file macutils.c
 *******************************************************************************
 ** \brief Provides utilities for MAC Layer
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
#include "queue_latest.h"
#include "phy.h"
#include "mac.h"
#include "mac_defs_sec.h"
#include "ie_element_info.h"
#include "mac_defs.h"
#include "macutils.h"

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

/* None */

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

int ieeeaddr_cmp(uchar *dst, uchar *src);
int macaddr_cmp(mac_address_t *a1, mac_address_t *a2, uchar panid );

/*
** ============================================================================
** Public Function Definitions
** ============================================================================
*/

int ieeeaddr_cmp(
					 uchar *dst, 
					 uchar *src  
                 )
{
    return memcmp( dst, src, 8);
}

/******************************************************************************/

int macaddr_cmp(
					mac_address_t *a1,
					mac_address_t *a2, 
					uchar panid 
                )
{
    /* check address mode */
	/* check PAN Id */
    if( ( a1->address_mode != a2->address_mode ) || ( panid && a1->pan_id != a2->pan_id ) )
    {
        return 1;
    }

    
    /*if( panid && a1->pan_id != a2->pan_id )
    {
        return 1;
    }
	*/

    /* check address */
    switch( a1->address_mode )
    {
    case MAC_NO_ADDRESS:
        return 0;

    case MAC_SHORT_ADDRESS:
        if( a1->address.short_address == a2->address.short_address )
        {
            return 0;
        }
        else
        {
            return 1;
        }

    case MAC_IEEE_ADDRESS:
        return memcmp( a1->address.ieee_address, a2->address.ieee_address, 8 );
    }

    return -1;
}

/******************************************************************************/

void mem_rev_cpy(uint8_t* dest, uint8_t* src, uint16_t len )
{
	uint16_t i = 0;

	for ( ; i < len; i++ )
	{
		dest[ len - i - 1 ] = src[ i ];
	}
}

/******************************************************************************/

/*
** ============================================================================
** Private Function Definitions
** ============================================================================
*/

/* None */

