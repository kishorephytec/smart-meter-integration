/*
 * Copyright (c) 2006, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *   may be used to endorse or promote products derived from this software
 *   without specific prior written permission.
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
 */

/**
 * \addtogroup uip6
 * @{
 */

/**
 * \file
 *    IPv6 data structure manipulation.
 *    Comprises part of the Neighbor discovery (RFC 4861)
 *    and auto configuration (RFC 4862) state machines.
 * \author Mathilde Durvy <mdurvy@cisco.com>
 * \author Julien Abeille <jabeille@cisco.com>
 */
#include "Stack6LoWPANConf.h"
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include "l3_configuration.h"
#include "l3_timer_utility.h"
#include "contiki-net.h"

#define DEBUG DEBUG_NONE
#include "uip-debug.h"

l3_etimer_t uip_ds6_timer_periodic;                           /**< Timer for maintenance of data structures */

#if UIP_CONF_ROUTER
struct stimer uip_ds6_timer_ra;                                 /**< RA timer, to schedule RA sending */
#if UIP_ND6_SEND_RA
static uint8_t racount;                                         /**< number of RA already sent */
static uint16_t rand_time;                                      /**< random time value for timers */
#endif
#else /* UIP_CONF_ROUTER */
struct etimer uip_ds6_timer_rs;                                 /**< RS timer, to schedule RS sending */
static uint8_t rscount;                                         /**< number of rs already sent */
#endif /* UIP_CONF_ROUTER */

/** \name "DS6" Data structures */
/** @{ */
uip_ds6_netif_t uip_ds6_if;                                     /**< The single interface */
uip_ds6_prefix_t uip_ds6_prefix_list[UIP_DS6_PREFIX_NB];        /**< Prefix list */

/* Used by Cooja to enable extraction of addresses from memory.*/
uint8_t uip_ds6_addr_size;
uint8_t uip_ds6_netif_addr_list_offset;

/** @} */

/* "full" (as opposed to pointer) ip address used in this file,  */
static uip_ipaddr_t loc_fipaddr;

/* Pointers used in this file */
static uip_ds6_addr_t *locaddr;
static uip_ds6_maddr_t *locmaddr;
#if UIP_DS6_AADDR_NB
static uip_ds6_aaddr_t *locaaddr;
#endif /* UIP_DS6_AADDR_NB */
static uip_ds6_prefix_t *locprefix;

uip_ds6_maddr_t *get_mcast_addr();

extern uint8_t get_dev_mac_join_state(void);
/*---------------------------------------------------------------------------*/
void
uip_ds6_init(void)
{

  uip_ds6_neighbors_init();
  uip_ds6_route_init();

  PRINTF("Init of IPv6 data structures\n");
  PRINTF("%u neighbors\n%u default routers\n%u prefixes\n%u routes\n%u unicast addresses\n%u multicast addresses\n%u anycast addresses\n",
     NBR_TABLE_MAX_NEIGHBORS, UIP_DS6_DEFRT_NB, UIP_DS6_PREFIX_NB, UIP_DS6_ROUTE_NB,
     UIP_DS6_ADDR_NB, UIP_DS6_MADDR_NB, UIP_DS6_AADDR_NB);
  memset(uip_ds6_prefix_list, 0, sizeof(uip_ds6_prefix_list));
  memset(&uip_ds6_if, 0, sizeof(uip_ds6_if));
  uip_ds6_addr_size = sizeof(struct uip_ds6_addr);
  uip_ds6_netif_addr_list_offset = offsetof(struct uip_ds6_netif, addr_list);

  /* Set interface parameters */
  uip_ds6_if.link_mtu = UIP_LINK_MTU;
  uip_ds6_if.cur_hop_limit = UIP_TTL;
  uip_ds6_if.base_reachable_time = UIP_ND6_REACHABLE_TIME;
  uip_ds6_if.reachable_time = uip_ds6_compute_reachable_time();
  uip_ds6_if.retrans_timer = UIP_ND6_RETRANS_TIMER;/*1000*/
  uip_ds6_if.maxdadns = UIP_ND6_DEF_MAXDADNS;

  /* Create link local address, prefix, multicast addresses, anycast addresses */
  uip_create_linklocal_prefix(&loc_fipaddr);
#if UIP_CONF_ROUTER
  uip_ds6_prefix_add(&loc_fipaddr, UIP_DEFAULT_PREFIX_LEN, 0, 0, 0, 0);
#else /* UIP_CONF_ROUTER */
  uip_ds6_prefix_add(&loc_fipaddr, UIP_DEFAULT_PREFIX_LEN, 0);
#endif /* UIP_CONF_ROUTER */
  uip_ds6_set_addr_iid(&loc_fipaddr, &uip_lladdr);
  uip_ds6_addr_add(&loc_fipaddr, 0, ADDR_AUTOCONF);

  uip_create_linklocal_allnodes_mcast(&loc_fipaddr);
  uip_ds6_maddr_add(&loc_fipaddr);

#if (UIP_MCAST6_CONF_ENGINE == UIP_MCAST6_ENGINE_ROLL_TM)//santosh, multicast  
  uip_create_realm_allnodes_mcast(&loc_fipaddr);
  uip_ds6_maddr_add(&loc_fipaddr);
  uip_create_mpl_forwarders_mcast(&loc_fipaddr);
  uip_ds6_maddr_add(&loc_fipaddr);
#endif
  
#if UIP_CONF_ROUTER
  uip_create_linklocal_allrouters_mcast(&loc_fipaddr);
  uip_ds6_maddr_add(&loc_fipaddr);
#if UIP_ND6_SEND_RA
  l3_stimer_set(&uip_ds6_timer_ra, 2);     /* wait to have a link local IP address */
#endif /* UIP_ND6_SEND_RA */
#else /* UIP_CONF_ROUTER */
  l3_etimer_set(&uip_ds6_timer_rs,
             l3_random_rand() % (UIP_ND6_MAX_RTR_SOLICITATION_DELAY *
                              CLOCK_SECOND));
#endif /* UIP_CONF_ROUTER */
  l3_etimer_set(&uip_ds6_timer_periodic, UIP_DS6_PERIOD);

  return;
}


