/**
 * Copyright (c) 2011, Loughborough University - Computer Science
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
 **/

/**
 * \addtogroup uip6-multicast
 * @{
 */
/**
 * \file
 * Header file with definition of multicast engine constants
 *
 * When writing a new engine, add it here with a unique number and
 * then modify uip-mcast6.h accordingly
 *
 * \author
 *     George Oikonomou - <oikonomou@users.sourceforge.net>
 */

#ifndef UIP_MCAST6_ENGINES_H_
#define UIP_MCAST6_ENGINES_H_

///**< Enabling Muliticast packet forwarding: FAN TPS 1v10 : 6.2.3.1.8*/
#define UIP_MCAST6_CONF_ENGINE        UIP_MCAST6_ENGINE_ROLL_TM //santosh, multicast

#define UIP_MCAST6_ENGINE_NONE        0 /**< Selecting this disables mcast */
#define UIP_MCAST6_ENGINE_SMRF        1 /**< The SMRF engine */
#define UIP_MCAST6_ENGINE_ROLL_TM     2 /**< The ROLL TM engine */
#define UIP_MCAST6_ENGINE_ESMRF       3 /**< The ESMRF engine */

#endif /* UIP_MCAST6_ENGINES_H_ */
/** @} */
