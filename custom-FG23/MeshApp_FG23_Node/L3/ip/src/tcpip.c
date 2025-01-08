/*
 * Copyright (c) 2004, Swedish Institute of Computer Science.
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
 *         Code for tunnelling uIP packets over the Rime mesh routing module
 *
 * \author  Adam Dunkels <adam@sics.se>\author
 * \author  Mathilde Durvy <mdurvy@cisco.com> (IPv6 related code)
 * \author  Julien Abeille <jabeille@cisco.com> (IPv6 related code)
 */
#include "Stack6LoWPANConf.h"
#include "contiki-net.h"
#include <string.h>

#define DEBUG DEBUG_NONE
#include "uip-debug.h"

#if UIP_LOGGING
#include <stdio.h>
void uip_log(char *msg);
#define UIP_LOG(m) uip_log(m)
#else
#define UIP_LOG(m)
#endif

#define UIP_ICMP_BUF ((struct uip_icmp_hdr *)&uip_buf[UIP_LLIPH_LEN + uip_ext_len])
#define UIP_IP_BUF ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])
#define UIP_TCP_BUF ((struct uip_tcpip_hdr *)&uip_buf[UIP_LLH_LEN])

#ifdef UIP_FALLBACK_INTERFACE
extern struct uip_fallback_interface UIP_FALLBACK_INTERFACE;
#endif


l3_process_event_t tcpip_event;
#if UIP_CONF_ICMP6
l3_process_event_t tcpip_icmp6_event;
#endif /* UIP_CONF_ICMP6 */

/* Periodic check of active connections. */
//static struct etimer periodic;
extern void uip_ds6_update_rsl_ie(uint8_t* data);
extern uint8_t root_device;
extern uint8_t* get_self_global_addr();
extern uint8_t get_node_type( void );
void set_src_dst_in_echo_structure (uip_ipaddr_t *src, uip_ipaddr_t *dest, uint16_t identifier, uint16_t seq_number);
void trig_wan_ping (uint8_t *address);
//void send_packt_after_some_randomness();
#if NETSTACK_CONF_WITH_IPV6 && UIP_CONF_IPV6_REASSEMBLY
/* Timer for reassembly. */
extern struct etimer uip_reass_timer;

#endif

#if UIP_TCP
/**
 * \internal Structure for holding a TCP port and a process ID.
 */
struct listenport {
  uint16_t port;
  struct process *p;
};

static struct internal_state {
  struct listenport listenports[UIP_LISTENPORTS];
  struct process *p;
} s;
#endif

enum {
  TCP_POLL,
  UDP_POLL,
  PACKET_INPUT,
  INIT_RPL
};

/* Called on IP packet output. */
#if NETSTACK_CONF_WITH_IPV6

