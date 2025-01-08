/*
 * Copyright (c) 2010, Swedish Institute of Computer Science.
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
 *         RPL timer management.
 *
 * \author Joakim Eriksson <joakime@sics.se>, Nicolas Tsiftes <nvt@sics.se>
 */

/**
 * \addtogroup uip6
 * @{
 */
#include "Stack6LoWPANConf.h"
   
#include "contiki-net.h"
#include "common_function.h"



#define NUD_PROBE_TIME          100       /* Seconds */ 
#define DEBUG DEBUG_NONE
#include "uip-debug.h"

/* A configurable function called after update of the RPL DIO interval */
#ifdef RPL_CALLBACK_NEW_DIO_INTERVAL
void RPL_CALLBACK_NEW_DIO_INTERVAL(uint8_t dio_interval);
#endif /* RPL_CALLBACK_NEW_DIO_INTERVAL */

#ifdef RPL_PROBING_SELECT_FUNC
rpl_parent_t *RPL_PROBING_SELECT_FUNC(rpl_dag_t *dag);
#endif /* RPL_PROBING_SELECT_FUNC */

#ifdef RPL_PROBING_DELAY_FUNC
clock_time_t RPL_PROBING_DELAY_FUNC(rpl_dag_t *dag);
#endif /* RPL_PROBING_DELAY_FUNC */

/*---------------------------------------------------------------------------*/
extern uint8_t* get_self_global_addr();
extern uip_ipaddr_t* uip_get_def_route_ipaddr();//getting the parent address from app
extern uip_ipaddr_t prfered_parent_linklocal_addr;
extern uip_ipaddr_t prfered_parent_global_addr; 
/*---------------------------------------------------------------------------*/
static l3_ctimer_t periodic_timer;
static l3_ctimer_t pan_timeout_timer;
void rpl_reset_probing_timer(rpl_instance_t *instance);
static void handle_periodic_timer(void *ptr);
static void new_dio_interval(rpl_instance_t *instance);
static void handle_dio_timer(void *ptr);
static void start_timer_for_pan_time_out (void);
static void stop_timer_for_pan_time_out (void);

static uint16_t next_dis;
static uint8_t pan_timeout_state;

