/** \file buffer_service.c
 *******************************************************************************
 ** \brief Implements the buffer related services
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

/*******************************************************************************
* File inclusion
*******************************************************************************/
#include "StackPHYConf.h"
#include "common.h"
#include "queue_latest.h"
#include "list_latest.h"
#include "buff_mgmt.h"
#include "sw_timer.h"
#include "buffer_service.h"
/*
** ============================================================================
** Private Macro definitions
** ============================================================================
*/

/*None*/

/*
** ============================================================================
** Private Structures, Unions & enums Type Definitions
** ============================================================================
*/

/*None*/

/*
** ============================================================================
** Private Variable Definitions
** ============================================================================
*/

static buf_mem_pool_t buf_cfg[] = {
#if NUM_BUF_SIZE_4_BYTES != 0   	
	    { BUF_SIZE_4_BYTES, NUM_BUF_SIZE_4_BYTES},
#endif
  
#if NUM_BUF_SIZE_8_BYTES != 0   	
	    { BUF_SIZE_8_BYTES, NUM_BUF_SIZE_8_BYTES},
#endif

#if NUM_BUF_SIZE_16_BYTES != 0 	
	    { BUF_SIZE_16_BYTES, NUM_BUF_SIZE_16_BYTES},
#endif


#if NUM_BUF_SIZE_32_BYTES != 0 	
	    { BUF_SIZE_32_BYTES, NUM_BUF_SIZE_32_BYTES},
#endif

#if NUM_BUF_SIZE_64_BYTES != 0 	
	    { BUF_SIZE_64_BYTES, NUM_BUF_SIZE_64_BYTES },
#endif

#if NUM_BUF_SIZE_128_BYTES != 0 	
	    { BUF_SIZE_128_BYTES, NUM_BUF_SIZE_128_BYTES } ,
#endif

#if NUM_BUF_SIZE_192_BYTES != 0 
	    { BUF_SIZE_192_BYTES, NUM_BUF_SIZE_192_BYTES } ,
#endif

#if NUM_BUF_SIZE_256_BYTES != 0 	
	    { BUF_SIZE_256_BYTES, NUM_BUF_SIZE_256_BYTES } ,
#endif

#if NUM_BUF_SIZE_320_BYTES != 0 	
	    { BUF_SIZE_320_BYTES, NUM_BUF_SIZE_320_BYTES },
#endif	

#if NUM_BUF_SIZE_384_BYTES != 0 	
	    { BUF_SIZE_384_BYTES, NUM_BUF_SIZE_384_BYTES },
#endif	


#if NUM_BUF_SIZE_448_BYTES != 0 	
	    { BUF_SIZE_448_BYTES, NUM_BUF_SIZE_448_BYTES },
#endif	

#if NUM_BUF_SIZE_512_BYTES != 0 	
	    { BUF_SIZE_512_BYTES, NUM_BUF_SIZE_512_BYTES },
#endif	

#if NUM_BUF_SIZE_576_BYTES != 0 	
	    { BUF_SIZE_576_BYTES, NUM_BUF_SIZE_576_BYTES },
#endif	

#if NUM_BUF_SIZE_640_BYTES != 0 	
	    { BUF_SIZE_640_BYTES, NUM_BUF_SIZE_640_BYTES },
#endif	

#if NUM_BUF_SIZE_768_BYTES != 0 	
	    { BUF_SIZE_768_BYTES, NUM_BUF_SIZE_768_BYTES },
#endif	

#if NUM_BUF_SIZE_1024_BYTES != 0 	
	    { BUF_SIZE_1024_BYTES, NUM_BUF_SIZE_1024_BYTES },
#endif	

#if NUM_BUF_SIZE_1152_BYTES != 0 	
	    { BUF_SIZE_1152_BYTES, NUM_BUF_SIZE_1152_BYTES },
#endif
#if NUM_BUF_SIZE_1408_BYTES != 0 	
	    { BUF_SIZE_1408_BYTES, NUM_BUF_SIZE_1408_BYTES },
#endif	
#if NUM_BUF_SIZE_1664_BYTES != 0 	
	    { BUF_SIZE_1664_BYTES, NUM_BUF_SIZE_1664_BYTES },
#endif	
#if NUM_BUF_SIZE_1920_BYTES != 0 	
	    { BUF_SIZE_1920_BYTES, NUM_BUF_SIZE_1920_BYTES },
#endif	
#if NUM_BUF_SIZE_2100_BYTES != 0 	
	    { BUF_SIZE_2100_BYTES, NUM_BUF_SIZE_2100_BYTES },
#endif
#if NUM_BUF_SIZE_2500_BYTES != 0 	
	    { BUF_SIZE_2500_BYTES, NUM_BUF_SIZE_2500_BYTES },
#endif	


};

/*
** ============================================================================
** Public Variable Definitions
** ============================================================================
*/

uint8_t heap[BUFFER_POOL_SIZE] = {0};

const uint16_t max_buffer_size = MAX_BUFFER_SIZE;

/*
** ============================================================================
** External Variable Declarations
** ============================================================================
*/

/*None*/

/*
** ============================================================================
** Private Function Prototypes
** ============================================================================
*/

/*None*/

/*
** ============================================================================
** Public Function Definitions
** ============================================================================
*/

void buffer_service_init(void)
{
	bm_init(  BUFFER_POOL_SIZE, heap, TYPES_OF_BUFF, buf_cfg );
}

/******************************************************************************/

void * app_bm_alloc(
    uint16_t length//uint16_t length       
    )

{
   return bm_alloc(heap,length);
}

/******************************************************************************/

void app_bm_free(
    uint8_t *pMem      
    )

{
    bm_free(heap,pMem);
}


/******************************************************************************/