/*---------------------------------------------------------------------------*/
void
uip_ds6_periodic(void)
{

  /* Periodic processing on unicast addresses */
  for(locaddr = uip_ds6_if.addr_list;
      locaddr < uip_ds6_if.addr_list + UIP_DS6_ADDR_NB; locaddr++) {
    if(locaddr->isused) {
      if((!locaddr->isinfinite) && (l3_stimer_expired(&locaddr->vlifetime))) {
        uip_ds6_addr_rm(locaddr);
#if UIP_ND6_DEF_MAXDADNS > 0
      } else if((locaddr->state == ADDR_TENTATIVE)
                && (locaddr->dadnscount <= uip_ds6_if.maxdadns)
                && (l3_timer_expired(&locaddr->dadtimer))
                && (uip_len == 0)) {
        uip_ds6_dad(locaddr);
#endif /* UIP_ND6_DEF_MAXDADNS > 0 */
      }
    }
  }

  /* Periodic processing on default routers */
  //uip_ds6_defrt_periodic();//removed beacause def-route is only added after parent switching
  /*  for(locdefrt = uip_ds6_defrt_list;
      locdefrt < uip_ds6_defrt_list + UIP_DS6_DEFRT_NB; locdefrt++) {
    if((locdefrt->isused) && (!locdefrt->isinfinite) &&
       (l3_stimer_expired(&(locdefrt->lifetime)))) {
      uip_ds6_defrt_rm(locdefrt);
    }
    }*/

#if !UIP_CONF_ROUTER
  /* Periodic processing on prefixes */
  for(locprefix = uip_ds6_prefix_list;
      locprefix < uip_ds6_prefix_list + UIP_DS6_PREFIX_NB;
      locprefix++) {
    if(locprefix->isused && !locprefix->isinfinite
       && l3_stimer_expired(&(locprefix->vlifetime))) {
      uip_ds6_prefix_rm(locprefix);
    }
  }
#endif /* !UIP_CONF_ROUTER */

#if UIP_ND6_SEND_NA
  uip_ds6_neighbor_periodic();
#endif /* UIP_ND6_SEND_RA */

#if UIP_CONF_ROUTER && UIP_ND6_SEND_RA
  /* Periodic RA sending */
  if(l3_stimer_expired(&uip_ds6_timer_ra) && (uip_len == 0)) {
    uip_ds6_send_ra_periodic();
  }
#endif /* UIP_CONF_ROUTER && UIP_ND6_SEND_RA */
  l3_etimer_reset(&uip_ds6_timer_periodic);
  return;
}

