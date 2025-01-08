/** \file hw_tmr.c
 **
 ** \brief Implements APIs and structures for use by Hardware 
 *    Timer Module.
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
#include "StackAppConf.h"
#include "em_device.h"
#include "em_cmu.h"
#include "em_emu.h"
#include "em_gpio.h"
#include "em_prs.h"
#include "em_system.h"
#include "em_timer.h"
#include "em_chip.h"
//#include "Pro_Stack_Config.h"
#include "common.h"
#include "queue_latest.h"
#include "buff_mgmt.h"
#include "list_latest.h"
#include "hw_tmr.h"
#include "sw_timer.h"
   

/*
 TIMER1 (uS ticker higher 16bit) <----TIMER0 (uS ticker lower 16bit)  
*/

//#define TIMER_DEBUG

/* Funcion declaraion*/
void       timer_initialization  (void);
void       ClockConfiguration    (void);
uint32_t   us_timer_read         (void);
void       us_timer_set_value    (uint32_t timestamp);

/* Variables to debug the interrupt counts */
#ifdef TIMER_DEBUG
  uint16_t ovfCnt     = 0;
  uint16_t ovfCnt_1   = 0;
  uint16_t compCnt    = 0; 
  uint16_t compCnt_1  = 0;
  uint16_t clk_cnt_0  = 0; 
  uint16_t clk_cnt_1  = 0;
  uint32_t check_val  = 0;

  #define MAX_DEBUG_COUNT_VAL 100

  typedef struct debug_timer_val
  {
    uint32_t timestampLowS  [MAX_DEBUG_COUNT_VAL];
    uint32_t timestampHighS [MAX_DEBUG_COUNT_VAL];
    uint32_t current_time   [MAX_DEBUG_COUNT_VAL];
    uint16_t indextimestamp;
    uint16_t low_time_val   [MAX_DEBUG_COUNT_VAL];
    uint16_t high_time_val  [MAX_DEBUG_COUNT_VAL];
    uint32_t someting_wrong_section;
  }debug_timer_val_t;

  debug_timer_val_t  debug_val;

  uint32_t timer_set_val_curr_time = 0;
  uint32_t timer_set_val_time      = 0;
  uint32_t timer_diff_val_time     = 0;
#endif


enum
{
  WAIT_FOR_NO_COMPARE      = 0,
  WAIT_FOR_COMPARE    = 1,
}; //d

uint32_t  LOW_VAL       = 0;
uint8_t   timer_state   = 0;
uint32_t  timestampLow  = 0;

/*
** ============================================================================
** Private Macro definitions
** ============================================================================
*/

/* None*/

/*
** ============================================================================
** Private Structures, Unions & enums Type Definitions
** ============================================================================
*/


/* None*/



/*
** ============================================================================
** Private Variable Definitions
** ============================================================================
*/

static uint8_t rnd_index   = 0;

static uint8_t rnd_array[] =
{
    0xd9, 0x7a, 0x96, 0x09, 0x2c, 0xa5, 0x57, 0x74,
    0xdb, 0x5e, 0x20, 0xfb, 0x38, 0xa8, 0x4e, 0xa6,
    0x8d, 0x43, 0x7b, 0xbe, 0x90, 0x16, 0x89, 0x9d,
    0xb4, 0x30, 0xd4, 0x34, 0x9d, 0x3a, 0x0d, 0x0f,
    0xd9, 0x3c, 0xa8, 0x35, 0xe5, 0x8f, 0x2a, 0xcd,
    0x29, 0x23, 0xb0, 0x5b, 0x1b, 0xa3, 0x2a, 0x82,
    0xae, 0x0f, 0x84, 0xcd, 0x49, 0xd6, 0x83, 0x42,
    0xf1, 0x8a, 0xfe, 0x21, 0x1b, 0x19, 0x7e, 0xe4
};

/*
** ============================================================================
** Public Variable Definitions
** ============================================================================
*/

hw_tmr_t* mp_hw_tmr_inst = NULL;


/*
** ============================================================================
** External Variable Declarations
** ============================================================================
*/

extern p3time_t system_time_high_32;

/*
** ============================================================================
** Private Function Prototypes
** ============================================================================
*/


