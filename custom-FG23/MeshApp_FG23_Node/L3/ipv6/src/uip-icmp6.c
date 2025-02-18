/*
 * Copyright (c) 2001-2003, Adam Dunkels.
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
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * This file is part of the uIP TCP/IP stack.
 *
 */

/**
 * \addtogroup uip6
 * @{
 */

/**
 * \file
 *    ICMPv6 (RFC 4443) implementation, with message and error handling
 * \author Julien Abeille <jabeille@cisco.com>
 * \author Mathilde Durvy <mdurvy@cisco.com>
 */
#include "Stack6LoWPANConf.h"
#include <string.h>
#include "contiki-net.h"
//#include "tri_tmr.h"/*Umesh 12-01-2018 commented*/
//#include "timer_service.h"/*Umesh 12-01-2018 commented*/

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#define PRINT6ADDR(addr) PRINTF(" %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x ", ((uint8_t *)addr)[0], ((uint8_t *)addr)[1], ((uint8_t *)addr)[2], ((uint8_t *)addr)[3], ((uint8_t *)addr)[4], ((uint8_t *)addr)[5], ((uint8_t *)addr)[6], ((uint8_t *)addr)[7], ((uint8_t *)addr)[8], ((uint8_t *)addr)[9], ((uint8_t *)addr)[10], ((uint8_t *)addr)[11], ((uint8_t *)addr)[12], ((uint8_t *)addr)[13], ((uint8_t *)addr)[14], ((uint8_t *)addr)[15])
#define PRINTLLADDR(lladdr) PRINTF(" %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x ",lladdr->addr[0], lladdr->addr[1], lladdr->addr[2], lladdr->addr[3],lladdr->addr[4], lladdr->addr[5],lladdr->addr[6],lladdr->addr[7])
#else
#define PRINTF(...)
#define PRINT6ADDR(addr)
#endif

#define UIP_IP_BUF                ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])
#define UIP_ICMP_BUF            ((struct uip_icmp_hdr *)&uip_buf[uip_l2_l3_hdr_len])
#define UIP_ICMP6_ERROR_BUF  ((struct uip_icmp6_error *)&uip_buf[uip_l2_l3_icmp_hdr_len])
#define UIP_EXT_BUF              ((struct uip_ext_hdr *)&uip_buf[uip_l2_l3_hdr_len])
#define UIP_FIRST_EXT_BUF        ((struct uip_ext_hdr *)&uip_buf[UIP_LLIPH_LEN])

#if UIP_CONF_IPV6_RPL
#include "rpl.h"
#endif /* UIP_CONF_IPV6_RPL */

extern int get_join_state (void);
/** \brief temporary IP address */
static uip_ipaddr_t tmp_ipaddr;
extern void create_rand_delay_in_us();

L3_LIST(echo_reply_callback_list);
/*---------------------------------------------------------------------------*/
/* List of input handlers */
L3_LIST(input_handler_list);
/*---------------------------------------------------------------------------*/
static uip_icmp6_input_handler_t *
input_handler_lookup(uint8_t type, uint8_t icode)
{
  uip_icmp6_input_handler_t *handler = NULL;

  for(handler = l3_list_head(input_handler_list);
      handler != NULL;
      handler = l3_list_item_next(handler)) {
    if(handler->type == type &&
       (handler->icode == icode ||
        handler->icode == UIP_ICMP6_HANDLER_CODE_ANY)) {
      return handler;
    }
  }

  return NULL;
}
/*********** Raka Added for extention Header Remove 18 July 2018 
for renesas DAO-ACK , need to test thoroughly *******************/
void remove_ext_hdr(void);