/*---------------------------------------------------------------------------*/
uint8_t
uip_ds6_list_loop(uip_ds6_element_t *list, uint8_t size,
                  uint16_t elementsize, uip_ipaddr_t *ipaddr,
                  uint8_t ipaddrlen, uip_ds6_element_t **out_element)
{
  uip_ds6_element_t *element;

  *out_element = NULL;

  for(element = list;
      element <
      (uip_ds6_element_t *)((uint8_t *)list + (size * elementsize));
      element = (uip_ds6_element_t *)((uint8_t *)element + elementsize)) {
    if(element->isused) {
      if(uip_ipaddr_prefixcmp(&element->ipaddr, ipaddr, ipaddrlen)) {
        *out_element = element;
        return FOUND;
      }
    } else {
      *out_element = element;
    }
  }

  return *out_element != NULL ? FREESPACE : NOSPACE;
}

/*---------------------------------------------------------------------------*/
#if UIP_CONF_ROUTER
/*---------------------------------------------------------------------------*/
uip_ds6_prefix_t *
uip_ds6_prefix_add(uip_ipaddr_t *ipaddr, uint8_t ipaddrlen,
                   uint8_t advertise, uint8_t flags, unsigned long vtime,
                   unsigned long ptime)
{
  if(uip_ds6_list_loop
     ((uip_ds6_element_t *)uip_ds6_prefix_list, UIP_DS6_PREFIX_NB,
      sizeof(uip_ds6_prefix_t), ipaddr, ipaddrlen,
      (uip_ds6_element_t **)&locprefix) == FREESPACE) {
        locprefix->isused = 1; //1; // Raka :: we are not using prefix option of RPL for address management
    uip_ipaddr_copy(&locprefix->ipaddr, ipaddr);
    locprefix->length = ipaddrlen;
    locprefix->advertise = advertise;
    locprefix->l_a_reserved = flags;
    locprefix->vlifetime = vtime;
    locprefix->plifetime = ptime;
    PRINTF("Adding prefix ");
    PRINT6ADDR(&locprefix->ipaddr);
    PRINTF("length %u, flags %x, Valid lifetime %lx, Preffered lifetime %lx\n",
       ipaddrlen, flags, vtime, ptime);
    return locprefix;
  } else {
    PRINTF("No more space in Prefix list\n");
  }
  return NULL;
}


#else /* UIP_CONF_ROUTER */
uip_ds6_prefix_t *
uip_ds6_prefix_add(uip_ipaddr_t *ipaddr, uint8_t ipaddrlen,
                   unsigned long interval)
{
  if(uip_ds6_list_loop
     ((uip_ds6_element_t *)uip_ds6_prefix_list, UIP_DS6_PREFIX_NB,
      sizeof(uip_ds6_prefix_t), ipaddr, ipaddrlen,
      (uip_ds6_element_t **)&locprefix) == FREESPACE) {
    locprefix->isused = 1; // 1;  // Raka :: we are not using prefix option of RPL for address management 
    uip_ipaddr_copy(&locprefix->ipaddr, ipaddr);
    locprefix->length = ipaddrlen;
    if(interval != 0) {
      l3_stimer_set(&(locprefix->vlifetime), interval);
      locprefix->isinfinite = 0;
    } else {
      locprefix->isinfinite = 1;
    }
    PRINTF("Adding prefix ");
    PRINT6ADDR(&locprefix->ipaddr);
    PRINTF("length %u, vlifetime %lu\n", ipaddrlen, interval);
    return locprefix;
  }
  return NULL;
}
#endif /* UIP_CONF_ROUTER */

/*---------------------------------------------------------------------------*/
void
uip_ds6_prefix_rm(uip_ds6_prefix_t *prefix)
{
  if(prefix != NULL) {
    prefix->isused = 0;
  }
  return;
}
/*---------------------------------------------------------------------------*/
uip_ds6_prefix_t *
uip_ds6_prefix_lookup(uip_ipaddr_t *ipaddr, uint8_t ipaddrlen)
{
  if(uip_ds6_list_loop((uip_ds6_element_t *)uip_ds6_prefix_list,
                       UIP_DS6_PREFIX_NB, sizeof(uip_ds6_prefix_t),
                       ipaddr, ipaddrlen,
                       (uip_ds6_element_t **)&locprefix) == FOUND) {
    return locprefix;
  }
  return NULL;
}