static uint8_t (* outputfunc)(const uip_lladdr_t *a);
/*---------------------------------------------------------------------------*/
uint8_t
tcpip_output(const uip_lladdr_t *a)
{
  int ret;
  if(outputfunc != NULL) {
    ret = outputfunc(a);
    return ret;
  }
  UIP_LOG("tcpip_output: Use tcpip_set_outputfunc() to set an output function");
  return 0;
}
/*---------------------------------------------------------------------------*/
void
tcpip_set_outputfunc(uint8_t (*f)(const uip_lladdr_t *))
{
  outputfunc = f;
}
#else
/*---------------------------------------------------------------------------*/
static uint8_t (* outputfunc)(void);
uint8_t
tcpip_output(void)
{
  if(outputfunc != NULL) {
    return outputfunc();
  }
  UIP_LOG("tcpip_output: Use tcpip_set_outputfunc() to set an output function");
  return 0;
}
/*---------------------------------------------------------------------------*/
void
tcpip_set_outputfunc(uint8_t (*f)(void))
{
  outputfunc = f;
}
#endif
/*---------------------------------------------------------------------------*/
#if UIP_CONF_IP_FORWARD
unsigned char tcpip_is_forwarding; /* Forwarding right now? */
#endif /* UIP_CONF_IP_FORWARD */
/*---------------------------------------------------------------------------*/
L3_PROCESS(tcpip_process, "TCP/IP stack");
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
//#if UIP_TCP || UIP_CONF_IP_FORWARD
//static void
//start_periodic_tcp_timer(void)
//{
//  if(l3_etimer_expired(&periodic)) {
//    l3_etimer_restart(&periodic);
//  }
//}
//#endif /* UIP_TCP || UIP_CONF_IP_FORWARD */
/*---------------------------------------------------------------------------*/
//static void
//check_for_tcp_syn(void)
//{
//#if UIP_TCP || UIP_CONF_IP_FORWARD
//  /* This is a hack that is needed to start the periodic TCP timer if
//     an incoming packet contains a SYN: since uIP does not inform the
//     application if a SYN arrives, we have no other way of starting
//     this timer.  This function is called for every incoming IP packet
//     to check for such SYNs. */
//#define TCP_SYN 0x02
//  if(UIP_IP_BUF->proto == UIP_PROTO_TCP &&
//     (UIP_TCP_BUF->flags & TCP_SYN) == TCP_SYN) {
//    start_periodic_tcp_timer();
//  }
//#endif /* UIP_TCP || UIP_CONF_IP_FORWARD */
//}
/*---------------------------------------------------------------------------*/
//uint8_t create_random_ness = 0x00;
//extern uint8_t hw_tmr_rand( void *hw_tmr_ins );
//extern void create_random_delay(uint8_t rand_value);
//void send_packt_after_some_randomness()
//{
//#if UIP_CONF_TCP_SPLIT
//    uip_split_output();
//#else /* UIP_CONF_TCP_SPLIT */
//#if NETSTACK_CONF_WITH_IPV6
//    tcpip_ipv6_output();
//#else /* NETSTACK_CONF_WITH_IPV6 */
//    PRINTF("tcpip packet_input output len %d\n", uip_len);
//    tcpip_output();
//#endif /* NETSTACK_CONF_WITH_IPV6 */
//#endif /* UIP_CONF_TCP_SPLIT */
//    create_random_ness = 0xFF;
//}
static void
packet_input(void)
{
  if(uip_len > 0) {

#if UIP_CONF_IP_FORWARD
    tcpip_is_forwarding = 1;
    if(uip_fw_forward() != UIP_FW_LOCAL) {
      tcpip_is_forwarding = 0;
      return;
    }
    tcpip_is_forwarding = 0;
#endif /* UIP_CONF_IP_FORWARD */

//    check_for_tcp_syn();
    uip_input();
/* Changes done by suneet in this file */
    if(uip_len > 0) {
#if UIP_CONF_TCP_SPLIT
      uip_split_output();
#else /* UIP_CONF_TCP_SPLIT */
#if NETSTACK_CONF_WITH_IPV6
      tcpip_ipv6_output();
#else /* NETSTACK_CONF_WITH_IPV6 */
      PRINTF("tcpip packet_input output len %d\n", uip_len);
      tcpip_output();
#endif /* NETSTACK_CONF_WITH_IPV6 */
#endif /* UIP_CONF_TCP_SPLIT */
    }
//    if(uip_len > 0)
//    {
//      unsigned short rand_val = 0;
//      uint8_t randSeed =  hw_tmr_rand(NULL);
//      random_init((randSeed << 8));
//      rand_val = l3_random_rand() % 10;
//      create_random_delay(rand_val);
//      create_random_ness = 0x01;
//    } 
  }
}
/*---------------------------------------------------------------------------*/
#if UIP_TCP
#if UIP_ACTIVE_OPEN
struct uip_conn *
tcp_connect(const uip_ipaddr_t *ripaddr, uint16_t port, void *appstate)
{
  struct uip_conn *c;

  c = uip_connect(ripaddr, port);
  if(c == NULL) {
    return NULL;
  }

  c->appstate.p = L3_PROCESS_CURRENT();
  c->appstate.state = appstate;

  tcpip_poll_tcp(c);

  return c;
}
#endif /* UIP_ACTIVE_OPEN */
/*---------------------------------------------------------------------------*/
void
tcp_unlisten(uint16_t port)
{
  unsigned char i;
  struct listenport *l;

  l = s.listenports;
  for(i = 0; i < UIP_LISTENPORTS; ++i) {
    if(l->port == port &&
       l->p == L3_PROCESS_CURRENT()) {
      l->port = 0;
      uip_unlisten(port);
      break;
    }
    ++l;
  }
}
/*---------------------------------------------------------------------------*/
void
tcp_listen(uint16_t port)
{
  unsigned char i;
  struct listenport *l;

  l = s.listenports;
  for(i = 0; i < UIP_LISTENPORTS; ++i) {
    if(l->port == 0) {
      l->port = port;
      l->p = L3_PROCESS_CURRENT();
      uip_listen(port);
      break;
    }
    ++l;
  }
}
/*---------------------------------------------------------------------------*/
void
tcp_attach(struct uip_conn *conn,
	   void *appstate)
{
  uip_tcp_appstate_t *s;

  s = &conn->appstate;
  s->p = L3_PROCESS_CURRENT();
  s->state = appstate;
}

