/** \file AppPING6Process.c
 *******************************************************************************
 ** \brief 
 **
 ** \cond 
 **
 ** COPYRIGHT(c) 2023-24 Procubed Innovations Pvt Ltd.
 ** All rights reserved.
 **
 ** THIS SOFTWARE IS PROVIDED BY "AS IS" AND ALL WARRANTIES OF ANY KIND,
 ** INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR USE,
 ** ARE EXPRESSLY DISCLAIMED.  THE DEVELOPER SHALL NOT BE LIABLE FOR ANY
 ** DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE. THIS SOFTWARE
 ** MAY NOT BE USED IN PRODUCTS INTENDED FOR USE IN IMPLANTATION OR OTHER
 ** DIRECT LIFE SUPPORT APPLICATIONS WHERE MALFUNCTION MAY RESULT IN THE DIRECT
 ** PHYSICAL HARM OR INJURY TO PERSONS. ALL SUCH IS USE IS EXPRESSLY PROHIBITED.
 **
 *******************************************************************************
 **  \endcond
 */
/*
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
#include "StackAppConf.h"
#include "common.h"
#include "l3_configuration.h"
#include "list_latest.h"
#include "queue_latest.h"
#include "contiki-net.h"
#include "uart_hal.h"
#include "hif_utility.h"
#include "hif_service.h"
#include "buff_mgmt.h"
#include "buffer_service.h"
#include "mac.h"
#include <string.h>
#include <stdio.h>
#include "ie_element_info.h"
#include "fan_app_test_harness.h"
#include "AppUDPprocess.h"
#include "fan_app_auto.h"
#include "fan_api.h"
#include "sm.h"
#include "fan_mac_interface.h"

/*----------------------------------------------------------------------------*/

//#if AUTO_CONFIG_ENABLE
extern uint8_t temp_buf_set;
//#endif

#define PING6_NB                1
#define PING6_DATALEN           (echo_config.echo_data_len)

#define UIP_IP_BUF              ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])
#define UIP_ICMP_BUF            ((struct uip_icmp_hdr *)&uip_buf[uip_l2_l3_hdr_len])


#define SEND_ONLY_FIRST_FRAGMENT                0x01
#define SEND_WRONG_CHECKSUM                     0x02
#define SEND_WRONG_ICMPv6_TYPE                  0x03
#define SEND_WRONG_PROTO                        0x04
#define SEND_ICMP_WITH_KNOWN_ID_SEQ_NUM         5
#define ECHO_PKT_IDENTIFIER                     0x1212

/*----------------------------------------------------------------------------*/
static l3_etimer_t ping6_periodic_timer;
static l3_ctimer_t echo_send_ctimer; 
uint16_t icmv6_counter = 0x0000;
uint16_t icmv6_response_counter = 0x00;
struct uip_icmp_hdr* p_icmp_packet = NULL;
static uint16_t ping_seq_number = 0;
typedef struct echo_request_config_tag
{
    uip_ipaddr_t src_ip_addr;
    uip_ipaddr_t dest_ip_addr;
    uint16_t echo_data_len;
    uint8_t *p_echo_data;
    uint16_t counter;
    uint8_t interval;
    uint8_t hop_limit;
    uint16_t identifier;                //Arjun: 23-03-18: to be set from tool as says swagger API
    uint16_t sequence_number;
}echo_request_config_t;
echo_request_config_t echo_config;

/*******************************************************************************/

static void process_timer_expiry(void);
static void process_echo( void );
static void process_dest_unreachable_msg(void);
static void process_para_problem_msg( void );
static uint8_t process_nans( void );
void echo_send_ctimer_callback(void* ptr);
void uip_icmp6_send_echo_request(void);
void send_icmpv6(uint8_t *buf, uint16_t length);
void send_icmpv6_with_count(uint8_t *buf, uint16_t length);
void send_icmv6_conf(void);