/*---------------------------------------------------------------------------*/
uint8_t
uip_ds6_is_addr_onlink(uip_ipaddr_t *ipaddr)
{
  if(uip_is_addr_linklocal(ipaddr)) {
    for(locprefix = uip_ds6_prefix_list;
        locprefix < uip_ds6_prefix_list + UIP_DS6_PREFIX_NB; locprefix++) {
      if(locprefix->isused &&
         uip_ipaddr_prefixcmp(&locprefix->ipaddr, ipaddr, locprefix->length)) {
        return 1;
      }
    }
  }
  return 0;
}

/*---------------------------------------------------------------------------*/

// Raka :: to check if the device is registered or not 

uint8_t
uip_ds6_is_addr_directlink(uip_ipaddr_t *ipaddr)
{ 
  uip_lladdr_t lladdr;
  const uip_lladdr_t *routerlladdr;
  int i, j;
  /*Suneet :: added for when router flag is set where use full device global address */
  for(i = 0; i < RPL_MAX_INSTANCES; ++i) {
    if(instance_table[i].used) {
      for(j = 0; j < RPL_MAX_DAG_PER_INSTANCE; ++j) {
           if(instance_table[i].dag_table[j].prefix_info.flags & UIP_ND6_NA_FLAG_OVERRIDE)
           {
              routerlladdr =  uip_ds6_nbr_lladdr_from_ipaddr(ipaddr);
              uip_ds6_nbr_t *l_nbr = uip_ds6_nbr_ll_lookup(routerlladdr);  
              if(l_nbr != NULL){
                return l_nbr->d_link_status;//direct link status.
              }
           }
           else if(instance_table[i].dag_table[j].prefix_info.flags & UIP_ND6_RA_FLAG_AUTONOMOUS)
           {
              if(ipaddr != NULL){
              memcpy(&lladdr.addr, &ipaddr->u8[8], 8);
              lladdr.addr[0] ^= 0x02;
              uip_ds6_nbr_t *l_nbr = uip_ds6_nbr_ll_lookup(&lladdr);  
              if(l_nbr != NULL){
                return l_nbr->d_link_status;//direct link status.
              }
            }
           }
      }
    }
  }
        return 0;
}

/*---------------------------------------------------------------------------*/
uip_ds6_addr_t *
uip_ds6_addr_add(uip_ipaddr_t *ipaddr, unsigned long vlifetime, uint8_t type)
{
  if(uip_ds6_list_loop
     ((uip_ds6_element_t *)uip_ds6_if.addr_list, UIP_DS6_ADDR_NB,
      sizeof(uip_ds6_addr_t), ipaddr, 128,
      (uip_ds6_element_t **)&locaddr) == FREESPACE) {
    locaddr->isused = 1; //1; // Raka :: we are not using prefix option of RPL for address management
    uip_ipaddr_copy(&locaddr->ipaddr, ipaddr);
    locaddr->type = type;
    if(vlifetime == 0) {
      locaddr->isinfinite = 1;
    } else {
      locaddr->isinfinite = 0;
      l3_stimer_set(&(locaddr->vlifetime), vlifetime);
    }
#if UIP_ND6_DEF_MAXDADNS > 0
    locaddr->state = ADDR_TENTATIVE;
    l3_timer_set(&locaddr->dadtimer,
              l3_random_rand() % (UIP_ND6_MAX_RTR_SOLICITATION_DELAY *
                               CLOCK_SECOND));
    locaddr->dadnscount = 0;
#else /* UIP_ND6_DEF_MAXDADNS > 0 */
    locaddr->state = ADDR_PREFERRED;
#endif /* UIP_ND6_DEF_MAXDADNS > 0 */
    uip_create_solicited_node(ipaddr, &loc_fipaddr);
    uip_ds6_maddr_add(&loc_fipaddr);
    return locaddr;
  }
  return NULL;
}

/*---------------------------------------------------------------------------*/
void
uip_ds6_addr_rm(uip_ds6_addr_t *addr)
{
  if(addr != NULL) {
    uip_create_solicited_node(&addr->ipaddr, &loc_fipaddr);
    if((locmaddr = uip_ds6_maddr_lookup(&loc_fipaddr)) != NULL) {
      uip_ds6_maddr_rm(locmaddr);
    }
    addr->isused = 0;
  }
  return;
}

