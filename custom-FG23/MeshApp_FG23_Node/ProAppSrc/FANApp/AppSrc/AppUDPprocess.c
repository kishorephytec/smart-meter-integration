/** \file udp_port8.c
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
********************************************************************************
* File inclusion
********************************************************************************
*/


#include "StackAppConf.h"
#include "common.h"
#include <stdio.h>
#include <string.h>
#include "contiki-net.h"
#include "AppUDPprocess.h"
#include "uart_hal.h"
#include "queue_latest.h"
#include "hif_utility.h"
#include "fan_app_auto.h"
#include "fan_api.h"
#include "ie_element_info.h"
#include "fan_app_test_harness.h"
#include "sm.h"
#include "fan_mac_interface.h"
#include "app_init.h"
//#include "hw_tmr.h"


/*
** =============================================================================
** Private Macro definitions
** =============================================================================
*/
	
#define EXPLICIT_UDP_SEND       0

/*
** =============================================================================
** Private Structures, Unions & enums Type Definitions
** =============================================================================
**/

typedef struct udp_config{
    uip_ipaddr_t src_ip_addr;
    uip_ipaddr_t dest_ip_addr;
    uint16_t source_port_number;    
    uint16_t destination_port_number;
    uint16_t data_length;
    uint16_t counter;    /*for sending number of packets*/
    uint8_t interval;    /*interval during send*/
    uint8_t *data_buff;
}udp_config_t;

/*
** =============================================================================
** Private Variable Definitions
** =============================================================================
*/

static struct simple_udp_connection unicast_connection;
static struct simple_udp_connection unicast_connection1;
static struct simple_udp_connection unicast_connection2;
static struct simple_udp_connection unicast_connection3;
static struct simple_udp_connection unicast_connection4;
static udp_config_t udp_param = {0};
#if EXPLICIT_UDP_SEND
static uint8_t udp_send_flag = 0x00;
#endif

/*Umesh :30-01-2018*/
uint8_t data_send_idx=0x00;
/*this varriable never used*/
static uint16_t udp_counter = 0;
static uint16_t recv_counter = 0;
//static l3_etimer_t udp_send_timer;
static l3_ctimer_t udp_send_ctimer;

/*
** =============================================================================
** Private Function Prototypes
** =============================================================================
*/
/*Umesh : 30-01-2018*/
//static uint8_t process_udp(uip_ipaddr_t addr, uint8_t *data,uint16_t datalen);
uint8_t send_udp(uint8_t *buf, uint16_t length);
//static void process_udp_packet(uint8_t *buffer, uint16_t length);
/*this function not called from anywhere*/
//static void explicit_trigger_udp_request();
static void udp_send_ctimer_callback(void* buf);
static void send_udp_conf(void);
static void
revceiver_callback (struct simple_udp_connection *c,
         const uip_ipaddr_t *sender_addr,
         uint16_t sender_port,
         const uip_ipaddr_t *receiver_addr,
         uint16_t receiver_port,
         const uint8_t *data,
         uint16_t datalen);

/*
** =============================================================================
** External Variable Declarations
** =============================================================================
*/

/* None */

/*
** =============================================================================
** External Function Prototypes
** =============================================================================
*/
extern uint8_t response_laye_ID;
extern uint8_t get_fan_device_type(void);
extern void red_led_off(void);
extern void mem_rev_cpy(uint8_t* dest, uint8_t* src, uint16_t len );
extern uint8_t gu2pcapp_node_config_set_conf( uint8_t status );
extern enet_msg_t* allocate_hif_msg( uint16_t length );
extern void free_hif_msg( enet_msg_t * msgp );
extern uint8_t* get_self_address(void);
extern void explicit_trigger_echo_request(uip_ipaddr_t* dest_ip_addr);
/*Umesh :30-01-2018*/
extern int uip_ds6_nbr_num(void);
/*this function not called form this file*/
#if(FAN_EDFE_FEATURE_ENABLED == 1)
extern edfe_info_t edfe_information;
#endif
extern void * app_bm_alloc( uint16_t length );
extern void app_bm_free( uint8_t *pMem  );