#if(FAN_EDFE_FEATURE_ENABLED == 1)
void send_status_to_edfe_fragment(uint8_t status);
void trigger_edfe_final_frame();
extern void enable_disable_edfe_frame(uint8_t value,uint8_t edfe_frame_type);
extern edfe_info_t edfe_information;  
#endif

uint8_t gu2pcapp_send_na_ind
(
 uint8_t type,
 uint8_t code,
 uint16_t checksum,
 uint8_t osr_flag,
 uip_ipaddr_t* p_tgt_addr,
 uint8_t icmpv6_opt_type,
 uint8_t icmpv6_opt_len,
 uip_lladdr_t* p_tlla
);

uint8_t gu2pcapp_send_ns_ind
(
 uint8_t type,
 uint8_t code,
 uint16_t checksum,
 uip_ipaddr_t* p_tgt_addr,
 uint8_t icmpv6_opt_type,
 uint8_t icmpv6_opt_len,
 uip_lladdr_t* p_tlla
);

uint8_t gu2pcapp_send_echo_req_res
(
 uint8_t type,
 uint8_t code,
 uint16_t checksum,
 uint16_t id,
 uint16_t seq_num,
 uint16_t data_len,        
 uint8_t* p_data
);

uint8_t gu2pcapp_send_dest_unreachable_icmp_ind
(
 uint8_t type,
 uint8_t code,
 uint16_t checksum,
 uint16_t payload_len,
 uint8_t* p_payload
);

uint8_t gu2pcapp_process_para_problem_msg_ind
(
 uint8_t type,
 uint8_t code,
 uint16_t checksum,
 uint16_t payload_len,
 uint8_t* p_payload
);
/*----------------------------------------------------------------------------*/
L3_PROCESS(ping6_process, "PING6 process");
/*----------------------------------------------------------------------------*/
static enet_msg_t* siclowpanPakcets[20];
static uint8_t siclowpanPakcetsIndex = 0;
//static uint8_t num_sicslowpan_pkts = 0;
extern uint8_t response_laye_ID;
extern enet_msg_t* allocate_hif_msg( uint16_t length );
extern void free_hif_msg( enet_msg_t * msgp );
extern uint8_t capture_nans_and_send;
//int __nbr;
extern uint8_t get_upper_layer_status();
extern uint8_t* get_self_address(void);
extern void uip_icmp6_send(const uip_ipaddr_t *dest, int type, int code, int payload_len,int hop_limit);
extern void wan_uip_icmp6_send (const uip_ipaddr_t *dest, const uip_ipaddr_t *src, int type, int code, int payload_len,int hop_limit);
//extern void red_led_off(void);
extern uint8_t* get_self_global_addr();
extern void mem_rev_cpy(uint8_t* dest, uint8_t* src, uint16_t len );

extern void * app_bm_alloc(
    uint16_t length//base_t length      
    );
    
extern void app_bm_free(
    uint8_t *pMem      
    );

void send_edfe_initial_frame(uint8_t *src_addr , uint8_t value,uint8_t edfe_frame_type);
extern uint16_t max_frag_size;

