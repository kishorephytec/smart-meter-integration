/** \file clock.c
 **
 ** \brief Implements APIs for Clock.
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

#include "StackAppConf.h"
#include "common.h"
#include "l3_configuration.h"
#include "l3_timer_utility.h"

//#include "sys/clock.h"
//#include "sys/etimer.h"
//#include "sys/energest.h"
//#include "sys/clock.h"
//#include "sys/etimer.h"
//#include "sys/energest.h"

//#include "sfr-bits.h"
//#include "cc253x.h"
//#include "r_cg_macrodriver.h"
//#include "r_cg_timer.h"
/* Start user code for include. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */
//#include "r_cg_userdefine.h"
//#include "energest.h"
#include "em_rtcc.h"
#include "em_cmu.h"
#include "time.h"

#define RTC_PRESENT

//#define CLOCK_USING_MAC_SOFT_TIMER
//#define CLOCK_USING_MAC_SYSTEM_TIMER

#ifdef CLOCK_USING_MAC_SOFT_TIMER

#include "board.h"
#include "list_latest.h"
#include "queue_latest.h"
#include "buff_mgmt.h"
#include "hw_tmr.h"
#include "sw_timer.h"
#include "timer_service.h"

//#include "r_cg_port.h"
//#include "r_cg_wdt.h"
#endif

/*
** =============================================================================
** Private Macro definitions
** =============================================================================
*/
	
//#define MAX_TICKS (~((clock_time_t)0) / 2)
#define MAX_TICKS (0x7FFF)

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

/* Do NOT remove the absolute address and do NOT remove the initialiser here */
//__xdata __at(0x0000) unsigned long timer_value = 0; // TODO
static volatile unsigned long timer_value = 0;

/* Sleep timer runs on the 32k RC osc. */
/* One clock tick is 7.8 ms */
//#define TICK_VAL (32768/128)  /* 256 */
#ifdef CLOCK_USING_MAC_SOFT_TIMER
static sw_tmr_t sw_tmr_for_clock;  

/*
** =============================================================================
** Private Function Prototypes
** =============================================================================
*/

//static void sw_mac_clock_timer_expiry_handler( void* s, void* tmr  );
#endif
#define SLEEPTIMER_EVENT_OF (0x01)
#define SLEEPTIMER_EVENT_COMP (0x02)
#define SL_SLEEPTIMER_FREQ_DIVIDER  1

/* RTC variables. Used for converting RTC counter to system time */
static uint16_t   rtcCountsPerSec       = 0;
static time_t     rtcStartTime          = 0;
static uint32_t   rtcOverflowCounter    = 0;
static uint32_t   rtcOverflowInterval   = 0;
static uint32_t   rtcOverflowIntervalR  = 0;

#define RTC_COUNTS_PER_SEC          32768 

//#define MAX_TICKS (~((clock_time_t)0) / 2)

//#define MAX_TICKS (0x7FFF)
/*---------------------------------------------------------------------------*/
/* Do NOT remove the absolute address and do NOT remove the initialiser here */
//__xdata __at(0x0000) unsigned long timer_value = 0; // TODO

static volatile clock_time_t count = 0; /* Uptime in ticks */
static volatile clock_time_t seconds = 0; /* Uptime in secs */

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

//static uint32_t timer_expiry_point = 0;
//static uint32_t max_ticks = MAX_TICKS;

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

/**
 * One delay is about 0.6 us, so this function delays for len * 0.6 us
 */

/*Umesh :31-01-2018*/
//static void
//clock_delay(unsigned int len)
//{
//  unsigned int i;
//  for(i = 0; i< len; i++) {
//    //_asm(nop);
//    __no_operation();
//    __no_operation();
//    __no_operation();
//    __no_operation();
//     /*TBDAnand.. This needs to be difined properly. And also the unit of this 
//    delay need to be defined*/
//  }
//}
/*this function is not used and called from anywhere*/

/*---------------------------------------------------------------------------*/
/**
 * Wait for a multiple of ~3906.25 (a tick)
 */
//static void
//clock_wait(clock_time_t i)
//{
//  clock_time_t start;
//
//  start = clock_time();
//  while(clock_time() - start < (clock_time_t)i);
//}
/*this function is not used and called from anywhere*/
/*---------------------------------------------------------------------------*/
CCIF clock_time_t
clock_time(void)
{
  //return RTCC_CounterGet();
  return count;
}
/*---------------------------------------------------------------------------*/
CCIF unsigned long
clock_seconds(void)
{
  return seconds;
}
/*---------------------------------------------------------------------------*/
/*
 * There is some ambiguity between TI cc2530 software examples and information
 * in the datasheet.
 *
 * TI examples appear to be writing to SLEEPCMD, initialising hardware in a
 * fashion semi-similar to cc2430
 *
 * However, the datasheet claims that those bits in SLEEPCMD are reserved
 *
 * The code here goes by the datasheet (ignore TI examples) and seems to work.
 */