L3_PROCESS(udp_process, "UDP process");

/*
** =============================================================================
** Public Variables Definitions
** =============================================================================
**/

uint8_t temp_buf_set = 0x11;

/*
** =============================================================================
** Public Function Prototypes
** =============================================================================
*/

uint8_t send_udp_packets(uint8_t* p_buff);

/*
** =============================================================================
** Public Function Definitions
** =============================================================================
*/

void mem_reverse_cpy(uint8_t* dest, uint8_t* src, uint16_t len )
{
    uint16_t i = 0;
    for (i=0;i<len;i++)
    {
      dest[len-i-1] = src[i];
    }
}
/*---------------------------------------------------------------------------*/
L3_PROCESS_THREAD(udp_process, ev, data)
{
    L3_PROCESS_BEGIN();
 
    simple_udp_register(&unicast_connection, UDP_PORT,
                      NULL, UDP_PORT, revceiver_callback);
    
    simple_udp_register(&unicast_connection1, UDP_PORT1,
                      NULL, UDP_PORT1, revceiver_callback);
    simple_udp_register(&unicast_connection2, UDP_PORT2,
                      NULL, UDP_PORT2, revceiver_callback);
    simple_udp_register(&unicast_connection3, UDP_PORT3,
                      NULL, UDP_PORT3, revceiver_callback);
    simple_udp_register(&unicast_connection4, UDP_PORT4,
                      NULL, UDP_PORT4, revceiver_callback);

#if EXPLICIT_UDP_SEND        
    if(get_fan_device_type() == 0x00)
      l3_etimer_set(&udp_send_timer, SEND_INTERVAL_ROOT);
    else
      l3_etimer_set(&udp_send_timer, SEND_INTERVAL);
#endif    

    while(1) {
      L3_PROCESS_YIELD();
      if ( ev == L3_PROCESS_EVENT_TIMER ){
        
#if EXPLICIT_UDP_SEND	  
        if((get_fan_device_type() == 0x01) ||
          (get_fan_device_type() == 0x02)){
          if(data == &udp_send_timer &&
            l3_etimer_expired(&udp_send_timer) &&
            udp_send_flag == 0x01){
            explicit_trigger_udp_request();
          }
          else{
            l3_etimer_stop(&udp_send_timer);
            l3_etimer_set(&udp_send_timer, SEND_INTERVAL);        
          }
        }
        else{
          if(data == &udp_send_timer &&
            l3_etimer_expired(&udp_send_timer)){
            explicit_trigger_udp_request();
          }
          else{
            l3_etimer_stop(&udp_send_timer);
            l3_etimer_set(&udp_send_timer, SEND_INTERVAL_ROOT);        
          }  
        }
#endif
        
      } 
    }
    L3_PROCESS_END();
}
/*---------------------------------------------------------------------------*/
//void trigger_udp_request( uint8_t* p_buff )
//{
//        uip_lladdr_t ll_add = {0};
//        uip_ipaddr_t dest_ip_addr = {0};
//        uint16_t datalen = 0;
//        uint8_t data[512] = {0};
//        uint16_t index = 0;
//        rpl_dag_t *dag = NULL;  
//        uint16_t prefix_id = 0;
//        uint8_t addr_opt = p_buff[3];
//        index += 4;
//    
//    dag = rpl_get_any_dag();//current dag
//    if(dag == NULL)
//      return;
//
//    
//    if(addr_opt == 0x00)//global
//      prefix_id = UIP_HTONS(0x2001);
//
//    if(addr_opt == 0x01)//link local
//      prefix_id = UIP_HTONS(0xfe80);
//
//    if(addr_opt == 0x02){//multicast
//      uip_create_linklocal_rplnodes_mcast(&dest_ip_addr);
//      index += 8;
//      datalen = ((p_buff[index] >> 8) | (p_buff[index+1]));    
//      index += 2;
//      memcpy(data, &p_buff[index], datalen);
//      simple_udp_sendto(&unicast_connection, data, datalen, &dest_ip_addr);
//      return;      
//    }
//
//    memcpy(ll_add.addr,&(p_buff[index]),8);
//    index += 8;
//    
//    uint8_t* self_ieee_addr = get_self_address();    
//    if(!memcmp(ll_add.addr, self_ieee_addr, 8))
//      return;//requesting to self address error    
//    
//    uip_create_prefix(&dest_ip_addr, prefix_id);        
//    uip_ds6_set_addr_iid(&dest_ip_addr, &ll_add);        
//    
//    datalen = ((p_buff[index] >> 8) | (p_buff[index+1]));    
//    index += 2;
//    
//    memcpy(data, &p_buff[index], datalen);
//    simple_udp_sendto(&unicast_connection, data, datalen, &dest_ip_addr);      
//
//    return;
//}
/*---------------------------------------------------------------------------*/

