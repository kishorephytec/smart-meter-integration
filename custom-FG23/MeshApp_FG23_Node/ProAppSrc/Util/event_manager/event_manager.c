/** \file event_manager.c
 *******************************************************************************
 ** \brief Implements the Event Manager Functionality
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
#include "event_manager.h"


/*
** =============================================================================
** Private Macro definitions
** =============================================================================
*/
	
#if ( MAX_EVENT_PRIO < 65 )
#define GRP1_SIZE 1
#define GRP_LEVEL 1

#elif ( MAX_EVENT_PRIO > 64 ) && ( MAX_EVENT_PRIO < 513 ) 
// Raka  .. This is not used in current code base
#define GRP1_SIZE ( MAX_EVENT_PRIO >> 6 )
#define GRP2_SIZE 1
#define GRP_LEVEL 2
///////////Raka .. ends .....

#elif ( MAX_EVENT_PRIO > 512 ) && ( MAX_EVENT_PRIO < 4097 ) 
// Raka  .. This is not used in current code base
#define GRP1_SIZE ( MAX_EVENT_PRIO >> 6 )
#define GRP2_SIZE ( MAX_EVENT_PRIO >> 6 ) >> 3
#define GRP3_SIZE 1
#define GRP_LEVEL 3
///////////Raka .. ends .....
#endif

/* Size of ready table */
#if ( GRP_LEVEL == 1 )
#define MIN_INDEX  ( MAX_EVENT_PRIO >> 3 )

#if MAX_EVENT_PRIO > ( MIN_INDEX << 3 )
#define RDY_TBL_SIZE   (( MAX_EVENT_PRIO >> 3 ) + 1 + 1 )
#else
// Raka  .. This is not used in current code base
#define RDY_TBL_SIZE   (( MAX_EVENT_PRIO >> 3 ) + 1 )
#endif

#elif ( GRP_LEVEL == 2 )
// Raka  .. This is not used in current code base
#define  RDY_TBL_SIZE   ((MAX_EVENT_PRIO) >> 3 + GRP1_SIZE + 1 )
#elif ( GRP_LEVEL == 3 )
// Raka  .. This is not used in current code base
#define RDY_TBL_SIZE   ((MAX_EVENT_PRIO) >> 3 + GRP1_SIZE + \
    GRP2_SIZE + 1 )
#elif
#error NOT SUPPORTED
#endif


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

/**< N-bit bit mask for supporting N threads with indicating which of the 
threads are in ready state and ready to be executed by the scheduler*/
static uint8_t event_queue[RDY_TBL_SIZE] = {0}; 

/**< Look up table to get the Lowest Bit Set in a byte */
static uint8_t const  lbs_lookup_tbl[] = {
    0, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0x00 to 0x0F */
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0x10 to 0x1F */
    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0x20 to 0x2F */
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0x30 to 0x3F */
    6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0x40 to 0x4F */
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0x50 to 0x5F */
    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0x60 to 0x6F */
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0x70 to 0x7F */
    7, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0x80 to 0x8F */
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0x90 to 0x9F */
    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0xA0 to 0xAF */
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0xB0 to 0xBF */
    6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0xC0 to 0xCF */
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0xD0 to 0xDF */
    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0xE0 to 0xEF */
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0        /* 0xF0 to 0xFF */
};

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
extern void signal_event_to_mac_task(uint8_t event);
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

uint32_t highest_prio_event_get( void )
{
    uint32_t prio = 0, level1 = 0;
    
    if( 0 == event_queue[0] ) return MAX_BASE_VALUE;
#if ( GRP_LEVEL == 1 )
    level1 = lbs_lookup_tbl[event_queue[0]];
    prio = ( level1 << 3 ) + lbs_lookup_tbl[event_queue[level1 + 1 ]];
#elif ( GRP_LEVEL == 2 )
    
     // Raka  .. This is not used in current code base
    uint32_t level2 = lbs_lookup_tbl[event_queue[0]];
    level1 = lbs_lookup_tbl[event_queue[level2 + 1 ]];
    prio = ( level2 << 3 ) << 3 + level1 << 3 +
        lbs_lookup_tbl[event_queue[GRP1_SIZE+1+level1]];
#elif ( STS_GRP_LEVEL == 3 )
    
     // Raka  .. This is not used in current code base
    uint32_t level3 = lbs_lookup_tbl[event_queue[0]];
    uint32_t level2 = lbs_lookup_tbl[event_queue[ 1 + level3 ]];
    level1 = lbs_lookup_tbl[event_queue[GRP2_SIZE + 1 + level2 ]];
    prio = (( level3 << 3 ) << 3 ) << 3 + ( level2 << 3 ) << 3 + ( level1 << 3 )
        lbs_lookup_tbl[event_queue[GRP2_SIZE + GRP1_SIZE + 1 + level1]];
#endif
    

    return prio;
}