void
clock_init(void)
{
#ifdef CLOCK_USING_MAC_SOFT_TIMER
  tmr_create_one_shot_timer
  (
          &(sw_tmr_for_clock),
          15625U, // usecs
          (sw_tmr_cb_t) sw_mac_clock_timer_expiry_handler,
          NULL
  );
  
  tmr_start_relative( &(sw_tmr_for_clock ) );
#else 
#ifdef CLOCK_USING_MAC_SYSTEM_TIMER

#else
 //R_TAU0_Channel2_Start();
#endif
#endif
  
}
/*---------------------------------------------------------------------------*/
//#if(0)
//#ifdef SDCC
//  void clock_isr(void) __interrupt(ST_VECTOR)
//#else
//  #pragma vector=ST_VECTOR
//  __near_func __interrupt void clock_isr(void)
//#endif
//#endif
//void timer2_interrupt_callback(void)
//{
//  DISABLE_INTERRUPTS();
//  ENERGEST_ON(ENERGEST_TYPE_IRQ);
//
//  /*
//   * If the Sleep timer throws an interrupt while we are powering down to
//   * PM1, we need to abort the power down. Clear SLEEP.MODE, this will signal
//   * main() to abort the PM1 transition
//   *
//   * On cc2430 this would be:
//   * SLEEPCMD &= 0xFC;
//   */
//
//  /*
//   * Read value of the ST0:ST1:ST2, add TICK_VAL and write it back.
//   * Next interrupt occurs after the current time + TICK_VAL
//   */
////  timer_value = ST0;
////  timer_value += ((unsigned long int) ST1) << 8;
////  timer_value += ((unsigned long int) ST2) << 16;
////  timer_value += TICK_VAL;
////  ST2 = (unsigned char) (timer_value >> 16);
////  ST1 = (unsigned char) (timer_value >> 8);
////  ST0 = (unsigned char) timer_value;
//  
//  //Increment after every 3906 us
//  
//     if( P1 & 0x20 )//check if set
//     {
//       //reset
//       P1 &= 0xDF;
//       
//     }
//     else
//     {
//        P1 |= 0x20;
//     }
//  
//  ++count;
//  
//  /* Make sure the CLOCK_CONF_SECOND is a power of two, to ensure
//     that the modulo operation below becomes a logical and and not
//     an expensive divide. Algorithm from Wikipedia:
//     http://en.wikipedia.org/wiki/Power_of_two */
//#if (CLOCK_CONF_SECOND & (CLOCK_CONF_SECOND - 1)) != 0
//#error CLOCK_CONF_SECOND must be a power of two (i.e., 1, 2, 4, 8, 16, 32, 64, ...).
//#error Change CLOCK_CONF_SECOND in contiki-conf.h.
//#endif
//  if(count % CLOCK_CONF_SECOND == 0) {
//    ++seconds;//3906 us * 256(CLOCK_CONF_SECOND)= 1 sec
//  }
//  if( !etimer_poll_event_pending() )
//  {
//        if(etimer_pending()
//          && (etimer_next_expiration_time() - count - 1) > MAX_TICKS) {
//        etimer_request_poll();
//  }
//
//  }
//  
////  STIF = 0; /* IRCON.STIF */
//  TMIF02 = 0U;    /* clear INTTM02 interrupt flag */// TBDAnand... this may not be required since it is a interval timer
//  ENERGEST_OFF(ENERGEST_TYPE_IRQ);
//  ENABLE_INTERRUPTS();
//}
/*----------------------------------------------------------------------------*/

void handle_phy_busy_rx_error (void);

#ifdef CLOCK_USING_MAC_SYSTEM_TIMER
void clock_tick_processor( void )
{
  ++count;
  if(count % CLOCK_CONF_SECOND == 0) {
    ++seconds;
    handle_phy_busy_rx_error ();
  }
  
  if(l3_etimer_pending()) 
  {
//    timer_expiry_point = etimer_next_expiration_time() - count - 1;
//    if( (timer_expiry_point) > max_ticks )
      l3_etimer_request_poll();
  }
}
#endif
/*---------------------------------------------------------------------------*/