/*
** ============================================================================
** Public Function Definitions
** ============================================================================
*/

#ifdef CLOCK_USING_MAC_SYSTEM_TIMER
void clock_tick_processor( void );
#endif

static void us_timer_isr(void) //d
{
  
#if 0 // This is for FG13
  switch(timer_state) 
  {
    case WAIT_FOR_COMPARE:
	  TIMER_IntClear(WTIMER0, TIMER_IF_CC0);
	  timer_state = WAIT_FOR_NO_COMPARE;
	  mp_hw_tmr_inst->cb(mp_hw_tmr_inst->ctx);
	  break;         
    default:
	  mp_hw_tmr_inst->cb(mp_hw_tmr_inst->ctx);
	  break;
  }
  
#else  // This is for FG23
  switch(timer_state) 
  {
    case WAIT_FOR_COMPARE:
	  TIMER_IntClear(TIMER0, TIMER_IF_CC0);
	  timer_state = WAIT_FOR_NO_COMPARE;
	  mp_hw_tmr_inst->cb(mp_hw_tmr_inst->ctx);
	  break;         
    default:
	  mp_hw_tmr_inst->cb(mp_hw_tmr_inst->ctx);
	  break;
  }
  
#endif
}



/**************************************************************************//**
 * @brief
 *    TIMER 0 handler
 *****************************************************************************/
//void TIMER0_IRQHandler(void)
//{
////  static uint32_t i = 0;
//
//  // Acknowledge the interrupt
//  uint32_t flags = TIMER_IntGet(TIMER0);
//  TIMER_IntClear(TIMER0, flags);
//
//  // Check for capture event on channel 0
//  if (flags & TIMER_IF_CC0) {
//
//    // Record input capture value
//   TIMER_CaptureGet(TIMER0, 0);
//#ifdef CLOCK_USING_MAC_SYSTEM_TIMER
//    clock_tick_processor();
//#endif
//    // Increment index and have it wrap around
//  }
//}

/**************************************************************************//**
 * @brief TIMER0_IRQHandler
 * Interrupt Service Routine TIMER0 Interrupt Line
 *****************************************************************************/
#if 0 // This is for FG13
void WTIMER0_IRQHandler(void) //d
{ 
  uint16_t intFlags = TIMER_IntGet(WTIMER0);
  TIMER_IntClear(WTIMER0, TIMER_IF_OF | TIMER_IF_CC0);
  
  /* Overflow interrupt occured */
  if(intFlags & TIMER_IF_OF)
  {
    mp_hw_tmr_inst->rollover_handled = 0; 
    us_timer_isr();
          
 
  }
  
  /* Capture interrupt occured */
  if((timer_state == WAIT_FOR_COMPARE) && (intFlags & TIMER_IF_CC0))
  {      
    TIMER_IntDisable(WTIMER0, TIMER_IF_CC0);
    us_timer_isr();
  }
}

#else //This is for FG23

void TIMER0_IRQHandler(void) //d
{ 
  uint16_t intFlags = TIMER_IntGet(TIMER0);
  TIMER_IntClear(TIMER0, TIMER_IF_OF | TIMER_IF_CC0);
  
  /* Overflow interrupt occured */
  if(intFlags & TIMER_IF_OF)
  {
    mp_hw_tmr_inst->rollover_handled = 0; 
    us_timer_isr();
          
 
  }
  
  /* Capture interrupt occured */
  if((timer_state == WAIT_FOR_COMPARE) && (intFlags & TIMER_IF_CC0))
  {      
    TIMER_IntDisable(TIMER0, TIMER_IF_CC0);
    us_timer_isr();
  }
}

#endif


