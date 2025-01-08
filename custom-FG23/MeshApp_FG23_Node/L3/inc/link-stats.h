/*
 * Copyright (c) 2015, SICS Swedish ICT.
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
 *
 * Authors: Simon Duquennoy <simonduq@sics.se>
 */

#ifndef LINK_STATS_H_
#define LINK_STATS_H_

#include "linkaddr.h"
//#include "uip.h"

/* ETX fixed point divisor. 128 is the value used by RPL (RFC 6551 and RFC 6719) */
#ifdef LINK_STATS_CONF_ETX_DIVISOR
#define LINK_STATS_ETX_DIVISOR              LINK_STATS_CONF_ETX_DIVISOR
#else /* LINK_STATS_CONF_ETX_DIVISOR */
#define LINK_STATS_ETX_DIVISOR              128
#endif /* LINK_STATS_CONF_ETX_DIVISOR */

#define MAX_EWMA_COUNT 4

/* All statistics of a given link */
struct link_stats {
  uint16_t etx;               /* ETX using ETX_DIVISOR as fixed point divisor */
  uint16_t ewma_etx[MAX_EWMA_COUNT];
  uint16_t path_cost;
  uint8_t tx_count;           //for calculation within 30 tx
  uint8_t rx_count;
  int16_t rssi;               /* RSSI (received signal strength) */
  int16_t ewma_rssi[MAX_EWMA_COUNT];
  int32_t rsl_ie;             //updating rsl from ns-ack 
  int32_t ewma_rsl_ie[MAX_EWMA_COUNT];
  uint8_t freshness;          /* Freshness of the statistics */
  clock_time_t last_tx_time;  /* Last Tx timestamp */
#if (EFR32FG13P_LBR == 0x00)  
  uint8_t ll_addr[8];          /*address of currunt link using this for nvm*/
#endif  
};



/* Returns the neighbor's link statistics */
const struct link_stats *link_stats_from_lladdr(const linkaddr_t *lladdr);
/* Are the statistics fresh? */
int link_stats_is_fresh(const struct link_stats *stats);

/* Initializes link-stats module */
void link_stats_init(void);
/* Packet sent callback. Updates statistics for transmissions on a given link */
void link_stats_packet_sent(const linkaddr_t *lladdr, int status, int numtx);
/* Packet input callback. Updates statistics for receptions on a given link */
void link_stats_input_callback(const linkaddr_t *lladdr);

#define RPL_PROBING_SEND_NS_FUNC(src, target1, target2, aro, lifetime) uip_nd6_ns_aro_output((src), (target1), (target2),\
   (aro), (lifetime))
#endif /* LINK_STATS_H_ */