//#if AUTO_CONFIG_ENABLE 

//void trigger_both_udp_ping_request(void)
//{
//    uint16_t datalen = 20;
//    uint8_t data[MAX_UDP_LENGTH_SUPPORT/*1024*/] = {0x00};
//    uip_lladdr_t ll_add  = {0};
//    uip_ipaddr_t dest_ip_addr = {0};
//    //uint16_t prefix_id = UIP_HTONS(0x2001);
//
//    if(unicast_connection.local_port != UDP_PORT &&
//      unicast_connection.remote_port != UDP_PORT){
//      simple_udp_register(&unicast_connection, UDP_PORT, NULL, UDP_PORT, 
//                          revceiver_callback);
//    }
//    
//    if(datalen > MAX_UDP_LENGTH_SUPPORT)
//      return;
//    
//    memset(data, temp_buf_set, datalen);
//    data[datalen-1] = 0x2f;//for getting the last byte
//    temp_buf_set+=1;
//
//    uip_ds6_nbr_t *nbr = nbr_table_head(ds6_neighbors);
//    while(nbr != NULL) {
//      if(!nbr->data_test){
//        nbr->data_test = 1;
//        memcpy(ll_add.addr,&nbr->lladdr,8);
//        break;
//      }      
//      nbr = nbr_table_next(ds6_neighbors, nbr);
//    }    
//     
//    if(nbr == NULL){
//      red_led_off();      
//      nbr = nbr_table_head(ds6_neighbors);
//      while(nbr != NULL) {
//        nbr->data_test = 0;
//        nbr = nbr_table_next(ds6_neighbors, nbr);
//      }      
//    }
//    else{      
//      //uip_create_prefix(&dest_ip_addr, prefix_id);
//      //uip_ds6_set_addr_iid(&dest_ip_addr, &ll_add);
//      memcpy(&dest_ip_addr,&nbr->global_addr,16);
//      simple_udp_sendto(&unicast_connection, data, datalen, &dest_ip_addr);
//      explicit_trigger_echo_request(&dest_ip_addr);    
//    }      
//}
//#endif
/*---------------------------------------------------------------------------*/
void send_udp_data_packet()
{
#if EXPLICIT_UDP_SEND  
  udp_send_flag = 0x01;
#endif
  return;
}
/*---------------------------------------------------------------------------*/
void process_udp_port_register(uint8_t *buf, uint16_t length)
{
    uint16_t source_port_number = 0;    
    uint16_t destination_port_number = 0; 
    uint8_t index=0;
    
    mem_rev_cpy((uint8_t *)&source_port_number, &buf[index], 2);
    index+=2;
    mem_rev_cpy((uint8_t *)&destination_port_number, &buf[index], 2);
    index+=2;
    
    simple_udp_register(&unicast_connection, source_port_number, NULL, 
                        destination_port_number, revceiver_callback);
    gu2pcapp_node_config_set_conf(0x00);//hif success
    return;
}
/*---------------------------------------------------------------------------*/
uint8_t send_udp_packets(uint8_t* p_buff)
{
  memset(&udp_param, 0x00, sizeof(udp_param));  
  uint16_t index=0;
  memcpy(udp_param.src_ip_addr.u8, &p_buff[index], 16);
  index+=16;
  mem_rev_cpy((uint8_t *)&udp_param.source_port_number, &p_buff[index], 2);
  index+=2;
  memcpy(udp_param.dest_ip_addr.u8, &p_buff[index], 16);
  index+=16; 
  mem_rev_cpy((uint8_t *)&udp_param.destination_port_number, &p_buff[index], 2);
  index+=2;
  uint16_t length; 
  mem_rev_cpy((uint8_t *)&length, &p_buff[index], 2);
  index+=2;
  udp_param.data_length = length;
  udp_param.data_buff = (uint8_t *)app_bm_alloc( udp_param.data_length );
  memcpy(udp_param.data_buff, &p_buff[index], udp_param.data_length);
  udp_param.counter =1;
  udp_param.interval = 5;
  simple_udp_register(&unicast_connection, udp_param.source_port_number, NULL, 
                        udp_param.destination_port_number, revceiver_callback);
  simple_udp_sendto(&unicast_connection, udp_param.data_buff, 
                      udp_param.data_length, &udp_param.dest_ip_addr);
   app_bm_free((uint8_t*)udp_param.data_buff);
  return 0;
}
/*---------------------------------------------------------------------------*/

