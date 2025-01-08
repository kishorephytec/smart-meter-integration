/*
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
#include "Stack6LoWPANConf.h"
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <math.h>

#include "contiki-net.h"
#include "common_function.h"

//#include "dhcpv6.h"


#define DEBUG DEBUG_NONE
#include "uip-debug.h"

#ifdef UIP_CONF_DS6_NEIGHBOR_STATE_CHANGED
#define NEIGHBOR_STATE_CHANGED(n) UIP_CONF_DS6_NEIGHBOR_STATE_CHANGED(n)
void NEIGHBOR_STATE_CHANGED(uip_ds6_nbr_t *n);
#else
#define NEIGHBOR_STATE_CHANGED(n)
#endif /* UIP_DS6_CONF_NEIGHBOR_STATE_CHANGED */

#ifdef UIP_CONF_DS6_LINK_NEIGHBOR_CALLBACK
#define LINK_NEIGHBOR_CALLBACK(addr, status, numtx) UIP_CONF_DS6_LINK_NEIGHBOR_CALLBACK(addr, status, numtx)
void LINK_NEIGHBOR_CALLBACK(const linkaddr_t *addr, int status, int numtx);
#else
#define LINK_NEIGHBOR_CALLBACK(addr, status, numtx)
#endif /* UIP_CONF_DS6_LINK_NEIGHBOR_CALLBACK */

/* Reject parents that have a higher path cost than the following. */
#define MAX_PATH_COST      32768   /* Eq path ETX of 256 */

NBR_TABLE_GLOBAL(uip_ds6_nbr_t, ds6_neighbors);

//FAN TPS 1v10:1003
//#define MIN_CAND_PARENT_THRESHOLD 120//180
/************ RAKA ** 18 July 2018 ************************/

// Raka chnaged to 40 from 120 since renesas is sending with RSSI 69 (-163 dBm )
#define MIN_CAND_PARENT_THRESHOLD -30

/************ RAKA ** 18 July 2018 ************************/

//1 = parent selection using RSL, 0 = parent selection using path cost
#define IN_HOUSE_TESTING 0
#define IN_HOUSE_TESTING_HOP 1

//typedef struct comp_rssi{
//	int val;
//	linkaddr_t addr;
//}comp_rssi_t;
//comp_rssi_t comp_arr[NBR_TABLE_MAX_NEIGHBORS];

uint8_t cold_boot_flag = 0x01;

extern uint8_t root_device;

#if(FAN_EAPOL_FEATURE_ENABLED == 1)
extern void get_eapol_parent_address(uint8_t *eapol_parent);
#endif

extern uint8_t* get_self_address(void);
extern void link_stats_rsl_receive(const linkaddr_t *lladdr, int32_t rssi);
extern void mem_rev_cpy(uint8_t* dest, uint8_t* src, uint16_t len );
extern void update_mac_routing_cost(uint16_t, uint8_t*);
extern void tcpip_post_event( l3_process_event_t ev, uint8_t* data);
extern uint8_t get_dev_mac_join5_status(void);
extern void rpl_dag_set_preferred_parent(rpl_dag_t *dag, rpl_parent_t *p);