void send_unicast_dis();
void check_new_parents_is_add();
void start_nud_ns_timer (clock_time_t timeval);
int fan_mac_nbr_num (void);
void get_fan_mac_nbr_lladr_by_index (int index, uint8_t *mac_addr);
uint8_t is_dis_sent (int index);
void mark_as_dis_sent (int index);
void kill_process_and_clean_rpl_nbr();
extern void quickSort(comp_rssi_t arr[], int low, int high);
void set_parent_status_in_mac_nbr_table (uint8_t *addr, uint8_t parent_status);
void remove_nbr_stats (struct link_stats *stats);
uint64_t get_time_now_64 (void);
void device_remove_from_nbr(const uip_lladdr_t *lladdr);
extern void tcpip_post_event (l3_process_event_t ev, uint8_t* data);
extern void mem_rev_cpy(uint8_t* dest, uint8_t* src, uint16_t len );
extern void del_key_and_device_from_mac_pib(uint8_t *device_addr);
extern void kill_process_and_clean_rpl_nbr();
rpl_rank_t rpl_rank_calculate (void);
/* dio_send_ok is true if the node is ready to send DIOs */
static uint8_t dio_send_ok;
static uint8_t send_dis_count = 0x00;   //Debdeep
static uint8_t send_unicast_dis_count = 0x00;   //Debdeep
#if(FAN_EAPOL_FEATURE_ENABLED == 1)
extern void get_eapol_parent_address(uint8_t *eapol_parent);
#endif
extern uint8_t check_probing_is_needed(uint8_t *device_ll_addr);
extern uip_ds6_nbr_t *uip_get_prio_based_parent_nbr(uint8_t prio);
extern void send_dao_tonext_parents();
extern uip_ds6_nbr_t* select_best_using_path_cost();
extern int get_join_state (void);
extern void trickle_timer_config_pa_send (void);
extern void trickle_timer_config_pc_send (void);
extern void process_schedule_end_pa (void);
extern void process_schedule_end_pc (void);
/*---------------------------------------------------------------------------*/
extern uint8_t root_device;
static void
handle_periodic_timer(void *ptr)
{
  uip_ds6_nbr_t* nbr;
  rpl_dag_t *dag = rpl_get_any_dag();
  unsigned int rpl_nbr_number, mac_nbr_number, diff_nbr_number;
  uint8_t mac_nbr_lladdr[8] = {0};
  
#if RPL_DIS_SEND
  if(!root_device)
  {
    if (dag == NULL)
    {
      if (send_dis_count < 5)
      {
        dis_output(NULL);
#if (PRINT_DEBUG_LEVEL==RUNTIME_LOG_FILE)
        send_runtime_log ("Sending Broadcast DIS");
#endif         
        send_dis_count++;  
        l3_ctimer_reset(&periodic_timer);
      }
      /* Debdeep added this block on 25-july-2018 */
      else if(send_unicast_dis_count < 3)
      {
#if(FAN_EAPOL_FEATURE_ENABLED == 1)
        get_eapol_parent_address (mac_nbr_lladdr);
#endif
        send_unicast_dis (mac_nbr_lladdr);
#if (PRINT_DEBUG_LEVEL==RUNTIME_LOG_FILE)
        send_runtime_log ("Sending Unicast DIS to EAPOL parent "LOG_MAC_ADDRESS(mac_nbr_lladdr));
#endif         
        send_unicast_dis_count++;
//        l3_ctimer_reset(&periodic_timer);
        l3_ctimer_set(&periodic_timer, 5*CLOCK_SECOND, handle_periodic_timer, NULL);
      }
      else
      {
        send_dis_count = 0;
        send_unicast_dis_count = 0;
        /* Go to join state 1 */
#if (PRINT_DEBUG_LEVEL==RUNTIME_LOG_FILE)
        send_runtime_log ("NO DIO RECVD :: Return to J1");
#endif         
        kill_process_and_clean_rpl_nbr ();
      }
    }
    else
    {
      rpl_nbr_number = uip_ds6_nbr_num ();
      mac_nbr_number = fan_mac_nbr_num ();
      
      if (mac_nbr_number >= rpl_nbr_number)
        diff_nbr_number = mac_nbr_number-rpl_nbr_number;
      else
        diff_nbr_number = 0;
      
      if (rpl_nbr_number >= mac_nbr_number)
      {
        send_dis_count = 0;
        send_unicast_dis_count = 0;
        start_nud_ns_timer (NUD_NEXT_NS_TIMER);      //success
      }
      else
      {
        int ii;
        
        for (ii = 0; ii < mac_nbr_number; ii++)
        {
          get_fan_mac_nbr_lladr_by_index (ii, mac_nbr_lladdr);
          for(nbr = nbr_table_head(ds6_neighbors); nbr != NULL; nbr = nbr_table_next(ds6_neighbors, nbr))
          {
            if (!memcmp (nbr->lladdr.addr, mac_nbr_lladdr, 8))
              break;
          }
          if (nbr == NULL)     //i.e, mac_nbr_lladdr is not present in ds6_neighbors
          {
            if (is_dis_sent(ii) > 0)   //checking whether DIS is already sent to that address or not
              continue;
            
            send_unicast_dis_count++;
            send_unicast_dis (mac_nbr_lladdr);
#if (PRINT_DEBUG_LEVEL==RUNTIME_LOG_FILE)
            send_runtime_log ("Sending Unicast DIS to "LOG_MAC_ADDRESS(mac_nbr_lladdr));
#endif             
            mark_as_dis_sent (ii);
            l3_ctimer_set(&periodic_timer, 2*CLOCK_SECOND, handle_periodic_timer, NULL);
            return;
          }
        }
        if (send_unicast_dis_count >= diff_nbr_number)
        {
          send_dis_count = 0;
          send_unicast_dis_count = 0;
          start_nud_ns_timer (NUD_NEXT_NS_TIMER);
          return;
        }
        l3_ctimer_set(&periodic_timer, 2*CLOCK_SECOND, handle_periodic_timer, NULL);
      }
    }
  }
#endif
}
/*---------------------------------------------------------------------------*/
void send_unicast_dis(uint8_t *mac_addr)
{
  uip_lladdr_t ll_add;
  uip_ipaddr_t dest_linklocal_addr;      
  memcpy(ll_add.addr,mac_addr,8);
  uip_create_linklocal_prefix(&dest_linklocal_addr);
  uip_ds6_set_addr_iid(&dest_linklocal_addr, &ll_add);
  dis_output(&dest_linklocal_addr);
}
/*---------------------------------------------------------------------------*/
static void
new_dio_interval(rpl_instance_t *instance)
{
  uint32_t time;
  clock_time_t ticks;

  /* TODO: too small timer intervals for many cases */
  time = 1UL << instance->dio_intcurrent;

  /* Convert from milliseconds to CLOCK_TICKS. */
  ticks = (time * CLOCK_SECOND) / 1000;
  instance->dio_next_delay = ticks;

  /* random number between I/2 and I */
  ticks = ticks / 2 + (ticks / 2 * (uint32_t)l3_random_rand()) / RANDOM_RAND_MAX;

  /*
   * The intervals must be equally long among the nodes for Trickle to
   * operate efficiently. Therefore we need to calculate the delay between
   * the randomized time and the start time of the next interval.
   */
  instance->dio_next_delay -= ticks;
  instance->dio_send = 1;

#if RPL_CONF_STATS
  /* keep some stats */
  instance->dio_totint++;
  instance->dio_totrecv += instance->dio_counter;
  ANNOTATE("#A rank=%u.%u(%u),stats=%d %d %d %d,color=%s\n",
	   DAG_RANK(instance->current_dag->rank, instance),
           (10 * (instance->current_dag->rank % instance->min_hoprankinc)) / instance->min_hoprankinc,
           instance->current_dag->version,
           instance->dio_totint, instance->dio_totsend,
           instance->dio_totrecv,instance->dio_intcurrent,
	   instance->current_dag->rank == ROOT_RANK(instance) ? "BLUE" : "ORANGE");
#endif /* RPL_CONF_STATS */

  /* reset the redundancy counter */
  instance->dio_counter = 0;

  /* schedule the timer */
  PRINTF("RPL: Scheduling DIO timer %lu ticks in future (Interval)\n", ticks);
  l3_ctimer_set(&instance->dio_timer, ticks, &handle_dio_timer, instance);

#ifdef RPL_CALLBACK_NEW_DIO_INTERVAL
  RPL_CALLBACK_NEW_DIO_INTERVAL(instance->dio_intcurrent);
#endif /* RPL_CALLBACK_NEW_DIO_INTERVAL */
}
/*---------------------------------------------------------------------------*/
// Raka added for Cisco testing [ 06-Dec-2017]
//void start_explicit_dio_timer ()
//{
//  
//  l3_ctimer_set(&instance->dio_timer, CLOCK_SECOND, &handle_dio_timer, instance);
//  
//}