/*----------------------------------------------------------------------------*/
//static void queue_sicslowpan_packet( enet_msg_t* msg )
//{
//  siclowpanPakcets[siclowpanPakcetsIndex++] = msg;
//  if( siclowpanPakcetsIndex == 20 )
//  {
//    siclowpanPakcetsIndex = 0;
//  } 
//}
/*----------------------------------------------------------------------------*/
void send_all_queued_sicslowpan_packets(void)
{
#ifdef SEND_SICSLOWPAN_PKT_TO_HIF
    uint8_t index = 0;
    enet_msg_t* msg = NULL;
    for( index=0; index<siclowpanPakcetsIndex; index++ )
    {
      msg = siclowpanPakcets[index];
      if( msg != NULL )
      {
        hif_send_msg_up(msg->data,msg->data_length-1,response_laye_ID,PROTOCOL_ID_FOR_APP);  
        free_hif_msg(msg);
        siclowpanPakcets[index] = NULL;
      }
    }
    siclowpanPakcetsIndex = 0;
#endif
}
/*---------------------------------------------------------------------------*/
static uint8_t ping_event_handler(l3_process_event_t ev, l3_process_data_t data)
{
    if ( ev == L3_PROCESS_EVENT_TIMER )
    {
      if(data == &ping6_periodic_timer &&
      l3_etimer_expired(&ping6_periodic_timer)) 
      {
        process_timer_expiry();                         
      }
    }
    else if ( ev == tcpip_icmp6_event )
    {
      if(( *(uint8_t *)data == ICMP6_ECHO_REPLY )|| ( *(uint8_t *)data == ICMP6_ECHO_REQUEST ))
      {        
        process_echo();
      }
      else if ( (*(uint8_t *)data == ICMP6_NA ) || (*(uint8_t *)data == ICMP6_NS ) )
      {
        process_nans();
      } 
      else if ((*(uint8_t *)data) == ICMP6_DST_UNREACH)
      {
        process_dest_unreachable_msg();
      }
      else if ( (*(uint8_t *)data) == ICMP6_PARAM_PROB )
      {
        process_para_problem_msg();
      }
    }
    return 0;
}
/*----------------------------------------------------------------------------*/
static void process_timer_expiry(void)
{
    /* set identifier and sequence number to 0 */
    memset((uint8_t *)UIP_ICMP_BUF + UIP_ICMPH_LEN, 0x00, 4);     
    /* reset icmpv6 data */
    memset((uint8_t *)UIP_ICMP_BUF + UIP_ICMPH_LEN + 2/*id*/ + 2/*seqno*/,
            0x00, PING6_DATALEN);  
    
    /* copy icmpv6 data */
    memcpy((uint8_t *)UIP_ICMP_BUF + UIP_ICMPH_LEN + 2/*id*/ + 2/*seqno*/, 
           echo_config.p_echo_data, echo_config.echo_data_len);    

    uip_icmp6_send(&(echo_config.dest_ip_addr), ICMP6_ECHO_REQUEST, 0x00/*code*/, 
                   echo_config.echo_data_len + 2/*id*/ + 2/*seqno*/,echo_config.hop_limit);
    
    l3_etimer_stop( &ping6_periodic_timer );
    return;
}
/*----------------------------------------------------------------------------*/
L3_PROCESS_THREAD(ping6_process, ev, data)
{
  L3_PROCESS_BEGIN();
  PRINTF("In Process PING6\n");
  PRINTF("Wait for DAD\n");
  icmp6_new(NULL);
  echo_config.sequence_number = 0x00;
  echo_config.identifier = ECHO_PKT_IDENTIFIER;
  while(1) {
    L3_PROCESS_YIELD();
    ping_event_handler(ev, data);
  }

  PRINTF("END PING6\n");
  L3_PROCESS_END();
}
/*----------------------------------------------------------------------------*/ 
void trigger_echo_request( uint8_t* p_buff  )
{
#if 0
    uint16_t index = 0;
    uip_lladdr_t ll_add;
    enet_msg_t* msg = NULL;
    rpl_dag_t *dag;
    uint16_t prefix_id;
    uint8_t addr_opt = p_buff[3];
    index += 4;
    uint8_t* self_ieee_addr = get_self_address();
    
    if( p_buff != NULL ){
      
      dag = rpl_get_any_dag();//current dag
      if(dag == NULL)
        return;
      
      if(addr_opt == 0x00)//global
        prefix_id = UIP_HTONS(0x2001);

      if(addr_opt == 0x01)//link local
        prefix_id = UIP_HTONS(0xfe80);

      if(addr_opt == 0x02){//multicast
        uip_create_linklocal_rplnodes_mcast(&(echo_config.dest_ip_addr));
        index += 8;
        goto send;
      }
      
      memcpy(ll_add.addr,&(p_buff[index]),8);
      index += 8;
                    
      if(!memcmp(ll_add.addr, self_ieee_addr, 8))
        return;//requesting to self address error
      
      uip_create_prefix(&(echo_config.dest_ip_addr), prefix_id);
      uip_ds6_set_addr_iid(&(echo_config.dest_ip_addr), &ll_add);              
 
send:      
      //echo_config.echo_data_len = get_ushort_BE(&(p_buff[index]));
      //(((ushort)*((bp)+1))+((ushort)(*(bp))<<8))
      echo_config.echo_data_len = p_buff[index++];
      echo_config.echo_data_len = echo_config.echo_data_len<<8;
      echo_config.echo_data_len |=  p_buff[index++];
      msg = allocate_hif_msg( echo_config.echo_data_len );
      uint8_t* buf = msg->data;	

      if( msg != NULL )
      {
        // copy echo data to be transmitted
        memcpy(buf,&(p_buff[index]),echo_config.echo_data_len);       
        echo_config.p_echo_data = msg;
      }
      index += echo_config.echo_data_len;
      
      //currently not using for FAN
      echo_config.echo_data_pattern = 0x00;
      echo_config.err_config_id = 0x00;
      echo_config.err_config_data_len = 0x00;      
      L3_PROCESS_CONTEXT_BEGIN(&ping6_process);
      l3_etimer_set(&ping6_periodic_timer, 1*CLOCK_SECOND);
      L3_PROCESS_CONTEXT_END(&ping6_process);
    } 
#endif    
} 
/*----------------------------------------------------------------------------*/

