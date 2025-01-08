/** \file buff_mgmt.h
 *******************************************************************************
 ** \brief  Provides Buffer Interface APIs for allocating and freeing the buffer 
 **    maintained by buffer management module.
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

#ifndef _BUFF_MGMT_H_
#define _BUFF_MGMT_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 *****************************************************************************
 * @ingroup sysdoc
 *
 * @{
 *****************************************************************************/

/**
 *****************************************************************************
 * @defgroup bmm Buffer Management 
 * @brief This module provides dynamic memory allocation feature from 
 *    statically allocated heap.
 *
 *    The buffer management provides a way of allocating memory dynamicaly 
 *    from a reserved memory block. The buffer management provides three APIs.
 *    One to initialize the buffer management module, other two are to 
 *    allocate and free memory.
 *        
 *    User has to initialize the buffer management module first, before using the
 *    MALLOC and FREE APIs.To initialize, it has to call bm_init API passing the
 *    pointer to the heap (statically allocated memory) and the configuration
 *    structure. The configuration structure takes the buffer size and number
 *    of buffers. An example is given below:-\n
 *
 *    const buf_mem_pool_t buf_cfg[4] = { \n
 *    { 16, 10 }, - buffer length is 16 and there would be 10 buffers of this size\n
 *    { 32, 10 }, - buffer length is 32 and there would be 10 buffers of this size\n
 *    { 64, 10 }, - buffer length is 64 and there would be 10 buffers of this size\n
 *    { 128, 10 }};- buffer length is 128 and there would be 10 buffers of this size\n
 *    
 *    uint8_t heap[5000];\n
 *    bm_init(5000, heap, 4, buf_cfg ); 
 *
 *    Once the buffer management is initialied, one can allocate by using MALLOC 
 *    MACRO and free the buffer using FREE MACRO.
 *    MALLOC and FREE takes the address of heap.
 *    One can have multiple instance of buffer management. They are distinguised 
 *    from the heap address.
 *
 *    To debug memory leak issue the buffer management module provides two 
 *    flavours  of debugging mechanism. One can enable defining it using
 *    BM_DEBUG_LEVEL_1 or BM_DEBUG_LEVEL_2 MACROs.\n
 *    
 *    The buffer management uses one words to link the buffers if no debug is  
 *    enabled. If BM_DEBUG_LEVEL_1 is enabled, then it uses 2 words and if  
 *    BM_DEBUG_LEVEL_2 is enabled it uses 3 words per buffer.
 *    Depending on the memory space available user can enable either no debug
 *    BM_DEBUG_LEVEL_1 or BM_DEBUG_LEVEL_2. \n 
 *
 *    If BM_DEBUG_LEVEL_1 is enabled, apart from link pointer the buffer 
 *    management stores a MAGIC number of 2 bytes to indicate the buffer is 
 *    free or used. In case of free the value is 0x3355 and for use it is 
 *    0xCCAA. One can easily find the boundary by locating these values. It 
 *    also stores the Line number of the last operation done with the buffer,
 *    the line number where the buffer got allocated if it is used else it is
 *    the line numnber where the buffer got freed.\n
 *
 *    If BM_DEBUG_LEVEL_2 is enabled apart from the MAGIC number and line
 *    number it also stores the pointer to a string where the function name
 *    is stored. When this is enbaled it requires more heap area also it 
 *    consumes FLASH (rodata) segment to store the functiuon names.
 *
 *    The buffer looks as shown below
 *    \verbatim
 *           ________________   ------
 *          |      LINK      |  ALLWAY THERE 
 *          |----------------|  ------
 *          |  MAGIC NUM     |  IF 
 *          |  0x3355/0xCCAA |  BM_DEBUG_LEVEL_1 or BM_DEBUG_LEVEL_2
 *          |----------------|  IS ENABLED
 *          |    LINE NUM    |          
 *          |----------------|  ------
 *          |  POINTER TO    |  IF
 *          |  FUNC NAME     |  BM_DEBUG_LEVEL_2
 *          |                |  IS ENABLED
 *          |----------------|<-\
 *          |                |   \
 *          |  BUFFER        |    \ALLOCATED BUFFER PTR
 *          |                |
 *          |----------------|
 *    \endverbatim
 *
 *
 *    
 ******************************************************************************/

/**
 *******************************************************************************
 * 
 * @}   
 ******************************************************************************/

/*
** ============================================================================
** Public Macro definitions
** ============================================================================
*/

/**
 ** \defgroup bmm_defs  BMM Definitions
 ** \ingroup bmm
 */

/*@{*/

/* None */

#if(0)

/**
 *******************************************************************************
 * @brief macro defining the DEBUG level.
 *    MACROs to enable debug level.
 * @note
 *    Uncommnet one of them to enbale debuging. The description of these MACRO 
 *    is explained above.
 ******************************************************************************/	
#define BM_DEBUG_LEVEL_1
#define BM_DEBUG_LEVEL_2

#endif

/*
** ============================================================================
** Public Structures, Unions & enums Type Definitions
** ============================================================================
*/

/**
 *******************************************************************************
 * @brief Buffer management configuration structure.
 *    Structure to configure the Memory Pool.
 ******************************************************************************/
