/** \file hw_tmr.h
 *******************************************************************************
 ** \brief  Hardware Timer 
 **    This file contains the public APIs and structures for use by Hardware 
 **    Timer Module.
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

#ifndef _HW_TMR_H_
#define _HW_TMR_H_

#ifdef __cplusplus
extern "C" {
#endif
  
#ifdef INTERVAL_TIMER_USED
#include "r_cg_it.h" 
#endif  
  
#define CLOCK_FREQUENCY         38400000  //d //32 // 48 MHz or 32 MHz
  
#if CLOCK_FREQUENCY==48
  // When the Clock Frequency is 48 MHz for the timer module
  // we are using PreScalar of 32 hence the clock frequency for 
  // timer module is 15000000 so the timer tick will be 1.5 times faster
  #define CLOCK_TICK_RESOLUTION   1.5
#else
  #define CLOCK_TICK_RESOLUTION   1.2 //d //1
#endif
  
  
/**
 *****************************************************************************
 * @ingroup sysdoc
 *
 * @{
 *****************************************************************************/


/**
 *****************************************************************************
 * @defgroup HwTmr HardWare Timer Module 
 * @brief  This file provides different APIs for configuring and using hardware 
 * timers available with the MCU. 
 *
 *    The hardware Timer Module provides APIs for configuring and using the 
 *    hardware timer channels for realizing software timers.   
 *
 *    The Hardware Timer Init function initializes the hardware Timer structure 
 *    instance and configures the specified Timer channel with the following 
 *    things.\n
 *    1) Configure the appropriate Clock Source\n
 *    2) Enable the specified Timer Channel clock source\n
 *    3) Configure and enable the OVERFLOW Intterrupt\n
 *    4) Register the Timer interrupt handler with the interrupt controller \n
 *        
 *    Software Timer Module has to initialize the hardware Timer module before 
 *    it can use START or STOP functionalities. To Initialize the hardware Timer
 *    module, hw_tmr_init(). The Hardware timer channel supported by the under-
 *    lying MCU is specified for use duing hw_tmr_init(). The hadware timer is 
 *    is configured to give clock ticks at a rate of 1 us by selecting the 
 *    appropriate clock source to the specified Timer channel.\n
 *    
 *    This module maintains a 32-bit counter in software which is updated 
 *    continuously to indicate the current time of the system. The OVERFLOW 
 *    interrupt occuring as a result of 16-bit counter register overflow is used
 *    for updating the time elapsed since the last overflow into the 32-bit 
 *    counter maintained in the software. Irrespective of whether any software 
 *    timer is running or not, this operation is repeated after every overflow 
 *    interrupt event is done to keep the system time updated even if there are 
 *    no software timers running.\n
 *    
 *    A software timer module can request hardware timer module to start a timer 
 *    and notify about the expiry after the specified period. This is achieved by 
 *    loading the expiry time in the hardware timer structure and requesting 
 *    hardware timer module to keep track of the current time and the expiry time. 
 *    When the hardware timer module notice that the currrent time has reached 
 *    the expiry time, it notifies the software timer module about the software 
 *    timer expiry and henceforth the hardware timer module keeps updating the 
 *    current time continuously as and when the overflow interrupts happen. 
 *   
 ******************************************************************************/

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
 ** \defgroup hw_timer  Hardware Timer Definitions
 ** \ingroup HwTmr
 */

/*@{*/

#define HWTIMER_SYMBOL_LENGTH   PHY_Get_Symbol_Rate()

/*
** =============================================================================
** Public Structures, Unions & enums Type Definitions
** =============================================================================
*/

typedef enum
{
  HW_TIMER_CHANNEL_0_INT = 0x01,
  HW_TIMER_CHANNEL_1_INT,
} hw_tmr_event_t;

/**
 *****************************************************************************
 * @enum Differnt Timer channel available.
 *    Enum used by software timer module to indicate the hardware timer channel 
 *    to be used.
 *****************************************************************************/
typedef enum
{
  HW_TIMER_CHANNEL_0 = 0x00000000,
  HW_TIMER_CHANNEL_1,
  HW_TIMER_CHANNEL_2,
  HW_TIMER_CHANNEL_3,
  HW_TIMER_CHANNEL_4,
  HW_TIMER_CHANNEL_5,
  HW_TIMER_CHANNEL_6,
  HW_TIMER_CHANNEL_7,
  HW_TIMER_CHANNEL_8,
} hw_timer_channel_t;

#define CLOCK_PERIOD    1

/**
 *****************************************************************************
 * @brief hardware Timer Call back type definition.
 * 
 *****************************************************************************/
typedef void (*cb_routine_t)( void *ctx );

/**
 *****************************************************************************
 * @struct Structure for the hardware Timer Instance.
 *    Structure difinition for the hardware Timer module used by the 
 *    Software Timer module
 *****************************************************************************/
typedef struct hw_tmr_tag
{
  hw_timer_channel_t 	tmr_channel;            /**< Hardware Timer channel in use*/
  cb_routine_t 		    cb;                     /**< Timer expiry call back function */
  void 				    *ctx;                   /**< Timer expiry call back function paramter */
  uint16_t              low_time_us;
  uint8_t               rollover_handled;		//bit0-TMR1 int, b1:TMR0 int, b2:
  uint8_t               timer_added_in_irq;
} hw_tmr_t;