void uip_ds6_nbr_update_rsl_ie(uint8_t* rcvd_addr, uint8_t* self_addr, int32_t rssi);
void quickSort(comp_rssi_t arr[], int low, int high);
uint8_t select_best_prefered_parent(void);
void clean_ds6_nbr_table();
uint8_t rsl_ie_receive_status(void);
uip_ds6_nbr_t* select_best_using_rssi();
uip_ds6_nbr_t* select_best_using_rsl_ie();
uip_ds6_nbr_t* select_best_using_path_cost();
rpl_rank_t rpl_rank_calculate();
uip_ds6_nbr_t *uip_get_prio_based_parent_nbr(uint8_t prio);
void choose_parent_set_among_nbr_set (void);
/*---------------------------------------------------------------------------*/
uip_ipaddr_t prfered_parent_linklocal_addr;
uip_ipaddr_t prfered_parent_global_addr;
/*---------------------------------------------------------------------------*/
void
uip_ds6_neighbors_init(void)
{
  link_stats_init();
  nbr_table_register(ds6_neighbors, (nbr_table_callback *)uip_ds6_nbr_rm);
}
/*---------------------------------------------------------------------------*/
uint8_t nbr_wait_time_flag = 0x01;//for starting wait timer once.
uip_ds6_nbr_t *
uip_ds6_nbr_add(const uip_ipaddr_t *ipaddr, const uip_lladdr_t *lladdr,
                uint8_t isrouter, uint8_t state, nbr_table_reason_t reason,
                void *data)
{
  uip_ipaddr_t link_loc_addr;
  uip_lladdr_t ll_add;
  
//  fan_nbr_table_t update_nbr;
  uip_ds6_nbr_t *nbr = nbr_table_add_lladdr(ds6_neighbors, (linkaddr_t*)lladdr
                                            , reason, data);
 
  if(nbr) {
    uip_ipaddr_copy(&nbr->ipaddr, ipaddr);
    if(reason != NBR_TABLE_REASON_RPL_DIO)
    {
        memcpy(&ll_add.addr,lladdr,8);
        uip_create_linklocal_prefix(&link_loc_addr);
        uip_ds6_set_addr_iid(&link_loc_addr, &ll_add);
        uip_ipaddr_copy(&nbr->global_addr,ipaddr);
        uip_ipaddr_copy(&nbr->ipaddr, &link_loc_addr);
//        memcpy (update_nbr.mac_addr, lladdr, 8);
//        memcpy (update_nbr.link_local_addr, &link_loc_addr, 16);
//        memcpy (update_nbr.global_addr, ipaddr, 16);
//        fan_mac_nbr_add (&update_nbr, NBR_TABLE_LINK_LOCAL_INDX | 
//                                      NBR_TABLE_GLOBAL_ADDR_INDX);
    }
	 if(lladdr != NULL) {
      memcpy(&nbr->lladdr, lladdr, UIP_LLADDR_LEN);
    } else {
      memset(&nbr->lladdr, 0, UIP_LLADDR_LEN);
    }
#if UIP_ND6_SEND_NA || UIP_ND6_SEND_RA || !UIP_CONF_ROUTER
    nbr->isrouter = isrouter;
#endif /* UIP_ND6_SEND_NA || UIP_ND6_SEND_RA || !UIP_CONF_ROUTER */
    nbr->state = state;
#if UIP_CONF_IPV6_QUEUE_PKT
    uip_packetqueue_new(&nbr->packethandle);
#endif /* UIP_CONF_IPV6_QUEUE_PKT */
#if UIP_ND6_SEND_NA
    if(nbr->state == NBR_REACHABLE) {
      l3_stimer_set(&nbr->reachable, UIP_ND6_REACHABLE_TIME / 1000);//no need,, check
    } else {
      /* We set the timer in expired state */
      l3_stimer_set(&nbr->reachable, 0);
    }
    //l3_stimer_set(&nbr->sendns, 0);//check, we are sending NS, in probing    
    
    nbr->nscount = 0;
#endif /* UIP_ND6_SEND_NA */
    PRINTF("Adding neighbor with ip addr ");
    PRINT6ADDR(ipaddr);
    PRINTF(" link addr ");
    PRINTLLADDR(lladdr);
    PRINTF(" state %u\n", state);
    NEIGHBOR_STATE_CHANGED(nbr);
    return nbr;
  } else {
    PRINTF("uip_ds6_nbr_add drop ip addr ");
    PRINT6ADDR(ipaddr);
    PRINTF(" link addr (%p) ", lladdr);
    PRINTLLADDR(lladdr);
    PRINTF(" state %u\n", state);
    return NULL;
  }
}
/*---------------------------------------------------------------------------*/
void get_linklocal_and_global_address_from_mac_address (uint8_t *mac_addr, uint8_t *link_local_addr, uint8_t *global_addr)
{
  uip_ds6_nbr_t* nbr = NULL;
  uint8_t mac_address[8] = {0};
  
  mem_rev_cpy (mac_address, mac_addr, 8);

  for(nbr = nbr_table_head(ds6_neighbors); nbr != NULL; nbr = nbr_table_next(ds6_neighbors, nbr))
  {
    if (!memcmp (nbr->lladdr.addr, mac_address, 8))
    {
      memcpy (link_local_addr, (uint8_t *)nbr->ipaddr.u8, 16);
      memcpy (global_addr, (uint8_t *)nbr->global_addr.u8, 16);
      return;
    }
  }
}
/*---------------------------------------------------------------------------*/
int
uip_ds6_nbr_rm(uip_ds6_nbr_t *nbr)
{
  if(nbr != NULL) {
#if UIP_CONF_IPV6_QUEUE_PKT
    uip_packetqueue_free(&nbr->packethandle);
#endif /* UIP_CONF_IPV6_QUEUE_PKT */
    NEIGHBOR_STATE_CHANGED(nbr);
    return nbr_table_remove(ds6_neighbors, nbr);
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
const uip_ipaddr_t *
uip_ds6_nbr_get_ipaddr(const uip_ds6_nbr_t *nbr)
{
  return (nbr != NULL) ? &nbr->ipaddr : NULL;
}
/*---------------------------------------------------------------------------*/
const uip_lladdr_t *
uip_ds6_nbr_get_ll(const uip_ds6_nbr_t *nbr)
{
  return (const uip_lladdr_t *)nbr_table_get_lladdr(ds6_neighbors, nbr);
}
/*---------------------------------------------------------------------------*/
int
uip_ds6_nbr_num(void)
{
  uip_ds6_nbr_t *nbr;
  int num;

  num = 0;
  for(nbr = nbr_table_head(ds6_neighbors);
      nbr != NULL;
      nbr = nbr_table_next(ds6_neighbors, nbr)) {
    num++;
  }
  return num;
}
/*---------------------------------------------------------------------------*/
uip_ds6_nbr_t *
uip_ds6_nbr_lookup(const uip_ipaddr_t *ipaddr)
{    
  uip_ds6_nbr_t *nbr = nbr_table_head(ds6_neighbors);
  if(ipaddr != NULL) {
    while(nbr != NULL) {
      if((uip_ipaddr_cmp(&nbr->ipaddr, ipaddr)) ||
         (uip_ipaddr_cmp(&nbr->global_addr, ipaddr))) {
        return nbr;
      }
      nbr = nbr_table_next(ds6_neighbors, nbr);
    }
  }
   return NULL;
 
}
/*---------------------------------------------------------------------------*/
uip_ds6_nbr_t *
uip_ds6_nbr_ll_lookup(const uip_lladdr_t *lladdr)
{
  return nbr_table_get_from_lladdr(ds6_neighbors, (linkaddr_t*)lladdr);
}
/*---------------------------------------------------------------------------*/
uip_ipaddr_t *
uip_ds6_nbr_ipaddr_from_lladdr(const uip_lladdr_t *lladdr)
{
  uip_ds6_nbr_t *nbr = uip_ds6_nbr_ll_lookup(lladdr);
  return nbr ? &nbr->ipaddr : NULL;
}
/*---------------------------------------------------------------------------*/
const uip_lladdr_t *
uip_ds6_nbr_lladdr_from_ipaddr(const uip_ipaddr_t *ipaddr)
{
  uip_ds6_nbr_t *nbr = uip_ds6_nbr_lookup(ipaddr);
  return nbr ? uip_ds6_nbr_get_ll(nbr) : NULL;
}
/*---------------------------------------------------------------------------*/
void
uip_ds6_link_neighbor_callback(int status, int numtx)
{
  const linkaddr_t *dest = packetbuf_addr(PACKETBUF_ADDR_RECEIVER);
  if(linkaddr_cmp(dest, &linkaddr_null)) {
    return;
  }

  /* Update neighbor link statistics */
  link_stats_packet_sent(dest, status, numtx);
  /* Call upper-layer callback (e.g. RPL) */
  LINK_NEIGHBOR_CALLBACK(dest, status, numtx);

#if UIP_DS6_LL_NUD
  /* From RFC4861, page 72, last paragraph of section 7.3.3:
   *
   *         "In some cases, link-specific information may indicate that a path to
   *         a neighbor has failed (e.g., the resetting of a virtual circuit). In
   *         such cases, link-specific information may be used to purge Neighbor
   *         Cache entries before the Neighbor Unreachability Detection would do
   *         so. However, link-specific information MUST NOT be used to confirm
   *         the reachability of a neighbor; such information does not provide
   *         end-to-end confirmation between neighboring IP layers."
   *
   * However, we assume that receiving a link layer ack ensures the delivery
   * of the transmitted packed to the IP stack of the neighbour. This is a
   * fair assumption and allows battery powered nodes save some battery by
   * not re-testing the state of a neighbour periodically if it
   * acknowledges link packets. */
  if(status == MAC_TX_OK) {
    uip_ds6_nbr_t *nbr;
    nbr = uip_ds6_nbr_ll_lookup((uip_lladdr_t *)dest);
    if(nbr != NULL && nbr->state != NBR_INCOMPLETE) {
      nbr->state = NBR_REACHABLE;
      l3_stimer_set(&nbr->reachable, UIP_ND6_REACHABLE_TIME / 1000);
      PRINTF("uip-ds6-neighbor : received a link layer ACK : ");
      PRINTLLADDR((uip_lladdr_t *)dest);
      PRINTF(" is reachable.\n");
    }
  }
#endif /* UIP_DS6_LL_NUD */

}
#if UIP_ND6_SEND_NA
/*---------------------------------------------------------------------------*/
/** Periodic processing on neighbors */
void
uip_ds6_neighbor_periodic(void)
{
  uip_ds6_nbr_t *nbr = nbr_table_head(ds6_neighbors); 
  
  while(nbr != NULL) 
  {
    switch(nbr->state) 
    {
    case NBR_REACHABLE:
      if(l3_stimer_expired(&nbr->reachable)) 
      {        
#if UIP_CONF_IPV6_RPL
        /* when a neighbor leave its REACHABLE state and is a default router,
           instead of going to STALE state it enters DELAY state in order to
           force a NUD on it. Otherwise, if there is no upward traffic, the
           node never knows if the default router is still reachable. This
           mimics the 6LoWPAN-ND behavior.
         */
        if(uip_ds6_defrt_lookup(&nbr->ipaddr) != NULL) {
          PRINTF("REACHABLE: defrt moving to DELAY (");
          PRINT6ADDR(&nbr->ipaddr);
          PRINTF(")\n");
          nbr->state = NBR_DELAY;
          l3_stimer_set(&nbr->reachable, UIP_ND6_DELAY_FIRST_PROBE_TIME);
          nbr->nscount = 0;
        } else {
          PRINTF("REACHABLE: moving to STALE (");
          PRINT6ADDR(&nbr->ipaddr);
          PRINTF(")\n");
          //nbr->state = NBR_STALE;//santosh,,not maintaining NBR STALE
        }
#else /* UIP_CONF_IPV6_RPL */
        PRINTF("REACHABLE: moving to STALE (");
        PRINT6ADDR(&nbr->ipaddr);
        PRINTF(")\n");
        nbr->state = NBR_STALE;
#endif /* UIP_CONF_IPV6_RPL */       
      }
      break;
      
    case NBR_INCOMPLETE:
      if(nbr->nscount >= UIP_ND6_MAX_MULTICAST_SOLICIT) {
        uip_ds6_nbr_rm(nbr);
      } else if(l3_stimer_expired(&nbr->sendns) && (uip_len == 0)) {
        nbr->nscount++;
        PRINTF("NBR_INCOMPLETE: NS %u\n", nbr->nscount);
        uip_nd6_ns_output(NULL, NULL, &nbr->ipaddr);
        l3_stimer_set(&nbr->sendns, uip_ds6_if.retrans_timer / 1000);        
      }
      break;
      
    case NBR_DELAY:
      if(l3_stimer_expired(&nbr->reachable)) {
        //nbr->state = NBR_PROBE;//santosh: currently not changing nbr state to probing from delay state
        nbr->nscount = 0;
        PRINTF("DELAY: moving to PROBE\n");
        l3_stimer_set(&nbr->sendns, 0);
      }
      break;
    
    /*maintaing this state while probing [after network establish, DAO-ACK 
      received, changing to probing state]*/  
    case NBR_PROBE:
#if !IN_HOUSE_TESTING_HOP      
      //nscount is incremented while sending NS (uip_nd6_ns_aro_output)
      //and is set to 0 while receiving ACK
      if(nbr->nscount >= UIP_ND6_MAX_UNICAST_SOLICIT/*3*/) {
        uip_ds6_defrt_t *locdefrt;
        PRINTF("PROBE END\n");
        if((locdefrt = uip_ds6_defrt_lookup(&nbr->ipaddr)) != NULL) {
          if (!locdefrt->isinfinite) {
            uip_ds6_defrt_rm(locdefrt);
          }
        }
        uip_ds6_nbr_rm(nbr);

        uip_ds6_nbr_t *l_nbr = uip_get_prio_based_parent_nbr(2);//2nd best preferred_parent        
        if(l_nbr == NULL)
          l_nbr = uip_get_prio_based_parent_nbr(3);//3rd best preferred_parent        
        
        if(l_nbr != NULL){
          l_nbr->parent_priority = 1;
          struct link_stats * nbr_stats = (struct link_stats *)link_stats_from_lladdr((linkaddr_t*)l_nbr->lladdr.addr);    
          /*updating mac_self_fan_info.pan_metrics.routing_cost for PA BS-IE*/
          //update_mac_routing_cost(nbr_stats->path_cost);
          
          rpl_dag_t *dag = rpl_get_dag(&l_nbr->ipaddr);
          rpl_parent_t* p = rpl_find_parent(dag, &l_nbr->ipaddr);  
          if (p != NULL)        //Debdeep
            dag->rank = (p->rank + 1);//RANK(self)//minHopRankInc = 128//dag->instance->min_hoprankinc/dag->instance->min_hoprankinc  

          if(!root_device)
            rpl_instance_t* instance = rpl_get_instance(instance_table[0].instance_id);
          else
            rpl_instance_t* instance = rpl_get_instance(RPL_DEFAULT_INSTANCE);
          //removes the previous default route and set a new default route
          if(rpl_set_default_route(instance, &l_nbr->ipaddr)){              
              tcpip_post_event( NBR_SEND_NS_ARO_EVENT, NULL );//with aro
          }      
        }   
        else{
          //send unicast DIS to each nbr whose rank is less than self rank
          //unicast dio will be received, calculate best parent, NS-ARO, then 
          //DAO
        }
      }
#endif      
      break;      
      
    default:
      break;
    }
    nbr = nbr_table_next(ds6_neighbors, nbr);
  }
}
/*---------------------------------------------------------------------------*/
#if UIP_ND6_SEND_NA
void
uip_ds6_nbr_refresh_reachable_state(const uip_ipaddr_t *ipaddr)
{
  uip_ds6_nbr_t *nbr;
  nbr = uip_ds6_nbr_lookup(ipaddr);
  if(nbr != NULL) {
    if(nbr->state != NBR_PROBE){
      nbr->state = NBR_REACHABLE;
      nbr->nscount = 0;//santosh
      //l3_stimer_set(&nbr->reachable, UIP_ND6_REACHABLE_TIME / 1000);
    }
  }
}
#endif /* UIP_ND6_SEND_NA */
/*---------------------------------------------------------------------------*/
uip_ds6_nbr_t *
uip_ds6_get_least_lifetime_neighbor(void)
{
  uip_ds6_nbr_t *nbr = nbr_table_head(ds6_neighbors);
  uip_ds6_nbr_t *nbr_expiring = NULL;
  while(nbr != NULL) {
    if(nbr_expiring != NULL) {
      clock_time_t curr = l3_stimer_remaining(&nbr->reachable);
      if(curr < l3_stimer_remaining(&nbr->reachable)) {
        nbr_expiring = nbr;
      }
    } else {
      nbr_expiring = nbr;
    }
    nbr = nbr_table_next(ds6_neighbors, nbr);
  }
  return nbr_expiring;
}
#endif /* UIP_ND6_SEND_NA */
/*---------------------------------------------------------------------------*/
void set_parent_status_in_mac_nbr_table (uint8_t *addr, uint8_t parent_status);
uint8_t select_best_prefered_parent(void)
{     
    uip_ipaddr_t target;
//    fan_nbr_table_t update_nbr;
    rpl_instance_t* instance;
    if(!root_device)
       instance = rpl_get_instance(instance_table[0].instance_id);
    else
       instance = rpl_get_instance(RPL_DEFAULT_INSTANCE);

    /*FAN TPS 1v20:999: node-to-neighbor(rsl-ie) and neighbor-to-node(rssi) RSL 
    EWMA must exceed CAND_PARENT_THRESHOLD*/ 
#if !IN_HOUSE_TESTING 
    //for calculating the path cost, selecting the best parent and updating it to the mac routing cost
    uip_ds6_nbr_t *best_nbr = select_best_using_path_cost();
#else
    //explicitly selecting the best parent for In-House Testing, using rsl_ie(ACK), rssi
    uip_ds6_nbr_t *best_nbr = select_best_using_rsl_ie();
    if(best_nbr == NULL)
      best_nbr = select_best_using_rssi();
#endif    

    if(best_nbr != NULL){
      memcpy(&target, &best_nbr->ipaddr, 16);
      memcpy(&prfered_parent_linklocal_addr, &best_nbr->ipaddr, 16);//temp, maintaing prefered parent,,santosh
      memcpy(&prfered_parent_global_addr,&best_nbr->global_addr,16);
//      memcpy (update_nbr.mac_addr, best_nbr->lladdr.addr, 8);
//      update_nbr.is_parent_status = NBR_PARENT_STATUS_PREFERRED_PARENT;
//      fan_mac_nbr_add (&update_nbr, NBR_TABLE_IS_PARENT_STATUS_INDX);
      set_parent_status_in_mac_nbr_table (best_nbr->lladdr.addr, 2);    //2 for PREFERRED_PARENT

#if !IN_HOUSE_TESTING       
      rpl_dag_t *best_dag = rpl_get_dag(&target);      
      best_dag->rank = rpl_rank_calculate();//calculalting and updating the rank
//      best_dag->rank = (rpl_rank_calculate()/(best_dag->instance->min_hoprankinc));
#endif      
            
      if(rpl_set_default_route(instance, &target) == 0)
        return 0;//fail to set default route
      return 1;//success
    }       
    cold_boot_flag = 0;
    return 0;//fail, no nbr is present
}
/*---------------------------------------------------------------------------*/
//rsl ie is received from ack
uint8_t rsl_ie_receive_status()
{
    linkaddr_t lladdr;
    //int16_t reset_rssi_val = 0x0000;
    //uint16_t reset_etx_val = 0x0000;
    uip_ds6_nbr_t *nbr = nbr_table_head(ds6_neighbors);
    while(nbr != NULL) {
      memcpy(lladdr.u8, nbr->lladdr.addr, 8);
      
      struct link_stats *nbr_stats = (struct link_stats *)link_stats_from_lladdr(&lladdr);      
      if(nbr_stats->rsl_ie == 0x00){//if ack is not received from a nbr
        nbr_stats->rssi = 0x0000;//reset_rssi_val;
        nbr_stats->etx = 0x0000;//reset_etx_val;
      }
      else{
        return 0;//at least one rsl ie is received
      }      
      nbr = nbr_table_next(ds6_neighbors, nbr);  
    }    
    return 1;//no rsl ie is received
}
/*---------------------------------------------------------------------------*/
void change_nbr_state_to_probe(uint8_t* p_addr)
{
  uip_ds6_nbr_t *l_nbr = uip_ds6_nbr_lookup((const uip_ipaddr_t *)p_addr);
  if(l_nbr != NULL){
    //l_nbr->state = NBR_PROBE;
    //l3_stimer_set(&l_nbr->sendns, 10);//60 sec
  }
  return;
}
/*----------------------------------------------------------------------------*/
void update_directlink_status(uint8_t *p_addr)
{
  uip_ds6_nbr_t *l_nbr = uip_ds6_nbr_lookup((const uip_ipaddr_t *)p_addr);
  if(l_nbr != NULL){
    l_nbr->d_link_status = 0x01;
  }
  return;
}
/*----------------------------------------------------------------------------*/
void start_nud_ns_timer (clock_time_t timeval);
void stop_nud_ns_timer (void);
uint8_t is_nud_ns_timer_running (void);
/* call back function of ctimer nud_ns_timer */
void send_ns_to_nbr (void *a)
{
  uint8_t status;
//  uint8_t eapol_parent_mac_addr[8] = {0};
  uint16_t rpl_nbr_number = 0;
    
  uip_ds6_nbr_t *nbr = nbr_table_head(ds6_neighbors);
  while(nbr != NULL){
    if(nbr->state == NBR_COLD_STARTUP/*6*/){
      /*count incremented while sending ns-output*/
      if(nbr->nscount == UIP_ND6_MAX_UNICAST_SOLICIT/*3*/){
        rpl_nbr_number = uip_ds6_nbr_num ();
//        if (rpl_nbr_number == 1)
//        {
//          get_eapol_parent_address (eapol_parent_mac_addr);
//          if (!memcmp (eapol_parent_mac_addr, nbr->lladdr.addr, 8))
//          {
//#if (PRINT_DEBUG_LEVEL==RUNTIME_LOG_FILE)
//            send_runtime_log ("NO ACK recvd for NS: trying to select EAPOL parent as DHCP parent "LOG_MAC_ADDRESS(nbr->lladdr.addr));
//#endif  
//            goto proceed;
//          }
//        }
        uip_ds6_nbr_rm(nbr);
        start_nud_ns_timer(NUD_NEXT_NS_TIMER);        
      }
      
      uint8_t* self_ieee_addr = get_self_address();
      uip_lladdr_t ll_add;
      uip_ipaddr_t src;
      uip_ipaddr_t dst;
      memcpy(ll_add.addr, self_ieee_addr, 8);
      uip_create_linklocal_prefix(&src);
      uip_ds6_set_addr_iid(&src, &ll_add);
      memcpy(&dst, &nbr->ipaddr, 16);
#if (PRINT_DEBUG_LEVEL == CONSOLE_ERROR_DEBUG)
      stack_print_debug ("Trig NS\n");
#endif
#if (PRINT_DEBUG_LEVEL==RUNTIME_LOG_FILE)
      send_runtime_log ("Sending NS to "LOG_MAC_ADDRESS(nbr->lladdr.addr));
#endif       
      uip_nd6_ns_aro_output(&src, &nbr->ipaddr, &nbr->ipaddr, 0, RPL_INFINITE_LIFETIME);//RPL_DEFAULT_LIFETIME //without aro
      start_nud_ns_timer(NUD_RETRY_NS_TIMER);
      return;
    }
    else if(nbr->state == NBR_PROBE)
    {
      uint8_t* self_ieee_addr = get_self_address();
      uip_lladdr_t ll_add;
      uip_ipaddr_t src;
      uip_ipaddr_t dst;
      memcpy(ll_add.addr, self_ieee_addr, 8);
      uip_create_linklocal_prefix(&src);
      uip_ds6_set_addr_iid(&src, &ll_add);
      memcpy(&dst, &nbr->ipaddr, 16);
#if (PRINT_DEBUG_LEVEL == CONSOLE_ERROR_DEBUG)
      stack_print_debug ("Trig NS\n");
#endif
#if (PRINT_DEBUG_LEVEL==RUNTIME_LOG_FILE)
      send_runtime_log ("Sending NS to "LOG_MAC_ADDRESS(nbr->lladdr.addr));
#endif       
      uip_nd6_ns_aro_output(&src, &nbr->ipaddr, &nbr->ipaddr, 1, RPL_INFINITE_LIFETIME);//RPL_DEFAULT_LIFETIME //without aro
      //uip_nd6_ns_output(NULL, NULL, &nbr->ipaddr);
      //nbr->state = NBR_DELAY;
      rpl_instance_t *instance = NULL;
      instance = rpl_get_instance(instance_table[0].instance_id);
      rpl_schedule_probing(instance);
      stop_nud_ns_timer ();
      return;
    }
    nbr = nbr_table_next(ds6_neighbors, nbr);
  }
  
proceed:
  stop_nud_ns_timer ();  
  
//  choose_parent_set_among_nbr_set ();
  
  status = select_best_prefered_parent();
  if(status){
#if ENABLE_DISABLE_DHCP_SERVICE
    l3_process_post_synch(&tcpip_process, EVENT_TO_DHCP_SOLICIT_REQUEST, NULL);
#else    
    set_global_address();
    printf("set_global_address\r\n");
    send_event_to_tcpip_process();
#endif    
      
  }
  else
  {
    //go to join state 
  }
  
  return;
}
/*----------------------------------------------------------------------------*/
uint8_t rsl_ie_data[9] = {0x00};
void uip_ds6_update_rsl_ie_event(uint8_t* rcvd_addr, uint8_t rssi)
{
  mem_rev_cpy(rsl_ie_data, rcvd_addr, 8);
  mem_rev_cpy(&rsl_ie_data[8], &rssi, 1);  
  tcpip_post_event(NBR_RSL_IE_ACK_EVENT, rsl_ie_data);
  return;
}
/*----------------------------------------------------------------------------*/
void uip_ds6_update_rsl_ie(uint8_t* data)
{  
  uint8_t rev_rcvd_addr[8] = {0};
  memcpy(rev_rcvd_addr, data, 8);
  uint8_t rssi = data[8];
  rpl_instance_t* instance;
  link_stats_rsl_receive((linkaddr_t *)rev_rcvd_addr, rssi);
  
  uip_ds6_nbr_t *l_nbr = uip_ds6_nbr_ll_lookup((uip_lladdr_t *)rev_rcvd_addr);
  if(!root_device)
    instance = rpl_get_instance(instance_table[0].instance_id);
  else
    instance = rpl_get_instance(RPL_DEFAULT_INSTANCE);
  if(l_nbr != NULL){
    l_nbr->state = NBR_REACHABLE;        
    l_nbr->nscount = 0;
    /*If there is no IP messaging exchanged between a node and neighbor for a 
    period of 30 seconds, the node MUST initiate NUD messaging to refresh its 
    neighbor link metrics.*/ 
    //for router device and after join state 5: probing
    if(!root_device && get_dev_mac_join5_status()){
      l3_ctimer_stop(&instance->probing_timer);
      l3_ctimer_restart(&instance->probing_timer);
    }
  }
  return;
}
/*----------------------------------------------------------------------------*/
/* Debdeep modified this function on 06-aug-2018 */
void uip_ds6_nbr_update_rsl_ie(uint8_t* rcvd_addr, uint8_t* self_addr, int32_t rssi)
{
  uint8_t rev_rcvd_addr[8] = {0};
  mem_rev_cpy(rev_rcvd_addr, rcvd_addr, 8);
  link_stats_rsl_receive((linkaddr_t *)rev_rcvd_addr, rssi);
  rpl_instance_t* instance;
  uip_ds6_nbr_t *l_nbr = uip_ds6_nbr_ll_lookup((uip_lladdr_t *)rev_rcvd_addr);
  if(!root_device)
    instance = rpl_get_instance(instance_table[0].instance_id);
  else
    instance = rpl_get_instance(RPL_DEFAULT_INSTANCE);
  if ((l_nbr != NULL) && (instance != NULL))
  {
    l_nbr->state = NBR_REACHABLE;        
    l_nbr->nscount = 0;
    if (is_nud_ns_timer_running())
    {
      stop_nud_ns_timer ();
      start_nud_ns_timer (NUD_NEXT_NS_TIMER);
    }
  }
  return;
}
/*----------------------------------------------------------------------------*/
uip_ds6_nbr_t* select_best_using_rsl_ie()
{
    uint8_t threshold_flag=0;
    uip_ds6_nbr_t* nbr = nbr_table_head(ds6_neighbors);//temp for looping
    rpl_dag_t *dag = rpl_get_dag(&nbr->ipaddr);//single dag
    rpl_parent_t* p = rpl_find_parent(dag, &nbr->ipaddr);    
    struct link_stats *nbr_stats = (struct link_stats *)link_stats_from_lladdr((linkaddr_t*)nbr->lladdr.addr);
    int32_t etx;  
    rpl_rank_t rank;    
    uint8_t i=0;
    comp_rssi_t comp_arr[NBR_TABLE_MAX_NEIGHBORS];
    memset(comp_arr, 0x00, sizeof(comp_arr));
    
    while(nbr != NULL) {      
      nbr_stats = (struct link_stats *)link_stats_from_lladdr((linkaddr_t*)nbr->lladdr.addr);
      comp_arr[i].val = nbr_stats->rsl_ie;
      memcpy((linkaddr_t*)&comp_arr[i].addr, (linkaddr_t*)&nbr->lladdr, 8);
      i++;
      //RSL EWMA both values must exceed the minimum threshold of CAND_PARENT_THRESHOLD//FAN TPS 1v10:999
      if(nbr_stats->rsl_ie >= MIN_CAND_PARENT_THRESHOLD)
        threshold_flag = 1;
      
      etx = nbr_stats->etx;            
      p = rpl_find_parent(dag, &nbr->ipaddr); 
      if (p != NULL)    //Debdeep
        rank = (p->rank)*(dag->instance->min_hoprankinc);//DAGRank(a)*minHopRankInc;       
      nbr_stats->path_cost = etx + rank;
      
      nbr = nbr_table_next(ds6_neighbors, nbr);  
    }
    
    //sorting the greatest element first...for RSSI
    quickSort(comp_arr, 0, i-1);
    for(i=0; i<3; i++){
      nbr = uip_ds6_nbr_ll_lookup((uip_lladdr_t *)&comp_arr[i].addr);
#if !IN_HOUSE_TESTING_HOP       
      nbr->parent_priority = i+1;
#endif      
    }

#if IN_HOUSE_TESTING_HOP
//    uip_lladdr_t l_addr = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};//for root
    uip_lladdr_t l_addr = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0x11, 0x11, 0x11};//r1
//    uip_lladdr_t l_addr = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0x22, 0x22, 0x22};//r2
//    uip_lladdr_t l_addr = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0x33, 0x33, 0x33};//r3  
//    uip_lladdr_t l_addr = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0x44, 0x44, 0x44};//r4 
//    uip_lladdr_t l_addr = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0x55, 0x55, 0x55};//r5 
//    uip_lladdr_t l_addr = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0x66, 0x66, 0x66};//r6 
//    uip_lladdr_t l_addr = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0x77, 0x77, 0x77};//r7 
//    uip_lladdr_t l_addr = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0x88, 0x88, 0x88};//r8
//    uip_lladdr_t l_addr = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0x99, 0x99, 0x99};//r9 
//    uip_lladdr_t l_addr = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xAA, 0xAA, 0xAA};//r10 
//    uip_lladdr_t l_addr = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xBB, 0xBB, 0xBB};//r11
//    uip_lladdr_t l_addr = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xCC, 0xCC, 0xCC};//r12
//    uip_lladdr_t l_addr = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xDD, 0xDD, 0xDD};//r13
//    uip_lladdr_t l_addr = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xEE, 0xEE, 0xEE};//r14    
    nbr = uip_ds6_nbr_ll_lookup(&l_addr);//best preferred_parent
    nbr->parent_priority = 1;
#else
    nbr = uip_get_prio_based_parent_nbr(1);//best preferred_parent
#endif
    
    nbr_stats = (struct link_stats *)link_stats_from_lladdr((linkaddr_t*)nbr->lladdr.addr);    
    /*updating mac_self_fan_info.pan_metrics.routing_cost for PA BS-IE*/
    //update_mac_routing_cost(nbr_stats->path_cost);

    p = rpl_find_parent(dag, &nbr->ipaddr); 
    if (p != NULL)    //Debdeep
      dag->rank = (p->rank + 1);//(dag->instance->min_hoprankinc/dag->instance->min_hoprankinc)//rfc6550#section-3.5.1
    
    if(threshold_flag)
      return nbr;
    return NULL;
}
/*----------------------------------------------------------------------------*/
uip_ds6_nbr_t* select_best_using_rssi()
{
    uip_ds6_nbr_t* nbr = nbr_table_head(ds6_neighbors);//temp for looping
    rpl_dag_t *dag = rpl_get_dag(&nbr->ipaddr);//single dag
    rpl_parent_t* p = rpl_find_parent(dag, &nbr->ipaddr);
    struct link_stats *nbr_stats = (struct link_stats *)link_stats_from_lladdr((linkaddr_t*)nbr->lladdr.addr);
    int32_t etx;  
    rpl_rank_t rank;    
    uint8_t i=0;
    comp_rssi_t comp_arr[NBR_TABLE_MAX_NEIGHBORS];
    memset(comp_arr, 0x00, sizeof(comp_arr));
    
    while(nbr != NULL) {
      nbr_stats = (struct link_stats *)link_stats_from_lladdr((linkaddr_t*)nbr->lladdr.addr);
      comp_arr[i].val = nbr_stats->rssi;
      memcpy((linkaddr_t*)&comp_arr[i].addr, (linkaddr_t*)&nbr->lladdr, 8); 
      i++;
      
      etx = nbr_stats->etx;            
      p = rpl_find_parent(dag, &nbr->ipaddr);   
      if (p != NULL)    //Debdeep
        rank = (p->rank)*(dag->instance->min_hoprankinc);//DAGRank(a)*minHopRankInc;       
      nbr_stats->path_cost = etx + rank;
      
      nbr = nbr_table_next(ds6_neighbors, nbr);  
    }
    
    //sorting the greatest element first...for RSSI
    quickSort(comp_arr, 0, i-1);
    for(i=0; i<3; i++){
      nbr = uip_ds6_nbr_ll_lookup((uip_lladdr_t *)&comp_arr[i].addr);
#if !IN_HOUSE_TESTING_HOP      
      nbr->parent_priority = i+1;
#endif      
    }  

#if IN_HOUSE_TESTING_HOP
//    uip_lladdr_t l_addr = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};//for root
    uip_lladdr_t l_addr = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0x11, 0x11, 0x11};//r1
//    uip_lladdr_t l_addr = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0x22, 0x22, 0x22};//r2
//    uip_lladdr_t l_addr = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0x33, 0x33, 0x33};//r3  
//    uip_lladdr_t l_addr = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0x44, 0x44, 0x44};//r4 
//    uip_lladdr_t l_addr = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0x55, 0x55, 0x55};//r5 
//    uip_lladdr_t l_addr = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0x66, 0x66, 0x66};//r6 
//    uip_lladdr_t l_addr = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0x77, 0x77, 0x77};//r7 
//    uip_lladdr_t l_addr = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0x88, 0x88, 0x88};//r8
//    uip_lladdr_t l_addr = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0x99, 0x99, 0x99};//r9 
//    uip_lladdr_t l_addr = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xAA, 0xAA, 0xAA};//r10 
//    uip_lladdr_t l_addr = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xBB, 0xBB, 0xBB};//r11
//    uip_lladdr_t l_addr = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xCC, 0xCC, 0xCC};//r12
//    uip_lladdr_t l_addr = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xDD, 0xDD, 0xDD};//r13
//    uip_lladdr_t l_addr = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xEE, 0xEE, 0xEE};//r14    
    nbr = uip_ds6_nbr_ll_lookup(&l_addr);//best preferred_parent
    nbr->parent_priority = 1;
#else
    nbr = uip_get_prio_based_parent_nbr(1);//best preferred_parent
#endif    
    nbr_stats = (struct link_stats *)link_stats_from_lladdr((linkaddr_t*)nbr->lladdr.addr);    
    /*updating mac_self_fan_info.pan_metrics.routing_cost for PA BS-IE*/
    //update_mac_routing_cost(nbr_stats->path_cost);
    
    p = rpl_find_parent(dag, &nbr->ipaddr); 
    //dag->rank = (p->rank + dag->instance->min_hoprankinc);//RANK(self)//minHopRankInc = 128
    if (p != NULL)    //Debdeep
      dag->rank = (p->rank + 1);//(dag->instance->min_hoprankinc/dag->instance->min_hoprankinc)//rfc6550#section-3.5.1
    //rpl_dag_set_preferred_parent(dag, p);
    return nbr;
}
/*----------------------------------------------------------------------------*/
uint8_t get_LQI_from_RSSI( int8_t rssi_val );
int8_t get_min_sense_rssi();
int8_t get_cand_parent_threshold ();
int8_t get_cand_parent_hysterysis();
uip_ds6_nbr_t* select_best_using_path_cost()
{
  uint8_t i=0;
  int rssi_best_range_low  = 0;
  int smallest_rpl_rank = 0;
  comp_rssi_t comp_path_cost_arr[NBR_TABLE_MAX_NEIGHBORS];
  comp_rssi_t comp_rssi_arr[NBR_TABLE_MAX_NEIGHBORS];
  comp_rssi_t comp_rpl_rank_arr[NBR_TABLE_MAX_NEIGHBORS];
  uint16_t path_cost;
  int32_t etx;  
  rpl_rank_t rank;
  
#if (PRINT_DEBUG_LEVEL==RUNTIME_LOG_FILE)
  send_runtime_log ("RPL Parent Selection");
#endif    
  
  //initialization
  uip_ds6_nbr_t* nbr = nbr_table_head(ds6_neighbors);//temp for looping    
  struct link_stats *nbr_stats = (struct link_stats *)link_stats_from_lladdr((linkaddr_t*)nbr->lladdr.addr);        
  rpl_dag_t *dag = rpl_get_dag(&nbr->ipaddr);//single dag
  rpl_parent_t* p = rpl_find_parent(dag, &nbr->ipaddr);
  memset(comp_path_cost_arr, 0x00, sizeof(comp_path_cost_arr));
  memset(comp_rssi_arr, 0x00, sizeof(comp_rssi_arr));
  memset(comp_rpl_rank_arr, 0x00, sizeof(comp_rpl_rank_arr));
  
  for (nbr = nbr_table_head(ds6_neighbors); nbr != NULL; nbr = nbr_table_next(ds6_neighbors, nbr))
  {
    nbr_stats = (struct link_stats *)link_stats_from_lladdr((linkaddr_t*)nbr->lladdr.addr);
    memcpy((linkaddr_t*)&comp_rssi_arr[i].addr, (linkaddr_t*)&nbr->lladdr, 8);
    comp_rssi_arr[i].val = nbr_stats->rssi;
#if (PRINT_DEBUG_LEVEL==RUNTIME_LOG_FILE)
    send_runtime_log ("NBR[%d] RSSI[%d]", i, nbr_stats->rssi);
    send_runtime_log ("--ADDR "LOG_MAC_ADDRESS(nbr->lladdr.addr));
#endif    
    i++;    
  }
  quickSort(comp_rssi_arr, 0, i-1);
  rssi_best_range_low = (comp_rssi_arr[i-1].val - APP_CFG_RSSI_BAND);  /* APP_CFG_RSSI_BAND = 10 dB */
  i = 0;
  
  for (nbr = nbr_table_head(ds6_neighbors); nbr != NULL; nbr = nbr_table_next(ds6_neighbors, nbr))
  {
    nbr_stats = (struct link_stats *)link_stats_from_lladdr((linkaddr_t*)nbr->lladdr.addr);
    p = rpl_find_parent(dag, &nbr->ipaddr);
    if ((p != NULL) && (nbr_stats->rssi >= rssi_best_range_low))
    {
      memcpy ((linkaddr_t*)&comp_rpl_rank_arr[i].addr, (linkaddr_t*)&nbr->lladdr, 8);
      comp_rpl_rank_arr[i].val = (int)p->rank;
#if (PRINT_DEBUG_LEVEL==RUNTIME_LOG_FILE)
      send_runtime_log ("NBR[%d] PAN Cost[%d]", i, (int)p->rank);
      send_runtime_log ("--ADDR "LOG_MAC_ADDRESS(nbr->lladdr.addr));
#endif      
    }
    else
    {
#if (PRINT_DEBUG_LEVEL==RUNTIME_LOG_FILE)
      send_runtime_log ("RSSI range Discard "LOG_MAC_ADDRESS(nbr->lladdr.addr));
#endif  
    }
    i++;
  }
  quickSort(comp_rpl_rank_arr, 0, i-1);
  smallest_rpl_rank = comp_rpl_rank_arr[0].val;
  i = 0;
  
  //updating the path cost for each neighbor
  for (nbr = nbr_table_head(ds6_neighbors); nbr != NULL; nbr = nbr_table_next(ds6_neighbors, nbr))
  {
    p = rpl_find_parent(dag, &nbr->ipaddr); 
    if (p == NULL)    //Debdeep
      continue;
    
    nbr_stats = (struct link_stats *)link_stats_from_lladdr((linkaddr_t*)nbr->lladdr.addr);
    if (nbr_stats == NULL)
      continue;
    
#if ((PRINT_DEBUG_LEVEL==ERROR_DEBUG)||(PRINT_DEBUG_LEVEL==CONSOLE_ERROR_DEBUG))
    stack_print_debug ("NBR parameter\r\n");
    stack_print_debug ("isrouter = %d, rssi = %d, rssi_best_range_low = %d, candidate rank = %d, self rank = %d, smallest_rpl_rank = %d path_cost = %d\r\n", nbr->isrouter, nbr_stats->rssi, rssi_best_range_low, p->rank, dag->rank, smallest_rpl_rank, nbr_stats->etx+p->rank);
#endif  
    
#if (PRINT_DEBUG_LEVEL==RUNTIME_LOG_FILE)
    send_runtime_log ("ADDR "LOG_MAC_ADDRESS(nbr->lladdr.addr));
    send_runtime_log ("--isrouter = %d, rssi = %d, rssi_best_range_low = %d, candidate rank = %d, self rank = %d, smallest_rpl_rank = %d path_cost = %d", nbr->isrouter, nbr_stats->rssi, rssi_best_range_low, p->rank, dag->rank, smallest_rpl_rank, nbr_stats->etx+p->rank);
#endif
    
    if(nbr->isrouter == 0x01)
    {
      rpl_remove_parent(p);
      nbr_table_remove (rpl_parents, p);
#if ((PRINT_DEBUG_LEVEL==ERROR_DEBUG)||(PRINT_DEBUG_LEVEL==CONSOLE_ERROR_DEBUG))
      stack_print_debug ("NBR Discard from parent set for isrouter\r\n");
#endif    
#if (PRINT_DEBUG_LEVEL==RUNTIME_LOG_FILE)
      send_runtime_log ("NBR Discard from parent set for isrouter");
#endif       
      continue;  //Suneet :: this is child device so continue in nbr table
    }            
    
    /* Debdeep :: Threshold values to choose parent candidate is done according to FAN TPS 1v28 */
    // RSL EWMA both values must exceed the minimum threshold of CAND_PARENT_THRESHOLD
    if((nbr_stats->rssi < get_LQI_from_RSSI(get_min_sense_rssi() + get_cand_parent_threshold () + get_cand_parent_hysterysis()))
       && (nbr_stats->rsl_ie < get_LQI_from_RSSI(get_min_sense_rssi() + get_cand_parent_threshold () + get_cand_parent_hysterysis())))
    {
      rpl_remove_parent(p);
      nbr_table_remove (rpl_parents, p);
#if ((PRINT_DEBUG_LEVEL==ERROR_DEBUG)||(PRINT_DEBUG_LEVEL==CONSOLE_ERROR_DEBUG))
      stack_print_debug ("NBR Discard from parent set for RSSI threshold\r\n");
#endif       
#if (PRINT_DEBUG_LEVEL==RUNTIME_LOG_FILE)
      send_runtime_log ("NBR Discard from parent set for RSSI threshold");
#endif        
      continue;//if not it should not participate in parent selection
    }
    
    /* Debdeep :: 08-march-2019 :: We make a 10 dB band, within which we will make RPL parent set */
    if (nbr_stats->rssi < rssi_best_range_low)
    {
      rpl_remove_parent(p);
      nbr_table_remove (rpl_parents, p);
#if ((PRINT_DEBUG_LEVEL==ERROR_DEBUG)||(PRINT_DEBUG_LEVEL==CONSOLE_ERROR_DEBUG))
      stack_print_debug ("NBR Discard from parent set for rssi_best_range_low\r\n");
#endif      
#if (PRINT_DEBUG_LEVEL==RUNTIME_LOG_FILE)
      send_runtime_log ("NBR Discard from parent set for rssi_best_range_low");
#endif        
      continue;
    }    
    
    etx = nbr_stats->etx;//etx metric
    
    if (p->rank > dag->rank)
    {
      rpl_remove_parent(p);
      nbr_table_remove (rpl_parents, p);
#if ((PRINT_DEBUG_LEVEL==ERROR_DEBUG)||(PRINT_DEBUG_LEVEL==CONSOLE_ERROR_DEBUG))
      stack_print_debug ("NBR Discard from parent set for higher rank\r\n");
#endif   
#if (PRINT_DEBUG_LEVEL==RUNTIME_LOG_FILE)
      send_runtime_log ("NBR Discard from parent set for higher rank");
#endif      
      continue;
    }
    
    if (smallest_rpl_rank != 0)
    {
      if (p->rank >= ((rpl_rank_t)smallest_rpl_rank + 2 * dag->instance->min_hoprankinc))
      {
        rpl_remove_parent(p);
        nbr_table_remove (rpl_parents, p);
#if ((PRINT_DEBUG_LEVEL==ERROR_DEBUG)||(PRINT_DEBUG_LEVEL==CONSOLE_ERROR_DEBUG))
        stack_print_debug ("NBR Discard from parent set for double rank distance\r\n");
#endif  
#if (PRINT_DEBUG_LEVEL==RUNTIME_LOG_FILE)
        send_runtime_log ("NBR Discard from parent set for double rank distance");
#endif         
        continue;
      }  
    }
    
    rank = p->rank;//rank of 1st parent 
    //      rank = (p->rank)*(dag->instance->min_hoprankinc);//DAGRank(a)*minHopRankInc
    
    //calculation of self path_cost against each nbr parent set
    path_cost = etx + rank;//MAX_PATH_COST = 32768, //path_cost: rfc6719#section-3.1
    nbr_stats->path_cost = path_cost;
    if (path_cost > MAX_PATH_COST)
    {
      rpl_remove_parent(p);
      nbr_table_remove (rpl_parents, p);
#if ((PRINT_DEBUG_LEVEL==ERROR_DEBUG)||(PRINT_DEBUG_LEVEL==CONSOLE_ERROR_DEBUG))
      stack_print_debug ("NBR Discard from parent set for MAX_PATH_COST\r\n");
#endif     
#if (PRINT_DEBUG_LEVEL==RUNTIME_LOG_FILE)
      send_runtime_log ("NBR Discard from parent set for MAX_PATH_COST");
#endif      
      continue;
    }
    
#if ((PRINT_DEBUG_LEVEL==ERROR_DEBUG)||(PRINT_DEBUG_LEVEL==CONSOLE_ERROR_DEBUG))
    stack_print_debug ("!!! New parent candidate found\r\n");
    stack_print_debug ("isrouter = %d, rssi = %d, rssi_best_range_low = %d, candidate rank = %d, self rank = %d, smallest_rpl_rank = %d path_cost = %d\r\n", nbr->isrouter, nbr_stats->rssi, rssi_best_range_low, p->rank, dag->rank, smallest_rpl_rank, nbr_stats->path_cost);
#endif    
#if (PRINT_DEBUG_LEVEL==RUNTIME_LOG_FILE)
    send_runtime_log ("New parent candidate found");
    send_runtime_log ("--ADDR "LOG_MAC_ADDRESS(nbr->lladdr.addr));
#endif      
    
    memcpy((linkaddr_t*)&comp_path_cost_arr[i].addr, (linkaddr_t*)&nbr->lladdr, 8);  
    comp_path_cost_arr[i].val = path_cost;
    i++;      
  }
  
  //sorting the smallest element first...
  quickSort(comp_path_cost_arr, 0, i-1);
  //selecting the best 3 parent, primary, secondary, tertiary
  for(i=0; i<3; i++){
    nbr = uip_ds6_nbr_ll_lookup((uip_lladdr_t *)&comp_path_cost_arr[i].addr);
    if(nbr == NULL)
      break;   
#if (PRINT_DEBUG_LEVEL==RUNTIME_LOG_FILE)
    send_runtime_log ("RPL Parent %d", i);
    send_runtime_log ("--ADDR "LOG_MAC_ADDRESS(nbr->lladdr.addr));
#endif        
    nbr->parent_priority = i+1;
  } 
  //smallest element is the best parent.
  nbr = uip_get_prio_based_parent_nbr(1);//returning the best parent, maintaining the order as 1, 2, 3...
  nbr_stats = (struct link_stats *)link_stats_from_lladdr((linkaddr_t*)nbr->lladdr.addr);
  
  /*updating mac_self_fan_info.pan_metrics.routing_cost for PA BS-IE*/
  //update_mac_routing_cost(nbr_stats->path_cost);
  
  return (nbr != NULL) ? nbr:NULL;
}
/*----------------------------------------------------------------------------*/
uip_ipaddr_t *uip_get_prio_based_parent_ip(uint8_t prio)
{
    uip_ds6_nbr_t* nbr = nbr_table_head(ds6_neighbors);        
    while(nbr != NULL) {
      if(nbr->parent_priority == prio)
        return &nbr->ipaddr;
      nbr = nbr_table_next(ds6_neighbors, nbr);  
    }  
    return NULL;
}
/*----------------------------------------------------------------------------*/
uip_ds6_nbr_t *uip_get_prio_based_parent_nbr(uint8_t prio)
{
    uip_ds6_nbr_t* nbr = nbr_table_head(ds6_neighbors);        
    while(nbr != NULL) {
      if(nbr->parent_priority == prio)
        return nbr;
      nbr = nbr_table_next(ds6_neighbors, nbr);  
    }  
    return NULL;
}
/*----------------------------------------------------------------------------*/
/*Suneet :: if device etx is high then remove the device in nbr table */
void device_remove_from_nbr(const uip_lladdr_t *lladdr)
{
  rpl_dag_t *dag = NULL;
  rpl_parent_t* p = NULL;
  uip_ds6_nbr_t *l_nbr = uip_ds6_nbr_ll_lookup (lladdr);
  
  dag = rpl_get_dag (&l_nbr->ipaddr);
  p = rpl_find_parent (dag, &l_nbr->ipaddr);
  
  if ((dag != NULL) && (p != NULL))
  {
    rpl_remove_parent (p);
    nbr_table_remove (rpl_parents, p);
  }
  l_nbr->parent_priority = 0;//resetting priority  
  uip_ds6_nbr_rm(l_nbr); 
}

