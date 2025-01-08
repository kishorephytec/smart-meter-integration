/** \file p3_debug.c
 *******************************************************************************
 ** \brief Provides variables for debugging
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

/*
********************************************************************************
* File inclusion
********************************************************************************
*/

///#include "CONFIG_PROJECT_EFM32GG295F_APP.h"
#define DEBUG_IS_ENABLED

#ifdef DEBUG_IS_ENABLED
   
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h> 
#include "common.h"
#include "em_cmu.h"
#include "em_gpio.h"

/*
** =============================================================================
** Private Macro definitions
** =============================================================================
*/

#define LED_SETUP_1W_865                0
#define LED_SETUP_20mW_865              1

#if LED_SETUP_1W_865
  #define LED_PORT              gpioPortD
  #define LED0                  4/*green*/
  #define LED1                  5/*red*/
#endif

#if LED_SETUP_20mW_865
  #define LED_PORT              gpioPortA
  #define LED0                  11/*red*/
  #define LED1                  9/*green*/
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

/*Umesh : 30-01-2018*/
//static uint16_t debug_structure_size=0;
//static uint8_t error_counter=0, working_counter=0;
/*this is not used*/

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

/* None */

/*
** =============================================================================
** External Function Prototypes
** =============================================================================
*/

extern void trigger_both_udp_ping_request(void);

/*
** =============================================================================
** Public Variables Definitions
** =============================================================================
**/

//uint8_t pa_send_counter=0;//
//uint8_t pc_send_counter=0;
//uint8_t pas_rcv_counter=0;
//uint8_t pcs_rcv_counter=0;
//
//uint8_t pa_recv_counter=0;//
//uint8_t extract_pa_recv_counter=0;//
//uint8_t pd_pa_recv_counter=0;//
//uint8_t pc_recv_counter=0;
//
//uint8_t pa_send_sm_counter=0;//
//uint8_t pc_send_sm_counter=0;
//
//uint8_t pa_send_smt_counter=0;//
//uint8_t pc_send_smt_counter=0;

//uint8_t link_local_flag=0;//SANTOSH, FOR TESTING

/*
** =============================================================================
** Public Function Prototypes
** =============================================================================
*/


void red_led_off(void);
void green_led_on(void);
void green_led_off(void);
void gpio_interrupt_setup(void);
void gpio_led_setup(void);
void configure_external_inttrupt_for_udp(void);

/*
** =============================================================================
** Public Function Definitions
** =============================================================================
*/

void red_led_on(void)
{
#if LED_SETUP_1W_865   
  GPIO_PinOutSet(LED_PORT, LED1);
#endif

#if LED_SETUP_20mW_865
  GPIO_PinOutClear(LED_PORT, LED0);/*red-on*/
#endif  
  return;
}
/*----------------------------------------------------------------------------*/
void red_led_off(void)
{
#if LED_SETUP_1W_865   
  GPIO_PinOutClear(LED_PORT, LED1);
#endif

#if LED_SETUP_20mW_865
  GPIO_PinOutSet(LED_PORT, LED0);/*red-off*/
#endif   
  return;
}
/*----------------------------------------------------------------------------*/
void green_led_on(void)
{
#if LED_SETUP_1W_865  
    GPIO_PinOutSet(LED_PORT, LED0);/*green-on*/
#endif

#if LED_SETUP_20mW_865
    GPIO_PinOutClear(LED_PORT, LED1);/*green-on*/
#endif    
  return;
}
/*----------------------------------------------------------------------------*/
void green_led_off(void)
{
#if LED_SETUP_1W_865   
  GPIO_PinOutClear(LED_PORT, LED0);
#endif

#if LED_SETUP_20mW_865
  GPIO_PinOutSet(LED_PORT, LED1);/*green-off*/
#endif   
  return;
}
/*----------------------------------------------------------------------------*/
void gpio_interrupt_setup(void)
{
  CMU_ClockEnable(cmuClock_GPIO, true);
  GPIO_PinModeSet(gpioPortA, 0, gpioModeInput, 1);
  GPIO_IntConfig(gpioPortA, 0, false, true, true);
  NVIC_ClearPendingIRQ(GPIO_EVEN_IRQn);
  NVIC_EnableIRQ(GPIO_EVEN_IRQn);
} 
/*----------------------------------------------------------------------------*/
void gpio_led_setup(void)
{
#if LED_SETUP_1W_865
  GPIO_PinModeSet(LED_PORT, LED0, gpioModePushPull, 0);// Configure LED0 pin as digital output (push-pull)
  GPIO_PinModeSet(LED_PORT, LED1, gpioModePushPull, 0);// Configure LED1 pin as digital output (push-pull) with drive-strength to lowest setting
#endif

#if LED_SETUP_20mW_865
  GPIO_PinModeSet(LED_PORT, LED0, gpioModePushPull, 1);// Configure LED0 pin as digital output (push-pull)
  GPIO_PinModeSet(LED_PORT, LED1, gpioModePushPull, 1);// Configure LED1 pin as digital output (push-pull) with drive-strength to lowest setting
#endif  
  //GPIO_DriveModeSet(LED_PORT, gpioDriveModeLowest);// Set DRIVEMODE to lowest setting (0.5 mA) for all LEDs configured with alternate drive strength

  return;
}
/*----------------------------------------------------------------------------*/

