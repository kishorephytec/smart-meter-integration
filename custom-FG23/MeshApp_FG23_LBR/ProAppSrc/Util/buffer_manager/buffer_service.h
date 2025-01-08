/** \file buffer_service.h
 *******************************************************************************
 ** \brief Implements the Buffer related services
 **  This is a buffer management configuration file. Following are the different 
 **  configuration parameters provided which shold be tweaked as per the 
 **  application needs and the MAC-PHY capabilities w.r.t the maximum sized PPDU 
 **  it can support.
 **  1) Configuration parameters for defining the number of buffers of a 
 **   particular size
 **  2)  Configuration defining the total number of different sized buffers that 
 **  BMM should be handling
 **  3) Configuration defining the maximum sized buffer that is made available 
 **  in the BMM using the configuration done as indicated in (1). While defining 
 **  this parameter, the user should be aware of the maximum size of the PPDU 
 **  supported by the MAC-PHY
 **    
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

#ifndef BUFFER_SERVICE_H
#define BUFFER_SERVICE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "fan_config_param.h"  
/*
** ============================================================================
** Public Macro definitions
** ============================================================================
*/

/**
 ** \defgroup buffer_service  Buffer Service Module
 */

/*@{*/

/**
 ** \defgroup buffer_def Buffer Service Definitions
 ** \ingroup buffer_service
 */

/*@{*/

enum
{
	BUF_SIZE_8_BYTES = 8,
	BUF_SIZE_16_BYTES = 16,
	BUF_SIZE_32_BYTES = 32,
	BUF_SIZE_64_BYTES = 64,
	BUF_SIZE_128_BYTES = 128,
	BUF_SIZE_192_BYTES = 192,
	BUF_SIZE_256_BYTES = 256,
	BUF_SIZE_320_BYTES = 320,
	BUF_SIZE_384_BYTES = 384,
	BUF_SIZE_448_BYTES = 448,
	BUF_SIZE_512_BYTES = 512,
	BUF_SIZE_576_BYTES = 576,
	BUF_SIZE_640_BYTES = 640,
	BUF_SIZE_768_BYTES = 768,
	BUF_SIZE_1024_BYTES = 1024,
	BUF_SIZE_1152_BYTES = 1152,
	BUF_SIZE_1408_BYTES = 1408,
	BUF_SIZE_1664_BYTES = 1664,
	BUF_SIZE_1920_BYTES = 1920,
	BUF_SIZE_2100_BYTES = 2100,
	BUF_SIZE_2300_BYTES = 2300

};



// configuration for the Wi-SUN ENET IoT & Conformance Test application 

#define NUM_BUF_SIZE_8_BYTES				16
#define NUM_BUF_SIZE_16_BYTES				16
#define NUM_BUF_SIZE_32_BYTES				7

#if (EFR32FG13P_LBR == 1)
#define NUM_BUF_SIZE_64_BYTES				10
#else
#define NUM_BUF_SIZE_64_BYTES				7
#endif

#if (EFR32FG13P_LBR == 1)
#define NUM_BUF_SIZE_128_BYTES				30
#else
#define NUM_BUF_SIZE_128_BYTES				7
#endif

#if (EFR32FG13P_LBR == 1)
#define NUM_BUF_SIZE_192_BYTES				15
#else
#define NUM_BUF_SIZE_192_BYTES				3
#endif

#define NUM_BUF_SIZE_256_BYTES				3
#define NUM_BUF_SIZE_320_BYTES				4
#define NUM_BUF_SIZE_384_BYTES				4
#define NUM_BUF_SIZE_448_BYTES				2
#define NUM_BUF_SIZE_512_BYTES				3
#define NUM_BUF_SIZE_576_BYTES				3
#define NUM_BUF_SIZE_640_BYTES				0
#define NUM_BUF_SIZE_768_BYTES				2
#define NUM_BUF_SIZE_1024_BYTES				2
#define NUM_BUF_SIZE_1152_BYTES				0
#define NUM_BUF_SIZE_1408_BYTES 			0
#define NUM_BUF_SIZE_1664_BYTES				1
#define NUM_BUF_SIZE_1920_BYTES				0
#define NUM_BUF_SIZE_2100_BYTES				0
#define NUM_BUF_SIZE_2300_BYTES				0