/*Suneet :: check device is in preferd psrents det*/
uint8_t is_in_my_prefered_parent_set(const uip_lladdr_t *lladdr)
{
  uint8_t ii = 1;
  uip_ds6_nbr_t *l_nbr = NULL;
  l_nbr = (uip_ds6_nbr_t *)uip_get_prio_based_parent_nbr(ii++);
  while(l_nbr !=NULL )
  {
    if(!memcmp(&l_nbr->lladdr.addr,&lladdr->addr,8))
      return 1;
      l_nbr = (uip_ds6_nbr_t *)uip_get_prio_based_parent_nbr(ii++);
  }
  return 0;
}
/*----------------------------------------------------------------------------*/
rpl_rank_t rpl_rank_calculate()
{
    rpl_rank_t a;
    rpl_rank_t b;
    rpl_rank_t c;
    uip_ipaddr_t *target = uip_get_prio_based_parent_ip(1);//1st parent: preferred parent
    rpl_dag_t *dag = rpl_get_dag(target);//single dag
    rpl_rank_t minHopRankInc = dag->instance->min_hoprankinc;
    rpl_rank_t maxRankInc = dag->instance->max_rankinc;
    rpl_parent_t* p = rpl_find_parent(dag, target); 
    
#if (PRINT_DEBUG_LEVEL==RUNTIME_LOG_FILE)
    send_runtime_log ("Calculating Self Rank");
#endif       
    
    /*method 1:The Rank calculated for the path through the preferred parent.rfc6719#section-3.3*/
    
    /* Debdeep changed this on 24-aug-2018. Because In freq hopping ETX value 
       of the parent gets very high due to MAC retry. As a result the value of 'a' 
       becomes very high. */
//    a = rpl_rank_via_parent (p);
    a = p->rank + minHopRankInc;
    
#if (PRINT_DEBUG_LEVEL==ERROR_DEBUG)
    stack_print_debug ("Calculating rank a = %d\r\n", a);
#endif 
#if (PRINT_DEBUG_LEVEL==RUNTIME_LOG_FILE)
    send_runtime_log ("a = %d", a);
#endif       
       
    /*method 2:The Rank of the member of the parent set with the highest 
    advertised Rank, rounded to the next higher integral Rank, i.e., to 
    MinHopRankIncrease * (1 + floor(Rank/MinHopRankIncrease)).rfc6719#section-3.3*/

    uip_ds6_nbr_t* nbr = nbr_table_head(ds6_neighbors);//temp for looping      
    rpl_rank_t largest_rank = 0;//largest rank among the parent set
    
    while(nbr != NULL) {            
      p = rpl_find_parent(dag, &nbr->ipaddr);//single dag
      if (p == NULL)
      {
        nbr = nbr_table_next(ds6_neighbors, nbr);
        continue;
      }
      if(p->rank > largest_rank){
          largest_rank = p->rank;//largest rank among the parent set
      }
      nbr = nbr_table_next(ds6_neighbors, nbr);
    }
    
    b = (rpl_rank_t)(minHopRankInc*(1+floor(largest_rank/minHopRankInc)));//minHopRankInc = 128
    
#if (PRINT_DEBUG_LEVEL==ERROR_DEBUG)
    stack_print_debug ("Calculating rank largest rank = %d b = %d\r\n", largest_rank, b);
#endif     
#if (PRINT_DEBUG_LEVEL==RUNTIME_LOG_FILE)
    send_runtime_log ("b = %d Largest rank = %d", b, largest_rank);
#endif       
    
    /*method 3:The largest calculated Rank among paths through the parent set,
    minus MaxRankIncrease.rfc6719#section-3.3*/
    c = largest_rank - maxRankInc;//MaxRankIncrease = 0
    
#if (PRINT_DEBUG_LEVEL==ERROR_DEBUG)
    stack_print_debug ("Calculating rank c = %d\r\n", c);
#endif    
#if (PRINT_DEBUG_LEVEL==RUNTIME_LOG_FILE)
    send_runtime_log ("c = %d", c);
#endif     
        
    return ((a>b && a>c)?a : (b>c)?b : c);//MAX(a,b,c)    
}
/*----------------------------------------------------------------------------*/
// A utility function to swap two elements
void swap(comp_rssi_t* a, comp_rssi_t* b)
{
    comp_rssi_t t = *a;
    *a = *b;
    *b = t;
}
/*---------------------------------------------------------------------------*/ 
/* This function takes last element as pivot, places
   the pivot element at its correct position in sorted
    array, and places all smaller (smaller than pivot)
   to left of pivot and all greater elements to right
   of pivot */
