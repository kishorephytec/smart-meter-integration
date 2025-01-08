/** \file uart_hal.c
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


/*******************************************************************************
* File inclusion
*******************************************************************************/

#include "StackPHYConf.h"
#include "pin_config.h"
#include "common.h"
#include "em_device.h"
#include "em_cmu.h"
#include "em_gpio.h"
#include "em_usart.h"
#include "em_eusart.h"
#include "uart_hal.h"


/*
** ============================================================================
** Private Macro definitions
** ============================================================================
*/
#ifndef UART_TX_BUF_LEN
#define UART_TX_BUF_LEN	2300
#endif

#ifndef NUMBER_OF_UART_TX_BUFFERS
#define NUMBER_OF_UART_TX_BUFFERS	0
#endif


#define APP_USART0_BAUDRATE 115200
#define APP_USART0_TX_PORT gpioPortA
#define APP_USART0_TX_PIN 8

#define APP_USART0_RX_PORT gpioPortA
#define APP_USART0_RX_PIN 7
/*
** ============================================================================
** Private Structures, Unions & enums Type Definitions
** ============================================================================
*/

typedef struct P3_BUFFER_tag
{
    uint8_t* p_data;
    uint16_t element_count;
    void* p_cb_param;
}P3_BUFFER_t;

P3_BUFFER_t uart0_rx_buffer;

P3_BUFFER_t uart1_rx_buffer;

/*
When Using Only Usart as commucation to meater Usae USART0 default
If SPI flash is requred the make communication USART0 to EUSART0 because USART0 is used by SPI
Only 1 USART phereperal is avaliable 
*/
/* Setup UART1 in async mode for RS232*/
static USART_TypeDef           *uart0   = USART0;


/*
** ============================================================================
** Private Variable Definitions
** ============================================================================
*/

volatile uint8_t * gp_uart0_tx_address;        /* uart1 transmit buffer address */
volatile uint16_t  g_uart0_tx_count;           /* uart1 transmit data number */
volatile uint8_t * gp_uart0_rx_address;        /* uart1 receive buffer address */
volatile uint16_t  g_uart0_rx_count;           /* uart1 receive data number */
volatile uint16_t  g_uart0_rx_length;          /* uart1 receive data length */


volatile uint8_t * gp_uart1_tx_address;        /* uart1 transmit buffer address */
volatile uint16_t  g_uart1_tx_count;           /* uart1 transmit data number */
volatile uint8_t * gp_uart1_rx_address;        /* uart1 receive buffer address */
volatile uint16_t  g_uart1_rx_count;           /* uart1 receive data number */
volatile uint16_t  g_uart1_rx_length;          /* uart1 receive data length */

/*
** ============================================================================
** Private Macro definitions
** ============================================================================
*/





/*
** ============================================================================
** Private Structures, Unions & enums Type Definitions
** ============================================================================
*/



/*
** ============================================================================
** Public Variable Definitions
** ============================================================================
*/

uart_hal_data_t uart_hal_info;
volatile uint8_t uart0_trigger_tx = 1;

volatile uint8_t uart1_trigger_tx = 1;
static uint8_t uart1_rx_byte;

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

  /* None */

/*
** ============================================================================
** Public Function Definitions
** ============================================================================
*/

