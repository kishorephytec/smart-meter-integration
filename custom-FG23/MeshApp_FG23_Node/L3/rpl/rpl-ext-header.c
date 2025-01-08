/*
 * Copyright (c) 2009, Swedish Institute of Computer Science.
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
 */

/**
 * \file
 *         Management of extension headers for ContikiRPL.
 *
 * \author Vincent Brillault <vincent.brillault@imag.fr>,
 *         Joakim Eriksson <joakime@sics.se>,
 *         Niclas Finne <nfi@sics.se>,
 *         Nicolas Tsiftes <nvt@sics.se>.
 */

/**
 * \addtogroup uip6
 * @{
 */
#include "Stack6LoWPANConf.h"
#include "l3_configuration.h"
#include "l3_timer_utility.h"
#include "contiki-net.h"

#define DEBUG DEBUG_NONE
#include "uip-debug.h"

#include <limits.h>
#include <string.h>

extern uip_ds6_nbr_t *uip_get_prio_based_parent_nbr(uint8_t prio);
/*---------------------------------------------------------------------------*/
#define UIP_IP_BUF                ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])
#define UIP_EXT_BUF               ((struct uip_ext_hdr *)&uip_buf[uip_l2_l3_hdr_len])
#define UIP_HBHO_BUF              ((struct uip_hbho_hdr *)&uip_buf[uip_l2_l3_hdr_len])
#define UIP_HBHO_NEXT_BUF         ((struct uip_ext_hdr *)&uip_buf[uip_l2_l3_hdr_len + RPL_HOP_BY_HOP_LEN])
#define UIP_RH_BUF                ((struct uip_routing_hdr *)&uip_buf[uip_l2_l3_hdr_len])
#define UIP_RPL_SRH_BUF           ((struct uip_rpl_srh_hdr *)&uip_buf[uip_l2_l3_hdr_len + RPL_RH_LEN])
#define UIP_EXT_HDR_OPT_BUF       ((struct uip_ext_hdr_opt *)&uip_buf[uip_l2_l3_hdr_len + uip_ext_opt_offset])
#define UIP_EXT_HDR_OPT_PADN_BUF  ((struct uip_ext_hdr_opt_padn *)&uip_buf[uip_l2_l3_hdr_len + uip_ext_opt_offset])
#define UIP_EXT_HDR_OPT_RPL_BUF   ((struct uip_ext_hdr_opt_rpl *)&uip_buf[uip_l2_l3_hdr_len + uip_ext_opt_offset]) //40 + 
/*---------------------------------------------------------------------------*/
int
rpl_verify_hbh_header(int uip_ext_opt_offset)
{
  rpl_instance_t *instance;
  int down;
  uint16_t sender_rank;
  uint8_t sender_closer;
  uip_ds6_route_t *route;
  rpl_parent_t *sender = NULL;

  if(UIP_HBHO_BUF->len != ((RPL_HOP_BY_HOP_LEN - 8) / 8)
      || UIP_EXT_HDR_OPT_RPL_BUF->opt_type != UIP_EXT_HDR_OPT_RPL
      || UIP_EXT_HDR_OPT_RPL_BUF->opt_len != RPL_HDR_OPT_LEN) {

    PRINTF("RPL: Hop-by-hop extension header has wrong size or type (%u %u %u)\n",
        UIP_HBHO_BUF->len,
        UIP_EXT_HDR_OPT_RPL_BUF->opt_type,
        UIP_EXT_HDR_OPT_RPL_BUF->opt_len);
    return 0; /* Drop */
  }

  instance = rpl_get_instance(UIP_EXT_HDR_OPT_RPL_BUF->instance);
  if(instance == NULL) {
    PRINTF("RPL: Unknown instance: %u\n",
           UIP_EXT_HDR_OPT_RPL_BUF->instance);
    return 0;
  }

  if(UIP_EXT_HDR_OPT_RPL_BUF->flags & RPL_HDR_OPT_FWD_ERR) {
    PRINTF("RPL: Forward error!\n");
    /* We should try to repair it by removing the neighbor that caused
         the packet to be forwareded in the first place. We drop any
         routes that go through the neighbor that sent the packet to
         us. */
    if(RPL_IS_STORING(instance)) {
      route = uip_ds6_route_lookup(&UIP_IP_BUF->destipaddr);
      if(route != NULL) {
        uip_ds6_route_rm(route);
      }
    }
    RPL_STAT(rpl_stats.forward_errors++);
    /* Trigger DAO retransmission */
    //rpl_reset_dio_timer(instance);//santosh
    /* drop the packet as it is not routable */
    return 0;
  }

  if(!instance->current_dag->joined) {
    PRINTF("RPL: No DAG in the instance\n");
    return 0;
  }
  down = 0;
  if(UIP_EXT_HDR_OPT_RPL_BUF->flags & RPL_HDR_OPT_DOWN) {
    down = 1;
  }

  sender_rank = UIP_HTONS(UIP_EXT_HDR_OPT_RPL_BUF->senderrank);
  sender = nbr_table_get_from_lladdr(rpl_parents, packetbuf_addr(PACKETBUF_ADDR_SENDER));

  if(sender != NULL && (UIP_EXT_HDR_OPT_RPL_BUF->flags & RPL_HDR_OPT_RANK_ERR)) {
    /* A rank error was signalled, attempt to repair it by updating
     * the sender's rank from ext header */
    sender->rank = sender_rank;
    if(RPL_IS_NON_STORING(instance)) {
      /* Select DAG and preferred parent only in non-storing mode. In storing mode,
       * a parent switch would result in an immediate No-path DAO transmission, dropping
       * current incoming packet. */
      rpl_select_dag(instance, sender);
    }
  }

  sender_closer = sender_rank < instance->current_dag->rank;
  
  if ( sender_rank == 0)
  {
    sender_closer = 0;
  }
  
  PRINTF("RPL: Packet going %s, sender closer %d (%d < %d)\n", down == 1 ? "down" : "up",
   sender_closer,
   sender_rank,
   instance->current_dag->rank
   );

  if((down && !sender_closer) || (!down && sender_closer)) {
    PRINTF("RPL: Loop detected - senderrank: %d my-rank: %d sender_closer: %d\n",
           sender_rank, instance->current_dag->rank,
           sender_closer);
    /* Attempt to repair the loop by sending a unicast DIO back to the sender
     * so that it gets a fresh update of our rank. */
    if(sender != NULL) {
      instance->unicast_dio_target = sender;
      rpl_schedule_unicast_dio_immediately(instance);
    }
    if(UIP_EXT_HDR_OPT_RPL_BUF->flags & RPL_HDR_OPT_RANK_ERR) {
      RPL_STAT(rpl_stats.loop_errors++);
      PRINTF("RPL: Rank error signalled in RPL option!\n");
      /* Packet must be dropped and dio trickle timer reset, see RFC6550 - 11.2.2.2 */
      //rpl_reset_dio_timer(instance);//santosh
      return 0;
    }
    PRINTF("RPL: Single error tolerated\n");
    RPL_STAT(rpl_stats.loop_warnings++);
    UIP_EXT_HDR_OPT_RPL_BUF->flags |= RPL_HDR_OPT_RANK_ERR;
    return 1;
  }

  PRINTF("RPL: Rank OK\n");
  return 1;
}
/*---------------------------------------------------------------------------*/
#if RPL_WITH_NON_STORING
int
rpl_srh_get_next_hop(uip_ipaddr_t *ipaddr)
{
  uint8_t *uip_next_hdr;
  int last_uip_ext_len = uip_ext_len;
  rpl_dag_t *dag;
  rpl_ns_node_t *dest_node;
  rpl_ns_node_t *root_node;

  uip_ext_len = 0;
  uip_next_hdr = &UIP_IP_BUF->proto;

  /* Look for routing header */
  while(uip_next_hdr != NULL && *uip_next_hdr != UIP_PROTO_ROUTING) {
    switch(*uip_next_hdr) {
      case UIP_PROTO_HBHO:
      case UIP_PROTO_DESTO:
        /*
         * As per RFC 2460, only the Hop-by-Hop Options header and
         * Destination Options header can appear before the Routing
         * header.
         */
        /* Move to next header */
        uip_next_hdr = &UIP_EXT_BUF->next;
        uip_ext_len += (UIP_EXT_BUF->len << 3) + 8;
        break;
      default:
        uip_next_hdr = NULL;
        break;
    }
  }

  dag = rpl_get_dag(&UIP_IP_BUF->destipaddr);
  root_node = rpl_ns_get_node(dag, &dag->dag_id);//looks into nodelist, it is updated, while we receive dao
  dest_node = rpl_ns_get_node(dag, &UIP_IP_BUF->destipaddr);//looks into nodelist, it is updated, while we receive dao

  if((uip_next_hdr != NULL && *uip_next_hdr == UIP_PROTO_ROUTING
      && UIP_RH_BUF->routing_type == RPL_RH_TYPE_SRH) ||
     (dest_node != NULL && root_node != NULL &&
      dest_node->parent == root_node)) {
    /* Routing header found or the packet destined for a direct child of the root.
     * The next hop should be already copied as the IPv6 destination
     * address, via rpl_process_srh_header. We turn this address into a link-local to enable
     * forwarding to next hop */
    uip_ipaddr_copy(ipaddr, &UIP_IP_BUF->destipaddr);
    //uip_create_linklocal_prefix(ipaddr); //Suneet :: why ?????do this 26/12/2017 
    uip_ext_len = last_uip_ext_len;
    return 1;   //for adding the srh and ipv6-to-ipv6 tunneling
  }

  uip_ext_len = last_uip_ext_len;
  return 0;//no srh header found, so no tunneling
}
/*---------------------------------------------------------------------------*/
int
rpl_process_srh_header(void)
{
  uint8_t *uip_next_hdr;
  int last_uip_ext_len = uip_ext_len;

  uip_ext_len = 0;
  uip_next_hdr = &UIP_IP_BUF->proto;

  /* Look for routing header */
  while(uip_next_hdr != NULL && *uip_next_hdr != UIP_PROTO_ROUTING) {
    switch(*uip_next_hdr) {
      case UIP_PROTO_HBHO:
      case UIP_PROTO_DESTO:
        /*
         * As per RFC 2460, only the Hop-by-Hop Options header and
         * Destination Options header can appear before the Routing
         * header.
         */
        /* Move to next header */
        uip_next_hdr = &UIP_EXT_BUF->next;
        uip_ext_len += (UIP_EXT_BUF->len << 3) + 8;
        break;
      default:
        uip_next_hdr = NULL;
        break;
    }
  }

  if(uip_next_hdr != NULL && *uip_next_hdr == UIP_PROTO_ROUTING
      && UIP_RH_BUF->routing_type == RPL_RH_TYPE_SRH) {
    /* SRH found, now look for next hop */
    uint8_t cmpri, cmpre;
    uint8_t ext_len;
    uint8_t padding;
    uint8_t path_len;
    uint8_t segments_left;
    uip_ipaddr_t current_dest_addr;

    segments_left = UIP_RH_BUF->seg_left;
    ext_len = (UIP_RH_BUF->len * 8) + 8;
    cmpri = UIP_RPL_SRH_BUF->cmpr >> 4;
    cmpre = UIP_RPL_SRH_BUF->cmpr & 0x0f;
    padding = UIP_RPL_SRH_BUF->pad >> 4;
    path_len = ((ext_len - padding - RPL_RH_LEN - RPL_SRH_LEN - (16 - cmpre)) / (16 - cmpri)) + 1;
    (void)path_len;

    PRINTF("RPL: read SRH, path len %u, segments left %u, Cmpri %u, Cmpre %u, ext len %u (padding %u)\n",
        path_len, segments_left, cmpri, cmpre, ext_len, padding);

    if(segments_left == 0) {
      /* We are the final destination, do nothing */
    } else {
      uint8_t i = path_len - segments_left; /* The index of the next address to be visited */
      uint8_t *addr_ptr = ((uint8_t *)UIP_RH_BUF) + RPL_RH_LEN + RPL_SRH_LEN + (i * (16 - cmpri));
      uint8_t cmpr = segments_left == 1 ? cmpre : cmpri;

      /* As per RFC6554: swap the IPv6 destination address and address[i] */

      /* First, copy the current IPv6 destination address */
      uip_ipaddr_copy(&current_dest_addr, &UIP_IP_BUF->destipaddr);
      /* Second, update the IPv6 destination address with addresses[i] */
      memcpy(((uint8_t *)&UIP_IP_BUF->destipaddr) + cmpr, addr_ptr, 16 - cmpr);
      /* Third, write current_dest_addr to addresses[i] */
      memcpy(addr_ptr, ((uint8_t *)&current_dest_addr) + cmpr, 16 - cmpr);

      /* Update segments left field */
      UIP_RH_BUF->seg_left--;
      UIP_IP_BUF->ttl = UIP_IP_BUF->ttl-1;
      PRINTF("RPL: SRH next hop ");
      PRINT6ADDR(&UIP_IP_BUF->destipaddr);
      PRINTF("\n");
    }
    uip_ext_len = last_uip_ext_len;
    return 1;
  }

  uip_ext_len = last_uip_ext_len;
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
count_matching_bytes(const void *p1, const void *p2, size_t n)
{
  int i = 0;
  for(i = 0; i < n; i++) {
    if(((uint8_t *)p1)[i] != ((uint8_t *)p2)[i]) {
      return i;
    }
  }
  return n;
}
/*---------------------------------------------------------------------------*/
extern uint16_t udp_port_no;
extern uint8_t data_pkt_type;
uint8_t next_header_data[100] = {0};
//uint8_t debug_uip_l2_l3_hdr_len=0;
static int
insert_srh_header(void)
{
  /* Implementation of RFC6554 */
  uint8_t temp_len;
  uint8_t path_len;
  uint8_t ext_len;
  uint8_t cmpri, cmpre; /* ComprI and ComprE fields of the RPL Source Routing Header */
  uint8_t *hop_ptr;
  uint8_t padding;
  rpl_ns_node_t *dest_node;
  rpl_ns_node_t *root_node;
  rpl_ns_node_t *node;
  rpl_dag_t *dag;
  uip_ipaddr_t node_addr;
  PRINTF("RPL: SRH creating source routing header with destination ");
  PRINT6ADDR(&UIP_IP_BUF->destipaddr);
  PRINTF(" \n");

  /* Construct source route. We do not do this recursively to keep the runtime 
  stack usage constant. */

  /* Get link of the destination and root */
  dag = rpl_get_dag(&UIP_IP_BUF->destipaddr);

  if(dag == NULL) {
    PRINTF("RPL: SRH DAG not found\n");
    return 0;
  }

  dest_node = rpl_ns_get_node(dag, &UIP_IP_BUF->destipaddr);//nodelist
  if(dest_node == NULL) {
    /* The destination is not found, skip SRH insertion */
    return 1;
  }

  root_node = rpl_ns_get_node(dag, &dag->dag_id);//nodelist
  if(root_node == NULL) {
    PRINTF("RPL: SRH root node not found\n");
    return 0;
  }

  //santosh: when should we check node reachable status....
  //  if(!rpl_ns_is_node_reachable(dag, &UIP_IP_BUF->destipaddr)) {
  //    PRINTF("RPL: SRH no path found to destination\n");
  //    return 0;
  //  }

  /* Compute path length and compression factors (we use cmpri == cmpre) */
  path_len = 0;
  node = dest_node->parent;
  /* For simplicity, we use cmpri = cmpre */
  cmpri = 15;
  cmpre = 15;

  if(node == root_node) {
    PRINTF("RPL: SRH no need to insert SRH\n");
    return 1;
  }
  
  //santosh: Look into below logic, srh should add list of hopped devices
  while(node != NULL && node != root_node) {
    UIP_IP_BUF->ttl--;
    rpl_ns_get_node_global_addr(&node_addr, node);

    /* How many bytes in common between all nodes in the path? */
    cmpri = MIN(cmpri, count_matching_bytes(&node_addr, &UIP_IP_BUF->destipaddr, 16));
    cmpre = cmpri;

    PRINTF("RPL: SRH Hop ");
    PRINT6ADDR(&node_addr);
    PRINTF("\n");
    node = node->parent;
    path_len++;

    if(path_len == 24)//max number of hop currently supporting//temp
      break;
  }

  /* Extension header length: fixed headers + (n-1) * (16-ComprI) + (16-ComprE)*/
  ext_len = RPL_RH_LEN + RPL_SRH_LEN
      + (path_len - 1) * (16 - cmpre)
      + (16 - cmpri);

  padding = ext_len % 8 == 0 ? 0 : (8 - (ext_len % 8));
  ext_len += padding;

  PRINTF("RPL: SRH Path len: %u, ComprI %u, ComprE %u, ext len %u (padding %u)\n",
      path_len, cmpri, cmpre, ext_len, padding);

  /* Check if there is enough space to store the extension header */
  if(uip_len + ext_len > UIP_BUFSIZE) {
    PRINTF("RPL: Packet too long: impossible to add source routing header (%u bytes)\n", ext_len);
    return 1;
  }

  ///* Move existing ext headers and payload uip_ext_len further */
  //memmove(uip_buf + uip_l2_l3_hdr_len + ext_len,
  //uip_buf + uip_l2_l3_hdr_len, uip_len - UIP_IPH_LEN);
  //memset(uip_buf + uip_l2_l3_hdr_len, 0, ext_len);

  //debug_uip_l2_l3_hdr_len = uip_l2_l3_hdr_len;//santosh: debug
  
  /* Insert source routing header */
  memcpy(next_header_data, (uint8_t*)&uip_buf, UIP_IPH_LEN);
  /* Move existing ext headers and payload uip_ext_len further */
  memmove(uip_buf + uip_l2_l3_hdr_len + ext_len + UIP_IPH_LEN,
          uip_buf + UIP_IPH_LEN, uip_len - UIP_IPH_LEN);                
  /*reset the outer-ip header and srh*/
  memset(uip_buf + uip_l2_l3_hdr_len, 0, ext_len+UIP_IPH_LEN);    
  UIP_RH_BUF->next = UIP_IPV6_NEXT_HEADER;//inner ipv6 header for tunneling
  UIP_IP_BUF->proto = UIP_PROTO_ROUTING;//sicslowpan next header as ipv6 header with srh
  
  
  /* Initialize IPv6 Routing Header */
  UIP_RH_BUF->len = (ext_len - 8) / 8;
  UIP_RH_BUF->routing_type = RPL_RH_TYPE_SRH;
  UIP_RH_BUF->seg_left = path_len;
  
  /* Initialize RPL Source Routing Header */
  UIP_RPL_SRH_BUF->cmpr = (cmpri << 4) + cmpre;
  UIP_RPL_SRH_BUF->pad = padding << 4;
  
  /* Initialize addresses field (the actual source route).
  * From last to first. */
  node = dest_node;
  hop_ptr = ((uint8_t *)UIP_RH_BUF) + ext_len - padding; /* Pointer where to write the next hop compressed address */
  
  while(node != NULL && node->parent != root_node) {
    rpl_ns_get_node_global_addr(&node_addr, node);
    
    hop_ptr -= (16 - cmpri);
    memcpy(hop_ptr, ((uint8_t*)&node_addr) + cmpri, 16 - cmpri);
    
    node = node->parent;
  }

  /* The next hop (i.e. node whose parent is the root) is placed as the current IPv6 destination */
  rpl_ns_get_node_global_addr(&node_addr, node);
  uip_ipaddr_copy(&UIP_IP_BUF->destipaddr, &node_addr);
  
  /* In-place update of IPv6 length field */
  temp_len = UIP_IP_BUF->len[1];
  
  
  UIP_IP_BUF->len[1] += (ext_len + UIP_IPH_LEN);
  
  
  if(UIP_IP_BUF->len[1] < temp_len) {
    UIP_IP_BUF->len[0]++;
  }
  
  UIP_IP_BUF->ttl = path_len +1;
  /*ext_len increases, while hop increases, tested with 5 hops*///santosh
  memcpy(uip_buf+UIP_IPH_LEN+ext_len, next_header_data, UIP_IPH_LEN);  //coping orginal inner header 
  
  
  uip_ext_len += ext_len;
  
  uip_len += (ext_len + UIP_IPH_LEN);
  
  
  return 1;
}
#else /* RPL_WITH_NON_STORING */
int insert_srh_header(void);
#endif /* RPL_WITH_NON_STORING */
/*---------------------------------------------------------------------------*/
static int
update_hbh_header(void)
{
  rpl_instance_t *instance;
  int uip_ext_opt_offset;
  int last_uip_ext_len;
  rpl_parent_t *parent;

  last_uip_ext_len = uip_ext_len;
  uip_ext_len = 0;
  uip_ext_opt_offset = 2;

  if(UIP_IP_BUF->proto == UIP_PROTO_HBHO && UIP_EXT_HDR_OPT_RPL_BUF->opt_type == UIP_EXT_HDR_OPT_RPL) {
    if(UIP_HBHO_BUF->len != ((RPL_HOP_BY_HOP_LEN - 8) / 8)
        || UIP_EXT_HDR_OPT_RPL_BUF->opt_len != RPL_HDR_OPT_LEN) {

      PRINTF("RPL: Hop-by-hop extension header has wrong size (%u %u)\n",
          UIP_EXT_HDR_OPT_RPL_BUF->opt_len,
          uip_ext_len);
      return 0; /* Drop */
    }

    instance = rpl_get_instance(UIP_EXT_HDR_OPT_RPL_BUF->instance);
    if(instance == NULL || !instance->used || !instance->current_dag->joined) {
      PRINTF("RPL: Unable to add/update hop-by-hop extension header: incorrect instance\n");
      uip_ext_len = last_uip_ext_len;
      return 0; /* Drop */
    }

    PRINTF("RPL: Updating RPL option\n");
    /* Update sender rank and instance, will update flags next */
    UIP_EXT_HDR_OPT_RPL_BUF->senderrank = UIP_HTONS(instance->current_dag->rank);
    UIP_EXT_HDR_OPT_RPL_BUF->instance = instance->instance_id;

    if(RPL_IS_STORING(instance)) { /* In non-storing mode, downwards traffic does not have the HBH option */
      /* Check the direction of the down flag, as per Section 11.2.2.3,
            which states that if a packet is going down it should in
            general not go back up again. If this happens, a
            RPL_HDR_OPT_FWD_ERR should be flagged. */
      if((UIP_EXT_HDR_OPT_RPL_BUF->flags & RPL_HDR_OPT_DOWN)) {
        if(uip_ds6_route_lookup(&UIP_IP_BUF->destipaddr) == NULL) {
          UIP_EXT_HDR_OPT_RPL_BUF->flags |= RPL_HDR_OPT_FWD_ERR;
          PRINTF("RPL forwarding error\n");
          /* We should send back the packet to the originating parent,
                but it is not feasible yet, so we send a No-Path DAO instead */
          PRINTF("RPL generate No-Path DAO\n");
          parent = rpl_get_parent((uip_lladdr_t *)packetbuf_addr(PACKETBUF_ADDR_SENDER));
          if(parent != NULL) {
            dao_output_target(parent, &UIP_IP_BUF->destipaddr, RPL_ZERO_LIFETIME);
          }
          /* Drop packet */
          return 0;
        }
      } else {
        /* Set the down extension flag correctly as described in Section
              11.2 of RFC6550. If the packet progresses along a DAO route,
              the down flag should be set. */
        if(uip_ds6_route_lookup(&UIP_IP_BUF->destipaddr) == NULL) {
          /* No route was found, so this packet will go towards the RPL
                root. If so, we should not set the down flag. */
          UIP_EXT_HDR_OPT_RPL_BUF->flags &= ~RPL_HDR_OPT_DOWN;
          PRINTF("RPL option going up\n");
        } else {
          /* A DAO route was found so we set the down flag. */
          UIP_EXT_HDR_OPT_RPL_BUF->flags |= RPL_HDR_OPT_DOWN;
          PRINTF("RPL option going down\n");
        }
      }
    }
  }

  uip_ext_len = last_uip_ext_len;
  return 1;
}
/*---------------------------------------------------------------------------*/
extern uip_ipaddr_t* get_parent_link_local_addr();
extern uip_ipaddr_t* get_parent_global_addr();
static uint8_t self_mac_addr[8] = {0};
extern uip_ipaddr_t* uip_get_def_route_ipaddr();
extern void get_self_extended_address_reverse(uint8_t *self_mac_addr);
uint8_t is_my_prefered_parent(); 

static int
insert_hbh_header(const rpl_instance_t *instance)
{
  int uip_ext_opt_offset;
  int last_uip_ext_len;
  uint8_t temp_len;
  
  last_uip_ext_len = uip_ext_len;
  uip_ext_len = 0;
  uip_ext_opt_offset = 2;
  
  /* Insert hop-by-hop header */
  PRINTF("RPL: Creating hop-by-hop option\n");
  if(uip_len + RPL_HOP_BY_HOP_LEN > UIP_BUFSIZE) {
    PRINTF("RPL: Packet too long: impossible to add hop-by-hop option\n");
    uip_ext_len = last_uip_ext_len;
    return 0;
  }
  
  if(!is_my_prefered_parent())
  {
    memcpy(next_header_data, (uint8_t*)&uip_buf, UIP_IPH_LEN);
    /* Move existing ext headers and payload UIP_EXT_BUF further */
    memmove(uip_buf+UIP_IPH_LEN+UIP_IPH_LEN+RPL_HOP_BY_HOP_LEN, uip_buf+UIP_IPH_LEN, 
            uip_len-UIP_IPH_LEN/*UIP_IPH_LEN+RPL_HOP_BY_HOP_LEN*/);
    /*reset the outer-ip header and hbh*/
    memset(UIP_HBHO_BUF, 0, UIP_IPH_LEN+RPL_HOP_BY_HOP_LEN); 
    /* Update IP and HBH protocol and fields */
    UIP_HBHO_BUF->next = UIP_IPV6_NEXT_HEADER;//santosh inner ipv6 header for tunneling
    UIP_IP_BUF->proto = UIP_PROTO_HBHO;//sicslowpan next header as ipv6 header with hbh  
    //uip_ipaddr_t* parent_link_addr = get_parent_link_local_addr();
    uip_ds6_nbr_t *l_nbr = (uip_ds6_nbr_t *)uip_get_prio_based_parent_nbr(1);
    
    get_self_extended_address_reverse(self_mac_addr);
    uip_lladdr_t ll_add;
    uip_ipaddr_t self_link_locaddr;      
    memcpy(ll_add.addr,self_mac_addr,8);
    uip_create_linklocal_prefix(&self_link_locaddr);
    uip_ds6_set_addr_iid(&self_link_locaddr, &ll_add);
    //uip_ipaddr_t* up_parent = uip_get_def_route_ipaddr();
    uip_ipaddr_copy(&UIP_IP_BUF->srcipaddr, &self_link_locaddr);
    uip_ipaddr_copy(&UIP_IP_BUF->destipaddr, &l_nbr->ipaddr);
    /* Initialize HBH option */
    UIP_HBHO_BUF->len = (RPL_HOP_BY_HOP_LEN - 8) / 8;
    UIP_EXT_HDR_OPT_RPL_BUF->opt_type = UIP_EXT_HDR_OPT_RPL;
    UIP_EXT_HDR_OPT_RPL_BUF->opt_len = RPL_HDR_OPT_LEN;
    UIP_EXT_HDR_OPT_RPL_BUF->flags = 0;
    UIP_EXT_HDR_OPT_RPL_BUF->senderrank  = 0;//UIP_HTONS(instance->current_dag->rank);
    UIP_EXT_HDR_OPT_RPL_BUF->instance = instance->instance_id;
    UIP_IP_BUF->ttl = uip_ds6_if.cur_hop_limit;
    memcpy(uip_buf+UIP_IPH_LEN+RPL_HOP_BY_HOP_LEN, next_header_data, UIP_IPH_LEN);
    uip_len += (RPL_HOP_BY_HOP_LEN+UIP_IPH_LEN);//+40 for udp packet type LEN OF UIP_IP_BUF santosh
    UIP_IP_BUF->len[1] += (RPL_HOP_BY_HOP_LEN+40);//+40 for udp packet type LEN OF UIP_IP_BUF santosh
  }
  else{
    /* Move existing ext headers and payload UIP_EXT_BUF further */
    memmove(UIP_HBHO_NEXT_BUF, UIP_EXT_BUF, uip_len - UIP_IPH_LEN);
    memset(UIP_HBHO_BUF, 0, RPL_HOP_BY_HOP_LEN);   
    /* Update IP and HBH protocol and fields */
    UIP_HBHO_BUF->next = UIP_IP_BUF->proto;
    UIP_IP_BUF->proto = UIP_PROTO_HBHO;   
    /* Initialize HBH option */
    UIP_HBHO_BUF->len = (RPL_HOP_BY_HOP_LEN - 8) / 8;
    UIP_EXT_HDR_OPT_RPL_BUF->opt_type = UIP_EXT_HDR_OPT_RPL;
    UIP_EXT_HDR_OPT_RPL_BUF->opt_len = RPL_HDR_OPT_LEN;
    UIP_EXT_HDR_OPT_RPL_BUF->flags = 0;
    UIP_EXT_HDR_OPT_RPL_BUF->senderrank = 0;//UIP_HTONS(instance->current_dag->rank);
    UIP_EXT_HDR_OPT_RPL_BUF->instance = instance->instance_id;	
    
    uip_len += RPL_HOP_BY_HOP_LEN;
    UIP_IP_BUF->len[1] += RPL_HOP_BY_HOP_LEN;
  }
  
  temp_len = UIP_IP_BUF->len[1];
  if(UIP_IP_BUF->len[1] < temp_len) {
    UIP_IP_BUF->len[0]++;
  }
  uip_ext_len = last_uip_ext_len + RPL_HOP_BY_HOP_LEN;
  
  /* Update header before returning */
  //return update_hbh_header();
  return 1;
}
/*---------------------------------------------------------------------------*/
void
rpl_remove_header(void)
{
  uint8_t temp_len;
  uint8_t rpl_ext_hdr_len;
  int uip_ext_opt_offset;
  uint8_t *uip_next_hdr;

  uip_ext_len = 0;
  uip_ext_opt_offset = 2;
  uip_next_hdr = &UIP_IP_BUF->proto;

  /* Look for hop-by-hop and routing headers */
  while(uip_next_hdr != NULL) {
    switch(*uip_next_hdr) {
      case UIP_PROTO_HBHO:
      case UIP_PROTO_ROUTING:
        if((*uip_next_hdr != UIP_PROTO_HBHO || UIP_EXT_HDR_OPT_RPL_BUF->opt_type == UIP_EXT_HDR_OPT_RPL)) {
          /* Remove hop-by-hop and routing headers */
          *uip_next_hdr = UIP_EXT_BUF->next;
          rpl_ext_hdr_len = (UIP_EXT_BUF->len * 8) + 8;
          temp_len = UIP_IP_BUF->len[1];
          uip_len -= rpl_ext_hdr_len;
          UIP_IP_BUF->len[1] -= rpl_ext_hdr_len;
          if(UIP_IP_BUF->len[1] > temp_len) {
            UIP_IP_BUF->len[0]--;
          }
          //LOG_DBG("Removing RPL extension header (type %u, len %u)\n", *uip_next_hdr, rpl_ext_hdr_len);
          memmove(UIP_EXT_BUF, ((uint8_t *)UIP_EXT_BUF) + rpl_ext_hdr_len, uip_len - UIP_IPH_LEN);
        } else {
          uip_next_hdr = &UIP_EXT_BUF->next;
          uip_ext_len += (UIP_EXT_BUF->len << 3) + 8;
        }
        break;
      case UIP_PROTO_DESTO:
        /*
         * As per RFC 2460, any header other than the Destination
         * Options header does not appear between the Hop-by-Hop
         * Options header and the Routing header.
         *
         * We're moving to the next header only if uip_next_hdr has
         * UIP_PROTO_DESTO. Otherwise, we'll return.
         */
        /* Move to next header */
        uip_next_hdr = &UIP_EXT_BUF->next;
        uip_ext_len += (UIP_EXT_BUF->len << 3) + 8;
        break;
    default:
      return;
    }
  }
}
/*---------------------------------------------------------------------------*/
//extern uint16_t udp_port_no;
uint8_t tunnling_down = 0XFF;
//extern uint8_t get_link_status(uint8_t* rcv_addr);
//int
//rpl_update_header(void)
//{
//  //Suneet :: add this check if packet send to link local no need to chack in nbr table and check direct aceess 
//  if(!(uip_is_addr_linklocal(&UIP_IP_BUF->destipaddr)))    
//  {
//    {
//  /*while a device is NOT in direct link and we are trying to send data with link
//    local address, then it should drop the packet*/  
//    if(uip_is_addr_linklocal(&UIP_IP_BUF->destipaddr) &&
//      uip_ds6_is_addr_directlink(&UIP_IP_BUF->destipaddr) != 0x01){  
//      return 0;
//      }
//    }
//  }
//  if(UIP_IP_BUF->proto == UIP_IPV6_NEXT_HEADER)/*0x29*/
//    goto next;
//  
////  if((data_pkt_type == 0x80 || data_pkt_type == 0x81 || 
////      UIP_IP_BUF->proto == 0x11) && /*get_link_status((uint8_t*)&UIP_IP_BUF->destipaddr) == 0x01*/
////     uip_ds6_is_addr_directlink(&UIP_IP_BUF->destipaddr) == 0x01){//udp header
////    data_pkt_type = 0xFF;
////    return 1;
////    }  
//
//  if((default_instance == NULL || default_instance->current_dag == NULL || 
//     uip_is_addr_linklocal(&UIP_IP_BUF->destipaddr) || 
//     uip_is_addr_mcast(&UIP_IP_BUF->destipaddr))) {
//       if(tunnling_down == 0x01)
//       {
//        UIP_IP_BUF->ttl = (UIP_IP_BUF->ttl -1);
//        tunnling_down = 0;
//       }
//    return 1;
//  }
//
//  
//next:
//  if(default_instance->current_dag->rank == ROOT_RANK(default_instance)) {
//    /* At the root, remove headers if any, and insert SRH or HBH
//     * (SRH is inserted only if the destination is in the DODAG) */
//    rpl_remove_header();
//    if(RPL_IS_NON_STORING(default_instance)) {
//      return insert_srh_header();
//    } else {
//      return insert_hbh_header(default_instance);
//    }
//  } else {
//    if(uip_ds6_is_my_addr(&UIP_IP_BUF->srcipaddr)
//        && UIP_IP_BUF->ttl == uip_ds6_if.cur_hop_limit) {
//      /* Insert HBH option at source. Checking the address is not sufficient because
//       * in non-storing mode, a packet may go up and then down the same path again */
//      return insert_hbh_header(default_instance);
//    } else {
//      /* Update HBH option at forwarders */
//      return update_hbh_header();
//    }
//  }
//}
int
rpl_update_header(void)
{
  if(default_instance == NULL || default_instance->current_dag == NULL
     || uip_is_addr_linklocal(&UIP_IP_BUF->destipaddr) || uip_is_addr_mcast(&UIP_IP_BUF->destipaddr)) {
       return 1;
  }

  if(default_instance->current_dag->rank == ROOT_RANK(default_instance)) {
    /* At the root, remove headers if any, and insert SRH or HBH
    * (SRH is inserted only if the destination is in the DODAG) */
    rpl_remove_header();
    if(rpl_get_dag(&UIP_IP_BUF->destipaddr) != NULL) {
      /* dest is in a DODAG; the packet is going down. */
      if(RPL_IS_NON_STORING(default_instance)) {
        return insert_srh_header();
      } else {
        return insert_hbh_header(default_instance);
      }
    } else {
      /* dest is outside of DODAGs; no ext header is needed. */
      return 1;
    }
  } else {
    if(uip_ds6_is_my_addr(&UIP_IP_BUF->srcipaddr)
        && UIP_IP_BUF->ttl == uip_ds6_if.cur_hop_limit) {
      /* Insert HBH option at source. Checking the address is not sufficient because
       * in non-storing mode, a packet may go up and then down the same path again */
      return insert_hbh_header(default_instance);
    } else {
      /* Update HBH option at forwarders */
      return update_hbh_header();
    }
  }
}
/*---------------------------------------------------------------------------*/
void rpl_ipv6_tunneling(uip_ipaddr_t *dest_ip_addr)
{
//    uint16_t IP_payload_length = uip_len;
//    
//    memcpy(next_header_data, (uint8_t*)UIP_IP_BUF, uip_len); 
//
//    UIP_IP_BUF->vtc = 0x60;
//    UIP_IP_BUF->tcflow = 0x00;
//    UIP_IP_BUF->flow = 0x00;
//    UIP_IP_BUF->proto = UIP_IPV6_NEXT_HEADER;/*0x29(41) decimal IPv6 header*/
//    UIP_IP_BUF->ttl = uip_ds6_if.cur_hop_limit;
//    uip_ipaddr_copy(&UIP_IP_BUF->destipaddr, (dest_ip_addr));
//    uip_ds6_select_src(&UIP_IP_BUF->srcipaddr, &UIP_IP_BUF->destipaddr);
//
//    uint8_t* ptr = (uint8_t*)&UIP_IP_BUF->destipaddr;
//    ptr+=16;
//
//    uip_len = UIP_IPH_LEN + uip_len;
//    UIP_IP_BUF->len[0] = (uint8_t)((uip_len - 40) >> 8);
//    UIP_IP_BUF->len[1] = (uint8_t)((uip_len - 40) & 0x00FF);
//    
//    memcpy(ptr,next_header_data,IP_payload_length);
//
//    tcpip_ipv6_output();    
}
/*---------------------------------------------------------------------------*/
uint8_t is_my_prefered_parent()
{
  //uip_ipaddr_t* parent_link_addr = get_parent_link_local_addr();
  uip_ds6_nbr_t *l_nbr = (uip_ds6_nbr_t *)uip_get_prio_based_parent_nbr(1);
  uip_ipaddr_t* parent_global_addr = get_parent_global_addr();
  uint8_t  dest_addr[16] = {0};
  memcpy(&dest_addr,&UIP_IP_BUF->destipaddr,16);
  if((!memcmp(&l_nbr->ipaddr,&dest_addr,16))
     ||(!memcmp(parent_global_addr->u8,&dest_addr,16)))
  {
    return 1;  
  }
  else
  {
    return 0;
  }
}
/*---------------------------------------------------------------------------*/
uint8_t* tunnel_ptr = NULL;
uint16_t uip_data_len = 0;
int compare_self_ipv6_addr(uint8_t *l_addr);
void change_dest_addr(uint8_t *l_addr);
extern uint8_t root_device;
extern uint16_t exten_header_offset;
uint8_t srh_len = 0x00;
extern uint8_t tunnling_down;
void 
process_ipv6_tunneling(){
  //int i, j;
  uint8_t down_flag = 0x00;
  uint8_t temp_len;
  uint8_t rpl_ext_hdr_len;
  uint8_t *uip_next_hdr; 
  uip_ext_len = 0;
  uip_next_hdr = &UIP_IP_BUF->proto;
  tunnel_ptr = (uint8_t*)UIP_IP_BUF;
  temp_len = UIP_IPH_LEN;
  //uip_ipaddr_t *parent_ipaddr;
  uip_ds6_addr_t addr;
  while(uip_next_hdr != NULL)
  {
    switch(*uip_next_hdr) 
    {
    case UIP_PROTO_HBHO:
      uip_next_hdr = &UIP_EXT_BUF->next;
      rpl_ext_hdr_len = (UIP_EXT_BUF->len * 8) + 8;
      tunnel_ptr += rpl_ext_hdr_len + temp_len;
      temp_len = 0;
      break;
    case UIP_PROTO_ROUTING:
      uip_next_hdr = &UIP_EXT_BUF->next;
      rpl_ext_hdr_len = (UIP_EXT_BUF->len * 8) + 8;
      tunnel_ptr += rpl_ext_hdr_len + temp_len;
      if(compare_self_ipv6_addr(tunnel_ptr+24)){//comparing self with destination
      //finding the srh len, depending upon the hop count
      uip_data_len = ((tunnel_ptr[4]<<8) | tunnel_ptr[5]) + UIP_IPH_LEN/*40*/;
      exten_header_offset = UIP_IPH_LEN;
      memcpy(UIP_IP_BUF, tunnel_ptr, uip_data_len);      
      uip_input();
      return;
    }
      break;
    case UIP_IPV6_NEXT_HEADER:
          memcpy(&addr.ipaddr,tunnel_ptr+24,16);
        if(*(tunnel_ptr+4) == 0x00){/*down flag index*/
          /*although this packet is for me(off-link), but in non-storing mode, the 
          packet will be forward to the root*/
          down_flag = 0x01;
        }
        if(compare_self_ipv6_addr(tunnel_ptr+24)){/*destination with self*/
          if(!down_flag || root_device == 0x01){/*on-link*/ 
            uip_data_len = ((tunnel_ptr[4]<<8) | tunnel_ptr[5]) + UIP_IPH_LEN;//>>//santosh:check
            exten_header_offset = UIP_IPH_LEN;
            memcpy(UIP_IP_BUF, tunnel_ptr, uip_data_len);
            uip_input();
            return;
          }
        }
        else if(uip_is_addr_mpl_mcast_forwarders_for_non_group(&addr.ipaddr))
        {
          uip_data_len = ((tunnel_ptr[4]<<8) | tunnel_ptr[5]) + UIP_IPH_LEN;//>>//santosh:check
          exten_header_offset = UIP_IPH_LEN;
          memcpy(UIP_IP_BUF, tunnel_ptr, uip_data_len);
          uip_input();
          return;
        }
        else{
          if(default_instance->current_dag->rank == ROOT_RANK(default_instance)){
            uip_data_len = ((tunnel_ptr[4]<<8) | tunnel_ptr[5]) + UIP_IPH_LEN;//>>//santosh:check
            memcpy(UIP_IP_BUF, tunnel_ptr, uip_data_len);
            if(!down_flag || root_device == 0x01){/*on-link*/ 
              uip_len = uip_data_len;  //Suneet :: after remove tunnling remaing length is uip_len
            }
            //change_dest_addr(tunnel_ptr+24);/*tunnel_ptr+24 is destination address*/
            return;
          }
          
        }
        
        uip_data_len = ((tunnel_ptr[4]<<8) | tunnel_ptr[5])+ UIP_IPH_LEN;  //suneet :: remove tunnling 
        memcpy(UIP_IP_BUF, tunnel_ptr, uip_data_len);
        uip_len = uip_data_len;  //Suneet :: after remove tunnling remaing length is uip_len
        UIP_IP_BUF->ttl--;
        insert_hbh_header(default_instance);//insert hop by hop header
        return;
//        /*forwarding the packet to the next device*/
//        parent_ipaddr = uip_get_prio_based_parent_ip(1);
//        if(parent_ipaddr != NULL){
//          update_hbh_header();
//          for(i = 0; i < RPL_MAX_INSTANCES; ++i) {
//            if(instance_table[i].used) {
//              for(j = 0; j < RPL_MAX_DAG_PER_INSTANCE; ++j) {
//                /*Suneet :: this condition used when router flag is set here 
//                no need to compare prefix instance_table[i].dag_table[j].prefix_info.flags & UIP_ND6_NA_FLAG_OVERRIDE*/
//                if(instance_table[i].dag_table[j].prefix_info.flags & UIP_ND6_NA_FLAG_OVERRIDE) //when r flag set
//                {
//                  /* uip_ipaddr_t *self_ip_addr = (uip_ipaddr_t*)get_self_global_addr();
//                  UIP_IP_BUF->srcipaddr = *self_ip_addr; //UIP_IP_BUF->destipaddr;*/
//                  UIP_IP_BUF->srcipaddr = UIP_IP_BUF->destipaddr;
//                  uip_create_linklocal_prefix(&UIP_IP_BUF->srcipaddr);
//                }
//                if(instance_table[i].dag_table[j].prefix_info.flags & UIP_ND6_RA_FLAG_AUTONOMOUS) //when a flag set 
//                {
//                  UIP_IP_BUF->srcipaddr = UIP_IP_BUF->destipaddr;
//                  uip_create_linklocal_prefix(&UIP_IP_BUF->srcipaddr);
//                }
//              }
//            }
//          }
//          tunnling_down = 0x01;
//          UIP_IP_BUF->destipaddr = *parent_ipaddr;
//          uip_create_linklocal_prefix(&UIP_IP_BUF->destipaddr);
//          uip_next_hdr = NULL;
//        }    
      break;
    default:
      uip_next_hdr = NULL;
      break;
    }
  }
//  uint8_t down_flag = 0x00;
//   int i, j;
//  tunnel_ptr = (uint8_t*)UIP_IP_BUF;
//  
//  /*checking UIP_PROTO_ROUTING header and comparing destination address with se-
//  lf address, if true, sending packet to the application, this condition was for
//  source device while it receives the echo reply*/
//  if(*(tunnel_ptr+6) == UIP_PROTO_ROUTING){/*0x43*/
//    srh_len = (tunnel_ptr[UIP_IPH_LEN+1] * 8) + 8;//UIP_IPH_LEN+1 = index of srh length
//    //removing the outer header
//    tunnel_ptr += (UIP_IPH_LEN+srh_len);
//    /*removing the outer-ip header, routing header and checking*/
//    //tunnel_ptr+=(UIP_IPH_LEN/*40*/+16)/*routing header length*/;//santosh
//    if(compare_self_ipv6_addr(tunnel_ptr+24)){//comparing self with destination
//      //finding the srh len, depending upon the hop count
//      uip_data_len = ((tunnel_ptr[4]<<8) | tunnel_ptr[5]) + UIP_IPH_LEN/*40*/;
//      exten_header_offset = UIP_IPH_LEN;
//      memcpy(UIP_IP_BUF, tunnel_ptr, uip_data_len);      
//      uip_input();
//      return;
//    }
//    else
//    {
//      uip_data_len = ((tunnel_ptr[4]<<8) | tunnel_ptr[5]) + UIP_IPH_LEN;//>>//santosh:check
//      memcpy(UIP_IP_BUF, tunnel_ptr, uip_data_len);
//      uip_len = uip_data_len;  //Suneet :: after remove tunnling remaing length is uip_len
//      //change_dest_addr(tunnel_ptr+24);/*tunnel_ptr+24 is destination address*/ 
//      return;
//    }
//  }
//  
//  /*removing the outer-ip header and checking*/
//  tunnel_ptr+=UIP_IPH_LEN;/*40*/  
//  
//  if(*tunnel_ptr == UIP_IPV6_NEXT_HEADER)/*0x29*/{
//    if(*(tunnel_ptr+4) == 0x00){/*down flag index*/
//      /*although this packet is for me(off-link), but in non-storing mode, the 
//      packet will be forward to the root*/
//      down_flag = 0x01;
//    }
//    tunnel_ptr+=8;
//    if(compare_self_ipv6_addr(tunnel_ptr+24)){/*destination with self*/
//      if(!down_flag || root_device == 0x01){/*on-link*/ 
//        uip_data_len = ((tunnel_ptr[4]<<8) | tunnel_ptr[5]) + UIP_IPH_LEN;//>>//santosh:check
//        exten_header_offset = UIP_IPH_LEN;
//        memcpy(UIP_IP_BUF, tunnel_ptr, uip_data_len);
//        uip_input();
//      }
//    }    
//    else{
//      if(default_instance->current_dag->rank == ROOT_RANK(default_instance)){
//        uip_data_len = ((tunnel_ptr[4]<<8) | tunnel_ptr[5]) + UIP_IPH_LEN;//>>//santosh:check
//        memcpy(UIP_IP_BUF, tunnel_ptr, uip_data_len);
//        if(!down_flag || root_device == 0x01){/*on-link*/ 
//         uip_len = uip_data_len;  //Suneet :: after remove tunnling remaing length is uip_len
//        }
//        //change_dest_addr(tunnel_ptr+24);/*tunnel_ptr+24 is destination address*/
//      }
//    }
//   
//    /*forwarding the packet to the next device*/
//    uip_ipaddr_t *parent_ipaddr = uip_get_prio_based_parent_ip(1);
//    if(parent_ipaddr != NULL){
//    
//        
//      for(i = 0; i < RPL_MAX_INSTANCES; ++i) {
//        if(instance_table[i].used) {
//          for(j = 0; j < RPL_MAX_DAG_PER_INSTANCE; ++j) {
//            /*Suneet :: this condition used when router flag is set here 
//            no need to compare prefix instance_table[i].dag_table[j].prefix_info.flags & UIP_ND6_NA_FLAG_OVERRIDE*/
//            if(instance_table[i].dag_table[j].prefix_info.flags & UIP_ND6_NA_FLAG_OVERRIDE) //when r flag set
//            {
//              /* uip_ipaddr_t *self_ip_addr = (uip_ipaddr_t*)get_self_global_addr();
//               UIP_IP_BUF->srcipaddr = *self_ip_addr; //UIP_IP_BUF->destipaddr;*/
//              UIP_IP_BUF->srcipaddr = UIP_IP_BUF->destipaddr;
//              uip_create_linklocal_prefix(&UIP_IP_BUF->srcipaddr);
//            }
//            if(instance_table[i].dag_table[j].prefix_info.flags & UIP_ND6_RA_FLAG_AUTONOMOUS) //when a flag set 
//            {
//                 UIP_IP_BUF->srcipaddr = UIP_IP_BUF->destipaddr;
//                 uip_create_linklocal_prefix(&UIP_IP_BUF->srcipaddr);
//            }
//          }
//        }
//      }
//      tunnling_down = 0x01;
//      UIP_IP_BUF->destipaddr = *parent_ipaddr;
//      uip_create_linklocal_prefix(&UIP_IP_BUF->destipaddr);
//    }    
//  }
//  else{
//    if(*(tunnel_ptr+6) == UIP_PROTO_HBHO){
//       if(compare_self_ipv6_addr(tunnel_ptr+24)){/*7th index is the proto && 25th index is the addr*/
//          uip_data_len = ((tunnel_ptr[4]<<8) | tunnel_ptr[5]) + UIP_IPH_LEN;//>>//santosh:check
//          memcpy(UIP_IP_BUF, tunnel_ptr, uip_data_len);
//          uip_input();
//       }
//       else{
//          change_dest_addr(tunnel_ptr+24);/*tunnel_ptr+24 is destination address*/
//       }
//    }
//  }  
  return;
}