/*
** =============================================================================
** Private Function Definitions
** =============================================================================
*/

 uint8_t udp_test_counter = 0;
uint8_t send_udp(uint8_t *buf, uint16_t length)
{      
    memset(&udp_param, 0x00, sizeof(udp_param));   
    udp_counter = 0x00;
    uint16_t index = 0;
    
    memcpy(udp_param.src_ip_addr.u8, &buf[index], 16);
    index+=16;//16
    
    //currently no port supporting
    //mem_rev_cpy((uint8_t *)&udp_param.source_port_number, &buf[index], 2);
    //index+=2;//18
    if(udp_test_counter == 0)
      udp_param.source_port_number = UDP_PORT;//sending with default port
    if(udp_test_counter == 1)
      udp_param.source_port_number = UDP_PORT1;//sending with default port
    if(udp_test_counter == 2)
      udp_param.source_port_number = UDP_PORT2;//sending with default port
    if(udp_test_counter == 3)
      udp_param.source_port_number = UDP_PORT3;//sending with default port
    if(udp_test_counter == 4)
      udp_param.source_port_number = UDP_PORT4;//sending with default port
          
    memcpy(udp_param.dest_ip_addr.u8, &buf[index], 16);
    index+=16;//34
    
    //currently no port supporting
    //mem_rev_cpy((uint8_t *)&udp_param.destination_port_number, &buf[index], 2);
    //index+=2;//36
    if(udp_test_counter == 0)
      udp_param.destination_port_number = UDP_PORT;//sending with default port
    if(udp_test_counter == 1)
      udp_param.destination_port_number = UDP_PORT1;//sending with default port
    if(udp_test_counter == 2)
      udp_param.destination_port_number = UDP_PORT2;//sending with default port
    if(udp_test_counter == 3)
      udp_param.destination_port_number = UDP_PORT3;//sending with default port
    if(udp_test_counter == 4)
      udp_param.destination_port_number = UDP_PORT4;//sending with default port

    uint8_t* self_ieee_addr = get_self_address();
    uip_lladdr_t ll_add;
    memcpy(ll_add.addr, &udp_param.dest_ip_addr.u8[8], 8);
    ll_add.addr[0] ^= 0x02;
    if(!memcmp(ll_add.addr, self_ieee_addr, 8))
      return 1;//requesting to self address error
    
    mem_rev_cpy((uint8_t *)&udp_param.counter, &buf[index], 2);
    index+=2;//38
#if(FAN_EDFE_FEATURE_ENABLED == 1)
    edfe_information.edfe_trigger_packt =  udp_param.counter;
#endif
    udp_param.interval = buf[index];
    index+=1;//39
    
    mem_rev_cpy((uint8_t *)&udp_param.data_length, &buf[index], 2);
    index+=2;//CURRENT_UDP_PARAM_LEN = 41         

    if(udp_param.data_length == 0 || udp_param.data_length > MAX_UDP_LENGTH_SUPPORT || 
       udp_param.counter == 0 || udp_param.counter >= 0xFFFF ||
       udp_param.interval == 0 || udp_param.interval >= 0xFF)
      return 1;//sending fail status
    
    //explicitly setting to 5 sec, due more than 10 hopping
    if(udp_param.interval <= 5)
      udp_param.interval = 5;
    udp_param.data_buff = (uint8_t *)app_bm_alloc( udp_param.data_length );
    if(udp_param.data_buff == NULL)
    {
      return 1;
    }
    memcpy(udp_param.data_buff, &buf[index], udp_param.data_length);//NEED DYNAMIC ALLOCATION
    index+=udp_param.data_length;    
    
    //simple_udp_register(&unicast_connection, udp_param.source_port_number, NULL, 
    //                    udp_param.destination_port_number, revceiver_callback);    
    
      if(udp_test_counter == 0)
      simple_udp_sendto(&unicast_connection, udp_param.data_buff, 
                      udp_param.data_length, &udp_param.dest_ip_addr);
    if(udp_test_counter == 1)
      simple_udp_sendto(&unicast_connection1, udp_param.data_buff, 
                      udp_param.data_length, &udp_param.dest_ip_addr);
    if(udp_test_counter == 2)
      simple_udp_sendto(&unicast_connection2, udp_param.data_buff, 
                      udp_param.data_length, &udp_param.dest_ip_addr);
    if(udp_test_counter == 3)
      simple_udp_sendto(&unicast_connection3, udp_param.data_buff, 
                      udp_param.data_length, &udp_param.dest_ip_addr);
    if(udp_test_counter == 4)
      simple_udp_sendto(&unicast_connection4, udp_param.data_buff, 
                      udp_param.data_length, &udp_param.dest_ip_addr);
    
//    simple_udp_sendto(&unicast_connection, udp_param.data_buff, 
//                      udp_param.data_length, &udp_param.dest_ip_addr);
    udp_param.counter--;
    send_udp_conf();
    //gu2pcapp_node_config_set_conf(0x00);

    if(udp_param.counter)
    {
      l3_ctimer_set(&udp_send_ctimer, udp_param.interval*CLOCK_SECOND, 
                 udp_send_ctimer_callback, NULL);      
    }
    if (udp_test_counter == 4)
      udp_test_counter = 0;
    else
      udp_test_counter++;
    
    return 0;
}
/*---------------------------------------------------------------------------*/
static void udp_send_ctimer_callback(void* ptr)
{
    simple_udp_sendto(&unicast_connection, udp_param.data_buff, udp_param.data_length, 
                      &udp_param.dest_ip_addr);
    udp_param.counter--;
    send_udp_conf();
    //gu2pcapp_node_config_set_conf(0x00);    
    if(udp_param.counter)
    {
      l3_ctimer_set(&udp_send_ctimer, udp_param.interval*CLOCK_SECOND, 
                 udp_send_ctimer_callback, NULL);      
    }
    else
    {
      l3_ctimer_stop(&udp_send_ctimer);          
    }
}
/*---------------------------------------------------------------------------*/
static void send_udp_conf(void)
{
#ifdef SEND_UDP_PKT_TO_HIF
    enet_msg_t* msg = allocate_hif_msg( udp_param.data_length+50 );//50 bytes is sefty zone    
    uint8_t* buf = msg->data;    
    if(msg != NULL)
     {
          *buf++ = SEND_UDP_PACKET;
          
          memcpy(buf,udp_param.src_ip_addr.u8,16);
          buf+=16;
          
          memcpy(buf,udp_param.dest_ip_addr.u8,16);
          buf+=16;
          ++udp_counter;
#if(FAN_EDFE_FEATURE_ENABLED == 1)
          edfe_information.edfe_sent_pkt = udp_counter;
#endif
          if(udp_counter == 0xFFFF)
          {
            udp_counter = 0x00;
          }
          mem_rev_cpy(buf,(uint8_t*)&udp_counter,2);
          buf+=2;
          
          *buf++ = udp_param.interval;
          
          put_ushort_BE(buf,udp_param.data_length);
          buf += 2;
          
          memcpy(buf,udp_param.data_buff,udp_param.data_length);
          buf += udp_param.data_length;
          
          *buf++ = DUMMY_COMPORT;
          
          msg->data_length  = buf - msg->data;
          // data_length -1 :: -1 is for command ID   
//[Kimbal]
//        hif_send_msg_up(msg->data,msg->data_length-1,response_laye_ID,PROTOCOL_ID_FOR_APP);
//          free_hif_msg( msg );
          if(udp_param.counter == 0x00)
            app_bm_free((uint8_t*)udp_param.data_buff);
          
          udp_param.data_buff = NULL;
     }
    
    if(udp_param.data_buff != NULL)
    {
      if(udp_param.counter == 0x00)
            app_bm_free((uint8_t*)udp_param.data_buff);
      
      udp_param.data_buff = NULL;
    }
     
#endif 
}