void TIMER1_IRQHandler()
{
  uint16_t intFlags = TIMER_IntGet(TIMER1);
  TIMER_IntClear(TIMER1, TIMER_IF_OF);
  if(intFlags & TIMER_IF_OF)
  {
#ifdef CLOCK_USING_MAC_SYSTEM_TIMER
    clock_tick_processor();
#endif
  }
  
}
void timer_initialization ( void ) //d
{
 // CMU_ClockEnable(cmuClock_HFPER, true); //d  
  
  /* Enable clock for TIMER0 module */
  CMU_ClockEnable(cmuClock_TIMER0, true);
     
//  /* Select CC channel parameters */
//  TIMER_InitCC_TypeDef timerCCInit = 
//  {
//    .eventCtrl  = timerEventEveryEdge,
//    .edge       = timerEdgeBoth,
//    .prsSel     = timerPRSSELCh0,
//    .cufoa      = timerOutputActionNone,
//    .cofoa      = timerOutputActionNone,
//    .cmoa       = timerOutputActionToggle,
//    .mode       = timerCCModePWM,
//    .filter     = false,
//    .prsInput   = false,
//    .coist      = false,
//    .outInvert  = false,
//  };
//  
//  /* Configure CC channel 0 */
//  TIMER_InitCC(WTIMER0, 0, &timerCCInit);
  
  
   // Configure WTIMER0 Compare/Capture for output compare
  // Use PWM mode, which sets output on overflow and clears on compare events
  TIMER_InitCC_TypeDef timerCCInit = TIMER_INITCC_DEFAULT;
  timerCCInit.mode = timerCCModePWM;
  TIMER_InitCC(TIMER0, 0, &timerCCInit);

  // Route WTIMER0 CC0 to location 30 and enable CC0 route pin
  // WTIM0_CC0 #30 is GPIO Pin PC10
 // TIMER0->ROUTELOC0 |=  TIMER_ROUTELOC0_CC0LOC_LOC30;  //Needed to be checked
 // TIMER0->ROUTEPEN |= TIMER_ROUTEPEN_CC0PEN;
  
  
  /* Set Top Value */
  TIMER_TopSet(TIMER0, (0xFFFFFFFF-1));

//  /* Select timer parameters */  
//  TIMER_Init_TypeDef timerInit =
//  {
//    .enable     = false,
//    .debugRun   = false,
//    .prescale   = timerPrescale32,
//    .clkSel     = timerClkSelHFPerClk,
//    .fallAction = timerInputActionNone,
//    .riseAction = timerInputActionNone,
//    .mode       = timerModeUp,
//    .dmaClrAct  = false,
//    .quadModeX4 = false,
//    .oneShot    = false,
//    .sync       = false,
//  };
//  
//  /* Enable overflow interrupt */ 
//  TIMER_IntEnable(WTIMER0, TIMER_IF_OF );  
//  /* Enable TIMER0 interrupt vector in NVIC */
//  NVIC_EnableIRQ(WTIMER0_IRQn);  
//  /* Configure timer */
//  TIMER_Init(WTIMER0, &timerInit);
  
  
   // Initialize the timer
  TIMER_Init_TypeDef timerInit = TIMER_INIT_DEFAULT;
  timerInit.prescale = timerPrescale32;
  TIMER_Init(TIMER0, &timerInit);
  
  
   // Enable WTIMER0 compare event interrupts to update the duty cycle
  //TIMER_IntEnable(WTIMER0, TIMER_IEN_CC0);
  TIMER_IntEnable(TIMER0, TIMER_IF_OF ); 
  NVIC_EnableIRQ(TIMER0_IRQn);
  
  
  

 /********************* For Contiki timers **********************************/
  CMU_ClockEnable(cmuClock_TIMER1, true);  
  TIMER_Init_TypeDef timerInit_2 = TIMER2_INIT_DEFAULT;
  TIMER_InitCC_TypeDef timerCCInit_2 = TIMER2_INITCC_DEFAULT;
  TIMER_InitCC(TIMER1, 0, &timerCCInit_2);
  
  TIMER_TopSet(TIMER1, (0X3E8-1));
 
  TIMER_IntEnable(TIMER1, TIMER_IF_OF );
  NVIC_EnableIRQ(TIMER1_IRQn);  
  TIMER_Init(TIMER1, &timerInit_2);
  TIMER_Enable(TIMER1, true);
}

uint32_t us_timer_read(void) //d
{
  uint32_t currTime = 0;
  
  currTime = TIMER0->CNT;
  currTime /= CLOCK_TICK_RESOLUTION;
 
  return currTime;
}

