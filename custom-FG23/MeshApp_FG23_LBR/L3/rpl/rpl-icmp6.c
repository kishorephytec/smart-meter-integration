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
 *
 */

/**
 * \file
 *         ICMP6 I/O for RPL control messages.
 *
 * \author Joakim Eriksson <joakime@sics.se>, Nicolas Tsiftes <nvt@sics.se>
 * Contributors: Niclas Finne <nfi@sics.se>, Joel Hoglund <joel@sics.se>,
 *               Mathieu Pouillot <m.pouillot@watteco.com>
 *               George Oikonomou <oikonomou@users.sourceforge.net> (multicast)
 */

/**
 * \addtogroup uip6
 * @{
 */
#include "Stack6LoWPANConf.h"
#include "l3_configuration.h"
#include "l3_timer_utility.h"
#include "contiki-net.h"
#include <limits.h>
#include <string.h>

#define DEBUG 0//DEBUG_PRINT

#include "uip-debug.h"

/*---------------------------------------------------------------------------*/
#define RPL_DIO_GROUNDED                 0x80
#define RPL_DIO_MOP_SHIFT                3
#define RPL_DIO_MOP_MASK                 0x38
#define RPL_DIO_PREFERENCE_MASK          0x07

#define UIP_IP_BUF       ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])
#define UIP_ICMP_BUF     ((struct uip_icmp_hdr *)&uip_buf[uip_l2_l3_hdr_len])
#define UIP_ICMP_PAYLOAD ((unsigned char *)&uip_buf[uip_l2_l3_icmp_hdr_len])

#define MULTI_DAO_OPT 1
/*---------------------------------------------------------------------------*/
#define RPL_SEQUENCE_WINDOW 16
 /*Comparison functions return one of these values - as they are bits, you
 * can test for "greater-than-or-equal" by "if (cmp & (RPL_CMP_GREATER|RPL_CMP_EQUAL))".
 */

#if ((EFM32GG512 == 1) || (EFR32FG13P_LBR == 1))
#define RPL_CHILD_MAX            RPL_CFG_MAX_ROUTE_SUPPORT
#else
#define RPL_CHILD_MAX            1
#endif

typedef struct rpl_parents_set_tag
{
  uip_lladdr_t *ll_add;
  uip_ipaddr_t *child_node_address;
  uip_ipaddr_t *primary_parents;
  uip_ipaddr_t *secondary_parents;
  uip_ipaddr_t *ternary_parents;
  uint8_t path_sequence;
  uint8_t path_lifetime;
}rpl_parents_set_tag_t;

typedef struct rpl_parnts_tag
{
  rpl_parents_set_tag_t rpl_parent_set[RPL_CHILD_MAX];
  uint8_t total_child_add;
  uint8_t parents_perority_mask;
}rpl_parnts_t;

rpl_parnts_t mentain_parents_set;

extern void * app_bm_alloc(
    uint16_t length//base_t length      
    );
    
extern void app_bm_free(
    uint8_t *pMem      
    );

extern void kill_process_and_clean_rpl_nbr();
/*---------------------------------------------------------------------------*/
#if(APP_NVM_FEATURE_ENABLED == 1)
extern void nvm_store_dao_info(rpl_dag_t *dag,const uip_ipaddr_t *child_address, const uip_ipaddr_t *parents_address);
#endif

static void dis_input(void);
static void dio_input(void);
static void dao_input(void);
static void dao_ack_input(void);
extern void change_join_state(uint8_t attemt_index);
int get_global_addr(uip_ipaddr_t *addr);
uip_ipaddr_t* get_prefered_parents_addr(uip_ipaddr_t *child_ip_addr);
static void dao_output_target_seq(rpl_parent_t *parent, uip_ipaddr_t *prefix,
				  uint8_t lifetime, uint8_t seq_no);

uint8_t dodag_preferences_to_dao_path_control(uint8_t position);

void update_client_solicit_mac_address(uip_lladdr_t *ll_add);
void child_add_global_address_add(uip_ipaddr_t *child_node_addr, uip_lladdr_t *ll_add);
/* some debug callbacks useful when debugging RPL networks */
#ifdef RPL_DEBUG_DIO_INPUT
void RPL_DEBUG_DIO_INPUT(uip_ipaddr_t *, rpl_dio_t *);
#endif

#ifdef RPL_DEBUG_DAO_OUTPUT
void RPL_DEBUG_DAO_OUTPUT(rpl_parent_t *);
#endif

static uint8_t dao_sequence = RPL_LOLLIPOP_INIT;

#if RPL_WITH_MULTICAST
static uip_mcast6_route_t *mcast_group;
#endif
/*---------------------------------------------------------------------------*/
/* Initialise RPL ICMPv6 message handlers */
UIP_ICMP6_HANDLER(dis_handler, ICMP6_RPL, RPL_CODE_DIS, dis_input);
UIP_ICMP6_HANDLER(dio_handler, ICMP6_RPL, RPL_CODE_DIO, dio_input);
UIP_ICMP6_HANDLER(dao_handler, ICMP6_RPL, RPL_CODE_DAO, dao_input);
UIP_ICMP6_HANDLER(dao_ack_handler, ICMP6_RPL, RPL_CODE_DAO_ACK, dao_ack_input);
/*---------------------------------------------------------------------------*/

uint8_t rpl_seq_init(void)
{
    return 256 - RPL_SEQUENCE_WINDOW;
}

uint8_t rpl_seq_inc(uint8_t seq)
{
    return seq == 127 ? 0 : (uint8_t)(seq+1);
}


/*---------------------------------------------------------------------------*/
uint8_t get_child_mac_address_index(uip_lladdr_t *ll_add)
{
  uint8_t index;
  for (index = 0; index <= mentain_parents_set.total_child_add; index++)
  {
    if (!memcmp (mentain_parents_set.rpl_parent_set[index].ll_add, ll_add, 8))
      return index;
  }
  return 0xFF;
}
/*---------------------------------------------------------------------------*/
void child_add_global_address_add(uip_ipaddr_t *child_node_addr, uip_lladdr_t *ll_add)
{
  uint8_t index = get_child_mac_address_index(ll_add);
  
  if (index == 0xFF) /*Failure*/
    return;
  
//  uint8_t index = 0;
//  uint8_t ii;
//  //uip_ds6_nbr_t *l_nbr  = uip_ds6_nbr_lookup(child_node_addr);
//  if(mentain_parents_set.total_child_add < RPL_CHILD_MAX)
//  {
//    for(ii=0; ii < mentain_parents_set.total_child_add; ii++)
//    {
//      if(!memcmp(mentain_parents_set.rpl_parent_set[ii].child_node_address,child_node_addr,16))
//      {
//        //do nothing
//      }
//      else
//      {
//        index++;
//      }
//    }
//    
    if(mentain_parents_set.rpl_parent_set[index].child_node_address == NULL)
    {
      mentain_parents_set.rpl_parent_set[index].child_node_address = app_bm_alloc(16 );
      memcpy(mentain_parents_set.rpl_parent_set[index].child_node_address,child_node_addr,16);
    }
    if(mentain_parents_set.rpl_parent_set[index].child_node_address!= NULL)
    {
      memcpy(mentain_parents_set.rpl_parent_set[index].child_node_address,child_node_addr,16);
    }
//  }
}
/*---------------------------------------------------------------------------*/
uint8_t get_child_index(uip_ipaddr_t *child_node_addr)
{
  uint8_t index;
  for (index = 0; index <= mentain_parents_set.total_child_add; index++)
  {
    if(mentain_parents_set.rpl_parent_set[index].child_node_address != NULL)
    {
      if (!memcmp (mentain_parents_set.rpl_parent_set[index].child_node_address, child_node_addr, 16))
        return index;
    }
  }
  return 0xFF;
}
/*---------------------------------------------------------------------------*/
#if RPL_WITH_DAO_ACK
static uip_ds6_route_t *
find_route_entry_by_dao_ack(uint8_t seq)
{
  uip_ds6_route_t *re;
  re = uip_ds6_route_head();
  while(re != NULL) {
    if(re->state.dao_seqno_out == seq && RPL_ROUTE_IS_DAO_PENDING(re)) {
      /* found it! */
      return re;
    }
    re = uip_ds6_route_next(re);
  }
  return NULL;
}
#endif /* RPL_WITH_DAO_ACK */