/*@}*/

/*
** =============================================================================
** Public Variable Declarations
** =============================================================================
*/

//extern volatile uint16_t system_time;


/*
** =============================================================================
** Public Function Prototypes
** =============================================================================
*/

/**
 ** \defgroup hw_timer_func  Hardware Timer APIs
 ** \ingroup HwTmr
 */

/*@{*/

/**
 *****************************************************************************
 * @brief Initialize the 
 *    This function initializes the timer module. 
 * @param *phw_tmr_ins[in] - pointer to the hardware timer structure instance to 
 *							 be initilized.
 * @param *sw_tmr_mod_ins - pointer to the software timer structure instance to 
 *							 be initilized.
 * @retval None.
 * @note
 * This function should be called once by the software timer module before it 
 * can start using the hardware timer module functionality. Software timer 
 * should initialize hw_timer_channel_t tmr_channel, cb_routine_t cb and 
 * void *ctx structure members before invoking this function for inizializing 
 * remaining members of the hardware timer structure.
 *****************************************************************************/
void hw_tmr_init( void *phw_tmr_ins, void *sw_tmr_mod_ins );

/**
 *****************************************************************************
 * @brief Starts the initialised hardware timer module with the specified ticks.
 * @param *phw_tmr_ins[in] - pointer to the hardware timer structure instance.
 * @param exp[in] - timer count at which the hardware timer module should notify 
 * with an an expiry event to the software timer module.
 * @retval None.
 * @note
 * This function should be passed with the point-in-time at which the hardware
 * timer module should trigger timer expiry  event. If the time duration after 
 * which the timer expiry event is known then expiry time stamp can be derived  
 * by adding the current time and the time duration to be elapsed. This derived 
 * expiry time stamp will be passed to this function.
 *****************************************************************************/
void hw_tmr_start( void *phw_tmr_ins, p3time_t exp );

/**
 *****************************************************************************
 * @brief Stops the running hardware timer module.
 * @param *phw_tmr_ins[in] - pointer to the hardware timer structure instance.
 * @retval None.
 * @note This function should be called by the software timer module when there 
 * are no pending software timers running.
 *****************************************************************************/
void hw_tmr_stop( void *phw_tmr_ins );


/**
 *****************************************************************************
 * @brief Provide the current System Time.
 * @param *phw_tmr_ins[in] - pointer to the hardware timer instance.
 * @retval The current system time(Timer clock tick counts).
 * @note - None
 *****************************************************************************/
p3time_t hw_tmr_get_time( void *phw_tmr_ins );


/**
 *****************************************************************************
 * @brief 
 *    This function is a blocking function which returns after the specified 
 *    amount of delay gets elapsed. 
 * @param *phw_tmr_ins[in] - pointer to the Hardware Timer structure instance.
 * @param duration_ticks[in] - duration to be elapsed by this function specified 
 *							in terms of clock ticks.
 * @retval - None
 * @note Software timer module has to calculate the exact timer ticks need be 
 *    elapsed and use this API to achieve the required delay.
 *****************************************************************************/
void hw_tmr_delay( void *phw_tmr_ins, uint32_t duration_ticks );


/**
 *****************************************************************************
 * @brief Provides a single byte random number.
 *    This function uses the current system time to get a random 
 *    number
 * @param *phw_tmr_ins[in] - pointer to the Hardware Timer structure instance.
 * @retval - 1 byte random value(00 - ff)
 * @note - None
 *****************************************************************************/
uint8_t hw_tmr_rand( void *phw_tmr_ins );

/**
 *****************************************************************************
 * @brief Provides a symbol value.
 *    This function is used to convert the micro second value to symbol value
 * @param time - time to convert to symbol
 * @retval - symbol value of given time
 * @note - None
 *****************************************************************************/
uint32_t hw_tmr_get_symbols( uint32_t time );

/**
 *****************************************************************************
 * @brief Function is not used still
 * @param - None
 * @retval - 
 * @note - None
 *****************************************************************************/
uint16_t hw_tmr_sys_time_high( void );
p3time_t hw_tmr_get_time_int( void *hw_tmr_ins );



#define TIMER2_INIT_DEFAULT                        \
{                                                  \
    .enable     = false,                           \
    .debugRun   = false,                           \
    .prescale   = timerPrescale32,                 \
    .clkSel     = timerClkSelHFPerClk,             \
    .fallAction = timerInputActionNone,            \
    .riseAction = timerInputActionNone,            \
    .mode       = timerModeUp,                     \
    .dmaClrAct  = false,                           \
    .quadModeX4 = false,                           \
    .oneShot    = false,                           \
    .sync       = false,                           \
};  

#define TIMER2_INITCC_DEFAULT                      \
{                                                  \
    .eventCtrl  = timerEventEveryEdge,             \
    .edge       = timerEdgeBoth,                   \
    .prsSel     = timerPRSSELCh0,                  \
    .cufoa      = timerOutputActionNone,           \
    .cofoa      = timerOutputActionNone,           \
    .cmoa       = timerOutputActionToggle,         \
    .mode       = timerCCModePWM,                  \
    .filter     = false,                           \
    .prsInput   = false,                           \
    .coist      = false,                           \
    .outInvert  = false,                           \
};

/*@}*/


#ifdef __cplusplus
}
#endif
#endif /* _HW_TMR_H_ */