static void
handle_dio_timer(void *ptr)
{
  rpl_instance_t *instance;

  instance = (rpl_instance_t *)ptr;    
  PRINTF("RPL: DIO Timer triggered\n");
  if(!dio_send_ok) {
    if(uip_ds6_get_link_local(ADDR_PREFERRED) != NULL) {
      dio_send_ok = 1;
    } else {
      PRINTF("RPL: Postponing DIO transmission since link local address is not ok\n");
      l3_ctimer_set(&instance->dio_timer, CLOCK_SECOND, &handle_dio_timer, instance);
      return;
    }
  }

  if(instance->dio_send) {
    /* send DIO if counter is less than desired redundancy */
    if(instance->dio_redundancy == 0 || instance->dio_counter < instance->dio_redundancy) {
#if RPL_CONF_STATS
      instance->dio_totsend++;
#endif /* RPL_CONF_STATS */
          dio_output(instance, NULL); 
      
    } else {
      PRINTF("RPL: Suppressing DIO transmission (%d >= %d)\n",
             instance->dio_counter, instance->dio_redundancy);
    }
    instance->dio_send = 0;
    PRINTF("RPL: Scheduling DIO timer %lu ticks in future (sent)\n",
           instance->dio_next_delay);
    l3_ctimer_set(&instance->dio_timer, instance->dio_next_delay, handle_dio_timer, instance);
  } else {
    /* check if we need to double interval */
    if(instance->dio_intcurrent < instance->dio_intmin + instance->dio_intdoubl) {
      instance->dio_intcurrent++;
      PRINTF("RPL: DIO Timer interval doubled %d\n", instance->dio_intcurrent);
    }
    new_dio_interval(instance);
  }

#if DEBUG
  rpl_print_neighbor_list();
#endif
}
/*---------------------------------------------------------------------------*/
void
rpl_reset_periodic_timer(void)
{
/* Debdeep increased this value to receive DIO from Cisco LBR */
  next_dis = 2;
//  next_dis = RPL_DIS_INTERVAL / 2 +
//    ((uint32_t)RPL_DIS_INTERVAL * (uint32_t)l3_random_rand()) / RANDOM_RAND_MAX -
//    RPL_DIS_START_DELAY;
  l3_ctimer_set(&periodic_timer, next_dis*CLOCK_SECOND, handle_periodic_timer, NULL);
}
/*---------------------------------------------------------------------------*/
/* Resets the DIO timer in the instance to its minimal interval. */
void
rpl_reset_dio_timer(rpl_instance_t *instance)
{
#if !RPL_LEAF_ONLY
  /* Do not reset if we are already on the minimum interval,
     unless forced to do so. */
  if(instance->dio_intcurrent > instance->dio_intmin) {
    instance->dio_counter = 0;
    instance->dio_intcurrent = instance->dio_intmin;
    new_dio_interval(instance);
  }
#if RPL_CONF_STATS
  rpl_stats.resets++;
#endif /* RPL_CONF_STATS */
#endif /* RPL_LEAF_ONLY */
  
  
  // Raka : modified when doing testing with Cisco 
  // our device does not send DIO  once received DAO-ACK
  
    instance->dio_counter = 0;
    instance->dio_intcurrent = instance->dio_intmin;
    new_dio_interval(instance);
  
}
/*---------------------------------------------------------------------------*/
static void handle_dao_timer(void *ptr);
static void
set_dao_lifetime_timer(rpl_instance_t *instance)
{
  if(rpl_get_mode() == RPL_MODE_FEATHER) {
    return;
  }

  /* Set up another DAO within half the expiration time, if such a
     time has been configured */
  if(instance->default_lifetime != RPL_INFINITE_LIFETIME) {
    clock_time_t expiration_time;
    expiration_time = (clock_time_t)instance->default_lifetime *
      (clock_time_t)instance->lifetime_unit *
      CLOCK_SECOND / 2;
    /* make the time for the re registration be betwen 1/2 - 3/4 of lifetime */
    expiration_time = expiration_time + (l3_random_rand() % (expiration_time / 2));
    PRINTF("RPL: Scheduling DAO lifetime timer %u ticks in the future\n",
           (unsigned)expiration_time);
    l3_ctimer_set(&instance->dao_lifetime_timer, expiration_time,
               handle_dao_timer, instance);
  }
}
/*---------------------------------------------------------------------------*/
static void
handle_dao_timer(void *ptr)
{
  rpl_instance_t *instance;
#if RPL_WITH_MULTICAST
  uip_mcast6_route_t *mcast_route;
  uint8_t i;
#endif

  instance = (rpl_instance_t *)ptr;

  if(!dio_send_ok && uip_ds6_get_link_local(ADDR_PREFERRED) == NULL) {
    PRINTF("RPL: Postpone DAO transmission\n");
    l3_ctimer_set(&instance->dao_timer, CLOCK_SECOND, handle_dao_timer, instance);
    return;
  }

  /* Send the DAO to the DAO parent set -- the preferred parent in our case. */
  if(instance->current_dag->preferred_parent != NULL) {
    PRINTF("RPL: handle_dao_timer - sending DAO\n");
    /* Set the route lifetime to the default value. */
    dao_output(instance->current_dag->preferred_parent, instance->default_lifetime);

#if RPL_WITH_MULTICAST
    /* Send DAOs for multicast prefixes only if the instance is in MOP 3 */
    if(instance->mop == RPL_MOP_STORING_MULTICAST) {
      /* Send a DAO for own multicast addresses */
      for(i = 0; i < UIP_DS6_MADDR_NB; i++) {
        if(uip_ds6_if.maddr_list[i].isused
            && uip_is_addr_mcast_global(&uip_ds6_if.maddr_list[i].ipaddr)) {
          dao_output_target(instance->current_dag->preferred_parent,
              &uip_ds6_if.maddr_list[i].ipaddr, RPL_MCAST_LIFETIME);
        }
      }

      /* Iterate over multicast routes and send DAOs */
      mcast_route = uip_mcast6_route_list_head();
      while(mcast_route != NULL) {
        /* Don't send if it's also our own address, done that already */
        if(uip_ds6_maddr_lookup(&mcast_route->group) == NULL) {
          dao_output_target(instance->current_dag->preferred_parent,
                     &mcast_route->group, RPL_MCAST_LIFETIME);
        }
        mcast_route = l3_list_item_next(mcast_route);
      }
    }
#endif
  } else {
    PRINTF("RPL: No suitable DAO parent\n");
  }

  l3_ctimer_stop(&instance->dao_timer);

  if(l3_etimer_expired((l3_etimer_t *)&instance->dao_lifetime_timer.ct.etimer)) {
    set_dao_lifetime_timer(instance);
  }
}
/*---------------------------------------------------------------------------*/
#if 0
static void
schedule_dao(rpl_instance_t *instance, clock_time_t latency)
{
  clock_time_t expiration_time;

  if(rpl_get_mode() == RPL_MODE_FEATHER) {
    return;
  }

  expiration_time = l3_ctimer_expiration_time(&instance->dao_timer.etimer);

  if(!l3_etimer_expired(&instance->dao_timer.etimer)) {
    PRINTF("RPL: DAO timer already scheduled\n");
  } else {
    if(latency != 0) {
      expiration_time = latency / 2 +
        (l3_random_rand() % (latency));
    } else {
      expiration_time = 0;
    }
    PRINTF("RPL: Scheduling DAO timer %u ticks in the future\n",
           (unsigned)expiration_time);
    l3_ctimer_set(&instance->dao_timer, expiration_time,
               handle_dao_timer, instance);

    set_dao_lifetime_timer(instance);
  }
}
#endif
/*---------------------------------------------------------------------------*/
void
rpl_schedule_dao(rpl_instance_t *instance)
{
  //schedule_dao(instance, RPL_DAO_DELAY);//commented for not sending dao
}
/*---------------------------------------------------------------------------*/
void
rpl_schedule_dao_immediately(rpl_instance_t *instance)
{
  //schedule_dao(instance, 0);//commented for not sending dao
}
/*---------------------------------------------------------------------------*/
void
rpl_cancel_dao(rpl_instance_t *instance)
{
  l3_ctimer_stop(&instance->dao_timer);
  l3_ctimer_stop(&instance->dao_lifetime_timer);
}
/*---------------------------------------------------------------------------*/
static void
handle_unicast_dio_timer(void *ptr)
{
  rpl_instance_t *instance = (rpl_instance_t *)ptr;
  uip_ipaddr_t *target_ipaddr = rpl_get_parent_ipaddr(instance->unicast_dio_target);

  if(target_ipaddr != NULL) {
    dio_output(instance, target_ipaddr);
  }
}
/*---------------------------------------------------------------------------*/
void
rpl_schedule_unicast_dio_immediately(rpl_instance_t *instance)
{
  l3_ctimer_set(&instance->unicast_dio_timer, 0,
                  handle_unicast_dio_timer, instance);
}
/*---------------------------------------------------------------------------*/
#if RPL_WITH_PROBING
clock_time_t
get_probing_delay(rpl_dag_t *dag)
{
  if(dag != NULL && dag->instance != NULL
      && dag->instance->urgent_probing_target != NULL) {
    /* Urgent probing needed (to find out if a neighbor may become preferred parent) */
    return l3_random_rand() % (CLOCK_SECOND * 10);
  } else {
    /* Else, use normal probing interval */
    return ((RPL_PROBING_INTERVAL) / 2) + l3_random_rand() % (RPL_PROBING_INTERVAL);
  }
}
/*---------------------------------------------------------------------------*/
rpl_parent_t *
get_probing_target(rpl_dag_t *dag)
{
  /* Returns the next probing target. The current implementation probes the urgent
   * probing target if any, or the preferred parent if its link statistics need refresh.
   * Otherwise, it picks at random between:
   * (1) selecting the best parent with non-fresh link statistics
   * (2) selecting the least recently updated parent
   */

  rpl_parent_t *p;
  rpl_parent_t *probing_target = NULL;
  rpl_rank_t probing_target_rank = INFINITE_RANK;
  clock_time_t probing_target_age = 0;
  clock_time_t clock_now = l3_clock_time();

  if(dag == NULL ||
      dag->instance == NULL) {
    return NULL;
  }

  /* There is an urgent probing target */
  if(dag->instance->urgent_probing_target != NULL) {
    return dag->instance->urgent_probing_target;
  }

  /* The preferred parent needs probing */
  if(dag->preferred_parent != NULL && !rpl_parent_is_fresh(dag->preferred_parent)) {
    return dag->preferred_parent;
  }

  /* With 50% probability: probe best non-fresh parent */
  if(l3_random_rand() % 2 == 0) {
    p = nbr_table_head(rpl_parents);
    while(p != NULL) {
      if(p->dag == dag && !rpl_parent_is_fresh(p)) {
        /* p is in our dag and needs probing */
        rpl_rank_t p_rank = rpl_rank_via_parent(p);
        if(probing_target == NULL
            || p_rank < probing_target_rank) {
          probing_target = p;
          probing_target_rank = p_rank;
        }
      }
      p = nbr_table_next(rpl_parents, p);
    }
  }

  /* If we still do not have a probing target: pick the least recently updated parent */
  if(probing_target == NULL) {
    p = nbr_table_head(rpl_parents);
    while(p != NULL) {
      const struct link_stats *stats =rpl_get_parent_link_stats(p);
      if(p->dag == dag && stats != NULL) {
        if(probing_target == NULL
            || clock_now - stats->last_tx_time > probing_target_age) {
          probing_target = p;
          probing_target_age = clock_now - stats->last_tx_time;
        }
      }
      p = nbr_table_next(rpl_parents, p);
    }
  }

  return probing_target;
}
/*---------------------------------------------------------------------------*/
#if 0
static void
handle_probing_timer(void *ptr)
{
  rpl_instance_t *instance = (rpl_instance_t *)ptr;
  rpl_parent_t *probing_target = RPL_PROBING_SELECT_FUNC(instance->current_dag);
  uip_ipaddr_t *target_ipaddr = rpl_get_parent_ipaddr(probing_target);
  /* Perform probing */
  if(target_ipaddr != NULL) {
    const struct link_stats *stats = rpl_get_parent_link_stats(probing_target);
    (void)stats;
    PRINTF("RPL: probing %u %s last tx %u min ago\n",
        rpl_get_parent_lladdr(probing_target)->u8[7],
        instance->urgent_probing_target != NULL ? "(urgent)" : "",
        probing_target != NULL ?
        (unsigned)((l3_clock_time() - stats->last_tx_time) / (60 * CLOCK_SECOND)) : 0
        );
    /* Send probe, e.g. unicast DIO or DIS */
    RPL_PROBING_SEND_FUNC(instance, target_ipaddr);//commented to send ns-unicast-packet.
    instance->urgent_probing_target = NULL;
  }

  /* Schedule next probing */
  //rpl_schedule_probing(instance);

#if DEBUG
  rpl_print_neighbor_list();
#endif
}
#endif
/*---------------------------------------------------------------------------*/
extern void get_self_extended_address_reverse(uint8_t *self_mac_addr);
static void
handle_ns_probing_timer(void *ptr)
{
  uint8_t ii = 1;
  rpl_instance_t *instance = (rpl_instance_t *)ptr;
  uip_ds6_nbr_t *l_nbr = NULL;
  //uip_ipaddr_t *target_ipaddr = uip_get_def_route_ipaddr();
#if 1  
  uip_ipaddr_t* src_ipaddr = (uip_ipaddr_t*)get_self_global_addr();//Suneet :: probing in global  address 
#else
  uint8_t self_mac[8] = {0x00};  //Suneet :: probing in link local address 
    get_self_extended_address_reverse(self_mac);
    uip_lladdr_t ll_add;
    uip_ipaddr_t self_ip_addr;      
    memcpy(ll_add.addr,self_mac,8);
    uip_create_linklocal_prefix(&self_ip_addr);
    uip_ds6_set_addr_iid(&self_ip_addr, &ll_add);
    uip_ipaddr_t* src_ipaddr = &self_ip_addr;//(uip_ipaddr_t*)get_self_global_addr(); 
#endif
  uint8_t aro = 0x01;/*true*///with aro
  uint16_t lifetime = 120;/*not sure need to check*/ 
  
  uip_ds6_nbr_t* nbr = nbr_table_head(ds6_neighbors);//temp for looping 
  if (nbr != NULL)
    check_new_parents_is_add();
  
  l_nbr = (uip_ds6_nbr_t *)uip_get_prio_based_parent_nbr(ii++);
  /*Suneet :: check when last packet is recived prob is needed or not*/
  while(l_nbr != NULL )
  {
    if(check_probing_is_needed((uint8_t *)&l_nbr->lladdr) != 1)
    {
      l_nbr = (uip_ds6_nbr_t *)uip_get_prio_based_parent_nbr(ii++);
      /* Schedule next probing */
      continue;
    }
    
    l_nbr->state = NBR_PROBE;
    
#if (PRINT_DEBUG_LEVEL==ERROR_DEBUG)
    stack_print_debug ("Sending NS with ARO for probing to ");
    print_mac_address ((uint8_t *)&l_nbr->lladdr);
#endif     
    
    //sending unicast-ns packet for probing without ARO 
    RPL_PROBING_SEND_NS_FUNC(src_ipaddr, &l_nbr->ipaddr, &l_nbr->ipaddr, aro, 
                             lifetime);
    instance->urgent_probing_target = NULL;
    l_nbr = (uip_ds6_nbr_t *)uip_get_prio_based_parent_nbr(ii++);
  }
  /* Schedule next probing */
  rpl_schedule_probing(instance);
}
/*---------------------------------------------------------------------------*/
void
rpl_schedule_probing(rpl_instance_t *instance)
{
  if (get_join_state () == 5)
    l3_ctimer_set (&instance->probing_timer, (NUD_PROBE_TIME*CLOCK_SECOND),
                handle_ns_probing_timer, instance);    
}

