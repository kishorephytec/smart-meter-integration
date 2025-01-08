/** \file ccasm.c
 *******************************************************************************
 ** \brief This file provides differrnt CCA state machine details.
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

#if(FAN_FRQ_HOPPING_FEATURE_ENABLED == 1)

#include "tri_tmr.h"
#include "common.h"
#include "queue_latest.h"
#include "buff_mgmt.h"
#include "list_latest.h"
#include "sw_timer.h"
#include "phy.h"
#include "mac_config.h"
#include "mac.h"
#include "mac_defs_sec.h"
#include "ie_element_info.h"
#include "mac_defs.h"
#include "mac_pib.h"
#include "sm.h"
//#include "fec.h"
#include "fan_mac_ie.h"
#include "timer_service.h"



/*
** ============================================================================
** Public Function Prototypes
** ============================================================================
*/

uint32_t hashword( const uint32_t *k, size_t length, uint32_t initval);

/*
** ============================================================================
** Private Function Prototypes
** ============================================================================
*/
/*Umesh : 21-12-2017*/
//static int nextPrimeNumber(uint16_t t);
/*this varriable not used any where*/
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

/*None*/

/*
** ============================================================================
** Private Function Definitions
** ============================================================================
*/

/*None*/

/*
** ============================================================================
** Public Function Definitions
** ============================================================================
*/

/*the pseudorandom hopping sequence is comprised of 2^16 slots (numbered 0 to 2^16-1). 
The total sequence duration is thus 2^16 * dwell interval. 
The current slot number is the current position in the hopping sequence.*/
int getUnicastChannelIndex(uint16_t slotNumber, uint8_t* MACAddr, uint16_t nChannels)
{
     uint16_t channelIndex;
     uint32_t x[3];
     x[0] = (uint32_t)slotNumber;
     x[1] = (uint32_t)MACAddr[4] << 24 | (uint32_t)MACAddr[5] << 16 | (uint32_t)MACAddr[6] <<8 | (uint32_t)MACAddr[7];
     x[2] = (uint32_t)MACAddr[0] << 24 | (uint32_t)MACAddr[1] << 16 | (uint32_t)MACAddr[2] <<8 | (uint32_t)MACAddr[3];

     channelIndex = hashword(x,3,0) % nChannels;
     return(channelIndex);
}

/******************************************************************************/
int getBroadcastChannelIndex(uint16_t slotNumber, uint16_t broadcastSchedID, uint16_t nChannels)
{
     uint16_t channelIndex;
     uint32_t x[3];
     x[0] = (uint32_t)slotNumber;
     x[1] = (uint32_t)broadcastSchedID << 16;
     x[2] = 0;
     channelIndex = hashword(x,3,0) % nChannels;
     return(channelIndex);
}
/******************************************************************************/


#endif