void UART_init(void)
{  
  
     /*

    Raka :: Needs to do clean up once UART is tested Properly  [ 13- Sep - 2022 ]
    */
CMU_ClockEnable(cmuClock_USART0, true);
    CMU_ClockEnable(cmuClock_GPIO, true);
  
  // Default asynchronous initializer (115.2 Kbps, 8N1, no flow control)
    
    USART_InitAsync_TypeDef init = USART_INITASYNC_DEFAULT;  // this is for FG13
    
    
     /* Configure GPIO pins  for communication UART  :: LOC 22*/
    GPIO_PinModeSet(APP_USART0_TX_PORT, APP_USART0_TX_PIN, gpioModePushPull, 1); // tx
    GPIO_PinModeSet(APP_USART0_RX_PORT, APP_USART0_RX_PIN, gpioModeInput, 0);     // rx

    
    // Route USART0 TX and RX to the board controller TX and RX pins
    GPIO->USARTROUTE[0].TXROUTE = (APP_USART0_TX_PORT << _GPIO_USART_TXROUTE_PORT_SHIFT)
      | (APP_USART0_TX_PIN << _GPIO_USART_TXROUTE_PIN_SHIFT);
  GPIO->USARTROUTE[0].RXROUTE = (APP_USART0_RX_PORT << _GPIO_USART_RXROUTE_PORT_SHIFT)
      | (APP_USART0_RX_PIN << _GPIO_USART_RXROUTE_PIN_SHIFT);
    // Clock configuration ....

   // Enable RX and TX signals now that they have been routed
  GPIO->USARTROUTE[0].ROUTEEN = GPIO_USART_ROUTEEN_RXPEN | GPIO_USART_ROUTEEN_TXPEN;
    
      // The Baud rate for the Communication.
    init.baudrate = APP_USART0_BAUDRATE ;

    // Configure and enable USART0
    USART_InitAsync(USART0, &init);          // this is for FG13
    
   

    
   //  EUSART_UartInit_TypeDef init = EUSART_UART_INIT_DEFAULT_HF;   // this is for FG23
 
   // EUSART_UartInitHf(EUSART1, &init);          // this is for FG23

    /* Prepare UART Rx and Tx interrupts */
   // USART_IntClear(uart0, _USART_IF_MASK);            // this is for FG13
  //   EUSART_IntClear(EUSART1, _EUSART_IEN_MASK);        // this is for FG23
   // USART_IntEnable(uart0, USART_IF_RXDATAV);         // this is for FG13
 //   EUSART_IntEnable(EUSART1, EUSART_IEN_RXFL);         // this is for FG23
    
    //// this is for FG13
    NVIC_ClearPendingIRQ(USART0_RX_IRQn);
    USART_IntEnable(USART0, USART_IEN_RXDATAV);
    NVIC_EnableIRQ(USART0_RX_IRQn);
    NVIC_ClearPendingIRQ(USART0_TX_IRQn);
    NVIC_EnableIRQ(USART0_TX_IRQn);
    
    //// this is for FG23
//  NVIC_ClearPendingIRQ(EUSART1_RX_IRQn);
//  NVIC_ClearPendingIRQ(EUSART1_TX_IRQn);
//  NVIC_EnableIRQ(EUSART1_RX_IRQn);
//  NVIC_EnableIRQ(EUSART1_TX_IRQn);
    
// this is for FG13 
  //for fg23 PIN loc not required
    // Enable RX and TX for USART0 VCOM connection
 //   USART0->ROUTELOC0 = USART_ROUTELOC0_RXLOC_LOC22 | USART_ROUTELOC0_TXLOC_LOC22;
  //  USART0->ROUTEPEN |= USART_ROUTEPEN_RXPEN | USART_ROUTEPEN_TXPEN;
  

}

/******************************************************************************/

void UART_close(void)
{
  USART_Reset(uart0);         //This is for FG13
//  EUSART_Reset(EUSART1);        //This is for FG23
}

/******************************************************************************/
int8_t uart_hal_read( uint8_t* p_data, uint16_t len )
{
	uart0_rx_buffer.p_data = p_data;
	uart0_rx_buffer.element_count = ( uint16_t )len;
        
        g_uart0_rx_count = 0U;
        g_uart0_rx_length = ( uint16_t )len;
        gp_uart0_rx_address = p_data;
        
	return 0;
}

/******************************************************************************/
int8_t uart_hal_write( uint8_t* p_data, uint16_t len )
{
	if( len > UART_TX_BUF_LEN )
	{
		//let this get freed up as it is of more than the allowed UART packet size
		return -2;//
	}

	/* Submit Outbound buffer to UART */
	if( uart0_trigger_tx )
	{
          uart0_trigger_tx = 0;

          gp_uart0_tx_address = p_data;
          g_uart0_tx_count = len;

          /* Enable interrupt on USART TX Buffer*/
          USART_IntEnable(uart0, USART_IF_TXBL);            //This is for FG13
          //EUSART_IntEnable(EUSART1, EUSART_IEN_TXFL);           //This is for FG23
	}
	else
	{
		//uart is busy doing transmission of a packet initiated earlier. 
		//So let this buffer be put in a queue to be processed ata a 
		//later stage
		return -1;
		//queue_item_put( &uart_tx_buf_q,  (queue_item_t*) p_uart_tx_buff );
	}

    return 0;
}

/******************************************************************************/

int8_t uart_hal_register_back(uart_hal_call_back cb, void* param)
{
	uart_hal_info.cb = cb;
	uart_hal_info.param = param;
	return 0;
}