#endif /* UIP_TCP */
/*---------------------------------------------------------------------------*/
#if UIP_UDP
void
udp_attach(struct uip_udp_conn *conn,
	   void *appstate)
{
  uip_udp_appstate_t *s;

  s = &conn->appstate;
  s->p = L3_PROCESS_CURRENT();
  s->state = appstate;
}
/*---------------------------------------------------------------------------*/
struct uip_udp_conn *
udp_new(const uip_ipaddr_t *ripaddr, uint16_t port, void *appstate)
{
  struct uip_udp_conn *c;
  uip_udp_appstate_t *s;

  c = uip_udp_new(ripaddr, port);
  if(c == NULL) {
    return NULL;
  }

  s = &c->appstate;
  s->p = L3_PROCESS_CURRENT();
  s->state = appstate;

  return c;
}
/*---------------------------------------------------------------------------*/
struct uip_udp_conn *
udp_broadcast_new(uint16_t port, void *appstate)
{
  uip_ipaddr_t addr;
  struct uip_udp_conn *conn;

#if NETSTACK_CONF_WITH_IPV6
  uip_create_linklocal_allnodes_mcast(&addr);
#else
  uip_ipaddr(&addr, 255,255,255,255);
#endif /* NETSTACK_CONF_WITH_IPV6 */
  conn = udp_new(&addr, port, appstate);
  if(conn != NULL) {
    udp_bind(conn, port);
  }
  return conn;
}
#endif /* UIP_UDP */
/*---------------------------------------------------------------------------*/
#if UIP_CONF_ICMP6
uint8_t
icmp6_new(void *appstate) {
  if(uip_icmp6_conns.appstate.p == L3_PROCESS_NONE) {
    uip_icmp6_conns.appstate.p = L3_PROCESS_CURRENT();
    uip_icmp6_conns.appstate.state = appstate;
    return 0;
  }
  return 1;
}