/*-------------------------Debdeep added for RTC------------------------------*/
//void rtcSetup(void)
//{
//
//  RTCC_Init_TypeDef rtcInit;
//
//  /* Configure RTC */
//  rtcInit.debugRun = false;
//  rtcInit.comp0Top = false;
//  rtcInit.enable = false;
//
//  /* Initialize RTC */
//  RTCC_Init(&rtcInit);
//
//  /* Enable COMP0 interrupt to update the display */
//  /* Enable overflow interrupt to keep track of overflows */
//  RTCC_IntEnable(RTC_IEN_COMP0 | RTC_IEN_OF);
//  
//  /* Enable RTC */
//  RTCC_Enable(true);
//}

/***************************************************************************//**
 * @brief Set the epoch offset
 *
 * @param[in] timeptr
 *   Calendar struct which is converted to unix time and used as new epoch 
 *   offset
 *
 ******************************************************************************/
void clockSetStartCalendar(struct tm * timeptr)
{
  rtcStartTime = mktime(timeptr); 
}

extern void mem_rev_cpy(uint8_t* dest, uint8_t* src, uint16_t len );
void set_rtc_start_time (uint8_t *data, uint16_t len)
{
  uint8_t time_now[8] = {0};
  
  memcpy (time_now, data, len);
  mem_rev_cpy ((uint8_t *)&rtcStartTime, time_now + 4, sizeof (rtcStartTime));
//  rtcStartTime = time_now;
}

void rtc_init (void)
{
//   RTCC_Init_TypeDef rtcc_init   = RTCC_INIT_DEFAULT;
//  RTCC_CCChConf_TypeDef channel = RTCC_CH_INIT_COMPARE_DEFAULT;
//
//  CMU_ClockEnable(cmuClock_RTCC, true);
//
//  rtcc_init.enable = false;
//  rtcc_init.presc = (RTCC_CntPresc_TypeDef)(CMU_PrescToLog2(SL_SLEEPTIMER_FREQ_DIVIDER - 1));
//#if (SL_SLEEPTIMER_DEBUGRUN == 1)
//  rtcc_init.debugRun = true;
//#endif
//
//  RTCC_Init(&rtcc_init);
//
//  // Compare channel starts disabled and is enabled only when compare match interrupt is enabled.
//  channel.chMode = rtccCapComChModeOff;
//  RTCC_ChannelInit(1u, &channel);
//
//  RTCC_IntDisable(_RTCC_IEN_MASK);
//  RTCC_IntClear(_RTCC_IF_MASK);
//  RTCC_CounterSet(0u);
//
//  RTCC_Enable(true);
//
//  NVIC_ClearPendingIRQ(RTCC_IRQn);
//  NVIC_EnableIRQ(RTCC_IRQn);
}

uint32_t rtc_check = 0;

void check_rtc ()
{
  //rtc_check = ( RTC->CNT / rtcCountsPerSec );
}

time_t __time32 (time_t * timer)
{
  time_t t;

  /* Add the time offset */
//  t = rtcStartTime;
//
//  /* Add time based on number of counter overflows*/
//  t += rtcOverflowCounter * rtcOverflowInterval;
//
//  /* Add remainder if the overflow interval is not an integer */   
//  if ( rtcOverflowIntervalR != 0 )
//  {
//    t += (rtcOverflowCounter * rtcOverflowIntervalR) / rtcCountsPerSec;
//  }
//  
//  /* Add the number of seconds for RTC */
//  t += ( RTCC->CNT / rtcCountsPerSec );
//
//  /* Copy system time to timer if not NULL*/  
//  if ( timer != NULL )
//  {
//    *timer = t;
//  }

  return t;
}

/***************************************************************************//**
 * @brief Call this function on counter overflow to let CLOCK know how many
 *        overflows has occured since start time
 *
 ******************************************************************************/
uint32_t clockOverflow(void)
{
  rtcOverflowCounter++;
  return rtcOverflowCounter;
}

/***************************************************************************//**
 * @brief RTC Interrupt Handler
 *
 ******************************************************************************/
void RTCC_IRQHandler(void)
{
//  uint8_t local_flag = 0;
//  uint32_t irq_flag;
//
//  irq_flag = RTCC_IntGet();
//
//  if (irq_flag & RTCC_IF_OF) {
//    local_flag |= SLEEPTIMER_EVENT_OF;
//  }
//  if (irq_flag & RTCC_IF_CC1) {
//    local_flag |= SLEEPTIMER_EVENT_COMP;
//  }
//  RTCC_IntClear(irq_flag & (RTCC_IF_OF | RTCC_IF_CC1 | RTCC_IF_CC0));
//  clock_tick_processor();
}
/*----------------------------------------------------------------------------*/