void rpl_cancel_probing_timer (void)
{
  l3_ctimer_stop (&instance_table[0].probing_timer);
}
#endif /* RPL_WITH_PROBING */

l3_ctimer_t nud_ns_timer;
static uint8_t nud_ns_timer_running = 0;
void send_ns_to_nbr (void *a);

void start_nud_ns_timer (clock_time_t timeval)      //Debdeep
{
  if(!root_device)//only for router and leaf node
  {
    l3_ctimer_set(&nud_ns_timer, timeval, send_ns_to_nbr, NULL);
    nud_ns_timer_running = 1;
  }
}

void stop_nud_ns_timer (void)
{
  l3_ctimer_stop (&nud_ns_timer);
  nud_ns_timer_running = 0;
}

uint8_t is_nud_ns_timer_running (void)
{
  return nud_ns_timer_running;
}

void change_preferred_parent (uip_ds6_nbr_t *primary_parent)
{
  rpl_dag_t *best_dag = NULL;
  uip_ipaddr_t target;
  rpl_instance_t *instance = NULL;
  
  memcpy (&target, &primary_parent->ipaddr, 16);
  memcpy (&prfered_parent_linklocal_addr, &primary_parent->ipaddr, 16);
  memcpy (&prfered_parent_global_addr, &primary_parent->global_addr, 16);
  
  set_parent_status_in_mac_nbr_table (primary_parent->lladdr.addr, 2);    //2 for PREFERRED_PARENT
  
  best_dag = rpl_get_dag (&target); 
  best_dag->rank = rpl_rank_calculate();
  
  instance = rpl_get_instance(instance_table[0].instance_id);
  if (rpl_set_default_route (instance, &target) == 0)
    return;//fail to set default route
  
#if (PRINT_DEBUG_LEVEL==ERROR_DEBUG)
  stack_print_debug ("Posting Event for NS with ARO to new parent ");
  print_mac_address (primary_parent->lladdr.addr);
#endif    
  
  tcpip_post_event( NBR_SEND_NS_ARO_EVENT, NULL );
}

