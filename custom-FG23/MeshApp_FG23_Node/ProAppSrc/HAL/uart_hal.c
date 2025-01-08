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


//#define APP_USART0_BAUDRATE 9600

#define APP_USART0_BAUDRATE 9600

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

uart0_callback g_uart_cb; //UART0 cb variable
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
  USART_InitAsync(USART0, &init);          // this is for FG13 / FG23
  
  //// this is for FG13
  NVIC_ClearPendingIRQ(USART0_RX_IRQn);
  USART_IntEnable(USART0, USART_IEN_RXDATAV);
  NVIC_EnableIRQ(USART0_RX_IRQn);
  NVIC_ClearPendingIRQ(USART0_TX_IRQn);
  NVIC_EnableIRQ(USART0_TX_IRQn);
  
  
}

/******************************************************************************/

void UART_close(void)
{
  USART_Reset(uart0);         //This is for FG13 / FG23
  
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
int8_t uart_hal_write( uint8_t* buffer, uint16_t len )
{
  
  uint32_t sent = 0;
  uint8_t * _buffer = (uint8_t *) buffer;
  
  while (len--)
  {
    USART_Tx(uart0, _buffer[sent++]);
    while (!(uart0->STATUS & USART_STATUS_TXC));
  }
  return sent;
  //	if( len > UART_TX_BUF_LEN )
  //	{
  //		//let this get freed up as it is of more than the allowed UART packet size
  //		return -2;//
  //	}
  //
  //	/* Submit Outbound buffer to UART */
  //	if( 1 )
  //	{
  //          uart0_trigger_tx = 0;
  //
  //          gp_uart0_tx_address = p_data;
  //          g_uart0_tx_count = len;
  //
  //          /* Enable interrupt on USART TX Buffer*/
  //          USART_IntEnable(uart0, USART_IF_TXBL);
  //	}
  //	else
  //	{
  //		//uart is busy doing transmission of a packet initiated earlier. 
  //		//So let this buffer be put in a queue to be processed ata a 
  //		//later stage
  //		return -1;
  //		//queue_item_put( &uart_tx_buf_q,  (queue_item_t*) p_uart_tx_buff );
  //	}
  //
  //    return 0;
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
* @brief UART0 Callback register
*
*
*****************************************************************************/

void register_uart0_cb( uart0_callback cb)
{
  if(!g_uart_cb)
  {
    g_uart_cb = cb;
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
    USART_IntClear(USART0, USART_IF_RXDATAV);
    
    if(g_uart_cb)
    {
      g_uart_cb(rxData); //indicate if any func register
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
      // UARTCallback(0);
    }
  }
}