/*---------------------------------------------------------------------------*/
uip_ds6_addr_t *
uip_ds6_addr_lookup(uip_ipaddr_t *ipaddr)
{
  if(uip_ds6_list_loop
     ((uip_ds6_element_t *)uip_ds6_if.addr_list, UIP_DS6_ADDR_NB,
      sizeof(uip_ds6_addr_t), ipaddr, 128,
      (uip_ds6_element_t **)&locaddr) == FOUND) {
    return locaddr;
  }
  return NULL;
}

void uip_ds6_address_remove (uip_ipaddr_t *ipaddr)
{
  uip_ds6_addr_t *address = uip_ds6_addr_lookup (ipaddr);
  uip_ds6_addr_rm (address);
}

/*---------------------------------------------------------------------------*/
/*
 * get a link local address -
 * state = -1 => any address is ok. Otherwise state = desired state of addr.
 * (TENTATIVE, PREFERRED, DEPRECATED)
 */
uip_ds6_addr_t *
uip_ds6_get_link_local(int8_t state)
{
  for(locaddr = uip_ds6_if.addr_list;
      locaddr < uip_ds6_if.addr_list + UIP_DS6_ADDR_NB; locaddr++) {
    if(locaddr->isused && (state == -1 || locaddr->state == state)
       && (uip_is_addr_linklocal(&locaddr->ipaddr))) {
      return locaddr;
    }
  }
  return NULL;
}

/*---------------------------------------------------------------------------*/
/*
 * get a global address -
 * state = -1 => any address is ok. Otherwise state = desired state of addr.
 * (TENTATIVE, PREFERRED, DEPRECATED)
 */
uip_ds6_addr_t *
uip_ds6_get_global(int8_t state)
{
  for(locaddr = uip_ds6_if.addr_list;
      locaddr < uip_ds6_if.addr_list + UIP_DS6_ADDR_NB; locaddr++) {
    if(locaddr->isused && (state == -1 || locaddr->state == state)
       && !(uip_is_addr_linklocal(&locaddr->ipaddr))) {
      return locaddr;
    }
  }
  return NULL;
}

/*---------------------------------------------------------------------------*/
uip_ds6_maddr_t *
uip_ds6_maddr_add(const uip_ipaddr_t *ipaddr)
{
  if(uip_ds6_list_loop
     ((uip_ds6_element_t *)uip_ds6_if.maddr_list, UIP_DS6_MADDR_NB,
      sizeof(uip_ds6_maddr_t), (void*)ipaddr, 128,
      (uip_ds6_element_t **)&locmaddr) == FREESPACE) {
    locmaddr->isused = 1; //1; // Raka :: we are not using prefix option of RPL for address management
    uip_ipaddr_copy(&locmaddr->ipaddr, ipaddr);
    return locmaddr;
  }
  return NULL;
}

/*---------------------------------------------------------------------------*/
void
uip_ds6_maddr_rm(uip_ds6_maddr_t *maddr)
{
  if(maddr != NULL) {
    maddr->isused = 0;
  }
  return;
}

/*---------------------------------------------------------------------------*/
uip_ds6_maddr_t *
uip_ds6_maddr_lookup(const uip_ipaddr_t *ipaddr)
{
  if(uip_ds6_list_loop
     ((uip_ds6_element_t *)uip_ds6_if.maddr_list, UIP_DS6_MADDR_NB,
      sizeof(uip_ds6_maddr_t), (void*)ipaddr, 128,
      (uip_ds6_element_t **)&locmaddr) == FOUND) {
    return locmaddr;
  }
  return NULL;
}
/*---------------------------------------------------------------------------*/
uip_ds6_aaddr_t *
uip_ds6_aaddr_add(uip_ipaddr_t *ipaddr)
{
#if UIP_DS6_AADDR_NB
  if(uip_ds6_list_loop
     ((uip_ds6_element_t *)uip_ds6_if.aaddr_list, UIP_DS6_AADDR_NB,
      sizeof(uip_ds6_aaddr_t), ipaddr, 128,
      (uip_ds6_element_t **)&locaaddr) == FREESPACE) {
    locaaddr->isused = 1; //1; // Raka :: we are not using prefix option of RPL for address management
    uip_ipaddr_copy(&locaaddr->ipaddr, ipaddr);
    return locaaddr;
  }
#endif /* UIP_DS6_AADDR_NB */
  return NULL;
}