/*---------------------------------------------------------------------------*/
static void
revceiver_callback (struct simple_udp_connection *c,
         const uip_ipaddr_t *sender_addr,
         uint16_t sender_port,
         const uip_ipaddr_t *receiver_addr,
         uint16_t receiver_port,
         const uint8_t *data,
         uint16_t datalen)
{

#if 0// Raka .. for checking RAW UART on HIF .. [ 12-OCt-2023 ]
#ifdef SEND_UDP_PKT_TO_HIF
  
    enet_msg_t* msg = allocate_hif_msg( datalen+25 );
    uint8_t* buf = msg->data;	
    
#if(FAN_EDFE_FEATURE_ENABLED == 1)
    if(edfe_information.edfe_frame_enabled == 0x01)
    {
      if((edfe_information.edfe_transmit_flow_contrl == 0)&&(edfe_information.edfe_receiver_flow_contrl != 0))
      {
        enable_disable_edfe_frame(1,FINAL_RESPONSE_FRAME);
        send_edfe_initial_frame(edfe_information.edfe_ini_mac_addr ,1,FINAL_RESPONSE_FRAME);
        enable_disable_edfe_frame(0,255);//go to currunt channel listing
      }
      if((edfe_information.edfe_transmit_flow_contrl != 0)&&(edfe_information.edfe_receiver_flow_contrl != 0))
      {
        enable_disable_edfe_frame(1,RESPONSE_FRAME);
        send_edfe_initial_frame(edfe_information.edfe_ini_mac_addr ,1,RESPONSE_FRAME);
      }
    }
#endif
     if(msg != NULL)
     {
       *buf++ = UDP_RECIVED_IND;
       
       memcpy(buf,sender_addr->u8,16);
       buf+=16;
       
       mem_rev_cpy(buf,(uint8_t *)&sender_port,2);
       buf+=2;
       ++recv_counter;
       if(recv_counter == 0xFFFF)
       {
         recv_counter = 0x00;
       }
       mem_rev_cpy(buf,(uint8_t *)&recv_counter,2);
       buf+=2;
       
       put_ushort_BE(buf,datalen);
       buf += 2;
       
       memcpy(buf,data,datalen);
       buf += datalen;
       
       *buf++ = DUMMY_COMPORT;
       
       msg->data_length  = buf - msg->data;
       
       // data_length -1 :: -1 is for command ID
//[Kimbal]
//       hif_send_msg_up(msg->data,msg->data_length-1,response_laye_ID,PROTOCOL_ID_FOR_APP);
//       free_hif_msg( msg );
     }
#endif 
    
#else  // for #if 0

// This code is to send the data to HES from the Linux Gateway ..
//    uint8_t tempMACAddr [ 8] = {0};
//    
//    enet_msg_t* msg = allocate_hif_msg( datalen+25 );
//    uint8_t* buf = msg->data;	
//    
//////[kimbal]
//    if(msg != NULL)
//    {
//       
//       memcpy(buf,data,datalen);       
//       buf += datalen;       
//       msg->data_length  = buf - msg->data;
//       //debug_UDP_revceiver_callback_cnt++;
//       
//      // hif_send_msg_up(msg->data,msg->data_length,response_laye_ID, 0xAA);
//       //free_hif_msg( msg );
//       
//        uart_hal_write((uint8_t *)msg->data, msg->data_length);
//        
//        free_hif_msg( msg );
//           
//    }        
    
    
    
    
    uart_hal_write((uint8_t *)data, datalen);
    
    
////[kimbal]
    
//     if(msg != NULL)
//     {
//       
//       *buf++ = 0x03; // HIF Frame CMD ID
//       
//       *buf++ = 0x03;  // pRIMITIVE id
//       *buf++ = 0x04;  // Frame ID
//       
//       put_ushort(buf,datalen);  // Data Lengtgh
//       buf += 2;
//       
//       *buf++ = 0x00;
//       
//       memcpy(&tempMACAddr[0], (uint8_t *)&sender_addr->u8[8], 8 );
//       tempMACAddr[0] ^= 0x02;
//       mem_rev_cpy(buf,(uint8_t *)&tempMACAddr[0],8); // Src MAC Addr
//       
//       buf+=8;
//         memcpy(buf,(uint8_t *)&sender_port,2);  // Src Port
//       buf+=2;
//       
//       memset (&tempMACAddr[0], 0x00, 8);
//       memcpy(&tempMACAddr[0], (uint8_t *)&receiver_addr->u8[8], 8 );
//       tempMACAddr[0] ^= 0x02;
//       mem_rev_cpy(buf,(uint8_t *)&tempMACAddr[0],8); // Dst MAC Addr
//       buf+=8;
//       
//       
//       memcpy(buf,(uint8_t *)&receiver_port,2); // Dst port
//       buf+=2;
//      
//       *buf++  = 0x05;  // Hop count 
//        memset(buf,0x01,4); // Travel Time TBD
//         buf+=4;
//         
//       put_ushort(buf,datalen);  // get_ushort  // APDU Length
//       buf += 2;
//       
//       memcpy(buf,data,datalen);       
//       buf += datalen;
//       
//       
//       *buf++ = DUMMY_COMPORT;//Dummy Comport 
//       
//       msg->data_length  = buf - msg->data;
//       debug_UDP_revceiver_callback_cnt++;
//       hif_send_msg_up(msg->data,msg->data_length-1,APP_DEF_LAYER_ID_LINUXGATEWAY, PROTOCOL_ID_FOR_APP);
//       free_hif_msg( msg );
//       
//      
//     }
     

#endif    
 
 for (int i = 0; i <= 6; i++) {
    APP_LED_TOGGLE();
    tmr_delay(500000);    
}
}
/*---------------------------------------------------------------------------*/