static void process_dest_unreachable_msg(void)
{
    send_all_queued_sicslowpan_packets();
    p_icmp_packet = UIP_ICMP_BUF;
    gu2pcapp_send_dest_unreachable_icmp_ind
                                          (
                                          p_icmp_packet->type, //type
                                          p_icmp_packet->icode,//uint8_t code, 
                                          p_icmp_packet->icmpchksum,//uint16_t checksum
                                          0,
                                          NULL
                                          );
}
/*----------------------------------------------------------------------------*/
static void process_para_problem_msg( void )
{
    send_all_queued_sicslowpan_packets();
    p_icmp_packet = UIP_ICMP_BUF;
    gu2pcapp_process_para_problem_msg_ind(
                                          p_icmp_packet->type, //type
                                          p_icmp_packet->icode,//uint8_t code, 
                                          p_icmp_packet->icmpchksum,//uint16_t checksum
                                          0,
                                          NULL
                                          );
}
/*----------------------------------------------------------------------------*/
uint8_t get_uncomp_hdr_len (void);
static void process_echo( void )
{
    uint16_t echo_id = 0;
    uint16_t echo_seq_num = 0;
#ifdef ENET_GOLDEN_UNIT
    send_all_queued_sicslowpan_packets();
#endif
    p_icmp_packet = UIP_ICMP_BUF;
    echo_id = get_ushort_BE((((uint8_t*)p_icmp_packet)+4));
    echo_seq_num = get_ushort_BE((((uint8_t*)p_icmp_packet)+6));
         
    gu2pcapp_send_echo_req_res(
                              p_icmp_packet->type, //type
                              p_icmp_packet->icode,//uint8_t code, 
                              p_icmp_packet->icmpchksum,//uint16_t checksum
                              echo_id,
                              echo_seq_num,
                              ( uip_len - (sizeof(struct uip_icmp_hdr) + 4 + get_uncomp_hdr_len() ) ),
                              (((uint8_t*)p_icmp_packet)+8)
                              );
    
    if( p_icmp_packet->type == 0x81)
    {
#if(FAN_EDFE_FEATURE_ENABLED == 1)
      if((edfe_information.edfe_frame_enabled == 0x01) && (echo_config.counter == 0x00))
      {
        enable_disable_edfe_frame(1,FINAL_RESPONSE_FRAME);
        send_edfe_initial_frame(edfe_information.edfe_ini_mac_addr ,1,FINAL_RESPONSE_FRAME);
        echo_config.counter  = 0x00;
        edfe_information.edfe_trigger_packt = 0x00;
      }
#endif
    }
    //red_led_off();  
}
/*----------------------------------------------------------------------------*/
static uint8_t process_nans( void )
{
    send_all_queued_sicslowpan_packets();    
    p_icmp_packet = UIP_ICMP_BUF;

    if(p_icmp_packet->type == ICMP6_NA)
    {
      gu2pcapp_send_na_ind(p_icmp_packet->type,
                          p_icmp_packet->icode,
                          p_icmp_packet->icmpchksum,
                          *(((uint8_t*)p_icmp_packet)+4),
                          (uip_ipaddr_t*) (((uint8_t*)p_icmp_packet)+8),
                          *(((uint8_t*)p_icmp_packet)+24),
                          *(((uint8_t*)p_icmp_packet)+25),
                          (uip_lladdr_t*) (((uint8_t*)p_icmp_packet)+26)
                          );
    }
    else
    {
      gu2pcapp_send_ns_ind(p_icmp_packet->type,
                          p_icmp_packet->icode,
                          p_icmp_packet->icmpchksum,
                          //*(((uint8_t*)p_icmp_packet)+4),
                          (uip_ipaddr_t*) (((uint8_t*)p_icmp_packet)+8),
                          *(((uint8_t*)p_icmp_packet)+24),
                          *(((uint8_t*)p_icmp_packet)+25),
                          (uip_lladdr_t*) (((uint8_t*)p_icmp_packet)+26)
                          );
    }    

    return 0;
}