void
tcpip_icmp6_call(uint8_t type)
{
  if(uip_icmp6_conns.appstate.p != L3_PROCESS_NONE) {
    /* XXX: This is a hack that needs to be updated. Passing a pointer (&type)
       like this only works with process_post_synch. */
    l3_process_post_synch(uip_icmp6_conns.appstate.p, tcpip_icmp6_event, &type);
  }
  return;
}
#endif /* UIP_CONF_ICMP6 */
/*---------------------------------------------------------------------------*/
static void
eventhandler(l3_process_event_t ev, l3_process_data_t data)
{
  struct process *p;
  
  switch(ev) {
  case PROCESS_EVENT_EXITED:
    /* This is the event we get if a process has exited. We go through
    the TCP/IP tables to see if this process had any open
    connections or listening TCP ports. If so, we'll close those
    connections. */
    p = (struct process *)data;
#if UIP_UDP
    {
      struct uip_udp_conn *cptr;
      
      for (cptr = &uip_udp_conns[0]; cptr < &uip_udp_conns[UIP_UDP_CONNS]; ++cptr)
      {
        if (cptr->appstate.p == p) 
        {
          cptr->lport = 0;
        }
      }
    }
#endif /* UIP_UDP */
    break;
    
  case L3_PROCESS_EVENT_TIMER:
    /* We get this event if one of our timers have expired. */
    {
      /* Check the clock so see if we should call the periodic uIP
      processing. */
      if(data == &uip_ds6_timer_periodic &&
         l3_etimer_expired(&uip_ds6_timer_periodic)) {
           uip_ds6_periodic();
           tcpip_ipv6_output();
         }    
    }
    break;
    
#if UIP_UDP
  case UDP_POLL:
    if(data != NULL) {
      uip_udp_periodic_conn(data);
#if NETSTACK_CONF_WITH_IPV6
      tcpip_ipv6_output();
#else
      if(uip_len > 0) {
        tcpip_output();
      }
#endif /* UIP_UDP */
    }
    break;
#endif /* UIP_UDP */
    
  case PACKET_INPUT:
    packet_input();
    break;
    
  case INIT_RPL:
    /* initialize RPL if configured for using RPL */
#if NETSTACK_CONF_WITH_IPV6 && UIP_CONF_IPV6_RPL
    rpl_init();
#endif /* UIP_CONF_IPV6_RPL */   
    break;
    
  case NBR_SEND_NS_ARO_EVENT:
    send_ns_with_aro();
    break;
#if ENABLE_DISABLE_DHCP_SERVICE     
  case EVENT_TO_DHCP_SOLICIT_REQUEST:
    event_to_send_dhcp_solicit_request();
    break;
#endif    
  case NBR_RSL_IE_ACK_EVENT:
    uip_ds6_update_rsl_ie((uint8_t *)data);
    break;
  };
}
/*---------------------------------------------------------------------------*/
void initialize_rpl (void)
{
   l3_process_post(&tcpip_process, INIT_RPL, NULL);
}