/*---------------------------------------------------------------------------*/
void
uip_ds6_aaddr_rm(uip_ds6_aaddr_t *aaddr)
{
  if(aaddr != NULL) {
    aaddr->isused = 0;
  }
  return;
}

/*---------------------------------------------------------------------------*/
uip_ds6_aaddr_t *
uip_ds6_aaddr_lookup(uip_ipaddr_t *ipaddr)
{
#if UIP_DS6_AADDR_NB
  if(uip_ds6_list_loop((uip_ds6_element_t *)uip_ds6_if.aaddr_list,
                       UIP_DS6_AADDR_NB, sizeof(uip_ds6_aaddr_t), ipaddr, 128,
                       (uip_ds6_element_t **)&locaaddr) == FOUND) {
    return locaaddr;
  }
#endif /* UIP_DS6_AADDR_NB */
  return NULL;
}

/*---------------------------------------------------------------------------*/
extern uint8_t dhcp_complete_flag;
extern uint8_t rpl_pkt_type;
extern uint8_t rpl_pkt_code;
extern uint8_t get_dev_mac_join5_status(void);
void
uip_ds6_select_src(uip_ipaddr_t *src, uip_ipaddr_t *dst)
{
  uint8_t best = 0;             /* number of bit in common with best match */
  uint8_t n = 0;
  uip_ds6_addr_t *matchaddr = NULL;

  if(uip_is_addr_linklocal(dst) && !uip_is_addr_mcast(dst)) {//santosh !
    /* find longest match */
    for(locaddr = uip_ds6_if.addr_list;
        locaddr < uip_ds6_if.addr_list + UIP_DS6_ADDR_NB; locaddr++) {
      /* Only preferred global (not link-local) addresses */
      if(locaddr->isused && locaddr->state == ADDR_PREFERRED &&
         uip_is_addr_linklocal(&locaddr->ipaddr)) //Suneet change this !uip_is_addr_linklocal(&locaddr->ipaddr) remove becasue all are happen on link local
      {
        n = get_match_length(dst, &locaddr->ipaddr);
        if(n >= best) {
          best = n;
          matchaddr = locaddr;
        }
      }
    }
#if UIP_IPV6_MULTICAST
  } else if(uip_is_addr_mcast_routable(dst)) {
    matchaddr = uip_ds6_get_global(ADDR_PREFERRED);
#endif
  }
  else 
  {
    /*
    uint8_t join_state = get_dev_mac_join5_status();
    if(join_state || dhcp_complete_flag)//join state 5
    {
      matchaddr = uip_ds6_get_global(ADDR_PREFERRED);
//      matchaddr = (link_local_flag == 1) ? uip_ds6_get_link_local(ADDR_PREFERRED):
//                  uip_ds6_get_global(ADDR_PREFERRED);//self global or link local address 
//      link_local_flag = 0;//for sending explicitly with link local address//ONLY FOR TESTING
    }
    else
    {
      matchaddr = uip_ds6_get_link_local(ADDR_PREFERRED);//self or source link local address
    }*/
    if(rpl_pkt_type == ICMP6_RPL)
    {
      if((rpl_pkt_code == RPL_CODE_DIO) || (rpl_pkt_code == RPL_CODE_DIS))
      {
        matchaddr = uip_ds6_get_link_local(ADDR_PREFERRED);//self or source link local address
        rpl_pkt_code = 0xFF;//Suneet ::
        rpl_pkt_type = 0xFF;
      }
      else if((rpl_pkt_code == RPL_CODE_DAO) || (rpl_pkt_code == RPL_CODE_DAO_ACK))
      {
        matchaddr = uip_ds6_get_global(ADDR_PREFERRED);
        rpl_pkt_code = 0xFF;//Suneet ::
        rpl_pkt_type = 0xFF;
      }
      
    }
    else if((rpl_pkt_type == ICMP6_ECHO_REQUEST) || (rpl_pkt_type == ICMP6_ECHO_REPLY))
    {
      matchaddr = uip_ds6_get_global(ADDR_PREFERRED);
      rpl_pkt_code = 0xFF;//Suneet ::
      rpl_pkt_type = 0xFF;
    }
    else
    {
      matchaddr = uip_ds6_get_global(ADDR_PREFERRED);
    }
    
  }

#if (UIP_MCAST6_CONF_ENGINE == UIP_MCAST6_ENGINE_ROLL_TM)//santosh, multicast
  if(uip_is_addr_mpl_mcast_forwarders(dst)) {
    matchaddr = uip_ds6_get_global(ADDR_PREFERRED);
  }
#endif
    
  /* use the :: (unspecified address) as source if no match found */
  if(matchaddr == NULL) {
    uip_create_unspecified(src);
  } else {
    uip_ipaddr_copy(src, &matchaddr->ipaddr);
  }
}