typedef struct buf_mem_pool_s
{
    
    uint16_t buf_size;/**< Size of the buffer in bytes, it should be divisible by 
                                sizeof(uint16_t) */   
    uint16_t num_of_buf; /**< Number of buffers */
}buf_mem_pool_t;

/**
 *******************************************************************************
 * @brief Buffer management configuration structure.
 *    Structure to configure the Memory Pool which holds the offset address of 
 *    next pool and the size of its pool along with the holdind the addresses
 *    of each buffer in the queue.
 ******************************************************************************/
typedef struct pool_s
{
    uint32_t  offset; /**< 0x33 for free, 0xAA for used */
    uint16_t  size;  /**< buffer size */
    queue_t   q; /**< queue to be intialised */
}pool_t;


/*@}*/

/*
** ============================================================================
** Public Variable Declarations
** ============================================================================
*/

/*None*/

/*
** ============================================================================
** Public Function Prototypes
** ============================================================================
*/

/**
 ** \defgroup bmm_req  BMM APIs
 ** \ingroup bmm
 */

/*@{*/

/**
 *******************************************************************************
 * @brief Initialization for Buffer management module.
 *    This function initializes the buffer management.
 * @param heap_size - IN Heap size used.
 * @param pHeap - IN Pointer to the heap.
 * @param num_of_types - IN number of different types of buffer.
 * @param pbuf_cfg - IN Pointer to the buffer configuration.
 * @retval returns TRUE(1) if the initialization procedure is successful. 
 * @retval returns FALSE(0) if the size of the heap is less than the 
 *    required size calculated from the buffer configuarion given.
 ******************************************************************************/
bool bm_init(
    uint32_t heap_size, uint8_t *pHeap, uint32_t num_of_types,
    buf_mem_pool_t *pbuf_cfg  );


 /**
 *******************************************************************************
 * @brief Allocates memory.
 *    This function allocates memory based on the size passed.
 * @param heap - Input pointer to the heap.
 * @param size - Size of the memory to be allocated
 * @retval Returns a valid pointer if the ALLOCATION procedure is successful. 
 *        Else returns NULL. It returns NULL if the it cannot allocate memory.
 * @note
 *    1)This function can be used only after initialising a heap.\n 2)The size 
 *   of the memory to be allocated should be with in the limit of the heap size. 
 ******************************************************************************/
#if defined BM_DEBUG_LEVEL_1
#define MALLOC(heap, size) bm_alloc(heap, size, __LINE__)

#elif defined BM_DEBUG_LEVEL_2
#define MALLOC(heap, size) bm_alloc(heap, size, __LINE__, __FUNCTION__)

#else
#define MALLOC(heap, size) bm_alloc(heap, size)

#endif

 /**
 *****************************************************************************
 * @brief Frees the memory.
 *    This function Fress up the memory.
 * @param heap - Input pointer to the heap.
 * @param ptr - Input pointer to the memory to be freed. 
 * @retval None
 * @note
 *    1)This function can be used only after initialising a heap.\n 2)The ptr 
 *  should be a valid pointer received from the MALLOC function call.
 *****************************************************************************/
#if defined BM_DEBUG_LEVEL_1
#define FREE(heap, ptr)    bm_free(heap, ptr, __LINE__)

#elif defined BM_DEBUG_LEVEL_2
#define FREE(heap, ptr)    bm_free(heap, ptr, __LINE__, __FUNCTION__)

#else
#define FREE(heap, ptr) bm_free(heap, ptr)

#endif

 /**
 *****************************************************************************
 * @brief Find the required heap size based on the configuration.
 *    This function calculates the heap size required for the configuration 
 *    passed
 * @param num_of_types - IN number of different types of buffer.
 * @param pbuf_cfg - IN Pointer to the buffer configuration.
 * @retval heap size needed in terms of number of bytes.
 *****************************************************************************/
// Sagar: Not Used
/*uint16_t bm_calc_heap_size(
    uint16_t num_of_types, buf_mem_pool_t *pbuf_cfg );*/


/**
 * \brief Allocates a block of memory.
 * \param pHeap - Input pointer to the heap.
 * \param length - length of the requested buffer in bytes.
 * \retval the address of the buffer allocated, 
 * \note returns NULL if failure
 **/
#if defined BM_DEBUG_LEVEL_1
void bm_alloc(
    uint8_t *pHeap, uint16_t length, uint16_t line_num );

#elif defined BM_DEBUG_LEVEL_2
void bm_alloc(
    uint8_t *pHeap, uint16_t length, uint16_t line_num, uint8_t *pFunc_name );
#else
void * bm_alloc(
	 uint8_t *pHeap,
     uint16_t length);//uint16_t length );
#endif

#if defined BM_DEBUG_LEVEL_1
void bm_free(
    uint8_t *pHeap, uint8_t *pMem, uint16_t line_num );

#elif defined BM_DEBUG_LEVEL_2
void bm_free(
    uint8_t *pHeap, uint8_t *pMem, uint16_t line_num, uint8_t *pFunc_name );

#else
void bm_free(
	uint8_t *pHeap,
    uint8_t *pMem );
#endif

/*@}*/

#ifdef __cplusplus
}
#endif
#endif /* _BUFF_MGMT_H_ */