//static void  remove_extention_Header_input(void)
//{
//  
//  
//  /*
//   * we send an echo reply. It is trivial if there was no extension
//   * headers in the request otherwise we need to remove the extension
//   * headers and change a few fields
//   */
//  PRINTF("Received Echo Request from ");
//  PRINT6ADDR(&UIP_IP_BUF->srcipaddr);
//  PRINTF(" to ");
//  PRINT6ADDR(&UIP_IP_BUF->destipaddr);
//  PRINTF("\n");
//
//  /* IP header */
//  UIP_IP_BUF->ttl = uip_ds6_if.cur_hop_limit;
//
//  if(uip_is_addr_mcast(&UIP_IP_BUF->destipaddr)){
//    uip_ipaddr_copy(&UIP_IP_BUF->destipaddr, &UIP_IP_BUF->srcipaddr);
//    uip_ds6_select_src(&UIP_IP_BUF->srcipaddr, &UIP_IP_BUF->destipaddr);
//    //santosh : create delay, for multicast echo request
//    create_rand_delay_in_us();
//  } else {
//    uip_ipaddr_copy(&tmp_ipaddr, &UIP_IP_BUF->srcipaddr);
//    uip_ipaddr_copy(&UIP_IP_BUF->srcipaddr, &UIP_IP_BUF->destipaddr);
//    uip_ipaddr_copy(&UIP_IP_BUF->destipaddr, &tmp_ipaddr);
//  }
//
//  if(uip_ext_len > 0) {
//    /* Remove extension headers if any */
//    UIP_IP_BUF->proto = UIP_PROTO_ICMP6;
//    uip_len -= uip_ext_len;
//    UIP_IP_BUF->len[0] = ((uip_len - UIP_IPH_LEN) >> 8);
//    UIP_IP_BUF->len[1] = ((uip_len - UIP_IPH_LEN) & 0xff);
//    /* move the echo request payload (starting after the icmp header)
//     * to the new location in the reply.
//     * The shift is equal to the length of the extension headers present
//     * Note: UIP_ICMP_BUF still points to the echo request at this stage
//     */
//    memmove((uint8_t *)UIP_ICMP_BUF + UIP_ICMPH_LEN - uip_ext_len,
//        (uint8_t *)UIP_ICMP_BUF + UIP_ICMPH_LEN,
//        (uip_len - UIP_IPH_LEN - UIP_ICMPH_LEN));
//    uip_ext_len = 0;
//  }
//
//  /* Below is important for the correctness of UIP_ICMP_BUF and the
//   * checksum
//   */
//
//  /* Note: now UIP_ICMP_BUF points to the beginning of the echo reply */
//  UIP_ICMP_BUF->type = ICMP6_ECHO_REPLY;
//  UIP_ICMP_BUF->icode = 0;
//  UIP_ICMP_BUF->icmpchksum = 0;
//  UIP_ICMP_BUF->icmpchksum = ~uip_icmp6chksum();
//
//  PRINTF("Sending Echo Reply to ");
//  PRINT6ADDR(&UIP_IP_BUF->destipaddr);
//  PRINTF(" from ");
//  PRINT6ADDR(&UIP_IP_BUF->srcipaddr);
//  PRINTF("\n");
//  UIP_STAT(++uip_stat.icmp.sent);
//  return;
//}