/*---------------------------------------------------------------------------*/
//Suneet :: send ping when external intrrupt is trigger
void explicit_trigger_echo_request(uip_ipaddr_t* dest_ip_addr){

    uint16_t datalen = 1232;
    uint8_t data[MAX_UDP_LENGTH_SUPPORT/*1024*/] = {0x00};     

    if(datalen > MAX_UDP_LENGTH_SUPPORT)
      return;
   
//#if AUTO_CONFIG_ENABLE      // Raka [ 02- Dec-2017]
    memset(data, temp_buf_set, datalen);
    data[datalen-1] = 0x2f;//for getting the last byte
    temp_buf_set+=1;
//#else    
//    memset(data, 0xaa, datalen);
//    data[datalen-1] = 0x2f;//for getting the last byte    
//#endif
        
    if( dest_ip_addr != NULL ){    
      memcpy(&(echo_config.dest_ip_addr), (uip_ipaddr_t*)dest_ip_addr, 16);     

      echo_config.echo_data_len = datalen;

      echo_config.p_echo_data = app_bm_alloc( echo_config.echo_data_len );
      //copy echo data to be transmitted
      memcpy(echo_config.p_echo_data, data, echo_config.echo_data_len);       

      L3_PROCESS_CONTEXT_BEGIN(&ping6_process);
      //10*CLOCK_SECOND: for 28 hops in 50Kbps, FAN_TPS1v07
      l3_etimer_set(&ping6_periodic_timer, 10*CLOCK_SECOND);
      L3_PROCESS_CONTEXT_END(&ping6_process);       
    }
    return;
}
/*----------------------------------------------------------------------------*/
void send_icmpv6(uint8_t *buf, uint16_t length)
{        
  memset(&echo_config, 0x00, sizeof(echo_config) - 4);
  icmv6_counter = 0x00;
  icmv6_response_counter = 0x00;
  uint16_t index=0;
  uint8_t tag = 0;
  uint8_t len = 0;
  
  while (length)
  {
    tag = *buf;
    length -= 1;
    buf += 1;
    switch (tag)
    {
    case ICMPV6_PING_SRC_ADDR:
      len = *buf;
      length -= 1;
      buf += 1;
      memcpy(&echo_config.src_ip_addr.u8, buf, len);
      length -= len;
      buf += len;
      break;
      
    case ICMPV6_PING_DST_ADDR:
      len = *buf;
      length -= 1;
      buf += 1;
      memcpy(&echo_config.dest_ip_addr.u8, buf, len);
      length -= len;
      buf += len;
      break;
      
    case ICMPV6_PING_HOP_LIMIT:
      len = *buf;
      length -= 1;
      buf += 1;
      if (len == 1)
        echo_config.hop_limit = *buf;
      length -= len;
      buf += len;
      break;
      
    case ICMPV6_PING_ECHO_DATA:
      mem_rev_cpy ((uint8_t *)&echo_config.echo_data_len, buf, 2);
      length -= 2;
      buf += 2;
      echo_config.p_echo_data = app_bm_alloc (echo_config.echo_data_len);
      if (echo_config.p_echo_data != NULL)
        memcpy (echo_config.p_echo_data, buf, echo_config.echo_data_len);
      length -= echo_config.echo_data_len;
      buf += echo_config.echo_data_len;
      break;
        
    case ICMPV6_PING_FRAME_EXCHANGE_PATTERN:
      len = *buf;
      length -= 1;
      buf += 1;
      if (len == 1)
      {
        if (*buf == 0x01)
        {
#if(FAN_EDFE_FEATURE_ENABLED == 1)
          if(echo_config.echo_data_len > max_frag_size)
          {
            edfe_information.is_fragmented = 0x001;
            enable_disable_edfe_frame(buf[index],0);//frameExchangePattern
          }
          else
          {
            enable_disable_edfe_frame(buf[index],0);//frameExchangePattern
            edfe_information.edfe_trigger_packt = 0x01;
            edfe_information.edfe_sent_pkt = 0x01;
            echo_config.counter = 0x01;
          }
#else
          echo_config.counter = 0x01;
#endif
        }
      }
      length -= len;
      buf += len;
      break;
      
    case ICMPV6_PING_IDENTIFIER:
      len = *buf;
      length -= 1;
      buf += 1;
      if (len == 2)
        memcpy((uint8_t *)&echo_config.identifier, buf, 2);
      length -= len;
      buf += len;
      break;
      
    case ICMPV6_PING_SEQUENCE_NUMBER:
      len = *buf;
      length -= 1;
      buf += 1;
      if (len == 2)
        memcpy((uint8_t *)&echo_config.sequence_number, buf, 2);
      length -= len;
      buf += len;
      break;
    }
  }
  
  uip_ipaddr_t* src_ipaddr = (uip_ipaddr_t*)get_self_global_addr();
  if(!memcmp(&src_ipaddr,&echo_config.src_ip_addr, 16))
    return;//requesting to self address error           
  
  //sending echo request
  uip_icmp6_send_echo_request();

#if(FAN_EDFE_FEATURE_ENABLED == 1)
  edfe_information.edfe_trigger_packt--;
  edfe_information.edfe_sent_pkt--;
#endif
  
  echo_config.counter--;
  app_bm_free((uint8_t*)echo_config.p_echo_data);
  return;
}


