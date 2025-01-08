/** \file buff_mgmt.c
 **
 ** \brief Implements the buffer management module
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
#include "buffer_service.h"

//#define BUFFER_CORRUPTION_TEST

/*
** =============================================================================
** Private Macro definitions
** =============================================================================
*/
	
/*
 *******************************************************************************
 * @brief macro defining the Magic number.
 *    Macro indicating that a buffer is free.
 *
 *******************************************************************************
*/

#define BUF_MEM_MAGIC_NUM_FREE 0x3355

/*
 *******************************************************************************
 * @brief macro defining the Magic number.
 *   Macro indicating that a buffer is in use.
 *
 *******************************************************************************
*/

#define BUF_MEM_MAGIC_NUM_USE  0xCCAA

/*
** =============================================================================
** Private Structures, Unions & enums Type Definitions
** =============================================================================
**/

/**
 *******************************************************************************
 * @brief Buffer management link structure.
 *    Structure to configure the Memory Pool.
 ******************************************************************************/

typedef struct buf_s
{
#if defined (BM_DEBUG_LEVEL_1) || defined (BM_DEBUG_LEVEL_2)
    uint16_t  magic_num; /**<0x33 for free, 0xAA for used */
    uint16_t  line_num; /**< It stores the Line number of the last operation 
						      done with the buffer*/
#if defined (BM_DEBUG_LEVEL_2)
    uint16_t  pFunc_name; /**< pointer to a string where the function name
                                     is stored. */
#endif
#endif
    queue_item_t * pNext; /**< pointer to link to next buffer */
}buf_t;


typedef struct debug_heap_tag
{
  uint32_t typesOfBuffs;
  pool_t pools[TYPES_OF_BUFF];  //Debdeep
}debug_heap_t;

/*
** =============================================================================
** Private Variable Definitions
** =============================================================================
*/

static debug_heap_t* p_debug_heap = NULL; 

/*
** =============================================================================
** Private Function Prototypes
** =============================================================================
*/

/* None */

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

bool bm_init(
    uint32_t heap_size,
    uint8_t *pHeap,
    uint32_t num_of_types,
    buf_mem_pool_t *pbuf_cfg
    )
{
    uint32_t i = 0, j  = 0;
    p_debug_heap = (debug_heap_t*)pHeap;
    uint32_t *pBuffTypes = (uint32_t*)(p_debug_heap);//( uint16_t* )pHeap;
    
    pool_t *pPool = &(p_debug_heap->pools[0]);//( pool_t * )&pBuffTypes[1];
    queue_t *pQueue = NULL;
    queue_item_t *pItem = NULL;
    /* Check the heap size is enough or not */
    uint32_t req_size, heap_header_size = sizeof(uint32_t) + 
        ( sizeof( pool_t ))* ( num_of_types );
    
    //Use i for aligning
    i = sizeof( uint32_t ) - 1;
    heap_header_size = ( heap_header_size + i ) & ( ~i );

    req_size = heap_header_size;

    for( i = 0; i < num_of_types; i++ )
    {
        req_size += ( sizeof(buf_t) + pbuf_cfg[ i ].buf_size ) * 
            pbuf_cfg[ i ].num_of_buf;
    }
    
    if( heap_size < req_size ) 
      return FALSE;

    /* Initialize the heap */
    /* Update the number of types */
    pBuffTypes[0] = num_of_types;
    /* Update the offset of each buffer pool */
    pPool[0].offset = heap_header_size;
    pPool[0].size = pbuf_cfg[ 0 ].buf_size;
    for( i = 1; i < num_of_types; i++ )
    {
        pPool[i].offset = pPool[i-1].offset + 
            ( sizeof(buf_t) + pbuf_cfg[ i - 1 ].buf_size ) * 
                pbuf_cfg[ i - 1 ].num_of_buf;
        pPool[i].size = pbuf_cfg[ i ].buf_size;
    }
    /* Initialize the queues */
    
    pItem = ( queue_item_t* )( pHeap + pPool[0].offset );
    for( i = 0; i < num_of_types ; i++ )
    {
        pQueue = &pPool[ i ].q;
        //Initialize the free queue
        queue_initialise( pQueue );
        
        for( j =0 ; j < pbuf_cfg[ i ].num_of_buf; j++ )
        {
            //Put all the item into free list
            queue_item_put( pQueue, pItem );
#if defined (BM_DEBUG_LEVEL_1) || defined (BM_DEBUG_LEVEL_2)
            {
                buf_t *pBuf = ( buf_t* )pItem;
                pBuf->magic_num = BUF_MEM_MAGIC_NUM_FREE;
            }
#endif
            pItem = ( queue_item_t* )(( uint8_t* )pItem + sizeof( buf_t ) + 
                pbuf_cfg[ i ].buf_size );
        }        
    }
    
   
    return TRUE;
}