/*---------------------------------------------------------------------------*/
uint8_t
uip_icmp6_input(uint8_t type, uint8_t icode)
{
  uip_icmp6_input_handler_t *handler = input_handler_lookup(type, icode);

  if(handler == NULL) 
  {
//     if ( type == 0xE2 &&  icode == 0x3A )
//     {
//       //remove_extention_Header_input();       
//       remove_ext_hdr();
//     }
//     else
//     {
      return UIP_ICMP6_INPUT_ERROR;
//     }
  }

  if(handler->handler == NULL) {
    return UIP_ICMP6_INPUT_ERROR;
  }
  
  /* Debdeep:: Router should only Process DIS in join state 5 */
  if ((handler->icode == RPL_CODE_DIS) && (get_join_state() != 5))
     return UIP_ICMP6_INPUT_ERROR;

  handler->handler();
  return UIP_ICMP6_INPUT_SUCCESS;
}
/*---------------------------------------------------------------------------*/
void
uip_icmp6_register_input_handler(uip_icmp6_input_handler_t *handler)
{
  l3_list_add(input_handler_list, handler);
}
/*---------------------------------------------------------------------------*/
static void
echo_request_input(void)
{
  /*
   * we send an echo reply. It is trivial if there was no extension
   * headers in the request otherwise we need to remove the extension
   * headers and change a few fields
   */
  PRINTF("Received Echo Request from ");
  PRINT6ADDR(&UIP_IP_BUF->srcipaddr);
  PRINTF(" to ");
  PRINT6ADDR(&UIP_IP_BUF->destipaddr);
  PRINTF("\n");

  /* IP header */
  UIP_IP_BUF->ttl = uip_ds6_if.cur_hop_limit;

  if(uip_is_addr_mcast(&UIP_IP_BUF->destipaddr)){
    uip_ipaddr_copy(&UIP_IP_BUF->destipaddr, &UIP_IP_BUF->srcipaddr);
    uip_ds6_select_src(&UIP_IP_BUF->srcipaddr, &UIP_IP_BUF->destipaddr);
  } else {
    uip_ipaddr_copy(&tmp_ipaddr, &UIP_IP_BUF->srcipaddr);
    uip_ipaddr_copy(&UIP_IP_BUF->srcipaddr, &UIP_IP_BUF->destipaddr);
    uip_ipaddr_copy(&UIP_IP_BUF->destipaddr, &tmp_ipaddr);
  }

  if(uip_ext_len > 0) {
    /* Remove extension headers if any */
    UIP_IP_BUF->proto = UIP_PROTO_ICMP6;
    uip_len -= uip_ext_len;
    UIP_IP_BUF->len[0] = ((uip_len - UIP_IPH_LEN) >> 8);
    UIP_IP_BUF->len[1] = ((uip_len - UIP_IPH_LEN) & 0xff);
    /* move the echo request payload (starting after the icmp header)
     * to the new location in the reply.
     * The shift is equal to the length of the extension headers present
     * Note: UIP_ICMP_BUF still points to the echo request at this stage
     */
    memmove((uint8_t *)UIP_ICMP_BUF + UIP_ICMPH_LEN - uip_ext_len,
        (uint8_t *)UIP_ICMP_BUF + UIP_ICMPH_LEN,
        (uip_len - UIP_IPH_LEN - UIP_ICMPH_LEN));
    uip_ext_len = 0;
  }

  /* Below is important for the correctness of UIP_ICMP_BUF and the
   * checksum
   */

  /* Note: now UIP_ICMP_BUF points to the beginning of the echo reply */
  UIP_ICMP_BUF->type = ICMP6_ECHO_REPLY;
  UIP_ICMP_BUF->icode = 0;
  UIP_ICMP_BUF->icmpchksum = 0;
  UIP_ICMP_BUF->icmpchksum = ~uip_icmp6chksum();

  PRINTF("Sending Echo Reply to ");
  PRINT6ADDR(&UIP_IP_BUF->destipaddr);
  PRINTF(" from ");
  PRINT6ADDR(&UIP_IP_BUF->srcipaddr);
  PRINTF("\n");
  UIP_STAT(++uip_stat.icmp.sent);
  return;
}
/*---------------------------------------------------------------------------*/
void
uip_icmp6_error_output(uint8_t type, uint8_t code, uint32_t param) {
  /* check if originating packet is not an ICMP error */
  if(uip_ext_len) {
    if(UIP_EXT_BUF->next == UIP_PROTO_ICMP6 && UIP_ICMP_BUF->type < 128) {
      uip_clear_buf();
      return;
    }
  } else {
    if(UIP_IP_BUF->proto == UIP_PROTO_ICMP6 && UIP_ICMP_BUF->type < 128) {
      uip_clear_buf();
      return;
    }
  }

#if UIP_CONF_IPV6_RPL
  rpl_remove_header();
#else
  uip_ext_len = 0;
#endif /* UIP_CONF_IPV6_RPL */

  /* remember data of original packet before shifting */
  uip_ipaddr_copy(&tmp_ipaddr, &UIP_IP_BUF->destipaddr);

  uip_len += UIP_IPICMPH_LEN + UIP_ICMP6_ERROR_LEN;

  if(uip_len > UIP_LINK_MTU) {
    uip_len = UIP_LINK_MTU;
  }

  memmove((uint8_t *)UIP_ICMP6_ERROR_BUF + uip_ext_len + UIP_ICMP6_ERROR_LEN,
          (void *)UIP_IP_BUF, uip_len - UIP_IPICMPH_LEN - uip_ext_len - UIP_ICMP6_ERROR_LEN);

  UIP_IP_BUF->vtc = 0x60;
  UIP_IP_BUF->tcflow = 0;
  UIP_IP_BUF->flow = 0;
  if (uip_ext_len) {
    UIP_FIRST_EXT_BUF->next = UIP_PROTO_ICMP6;
  } else {
    UIP_IP_BUF->proto = UIP_PROTO_ICMP6;
  }
  UIP_IP_BUF->ttl = uip_ds6_if.cur_hop_limit;

  /* the source should not be unspecified nor multicast, the check for
     multicast is done in uip_process */
  if(uip_is_addr_unspecified(&UIP_IP_BUF->srcipaddr)){
    uip_clear_buf();
    return;
  }

  uip_ipaddr_copy(&UIP_IP_BUF->destipaddr, &UIP_IP_BUF->srcipaddr);

  if(uip_is_addr_mcast(&tmp_ipaddr)){
    if(type == ICMP6_PARAM_PROB && code == ICMP6_PARAMPROB_OPTION){
      uip_ds6_select_src(&UIP_IP_BUF->srcipaddr, &tmp_ipaddr);
    } else {
      uip_clear_buf();
      return;
    }
  } else {
#if UIP_CONF_ROUTER
    /* need to pick a source that corresponds to this node */
    uip_ds6_select_src(&UIP_IP_BUF->srcipaddr, &tmp_ipaddr);
#else
    uip_ipaddr_copy(&UIP_IP_BUF->srcipaddr, &tmp_ipaddr);
#endif
  }

  UIP_ICMP_BUF->type = type;
  UIP_ICMP_BUF->icode = code;
  UIP_ICMP6_ERROR_BUF->param = uip_htonl(param);
  UIP_IP_BUF->len[0] = ((uip_len - UIP_IPH_LEN) >> 8);
  UIP_IP_BUF->len[1] = ((uip_len - UIP_IPH_LEN) & 0xff);
  UIP_ICMP_BUF->icmpchksum = 0;
  UIP_ICMP_BUF->icmpchksum = ~uip_icmp6chksum();

  UIP_STAT(++uip_stat.icmp.sent);

  PRINTF("Sending ICMPv6 ERROR message type %d code %d to ", type, code);
  PRINT6ADDR(&UIP_IP_BUF->destipaddr);
  PRINTF(" from ");
  PRINT6ADDR(&UIP_IP_BUF->srcipaddr);
  PRINTF("\n");
  return;
}

