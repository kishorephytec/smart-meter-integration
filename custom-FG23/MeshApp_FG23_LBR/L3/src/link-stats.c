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
#include "Stack6LoWPANConf.h"
#include "contiki-net.h"

#include <stdio.h>

#define DEBUG 0
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

void send_dao_tonext_parents();
//uip_ds6_nbr_t* select_best_using_path_cost();
//void device_remove_from_nbr(const uip_lladdr_t *lladdr);
//uint8_t is_in_my_prefered_parent_set(const uip_lladdr_t *lladdr);
extern void del_key_and_device_from_mac_pib(uint8_t *device_addr);
extern void mem_rev_cpy(uint8_t* dest, uint8_t* src, uint16_t len );
extern void kill_process_and_clean_rpl_nbr();
/* Half time for the freshness counter, in minutes */
#define FRESHNESS_HALF_LIFE             20
/* Statistics are fresh if the freshness counter is FRESHNESS_TARGET or more */
#define FRESHNESS_TARGET                 4
/* Maximum value for the freshness counter */
#define FRESHNESS_MAX                   16
/* Statistics with no update in FRESHNESS_EXPIRATION_TIMEOUT is not fresh */
#define FRESHNESS_EXPIRATION_TIME       (10 * 60 * (clock_time_t)CLOCK_SECOND)

/* EWMA (exponential moving average) used to maintain statistics over time */
#define EWMA_SCALE            100
#define EWMA_ALPHA             12       //Debdeep:: 15
#define EWMA_BOOTSTRAP_ALPHA   30

/* ETX fixed point divisor. 128 is the value used by RPL (RFC 6551 and RFC 6719) */
#define ETX_DIVISOR     LINK_STATS_ETX_DIVISOR
/* Number of Tx used to update the ETX EWMA in case of no-ACK */
#define ETX_NOACK_PENALTY                   10
/* Initial ETX value */
#define ETX_INIT                             2

/* Per-neighbor link statistics table */
NBR_TABLE(struct link_stats, link_stats);

/* Called every FRESHNESS_HALF_LIFE minutes */
l3_ctimer_t periodic_timer;

/* Used to initialize ETX before any transmission occurs. In order to
 * infer the initial ETX from the RSSI of previously received packets, use: */
/* #define LINK_STATS_CONF_INIT_ETX(stats) guess_etx_from_rssi(stats) */

#ifdef LINK_STATS_CONF_INIT_ETX
#define LINK_STATS_INIT_ETX(stats) LINK_STATS_CONF_INIT_ETX(stats)
#else /* LINK_STATS_INIT_ETX */
#define LINK_STATS_INIT_ETX(stats) (ETX_INIT/*2*/ * ETX_DIVISOR/*128*/)
#endif /* LINK_STATS_INIT_ETX */