#if RPL_WITH_STORING
/* prepare for forwarding of DAO */
static uint8_t
prepare_for_dao_fwd(uint8_t sequence, uip_ds6_route_t *rep)
{
  /* not pending - or pending but not a retransmission */
  RPL_LOLLIPOP_INCREMENT(dao_sequence);

  /* set DAO pending and sequence numbers */
  rep->state.dao_seqno_in = sequence;
  rep->state.dao_seqno_out = dao_sequence;
  RPL_ROUTE_SET_DAO_PENDING(rep);
  return dao_sequence;
}
#endif /* RPL_WITH_STORING */
/*---------------------------------------------------------------------------*/
/*static*/ 
int get_global_addr(uip_ipaddr_t *addr)
{
  int i;
  int state;

  for(i = 0; i < UIP_DS6_ADDR_NB; i++) {
    state = uip_ds6_if.addr_list[i].state;
    if(uip_ds6_if.addr_list[i].isused &&
       (state == ADDR_TENTATIVE || state == ADDR_PREFERRED)) {
      if(!uip_is_addr_linklocal(&uip_ds6_if.addr_list[i].ipaddr)) {
        memcpy(addr, &uip_ds6_if.addr_list[i].ipaddr, sizeof(uip_ipaddr_t));
        return 1;
      }
    }
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
static uint32_t
get32(uint8_t *buffer, int pos)
{
  return (uint32_t)buffer[pos] << 24 | (uint32_t)buffer[pos + 1] << 16 |
         (uint32_t)buffer[pos + 2] << 8 | buffer[pos + 3];
}
/*---------------------------------------------------------------------------*/
static void
set32(uint8_t *buffer, int pos, uint32_t value)
{
  buffer[pos++] = value >> 24;
  buffer[pos++] = (value >> 16) & 0xff;
  buffer[pos++] = (value >> 8) & 0xff;
  buffer[pos++] = value & 0xff;
}
/*---------------------------------------------------------------------------*/
static uint16_t
get16(uint8_t *buffer, int pos)
{
  return (uint16_t)buffer[pos] << 8 | buffer[pos + 1];
}
/*---------------------------------------------------------------------------*/
static void
set16(uint8_t *buffer, int pos, uint16_t value)
{
  buffer[pos++] = value >> 8;
  buffer[pos++] = value & 0xff;
}
/*---------------------------------------------------------------------------*/
extern uint8_t cold_boot_flag;
uip_ds6_nbr_t *
rpl_icmp6_update_nbr_table(uip_ipaddr_t *from, nbr_table_reason_t reason, void *data)
{
  uip_ds6_nbr_t *nbr;

  if((nbr = uip_ds6_nbr_lookup(from)) == NULL) {
    if(cold_boot_flag){//for router
      nbr = uip_ds6_nbr_add(from, (uip_lladdr_t *)packetbuf_addr
                            (PACKETBUF_ADDR_SENDER), 0, NBR_COLD_STARTUP, reason, 
                            data);
    }
    else if((nbr = uip_ds6_nbr_add(from, (uip_lladdr_t *)
                              packetbuf_addr(PACKETBUF_ADDR_SENDER),
                              0, NBR_REACHABLE, reason, data)) != NULL) {
      PRINTF("RPL: Neighbor added to neighbor cache ");
      PRINT6ADDR(from);
      PRINTF(", ");
      PRINTLLADDR((uip_lladdr_t *)packetbuf_addr(PACKETBUF_ADDR_SENDER));
      PRINTF("\n");
    }
  }

  return nbr;
 }
/*---------------------------------------------------------------------------*/
static void
dis_input(void)
{
  rpl_instance_t *instance;
  rpl_instance_t *end;

  /* DAG Information Solicitation */
  PRINTF("RPL: Received a DIS from ");
  PRINT6ADDR(&UIP_IP_BUF->srcipaddr);
  PRINTF("\n");

  for(instance = &instance_table[0], end = instance + RPL_MAX_INSTANCES;
      instance < end; ++instance) {
    if(instance->used == 1) {
#if RPL_LEAF_ONLY
      if(!uip_is_addr_mcast(&UIP_IP_BUF->destipaddr)) {
	PRINTF("RPL: LEAF ONLY Multicast DIS will NOT reset DIO timer\n");
#else /* !RPL_LEAF_ONLY */
      if(uip_is_addr_mcast(&UIP_IP_BUF->destipaddr)/* && !cold_boot_flag*/) {
        PRINTF("RPL: Multicast DIS => reset DIO timer\n");
        dio_output(instance, NULL); //Suneet :: send dio when received dis packet 
        rpl_reset_dio_timer(instance);//reseting the dio timer for sending dio
      } else {
        if(!uip_is_addr_mcast(&UIP_IP_BUF->destipaddr)){
#endif /* !RPL_LEAF_ONLY */
          
/* Debdeep:31-aug-2018:: We should not add nbr to RPL table when we receive DIS */
	/* Check if this neighbor should be added according to the policy. */
//        if(rpl_icmp6_update_nbr_table(&UIP_IP_BUF->srcipaddr,
//                                      NBR_TABLE_REASON_RPL_DIS, NULL) == NULL) {
//          PRINTF("RPL: Out of Memory, not sending unicast DIO, DIS from ");
//          PRINT6ADDR(&UIP_IP_BUF->srcipaddr);
//          PRINTF(", ");
//          PRINTLLADDR((uip_lladdr_t *)packetbuf_addr(PACKETBUF_ADDR_SENDER));
//          PRINTF("\n");
//        } else {
          
          PRINTF("RPL: Unicast DIS, reply to sender\n");
          dio_output(instance, &UIP_IP_BUF->srcipaddr);//unicast DIO
//        }
       }
      }
    }
  }
  uip_clear_buf();
}
/*---------------------------------------------------------------------------*/
extern void start_nud_process(void);
void
dis_output(uip_ipaddr_t *addr)
{
  unsigned char *buffer;
  uip_ipaddr_t tmpaddr;

  /*
   * DAG Information Solicitation  - 2 bytes reserved
   *      0                   1                   2
   *      0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3
   *     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   *     |     Flags     |   Reserved    |   Option(s)...
   *     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   */

  buffer = UIP_ICMP_PAYLOAD;
  buffer[0] = buffer[1] = 0;

  if(addr == NULL) {    
//#if (UIP_MCAST6_CONF_ENGINE == UIP_MCAST6_ENGINE_ROLL_TM)
//    uip_create_all_mpl_forwarders_mcast(&tmpaddr);//santosh, multicast
//#else    
//    uip_create_linklocal_rplnodes_mcast(&tmpaddr);    
//#endif    
    uip_create_linklocal_rplnodes_mcast(&tmpaddr); //Suneet :: dis ALWAYS SEND the link-local-scope all-RPL-nodes group (ff02::1a)
    addr = &tmpaddr;
  }

  PRINTF("RPL: Sending a DIS to ");
  PRINT6ADDR(addr);
  PRINTF("\n");

  uip_icmp6_send(addr, ICMP6_RPL, RPL_CODE_DIS, 2,uip_ds6_if.cur_hop_limit);
}
/*---------------------------------------------------------------------------*/
extern uint8_t rpl_check_new_dev_ipaddr(uip_ipaddr_t *new_addr);
extern void rpl_send_ns(uip_ipaddr_t *target);
extern uint8_t root_device;
extern uint8_t get_device_join_status(uip_ipaddr_t *n_addr);
extern void add_nbr_in_ns_struct(uip_ipaddr_t target);
extern void update_dio_received(uip_ipaddr_t* target);
uint16_t dio_input_count = 0;
static void
dio_input(void)
{
  unsigned char *buffer;
  uint8_t buffer_length;
  rpl_dio_t dio;
  uint8_t subopt_type;
  int i;
  int len;
  uip_ipaddr_t from; 
//  fan_nbr_table_t update_nbr;
  uint8_t zero_dagID[16] = {0};

  memset(&dio, 0, sizeof(dio));

  /* Set default values in case the DIO configuration option is missing. */
  dio.dag_intdoubl = RPL_DIO_INTERVAL_DOUBLINGS;
  dio.dag_intmin = RPL_DIO_INTERVAL_MIN;
  dio.dag_redund = RPL_DIO_REDUNDANCY;
  dio.dag_min_hoprankinc = RPL_MIN_HOPRANKINC;
  dio.dag_max_rankinc = RPL_MAX_RANKINC;
  dio.ocp = RPL_OF_OCP;
  dio.default_lifetime = RPL_DEFAULT_LIFETIME;
  dio.lifetime_unit = RPL_DEFAULT_LIFETIME_UNIT;

  uip_ipaddr_copy(&from, &UIP_IP_BUF->srcipaddr);
  //uip_ds6_nbr_t *l_nbr = uip_ds6_nbr_lookup(&from);
  
  /* DAG Information Object */
  PRINTF("RPL: Received a DIO from ");
  PRINT6ADDR(&from);
  PRINTF("\n");

  buffer_length = uip_len - uip_l3_icmp_hdr_len;

  /* Process the DIO base option. */
  i = 0;
  buffer = UIP_ICMP_PAYLOAD;

  dio.instance_id = buffer[i++];
  dio.version = buffer[i++];
  dio.rank = get16(buffer, i);
  i += 2;
  
  /*Debdeep::04-dec-2018::We should discard the packet if RANK is infinite i.e, 65535*/
  if (dio.rank == 65535)
    goto discard;

  PRINTF("RPL: Incoming DIO (id, ver, rank) = (%u,%u,%u)\n",
         (unsigned)dio.instance_id,
         (unsigned)dio.version,
         (unsigned)dio.rank);

  dio.grounded = buffer[i] & RPL_DIO_GROUNDED;
  dio.mop = (buffer[i]& RPL_DIO_MOP_MASK) >> RPL_DIO_MOP_SHIFT;
  dio.preference = buffer[i++] & RPL_DIO_PREFERENCE_MASK;

  dio.dtsn = buffer[i++];
  /* two reserved bytes */
  i += 2;

  memcpy(&dio.dag_id, buffer + i, sizeof(dio.dag_id));
  i += sizeof(dio.dag_id);
  
  /*Debdeep::04-dec-2018::We should discard the packet if NULL DAGID is received*/
  if (!memcmp(&dio.dag_id, zero_dagID, sizeof(dio.dag_id)))
    goto discard;

  PRINTF("RPL: Incoming DIO (dag_id, pref) = (");
  PRINT6ADDR(&dio.dag_id);
  PRINTF(", %u)\n", dio.preference);

  /* Check if there are any DIO suboptions. */
  for(; i < buffer_length; i += len) {
    subopt_type = buffer[i];
    if(subopt_type == RPL_OPTION_PAD1) {
      len = 1;
    } else {
      /* Suboption with a two-byte header + payload */
      len = 2 + buffer[i + 1];
    }

    if(len + i > buffer_length) {
      PRINTF("RPL: Invalid DIO packet\n");
      RPL_STAT(rpl_stats.malformed_msgs++);
      goto discard;
    }

    PRINTF("RPL: DIO option %u, length: %u\n", subopt_type, len - 2);

    switch(subopt_type) {
    case RPL_OPTION_DAG_METRIC_CONTAINER:
#if RPL_OPTION_DAG_METRIC_CONTAINER_SUPPORT//0           
      if(len < 6) {
        PRINTF("RPL: Invalid DAG MC, len = %d\n", len);
	RPL_STAT(rpl_stats.malformed_msgs++);
        goto discard;
      }
      dio.mc.type = buffer[i + 2];
      dio.mc.flags = buffer[i + 3] << 1;
      dio.mc.flags |= buffer[i + 4] >> 7;
      dio.mc.aggr = (buffer[i + 4] >> 4) & 0x3;
      dio.mc.prec = buffer[i + 4] & 0xf;
      dio.mc.length = buffer[i + 5];

      if(dio.mc.type == RPL_DAG_MC_NONE) {
        /* No metric container: do nothing */
      } else if(dio.mc.type == RPL_DAG_MC_ETX) {
        dio.mc.obj.etx = get16(buffer, i + 6);

        PRINTF("RPL: DAG MC: type %u, flags %u, aggr %u, prec %u, length %u, ETX %u\n",
	       (unsigned)dio.mc.type,
	       (unsigned)dio.mc.flags,
	       (unsigned)dio.mc.aggr,
	       (unsigned)dio.mc.prec,
	       (unsigned)dio.mc.length,
	       (unsigned)dio.mc.obj.etx);
      } else if(dio.mc.type == RPL_DAG_MC_ENERGY) {
        dio.mc.obj.energy.flags = buffer[i + 6];
        dio.mc.obj.energy.energy_est = buffer[i + 7];
      } else {
       PRINTF("RPL: Unhandled DAG MC type: %u\n", (unsigned)dio.mc.type);
       goto discard;
      }
#else
      goto discard;
#endif      
      break;
    case RPL_OPTION_ROUTE_INFO:
      if(len < 9) {
        PRINTF("RPL: Invalid destination prefix option, len = %d\n", len);
	RPL_STAT(rpl_stats.malformed_msgs++);
        goto discard;
      }

      /* The flags field includes the preference value. */
      dio.destination_prefix.length = buffer[i + 2];
      dio.destination_prefix.flags = buffer[i + 3];
      dio.destination_prefix.lifetime = get32(buffer, i + 4);

      if(((dio.destination_prefix.length + 7) / 8) + 8 <= len &&
         dio.destination_prefix.length <= 128) {
        PRINTF("RPL: Copying destination prefix\n");
        memcpy(&dio.destination_prefix.prefix, &buffer[i + 8],
               (dio.destination_prefix.length + 7) / 8);
      } else {
        PRINTF("RPL: Invalid route info option, len = %d\n", len);
	RPL_STAT(rpl_stats.malformed_msgs++);
        goto discard;
      }

      break;
    case RPL_OPTION_DAG_CONF:
      if(len != 16) {
        PRINTF("RPL: Invalid DAG configuration option, len = %d\n", len);
	RPL_STAT(rpl_stats.malformed_msgs++);
        goto discard;
      }

      /* Path control field  at i + 2 */
      dio.path_control_size = buffer[i + 2];
      dio.dag_intdoubl = buffer[i + 3];
      dio.dag_intmin = buffer[i + 4];
      dio.dag_redund = buffer[i + 5];
      dio.dag_max_rankinc = get16(buffer, i + 6);
      dio.dag_min_hoprankinc = get16(buffer, i + 8);
      dio.ocp = get16(buffer, i + 10);
      /* buffer + 12 is reserved */
      dio.default_lifetime = buffer[i + 13];
      dio.lifetime_unit = get16(buffer, i + 14);
      PRINTF("RPL: DAG conf:dbl=%d, min=%d red=%d maxinc=%d mininc=%d ocp=%d d_l=%u l_u=%u\n",
             dio.dag_intdoubl, dio.dag_intmin, dio.dag_redund,
             dio.dag_max_rankinc, dio.dag_min_hoprankinc, dio.ocp,
             dio.default_lifetime, dio.lifetime_unit);
      break;
    case RPL_OPTION_PREFIX_INFO:
      if(len != 32) {
        PRINTF("RPL: Invalid DAG prefix info, len != 32\n");
	RPL_STAT(rpl_stats.malformed_msgs++);
        goto discard;
      }
      dio.prefix_info.length = buffer[i + 2];
      dio.prefix_info.flags = buffer[i + 3];
      /* valid lifetime is ingnored for now - at i + 4 */
      /* preferred lifetime stored in lifetime */
      dio.prefix_info.lifetime = get32(buffer, i + 8);
      /* 32-bit reserved at i + 12 */
      PRINTF("RPL: Copying prefix information\n");
      memcpy(&dio.prefix_info.prefix, &buffer[i + 16], 16);
      break;
    default:
      PRINTF("RPL: Unsupported suboption type in DIO: %u\n",
	(unsigned)subopt_type);
    }
  }

#ifdef RPL_DEBUG_DIO_INPUT
  RPL_DEBUG_DIO_INPUT(&from, &dio);
#endif  
    
  //processing dio for once, since parent selection, rank calculation is based on 
  //DIO, but as per FAN TPS:1v10: parent selection, rank calculation should be
  //after NUD process
  //if(l_nbr == NULL)
  
  // Raka :: Quick fix on LBR , has to be fixed properly once fixed on Router with Parent set.
  // 26-Dec-2017
  if (!root_device)
  {

#if ((EFR32FG13P_LBR == 0x00) && (APP_NVM_FEATURE_ENABLED == 1))
    nvm_store_write_rpl_dio_info(&dio,&from);  
#endif
  
    rpl_process_dio(&from, &dio);
    uip_ds6_nbr_t *nbr;
    nbr = uip_ds6_nbr_lookup(&from);
    uip_ipaddr_copy(&nbr->global_addr,&dio.prefix_info.prefix); //suneet :: copy global address in  nbr table received from dio 
//    memcpy (update_nbr.mac_addr, (uint8_t *)nbr->lladdr.addr, 8);
//    memcpy (update_nbr.link_local_addr, (uint8_t *)nbr->ipaddr.u8, 16);
//    memcpy (update_nbr.global_addr, (uint8_t *)&nbr->global_addr, 16);
//    fan_mac_nbr_add(&update_nbr, NBR_TABLE_GLOBAL_ADDR_INDX | NBR_TABLE_LINK_LOCAL_INDX);
    dio_input_count++;
#if (PRINT_DEBUG_LEVEL==RUNTIME_LOG_FILE)
    send_runtime_log ("RECVD DIO from "LOG_MAC_ADDRESS((uint8_t *)nbr->lladdr.addr));
#endif     
  } 
  
 discard:
  uip_clear_buf();
}
/*---------------------------------------------------------------------------*/
uip_ipaddr_t root_global_addr;  //Suneet :: for root global address 
uint16_t dio_output_count = 0;

void
dio_output(rpl_instance_t *instance, uip_ipaddr_t *uc_addr)
{
  unsigned char *buffer;
  int pos;
  int is_root;
  rpl_dag_t *dag = instance->current_dag;
  
  // Raka added while testing with Cisco [ 06- Dec- 2017]
  uip_ipaddr_t myGlobalAddress;
  get_global_addr(&myGlobalAddress);
    
  memcpy(&root_global_addr, &instance->current_dag->dag_id, 16);
#if !RPL_LEAF_ONLY
  uip_ipaddr_t addr;
#endif /* !RPL_LEAF_ONLY */

#if RPL_LEAF_ONLY
  /* In leaf mode, we only send DIO messages as unicasts in response to
     unicast DIS messages. */
  if(uc_addr == NULL) {
    PRINTF("RPL: LEAF ONLY have multicast addr: skip dio_output\n");
    return;
  }
#endif /* RPL_LEAF_ONLY */

  /* DAG Information Object */
  pos = 0;

  buffer = UIP_ICMP_PAYLOAD;
  buffer[pos++] = instance->instance_id;
  buffer[pos++] = dag->version;
  is_root = (dag->rank == ROOT_RANK(instance));

#if RPL_LEAF_ONLY
  PRINTF("RPL: LEAF ONLY DIO rank set to INFINITE_RANK\n");
  set16(buffer, pos, INFINITE_RANK);
#else /* RPL_LEAF_ONLY */
  
  // Raka .. only for debugging with Cisco [ 06 -Dec -2017 ]
  // we need to validate our Rank calculation based on
  // Latest TPS and based on ETX calculation.
  //dag->rank = 512 ;
  
  set16(buffer, pos, dag->rank);
#endif /* RPL_LEAF_ONLY */
  pos += 2;

  buffer[pos] = 0;
  if(dag->grounded) {
    buffer[pos] |= RPL_DIO_GROUNDED;
  }

  
  buffer[pos] |= instance->mop << RPL_DIO_MOP_SHIFT;
  buffer[pos] |= dag->preference & RPL_DIO_PREFERENCE_MASK;
  pos++;

  buffer[pos++] = instance->dtsn_out;

  if(RPL_DIO_REFRESH_DAO_ROUTES && is_root && uc_addr == NULL) {
    /* Request new DAO to refresh route. We do not do this for unicast DIO
     * in order to avoid DAO messages after a DIS-DIO update,
     * or upon unicast DIO probing. */
    RPL_LOLLIPOP_INCREMENT(instance->dtsn_out);
  }

  /* reserved 2 bytes */
  buffer[pos++] = 0; /* flags */
  buffer[pos++] = 0; /* reserved */

  memcpy(buffer + pos, &dag->dag_id, sizeof(dag->dag_id));
  pos += 16;

#if !RPL_LEAF_ONLY
  if(instance->mc.type != RPL_DAG_MC_NONE) {
    instance->of->update_metric_container(instance);

    buffer[pos++] = RPL_OPTION_DAG_METRIC_CONTAINER;
    buffer[pos++] = 6;
    buffer[pos++] = instance->mc.type;
    buffer[pos++] = instance->mc.flags >> 1;
    buffer[pos] = (instance->mc.flags & 1) << 7;
    buffer[pos++] |= (instance->mc.aggr << 4) | instance->mc.prec;
    if(instance->mc.type == RPL_DAG_MC_ETX) {
      buffer[pos++] = 2;
      set16(buffer, pos, instance->mc.obj.etx);
      pos += 2;
    } else if(instance->mc.type == RPL_DAG_MC_ENERGY) {
      buffer[pos++] = 2;
      buffer[pos++] = instance->mc.obj.energy.flags;
      buffer[pos++] = instance->mc.obj.energy.energy_est;
    } else {
      PRINTF("RPL: Unable to send DIO because of unhandled DAG MC type %u\n",
	(unsigned)instance->mc.type);
      return;
    }
  }
#endif /* !RPL_LEAF_ONLY */

#if (PRINT_DEBUG_LEVEL == CONSOLE_ERROR_DEBUG)  
  stack_print_debug ("Trig-DIO\n");
#endif

  /* Always add a DAG configuration option. */
  buffer[pos++] = RPL_OPTION_DAG_CONF;
  buffer[pos++] = 14;
  buffer[pos++] = instance->path_control_size; /* No Auth, PCS = 7 */
  buffer[pos++] = instance->dio_intdoubl;
  buffer[pos++] = instance->dio_intmin;
  buffer[pos++] = instance->dio_redundancy;
  set16(buffer, pos, instance->max_rankinc);
  pos += 2;
  set16(buffer, pos, instance->min_hoprankinc);
  pos += 2;
  /* OCP is in the DAG_CONF option */
  set16(buffer, pos, instance->of->ocp);
  pos += 2;
  buffer[pos++] = 0; /* reserved */
  buffer[pos++] = instance->default_lifetime;
  set16(buffer, pos, instance->lifetime_unit);
  pos += 2;

  /* Check if we have a prefix to send also. */
  if(dag->prefix_info.length > 0) {
    buffer[pos++] = RPL_OPTION_PREFIX_INFO;
    buffer[pos++] = 30; /* always 30 bytes + 2 long */
    buffer[pos++] = dag->prefix_info.length;
    buffer[pos++] = dag->prefix_info.flags;
    set32(buffer, pos, dag->prefix_info.lifetime);
    pos += 4;
    set32(buffer, pos, dag->prefix_info.lifetime);
    pos += 4;
    memset(&buffer[pos], 0, 4);
    pos += 4;
    //memcpy(&buffer[pos], &dag->prefix_info.prefix, 16); //suneet :: not used auto  
    //memcpy(&buffer[pos], &dag->dag_id.u8, 16);
    
    // Raka ... Modified while testing with Cisco [ 06- Dec- 2017 ]
    memcpy(&buffer[pos], &myGlobalAddress.u8, 16);
    
    
    pos += 16;
    PRINTF("RPL: Sending prefix info in DIO for ");
    PRINT6ADDR(&dag->prefix_info.prefix);
    PRINTF("\n");
  } else {
    PRINTF("RPL: No prefix to announce (len %d)\n",
           dag->prefix_info.length);
  }

#if RPL_LEAF_ONLY
#if (DEBUG) & DEBUG_PRINT
  if(uc_addr == NULL) {
    PRINTF("RPL: LEAF ONLY sending unicast-DIO from multicast-DIO\n");
  }
#endif /* DEBUG_PRINT */
  PRINTF("RPL: Sending unicast-DIO with rank %u to ",
      (unsigned)dag->rank);
  PRINT6ADDR(uc_addr);
  PRINTF("\n");
  uip_icmp6_send(uc_addr, ICMP6_RPL, RPL_CODE_DIO, pos,uip_ds6_if.cur_hop_limit);
#else /* RPL_LEAF_ONLY */
  /* Unicast requests get unicast replies! */
  if(uc_addr == NULL) {
    PRINTF("RPL: Sending a multicast-DIO with rank %u\n",
        (unsigned)instance->current_dag->rank);  
//#if (UIP_MCAST6_CONF_ENGINE == UIP_MCAST6_ENGINE_ROLL_TM)//santosh, multicast
//    uip_create_all_mpl_forwarders_mcast(&addr);
//#else    
//    uip_create_linklocal_rplnodes_mcast(&addr);
//#endif  
    uip_create_linklocal_rplnodes_mcast(&addr); //Suneet :: dIO ALWAYS SEND the link-local-scope all-RPL-nodes group (ff02::1a) 
//    link_local_flag = 1;//SANTOSH, FOR TESTING
    uip_icmp6_send(&addr, ICMP6_RPL, RPL_CODE_DIO, pos,uip_ds6_if.cur_hop_limit);    
  } else {
    PRINTF("RPL: Sending unicast-DIO with rank %u to ",
        (unsigned)instance->current_dag->rank);
    PRINT6ADDR(uc_addr);
    PRINTF("\n");
//    link_local_flag = 1;//SANTOSH, FOR TESTING
    uip_icmp6_send(uc_addr, ICMP6_RPL, RPL_CODE_DIO, pos,uip_ds6_if.cur_hop_limit);
  }
#endif /* RPL_LEAF_ONLY */
  dio_output_count++;
}
/*---------------------------------------------------------------------------*/
static void
dao_input_storing(void)
{
#if RPL_WITH_STORING
  uip_ipaddr_t dao_sender_addr;
  rpl_dag_t *dag;
  rpl_instance_t *instance;
  unsigned char *buffer;
  uint16_t sequence;
  uint8_t instance_id;
  uint8_t lifetime;
  uint8_t prefixlen;
  uint8_t flags;
  uint8_t subopt_type;
  /*
  uint8_t pathcontrol;
  uint8_t pathsequence;
  */
  uip_ipaddr_t prefix;
  uip_ds6_route_t *rep;
  uint8_t buffer_length;
  int pos;
  int len;
  int i;
  int learned_from;
  rpl_parent_t *parent;
  uip_ds6_nbr_t *nbr;
  int is_root;

  prefixlen = 0;
  parent = NULL;

  uip_ipaddr_copy(&dao_sender_addr, &UIP_IP_BUF->srcipaddr);

  buffer = UIP_ICMP_PAYLOAD;
  buffer_length = uip_len - uip_l3_icmp_hdr_len;

  pos = 0;
  instance_id = buffer[pos++];

  instance = rpl_get_instance(instance_id);

  lifetime = instance->default_lifetime;

  flags = buffer[pos++];
  /* reserved */
  pos++;
  sequence = buffer[pos++];

  dag = instance->current_dag;
  is_root = (dag->rank == ROOT_RANK(instance));

  /* Is the DAG ID present? */
  if(flags & RPL_DAO_D_FLAG) {
    if(memcmp(&dag->dag_id, &buffer[pos], sizeof(dag->dag_id))) {
      PRINTF("RPL: Ignoring a DAO for a DAG different from ours\n");
      return;
    }
    pos += 16;
  }

  learned_from = uip_is_addr_mcast(&dao_sender_addr) ?
                 RPL_ROUTE_FROM_MULTICAST_DAO : RPL_ROUTE_FROM_UNICAST_DAO;

  /* Destination Advertisement Object */
  PRINTF("RPL: Received a (%s) DAO with sequence number %u from ",
      learned_from == RPL_ROUTE_FROM_UNICAST_DAO? "unicast": "multicast", sequence);
  PRINT6ADDR(&dao_sender_addr);
  PRINTF("\n");

  if(learned_from == RPL_ROUTE_FROM_UNICAST_DAO) {
    /* Check whether this is a DAO forwarding loop. */
    parent = rpl_find_parent(dag, &dao_sender_addr);
    /* check if this is a new DAO registration with an "illegal" rank */
    /* if we already route to this node it is likely */
    if(parent != NULL &&
       DAG_RANK(parent->rank, instance) < DAG_RANK(dag->rank, instance)) {
      PRINTF("RPL: Loop detected when receiving a unicast DAO from a node with a lower rank! (%u < %u)\n",
          DAG_RANK(parent->rank, instance), DAG_RANK(dag->rank, instance));
      parent->rank = INFINITE_RANK;
      parent->flags |= RPL_PARENT_FLAG_UPDATED;
      return;
    }

    /* If we get the DAO from our parent, we also have a loop. */
    if(parent != NULL && parent == dag->preferred_parent) {
      PRINTF("RPL: Loop detected when receiving a unicast DAO from our parent\n");
      parent->rank = INFINITE_RANK;
      parent->flags |= RPL_PARENT_FLAG_UPDATED;
      return;
    }
  }

  /* Check if there are any RPL options present. */
  for(i = pos; i < buffer_length; i += len) {
    subopt_type = buffer[i];
    if(subopt_type == RPL_OPTION_PAD1) {
      len = 1;
    } else {
      /* The option consists of a two-byte header and a payload. */
      len = 2 + buffer[i + 1];
    }

    switch(subopt_type) {
    case RPL_OPTION_TARGET:
      /* Handle the target option. */
      prefixlen = buffer[i + 3];
      memset(&prefix, 0, sizeof(prefix));
      memcpy(&prefix, buffer + i + 4, (prefixlen + 7) / CHAR_BIT);
      break;
    case RPL_OPTION_TRANSIT:
      /* The path sequence and control are ignored. */
      /*      pathcontrol = buffer[i + 3];
              pathsequence = buffer[i + 4];*/
      lifetime = buffer[i + 5];
      /* The parent address is also ignored. */
      break;
    }
  }

  PRINTF("RPL: DAO lifetime: %u, prefix length: %u prefix: ",
          (unsigned)lifetime, (unsigned)prefixlen);
  PRINT6ADDR(&prefix);
  PRINTF("\n");

#if RPL_WITH_MULTICAST
  if(uip_is_addr_mcast_global(&prefix)) {
    mcast_group = uip_mcast6_route_add(&prefix);
    if(mcast_group) {
      mcast_group->dag = dag;
      mcast_group->lifetime = RPL_LIFETIME(instance, lifetime);
    }
    goto fwd_dao;
  }
#endif

  rep = uip_ds6_route_lookup(&prefix);

  if(lifetime == RPL_ZERO_LIFETIME) {
    PRINTF("RPL: No-Path DAO received\n");
    /* No-Path DAO received; invoke the route purging routine. */
    if(rep != NULL &&
       !RPL_ROUTE_IS_NOPATH_RECEIVED(rep) &&
       rep->length == prefixlen &&
       uip_ds6_route_nexthop(rep) != NULL &&
       uip_ipaddr_cmp(uip_ds6_route_nexthop(rep), &dao_sender_addr)) {
      PRINTF("RPL: Setting expiration timer for prefix ");
      PRINT6ADDR(&prefix);
      PRINTF("\n");
      RPL_ROUTE_SET_NOPATH_RECEIVED(rep);
      rep->state.lifetime = RPL_NOPATH_REMOVAL_DELAY;

      /* We forward the incoming No-Path DAO to our parent, if we have
         one. */
      if(dag->preferred_parent != NULL &&
         rpl_get_parent_ipaddr(dag->preferred_parent) != NULL) {
        uint8_t out_seq;
        out_seq = prepare_for_dao_fwd(sequence, rep);

        PRINTF("RPL: Forwarding No-path DAO to parent - out_seq:%d",
	       out_seq);
        PRINT6ADDR(rpl_get_parent_ipaddr(dag->preferred_parent));
        PRINTF("\n");

        buffer = UIP_ICMP_PAYLOAD;
        buffer[3] = out_seq; /* add an outgoing seq no before fwd */
        uip_icmp6_send(rpl_get_parent_ipaddr(dag->preferred_parent),
                       ICMP6_RPL, RPL_CODE_DAO, buffer_length,uip_ds6_if.cur_hop_limit);
      }
    }
    /* independent if we remove or not - ACK the request */
    if(flags & RPL_DAO_K_FLAG) {
      /* indicate that we accepted the no-path DAO */
      uip_clear_buf();
      dao_ack_output(instance, &dao_sender_addr, sequence,
                     RPL_DAO_ACK_UNCONDITIONAL_ACCEPT);
    }
    return;
  }

  PRINTF("RPL: Adding DAO route\n");

  /* Update and add neighbor - if no room - fail. */
  if((nbr = rpl_icmp6_update_nbr_table(&dao_sender_addr, NBR_TABLE_REASON_RPL_DAO, instance)) == NULL) {
    PRINTF("RPL: Out of Memory, dropping DAO from ");
    PRINT6ADDR(&dao_sender_addr);
    PRINTF(", ");
    PRINTLLADDR((uip_lladdr_t *)packetbuf_addr(PACKETBUF_ADDR_SENDER));
    PRINTF("\n");
    if(flags & RPL_DAO_K_FLAG) {
      /* signal the failure to add the node */
      dao_ack_output(instance, &dao_sender_addr, sequence,
		     is_root ? RPL_DAO_ACK_UNABLE_TO_ADD_ROUTE_AT_ROOT :
		     RPL_DAO_ACK_UNABLE_TO_ACCEPT);
    }
    return;
  }

  rep = rpl_add_route(dag, &prefix, prefixlen, &dao_sender_addr);
  if(rep == NULL) {
    RPL_STAT(rpl_stats.mem_overflows++);
    PRINTF("RPL: Could not add a route after receiving a DAO\n");
    if(flags & RPL_DAO_K_FLAG) {
      /* signal the failure to add the node */
      dao_ack_output(instance, &dao_sender_addr, sequence,
		     is_root ? RPL_DAO_ACK_UNABLE_TO_ADD_ROUTE_AT_ROOT :
		     RPL_DAO_ACK_UNABLE_TO_ACCEPT);
    }
    return;
  }

  /* set lifetime and clear NOPATH bit */
  rep->state.lifetime = RPL_LIFETIME(instance, lifetime);
  RPL_ROUTE_CLEAR_NOPATH_RECEIVED(rep);

#if RPL_WITH_MULTICAST
fwd_dao:
#endif

  if(learned_from == RPL_ROUTE_FROM_UNICAST_DAO) {
    int should_ack = 0;

    if(flags & RPL_DAO_K_FLAG) {
      /*
       * check if this route is already installed and we can ack now!
       * not pending - and same seq-no means that we can ack.
       * (e.g. the route is installed already so it will not take any
       * more room that it already takes - so should be ok!)
       */
      if((!RPL_ROUTE_IS_DAO_PENDING(rep) &&
          rep->state.dao_seqno_in == sequence) ||
          dag->rank == ROOT_RANK(instance)) {
        should_ack = 1;
      }
    }

    if(dag->preferred_parent != NULL &&
       rpl_get_parent_ipaddr(dag->preferred_parent) != NULL) {
      uint8_t out_seq = 0;
      /* if this is pending and we get the same seq no it is a retrans */
      if(RPL_ROUTE_IS_DAO_PENDING(rep) &&
         rep->state.dao_seqno_in == sequence) {
        /* keep the same seq-no as before for parent also */
        out_seq = rep->state.dao_seqno_out;
      } else {
        out_seq = prepare_for_dao_fwd(sequence, rep);
      }

      PRINTF("RPL: Forwarding DAO to parent ");
      PRINT6ADDR(rpl_get_parent_ipaddr(dag->preferred_parent));
      PRINTF(" in seq: %d out seq: %d\n", sequence, out_seq);

      buffer = UIP_ICMP_PAYLOAD;
      buffer[3] = out_seq; /* add an outgoing seq no before fwd */
      uip_icmp6_send(rpl_get_parent_ipaddr(dag->preferred_parent),
                     ICMP6_RPL, RPL_CODE_DAO, buffer_length,uip_ds6_if.cur_hop_limit);
    }
    if(should_ack) {
      PRINTF("RPL: Sending DAO ACK\n");
      uip_clear_buf();
      dao_ack_output(instance, &dao_sender_addr, sequence,
                     RPL_DAO_ACK_UNCONDITIONAL_ACCEPT);
    }
  }
#endif /* RPL_WITH_STORING */
}
/*---------------------------------------------------------------------------*/
static void
dao_input_nonstoring(void)
{
#if RPL_WITH_NON_STORING
  uip_ipaddr_t dao_sender_addr;
  uip_ipaddr_t dao_parent_addr;
  rpl_dag_t *dag;
  rpl_instance_t *instance;
  unsigned char *buffer;
  uint16_t sequence;
  uint8_t instance_id;
  uint8_t lifetime;
  uint8_t prefixlen;
  uint8_t flags;
  uint8_t subopt_type;
  uip_ipaddr_t prefix;
  uint8_t buffer_length;
  int pos;
  int len;
  int i;
  uint8_t pathcontrol;
  uint8_t pathsequence;
  prefixlen = 0;

  uip_ipaddr_copy(&dao_sender_addr, &UIP_IP_BUF->srcipaddr);
  memset(&dao_parent_addr, 0, 16);

  buffer = UIP_ICMP_PAYLOAD;
  buffer_length = uip_len - uip_l3_icmp_hdr_len;

  pos = 0;
  instance_id = buffer[pos++];
  instance = rpl_get_instance(instance_id);
  lifetime = instance->default_lifetime;

  flags = buffer[pos++];
  /* reserved */
  pos++;
  sequence = buffer[pos++];

  dag = instance->current_dag;
  /* Is the DAG ID present? */
  if(flags & RPL_DAO_D_FLAG) {
    if(memcmp(&dag->dag_id, &buffer[pos], sizeof(dag->dag_id))) {
      PRINTF("RPL: Ignoring a DAO for a DAG different from ours\n");
      return;
    }
    pos += 16;
  }

  /* Check if there are any RPL options present. */
  for(i = pos; i < buffer_length; i += len) {
    subopt_type = buffer[i];
    if(subopt_type == RPL_OPTION_PAD1) {
      len = 1;
    } else {
      /* The option consists of a two-byte header and a payload. */
      len = 2 + buffer[i + 1];
    }

    switch(subopt_type) {
    case RPL_OPTION_TARGET:
      /* Handle the target option. */
      prefixlen = buffer[i + 3];
      memset(&prefix, 0, sizeof(prefix));
      memcpy(&prefix, buffer + i + 4, (prefixlen + 7) / CHAR_BIT);
      break;
    case RPL_OPTION_TRANSIT:
      /* The path sequence and control are ignored. */
      pathcontrol = buffer[i + 3];
      pathsequence = buffer[i + 4];
      lifetime = buffer[i + 5];
      
      uint8_t index = get_child_index(&prefix);
#if ENABLE_DISABLE_DHCP_SERVICE 	  
      if (index == 0xFF) /*Failure*/
      {
		break;
      }
#else	  
      if (index == 0xFF) /*Failure*/
      {
        uip_lladdr_t ll_add = {0};
        uip_ds6_get_addr_iid(&prefix,&ll_add);
        update_client_solicit_mac_address(&ll_add);
        child_add_global_address_add(&prefix,&ll_add);
        index = get_child_index(&prefix);
      }
#endif      
      if((pathcontrol >> 6) != 0)
      {
        if(len >= 20) {
          mentain_parents_set.rpl_parent_set[index].path_sequence = pathsequence;
          mentain_parents_set.rpl_parent_set[index].path_lifetime = lifetime;
          if(mentain_parents_set.rpl_parent_set[index].primary_parents == NULL)
          {
            mentain_parents_set.rpl_parent_set[index].primary_parents = app_bm_alloc(16 );
          }
          if(mentain_parents_set.rpl_parent_set[index].primary_parents!= NULL)
            memcpy(mentain_parents_set.rpl_parent_set[index].primary_parents,buffer + i + 6,16);
          memcpy(&dao_parent_addr, buffer + i + 6, 16);
        }
        break;
      }
      if((pathcontrol >> 4) != 0)
      {
        if(len >= 20) {
          mentain_parents_set.rpl_parent_set[index].path_sequence = pathsequence;
          mentain_parents_set.rpl_parent_set[index].path_lifetime = lifetime;
          if(mentain_parents_set.rpl_parent_set[index].secondary_parents == NULL)
          {
            mentain_parents_set.rpl_parent_set[index].secondary_parents = app_bm_alloc(16 );
          }
          if(mentain_parents_set.rpl_parent_set[index].secondary_parents!= NULL)
            memcpy(mentain_parents_set.rpl_parent_set[index].secondary_parents,buffer + i + 6,16);
        }
        break;
      }
      if((pathcontrol >> 2) != 0)
      {
        if(len >= 20) {
          mentain_parents_set.rpl_parent_set[index].path_sequence = pathsequence;
          mentain_parents_set.rpl_parent_set[index].path_lifetime = lifetime;
          if(mentain_parents_set.rpl_parent_set[index].ternary_parents == NULL)
          {
            mentain_parents_set.rpl_parent_set[index].ternary_parents = app_bm_alloc(16 );
          }
          if(mentain_parents_set.rpl_parent_set[index].ternary_parents!= NULL)
            memcpy(mentain_parents_set.rpl_parent_set[index].ternary_parents,buffer + i + 6,16);
        }
        break;
      }
      if(pathcontrol == 0x00)
      {
        if(mentain_parents_set.parents_perority_mask == 0x00)
        {
          if(len >= 20) {
            mentain_parents_set.rpl_parent_set[index].path_sequence = pathsequence;
            mentain_parents_set.rpl_parent_set[index].path_lifetime = lifetime;
            if(mentain_parents_set.rpl_parent_set[index].primary_parents == NULL)
            {
              mentain_parents_set.rpl_parent_set[index].primary_parents = app_bm_alloc(16 );
            }
            if(mentain_parents_set.rpl_parent_set[index].primary_parents!= NULL)
              memcpy(mentain_parents_set.rpl_parent_set[index].primary_parents,buffer + i + 6,16);
            memcpy(&dao_parent_addr, buffer + i + 6, 16);
            mentain_parents_set.parents_perority_mask = 0x80 >> (mentain_parents_set.parents_perority_mask );
          }
        }
      }
      break;
    }
  }
  PRINTF("RPL: DAO lifetime: %u, prefix length: %u prefix: ",
          (unsigned)lifetime, (unsigned)prefixlen);
  PRINT6ADDR(&prefix);
  PRINTF(", parent: ");
  PRINT6ADDR(&dao_parent_addr);
  PRINTF(" \n");

  if(lifetime == RPL_ZERO_LIFETIME) {
    PRINTF("RPL: No-Path DAO received\n");
    rpl_ns_expire_parent(dag, &prefix, &dao_parent_addr);
  } else {
#if(APP_NVM_FEATURE_ENABLED == 1)
    nvm_store_dao_info(dag,&prefix, &dao_parent_addr);   //suneet :: add dao info in nvm
#endif
    if(rpl_ns_update_node(dag, &prefix, &dao_parent_addr, RPL_LIFETIME(instance, lifetime)) == NULL) {
      PRINTF("RPL: failed to add link\n");
      return;
    }
  }

  if(flags & RPL_DAO_K_FLAG) {
    PRINTF("RPL: Sending DAO ACK\n");
    uip_clear_buf();
    dao_ack_output(instance, &dao_sender_addr, sequence,
        RPL_DAO_ACK_UNCONDITIONAL_ACCEPT);
  }
#endif /* RPL_WITH_NON_STORING */
}
/*---------------------------------------------------------------------------*/

uint8_t dao_input_debug_count = 0;

static void
dao_input(void)
{
  rpl_instance_t *instance;
  uint8_t instance_id;

  /* Destination Advertisement Object */
  PRINTF("RPL: Received a DAO from ");
  PRINT6ADDR(&UIP_IP_BUF->srcipaddr);
  PRINTF("\n");

  instance_id = UIP_ICMP_PAYLOAD[0];
  instance = rpl_get_instance(instance_id);
  if(instance == NULL) {
    PRINTF("RPL: Ignoring a DAO for an unknown RPL instance(%u)\n",
           instance_id);
    goto discard;
  }

  dao_input_debug_count++;
  
  if(RPL_IS_STORING(instance)) {
    dao_input_storing();
  } else if(RPL_IS_NON_STORING(instance)) {
    dao_input_nonstoring();
  }
 discard:
  uip_clear_buf();
}
/*---------------------------------------------------------------------------*/
#if RPL_WITH_DAO_ACK
static void
handle_dao_retransmission(void *ptr)
{
  rpl_parent_t *parent;
  uip_ipaddr_t prefix;
  rpl_instance_t *instance;

  parent = ptr;
  if(parent == NULL || parent->dag == NULL || parent->dag->instance == NULL) {
    return;
  }
  instance = parent->dag->instance;

  if(instance->my_dao_transmissions >= RPL_DAO_MAX_RETRANSMISSIONS) {
    /* No more retransmissions - give up. */
    
    kill_process_and_clean_rpl_nbr();
    return;
    
//    if(instance->lifetime_unit == 0xffff && instance->default_lifetime == 0xff) {
//      /*
//       * ContikiRPL was previously using infinite lifetime for routes
//       * and no DAO_ACK configured. This probably means that the root
//       * and possibly other nodes might be running an old version that
//       * does not support DAO ack. Assume that everything is ok for
//       * now and let the normal repair mechanisms detect any problems.
//       */
//      return;
//    }
//
//    if(RPL_IS_STORING(instance) && instance->of->dao_ack_callback) {
//      /* Inform the objective function about the timeout. */
//      instance->of->dao_ack_callback(parent, RPL_DAO_ACK_TIMEOUT);
//    }
//
//    /* Perform local repair and hope to find another parent. */
//    //rpl_local_repair(instance);//currently, not performing from dao
//    return;   
  }

  PRINTF("RPL: will retransmit DAO - seq:%d trans:%d\n", instance->my_dao_seqno,
	 instance->my_dao_transmissions);

  if(get_global_addr(&prefix) == 0) {
    return;
  }

  l3_ctimer_set(&instance->dao_retransmit_timer,
             RPL_DAO_RETRANSMISSION_TIMEOUT / 2 +
             (l3_random_rand() % (RPL_DAO_RETRANSMISSION_TIMEOUT / 2)),
	     handle_dao_retransmission, parent);

  instance->my_dao_transmissions++;
  dao_output_target_seq(parent, &prefix,
			instance->default_lifetime, instance->my_dao_seqno);
}
#endif /* RPL_WITH_DAO_ACK */
/*---------------------------------------------------------------------------*/
// Raka  [ 06-Dec-2017] we are not sending DIO so i have to change 
// to test as RELAY device  when testing with Cisco
extern uip_ipaddr_t root_global_addr;  


void
dao_output(rpl_parent_t *parent, uint8_t lifetime)
{
  /* Destination Advertisement Object */  
  //set_device_as_router();
  uip_ipaddr_t prefix;

  if(get_global_addr(&prefix) == 0) {
    PRINTF("RPL: No global address set for this node - suppressing DAO\n");
    return;
  }

  if(parent == NULL || parent->dag == NULL || parent->dag->instance == NULL) {
    return;
  }

  RPL_LOLLIPOP_INCREMENT(dao_sequence);
#if RPL_WITH_DAO_ACK
  /* set up the state since this will be the first transmission of DAO */
  /* retransmissions will call directly to dao_output_target_seq */
  /* keep track of my own sending of DAO for handling ack and loss of ack */
  if(lifetime != RPL_ZERO_LIFETIME) {
    rpl_instance_t *instance;
    instance = parent->dag->instance;

    instance->my_dao_seqno = dao_sequence;
    instance->my_dao_transmissions = 1;
    l3_ctimer_set(&instance->dao_retransmit_timer, RPL_DAO_RETRANSMISSION_TIMEOUT,
 	       handle_dao_retransmission, parent);
  }
#else
   /* We know that we have tried to register so now we are assuming
      that we have a down-link - unless this is a zero lifetime one */
  parent->dag->instance->has_downward_route = lifetime != RPL_ZERO_LIFETIME;
#endif /* RPL_WITH_DAO_ACK */

  // Raka :: Added this ..
        rpl_instance_t *instance;
        instance = parent->dag->instance;
        memcpy(&root_global_addr, &instance->current_dag->dag_id, 16);
  /* Sending a DAO with own prefix as target */
  dao_output_target(parent, &prefix, instance->default_lifetime);
}
/*---------------------------------------------------------------------------*/
void
dao_output_target(rpl_parent_t *parent, uip_ipaddr_t *prefix, uint8_t lifetime)
{
  dao_output_target_seq(parent, prefix, lifetime, dao_sequence);
}
/*---------------------------------------------------------------------------*/
int uip_create_multi_dao_opt(uip_ipaddr_t *prefix, unsigned char *dao_opt_buf, 
                            int pos, rpl_instance_t *instance, uint8_t lifetime,
                            rpl_parent_t *parent, uip_ipaddr_t *parent_ipaddr);

extern uip_ds6_nbr_t *uip_get_prio_based_parent_nbr(uint8_t prio);

static void
dao_output_target_seq(rpl_parent_t *parent, uip_ipaddr_t *prefix,
		      uint8_t lifetime, uint8_t seq_no)
{
  rpl_dag_t *dag;
  rpl_instance_t *instance;
  unsigned char *buffer;  
  int pos;
  uip_ipaddr_t *parent_ipaddr = NULL;  
  uip_ipaddr_t *dest_ipaddr = NULL;
  uip_ds6_nbr_t *l_nbr = (uip_ds6_nbr_t *)uip_get_prio_based_parent_nbr(1);//Suneet :: best perfferrd parents according peroirity 
  /* Destination Advertisement Object */

  /* If we are in feather mode, we should not send any DAOs */
  if(rpl_get_mode() == RPL_MODE_FEATHER) {
    return;
  }

  if(parent == NULL) {
    PRINTF("RPL dao_output_target error parent NULL\n");
    return;
  }

  parent_ipaddr = rpl_get_parent_ipaddr(parent);
  if(parent_ipaddr == NULL) {
    PRINTF("RPL dao_output_target error parent IP address NULL\n");
    return;
  }

  dag = parent->dag;
  if(dag == NULL) {
    PRINTF("RPL dao_output_target error dag NULL\n");
    return;
  }

  instance = dag->instance;

  if(instance == NULL) {
    PRINTF("RPL dao_output_target error instance NULL\n");
    return;
  }
  if(prefix == NULL) {
    PRINTF("RPL dao_output_target error prefix NULL\n");
    return;
  }
#ifdef RPL_DEBUG_DAO_OUTPUT
  RPL_DEBUG_DAO_OUTPUT(parent);
#endif

  buffer = UIP_ICMP_PAYLOAD;
  pos = 0;

  buffer[pos++] = instance->instance_id;
  buffer[pos] = 0;
#if RPL_DAO_SPECIFY_DAG
//  buffer[pos] |= RPL_DAO_D_FLAG;
#endif /* RPL_DAO_SPECIFY_DAG */
#if RPL_WITH_DAO_ACK
  if(lifetime != RPL_ZERO_LIFETIME) {
    buffer[pos] |= RPL_DAO_K_FLAG;
  }
#endif /* RPL_WITH_DAO_ACK */
  ++pos;
  buffer[pos++] = 0; /* reserved */
  buffer[pos++] = seq_no;
#if RPL_DAO_SPECIFY_DAG
//  memcpy(buffer + pos, &dag->dag_id, sizeof(dag->dag_id));
//  pos+=sizeof(dag->dag_id);
#endif /* RPL_DAO_SPECIFY_DAG */

#if MULTI_DAO_OPT
    pos = uip_create_multi_dao_opt(prefix, buffer, pos, instance, lifetime, 
                                   parent, (uip_ipaddr_t *)&l_nbr->global_addr);    
    /* Send DAO to root */
    dest_ipaddr = &parent->dag->dag_id;    
#else
    uint8_t prefixlen;
  /* create target subopt */
  prefixlen = sizeof(*prefix) * CHAR_BIT;
  buffer[pos++] = RPL_OPTION_TARGET;
  buffer[pos++] = 2 + ((prefixlen + 7) / CHAR_BIT);
  buffer[pos++] = 0; /* reserved */
  buffer[pos++] = prefixlen;
  memcpy(buffer + pos, prefix, (prefixlen + 7) / CHAR_BIT);
  pos += ((prefixlen + 7) / CHAR_BIT);

  /* Create a transit information sub-option. */
  buffer[pos++] = RPL_OPTION_TRANSIT;
  buffer[pos++] = (instance->mop != RPL_MOP_NON_STORING) ? 4 : 20;
  buffer[pos++] = 0; /* flags - ignored */
  buffer[pos++] = dodag_preferences_to_dao_path_control(1); /* path control - ignored */
  buffer[pos++] = 0; /* path seq - ignored */
  buffer[pos++] = lifetime;

  if(instance->mop != RPL_MOP_NON_STORING) {
    /* Send DAO to parent */
    dest_ipaddr = parent_ipaddr;
  } else {
    /* Include parent global IP address */
    
    // Raka ..  Commented this 05- Dec-2017 with Cisco Testing....
    
//    memcpy(buffer + pos, &parent->dag->dag_id, 8); /* Prefix */
//    pos += 8;
//    memcpy(buffer + pos, ((const unsigned char *)parent_ipaddr) + 8, 8); /* Interface identifier */
//    pos += 8;    
    
      memcpy(buffer + pos, ((const unsigned char *)&l_nbr->global_addr), 16);  //Suneet :: best parent send in dao trnsit information 
      //memcpy(buffer + pos, ((const unsigned char *)&parent->dag->preferred_parent->dag->prefix_info.prefix), 16); 
      pos += 16; 
    
    /* Send DAO to root */
    dest_ipaddr = &parent->dag->dag_id;    
  }
#endif
  
  PRINTF("RPL: Sending a %sDAO with sequence number %u, lifetime %u, prefix ",
      lifetime == RPL_ZERO_LIFETIME ? "No-Path " : "", seq_no, lifetime);

  PRINT6ADDR(prefix);
  PRINTF(" to ");
  PRINT6ADDR(dest_ipaddr);
  PRINTF(" , parent ");
  PRINT6ADDR(parent_ipaddr);
  PRINTF("\n");

  if(dest_ipaddr != NULL) {
    uip_icmp6_send(dest_ipaddr, ICMP6_RPL, RPL_CODE_DAO, pos,uip_ds6_if.cur_hop_limit);    
  }
  uip_clear_buf();
  return;
}
/*---------------------------------------------------------------------------*/
extern void rpl_schedule_probing(rpl_instance_t *instance);
extern void send_udp_data_packet();
extern void change_nbr_state_to_probe(uint8_t* p_addr);
extern void change_to_join_state_05(void);
#define SEND_UNICAST_DIS_PKT           0  //Suneet :: add this to send unicast dis 
static void
dao_ack_input(void)
{
#if RPL_WITH_DAO_ACK

  uint8_t *buffer;
  uint8_t instance_id;
  uint8_t sequence;
  uint8_t status;
  rpl_instance_t *instance;
  rpl_parent_t *parent;

  buffer = UIP_ICMP_PAYLOAD;

  instance_id = buffer[0];
  sequence = buffer[2];
  status = buffer[3];

  instance = rpl_get_instance(instance_id);
  if(instance == NULL) {
    uip_clear_buf();
    return;
  }

  if(RPL_IS_STORING(instance)) {
    parent = rpl_find_parent(instance->current_dag, &UIP_IP_BUF->srcipaddr);
    if(parent == NULL) {
      /* not a known instance - drop the packet and ignore */
      uip_clear_buf();
      return;
    }
  } else {
    parent = NULL;
  }

  PRINTF("RPL: Received a DAO %s with sequence number %d (%d) and status %d from ",
   status < 128 ? "ACK" : "NACK",
	 sequence, instance->my_dao_seqno, status);
  PRINT6ADDR(&UIP_IP_BUF->srcipaddr);
  PRINTF("\n");

  if(sequence == instance->my_dao_seqno) {
    instance->has_downward_route = status < 128;

    /* always stop the retransmit timer when the ACK arrived */
    l3_ctimer_stop(&instance->dao_retransmit_timer);
    
    rpl_reset_dio_timer(instance);//for sending dio after receiving DAO-ACK input in router
    
    /* Inform objective function on status of the DAO ACK */
    if(RPL_IS_STORING(instance) && instance->of->dao_ack_callback) {
      instance->of->dao_ack_callback(parent, status);
    }

#if RPL_REPAIR_ON_DAO_NACK
    if(status >= RPL_DAO_ACK_UNABLE_TO_ACCEPT) {
      /*
       * Failed the DAO transmission - need to remove the default route.
       * Trigger a local repair since we can not get our DAO in.
       */
      rpl_local_repair(instance);
    }
#endif

  } else if(RPL_IS_STORING(instance)) {
    /* this DAO ACK should be forwarded to another recently registered route */
    uip_ds6_route_t *re;
    uip_ipaddr_t *nexthop;
    if((re = find_route_entry_by_dao_ack(sequence)) != NULL) {
      /* pick the recorded seq no from that node and forward DAO ACK - and
         clear the pending flag*/
      RPL_ROUTE_CLEAR_DAO_PENDING(re);

      nexthop = uip_ds6_route_nexthop(re);
      if(nexthop == NULL) {
        PRINTF("RPL: No next hop to fwd DAO ACK to\n");
      } else {
        PRINTF("RPL: Fwd DAO ACK to:");
        PRINT6ADDR(nexthop);
        PRINTF("\n");
        buffer[2] = re->state.dao_seqno_in;
        uip_icmp6_send(nexthop, ICMP6_RPL, RPL_CODE_DAO_ACK, 4,uip_ds6_if.cur_hop_limit);
      }

      if(status >= RPL_DAO_ACK_UNABLE_TO_ACCEPT) {
        /* this node did not get in to the routing tables above... - remove */
        uip_ds6_route_rm(re);
      }
    } else {
      PRINTF("RPL: No route entry found to forward DAO ACK (seqno %u)\n", sequence);
    }
  }
  
 
  
#endif /* RPL_WITH_DAO_ACK */ 
  //change_to_join_state_05();
  change_join_state(0);
  //change_nbr_state_to_probe(buffer+38);//santosh, buffer+38 = dest address
  rpl_schedule_probing(instance);  
  send_udp_data_packet();//explicit sent of udp packet for testing.
  uip_clear_buf();
#if SEND_UNICAST_DIS_PKT
    {
      unsigned char *buffer;
      int pos;
      uip_ds6_nbr_t *nbr = nbr_table_head(ds6_neighbors);//choosing from defaultrouterlist
      buffer = UIP_ICMP_PAYLOAD;
      
      uip_icmp6_send(&nbr->ipaddr, ICMP6_RPL, RPL_CODE_DIS, 2,uip_ds6_if.cur_hop_limit);
    }
#endif 
}
/*---------------------------------------------------------------------------*/

uint8_t dao_ack_output_debug_count = 0;

void
dao_ack_output(rpl_instance_t *instance, uip_ipaddr_t *dest, uint8_t sequence,
	       uint8_t status)
{
#if RPL_WITH_DAO_ACK
  unsigned char *buffer;

  PRINTF("RPL: Sending a DAO %s with sequence number %d to ", status < 128 ? "ACK" : "NACK", sequence);
  PRINT6ADDR(dest);
  PRINTF(" with status %d\n", status);

  buffer = UIP_ICMP_PAYLOAD;

  buffer[0] = instance->instance_id;
  buffer[1] = 0;
  buffer[2] = sequence;
  buffer[3] = status;
  
  dao_ack_output_debug_count++;

  uip_icmp6_send(dest, ICMP6_RPL, RPL_CODE_DAO_ACK, 4,uip_ds6_if.cur_hop_limit);
#endif /* RPL_WITH_DAO_ACK */
}
/*---------------------------------------------------------------------------*/
void
rpl_icmp6_register_handlers()
{
  uip_icmp6_register_input_handler(&dis_handler);
  uip_icmp6_register_input_handler(&dio_handler);
  uip_icmp6_register_input_handler(&dao_handler);
  uip_icmp6_register_input_handler(&dao_ack_handler);
}
/*---------------------------------------------------------------------------*/
int uip_create_multi_dao_opt(uip_ipaddr_t *prefix, unsigned char *dao_opt_buf, 
                            int pos, rpl_instance_t *instance, uint8_t lifetime,
                            rpl_parent_t *parent, uip_ipaddr_t *parent_ipaddr)
{
    uint8_t prefixlen;
    int dao_opt_buf_len = pos;
    /* create target subopt */
    prefixlen = sizeof(*prefix) * CHAR_BIT;
    dao_opt_buf[pos++] = RPL_OPTION_TARGET;
    dao_opt_buf[pos++] = 2 + ((prefixlen + 7) / CHAR_BIT);
    dao_opt_buf[pos++] = 0; /* reserved */
    dao_opt_buf[pos++] = prefixlen;
    memcpy(dao_opt_buf + pos, prefix, (prefixlen + 7) / CHAR_BIT);
    pos += ((prefixlen + 7) / CHAR_BIT);

    /* Create a transit information sub-option. */
    dao_opt_buf[pos++] = RPL_OPTION_TRANSIT;
    dao_opt_buf[pos++] = (instance->mop != RPL_MOP_NON_STORING) ? 4 : 20;
    dao_opt_buf[pos++] = 0; /* flags - ignored */
    dao_opt_buf[pos++] = dodag_preferences_to_dao_path_control(1); /* path control - ignored */
    dao_opt_buf[pos++] = rpl_seq_init(); /* path seq - ignored */
    dao_opt_buf[pos++] = lifetime;

    /*for RPL_MOP_NON_STORING*/
    /* Include parent global IP address */
//    memcpy(dao_opt_buf + pos, &parent->dag->dag_id, 8); /* Prefix */
//    pos += 8;
//    memcpy(dao_opt_buf + pos, ((const unsigned char *)parent_ipaddr) + 8, 8); /* Interface identifier */
//    pos += 8; 
     memcpy(dao_opt_buf + pos, ((const unsigned char *)parent_ipaddr), 16);
     pos += 16;
    //uip_ipaddr_t *second_parent_ipaddr = uip_get_prio_based_parent_ip(2);
      uip_ds6_nbr_t *l_nbr2 = (uip_ds6_nbr_t *)uip_get_prio_based_parent_nbr(2);//Suneet :: best perfferrd parents according peroirity
    if(/*second_parent_ipaddr*/l_nbr2 != NULL){
      /* create target subopt */
//      prefixlen = sizeof(*prefix) * CHAR_BIT;
//      dao_opt_buf[pos++] = RPL_OPTION_TARGET;
//      dao_opt_buf[pos++] = 2 + ((prefixlen + 7) / CHAR_BIT);
//      dao_opt_buf[pos++] = 0; /* reserved */
//      dao_opt_buf[pos++] = prefixlen;
//      memcpy(dao_opt_buf + pos, prefix, (prefixlen + 7) / CHAR_BIT);
//      pos += ((prefixlen + 7) / CHAR_BIT);
      
      /* Create a transit information sub-option. */
      dao_opt_buf[pos++] = RPL_OPTION_TRANSIT;
      dao_opt_buf[pos++] = (instance->mop != RPL_MOP_NON_STORING) ? 4 : 20;
      dao_opt_buf[pos++] = 0; /* flags - ignored */
      dao_opt_buf[pos++] = dodag_preferences_to_dao_path_control(2); /* path control - ignored */
      dao_opt_buf[pos++] = rpl_seq_init(); /* path seq - ignored */
      dao_opt_buf[pos++] = lifetime;

      /*for RPL_MOP_NON_STORING*/
      /* Include parent global IP address */
//      memcpy(dao_opt_buf + pos, &parent->dag->dag_id, 8); /* Prefix */
//      pos += 8;
//      memcpy(dao_opt_buf + pos, ((const unsigned char *)second_parent_ipaddr) + 8, 8); /* Interface identifier */
//      pos += 8;     
       memcpy(dao_opt_buf + pos, ((const unsigned char *)&l_nbr2->global_addr), 16);  //Suneet :: best parent send in dao trnsit information 
      //memcpy(buffer + pos, ((const unsigned char *)&parent->dag->preferred_parent->dag->prefix_info.prefix), 16); 
      pos += 16; 
    }
    
    //uip_ipaddr_t *third_parent_ipaddr = uip_get_prio_based_parent_ip(3);
    uip_ds6_nbr_t *l_nbr3 = (uip_ds6_nbr_t *)uip_get_prio_based_parent_nbr(3);//Suneet :: best perfferrd parents according peroirity
    if(l_nbr3 != NULL){
      /* create target subopt */
//      prefixlen = sizeof(*prefix) * CHAR_BIT;
//      dao_opt_buf[pos++] = RPL_OPTION_TARGET;
//      dao_opt_buf[pos++] = 2 + ((prefixlen + 7) / CHAR_BIT);
//      dao_opt_buf[pos++] = 0; /* reserved */
//      dao_opt_buf[pos++] = prefixlen;
//      memcpy(dao_opt_buf + pos, prefix, (prefixlen + 7) / CHAR_BIT);
//      pos += ((prefixlen + 7) / CHAR_BIT);

      /* Create a transit information sub-option. */
      dao_opt_buf[pos++] = RPL_OPTION_TRANSIT;
      dao_opt_buf[pos++] = (instance->mop != RPL_MOP_NON_STORING) ? 4 : 20;
      dao_opt_buf[pos++] = 0; /* flags - ignored */
      dao_opt_buf[pos++] = dodag_preferences_to_dao_path_control(3); /* path control - ignored */
      dao_opt_buf[pos++] =  rpl_seq_init(); /* path seq - ignored */
      dao_opt_buf[pos++] = lifetime;

      /*for RPL_MOP_NON_STORING*/
      /* Include parent global IP address */
//      memcpy(dao_opt_buf + pos, &parent->dag->dag_id, 8); /* Prefix */
//      pos += 8;
//      memcpy(dao_opt_buf + pos, ((const unsigned char *)third_parent_ipaddr) + 8, 8); /* Interface identifier */
//      pos += 8; 
      memcpy(dao_opt_buf + pos, ((const unsigned char *)&l_nbr3->global_addr), 16);  //Suneet :: best parent send in dao trnsit information 
      //memcpy(buffer + pos, ((const unsigned char *)&parent->dag->preferred_parent->dag->prefix_info.prefix), 16); 
      pos += 16; 
    }
    
    dao_opt_buf_len = pos;
    return dao_opt_buf_len;
}

uint8_t dodag_preferences_to_dao_path_control(uint8_t position)
{
    uint8_t bit_set = 0x01;
    if (position == 1)
      bit_set <<= 6;
    if (position == 2)
      bit_set <<= 4;
    if (position == 3)
      bit_set <<= 2;
    if (position == 4)
      bit_set <<= 0;
    
  return bit_set;
}

uip_ipaddr_t*
get_prefered_parents_addr(uip_ipaddr_t *child_ip_addr)
{
    uint8_t index = get_child_index(child_ip_addr);
    if (index == 0xFF)
      return NULL;
    return mentain_parents_set.rpl_parent_set[index].primary_parents;
}
/*Suneet :: mentain information RPL child address */
void update_client_solicit_mac_address(uip_lladdr_t *ll_add)
{
  uint8_t index = 0;
  uint8_t ii;
  //uip_ds6_nbr_t *l_nbr  = uip_ds6_nbr_lookup(child_node_addr);
  if(mentain_parents_set.total_child_add < RPL_CHILD_MAX)
  {
    for(ii=0; ii < mentain_parents_set.total_child_add; ii++)
    {
      if(!memcmp(mentain_parents_set.rpl_parent_set[ii].ll_add,ll_add,8))
      {
        //do nothing
      }
      else
      {
        index++;
      }
    }
    
    if(mentain_parents_set.rpl_parent_set[index].ll_add == NULL)
    {
      mentain_parents_set.rpl_parent_set[index].ll_add = app_bm_alloc(8 );
      memcpy(mentain_parents_set.rpl_parent_set[index].ll_add,ll_add,8);
      mentain_parents_set.total_child_add++;
    }
    if(mentain_parents_set.rpl_parent_set[index].ll_add!= NULL)
    {
      memcpy(mentain_parents_set.rpl_parent_set[index].ll_add,ll_add,8);
    }
  }
}
/** @}*/