/*Suneet :: if all parents is remove check new dio is recive or not and make new parents and send dao to new parnets*/
void check_new_parents_is_add()
{
  uip_ds6_nbr_t *primary_parent = NULL;
  uip_ds6_nbr_t *secondary_parent = NULL;
  uip_ds6_nbr_t *ternary_parent = NULL;
  struct link_stats *nbr_stats = NULL;
  uint8_t device_address[8] = {0};
  
  primary_parent = (uip_ds6_nbr_t *)uip_get_prio_based_parent_nbr (1);
  secondary_parent = (uip_ds6_nbr_t *)uip_get_prio_based_parent_nbr (2);
  ternary_parent = (uip_ds6_nbr_t *)uip_get_prio_based_parent_nbr (3);
  
  if (primary_parent != NULL)
  {
    nbr_stats = (struct link_stats *)link_stats_from_lladdr ((linkaddr_t*)primary_parent->lladdr.addr);
    if ((nbr_stats != NULL) && (nbr_stats->etx >= 384))
    {
#if ((PRINT_DEBUG_LEVEL==ERROR_DEBUG)||(PRINT_DEBUG_LEVEL==CONSOLE_ERROR_DEBUG))
      stack_print_debug ("Removing Primary parent ");
      print_mac_address (primary_parent->lladdr.addr);
#endif     
#if (PRINT_DEBUG_LEVEL==RUNTIME_LOG_FILE)
      send_runtime_log ("Removing Primary parent due to ETX "LOG_MAC_ADDRESS(primary_parent->lladdr.addr));
#endif          
      device_remove_from_nbr ((const uip_lladdr_t *)primary_parent->lladdr.addr);
      mem_rev_cpy (device_address, (uint8_t *)primary_parent->lladdr.addr, 8);
      del_key_and_device_from_mac_pib (device_address);
      remove_nbr_stats (nbr_stats);
      nbr_stats = NULL;
      primary_parent = NULL;
    }
  }
  
  if (secondary_parent != NULL)
  {
    nbr_stats = (struct link_stats *)link_stats_from_lladdr ((linkaddr_t*)secondary_parent->lladdr.addr);
    if ((nbr_stats != NULL) && (nbr_stats->etx >= 384))
    {
#if ((PRINT_DEBUG_LEVEL==ERROR_DEBUG)||(PRINT_DEBUG_LEVEL==CONSOLE_ERROR_DEBUG))
      stack_print_debug ("Removing Secondary parent ");
      print_mac_address (secondary_parent->lladdr.addr);
#endif     
#if (PRINT_DEBUG_LEVEL==RUNTIME_LOG_FILE)
      send_runtime_log ("Removing Secondary parent due to ETX "LOG_MAC_ADDRESS(secondary_parent->lladdr.addr));
#endif      
      device_remove_from_nbr ((const uip_lladdr_t *)secondary_parent->lladdr.addr);
      mem_rev_cpy (device_address, (uint8_t *)secondary_parent->lladdr.addr, 8);
      del_key_and_device_from_mac_pib(device_address);
      remove_nbr_stats (nbr_stats);
      nbr_stats = NULL;
      secondary_parent = NULL;
    }
  }
  
  if (ternary_parent != NULL)
  {
    nbr_stats = (struct link_stats *)link_stats_from_lladdr ((linkaddr_t*)ternary_parent->lladdr.addr);
    if ((nbr_stats != NULL) && (nbr_stats->etx >= 384))
    {
#if ((PRINT_DEBUG_LEVEL==ERROR_DEBUG)||(PRINT_DEBUG_LEVEL==CONSOLE_ERROR_DEBUG))
      stack_print_debug ("Removing Ternaery parent ");
      print_mac_address (ternary_parent->lladdr.addr);
#endif       
#if (PRINT_DEBUG_LEVEL==RUNTIME_LOG_FILE)
      send_runtime_log ("Removing Ternary parent due to ETX "LOG_MAC_ADDRESS(ternary_parent->lladdr.addr));
#endif      
      device_remove_from_nbr ((const uip_lladdr_t *)ternary_parent->lladdr.addr);
      mem_rev_cpy (device_address, (uint8_t *)ternary_parent->lladdr.addr, 8);
      del_key_and_device_from_mac_pib(device_address);
      remove_nbr_stats (nbr_stats);
      nbr_stats = NULL;
      ternary_parent = NULL;
    }
  }
  
  if (primary_parent == NULL)
  {
#if ((PRINT_DEBUG_LEVEL==ERROR_DEBUG)||(PRINT_DEBUG_LEVEL==CONSOLE_ERROR_DEBUG))
    stack_print_debug ("Searching for new parent from NBR set\r\n");
#endif          
    primary_parent = select_best_using_path_cost(); //again make prefered parents
    if (primary_parent != NULL)
    {
#if ((PRINT_DEBUG_LEVEL==ERROR_DEBUG)||(PRINT_DEBUG_LEVEL==CONSOLE_ERROR_DEBUG))
      stack_print_debug ("Got preffered parent ");
      print_mac_address (primary_parent->lladdr.addr);
#endif       
#if (PRINT_DEBUG_LEVEL==RUNTIME_LOG_FILE)
      send_runtime_log ("GOT Primary parent "LOG_MAC_ADDRESS(primary_parent->lladdr.addr));
#endif           
      /* Stop PAN Timeout timer */
      stop_timer_for_pan_time_out ();
      change_preferred_parent (primary_parent);
    }
    else
    {
      if (pan_timeout_state == 0)
      {
#if ((PRINT_DEBUG_LEVEL==ERROR_DEBUG)||(PRINT_DEBUG_LEVEL==CONSOLE_ERROR_DEBUG))
        stack_print_debug ("No Parent set found :: starting PAN timeout timer - %lld\r\n", get_time_now_64());
#endif
#if (PRINT_DEBUG_LEVEL==RUNTIME_LOG_FILE)
        send_runtime_log ("No Parent set found :: starting PAN timeout timer");
#endif         
        /* Start PAN Timeout timer */
        start_timer_for_pan_time_out ();
      }
    }
  }
  else
  {
    uint8_t new_parent_candidate_found = 0;
    uip_ds6_nbr_t *nbr = nbr_table_head(ds6_neighbors);
    rpl_parent_t *p = NULL;
    struct link_stats *nbr_stats = NULL;
    rpl_dag_t *dag = rpl_get_dag(&nbr->ipaddr);
    comp_rssi_t comp_rssi_arr[NBR_TABLE_MAX_NEIGHBORS];
    comp_rssi_t comp_rpl_rank_arr[NBR_TABLE_MAX_NEIGHBORS];
    memset(comp_rssi_arr, 0x00, sizeof(comp_rssi_arr));
    memset(comp_rpl_rank_arr, 0x00, sizeof(comp_rpl_rank_arr));
    uint8_t i = 0;
    int rssi_best_range_low = 0;
    int smallest_rpl_rank = 0;
    
    for (nbr = nbr_table_head(ds6_neighbors); nbr != NULL; nbr = nbr_table_next(ds6_neighbors, nbr))
    {
      nbr_stats = (struct link_stats *)link_stats_from_lladdr((linkaddr_t*)nbr->lladdr.addr);
      memcpy((linkaddr_t*)&comp_rssi_arr[i].addr, (linkaddr_t*)&nbr->lladdr, 8);
      comp_rssi_arr[i].val = nbr_stats->rssi;
      i++;
    }
    quickSort(comp_rssi_arr, 0, i-1);
    rssi_best_range_low  = (comp_rssi_arr[i-1].val - APP_CFG_RSSI_BAND);  /* APP_CFG_RSSI_BAND = 10 dB */
    i = 0;
    
    for (nbr = nbr_table_head(ds6_neighbors); nbr != NULL; nbr = nbr_table_next(ds6_neighbors, nbr))
    {
      nbr_stats = (struct link_stats *)link_stats_from_lladdr((linkaddr_t*)nbr->lladdr.addr);
      p = rpl_find_parent(dag, &nbr->ipaddr);
      if ((p != NULL) && (nbr_stats->rssi >= rssi_best_range_low))
      {
        memcpy ((linkaddr_t*)&comp_rpl_rank_arr[i].addr, (linkaddr_t*)&nbr->lladdr, 8);
        comp_rpl_rank_arr[i].val = (int)p->rank;
      }
      i++;
    }
    quickSort(comp_rpl_rank_arr, 0, i-1);
    smallest_rpl_rank = comp_rpl_rank_arr[0].val;
    i = 0;
    
    for (nbr = nbr_table_head(ds6_neighbors); nbr != NULL; nbr = nbr_table_next(ds6_neighbors, nbr))
    {
      if ((nbr->isrouter == 0x01) && (p->rank < dag->rank))
        nbr->isrouter = 0;
      
      if(nbr->isrouter == 0x01)
        continue;
      
      nbr_stats = (struct link_stats *)link_stats_from_lladdr((linkaddr_t*)nbr->lladdr.addr); 
      if (nbr_stats->rssi < rssi_best_range_low)
        continue;
      
      p = rpl_find_parent(dag, &nbr->ipaddr); 
      if (p == NULL)
        continue;
      
      if (p->rank > dag->rank)
        continue;
      
      if ((primary_parent != NULL) && (!memcmp (&nbr->ipaddr, &primary_parent->ipaddr, 16)))
        continue;      
      
      if ((secondary_parent != NULL) && (!memcmp (&nbr->ipaddr, &secondary_parent->ipaddr, 16)))
        continue;
      
      if ((ternary_parent != NULL) && (!memcmp (&nbr->ipaddr, &ternary_parent->ipaddr, 16)))
        continue;
      
      if (p->rank >= ((rpl_rank_t)smallest_rpl_rank + 2 * dag->instance->min_hoprankinc))
        continue;
      
      if (ternary_parent != NULL)
      {
        rpl_parent_t *ternary_rpl_parent = NULL;
        ternary_rpl_parent = rpl_find_parent(dag, &ternary_parent->ipaddr);
        if (ternary_rpl_parent == NULL)
          continue;
        if (ternary_rpl_parent->rank >= p->rank)
          continue;
#if ((PRINT_DEBUG_LEVEL==ERROR_DEBUG)||(PRINT_DEBUG_LEVEL==CONSOLE_ERROR_DEBUG))
        stack_print_debug ("Ternary parent rank = %d\r\n", ternary_rpl_parent->rank);
        stack_print_debug ("New parent candidate rank = %d\r\n", p->rank);
#endif         
      }
      else
        goto new_candidate_found;
      
      if (secondary_parent != NULL)
      {
        rpl_parent_t *secondary_rpl_parent = NULL;
        secondary_rpl_parent = rpl_find_parent(dag, &secondary_parent->ipaddr);
        if (secondary_rpl_parent == NULL)
          continue;
        if (secondary_rpl_parent->rank >= p->rank)
          continue;
#if ((PRINT_DEBUG_LEVEL==ERROR_DEBUG)||(PRINT_DEBUG_LEVEL==CONSOLE_ERROR_DEBUG))
        stack_print_debug ("Secondary parent rank = %d\r\n", secondary_rpl_parent->rank);
        stack_print_debug ("New parent candidate rank = %d\r\n", p->rank);
#endif        
      }
      else
        goto new_candidate_found;
      
      if (primary_parent != NULL)
      {
        rpl_parent_t *primary_rpl_parent = NULL;
        primary_rpl_parent = rpl_find_parent(dag, &primary_parent->ipaddr);
        if (primary_rpl_parent == NULL)
          continue;
        if (primary_rpl_parent->rank >= p->rank)
          continue;
#if ((PRINT_DEBUG_LEVEL==ERROR_DEBUG)||(PRINT_DEBUG_LEVEL==CONSOLE_ERROR_DEBUG))
        stack_print_debug ("Primary parent rank = %d\r\n", primary_rpl_parent->rank);
        stack_print_debug ("New parent candidate rank = %d\r\n", p->rank);
#endif           
      }
      
#if ((PRINT_DEBUG_LEVEL==ERROR_DEBUG)||(PRINT_DEBUG_LEVEL==CONSOLE_ERROR_DEBUG))
      stack_print_debug ("!!! New parent candidate found\r\n");
      stack_print_debug ("isrouter = %d, rssi = %d, rssi_best_range_low = %d, candidate rank = %d, self rank = %d, smallest_rpl_rank = %d\r\n", nbr->isrouter, nbr_stats->rssi, rssi_best_range_low, p->rank, dag->rank, smallest_rpl_rank);
#endif       
#if (PRINT_DEBUG_LEVEL==RUNTIME_LOG_FILE)
      send_runtime_log ("New parent candidate found");
      send_runtime_log ("--ADDR "LOG_MAC_ADDRESS(nbr->lladdr.addr));
      send_runtime_log ("--isrouter = %d, rssi = %d, rssi_best_range_low = %d, candidate rank = %d, self rank = %d, smallest_rpl_rank = %d path_cost = %d", nbr->isrouter, nbr_stats->rssi, rssi_best_range_low, p->rank, dag->rank, smallest_rpl_rank, nbr_stats->path_cost);
#endif       
 
    new_candidate_found:
      new_parent_candidate_found = 1;
    }
    
    if (new_parent_candidate_found == 1)
    {
      primary_parent = select_best_using_path_cost(); //again make prefered parents
      if (primary_parent != NULL)
        change_preferred_parent (primary_parent);
    }
  }
}

