/** \file uart_hal.h
 *******************************************************************************
 ** \brief  Provides APIs for UART driver
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


#ifndef _UART_HAL_
#define _UART_HAL_

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
 * 
 * @}     
 *****************************************************************************/

/*
** =============================================================================
** Public Macro definitions
** =============================================================================
*/

/**
 ** \defgroup uart_hal UART HAL Interface
 */

/*@{*/

/**
 ** \defgroup uart_hal_def  UART HAL Definitions
 ** \ingroup uart_hal
 */

/*@{*/

	/* None */

/*
** =============================================================================
** Public Structures, Unions & enums Type Definitions
** =============================================================================
*/

/**
 *****************************************************************************
 * @enum Transceiver_events
 *    Enumeration to indicate the transceiver state
 *****************************************************************************/
enum 
{                                              
    TX_COMPLETE_EVENT = 2,
    RX_COMPLETE_EVENT 
};

/* Function Prototype */
typedef void (*uart_hal_call_back) (void*, uint32_t, void*);
typedef void (*app_call_back_t)( uint8_t evt , uint8_t ch);
typedef void ( *drv_init_t )( void );
typedef void ( *drv_deinit_t )( void );
typedef int8_t (* drv_send_t )(uint8_t* p_data, uint16_t len);
typedef int8_t (* drv_recv_t )(uint8_t* p_data, uint16_t len);

/* Function Prototype */
//typedef void (*uart_hal_call_back) ( void*, uint16_t );
//[KIMBAL]
typedef void (*uart0_callback)(uint8_t ch);


/**
 *****************************************************************************
 * @struct Structure for UART Data
 *****************************************************************************/
typedef struct uart_hal_data_tag
{
	uart_hal_call_back cb;
	void* param;
}uart_hal_data_t;


/*@}*/

/*
** =============================================================================
** Public Variable Declarations
** =============================================================================
*/

/* None */

/*
** =============================================================================
** Public Function Prototypes
** =============================================================================
*/

/**
 ** \defgroup uart_data_func UART HAL APIs
 ** \ingroup uart_hal
 */

/*@{*/


/**
 *****************************************************************************
 * @brief Initialises the UART
 * @param - None
 * @retval - DDSSResult
 * @note
 *****************************************************************************/
void UART_init(void);

/**
 *****************************************************************************
 * @brief Closes the UART driver
 * @param - None 
 * @retval - None
 * @note
 *****************************************************************************/
void UART_close(void);

/**
 *****************************************************************************
 * @brief This function submits the packet to the UART driver for transmission
 * @param *p_data - location which holds the packet to be transmited
 * @param len - length of the packet
 * @retval -1 If no buffer is available
 * @retval 0 on success. 
 * @note - This is a non-blocking function.
 *****************************************************************************/
int8_t uart_hal_write(uint8_t* p_data, uint16_t len );

/**
 *****************************************************************************
 * @brief This function provides the location where the UART packet needs to be rcewived and copied.
 * @param *p_data - location where the rece packet should be copied
 * @param len - length of the packet to be received
 * @retval - if the read is failure
 * @note - This is a non-blocking function.
 *****************************************************************************/
int8_t uart_hal_read(uint8_t* p_data, uint16_t len );


/**
 *****************************************************************************
 * @brief This function registers the application call back
 * @param cb - application call back func
 * @param *param - call back param
 * @retval - 0 on succes
 * @note - None
 *****************************************************************************/
int8_t uart_hal_register_back(uart_hal_call_back cb, void* param);


/*@}*/

/*@}*/



 /*
 ** ============================================================================
 ** Public Macro definitions
 ** ============================================================================
 */

 
 /*
 ** ============================================================================
 ** Public Structures, Unions & enums Type Definitions
 ** ============================================================================
 */
/* Function Prototype */
typedef void (*debug_uart_hal_call_back) (void*, uint8_t , uint8_t , uint8_t);



 /*
 ** ============================================================================
 ** Public Variable Declarations
 ** ============================================================================
 */


//[KIMBAL]
void register_uart0_cb( uart0_callback cb);
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
 *****************************************************************************
 * @brief Initialises the UART
 * @param - None
 * @retval - DDSSResult
 * @note
 *****************************************************************************/
  
  void init_Usart1(void);
  
  /**
 *****************************************************************************
 * @brief Closes the UART driver
 * @param - None 
 * @retval - None
 * @note
 *****************************************************************************/
  void UART1_close(void);
  
  int8_t raw_uart_hal_read(uint8_t* p_data, uint16_t len );  
  int8_t raw_uart_hal_write(uint8_t* p_data, uint16_t len );
  int8_t Debug_uart_hal_register_back(debug_uart_hal_call_back cb, void* param);
  
  void UART1_Callback( uint8_t eventid );
  
  
  
#ifdef __cplusplus
}
#endif
#endif /* _ADI_UART_HAL_ */