#define TYPES_OF_BUFF						15 //(12-1)

//UART buffer size

//#define MAX_BUFFER_SIZE                          BUF_SIZE_2300_BYTES

#define MAX_BUFFER_SIZE                          BUF_SIZE_1664_BYTES



#define BUFFER_HEADER  (4 + (sizeof(pool_t) * TYPES_OF_BUFF))

/**< Indicates Total Buffer Size  */
#define BUFFER_POOL_SIZE	(200 + BUFFER_HEADER + ((BUF_SIZE_8_BYTES + 4)* NUM_BUF_SIZE_8_BYTES) + \
							((BUF_SIZE_16_BYTES + 4) * NUM_BUF_SIZE_16_BYTES) + \
							((BUF_SIZE_32_BYTES + 4) * NUM_BUF_SIZE_32_BYTES) + \
							((BUF_SIZE_64_BYTES + 4) * NUM_BUF_SIZE_64_BYTES) + \
							((BUF_SIZE_128_BYTES + 4)* NUM_BUF_SIZE_128_BYTES) + \
							((BUF_SIZE_192_BYTES + 4)* NUM_BUF_SIZE_192_BYTES) +\
							((BUF_SIZE_256_BYTES + 4)* NUM_BUF_SIZE_256_BYTES) + \
							((BUF_SIZE_320_BYTES + 4)* NUM_BUF_SIZE_320_BYTES) + \
							((BUF_SIZE_384_BYTES + 4)* NUM_BUF_SIZE_384_BYTES) +\
							((BUF_SIZE_448_BYTES + 4)* NUM_BUF_SIZE_448_BYTES) +\
							((BUF_SIZE_512_BYTES + 4)* NUM_BUF_SIZE_512_BYTES)+\
							((BUF_SIZE_576_BYTES + 4)* NUM_BUF_SIZE_576_BYTES)+\
							((BUF_SIZE_640_BYTES + 4)* NUM_BUF_SIZE_640_BYTES)+\
							((BUF_SIZE_768_BYTES + 4)* NUM_BUF_SIZE_768_BYTES)+\
							((BUF_SIZE_1024_BYTES + 4)* NUM_BUF_SIZE_1024_BYTES)+\
							((BUF_SIZE_1152_BYTES + 4)* NUM_BUF_SIZE_1152_BYTES)+\
							((BUF_SIZE_1408_BYTES + 4)* NUM_BUF_SIZE_1408_BYTES)+\
							((BUF_SIZE_1664_BYTES + 4)* NUM_BUF_SIZE_1664_BYTES)+\
							((BUF_SIZE_1920_BYTES + 4)* NUM_BUF_SIZE_1920_BYTES)+\
							((BUF_SIZE_2100_BYTES + 4)* NUM_BUF_SIZE_2100_BYTES)+\
							((BUF_SIZE_2300_BYTES + 4)* NUM_BUF_SIZE_2300_BYTES))


/*
** ============================================================================
** Public Structures, Unions & enums Type Definitions
** ============================================================================
*/

/* None */

/*@}*/

/*
** ============================================================================
** Public Variable Declarations
** ============================================================================
*/


/*
** ============================================================================
** Public Function Prototypes
** ============================================================================
*/



/**
 ** \defgroup timer_func Timer Service APIs
 ** \ingroup timer_service
 */

/*@{*/

/**
 *****************************************************************************
 * @brief Initializes the software buffer service
 * @param None
 * @retval None
 * @note This function should be called atleast once during the system
 * initialization and only after this, the buffer services should be used.
 *****************************************************************************/
void buffer_service_init(void);

/**
 *****************************************************************************
 * @brief Implements the buffer allocation functionality.
 * @param length - the size of the buffer to be allocated
 * @retval None
 *****************************************************************************/
void * app_bm_alloc(
    uint16_t length //uint16_t length
    );

/**
 *****************************************************************************
 * @brief Implements the delay functionality
 * @param *pMem - the buffer to be freed
 * @retval None
 *****************************************************************************/
void app_bm_free(
    uint8_t *pMem
    );

/*@}*/

/*@}*/

#ifdef __cplusplus
}
#endif
#endif /* TIMER_SERVICE_H */
