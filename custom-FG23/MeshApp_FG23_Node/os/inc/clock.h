/*
 * Copyright (c) 2004, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

/** \addtogroup sys
 * @{
 */

/**
 * \defgroup clock Clock library
 *
 * The clock library is the interface between Contiki and the platform
 * specific clock functionality. The clock library defines a macro,
 * CLOCK_SECOND, to convert seconds into the tick resolution of the platform.
 * Typically this is 1-10 milliseconds, e.g. 4*CLOCK_SECOND could be 512.
 * A 16 bit counter would thus overflow every 1-10 minutes.
 * Platforms use the tick interrupt to maintain a long term count
 * of seconds since startup.
 *
 * Platforms may also implement rtimers for greater time resolution
 * and for real-time interrupts, These use a corresponding RTIMER_SECOND.
 *
 * \note These timers do not necessarily have a common divisor or are phase locked.
 * One may be crystal controlled and the other may not. Low power operation
 * or sleep will often use one for wake and disable the other, then give
 * it a tick correction after wakeup.
 *
 * \note The clock library need in many cases not be used
 * directly. Rather, the \ref timer "timer library", \ref etimer
 * "event timers", or \ref rtimer "rtimer library" should be used.
 *
 * \sa \ref timer "Timer library"
 * \sa \ref etimer "Event timers"
 * \sa \ref rtimer "Realtime library"
 *
 * @{
 */

#ifndef CLOCK_H_
#define CLOCK_H_

//#include "contiki-conf.h"
#include "time.h"

/**
 * A second, measured in system clock time.
 *
 * \hideinitializer
 */
#ifdef CLOCK_CONF_SECOND
#define CLOCK_SECOND CLOCK_CONF_SECOND
#else
#define CLOCK_SECOND (clock_time_t)32
#endif

/* Setting up a structure to initialize the calendar
   for January 1 2012 12:00:00
   The struct tm is declared in time.h 
   More information about time.h library is found on http://en.wikipedia.org/wiki/Time.h */  
#if defined (__CS_GNUC__)
#define CLOCK_DEFAULT_START_DATE                                                            \
{                                                                                           \
  0,    /* tm_sec:   0 seconds (0-60, 60 = leap second)*/                                   \
  0,    /* tm_min:   0 minutes (0-59) */                                                    \
  12,   /* tm_hour:  0 hours (0-23) */                                                      \
  15,    /* tm_mday:  1st day of the month (1 - 31) */                                       \
  9,    /* tm_mon:   January (0 - 11, 0 = January) */                                       \
  118,  /* tm_year:  Year 2012 (year since 1900) */                                         \
  0,    /* tm_wday:  Sunday (0 - 6, 0 = Sunday) */                                          \
  0,    /* tm_yday:  1st day of the year (0-365) */                                         \
  -1,   /* tm_isdst: Daylight saving time; enabled (>0), disabled (=0) or unknown (<0) */   \
  0,    /* __extra_1 */                                                                     \
  0     /* __extra_2 */                                                                     \
}
#else
#define CLOCK_DEFAULT_START_DATE                                                            \
{                                                                                           \
  0,    /* tm_sec:   0 seconds (0-60, 60 = leap second)*/                                   \
  0,    /* tm_min:   0 minutes (0-59) */                                                    \
  12,   /* tm_hour:  0 hours (0-23) */                                                      \
  15,    /* tm_mday:  1st day of the month (1 - 31) */                                       \
  9,    /* tm_mon:   January (0 - 11, 0 = January) */                                       \
  118,  /* tm_year:  Year 2018 (year since 1900) */                                         \
  0,    /* tm_wday:  Sunday (0 - 6, 0 = Sunday) */                                          \
  0,    /* tm_yday:  1st day of the year (0-365) */                                         \
  -1    /* tm_isdst: Daylight saving time; enabled (>0), disabled (=0) or unknown (<0) */   \
}
#endif  
   
/** CLOCK initialization structure. */
typedef struct
{
  struct tm startDate;          /**< Date when RTC was started */
  uint32_t rtcCountsPerSec;     /**< RTC count frequency [Hz]*/
} Clock_Init_TypeDef;
   
/** Suggested default config for Clock init structure. */
#define CLOCK_INIT_DEFAULT                                                 \
{                                                                          \
  CLOCK_DEFAULT_START_DATE,   /* Use default start date */                 \
  32768                       /* RTC frequency is 32.768 kHz */            \
}


/**
 * Initialize the clock library.
 *
 * This function initializes the clock library and should be called
 * from the main() function of the system.
 *
 */
void clock_init(void);

/**
 * Get the current clock time.
 *
 * This function returns the current system clock time.
 *
 * \return The current clock time, measured in system ticks.
 */
CCIF clock_time_t clock_time(void);

/**
 * Get the current value of the platform seconds.
 *
 * This could be the number of seconds since startup, or
 * since a standard epoch.
 *
 * \return The value.
 */
CCIF unsigned long clock_seconds(void);

/**
 * Set the value of the platform seconds.
 * \param sec   The value to set.
 *
 */
void clock_set_seconds(unsigned long sec);

/**
 * Wait for a given number of ticks.
 * \param t   How many ticks.
 *
 */
void clock_wait(clock_time_t t);

/**
 * Delay a given number of microseconds.
 * \param dt   How many microseconds to delay.
 *
 * \note Interrupts could increase the delay by a variable amount.
 */
void clock_delay_usec(uint16_t dt);

/**
 * Deprecated platform-specific routines.
 *
 */
int clock_fine_max(void);
unsigned short clock_fine(void);
void clock_delay(unsigned int delay);
void rtc_init (void);
void set_rtc_start_time (uint8_t *data, uint16_t len);

#endif /* CLOCK_H_ */

/** @} */
/** @} */