void init_gpio_toggle (void)
{
  CMU_ClockEnable(cmuClock_GPIO, true);
  GPIO_PinModeSet(gpioPortD, 7, gpioModePushPull, 1);
 // GPIO_DriveModeSet(gpioPortD, gpioDriveModeLowest);
  gpio_led_setup();
}

void toggle_gpio (void)
{
  GPIO_PinOutToggle (gpioPortD, 7);
}

/* Debdeep :: for debug purpose only */
void high_gpio ()
{
  GPIO_PinOutSet (gpioPortD, 7);
}

void low_gpio ()
{
  GPIO_PinOutClear (gpioPortD, 7);
}


#endif

void detection_callback(void)
{
#ifdef DEBUG_IS_ENABLED    
  red_led_on();
  trigger_both_udp_ping_request();
#endif  
  return;
}
/*----------------------------------------------------------------------------*/

void configure_external_inttrupt_for_udp(void)
{ 
#ifdef DEBUG_IS_ENABLED   
    gpio_interrupt_setup();        
    gpio_led_setup(); 
#endif    
}

#if (PRINT_DEBUG_LEVEL != NONE_DEBUG)
/*----------------------------------------------------------------------------*/
/*** This is a Serial Wire Output based console.***
To use it you must do two things:
1. Set up the Serial Wire Output in your application.
   You can use the function below for this.
   NOTE: This funtion is different than the one used
   in the profiler!
2. To print a single character use the CMSIS function
   ITM_SendChar(char c).*/


/*Suneet :: For using printf use this config
  Select swo config in iar which is SWO in green color
  ON: SWO Trace Window Forced PC Sampling 
  Rate(Sample/sec) 1159
    CPU Clock 19
    wanted  1000 
*/
void setupSWOForPrint(void)
{ 
  uint32_t freq;
  uint32_t div;
  /* Enable GPIO clock. */
  CMU->HFPERCLKEN0 |= CMU_HFPERCLKEN0_GPIO;

  /* Enable Serial wire output pin */
  GPIO->ROUTE |= GPIO_ROUTE_SWOPEN;

#if defined(_EFM32_GIANT_FAMILY) || defined(_EFM32_LEOPARD_FAMILY) || defined(_EFM32_WONDER_FAMILY)
  /* Set location 0 */
  GPIO->ROUTE = (GPIO->ROUTE & ~(_GPIO_ROUTE_SWLOCATION_MASK)) | GPIO_ROUTE_SWLOCATION_LOC0;

  /* Enable output on pin - GPIO Port F, Pin 2 */
  GPIO->P[5].MODEL &= ~(_GPIO_P_MODEL_MODE2_MASK);
  GPIO->P[5].MODEL |= GPIO_P_MODEL_MODE2_PUSHPULL;
#else
  /* Set location 1 */
  GPIO->ROUTE = (GPIO->ROUTE & ~(_GPIO_ROUTE_SWLOCATION_MASK)) |GPIO_ROUTE_SWLOCATION_LOC1;
  /* Enable output on pin */
  GPIO->P[2].MODEH &= ~(_GPIO_P_MODEH_MODE15_MASK);
  GPIO->P[2].MODEH |= GPIO_P_MODEH_MODE15_PUSHPULL;
#endif

  /* Enable debug clock AUXHFRCO */
  CMU->OSCENCMD = CMU_OSCENCMD_AUXHFRCOEN;


/* Wait until clock is ready */
while (!(CMU->STATUS & CMU_STATUS_AUXHFRCORDY));

/* Enable trace in core debug */
CoreDebug->DHCSR |= CoreDebug_DHCSR_C_DEBUGEN_Msk;
CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;

 

/* Enable PC and IRQ sampling output */
DWT->CTRL = 0x400113FF;

/* Set TPIU prescaler for the current debug clock frequency. Target frequency
is 1 kHz so we choose a divider that gives us the closest match.
Actual divider is TPI->ACPR + 1. */
freq = CMU_ClockFreqGet(cmuClock_DBG);
div = freq / 1000000;
TPI->ACPR = div - 1;
/* Set protocol to NRZ */
TPI->SPPR = 2;
/* Disable continuous formatting */
TPI->FFCR = 0x100;
/* Unlock ITM and output data */
ITM->LAR = 0xC5ACCE55;
ITM->TCR = 0x10009;
/* ITM Channel 0 is used for UART output */
ITM->TER |= (1UL << 0);
//  /* Enable trace in core debug */
//  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
//  ITM->LAR  = 0xC5ACCE55;
//  ITM->TER  = 0x0;
//  ITM->TCR  = 0x0;
//  TPI->SPPR = 2;
//  TPI->ACPR = 0xf;
//  ITM->TPR  = 0x0;
//  DWT->CTRL = 0x400003FE;
//  ITM->TCR  = 0x0001000D;
//  TPI->FFCR = 0x00000100;
//  ITM->TER  = 0x1;
}


void stack_print_debug(const char *format, ...)
{
    va_list vargs;
    va_start(vargs, format);
    vprintf(format, vargs);
    va_end(vargs);
}

void print_mac_address (uint8_t *addr)
{
  stack_print_debug(" %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n",addr[0], addr[1], addr[2], addr[3], addr[4], addr[5], addr[6], addr[7]);
}
#endif