int partition (comp_rssi_t arr[], int low, int high)
{
    int pivot = arr[high].val;// pivot
    int i = (low - 1);// Index of smaller element
 
    for (int j = low; j <= high- 1; j++)
    {
#if !IN_HOUSE_TESTING      
        // If current element is smaller than or equal to pivot, for PATH COST
        if (arr[j].val <= pivot)
#else
        // If current element is greater than or equal to pivot, for RSSI
        if (arr[j].val >= pivot)          
#endif          
        {
            i++;// increment index of smaller element
            swap(&arr[i], &arr[j]);
        }
    }
    swap(&arr[i + 1], &arr[high]);
    return (i + 1);
}
/*---------------------------------------------------------------------------*/ 
/* The main function that implements QuickSort
  arr[] --> Array to be sorted,
  low  --> Starting index,
  high  --> Ending index */
void quickSort(comp_rssi_t arr[], int low, int high)
{
    if (low < high)
    {
        /* pi is partitioning index, arr[p] is now at right place */
        int pi = partition(arr, low, high);
 
        // Separately sort elements before partition and after partition
        quickSort(arr, low, pi - 1);
        quickSort(arr, pi + 1, high);
    }
}
/*---------------------------------------------------------------------------*/ 
void clean_ds6_nbr_table()
{
  uip_ds6_nbr_t *nbr = nbr_table_head(ds6_neighbors);
  while(nbr != NULL)
  {
    uip_ds6_nbr_rm(nbr);
    nbr = nbr_table_next(ds6_neighbors, nbr);
  }
}
/*---------------------------------------------------------------------------*/ 
#if(APP_NVM_FEATURE_ENABLED == 1)
void add_ds6_nbr_from_nvm(uip_ds6_nbr_t *nbr)
{
  uip_ds6_nbr_t *l_nbr = nbr_table_add_lladdr(ds6_neighbors,(linkaddr_t*)&nbr->lladdr
                                              , NBR_TABLE_REASON_UNDEFINED, NULL);
  uip_ds6_nbr_t p_nbr = *nbr;
  memcpy(l_nbr,nbr,sizeof(p_nbr));
  NEIGHBOR_STATE_CHANGED(l_nbr);
  if(l_nbr->parent_priority == 1)
  set_parent_status_in_mac_nbr_table (l_nbr->lladdr.addr, 2);
}
#endif

/** @} */