/*---------------------------------------------------------------------------*/
void
uip_ds6_set_addr_iid(uip_ipaddr_t *ipaddr, uip_lladdr_t *lladdr)
{
  /* We consider only links with IEEE EUI-64 identifier or
   * IEEE 48-bit MAC addresses */
#if (UIP_LLADDR_LEN == 8)
  memcpy(ipaddr->u8 + 8, lladdr, UIP_LLADDR_LEN);
  ipaddr->u8[8] ^= 0x02;
#elif (UIP_LLADDR_LEN == 6)
  memcpy(ipaddr->u8 + 8, lladdr, 3);
  ipaddr->u8[11] = 0xff;
  ipaddr->u8[12] = 0xfe;
  memcpy(ipaddr->u8 + 13, (uint8_t *)lladdr + 3, 3);
  ipaddr->u8[8] ^= 0x02;
#else
#error uip-ds6.c cannot build interface address when UIP_LLADDR_LEN is not 6 or 8
#endif
}
/*---------------------------------------------------------------------------*/
void
uip_ds6_get_addr_iid(uip_ipaddr_t *ipaddr, uip_lladdr_t *lladdr)
{
  /* We consider only links with IEEE EUI-64 identifier or
  * IEEE 48-bit MAC addresses */
#if (UIP_LLADDR_LEN == 8)
  memcpy(lladdr,ipaddr->u8 + 8, UIP_LLADDR_LEN);
  lladdr->addr[0] ^= 0x02;
#elif (UIP_LLADDR_LEN == 6)
  memcpy(lladdr ,ipaddr->u8 + 8 , 3);
  ipaddr->u8[3] = 0xff;
  ipaddr->u8[4] = 0xfe;
  memcpy((uint8_t *)lladdr + 3,ipaddr->u8 + 13, 3);
  ipaddr->u8[0] ^= 0x02;
#else
#error uip-ds6.c cannot build interface address when UIP_LLADDR_LEN is not 6 or 8
#endif
}
/*---------------------------------------------------------------------------*/
uint8_t
get_match_length(uip_ipaddr_t *src, uip_ipaddr_t *dst)
{
  uint8_t j, k, x_or;
  uint8_t len = 0;

  for(j = 0; j < 16; j++) {
    if(src->u8[j] == dst->u8[j]) {
      len += 8;
    } else {
      x_or = src->u8[j] ^ dst->u8[j];
      for(k = 0; k < 8; k++) {
        if((x_or & 0x80) == 0) {
          len++;
          x_or <<= 1;
        } else {
          break;
        }
      }
      break;
    }
  }
  return len;
}

/*---------------------------------------------------------------------------*/
#if UIP_ND6_DEF_MAXDADNS > 0
void
uip_ds6_dad(uip_ds6_addr_t *addr)
{
  /* send maxdadns NS for DAD  */
  if(addr->dadnscount < uip_ds6_if.maxdadns) {
    uip_nd6_ns_output(NULL, NULL, &addr->ipaddr);
    addr->dadnscount++;
    l3_timer_set(&addr->dadtimer,
              uip_ds6_if.retrans_timer / 1000 * CLOCK_SECOND);
    return;
  }
  /*
   * If we arrive here it means DAD succeeded, otherwise the dad process
   * would have been interrupted in ds6_dad_ns/na_input
   */
  PRINTF("DAD succeeded, ipaddr: ");
  PRINT6ADDR(&addr->ipaddr);
  PRINTF("\n");

  addr->state = ADDR_PREFERRED;
  return;
}

/*---------------------------------------------------------------------------*/
/*
 * Calling code must handle when this returns 0 (e.g. link local
 * address can not be used).
 */
