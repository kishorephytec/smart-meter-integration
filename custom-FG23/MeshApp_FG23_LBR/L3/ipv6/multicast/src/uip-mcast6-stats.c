/**
 * Copyright (c) 2014, University of Bristol - http://www.bris.ac.uk
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
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 **/
/**
 * \addtogroup uip6-multicast
 * @{
 */
/**
 * \file
 *    IPv6 multicast forwarding stats maintenance
 *
 * \author
 *    George Oikonomou - <oikonomou@users.sourceforge.net>
 */
#include "Stack6LoWPANConf.h"
#include "l3_configuration.h"
#include "l3_timer_utility.h"
#include "contiki-net.h"

#include <string.h>
/*---------------------------------------------------------------------------*/
uip_mcast6_stats_t uip_mcast6_stats;
/*---------------------------------------------------------------------------*/
void
uip_mcast6_stats_init(void *stats)
{
  memset(&uip_mcast6_stats, 0, sizeof(uip_mcast6_stats));
  uip_mcast6_stats.engine_stats = stats;
}
/*---------------------------------------------------------------------------*/
/** @} */