void
tcpip_input(void)
{
  l3_process_post_synch(&tcpip_process, PACKET_INPUT, NULL);
//  if(create_random_ness != 0x01)  //Suneet :: added for send packet after some delay 
  uip_clear_buf();
}
/*---------------------------------------------------------------------------*/
uint8_t *my_pointer = NULL;//for debugging UIP_IP_BUF
extern void rpl_srh_tunneling(uip_ipaddr_t *dest_ip_addr);
extern uint8_t get_link_status(uint8_t* rcv_addr);
uint16_t udp_port_no = 0xFFFF;//for separating dhcp udp(547, 546) and normal udp (8355)
uint8_t data_pkt_type = 0xFF;//we requited this to check data packet for ipv6 tunneling
#if NETSTACK_CONF_WITH_IPV6
void
tcpip_ipv6_output(void)
{
  uip_ds6_nbr_t *nbr = NULL;
  uip_ipaddr_t *nexthop = NULL;
  uip_lladdr_t lladdr;

  my_pointer = (uint8_t*)UIP_IP_BUF;
    
  if(uip_len == 0) {
    return;
  }

  /*there are rpl packets and type is ICMPV6, no need for ipv6 tunneling only 
  data packet required*/
  if(UIP_IP_BUF->proto == UIP_PROTO_ICMP6 ){
    uint8_t* ptr = (uint8_t*)UIP_IP_BUF;
    ptr = ptr+UIP_IPH_LEN;    
    data_pkt_type = *ptr; 
  } 
  else{
       data_pkt_type = UIP_IP_BUF->proto;
//    uint16_t* ptr = (uint16_t*)UIP_IP_BUF;
//    ptr = ptr+20;    
//    udp_port_no = UIP_HTONS(*ptr);     
 }
  
  if(uip_len > UIP_LINK_MTU) {
    UIP_LOG("tcpip_ipv6_output: Packet to big");
    uip_clear_buf();
    return;
  }

  if(uip_is_addr_unspecified(&UIP_IP_BUF->destipaddr)){
    UIP_LOG("tcpip_ipv6_output: Destination address unspecified");
    uip_clear_buf();
    return;
  }

#if 0  
//Suneet :: add this check if packet send to link local no need to chack in nbr table and update srh header direct send to link local 
  if(uip_is_addr_linklocal(&UIP_IP_BUF->destipaddr))    
  {//dhcpv6 udp src port & dst port
    udp_port_no = 0xFFFF;
    nexthop = &UIP_IP_BUF->destipaddr;
    memcpy(lladdr.addr, &(nexthop->u8[8]), 8);
    lladdr.addr[0] ^= 0x02;    
    tcpip_output(&lladdr);
    uip_clear_buf();
    return;//for dhcpv6 no need to add hbh or srh, these are direct link udp packets
  }  
#endif //if 0 
  
#if UIP_CONF_IPV6_RPL
  if(!rpl_update_header()) {//for updating srh or hbh
    /* Packet can not be forwarded */
    PRINTF("tcpip_ipv6_output: RPL header update error\n");
    uip_clear_buf();
    return;
  }
#endif /* UIP_CONF_IPV6_RPL */

  if(!uip_is_addr_mcast(&UIP_IP_BUF->destipaddr)) {
    /* Next hop determination */

#if UIP_CONF_IPV6_RPL && RPL_WITH_NON_STORING
    uip_ipaddr_t ipaddr;
    /* Look for a RPL Source Route */
    if(rpl_srh_get_next_hop(&ipaddr)) {//looks into nodelist, it is updated, while we receive dao
      nexthop = &ipaddr;
    }
#endif /* UIP_CONF_IPV6_RPL && RPL_WITH_NON_STORING */

    nbr = NULL;

    if(uip_ds6_is_addr_directlink(&UIP_IP_BUF->destipaddr) != 0x01){//if the devices is not connected, directly
    /* We first check if the destination address is on our immediate
       link. If so, we simply use the destination address as our
       nexthop address. */
    if(nexthop == NULL && uip_ds6_is_addr_onlink(&UIP_IP_BUF->destipaddr)
       /*uip_ds6_is_addr_directlink(&UIP_IP_BUF->destipaddr)*/){
      nexthop = &UIP_IP_BUF->destipaddr;//only for immediate or direct link nbr
    }

    if(nexthop == NULL) {
      uip_ds6_route_t *route;
      /* Check if we have a route to the destination address. */
      route = uip_ds6_route_lookup(&UIP_IP_BUF->destipaddr);//this will be only updated when RA RS happinng

      /* No route was found - we send to the default route instead. */
      if(route == NULL) {
        PRINTF("tcpip_ipv6_output: no route found, using default route\n");
        nexthop = uip_ds6_rpl_choose();//for choosing the default route
        /* if neighbor is not available so it should add ipv6 tunnling and 
        put IPV6 tunling address*/
        if(nexthop == NULL) {
          if (get_node_type () == 0x00)
          {
            uint16_t seq_number;
            uint16_t identifier;
            uint8_t wan_dest_address[16] = {0};
            identifier = *(uint16_t *)&my_pointer[44];
            seq_number = *(uint16_t *)&my_pointer[46];
            set_src_dst_in_echo_structure (&UIP_IP_BUF->srcipaddr, &UIP_IP_BUF->destipaddr, identifier, seq_number);  
            memcpy (wan_dest_address, &UIP_IP_BUF->destipaddr, 16);
            uip_clear_buf();
            trig_wan_ping (wan_dest_address);
          }
          
//#ifdef UIP_FALLBACK_INTERFACE
//          PRINTF("FALLBACK: removing ext hdrs & setting proto %d %d\n",
//              uip_ext_len, *((uint8_t *)UIP_IP_BUF + 40));
//          if(uip_ext_len > 0) {
//            extern void remove_ext_hdr(void);
//            uint8_t proto = *((uint8_t *)UIP_IP_BUF + 40);
//            remove_ext_hdr();
//            /* This should be copied from the ext header... */
//            UIP_IP_BUF->proto = proto;
//          }
//          /* Inform the other end that the destination is not reachable. If it's
//           * not informed routes might get lost unexpectedly until there's a need
//           * to send a new packet to the peer */
//          if(UIP_FALLBACK_INTERFACE.output() < 0) {
//            PRINTF("FALLBACK: output error. Reporting DST UNREACH\n");
//            uip_icmp6_error_output(ICMP6_DST_UNREACH, ICMP6_DST_UNREACH_ADDR, 0);
//            uip_flags = 0;
//            tcpip_ipv6_output();
//            return;
//          }
//#else
//          PRINTF("tcpip_ipv6_output: Destination off-link but no route\n");
//#endif /* !UIP_FALLBACK_INTERFACE */
          uip_clear_buf();
          return;
        }

      } else {
        /* A route was found, so we look up the nexthop neighbor for
           the route. */
        nexthop = uip_ds6_route_nexthop(route);

        /* If the nexthop is dead, for example because the neighbor
           never responded to link-layer acks, we drop its route. */
        if(nexthop == NULL) {
#if UIP_CONF_IPV6_RPL
          /* If we are running RPL, and if we are the root of the
             network, we'll trigger a global repair berfore we remove
             the route. */
          rpl_dag_t *dag;
          rpl_instance_t *instance;

          dag = (rpl_dag_t *)route->state.dag;
          if(dag != NULL) {
            instance = dag->instance;

            rpl_repair_root(instance->instance_id);
          }
#endif /* UIP_CONF_IPV6_RPL */
          uip_ds6_route_rm(route);

          /* We don't have a nexthop to send the packet to, so we drop
             it. */
          return;
        }
      }
#if TCPIP_CONF_ANNOTATE_TRANSMISSIONS
      if(nexthop != NULL) {
        static uint8_t annotate_last;
        static uint8_t annotate_has_last = 0;

        if(annotate_has_last) {
          printf("#L %u 0; red\n", annotate_last);
        }
        printf("#L %u 1; red\n", nexthop->u8[sizeof(uip_ipaddr_t) - 1]);
        annotate_last = nexthop->u8[sizeof(uip_ipaddr_t) - 1];
        annotate_has_last = 1;
      }
#endif /* TCPIP_CONF_ANNOTATE_TRANSMISSIONS */
    }
    }
    else{      
      nexthop = &UIP_IP_BUF->destipaddr;
    }
    /* End of next hop determination */

    nbr = uip_ds6_nbr_lookup(nexthop);
    
    if(nbr == NULL){
#if UIP_ND6_SEND_NA  
      
        memcpy(lladdr.addr, &(nexthop->u8[8]), 8);
        lladdr.addr[0] ^= 0x02;      
        tcpip_output(&lladdr);//added shubham       
        uip_clear_buf();
        return; 
        
#endif /* UIP_ND6_SEND_NA */
    } else {
#if UIP_ND6_SEND_NA
      if(nbr->state == NBR_INCOMPLETE) {
        PRINTF("tcpip_ipv6_output: nbr cache entry incomplete\n");
#if UIP_CONF_IPV6_QUEUE_PKT
        /* Copy outgoing pkt in the queuing buffer for later transmit and set
           the destination nbr to nbr. */
        if(uip_packetqueue_alloc(&nbr->packethandle, UIP_DS6_NBR_PACKET_LIFETIME) != NULL) {
          memcpy(uip_packetqueue_buf(&nbr->packethandle), UIP_IP_BUF, uip_len);
          uip_packetqueue_set_buflen(&nbr->packethandle, uip_len);
        }
#endif /*UIP_CONF_IPV6_QUEUE_PKT*/
        uip_clear_buf();
        return;
      }
#endif /* UIP_ND6_SEND_NA */

      tcpip_output(uip_ds6_nbr_get_ll(nbr));

#if UIP_CONF_IPV6_QUEUE_PKT
      /*
       * Send the queued packets from here, may not be 100% perfect though.
       * This happens in a few cases, for example when instead of receiving a
       * NA after sending a NS, you receive a NS with SLLAO: the entry moves
       * to STALE, and you must both send a NA and the queued packet.
       */
      if(uip_packetqueue_buflen(&nbr->packethandle) != 0) {
        uip_len = uip_packetqueue_buflen(&nbr->packethandle);
        memcpy(UIP_IP_BUF, uip_packetqueue_buf(&nbr->packethandle), uip_len);
        uip_packetqueue_free(&nbr->packethandle);
        tcpip_output(uip_ds6_nbr_get_ll(nbr));
      }
#endif /*UIP_CONF_IPV6_QUEUE_PKT*/

      uip_clear_buf();
      return;
    }
  }
  /* Multicast IP destination address. */
  tcpip_output(NULL);
  uip_clear_buf();
}
#endif /* NETSTACK_CONF_WITH_IPV6 */
/*---------------------------------------------------------------------------*/
#if UIP_UDP
void
tcpip_poll_udp(struct uip_udp_conn *conn)
{
  l3_process_post(&tcpip_process, UDP_POLL, conn);
}
#endif /* UIP_UDP */
/*---------------------------------------------------------------------------*/
#if UIP_TCP
void
tcpip_poll_tcp(struct uip_conn *conn)
{
  l3_process_post(&tcpip_process, TCP_POLL, conn);
}
#endif /* UIP_TCP */
/*---------------------------------------------------------------------------*/
void tcpip_post_event( l3_process_event_t ev, uint8_t* data)
{
  l3_process_post(&tcpip_process, ev, data);
}
/*---------------------------------------------------------------------------*/
void
tcpip_uipcall(void)
{
  uip_udp_appstate_t *ts;

#if UIP_UDP
  if(uip_conn != NULL) {
    ts = &uip_conn->appstate;
  } else {
    ts = &uip_udp_conn->appstate;
  }
#else /* UIP_UDP */
  ts = &uip_conn->appstate;
#endif /* UIP_UDP */

#if UIP_TCP
  {
    unsigned char i;
    struct listenport *l;

    /* If this is a connection request for a listening port, we must
      mark the connection with the right process ID. */
    if(uip_connected()) {
      l = &s.listenports[0];
      for(i = 0; i < UIP_LISTENPORTS; ++i) {
        if(l->port == uip_conn->lport &&
            l->p != L3_PROCESS_NONE) {
          ts->p = l->p;
          ts->state = NULL;
          break;
        }
        ++l;
      }

      /* Start the periodic polling, if it isn't already active. */
      start_periodic_tcp_timer();
    }
  }
#endif /* UIP_TCP */

  if(ts->p != NULL) {
    l3_process_post_synch(ts->p, tcpip_event, ts->state);
  }
}
/*---------------------------------------------------------------------------*/
L3_PROCESS_THREAD(tcpip_process, ev, data)
{
  L3_PROCESS_BEGIN();
  
  tcpip_event = l3_process_alloc_event();
#if UIP_CONF_ICMP6
  tcpip_icmp6_event = l3_process_alloc_event();
#endif /* UIP_CONF_ICMP6 */  
  uip_init();
  
  while(1) {
    L3_PROCESS_YIELD();
    eventhandler(ev, data);
  }
  
  L3_PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
extern uint8_t* get_self_global_addr();

#if(FAN_EDFE_FEATURE_ENABLED == 1)
void check_is_edfe_frame();
#endif

int compare_self_ipv6_addr(uint8_t *l_addr){

  uip_ipaddr_t* src_ipaddr = (uip_ipaddr_t*)get_self_global_addr();

  if(!memcmp(src_ipaddr->u8, l_addr, 16))
  {
    return 1;
  }
  else
  {
#if(FAN_EDFE_FEATURE_ENABLED == 1)
    check_is_edfe_frame();   
#endif
    return 0;
  }
}
/*---------------------------------------------------------------------------*/
extern void get_self_extended_address_reverse(uint8_t *self_mac_addr);
void change_dest_addr(uint8_t *l_addr){
  
  uint8_t self_addr[8] = {0};
  
  get_self_extended_address_reverse(self_addr);
  
  uip_lladdr_t ll_add;
  uip_ipaddr_t self_ext_addr;
  
  memcpy(ll_add.addr, self_addr, 8);
  uip_create_global_prefix(&self_ext_addr);
  uip_ds6_set_addr_iid(&self_ext_addr, &ll_add);
  
  UIP_IP_BUF->srcipaddr = self_ext_addr;
  memcpy(UIP_IP_BUF->destipaddr.u8, l_addr, 16);  
  return;
}
/*---------------------------------------------------------------------------*/