/*----------------------------------------------------------------------------*/

#ifdef BUFFER_CORRUPTION_TEST

#define NUMBER_OF_TEST_BUFFER           10
#define BUFFER_SIZE                     1920
#define TEST_BUFF_OFFSET                51812
#define NEXT_TO_TEST_BUFF_OFFSET        99812

struct Track_64_buff
{
  uint32_t alloc[NUMBER_OF_TEST_BUFFER];
  uint32_t free[NUMBER_OF_TEST_BUFFER];
  uint32_t alloc_index;
  uint8_t alloc_loop;
  uint32_t free_index;
  uint8_t free_loop;
};
struct Track_64_buff track_64_buff;
uint8_t present_in_alloc_buf (uint32_t mem)
{
  int ii;
  
  for (ii = 0; ii < NUMBER_OF_TEST_BUFFER; ii++)
  {
    if (track_64_buff.alloc[ii] == mem)
    {
      track_64_buff.alloc[ii] = 0;
      return 1;
    }
  }  
  return 0;
}
                                 
void sort_alloc_array (void)
{
 int ii;
 track_64_buff.alloc_index = 0;
 for(ii = 0; ii < NUMBER_OF_TEST_BUFFER; ii++)
 {
   if (track_64_buff.alloc[ii] != 0)
   {
     if (track_64_buff.alloc_index != ii)
     {
       track_64_buff.alloc[track_64_buff.alloc_index] = track_64_buff.alloc[ii];
       track_64_buff.alloc[ii] = 0;
     }
     track_64_buff.alloc_index++;
   }
 }
}
#endif  //BUFFER_CORRUPTION_TEST

uint32_t bigger_buff_alloc_count;


#if defined BM_DEBUG_LEVEL_1
void bm_alloc(
    uint8_t *pHeap,
    uint16_t length,
    uint16_t line_num
    )

#elif defined BM_DEBUG_LEVEL_2
void bm_alloc(
    uint8_t *pHeap,
    uint16_t length,
    uint16_t line_num,
    uint8_t *pFunc_name
    )
#else
void * bm_alloc(
    uint8_t *pHeap,
    uint16_t length//uint16_t length       
    )