/*
** ============================================================================
** Private Function Definitions
** ============================================================================
*/

void UARTCallback( uint8_t eventid )
{
    switch (eventid) 
    {
        case 0://tx complete
			
			/*put back the buffer in the free pool*/
			uart0_trigger_tx = 1;

			/*invoke callback informing packet tx completion*/
			uart_hal_info.cb( NULL,TX_COMPLETE_EVENT,uart_hal_info.param );
			break;
		case 1://rx complete
			/*invoke callback informing packet rx completion*/
			uart_hal_info.cb( NULL,RX_COMPLETE_EVENT,uart_hal_info.param );
            break;
		case 2://error
			break;

		default:
			break;                    
    }
}



/**************************************************************************//**
 * @brief UART1 RX IRQ Handler
 *
 * Set up the interrupt prior to use
 *
 * Note that this function handles overflows in a very simple way.
 *
 *****************************************************************************/
uint8_t uart_debug_buff [100] = {0};
uint16_t uart_debug_buff_Cnt = 0;

void USART0_RX_IRQHandler(void)
{
  uint8_t rxData;
  /* Check for RX data valid interrupt */
  if (uart0->STATUS & USART_STATUS_RXDATAV)
  {
    /* Copy data into RX Buffer */
    rxData = USART_Rx(uart0);
    /* Clear RXDATAV interrupt */
    
    uart_debug_buff [uart_debug_buff_Cnt++] = rxData;
    if ( uart_debug_buff_Cnt == 100 )
    {
      uart_debug_buff_Cnt = 0;
    }
    
    USART_IntClear(USART0, USART_IF_RXDATAV);
    
    if (g_uart0_rx_length > g_uart0_rx_count)
    {
        *gp_uart0_rx_address = rxData;
        
        g_uart0_rx_count++;

        if (g_uart0_rx_length == g_uart0_rx_count)
        {
            UARTCallback(1);
        }
        else
        {
          gp_uart0_rx_address++;
        }
    }
  }
  else
  {
    USART_IntClear(USART0, USART_STATUS_RXFULL);
  }
}

/**************************************************************************//**
 * @brief UART1 TX IRQ Handler
 *
 * Set up the interrupt prior to use
 *
 *****************************************************************************/
void USART0_TX_IRQHandler(void)
{
  /* Check TX buffer level status */
  if (uart0->STATUS & USART_STATUS_TXBL)
  {
    if (g_uart0_tx_count > 0)
    {
      /* Transmit pending character */
      USART_Tx(uart0, *gp_uart0_tx_address++);      
      g_uart0_tx_count--;
    }

    /* Disable Tx interrupt if no more bytes to be sent */
    if (g_uart0_tx_count == 0)
    {
      USART_IntDisable(uart0, USART_IF_TXBL);
      //send end
      UARTCallback(0);
    }
  }
}



#if 0
/**************************************************************************//**
* @brief
*    USART0 initialization (VCOM on xG1/xG12/xG13 boards)
*****************************************************************************/
void init_Usart1(void)
{
  
  
  //CMU_ClockEnable(cmuClock_GPIO, true);
  
  
  /* Configure GPIO pins for Debug UART :: LOC */ 
  
  GPIO_PinModeSet(APP_USART1_TX_PORT, APP_USART1_TX_PIN, gpioModeInput, 0); //tx
  GPIO_PinModeSet(APP_USART1_RX_PORT, APP_USART1_RX_PIN, gpioModePushPull, 1); //rx
  
  CMU_ClockEnable(cmuClock_USART1, true);
  
  // Default asynchronous initializer (115.2 Kbps, 8N1, no flow control)
  USART_InitAsync_TypeDef init = USART_INITASYNC_DEFAULT;
     
  // The Baud rate for the Communication.
  init.baudrate = APP_USART1_BAUDRATE ;
  
  // Configure and enable USART0
  USART_InitAsync(USART1, &init);
  
  
  USART_IntClear(USART1, _USART_IF_MASK);
  USART_IntEnable(USART1, USART_IEN_RXDATAV);
  
  // Enable NVIC USART sources
  NVIC_ClearPendingIRQ(USART1_RX_IRQn);
  NVIC_EnableIRQ(USART1_RX_IRQn);
  NVIC_ClearPendingIRQ(USART1_TX_IRQn);
  NVIC_EnableIRQ(USART1_TX_IRQn);
  
  /* Enable I/O pins at UART0 location #22 */
  USART1->ROUTELOC0 = USART_ROUTELOC0_RXLOC_LOC25  | USART_ROUTELOC0_TXLOC_LOC27;
  USART1->ROUTEPEN |= USART_ROUTEPEN_RXPEN | USART_ROUTEPEN_TXPEN;
  
}
/******************************************************************************/