/*---------------------------------------------------------------------------*/
/* Returns the neighbor's link stats */
const struct link_stats *
link_stats_from_lladdr(const linkaddr_t *lladdr)
{
  return nbr_table_get_from_lladdr(link_stats, lladdr);
}
/*---------------------------------------------------------------------------*/
/* Are the statistics fresh? */
int
link_stats_is_fresh(const struct link_stats *stats)
{
  return (stats != NULL)
      && l3_clock_time() - stats->last_tx_time < FRESHNESS_EXPIRATION_TIME
      && stats->freshness >= FRESHNESS_TARGET;
}
/*---------------------------------------------------------------------------*/
uint16_t
guess_etx_from_rssi(const struct link_stats *stats)
{
  if(stats != NULL) {
    if(stats->rssi == 0) {
      return ETX_INIT * ETX_DIVISOR;
    } else {
      /* A rough estimate of PRR from RSSI, as a linear function where:
       *      RSSI >= -60 results in PRR of 1
       *      RSSI <= -90 results in PRR of 0
       * prr = (bounded_rssi - RSSI_LOW) / (RSSI_DIFF)
       * etx = ETX_DIVOSOR / ((bounded_rssi - RSSI_LOW) / RSSI_DIFF)
       * etx = (RSSI_DIFF * ETX_DIVOSOR) / (bounded_rssi - RSSI_LOW)
       * */
#define ETX_INIT_MAX 3
#define RSSI_HIGH -60
#define RSSI_LOW  -90
#define RSSI_DIFF (RSSI_HIGH - RSSI_LOW)
      uint16_t etx;
      int16_t bounded_rssi = stats->rssi;
      bounded_rssi = MIN(bounded_rssi, RSSI_HIGH);
      bounded_rssi = MAX(bounded_rssi, RSSI_LOW + 1);
      etx = RSSI_DIFF * ETX_DIVISOR / (bounded_rssi - RSSI_LOW);
      return MIN(etx, ETX_INIT_MAX * ETX_DIVISOR);
    }
  }
  return 0xffff;
}
/*---------------------------------------------------------------------------*/
/* Packet sent callback. Updates stats for transmissions to lladdr *///only for unicast
uint8_t ewma_etx_idx = 0;
extern uint8_t* get_self_global_addr();
extern void update_mac_routing_cost(uint16_t, uint8_t*);
void update_nbr_etx (uint8_t *addr, uint16_t etx);
//extern void update_mac_routing_cost(uint16_t);
void
link_stats_packet_sent(const linkaddr_t *lladdr, int status, int numtx)
{
  static struct link_stats *stats = NULL;
//  uint8_t device_add[8] = {0};
//  uint8_t aro = 0x01;/*false*///without aro
//  uint16_t lifetime = 120;/*not sure need to check*/
  uint16_t packet_etx;
  uint8_t ewma_alpha;
  if(status != MAC_TX_OK/*0*/ && status != MAC_TX_NOACK/*2*/) {
    /* Do not penalize the ETX when collisions or transmission errors occur. */
    return;
  }
  
  stats = nbr_table_get_from_lladdr(link_stats, lladdr); 
  
  if(stats == NULL) {
    /* Add the neighbor */
    stats = nbr_table_add_lladdr(link_stats, lladdr, NBR_TABLE_REASON_LINK_STATS, NULL);
    if(stats != NULL) {
      stats->tx_count++;
      return;
      //updating while, link_stats_rsl_receive
      //stats->etx = LINK_STATS_INIT_ETX(stats);//(ETX_INIT/*2*/ * ETX_DIVISOR/*128*/)
    } else {
      return; /* No space left, return */
    }
  }  
  
  /*The ETX * 128 is encoded using 16 bits in unsigned integer format, rounded 
  off to the nearest whole number.  For example, if ETX = 3.569, the object value
  will be 457 (3.569*128).  If ETX > 511.9921875, the object value will be the 
  maximum, which is 65535. rfc6551#section-4.3.2*/  
  if(stats->tx_count == MAX_EWMA_COUNT){
    if (stats->rx_count == 0)
      packet_etx = 1024;
    else
      packet_etx = (stats->tx_count/stats->rx_count)* ETX_DIVISOR/*128*/;
    stats->tx_count = 0;
    stats->rx_count = 0;   
    stats->tx_count++;
//    if(stats->etx > 512/*512*/){
//      if(is_in_my_prefered_parent_set(((const uip_lladdr_t *)lladdr)) == 1)
//      {
//        {
//          uip_ds6_nbr_t *primary_parent = NULL;
//          stats->etx = 0;
//          device_remove_from_nbr((const uip_lladdr_t *)lladdr);//removing the current parent and switching
//          mem_rev_cpy(device_add,(uint8_t *)lladdr,8);
//          del_key_and_device_from_mac_pib(device_add);
//          
//          primary_parent = select_best_using_path_cost(); //again make prefered parents
//          if (primary_parent == NULL)
//          {
//            /* TBD :: PAN Timeout */
//            kill_process_and_clean_rpl_nbr ();
//            return;
//          }
//        }
//      }
//      else
//      {
//        device_remove_from_nbr((const uip_lladdr_t *)lladdr);
//      }
//    }
  }
  else{
    stats->tx_count++;
    return;//expecting MAX_EWMA_COUNT for that nbr
  }  
  
  /*Debdeep:: After checking stats->tx_count with MAX_EWMA_COUNT stats points to register memory.
  so again we are fetching the table. Need to debug this 09-March-18*/
  //  stats = nbr_table_get_from_lladdr(link_stats, lladdr);
  
  /* Update last timestamp and freshness */
  //  stats->last_tx_time = l3_clock_time();
  //  stats->freshness = MIN(stats->freshness + numtx, FRESHNESS_MAX);
  
  /* ETX used for this update */
  //  packet_etx = (stats->tx_count/stats->rx_count)* ETX_DIVISOR/*128*/;
  /* ETX alpha used for this update */
  //  ewma_alpha = link_stats_is_fresh(stats) ? EWMA_ALPHA/*15*/ : EWMA_BOOTSTRAP_ALPHA/*30*/;
  
  ewma_alpha = EWMA_ALPHA;
  
  /* Compute EWMA and update ETX */
  /*FAN TPS:1v10:page13:Exponentially Weighted Moving Average.*/
  stats->etx = ((uint32_t)stats->etx * (EWMA_SCALE/*100*/ - ewma_alpha) +
                (uint32_t)packet_etx * ewma_alpha) / EWMA_SCALE/*100*/;
  
  //  stats->etx = ((EWMA_ALPHA * (packet_etx - stats->etx)) + (EWMA_SCALE * stats->etx)) / EWMA_SCALE;
  
  if (stats->etx > (uint16_t)1024)
    stats->etx = 1024;
  
  update_nbr_etx ((uint8_t *)lladdr, stats->etx);
  
  /*updating mac_self_fan_info.pan_metrics.routing_cost for PA BS-IE*/
  update_mac_routing_cost(stats->etx, (uint8_t *)lladdr);
  
  /*only updating*/
  stats->ewma_etx[ewma_etx_idx++] = stats->etx;
  if(ewma_etx_idx == MAX_EWMA_COUNT){
    ewma_etx_idx = (MAX_EWMA_COUNT-1);
    uint8_t i=0;
    for(i=0; i<ewma_etx_idx; i++){
      stats->ewma_etx[i] = stats->ewma_etx[i+1];//poping the first and pushing next into last
    }    
  }    
}
/*---------------------------------------------------------------------------*/
/* Packet input callback. Updates statistics for receptions on a given link */
uint8_t ewma_rssi_idx = 0;
void
link_stats_input_callback(const linkaddr_t *lladdr)
{
  static struct link_stats *stats;
  int16_t packet_rssi = packetbuf_attr(PACKETBUF_ATTR_RSSI);
  stats = nbr_table_get_from_lladdr(link_stats, lladdr);
  if(stats == NULL) {
    /* Add the neighbor *///adds to nbr_table_keys
    stats = nbr_table_add_lladdr(link_stats, lladdr, NBR_TABLE_REASON_LINK_STATS, NULL);
    if(stats != NULL) {
      /* Initialize */
      stats->rssi = packet_rssi;
      //stats->etx = LINK_STATS_INIT_ETX(stats);       
    }
    return;
  }

  /* Update RSSI EWMA */
  /*FAN TPS:1v10:page13:Exponentially Weighted Moving Average.*/
  stats->rssi = (int32_t)((int32_t)stats->rssi * (int32_t)(EWMA_SCALE - EWMA_ALPHA) +
      ((int32_t)packet_rssi * (int32_t)EWMA_ALPHA)) / (int32_t)EWMA_SCALE;
  
  /*only updating*/
  stats->ewma_rssi[ewma_rssi_idx++] = stats->rssi;
  if(ewma_rssi_idx == MAX_EWMA_COUNT){
    ewma_rssi_idx = (MAX_EWMA_COUNT-1);
    uint8_t i=0;
    for(i=0; i<ewma_rssi_idx; i++){
      stats->ewma_rssi[i] = stats->ewma_rssi[i+1];
    }    
  }   
}
/*---------------------------------------------------------------------------*/
/*calculation of ewma for rsl, received from enhanced-ack*/ 
uint8_t ewma_rsl_ie_idx = 0;
void
link_stats_rsl_receive(const linkaddr_t *lladdr, int32_t rsl_ie)
{
  struct link_stats *stats;
  stats = nbr_table_get_from_lladdr(link_stats, lladdr);
  
  if(stats == NULL)
    return;
  
  if(stats->rsl_ie == 0x00)
    stats->rsl_ie = rsl_ie;
  
  stats->rx_count++;
  if(stats->etx == 0){
    //link_stats_packet_sent(lladdr, 0, 1);
    /*FAN:TPS:1v10:page 35*/
    stats->etx = 128*(stats->tx_count/stats->rx_count);
    stats->etx = ((uint32_t)stats->etx * (EWMA_SCALE/*100*/ - EWMA_ALPHA) +
        (uint32_t)stats->etx * EWMA_ALPHA) / EWMA_SCALE/*100*/;
  }

  /* Update RSSI EWMA */
  /*FAN TPS:1v10:page13:Exponentially Weighted Moving Average.*/
  stats->rsl_ie = ((int32_t)stats->rsl_ie * (EWMA_SCALE/*100*/ - EWMA_ALPHA/*15*/) +
      (int32_t)rsl_ie * EWMA_ALPHA/*15*/) / EWMA_SCALE/*100*/;
   
  /*only updating*/
  stats->ewma_rsl_ie[ewma_rsl_ie_idx++] = stats->rsl_ie;
  if(ewma_rsl_ie_idx == MAX_EWMA_COUNT){
    ewma_rsl_ie_idx = (MAX_EWMA_COUNT-1);
    uint8_t i=0;
    for(i=0; i<ewma_rsl_ie_idx; i++){
      stats->ewma_rsl_ie[i] = stats->ewma_rsl_ie[i+1];
    }    
  }  
}
/*---------------------------------------------------------------------------*/
/* Periodic timer called every FRESHNESS_HALF_LIFE minutes */
static void
periodic(void *ptr)
{
  /* Age (by halving) freshness counter of all neighbors */
  struct link_stats *stats;
  l3_ctimer_reset(&periodic_timer);
  for(stats = nbr_table_head(link_stats); stats != NULL; stats = nbr_table_next(link_stats, stats)) {
    stats->freshness >>= 1;
  }
}
/*---------------------------------------------------------------------------*/
/* Initializes link-stats module */
void
link_stats_init(void)
{
  nbr_table_register(link_stats, NULL);
  l3_ctimer_set(&periodic_timer, 60 * (clock_time_t)CLOCK_SECOND * FRESHNESS_HALF_LIFE,
      periodic, NULL);
}
/*----------------------------------------------------------------------------*/
  
void clean_link_stat_nbr_table (void)
{
  struct link_stats *stats = nbr_table_head(link_stats);
  while(stats != NULL)
  {
    nbr_table_remove(link_stats, stats);
    stats = nbr_table_next(link_stats, stats);
  }
}

void remove_nbr_stats (struct link_stats *stats)
{
  nbr_table_remove(link_stats, stats);
}

/*----------------------------------------------------------------------------*/
#if ((EFR32FG13P_LBR == 0x00) && (APP_NVM_FEATURE_ENABLED == 1))
void add_link_stats_nbr_from_nvm( struct link_stats *stats)
{
   struct link_stats *temp_stats;
     temp_stats = nbr_table_add_lladdr(link_stats, (const linkaddr_t *)&stats->ll_addr[0], NBR_TABLE_REASON_LINK_STATS, NULL);
     if(stats != NULL)
     {
       struct link_stats p_nbr = *temp_stats;
       memcpy(temp_stats,stats,sizeof(p_nbr));
     }
}
#endif