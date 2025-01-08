/** \file l3_timer_utility.h
 *******************************************************************************
 ** \brief  Contains all the Processor architecture selection macros for selection 
 ** of processor Architecture.
 **
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


#ifndef _CONFIG_L3_TIMER_UTILITY_H_
#define _CONFIG_L3_TIMER_UTILITY_H_


#include "l3_configuration.h"
#if (L3_SUPPORTING_OS == OS_CONTIKI)
#include "timer.h"
#include "etimer.h"
#include "ctimer.h"
#include "stimer.h"
#include "rtimer.h"
#include "clock.h"
#include "energest.h"
#endif

#if (L3_SUPPORTING_OS == OS_NONE) 
#define CLOCK_SECOND            1000
#endif

#if (L3_SUPPORTING_OS == OS_NONE) 
typedef unsigned long long l3_clock_time_t;
#elif (L3_SUPPORTING_OS == OS_CONTIKI)  
typedef clock_time_t l3_clock_time_t;
#endif



typedef struct l3_etimer
{
#if (L3_SUPPORTING_OS == OS_NONE)  
  void *et;
#elif (L3_SUPPORTING_OS == OS_CONTIKI)  
  struct etimer et;
#endif
}l3_etimer_t;

void l3_etimer_set (l3_etimer_t *et, l3_clock_time_t interval);
void l3_etimer_reset (l3_etimer_t *et);
void l3_etimer_reset_with_new_interval (l3_etimer_t *et, l3_clock_time_t interval);
void l3_etimer_restart (l3_etimer_t *et);
void l3_etimer_adjust (l3_etimer_t *et, int td);
l3_clock_time_t l3_etimer_expiration_time (l3_etimer_t *et);
l3_clock_time_t l3_etimer_start_time (l3_etimer_t *et);
int l3_etimer_expired (l3_etimer_t *et);
void l3_etimer_stop (l3_etimer_t *et);
void l3_etimer_request_poll (void);
int l3_etimer_pending (void);
l3_clock_time_t l3_etimer_next_expiration_time (void);




typedef struct l3_ctimer
{
#if (L3_SUPPORTING_OS == OS_NONE)
  void *ct;
#elif (L3_SUPPORTING_OS == OS_CONTIKI)  
  struct ctimer ct;
#endif  
}l3_ctimer_t;

void l3_ctimer_reset (l3_ctimer_t *c);
void l3_ctimer_restart (l3_ctimer_t *c);
void l3_ctimer_set (l3_ctimer_t *c, l3_clock_time_t t, void (*f)(void *), void *ptr);
void l3_ctimer_stop (l3_ctimer_t *c);
int l3_ctimer_expired (l3_ctimer_t *c);
l3_clock_time_t l3_ctimer_expiration_time (l3_ctimer_t *c);




typedef struct l3_stimer
{
#if (L3_SUPPORTING_OS == OS_NONE)  
  void *st;
#elif (L3_SUPPORTING_OS == OS_CONTIKI)  
  struct stimer st;
#endif
}l3_stimer_t;

void l3_stimer_set (l3_stimer_t *t, unsigned long interval);
void l3_stimer_reset (l3_stimer_t *t);
void l3_stimer_restart (l3_stimer_t *t);
int l3_stimer_expired (l3_stimer_t *t);
unsigned long l3_stimer_remaining (l3_stimer_t *t);
unsigned long l3_stimer_elapsed (l3_stimer_t *t);




l3_clock_time_t l3_clock_time (void);





typedef struct l3_timer
{
#if (L3_SUPPORTING_OS == OS_NONE)  
  void *t;
#elif (L3_SUPPORTING_OS == OS_CONTIKI)  
  struct timer t;
#endif
}l3_timer_t;

void l3_timer_set (l3_timer_t *t, l3_clock_time_t interval);
void l3_timer_reset (l3_timer_t *t);
void l3_timer_restart (l3_timer_t *t);
int l3_timer_expired (l3_timer_t *t);
l3_clock_time_t l3_timer_remaining (l3_timer_t *t);


#endif //_CONFIG_L3_TIMER_UTILITY_H_