static void handle_pan_timeout(void *ptr)
{
#if ((PRINT_DEBUG_LEVEL==ERROR_DEBUG)||(PRINT_DEBUG_LEVEL==CONSOLE_ERROR_DEBUG))
  stack_print_debug ("Pan timeout expired :: Going to J1 - %lld\r\n", get_time_now_64());
#endif    
  kill_process_and_clean_rpl_nbr ();
}

static void start_timer_for_pan_time_out (void)
{
  process_schedule_end_pc ();
  process_schedule_end_pa ();
  l3_ctimer_stop (&instance_table[0].dio_timer);
  l3_ctimer_set (&pan_timeout_timer, APP_CFG_PAN_TIMEOUT*CLOCK_SECOND, handle_pan_timeout, NULL);
  pan_timeout_state = 1;
}

static void stop_timer_for_pan_time_out (void)
{
  if (pan_timeout_state == 1)
  {
    trickle_timer_config_pa_send();          
    trickle_timer_config_pc_send();
    rpl_reset_dio_timer (&instance_table[0]);
    l3_ctimer_stop (&pan_timeout_timer);
    pan_timeout_state = 0;
  }
}

uint8_t is_in_pantimeout (void)
{
  return pan_timeout_state;
}

void reset_pan_timeout_state (void)
{
  pan_timeout_state = 0;
}

/** @}*/