void send_icmpv6_with_count(uint8_t *buf, uint16_t length)
{
  memset(&echo_config, 0x00,  sizeof(echo_config) - 4);
  icmv6_counter = 0x00;
  icmv6_response_counter = 0x00;
  uint16_t index=0;    
  memcpy(&echo_config.src_ip_addr.u8, &buf[index], 16);
  index+=16;//16
  
  memcpy(&echo_config.dest_ip_addr.u8, &buf[index], 16);
  index+=16;//32
  
   echo_config.hop_limit = 0xFF;
  
  // uip_ds6_nbr_t *nbr = nbr_table_head(ds6_neighbors);
  uip_ipaddr_t* src_ipaddr = (uip_ipaddr_t*)get_self_global_addr();
  if(!memcmp(&src_ipaddr,&echo_config.dest_ip_addr, 16))
    return;//requesting to self address error
  
  mem_rev_cpy((uint8_t *)&echo_config.counter, &buf[index], 2);
  index+=2;//34

#if(FAN_EDFE_FEATURE_ENABLED == 1)
  edfe_information.edfe_trigger_packt  = echo_config.counter;
#endif
  
  echo_config.interval = buf[index];
  index+=1;//35        
  
  if(buf[index] == 0x01)
  {
#if(FAN_EDFE_FEATURE_ENABLED == 1)
    enable_disable_edfe_frame(buf[index],0);//frameExchangePattern
#endif
  }
  index+=1;//36  
  mem_rev_cpy((uint8_t *)&echo_config.echo_data_len, &buf[index], 2);
  
  index+=2;
  if( echo_config.counter == 0 || echo_config.counter >= 0xFFFF ||
     echo_config.interval == 0 || echo_config.interval >= 0xFF)
    return;//sending fail status
  
  echo_config.p_echo_data = app_bm_alloc( echo_config.echo_data_len );
  if(echo_config.p_echo_data == NULL)
  {
    return;
  }
  memcpy(echo_config.p_echo_data, &buf[index], echo_config.echo_data_len);//NEED DYNAMIC ALLOCATION                
  //sending echo request
  uip_icmp6_send_echo_request();
  ping_seq_number++;
  echo_config.sequence_number = (((0xFF00 & ping_seq_number) >> 8) | ((0x00FF & ping_seq_number) << 8));
  if(ping_seq_number == 0xFFFF)
  {
    ping_seq_number = 0;
  }
  echo_config.counter--;
  //gu2pcapp_node_config_set_conf(0x00);
  send_icmv6_conf();
  if(echo_config.counter)
  {
    l3_ctimer_set(&echo_send_ctimer, echo_config.interval*CLOCK_SECOND, 
               echo_send_ctimer_callback, NULL);
  }
  
  return;
}
/*---------------------------------------------------------------------------*/
void echo_send_ctimer_callback(void* ptr)
{  
  //sending echo request
  uip_icmp6_send_echo_request();
  ping_seq_number++;
  echo_config.sequence_number = (((0xFF00 & ping_seq_number) >> 8) | ((0x00FF & ping_seq_number) << 8));
  if(ping_seq_number == 0xFFFF)
  {
    ping_seq_number = 0;
  }      
  echo_config.counter--;    
  //gu2pcapp_node_config_set_conf(0x00);    
  send_icmv6_conf();
  if(echo_config.counter)
  {
    l3_ctimer_set(&echo_send_ctimer, echo_config.interval*CLOCK_SECOND, 
               echo_send_ctimer_callback, NULL);      
  }
  else
  {
    l3_ctimer_stop(&echo_send_ctimer);    
  }
}
/*----------------------------------------------------------------------------*/
void uip_icmp6_send_echo_request(void)
{
    /* set identifier and sequence number to 0 */
    memset((uint8_t *)UIP_ICMP_BUF + UIP_ICMPH_LEN, 0x00, 4);       
    /* reset icmpv6 data */
    memset((uint8_t *)UIP_ICMP_BUF + UIP_ICMPH_LEN + 2/*id*/ + 2/*seqno*/,
            0x00, PING6_DATALEN);        
    
    /* copy identifier*/                                                                        //Arjun: 23-0318: to be set fom tool
    memcpy((uint8_t *)UIP_ICMP_BUF + UIP_ICMPH_LEN, (uint8_t *)&echo_config.identifier, 2);
    /* copy sequnce_number*/
    memcpy((uint8_t *)UIP_ICMP_BUF + UIP_ICMPH_LEN + 2/*id*/, (uint8_t *)&echo_config.sequence_number, 2);
    
    
    /* copy icmpv6 data */
    memcpy((uint8_t *)UIP_ICMP_BUF + UIP_ICMPH_LEN + 2/*id*/ + 2/*seqno*/, 
           echo_config.p_echo_data, echo_config.echo_data_len);    
    /* send icmpv6 request */
    uip_icmp6_send(&(echo_config.dest_ip_addr), ICMP6_ECHO_REQUEST, 0x00/*code*/, 
                   echo_config.echo_data_len + 2/*id*/ + 2/*seqno*/,echo_config.hop_limit); 
}
/*----------------------------------------------------------------------------*/
void wan_uip_icmp6_send_echo_reply (void)
{
    echo_config.hop_limit = uip_ds6_if.cur_hop_limit;
    /* set identifier and sequence number to 0 */
    memset((uint8_t *)UIP_ICMP_BUF + UIP_ICMPH_LEN, 0x00, 4);       
    /* reset icmpv6 data */
    memset((uint8_t *)UIP_ICMP_BUF + UIP_ICMPH_LEN + 2/*id*/ + 2/*seqno*/,
            0x00, PING6_DATALEN);        
    
    /* copy identifier*/                                                                        //Arjun: 23-0318: to be set fom tool
    memcpy((uint8_t *)UIP_ICMP_BUF + UIP_ICMPH_LEN, (uint8_t *)&echo_config.identifier, 2);
    /* copy sequnce_number*/
    memcpy((uint8_t *)UIP_ICMP_BUF + UIP_ICMPH_LEN + 2/*id*/, (uint8_t *)&echo_config.sequence_number, 2);
    
    /* send icmpv6 request */
    wan_uip_icmp6_send(&(echo_config.src_ip_addr), &(echo_config.dest_ip_addr), ICMP6_ECHO_REPLY, 0x00/*code*/, 
                   0 + 2/*id*/ + 2/*seqno*/,echo_config.hop_limit); 
}
/*----------------------------------------------------------------------------*/
uint16_t hex2int(uint16_t val)
{
    uint8_t j = 0;
    uint8_t i = 0;
    uint16_t sum = 0;
    uint16_t rem = 0;
    for(i=0;i<4;i++)
    {
      rem = (val%10);
      rem = rem<<j;
      j = j + 4;
      sum = sum | rem;
      //sum = sum|rem;
      val = val/10;
    }
    
    return sum;
}
/*----------------------------------------------------------------------------*/
void send_icmv6_conf(void)
{
  
#ifdef SEND_PING_PKT_TO_HIF
  
  enet_msg_t* msg = allocate_hif_msg( echo_config.echo_data_len+50 );//50 bytes is sefty zone    
  uint8_t* buf = msg->data;    
  if(msg != NULL)
  {
    *buf++ = SEND_ICMPV6_CONF;
    
    memcpy(buf,echo_config.src_ip_addr.u8,16);
    buf+=16;
    
    memcpy(buf,echo_config.dest_ip_addr.u8,16);
    buf+=16;
    ++icmv6_counter;

#if(FAN_EDFE_FEATURE_ENABLED == 1)
    edfe_information.edfe_sent_pkt =  icmv6_counter;
#endif

    if(icmv6_counter == 0xFFFF)
    {
      icmv6_counter = 0x00;
    }
    
    mem_rev_cpy(buf,(uint8_t*)&icmv6_counter,2);
    buf+=2;
    
    *buf++ = echo_config.interval;
    
    put_ushort_BE(buf,echo_config.echo_data_len);
    buf += 2;
    
    memcpy(buf,echo_config.p_echo_data,echo_config.echo_data_len);
    buf += echo_config.echo_data_len;
    
    *buf++ = DUMMY_COMPORT;
    
    msg->data_length  = buf - msg->data;
    
    // data_length -1 :: -1 is for command ID
    hif_send_msg_up(msg->data,msg->data_length-1,response_laye_ID,PROTOCOL_ID_FOR_APP);
    free_hif_msg( msg );
    if(echo_config.counter == 0x00)
    {
      app_bm_free((uint8_t*)echo_config.p_echo_data);
      echo_config.p_echo_data = NULL;
     }
  }
  if(echo_config.p_echo_data != NULL)
  {
    if(echo_config.counter == 0x00)
    {
      app_bm_free((uint8_t*)echo_config.p_echo_data);
      echo_config.p_echo_data = NULL;
     }
  }
#endif
     
}

/*----------------------------------------------------------------------------*/

void set_src_dst_in_echo_structure (uip_ipaddr_t *src, uip_ipaddr_t *dest, uint16_t identifier, uint16_t seq_number)
{
  memcpy(&(echo_config.dest_ip_addr), dest, 16);
  memcpy(&(echo_config.src_ip_addr), src, 16);
  echo_config.identifier = identifier;
  echo_config.sequence_number = seq_number;
}
