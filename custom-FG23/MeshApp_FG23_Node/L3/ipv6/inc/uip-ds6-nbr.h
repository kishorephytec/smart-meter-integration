/**
 * Copyright (c) 2013, Swedish Institute of Computer Science.
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
 */

/**
 * \addtogroup uip6
 * @{
 */

/**
 * \file
 *    IPv6 Neighbor cache (link-layer/IPv6 address mapping)
 * \author Mathilde Durvy <mdurvy@cisco.com>
 * \author Julien Abeille <jabeille@cisco.com>
 * \author Simon Duquennoy <simonduq@sics.se>
 *
 */

#ifndef UIP_DS6_NEIGHBOR_H_
#define UIP_DS6_NEIGHBOR_H_

#include "contiki-net.h"

#if UIP_CONF_IPV6_QUEUE_PKT
#include "uip-packetqueue.h"
#endif                          /*UIP_CONF_QUEUE_PKT */

/*--------------------------------------------------*/
/** \brief Possible states for the nbr cache entries */
#define  NBR_INCOMPLETE         0
#define  NBR_REACHABLE          1
#define  NBR_STALE              2
#define  NBR_DELAY              3
#define  NBR_PROBE              4//in case of NS
#define  NBR_DATA_PROBE         5//in case of all unicast data
#define  NBR_RPL_DONE           6
#define  NBR_COLD_STARTUP       7//only for waiting a duration of 3 mins, before selecting a best parent in case of router

NBR_TABLE_DECLARE(ds6_neighbors);

/** \brief An entry in the nbr cache */
typedef struct uip_ds6_nbr {
  uip_ipaddr_t ipaddr;
  uip_lladdr_t lladdr;
  uip_ipaddr_t global_addr;
  uint8_t isrouter;
  uint8_t state;
  uint8_t d_link_status;
  uint8_t ns_sent;
  uint8_t ns_rcvd;
#if UIP_ND6_SEND_NA || UIP_ND6_SEND_RA
  l3_stimer_t reachable;
  l3_stimer_t sendns;
  uint8_t nscount;
  uint8_t data_test;//santosh: temp var need to remove
#endif /* UIP_ND6_SEND_NA || UIP_ND6_SEND_RA */
#if UIP_CONF_IPV6_QUEUE_PKT
  struct uip_packetqueue_handle packethandle;
#define UIP_DS6_NBR_PACKET_LIFETIME CLOCK_SECOND * 4
#endif                          /*UIP_CONF_QUEUE_PKT */
  uint8_t parent_priority;
}uip_ds6_nbr_t;

void uip_ds6_neighbors_init(void);

/** \brief Neighbor Cache basic routines */
uip_ds6_nbr_t *uip_ds6_nbr_add(const uip_ipaddr_t *ipaddr,
                               const uip_lladdr_t *lladdr,
                               uint8_t isrouter, uint8_t state,
                               nbr_table_reason_t reason, void *data);
int uip_ds6_nbr_rm(uip_ds6_nbr_t *nbr);
const uip_lladdr_t *uip_ds6_nbr_get_ll(const uip_ds6_nbr_t *nbr);
const uip_ipaddr_t *uip_ds6_nbr_get_ipaddr(const uip_ds6_nbr_t *nbr);
uip_ds6_nbr_t *uip_ds6_nbr_lookup(const uip_ipaddr_t *ipaddr);
uip_ds6_nbr_t *uip_ds6_nbr_ll_lookup(const uip_lladdr_t *lladdr);
uip_ipaddr_t *uip_ds6_nbr_ipaddr_from_lladdr(const uip_lladdr_t *lladdr);
const uip_lladdr_t *uip_ds6_nbr_lladdr_from_ipaddr(const uip_ipaddr_t *ipaddr);
void uip_ds6_link_neighbor_callback(int status, int numtx);
void uip_ds6_neighbor_periodic(void);
int uip_ds6_nbr_num(void);

#if UIP_ND6_SEND_NA
/**
 * \brief Refresh the reachable state of a neighbor. This function
 * may be called when a node receives an IPv6 message that confirms the
 * reachability of a neighbor.
 * \param ipaddr pointer to the IPv6 address whose neighbor reachability state
 * should be refreshed.
 */
void uip_ds6_nbr_refresh_reachable_state(const uip_ipaddr_t *ipaddr);
#endif /* UIP_ND6_SEND_NA */

/**
 * \brief
 *    This searches inside the neighbor table for the neighbor that is about to
 *    expire the next.
 *
 * \return
 *    A reference to the neighbor about to expire the next or NULL if
 *    table is empty.
 */
uip_ds6_nbr_t *uip_ds6_get_least_lifetime_neighbor(void);

uip_ipaddr_t *uip_get_prio_based_parent_ip(uint8_t);

#endif /* UIP_DS6_NEIGHBOR_H_ */
/** @} */