#endif
{
  uint32_t *pBufTypes = ( uint32_t* )pHeap;
  uint32_t i = 0;
  pool_t *pPool = ( pool_t * )&pBufTypes[1];    
  uint8_t *pMem = NULL;
  
  if ( !length)
  {
    return NULL;
  }
  
  for( i = 0; i < *pBufTypes; i++ )
  {
    /*Check if the requested buffer size is well within the limit of the 
    buffer type available AND there are buffers available in that pool*/  
    if ((length <= pPool[ i ].size ) && queue_count_get( &pPool[ i ].q )
        && (pPool[ i ].q.start != NULL))
    {          
      if ((length > 384) && (length <= 448) && (pPool[ i ].size > 448))
        bigger_buff_alloc_count++;
      
      pMem = ( uint8_t* )( queue_item_get( &pPool[ i ].q ));
      
#ifdef BUFFER_CORRUPTION_TEST
      if (pPool[ i ].size == BUFFER_SIZE)
      {
        track_64_buff.alloc[track_64_buff.alloc_index] = (uint32_t)pMem;
        track_64_buff.alloc_index++;
        if (track_64_buff.alloc_index >= NUMBER_OF_TEST_BUFFER)
        {
          track_64_buff.alloc_loop++;
          track_64_buff.alloc_index = 0;
        }
      }
#endif  //BUFFER_CORRUPTION_TEST
//#if ((PRINT_DEBUG_LEVEL==ERROR_DEBUG)||(PRINT_DEBUG_LEVEL==CONSOLE_ERROR_DEBUG))      
//      if (queue_count_get( &pPool[ i ].q ) == 0)
//        stack_print_debug ("%d size buffer exhausted\n", pPool[ i ].size);
//#endif
      
      if( pMem != NULL)
      {
#if defined (BM_DEBUG_LEVEL_1) || defined (BM_DEBUG_LEVEL_2)
        {
          buf_t *pBuf = NULL;
          pMem = ( pMem - sizeof( buf_t ) + 
                  sizeof( queue_item_t *);
                   pBuf = ( buf_t* )pMem;
                   pBuf->magic_num = BUF_MEM_MAGIC_NUM_USE;
                   pBuf->line_num = line_num;
#if defined (BM_DEBUG_LEVEL_2)
                   pBuf->pFunc_name = pFunc_name;
#endif
        }
#endif
                   pMem += sizeof( buf_t );
                   memset((uint8_t *)pMem, 0xFF, pPool[ i ].size);
      } 
      return ( void* )pMem;
    }
  }
  return ( void* )pMem;
}

/*----------------------------------------------------------------------------*/  
#if defined BM_DEBUG_LEVEL_1
void bm_free(
    uint8_t *pHeap,
    uint8_t *pMem,
    uint16_t line_num
    )

#elif defined BM_DEBUG_LEVEL_2
void bm_free(
    uint8_t *pHeap,
    uint8_t *pMem,
    uint16_t line_num,
    uint8_t *pFunc_name
    )

#else
/*----------------------------------------------------------------------------*/
void bm_free(
    uint8_t *pHeap,
    uint8_t *pMem      
    )
#endif
     {
       uint32_t *pBufTypes = ( uint32_t* )pHeap;
       uint16_t i = 0;
       pool_t *pPool = ( pool_t * )&pBufTypes[1];
       int offset = 0;
       queue_item_t* pItem = NULL;
       
       pItem = ( queue_item_t* )( pMem - sizeof( buf_t ));
       
       if( pHeap < pMem )
         offset = pMem - pHeap;
       else
       {
         return;
       }
       
       if (offset >= BUFFER_POOL_SIZE)
         return;
       
       for( i = 0; i < *pBufTypes - 1; i++ )
       {        
         if( offset < pPool[i + 1].offset )
         {
           memset( (uint8_t*)pMem,0,pPool[ i ].size);
           break;
         }
       }
       
#ifdef BUFFER_CORRUPTION_TEST    
       if ((offset >= TEST_BUFF_OFFSET) && (offset < NEXT_TO_TEST_BUFF_OFFSET))
       {
         track_64_buff.free[track_64_buff.free_index] = (uint32_t)pItem;
         track_64_buff.free_index++;
         if (track_64_buff.free_index >= NUMBER_OF_TEST_BUFFER)
         {
           track_64_buff.free_loop++;
           track_64_buff.free_index = 0;
         }
         
         if (present_in_alloc_buf((uint32_t)pItem))
         {
           track_64_buff.free_index--;
           if (track_64_buff.free_index <= 0)
             track_64_buff.free_index = 0;
           track_64_buff.free[track_64_buff.free_index] = 0;
         }
         
         sort_alloc_array ();
       }
#endif  //BUFFER_CORRUPTION_TEST
       
       queue_item_put( &pPool[i].q , pItem );
       
       
#if defined (BM_DEBUG_LEVEL_1) || defined (BM_DEBUG_LEVEL_2)
       {
         buf_t *pBuf = ( buf_t* )(( uint8_t* )pItem - sizeof( buf_t ) + 
                                  sizeof( queue_item_t* ));
         pBuf->magic_num = BUF_MEM_MAGIC_NUM_FREE;
         pBuf->line_num = line_num;
#if defined (BM_DEBUG_LEVEL_2)
         pBuf->pFunc_name = pFunc_name;
#endif
       }
#endif
     }


#if(0)
/*----------------------------------------------------------------------------*/
uint16_t bm_calc_heap_size(
    uint16_t num_of_types,
    buf_mem_pool_t *pbuf_cfg
    )
{
    uint16_t i = 0;
    uint16_t req_size = sizeof(uint16_t) + 
        sizeof(pool_t) * ( num_of_types ) + 
        sizeof(queue_t) * num_of_types;
    
    for( i = 0; i < num_of_types; i++ )
    {
        req_size += ( sizeof(buf_t) + pbuf_cfg[ i ].buf_size ) * 
            pbuf_cfg[ i ].num_of_buf;
    }
    return req_size;
}

#endif
/*----------------------------------------------------------------------------*/
   
/*
** =============================================================================
** Private Function Definitions
** =============================================================================
*/                           
                   
//static queue_t* get_pool(uint8_t i)
//{
//  return &(p_debug_heap->pools[1].q);
//}
                                 
/*----------------------------------------------------------------------------*/