void us_timer_set_value(uint32_t timestamp) //d
{  
  timestamp *= CLOCK_TICK_RESOLUTION;
   
  timestamp -= 1;   
        
  if ((TIMER0->CNT + 100) < timestamp)
  {
    timer_state = WAIT_FOR_COMPARE;
    TIMER_CompareSet(TIMER0, 0, timestamp);
    TIMER_IntEnable(TIMER0, TIMER_IF_CC0);
  }
  else
  {
    timer_state = WAIT_FOR_NO_COMPARE;
    //something wrong
    us_timer_isr();//just invoke callback expiry
  } 
}

                
void hw_tmr_init( void *hw_tmr_ins,  void *sw_tmr_mod_ins )
{
  mp_hw_tmr_inst = (hw_tmr_t*)hw_tmr_ins;     
  mp_hw_tmr_inst->rollover_handled = 1;      
  timer_initialization ();
}
/******************************************************************************/

void hw_tmr_uninit( void *hw_tmr_ins,  void *sw_tmr_mod_ins )
{
  TIMER_IntClear(TIMER0, TIMER_IF_CC0);
}

/******************************************************************************/
uint16_t hw_tmr_sys_time_high( void ) //d
{
  uint32_t curr;
  uint16_t high16 = 0;
  
  curr = TIMER0->CNT;
  high16 = curr >> 16;
  
  return (high16 & 0xFFFF);
}
/******************************************************************************/

p3time_t hw_tmr_get_time( void *hw_tmr_ins )
{  
  p3time_t current_time;
  current_time = us_timer_read() ;
  return current_time;
}


p3time_t hw_tmr_get_time_int( void *hw_tmr_ins ) //d
{  
  uint32_t current_time;
  
  current_time = TIMER0->CNT;
  current_time /= CLOCK_TICK_RESOLUTION;
  
  return current_time ;
}

/******************************************************************************/

void hw_tmr_start( void *hw_tmr_ins, p3time_t cycles_at_expiry_us )
{
  p3time_t cycles_now;    
  uint32_t  diff_time = 0;

  cycles_now = hw_tmr_get_time(hw_tmr_ins);	
      
  #if 0           
  if( cycles_at_expiry_us > cycles_now )      
    us_timer_set_value(cycles_at_expiry_us);
  else
    mp_hw_tmr_inst->cb(mp_hw_tmr_inst->ctx);

  #else
  if( cycles_at_expiry_us > cycles_now )
    diff_time = cycles_at_expiry_us - cycles_now;
  else
    diff_time =  cycles_now - cycles_at_expiry_us;
     
  if( diff_time > 100 )       
    us_timer_set_value(cycles_at_expiry_us);
  else
    mp_hw_tmr_inst->cb(mp_hw_tmr_inst->ctx);        
  #endif
      
}

/******************************************************************************/

void hw_tmr_stop( void *hw_tmr_ins )
{    
  TIMER_CompareSet(TIMER0,0,0);
  TIMER_CompareSet(TIMER0,0,0);
  timer_state = WAIT_FOR_NO_COMPARE;
  TIMER_IntClear(TIMER0, TIMER_IF_CC0);
  mp_hw_tmr_inst->low_time_us = 0;
}

/******************************************************************************/

uint8_t hw_tmr_rand( void *hw_tmr_ins )
{
  p3time_t seed = 0x00;
  rnd_index += 1;	
  seed = hw_tmr_get_time(hw_tmr_ins);
  return (rnd_array[ (rnd_index & 0x3f) ] ^ (seed & 0xff));
}

/******************************************************************************/

void hw_tmr_delay( void *hw_tmr_ins, uint32_t duration_in_us )
{
  p3time_t cycles_now = 0;
  p3time_t target_cycles = hw_tmr_get_time(hw_tmr_ins);        
  target_cycles += duration_in_us;	
  do
    cycles_now = hw_tmr_get_time(hw_tmr_ins);
  while( target_cycles > cycles_now );
}

/******************************************************************************/

uint32_t hw_tmr_get_symbols( uint32_t time )
{
  // TO DO : once the PHY library is integerated : Raka
  //return (time/HWTIMER_SYMBOL_LENGTH);  //time /20 usecs //d
  return 0; //d
}




/*
** ============================================================================
** Private Function Definitions
** ============================================================================
*/