/*---------------------------------------------------------------------------*/
uint8_t rpl_pkt_type = 0xFF;
uint8_t rpl_pkt_code = 0xFF;
void
uip_icmp6_send(const uip_ipaddr_t *dest, int type, int code, int payload_len,int hop_limit)
{
  UIP_IP_BUF->vtc = 0x60;
  UIP_IP_BUF->tcflow = 0;
  UIP_IP_BUF->flow = 0;
  UIP_IP_BUF->proto = UIP_PROTO_ICMP6;
  UIP_IP_BUF->ttl = hop_limit;//uip_ds6_if.cur_hop_limit;
  UIP_IP_BUF->len[0] = (UIP_ICMPH_LEN + payload_len) >> 8;
  UIP_IP_BUF->len[1] = (UIP_ICMPH_LEN + payload_len) & 0xff;

  memcpy(&UIP_IP_BUF->destipaddr, dest, sizeof(*dest));
  rpl_pkt_type = type;
  rpl_pkt_code = code;  //Suneet :: what type pkt send 
  packetbuf_set_attr(PACKETBUF_ATTR_PACKET_TYPE,code);
  uip_ds6_select_src(&UIP_IP_BUF->srcipaddr, &UIP_IP_BUF->destipaddr);

  UIP_ICMP_BUF->type = type;
  UIP_ICMP_BUF->icode = code;

  UIP_ICMP_BUF->icmpchksum = 0;
  UIP_ICMP_BUF->icmpchksum = ~uip_icmp6chksum();

  uip_len = UIP_IPH_LEN + UIP_ICMPH_LEN + payload_len;
  
  if(uip_is_addr_mpl_mcast_forwarders(&UIP_IP_BUF->destipaddr)
     || (uip_is_addr_mpl_mcast_forwarders_for_non_group(&UIP_IP_BUF->destipaddr))
       ||(uip_is_mcast_group_id_all_nodes(&UIP_IP_BUF->destipaddr)))
  {
#if (UIP_MCAST6_CONF_ENGINE == UIP_MCAST6_ENGINE_ROLL_TM)    
    UIP_MCAST6.out();//santosh, multicast
#endif    
  }
  else
  {
    UIP_STAT(++uip_stat.icmp.sent);
    UIP_STAT(++uip_stat.ip.sent);

    tcpip_ipv6_output();    
  }   
}