void UART1_close(void)
{
  USART_Reset(USART1);
}



/******************************************************************************/

int8_t raw_uart_hal_read(uint8_t* p_data, uint16_t len )
{
  uart1_rx_buffer.p_data = p_data;
  uart1_rx_buffer.element_count = ( uint16_t )len;
  
  g_uart1_rx_count = 0U;
  g_uart1_rx_length = ( uint16_t )len;
  gp_uart1_rx_address = p_data;
  
  return 0;
}

/******************************************************************************/

int8_t raw_uart_hal_write(uint8_t* p_data, uint16_t len )
{
 
  /* Submit Outbound buffer to UART */
  if( uart1_trigger_tx )
  {
    uart1_trigger_tx = 0;
    
    gp_uart1_tx_address = p_data;
    g_uart1_tx_count = len;
    
    /* Enable interrupt on USART TX Buffer*/
    USART_IntEnable(USART1, USART_IF_TXBL);
  }
  else
  {
    //uart is busy doing transmission of a packet initiated earlier. 
    //So let this buffer be put in a queue to be processed ata a 
    //later stage
    return -1;
    
  }
  
  return 0;
}



/*
** ============================================================================
** Private Function Definitions
** ============================================================================
*/

void UART1_Callback( uint8_t eventid )
{
  int8_t ch;
  switch (eventid) 
  {
  case 0://tx complete
    
    /*put back the buffer in the free pool*/
    
    uart1_trigger_tx = 1;
    
    /*invoke callback informing packet tx completion*/
    uart_call_back(NULL,0,0,0);
    break;
  case 1://rx complete
    ch = uart1_rx_byte;
    
    raw_uart_hal_read(&uart1_rx_byte ,1 );
    /*invoke callback informing packet rx completion*/
    uart_call_back(NULL,0,1,ch);
    break;
  case 2:
    uart_call_back(NULL,0,2,0);
    break;
    
  default:
    break;                    
  }
  
}

/*
** ============================================================================
** Private Function Definitions
** ============================================================================
*/


/**************************************************************************//**
* @brief UART1 RX IRQ Handler
*
* Set up the interrupt prior to use
*
* Note that this function handles overflows in a very simple way.
*
*****************************************************************************/
void USART1_RX_IRQHandler(void)
{
  uint8_t rxData;
  /* Check for RX data valid interrupt */
  if (USART1->STATUS & USART_STATUS_RXDATAV)
  {
    /* Copy data into RX Buffer */
    rxData = USART_Rx(USART1);
    /* Clear RXDATAV interrupt */
    USART_IntClear(USART1, USART_IF_RXDATAV);
    if (g_uart1_rx_length > g_uart1_rx_count)
    {
      *gp_uart1_rx_address = rxData;
      
      g_uart1_rx_count++;
      
      if (g_uart1_rx_length == g_uart1_rx_count)
      {
        UART1_Callback(1);
      }
      else
      {
        gp_uart1_rx_address++;
      }
    }
  }
  else
  {
    USART_IntClear(USART1, USART_STATUS_RXFULL);
  }
}

/**************************************************************************//**
* @brief UART1 TX IRQ Handler
*
* Set up the interrupt prior to use
*
*****************************************************************************/
void USART1_TX_IRQHandler(void)
{
  /* Check TX buffer level status */
  if (USART1->STATUS & USART_STATUS_TXBL)
  {
    if (g_uart1_tx_count > 0)
    {
      /* Transmit pending character */
      USART_Tx(USART1, *gp_uart1_tx_address++);      
      g_uart1_tx_count--;
    }
    
    /* Disable Tx interrupt if no more bytes to be sent */
    if (g_uart1_tx_count == 0)
    {
      USART_IntDisable(USART1, USART_IF_TXBL);
      //send end
      UART1_Callback(0);
    }
  }
}

/**************************************************************************//**
* @brief UART1 TX IRQ Handler
*
* Set up the interrupt prior to use
*
*****************************************************************************/

#endif