int
uip_ds6_dad_failed(uip_ds6_addr_t *addr)
{
  if(uip_is_addr_linklocal(&addr->ipaddr)) {
    PRINTF("Contiki shutdown, DAD for link local address failed\n");
    return 0;
  }
  uip_ds6_addr_rm(addr);
  return 1;
}
#endif /*UIP_ND6_DEF_MAXDADNS > 0 */

/*---------------------------------------------------------------------------*/
#if UIP_CONF_ROUTER
#if UIP_ND6_SEND_RA
void
uip_ds6_send_ra_sollicited(void)
{
  /* We have a pb here: RA timer max possible value is 1800s,
   * hence we have to use stimers. However, when receiving a RS, we
   * should delay the reply by a random value between 0 and 500ms timers.
   * stimers are in seconds, hence we cannot do this. Therefore we just send
   * the RA (setting the timer to 0 below). We keep the code logic for
   * the days contiki will support appropriate timers */
  rand_time = 0;
  PRINTF("Solicited RA, random time %u\n", rand_time);

  if(l3_stimer_remaining(&uip_ds6_timer_ra) > rand_time) {
    if(l3_stimer_elapsed(&uip_ds6_timer_ra) < UIP_ND6_MIN_DELAY_BETWEEN_RAS) {
      /* Ensure that the RAs are rate limited */
/*      l3_stimer_set(&uip_ds6_timer_ra, rand_time +
                 UIP_ND6_MIN_DELAY_BETWEEN_RAS -
                 l3_stimer_elapsed(&uip_ds6_timer_ra));
  */ } else {
      l3_stimer_set(&uip_ds6_timer_ra, rand_time);
    }
  }
}

/*---------------------------------------------------------------------------*/
void
uip_ds6_send_ra_periodic(void)
{
  if(racount > 0) {
    /* send previously scheduled RA */
    uip_nd6_ra_output(NULL);
    PRINTF("Sending periodic RA\n");
  }

  rand_time = UIP_ND6_MIN_RA_INTERVAL + l3_random_rand() %
    (uint16_t) (UIP_ND6_MAX_RA_INTERVAL - UIP_ND6_MIN_RA_INTERVAL);
  PRINTF("Random time 1 = %u\n", rand_time);

  if(racount < UIP_ND6_MAX_INITIAL_RAS) {
    if(rand_time > UIP_ND6_MAX_INITIAL_RA_INTERVAL) {
      rand_time = UIP_ND6_MAX_INITIAL_RA_INTERVAL;
      PRINTF("Random time 2 = %u\n", rand_time);
    }
    racount++;
  }
  PRINTF("Random time 3 = %u\n", rand_time);
  l3_stimer_set(&uip_ds6_timer_ra, rand_time);
}

#endif /* UIP_ND6_SEND_RA */
#else /* UIP_CONF_ROUTER */
/*---------------------------------------------------------------------------*/
void
uip_ds6_send_rs(void)
{
  if((uip_ds6_defrt_choose() == NULL)
     && (rscount < UIP_ND6_MAX_RTR_SOLICITATIONS)) {
    PRINTF("Sending RS %u\n", rscount);
    uip_nd6_rs_output();
    rscount++;
    l3_etimer_set(&uip_ds6_timer_rs,
               UIP_ND6_RTR_SOLICITATION_INTERVAL * CLOCK_SECOND);
  } else {
    PRINTF("Router found ? (boolean): %u\n",
           (uip_ds6_defrt_choose() != NULL));
    l3_etimer_stop(&uip_ds6_timer_rs);
  }
  return;
}

#endif /* UIP_CONF_ROUTER */
/*---------------------------------------------------------------------------*/
uint32_t
uip_ds6_compute_reachable_time(void)
{
  return (uint32_t) (UIP_ND6_MIN_RANDOM_FACTOR
                     (uip_ds6_if.base_reachable_time)) +
    ((uint16_t) (l3_random_rand() << 8) +
     (uint16_t) l3_random_rand()) %
    (uint32_t) (UIP_ND6_MAX_RANDOM_FACTOR(uip_ds6_if.base_reachable_time) -
                UIP_ND6_MIN_RANDOM_FACTOR(uip_ds6_if.base_reachable_time));
}
/*---------------------------------------------------------------------------*/
uip_ds6_maddr_t *get_mcast_addr()
{
    uip_ds6_maddr_t *locmpl_addr;
    locmpl_addr = (uip_ds6_maddr_t *)&uip_ds6_if.maddr_list[3];
    return locmpl_addr;
}
/** @}*/