void wan_uip_icmp6_send (const uip_ipaddr_t *dest, const uip_ipaddr_t *src, int type, int code, int payload_len,int hop_limit)
{
  UIP_IP_BUF->vtc = 0x60;
  UIP_IP_BUF->tcflow = 0;
  UIP_IP_BUF->flow = 0;
  UIP_IP_BUF->proto = UIP_PROTO_ICMP6;
  UIP_IP_BUF->ttl = hop_limit;//uip_ds6_if.cur_hop_limit;
  UIP_IP_BUF->len[0] = (UIP_ICMPH_LEN + payload_len) >> 8;
  UIP_IP_BUF->len[1] = (UIP_ICMPH_LEN + payload_len) & 0xff;

  memcpy (&UIP_IP_BUF->destipaddr, dest, sizeof(*dest));
  memcpy (&UIP_IP_BUF->srcipaddr, src, sizeof(*src));
  packetbuf_set_attr(PACKETBUF_ATTR_PACKET_TYPE,code);
//  uip_ds6_select_src(&UIP_IP_BUF->srcipaddr, &UIP_IP_BUF->destipaddr);

  UIP_ICMP_BUF->type = type;
  UIP_ICMP_BUF->icode = code;

  UIP_ICMP_BUF->icmpchksum = 0;
  UIP_ICMP_BUF->icmpchksum = ~uip_icmp6chksum();

  uip_len = UIP_IPH_LEN + UIP_ICMPH_LEN + payload_len;
  
  if(uip_is_addr_mpl_mcast_forwarders(&UIP_IP_BUF->destipaddr)
     || (uip_is_addr_mpl_mcast_forwarders_for_non_group(&UIP_IP_BUF->destipaddr)))
  {
#if (UIP_MCAST6_CONF_ENGINE == UIP_MCAST6_ENGINE_ROLL_TM)    
    UIP_MCAST6.out();//santosh, multicast
#endif    
  }
  else
  {
    UIP_STAT(++uip_stat.icmp.sent);
    UIP_STAT(++uip_stat.ip.sent);

    tcpip_ipv6_output();    
  }   
}
/*---------------------------------------------------------------------------*/
static void
echo_reply_input(void)
{
  int ttl;
  uip_ipaddr_t sender;

  PRINTF("Received Echo Reply from ");
  PRINT6ADDR(&UIP_IP_BUF->srcipaddr);
  PRINTF(" to ");
  PRINT6ADDR(&UIP_IP_BUF->destipaddr);
  PRINTF("\n");

  uip_ipaddr_copy(&sender, &UIP_IP_BUF->srcipaddr);
  ttl = UIP_IP_BUF->ttl;

  if(uip_ext_len > 0) {
    /* Remove extension headers if any */
    UIP_IP_BUF->proto = UIP_PROTO_ICMP6;
    uip_len -= uip_ext_len;
    UIP_IP_BUF->len[0] = ((uip_len - UIP_IPH_LEN) >> 8);
    UIP_IP_BUF->len[1] = ((uip_len - UIP_IPH_LEN) & 0xff);
    /* move the echo reply payload (starting after the icmp header)
     * to the new location in the reply.  The shift is equal to the
     * length of the extension headers present Note: UIP_ICMP_BUF
     * still points to the echo request at this stage
     */
    memmove((uint8_t *)UIP_ICMP_BUF + UIP_ICMPH_LEN - uip_ext_len,
        (uint8_t *)UIP_ICMP_BUF + UIP_ICMPH_LEN,
        (uip_len - UIP_IPH_LEN - UIP_ICMPH_LEN));
    uip_ext_len = 0;
  }

  /* Call all registered applications to let them know an echo reply
     has been received. */
  {
    struct uip_icmp6_echo_reply_notification *n;
    for(n = l3_list_head(echo_reply_callback_list);
        n != NULL;
        n = l3_list_item_next(n)) {
      if(n->callback != NULL) {
        n->callback(&sender, ttl,
                    (uint8_t *)&UIP_ICMP_BUF[sizeof(struct uip_icmp_hdr)],
                    uip_len - sizeof(struct uip_icmp_hdr) - UIP_IPH_LEN);
      }
    }
  }

  uip_clear_buf();
  return;
}
/*---------------------------------------------------------------------------*/
void
uip_icmp6_echo_reply_callback_add(struct uip_icmp6_echo_reply_notification *n,
                                  uip_icmp6_echo_reply_callback_t c)
{
  if(n != NULL && c != NULL) {
    n->callback = c;
    l3_list_add(echo_reply_callback_list, n);
  }
}
/*---------------------------------------------------------------------------*/
void
uip_icmp6_echo_reply_callback_rm(struct uip_icmp6_echo_reply_notification *n)
{
  l3_list_remove(echo_reply_callback_list, n);
}
/*---------------------------------------------------------------------------*/
UIP_ICMP6_HANDLER(echo_request_handler, ICMP6_ECHO_REQUEST,
                  UIP_ICMP6_HANDLER_CODE_ANY, echo_request_input);
UIP_ICMP6_HANDLER(echo_reply_handler, ICMP6_ECHO_REPLY,
                  UIP_ICMP6_HANDLER_CODE_ANY, echo_reply_input);
/*---------------------------------------------------------------------------*/
void
uip_icmp6_init()
{
  /* Register Echo Request and Reply handlers */
  uip_icmp6_register_input_handler(&echo_request_handler);
  uip_icmp6_register_input_handler(&echo_reply_handler);
}
/*---------------------------------------------------------------------------*/
//void net_debug_lladdr_print(const uip_lladdr_t *addr)
//{
//#if DEBUG
//  PRINTLLADDR(addr);
//#endif
//}
//extern unsigned short
//l3_random_rand(void);
//static void create_rand_delay_in_us()
//{
//  unsigned short rand_val = l3_random_rand();
//  tmr_delay(rand_val);
//}
/** @} */
