/*
 * Copyright (c) 2007, Swedish Institute of Computer Science.
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
 */

/**
 * \file
 *         Hardware-dependent function declarations used to
 *         support the contiki rtimer module.
 *
 *         We use Timer 1 on the cc2431.
 *
 * \author
 *         Zach Shelby (Original)
 *         George Oikonomou - <oikonomou@users.sourceforge.net>
 *           (rtimer-arch implementation for cc2430)
 */

#ifndef __RTIMER_ARCH_H__
#define __RTIMER_ARCH_H__
//#include "r_cg_macrodriver.h"
//#include "r_cg_timer.h"
//#include "contiki-conf.h"


/*
 * 32 MHz clock, prescaled down to 7812.5 Hz. Each tick is 128 us
 */
#define RTIMER_ARCH_SECOND (7812U)
// since this is down counter, the value from the counter register of 
//timer 3 which is configured as interval timer ( free running timer running at 
//a freq  7812.5 Hz and ticking at every 128 us) should be 
//subtracted from 0xFFFF to get the current time
#define rtimer_arch_now() ((rtimer_clock_t)(0xFFFF - TCR03))

//void cc2430_timer_1_ISR(void) __interrupt(T1_VECTOR);TBDAnand

#endif /* __RTIMER_ARCH_H__ */