/*----------------------------------------------------------------------------*/

void event_set( uint32_t prio )
{
    uint32_t level1  = 0, reminder = prio;
    
//    irq_state_t flags = __get_interrupt_state();//Umesh

//    flags = irq_disable();
        
       
#if ( GRP_LEVEL == 1 )
    level1 = prio >> 3;
    reminder -= ( level1 << 3 );
    event_queue[ 0 ] |= ( 1 << level1 );
    event_queue[ level1 + 1 ] |= ( 1 << reminder );
#elif ( GRP_LEVEL == 2 )
   // Raka  .. This is not used in current code base
    uint32_t level2 = prio >> 3;
    reminder -= ( level2 << 3 );
    level1 = reminder >> 3;
    reminder -= ( level1 << 3 );
    event_queue[ 0 ] |= level2;
    event_queue[level2 + 1 ] |= level1;
    event_queue[GRP1_SIZE + 1 + level1] |= reminder;
#elif ( GRP_LEVEL == 3 )
    // Raka  .. This is not used in current code base
    uint32_t level3 = prio >> 3;
    uint32_t level2;
    reminder -= ( level3 << 3 );
    level2 = reminder >> 3;
    reminder -= ( level2 << 3 );
    level1 = reminder >> 3;
    reminder -= ( level1 << 3 );
    event_queue[ 0 ] |= level3;
    event_queue[ 1 + level3 ] |= level2;
    event_queue[GRP2_SIZE + 1 + level2 ] |= level1;
    event_queue[GRP2_SIZE + GRP1_SIZE + 1 + level1] |= reminder;
#endif

//    irq_enable( flags );

    signal_event_to_mac_task((uint8_t)prio);
        
  
}

/*----------------------------------------------------------------------------*/

void event_clear( uint32_t prio )
{
    uint32_t level1 = 0, reminder = prio;
    
//    irq_state_t flags = __get_interrupt_state();//Umesh;;

//    flags = irq_disable();
        
    
#if ( GRP_LEVEL == 1 )
    level1 = prio >> 3;
    reminder -= ( level1 << 3 );
    
    event_queue[ level1 + 1 ] &= ~( 1 << reminder );
    if( 0 == event_queue[ level1 + 1 ] )
        event_queue[ 0 ] &= ~( 1 << level1 );
#elif ( GRP_LEVEL == 2 )
    
     // Raka  .. This is not used in current code base
    uint32_t level2 = prio >> 3;
    reminder -= ( level2 << 3 );
    level1 = reminder >> 3;
    reminder -= ( level1 << 3 );
    event_queue[ 0 ] &= ~level2;
    event_queue[level2 + 1 ] &= ~level1;
    event_queue[GRP1_SIZE + 1 + level1] &= ~reminder;
#elif ( GRP_LEVEL == 3 )
    
     // Raka  .. This is not used in current code base
    uint32_t level3 = prio >> 3;
    uint32_t level2;
    reminder -= ( level3 << 3 ); 
    level2 = reminder >> 3;
    reminder -= ( level2 << 3 ); 
    level1 = level2 >> 3;
    reminder -= ( reminder << 3 ); 
    event_queue[ 0 ] &= ~level3;
    event_queue[ 1 + level3 ] &= ~level2;
    event_queue[GRP2_SIZE + 1 + level2 ] &= ~level1;
    event_queue[GRP2_SIZE + GRP1_SIZE + 1 + level1] &= ~reminder;
#endif
     
//    irq_enable( flags );
}
/*----------------------------------------------------------------------------*/