extern uint8_t global_addr_device[16];
//uint8_t sorce_ip[16]={0X00};
//2020000D00B800001322334411112222
extern uip_ipaddr_t root_global_addr; 
//uint8_t destination_ip[16]={0x20,0x20,0x00,0x0D,0x00,0xB8,0x00,0x00,0x13,0x22,0x33,0x44,0xEE,0xEE,0xAA,0xAA};
//2020000D00B8000013223344EEEEAAAA
int udp_send_counter = 0;



uint8_t UART_Data_send_udp(uint8_t *buf, uint16_t length)
{      
    udp_send_counter++;
    memset(&udp_param, 0x00, sizeof(udp_param));   
    udp_counter = 0x00;
    //uint16_t index = 0;
    
    memcpy(udp_param.src_ip_addr.u8, &global_addr_device[0], 16);
   // index+=16;//16
    
    //currently no port supporting
    //mem_rev_cpy((uint8_t *)&udp_param.source_port_number, &buf[index], 2);
    //index+=2;//18
    udp_param.source_port_number = UDP_PORT;//sending with default port
    
    memcpy(udp_param.dest_ip_addr.u8, &root_global_addr.u8[0], 16);
   // index+=16;//34
    
    //currently no port supporting
    //mem_rev_cpy((uint8_t *)&udp_param.destination_port_number, &buf[index], 2);
    //index+=2;//36
    udp_param.destination_port_number = UDP_PORT;//sending with default port

    uint8_t* self_ieee_addr = get_self_address();
    uip_lladdr_t ll_add;
    memcpy(ll_add.addr, &udp_param.dest_ip_addr.u8[8], 8);
    ll_add.addr[0] ^= 0x02;
    if(!memcmp(ll_add.addr, self_ieee_addr, 8))
      return 1;//requesting to self address error
    
//    mem_rev_cpy((uint8_t *)&udp_param.counter, &buf[index], 2);
//    index+=2;//38
    
    udp_param.counter = 1;
    
    
#if(FAN_EDFE_FEATURE_ENABLED == 1)
    edfe_information.edfe_trigger_packt =  udp_param.counter;
#endif
    
    udp_param.interval = 1 ; //buf[index];
    //index+=1;//39
    
//    mem_rev_cpy((uint8_t *)&udp_param.data_length, &buf[index], 2);
//    index+=2;//CURRENT_UDP_PARAM_LEN = 41    
    
    udp_param.data_length = length ;

    if(udp_param.data_length == 0 || udp_param.data_length > MAX_UDP_LENGTH_SUPPORT || 
       udp_param.counter == 0 || udp_param.counter >= 0xFFFF ||
       udp_param.interval == 0 || udp_param.interval >= 0xFF)
      return 1;//sending fail status
    
    //explicitly setting to 5 sec, due more than 10 hopping
    if(udp_param.interval <= 5)
      udp_param.interval = 5;
    udp_param.data_buff = (uint8_t *)app_bm_alloc( udp_param.data_length );
    if(udp_param.data_buff == NULL)
    {
      return 1;
    }
    memcpy(udp_param.data_buff, &buf[0], udp_param.data_length);//NEED DYNAMIC ALLOCATION
    //index+=udp_param.data_length;    
    
    //simple_udp_register(&unicast_connection, udp_param.source_port_number, NULL, 
    //                    udp_param.destination_port_number, revceiver_callback);    
    
    simple_udp_sendto(&unicast_connection, udp_param.data_buff, 
                      udp_param.data_length, &udp_param.dest_ip_addr);
    udp_param.counter--;

   // send_udp_conf();
    //gu2pcapp_node_config_set_conf(0x00);

    app_bm_free(udp_param.data_buff);
    if(udp_param.counter)
    {
      l3_ctimer_set(&udp_send_ctimer, udp_param.interval*CLOCK_SECOND, 
                 udp_send_ctimer_callback, NULL);      
    }
    return 0;
}
