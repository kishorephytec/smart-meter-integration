/** \file fan_app_test_harness.c
 *******************************************************************************
 ** \brief Implements the finctions for testing the FAN stack using 
 **             Procubed Stack Validation tool and TBC
 **
 ** \cond STD_FILE_HEADER
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
#include "em_device.h"
#include "list_latest.h"
#include "queue_latest.h"
#include "uart_hal.h"
#include "mac.h"
//[Kimbal]
//#include "hif_utility.h"
//#include "hif_service.h"
#include "buff_mgmt.h"
#include "buffer_service.h"
#include "hw_tmr.h"
#include "sw_timer.h"
#include "mac_interface_layer.h"
#include "fan_mac_security.h"
#include "sm.h"
#include "contiki-net.h"
#include "ie_element_info.h"
#include "fan_app_test_harness.h"
#include "network-manager.h"
#include "fan_api.h"
#include "fan_app_auto.h"
#include "fan_factorycmd.h"
#include "fan_mac_security.h"

/*
** =============================================================================
** Private Macro definitions
** =============================================================================
*/
	
#define VERSION_NUMBER  "1.0.2"

/*
** =============================================================================
** Private Structures, Unions & enums Type Definitions
** =============================================================================
**/

enum modulation{
      FSK2,
      NOT_APLICABLE  
      };


/*
** =============================================================================
** Private Variable Definitions
** =============================================================================
*/

//[Kimbal]
//static hif_service_t hif_fan_test;



/*
** =============================================================================
** Private Function Prototypes
** =============================================================================
*/

static void process_send_udp(uint8_t *buf, uint16_t length);
extern void process_set_facort_mode_PA_level_api(uint8_t *buf, uint16_t length);

/*
** =============================================================================
** External Variable Declarations
** =============================================================================
*/

#if(APP_NVM_FEATURE_ENABLED == 1)
extern void change_join_state_for_nvm();
#endif
extern fan_nwk_manager_sm_t fan_nwk_manager_app;
/*Umesh : 30-01-2018*/
extern bool upper_layer_started;
extern volatile uint8_t load_wd;
extern uint16_t icmv6_response_counter;
extern uint8_t MACSecKey1[16];
/*this varriable not used*/
extern uint8_t send_udp(uint8_t *buf, uint16_t length);
extern uint8_t key_id_index  ; 
extern uint8_t relay_reply_flag;
extern uint8_t authnt_interfac_id [8];
extern uint8_t TANSIT_KMP_ID ;
extern uint8_t trickle_IMIN;
extern uint8_t trickle_IMAX;

#ifdef WISUN_FAN_MAC  
extern self_info_fan_mac_t mac_self_fan_info;
extern fan_mac_security mac_key_list;
#endif

extern void App_factory_mode_channel_set();

//[Kimbal]
//extern hif_t hif;

/*
** =============================================================================
** External Function Prototypes
** =============================================================================
*/

/*UDP*/
extern uint8_t send_udp_packets(uint8_t* p_buff);
extern void process_udp_port_register(uint8_t *buf, uint16_t length);

/*PING*/
extern void send_icmpv6(uint8_t *buf, uint16_t length);

/*Umesh : 30-01-2018*/
//extern uint8_t send_udp(uint8_t *buf, uint16_t length);
extern  void mem_reverse_cpy(uint8_t* dest, uint8_t* src, uint16_t len );
//extern void nvm_erase_node_basic_info( void );
extern void set_mac_security_enable_disable(uint8_t enable_security_flag);
/*this varriable not used*/

extern uint8_t send_hif_conf_cb( uint8_t cmd_id,uint8_t status );
extern uint8_t send_hif_seckey_cb( uint8_t cmd_id,uint8_t* seckey );
extern void trigger_echo_request( uint8_t* p_buff  );
extern uint8_t process_telec_set_operating_country( uint8_t CountryCode  ) ;
//extern void trigger_udp_request( uint8_t* p_buff );
extern void send_host_apd_bootup();
/*Umesh :  30-01-2018*/
extern void send_mac_addr(uint8_t *buff , uint16_t len);
/*this fuctions was defined this file and extern also used????*/
extern uint8_t generate_MAC_Security_Key (uint8_t live_gtk_key_index,uint16_t len);
extern void set_mac_security_on_LBR(uint8_t *rec_buff, uint16_t len);
extern void set_mac_security_on_router_node(uint8_t *rec_buff, uint16_t len);

#if(FAN_EAPOL_FEATURE_ENABLED == 1)
extern void send_data_to_eapol_relay_process(uint8_t *payload ,uint16_t payload_length);
extern void send_eapol_data_to_mac_request(uint8_t *eapol_data, uint16_t data_len);
void send_eapol_packt_to_lbr(uint8_t *buff , uint16_t len , uint8_t *self_addr);
#endif

extern uint8_t is_send_as_udp(uint8_t *data_ptr);
extern uint8_t get_current_join_state();
void trickle_timer_inconsistency_pc(void);
void trickle_timer_consistency_pc(void);
extern void add_dev_desc_on_MAC_for_security(uint8_t* macAddrOfNeighbour);
extern void reset_incoming_frame_counter_for_stale_key (uint8_t stale_key_index);
uint8_t set_revoaction_key(uint8_t *revoaction_list, uint16_t len);
uint8_t set_lbr_mac_lifetime_config (uint32_t pmk_lifetime,
                                     uint32_t ptk_lifetime,
                                     uint32_t gtk_lifetime,
                                     uint32_t gtk_new_activation_time,
                                     uint32_t revocation_lifetime_reduction);
void send_all_param_info_req (void);

#if(FAN_FRQ_HOPPING_FEATURE_ENABLED == 1)
void fan_freq_hop_start_hopping (void *);
#endif

void send_dhcp_data_to_server(uint8_t* buff,uint16_t len);
void set_key_life_time(uint8_t *buf,uint16_t len);
void send_revocation_key(uint8_t *buff , uint16_t len);
void send_gtk_update_indication(uint8_t *pBuff,uint16_t len);
void send_mac_security_set_request(uint8_t *pBuff,uint16_t len);
void set_seq_key(uint8_t live_key_id_index);
void send_gtkhash_to_hostapd(uint8_t *buff , uint16_t len);
int get_join_state (void);
/*
** =============================================================================
** Public Variables Definitions
** =============================================================================
**/

uint8_t phyModeMapArr[8]={1,2,4,8,16,32,64,128};
uint8_t response_laye_ID = APP_DEF_LAYER_ID_TOOL;
/*
** =============================================================================
** Public Function Prototypes
** =============================================================================
*/

uint8_t gu2pcapp_capture_ns_conf(uint8_t status);
void process_tx_sechedule_end(uint8_t);
void process_tx_sechedule_start(uint8_t val);
void fan_nwk_manager_init( );
uint8_t gu2pcapp_node_config_set_conf( uint8_t status );


/*
** =============================================================================
** Public Function Definitions
** =============================================================================
*/

/*----------------------------------------------------------------------------*/
uint32_t ChangeEndianness(uint32_t value)
{
    uint32_t result = 0;
    result |= (value & 0x000000FF) << 24;
    result |= (value & 0x0000FF00) << 8;
    result |= (value & 0x00FF0000) >> 8;
    result |= (value & 0xFF000000) >> 24;
    return result;
}
/*----------------------------------------------------------------------------*/
enet_msg_t* allocate_hif_msg( uint16_t length )
{
    enet_msg_t *msg = NULL;

    msg = (enet_msg_t *) app_bm_alloc( length + 6 );// 5 for including next and data_length members

    if (msg != NULL)
    {
        msg->data_length = length;
    }
    return msg;
}
/*----------------------------------------------------------------------------*/
static void send_cmd_not_support()
{
  uint8_t host_pkt[2];
  host_pkt[0] = ACTIVE_CMD_NOT_SUPPORTED;
  host_pkt[1] = 1; 
//[Kimbal]
//  hif_send_msg_up(host_pkt,2,response_laye_ID,PROTOCOL_ID_FOR_APP);
 
}

#if 0 
/*----------------------------------------------------------------------------*/
void free_hif_msg( enet_msg_t * msgp )
{
    app_bm_free((uint8_t*)msgp);
}


/*----------------------------------------------------------------------------*/
void APPhifForToolTest_Init(void)
{
  hif_register_parser
     (
          &hif,
          &hif_fan_test,
          APP_DEF_LAYER_ID_TOOL,
          hif_2_App_Interface_cb
     );
  

}
#endif //#if 0 
/*----------------------------------------------------------------------------*/

//void process_set_brodcast_timing_info_ie(uint8_t *buf, uint16_t length)
//{
//  mem_rev_cpy((uint8_t*)&fan_nwk_manager_app.fan_mac_header_ie.bt_ie.broadcast_time_interval,buf,2);
//  mem_rev_cpy((uint8_t*)&fan_nwk_manager_app.fan_mac_header_ie.bt_ie.broadcast_fraction_interval_offset,&buf[2],4);
//  fan_nwk_manager_app.fan_mac_header_ie.bt_ie.length = length;
//  send_hif_conf_cb(FAN_BROADCAST_TIMING_SET_CONFIRM,0x00);
//}


/*******************************************************************************
          Application to HIF Send 
*******************************************************************************/

/*----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*/
uint8_t gu2pcapp_node_config_set_conf( uint8_t status )
{
  uint8_t hif_Send_buff [5] = {0};
  uint8_t* buf = &hif_Send_buff[0];
  
  
  *buf++ = 0xE1;
  *buf++ = status;//status
  *buf++ = DUMMY_COMPORT;//Dummy Comport       
  //hif_send_msg_up(&hif_Send_buff[0], 2,response_laye_ID,PROTOCOL_ID_FOR_APP);
  return 0;
}

/*----------------------------------------------------------------------------*/

uint8_t gu2pcapp_process_para_problem_msg_ind
(
 uint8_t type,
 uint8_t code,
 uint16_t checksum,
 uint16_t payload_len,
 uint8_t* p_payload
)
{
  
 // Raka .. [23-Nov-2017]
  /*
      Since following 2 files coming as zero
  I have taken the static Variable
   uint16_t payload_len,
 uint8_t* p_payload
  */
    uint8_t hif_Send_buff [8] = {0};
    uint8_t* buf = &hif_Send_buff [0];


    *buf++ = ENET_2_APP_PARA_PROB_IND_CMD_ID;
    *buf++ = type;
    *buf++ = code;
    put_ushort(buf,checksum);
    buf += 2; 
    *buf++ = DUMMY_COMPORT;//Dummy Comport     
         
    //hif_send_msg_up(&hif_Send_buff[0], 5,response_laye_ID,PROTOCOL_ID_FOR_APP);
    return 0;
}

/*----------------------------------------------------------------------------*/
uint8_t gu2pcapp_send_dest_unreachable_icmp_ind
(
 uint8_t type,
 uint8_t code,
 uint16_t checksum,
 uint16_t payload_len,
 uint8_t* p_payload
)
{
     
     // Raka .. [23-Nov-2017]
  /*
      Since following 2 files coming as zero
  I have taken the static Variable
   uint16_t payload_len,
 uint8_t* p_payload
  */
    uint8_t hif_Send_buff [8] = {0};
    uint8_t* buf = &hif_Send_buff [0];


    *buf++ = ENET_2_APP_DEST_UNREACHABLE_CMD_ID;
    *buf++ = type;
    *buf++ = code;
    put_ushort(buf,checksum);
    buf += 2; 
    *buf++ = DUMMY_COMPORT;//Dummy Comport 
    //hif_send_msg_up(&hif_Send_buff[0], 5,response_laye_ID,PROTOCOL_ID_FOR_APP);
     return 0;
}

uint8_t gu2pcapp_send_ns_ind
(
 uint8_t type,
 uint8_t code,
 uint16_t checksum,
 uip_ipaddr_t* p_tgt_addr,
 uint8_t icmpv6_opt_type,
 uint8_t icmpv6_opt_len,
 uip_lladdr_t* p_tlla
)
{
//     //capture_nans_and_send = 0;
//     enet_msg_t* msg = allocate_hif_msg( 40 );
//     uint8_t* buf = msg->data;	
//
//     if(msg != NULL)
//     {
//          //*buf++ = ENET_2_APP_NA_IND;
//          
//          //#define ENET_2_APP_NA_IND_CMD_ID                     		0x05
////#define ENET_2_APP_NS_IND_CMD_ID                     		0x06
//          *buf++ = ENET_2_APP_NS_IND_CMD_ID;
//          
//          *buf++ = type;
//          *buf++ = code;
//          put_ushort(buf,checksum);
//         buf += 2; 
//
//          memcpy(buf,p_tgt_addr->u8,sizeof(uip_ipaddr_t));
//          buf += sizeof(uip_ipaddr_t);
//
//          *buf++ = icmpv6_opt_type;
//          *buf++ = icmpv6_opt_len;
//          
//          memcpy(buf,p_tlla->addr,sizeof(uip_lladdr_t));
//          buf += sizeof(uip_lladdr_t);
//
//          msg->data_length  = buf - msg->data;
//
//          hif_send_msg_up(msg->data,msg->data_length-1,6);
//         
//          free_hif_msg( msg );
//     }

     return 0;
  
}
/*----------------------------------------------------------------------------*/

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
)
{
////    #define ENET_2_APP_NA_IND                      0x05
////  // send echo reply
////     //capture_nans_and_send = 0;
////     enet_msg_t* msg = allocate_hif_msg( 40 );
////     uint8_t* buf = msg->data;	
////
////     if(msg != NULL)
////     {
////          //*buf++ = ENET_2_APP_NA_IND;
////          
////          //#define ENET_2_APP_NA_IND_CMD_ID                     		0x05
//////#define ENET_2_APP_NS_IND_CMD_ID                     		0x06
////          *buf++ = ENET_2_APP_NA_IND_CMD_ID;
////          
////          *buf++ = type;
////          *buf++ = code;
////          put_ushort(buf,checksum);
////         buf += 2; 
////          *buf++ = osr_flag;
////          
////          memcpy(buf,p_tgt_addr->u8,sizeof(uip_ipaddr_t));
////          buf += sizeof(uip_ipaddr_t);
////          
////          *buf++ = icmpv6_opt_type;
////          *buf++ = icmpv6_opt_len;
////          
////          memcpy(buf,p_tlla->addr,sizeof(uip_lladdr_t));
////          buf += sizeof(uip_lladdr_t);
////
////          msg->data_length  = buf - msg->data;
////          hif_send_msg_up(msg->data,msg->data_length-1,6);         
////          free_hif_msg( msg );
////     }
//
     return 0;
}
/*----------------------------------------------------------------------------*/

uint8_t gu2pcapp_send_echo_req_res
(
 uint8_t type,
 uint8_t code,
 uint16_t checksum,
 uint16_t id,
 uint16_t seq_num,
 uint16_t data_len,        
 uint8_t* p_data
)
{
  /* Debdeep:: 07-sep-2018:: 15 bytes more is added for buffer allocation length.
     Because we store type, code, checksum, seq num, response counter and data length 
     along with ping data */
  enet_msg_t* msg = allocate_hif_msg (data_len + 15 + sizeof(enet_msg_t));
  
  uint8_t* buf = msg->data;	
  
  if(msg != NULL)
  {    
    *buf++ = ((type==ICMP6_ECHO_REPLY)? RECV_ICMPV6_REPLY_IND:RECV_ICMPv6_REQ);
    *buf++ = type;
    *buf++ = code;
    put_ushort(buf,checksum);
    buf += 2;
    put_ushort_BE(buf,id);
    buf += 2;
    put_ushort_BE(buf,seq_num);
    buf += 2;
    ++icmv6_response_counter;
    if(icmv6_response_counter == 0xFFFF)
    {
      icmv6_response_counter = 0x00;
    }
    mem_rev_cpy(buf,(uint8_t*)&icmv6_response_counter,2);
    buf += 2;
    
    put_ushort_BE(buf,data_len);
    buf += 2;
    
    memcpy(buf,p_data,data_len);
    buf += data_len;
    *buf++ = DUMMY_COMPORT;
    msg->data_length  = buf - msg->data;
    //hif_send_msg_up(msg->data,msg->data_length-1,response_laye_ID,PROTOCOL_ID_FOR_APP);         
    //free_hif_msg( msg );
  }
  return 0;
}
/*----------------------------------------------------------------------------*/

//uint8_t App_UDP_Data_Indication_cb(
//         uint8_t* data,
//         uint16_t len, 
//         uint8_t* p_src_ll_addr, 
//         uint16_t sender_port )
//{
//    enet_msg_t* msg = allocate_hif_msg( len + 10 );
//     uint8_t* buf = msg->data;	
//    
//     if(msg != NULL)
//     {
//          *buf++ = APP_2_FAN_UDP_RCV_CB;// cmd id
//          
//          mem_reverse_cpy( buf,p_src_ll_addr,8 );
//          buf+=8;
//          put_ushort_BE(buf,sender_port); buf+=2;
//          put_ushort_BE(buf,len); buf+=2;
//          memcpy( buf,data,len );
//          buf += len;
//
//          msg->data_length  = buf - msg->data;
//          hif_send_msg_up(msg->data,msg->data_length-1,6);         
//          free_hif_msg( msg );
//     }
//  
//     return 0;
//}
/*----------------------------------------------------------------------------*/

/*------------set_fan_api as per given spec-----------------------------------*/
void process_set_node_start_stop(uint8_t *buf, uint16_t length)
{
  
   //uint8_t status = 0xFF;
  uint8_t node_start = *buf; //*(buf+3); // ENUM From Wi-SUN Test Bench is of 4 Bytes
  
  node_start_stop( node_start);
 
}
/*----------------------------------------------------------------------------*/


/*-----------------------start for TLV FORMAT-------------------------------*/
//void process_set_mac_unicast_api_tbc(uint8_t *buf, uint16_t length)
//{
//  uint8_t status = 0xFF;
//  uint8_t channel_index = 0;
//  uint8_t dwell_interval = 0x00;
//  uint8_t channel_function = 0x00;
//  uint8_t exc_chnl_cont = 0x00;
//  uint8_t excluded_channel_Range = 0x00;
//  uint16_t excluded_channel[20] = {0};
//  uint8_t u_ex_ch_mask[17] = {0};
//
//  uint8_t offset = 0;
//  while(length > offset)
//  {
//    uint8_t var_type = *buf++;
//    uint16_t rcvd_length = 0x00;
//    
//    if(var_type == DWELL_INTERVAL_US)
//    {
//      rcvd_length =  *buf++;
//      dwell_interval = *buf;
//      buf += rcvd_length;
//    }
//    else if(var_type == CHANNEL_FUNCTION_US)
//    {
//      rcvd_length =  *buf++;
//      channel_function = *buf;
//      buf += rcvd_length;
//    }
//    else if(var_type == EXC_CHAN_RANGE_US)
//    {
//      rcvd_length =  *buf++;;
//      uint8_t temp_len = rcvd_length;
//      
//      if(rcvd_length>0)
//        exc_chnl_cont = 1;
//     
//      while(temp_len!=0)
//      {
//        mem_rev_cpy((uint8_t *)&excluded_channel[channel_index++],buf,2);
//        buf +=2;
//        temp_len-=2;
//        excluded_channel_Range++;
////        if((excluded_channel[0] )&& (excluded_channel[1] )!= 0x000)                   //Arjun: this logic is not making excluded_channel_Range 1 for a pair of range
////        {
////          excluded_channel_Range++;
////          exc_chnl_cont = 1;
////        }
//      }
//      excluded_channel_Range = (excluded_channel_Range/2);
//    }
//    else if(var_type == EXC_CHAN_MASK_US)
//    {
//        rcvd_length =  *buf++;
//        uint8_t temp_len = rcvd_length;
//        
//      if(rcvd_length != 0)
//      {
//        exc_chnl_cont = 2;
///* Debdeep :: Excluded channel mask should be set in while loop :: problem occured while testing with LnG */
//        while(temp_len != 0x00)
//        {
//          mem_rev_cpy((uint8_t *)&u_ex_ch_mask[channel_index++],buf,1);
//          buf +=1;
//          temp_len-=1;
//          excluded_channel_Range++;
//        }
////        if((excluded_channel[0] )&& (excluded_channel[1] )!= 0x000)
////        {
////          excluded_channel_Range++;
////          //exc_chnl_cont = 2;
////        }
//      }
//
//    }
//    offset += (rcvd_length+1+1);//TYPE+LENTHBYTE
//  }
//   status = set_mac_unicast_chan_plan(dwell_interval,
//                                     channel_function,
//                                     exc_chnl_cont,
//                                     excluded_channel_Range,
//                                     excluded_channel,
//                                     u_ex_ch_mask);
//  
//  send_hif_conf_cb(SET_MAC_UNICAST_API_CONF,status);  
//}

/*----------------------------------------------------------------------------*/
//void process_set_mac_broadcast_api_tbc(uint8_t *buf, uint16_t length)
//{   
//  uint8_t status = 0xFF;
//  uint8_t channel_index = 0;  
//  uint32_t broad_cast_intver = 0x00000000;
//  uint16_t b_sech_indinti = 0x0000;
//  uint8_t dwell_interval = 0x00;
//  uint8_t channel_function = 0x00;
//  uint8_t exc_chnl_cont = 0x00;
//  uint8_t excluded_channel_Range = 0x00;
//  uint16_t excluded_channel[20] = {0};
//  uint8_t b_ex_ch_mask[17] = {0};
//  
//    uint8_t offset = 0;
//  while(length > offset)
//  {
//    uint8_t var_type = *buf++;
//    uint16_t rcvd_length = 0x00;
//    if(var_type == BCAST_INTERVAL_BS)
//    {
//      rcvd_length =  *buf++;
//      mem_rev_cpy((uint8_t *)&broad_cast_intver,&buf[0],4);
//      buf += rcvd_length;
//    }
//    else if(var_type == BCAST_SCH_IDNT_BS)
//    {
//      rcvd_length =  *buf++;
//      mem_rev_cpy((uint8_t *)&b_sech_indinti,&buf[0],2);
//      buf += rcvd_length;
//    }
//    else if(var_type == DWELL_INTERVAL_BS)
//    {
//      rcvd_length =  *buf++;
//      dwell_interval = *buf;
//      buf += rcvd_length;
//    }
//    else if(var_type == CHANNEL_FUNCTION_BS)
//    {
//      rcvd_length =  *buf++;
//      channel_function = *buf;
//      buf += rcvd_length;
//    }
//    else if(var_type == EXC_CHAN_RANGE_BS)
//    {
//      rcvd_length =  *buf++;;
//      uint8_t temp_len = rcvd_length;
//      if(rcvd_length>0)
//        exc_chnl_cont = 1;
//      
//      while(temp_len!=0)
//      {
//        mem_rev_cpy((uint8_t *)&excluded_channel[channel_index++],buf,2);
//        buf +=2;
//        temp_len-=2;
//        excluded_channel_Range++;
////        if((excluded_channel[0] )&& (excluded_channel[1] )!= 0x000)                   //Arjun: this logic is not making excluded_channel_Range 1 for a pair of range
////        {
////        {
////          excluded_channel_Range++;
////          exc_chnl_cont = 1;
////        }
//      }
//      excluded_channel_Range = (excluded_channel_Range/2);
//    }
//    else if(var_type == EXC_CHAN_MASK_BS)
//    {
//        rcvd_length =  *buf++;
//        uint8_t temp_len = rcvd_length;
//        
//      if(rcvd_length != 0)
//      {
//        exc_chnl_cont = 2;
//        mem_rev_cpy((uint8_t *)&excluded_channel[channel_index++],buf,2);
//        buf +=2;
//        temp_len-=2;
//         if((excluded_channel[0] )&& (excluded_channel[1] )!= 0x000)
//          {
//            excluded_channel_Range++;
//            exc_chnl_cont = 2;
//          }
//      }
//      
//    }
//    offset += (rcvd_length+1+1);//TYPE+LENTHBYTE
//  }
//  
//  status =  set_mac_bcast_chan_plan(broad_cast_intver,
//                                     b_sech_indinti,
//                                     dwell_interval,
//                                     channel_function,
//                                     exc_chnl_cont,
//                                     excluded_channel_Range,
//                                     excluded_channel,
//                                     b_ex_ch_mask);
//  
//  send_hif_conf_cb(SET_MAC_BROADCAST_API_CONF,status);
//}

/*----------------------------------------------------------------------------*/
//void process_set_mac_unicast_api(uint8_t *buf, uint16_t length)
//{
//   uint8_t status = 0xFF;
//  uint8_t channel_index = 0;
//  uint8_t dwell_interval = *buf++;
//  uint8_t channel_function = *buf++;
//  uint8_t exc_chnl_cont = 0x00;
//  uint8_t excluded_channel_Range = *buf++;
//  uint16_t excluded_channel[20] = {0};
//  uint8_t u_ex_ch_mask[17] = {0};
// if(excluded_channel_Range != 0x00)
// {
//  for(int i =1 ; i<= excluded_channel_Range;i++)
//  {
//    
//    excluded_channel[channel_index++] = *buf++;
//    excluded_channel[channel_index++] = *buf++;
//    exc_chnl_cont = 1;
//  }
// }
//else if(excluded_channel_Range == 0x00)
//{
//  if(*buf != 0x00)
//    {
//      memcpy(u_ex_ch_mask,buf,16);
//      exc_chnl_cont = 2;
//    }
//}
//   status = set_mac_unicast_chan_plan(dwell_interval,
//                                     channel_function,
//                                     exc_chnl_cont,
//                                     excluded_channel_Range,
//                                     excluded_channel,
//                                     u_ex_ch_mask);
//  
//  send_hif_conf_cb(SET_MAC_UNICAST_API_CONF,status);  
//}
/*----------------------------------------------------------------------------*/
//void process_set_mac_broadcast_api(uint8_t *buf, uint16_t length)
//{
//   uint8_t status = 0xFF;
//  uint8_t channel_index = 0;  
//  uint32_t broad_cast_intver = 0x00000000;
//  uint16_t b_sech_indinti = 0x0000;
//  mem_rev_cpy((uint8_t *)&broad_cast_intver,&buf[0],4);
//  buf+=4;
//  mem_rev_cpy((uint8_t *)&b_sech_indinti,&buf[0],2);
//  buf+=2;
//  uint8_t dwell_interval = *buf++;
//  uint8_t channel_function = *buf++;
//  uint8_t exc_chnl_cont = 0x00;//*buf++;
//  uint8_t excluded_channel_Range = *buf++;
//  uint16_t excluded_channel[20] = {0};
//  uint8_t b_ex_ch_mask[17] = {0};
//  
// if(excluded_channel_Range != 0x00)
// {
//  for(int i =1 ; i<= excluded_channel_Range;i++)
//  {
//    
//    excluded_channel[channel_index++] = *buf++;
//    excluded_channel[channel_index++] = *buf++;
//    exc_chnl_cont = 1;
//  }
// }
//else if(excluded_channel_Range == 0x00)
//{
//    if(*buf != 0x00)
//    {
//        memcpy(b_ex_ch_mask,buf,16);
//        exc_chnl_cont = 2;
//    }  
//}
//  
//  status =  set_mac_bcast_chan_plan(broad_cast_intver,
//                                     b_sech_indinti,
//                                     dwell_interval,
//                                     channel_function,
//                                     exc_chnl_cont,
//                                     excluded_channel_Range,
//                                     excluded_channel,
//                                     b_ex_ch_mask);
//  
//  send_hif_conf_cb(SET_MAC_BROADCAST_API_CONF,status);
//}
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/

void process_set_mac_reg_op_api(uint8_t *buf, uint16_t length)
{
  
//   uint8_t status = 0xFF;
//  uint8_t reg_domain = *buf++;
//  uint8_t oper_class = *buf++;
//  status = set_mac_chan_plan_reg_op(reg_domain, oper_class);
//  send_hif_conf_cb(SET_MAC_CHANNEL_PLAN_REG_OP_API_CONF,status);  
}
/*----------------------------------------------------------------------------*/
//void process_set_mac_explicit_api(uint8_t *buf, uint16_t length)
//{
//   uint8_t status = 0xFF;
//    uint32_t CH0 = 0x000000;//0x0DC438;
//    uint16_t channel_numbers = 0x0000;
//    mem_rev_cpy((uint8_t *)&CH0 ,&buf[0],4);
//    buf+=4;
//    uint8_t channelspacing = *buf++;
//    mem_rev_cpy((uint8_t *)&channel_numbers,&buf[0],2);
//    buf+=2;
//  status =  set_mac_chan_plan_explicit( CH0, channelspacing, channel_numbers);
//  
//  send_hif_conf_cb(SET_MAC_CHANNEL_PLAN_EXPLICIT_API_CONF,status);
//}
/*----------------------------------------------------------------------------*/
void process_set_mac_fixed_chan(uint8_t *buf, uint16_t length)
{
  uint8_t status = 0xFF;
  uint16_t fixed_channel = 0;
  mem_rev_cpy((uint8_t *)&fixed_channel ,&buf[0],2);
  status =  set_mac_chan_plan_fixed(fixed_channel);
  
 //if(fan_nwk_manager_app.node_basic_cfg.operational_mode == FACTORY_MODE) 
     App_factory_mode_channel_set();
 
  send_hif_conf_cb(SET_MAC_CHAN_PLAN_FIXED_CONF,status);
}
/*----------------------------------------------------------------------------*/
//void process_set_lbr_mac_gtks_config_tbc(uint8_t *buf, uint16_t length)
//{
//    uint8_t status = 0xFF;
//  uint8_t GTK0_Hash[16] = {0};
//  uint8_t GTK1_Hash[16] = {0};
//  uint8_t GTK2_Hash[16] = {0};
//  uint8_t GTK3_Hash[16] = {0};
//  uint8_t gtkl = 0;
//  
//
//  uint8_t offset = 0;
//  while(length > offset)
//  {
//    uint8_t var_type = *buf++;
//    uint16_t rcvd_length = 0x00;
//    if(var_type == GTK0)
//    {
//      rcvd_length =  *buf++;
//      memcpy(GTK0_Hash,&buf[0],rcvd_length);
//      buf += rcvd_length;
//      gtkl |= 0x01;
//    }else if(var_type == GTK1)
//    {
//      rcvd_length =  *buf++;
//      memcpy(GTK1_Hash,&buf[0],rcvd_length);
//      buf += rcvd_length;
//      gtkl |= 0x02;
//    }else if(var_type == GTK2)
//    {
//      rcvd_length =  *buf++;
//      memcpy(GTK2_Hash,&buf[0],rcvd_length);
//      buf += rcvd_length;
//      gtkl |= 0x04;
//    }else if(var_type == GTK3)
//    {
//      rcvd_length =  *buf++;
//      memcpy(GTK3_Hash,&buf[0],rcvd_length);
//      buf += rcvd_length;
//      gtkl |= 0x08;
//    }
//    offset += (rcvd_length+1+1);
//  }
//  status = set_lbr_mac_gtks_config(GTK0_Hash,
//                                   GTK1_Hash,
//                                   GTK2_Hash,
//                                   GTK3_Hash,
//                                   gtkl);
//  send_hif_conf_cb(SET_LBR_MAC_GTKS_CONFIG_CONF,status);
//}

//void process_set_lbr_mac_gtks_config(uint8_t *buf, uint16_t length)
//{
//  uint8_t status = 0xFF;
//  uint8_t GTK0_Hash[16] = {0};
//  uint8_t GTK1_Hash[16] = {0};
//  uint8_t GTK2_Hash[16] = {0};
//  uint8_t GTK3_Hash[16] = {0};
//  uint8_t gtkl = 0;
//  
//  if (memcmp (buf, GTK0_Hash, 16))
//  {
//    memcpy(GTK0_Hash,buf,16);
//    gtkl |= 0x01;
//  }
//  buf+=16;
//  if (memcmp (buf, GTK1_Hash, 16))
//  {
//    memcpy(GTK1_Hash,buf,16);
//    gtkl |= 0x02;
//  }
//  buf+=16;
//  if (memcmp (buf, GTK2_Hash, 16))
//  {
//    memcpy(GTK2_Hash,buf,16);
//    gtkl |= 0x04;
//  }
//  buf+=16;
//  if (memcmp (buf, GTK3_Hash, 16))
//  {
//    memcpy(GTK3_Hash,buf,16);
//    gtkl |= 0x08;
//  }
//
//  status = set_lbr_mac_gtks_config(GTK0_Hash,
//                                   GTK1_Hash,
//                                   GTK2_Hash,
//                                   GTK3_Hash,
//                                   gtkl);
//  send_hif_conf_cb(SET_LBR_MAC_GTKS_CONFIG_CONF,status);
//}

//void process_revoke_sta_request (uint8_t *sta_address, uint16_t length)
//{
//  uint8_t hifBuff[15] = {0};
//  uint8_t* buf = &hifBuff[0];
//  
//  *buf++ = SEND_REVOKE_STA_REQ;
//  memcpy (buf, sta_address, 8);
//  buf += 8;
//  *buf++ = DUMMY_COMPORT;//Dummy Comport 
//  hif_send_msg_up (hifBuff, buf - hifBuff, response_laye_ID, PROTOCOL_ID_FOR_APP);
//}

/*----------------------------------------------------------------------------*/
//void process_set_lbr_mac_pmk_ptk_gtk_lifetime_config(uint8_t *buf, uint16_t length)
//{
//  uint8_t status = 0xFF;
//  uint32_t pmk_lifetime = 0;
//  uint32_t ptk_lifetime = 0;
//  uint32_t gtk_lifetime = 0;
//  uint32_t gtk_new_activation_time = 0;
//  uint32_t revocation_lifetime_reduction = 0;
//  uint8_t tag = 0;              
//  uint8_t len = 0;
//  
//  if (response_laye_ID == APP_DEF_LAYER_ID_TOOL)
//  {
//    mem_rev_cpy ((uint8_t*)&pmk_lifetime, buf, 4);
//    buf += 4;
//    mem_rev_cpy ((uint8_t*)&ptk_lifetime, buf, 4);
//    buf += 4;
//    mem_rev_cpy ((uint8_t*)&gtk_lifetime, buf, 4);
//    buf += 4;
//    mem_rev_cpy ((uint8_t*)&gtk_new_activation_time, buf, 4);
//    buf += 4;
//    mem_rev_cpy ((uint8_t*)&revocation_lifetime_reduction, buf, 4);
//  }
//  
//  if (response_laye_ID == APP_DEF_LAYER_ID_TBC)
//  {
//    while (length > 0)
//    {
//      tag = *buf++;
//      len = *buf++;
//      
//      if (tag == PMK_LIFETIME)
//      {
//        mem_rev_cpy ((uint8_t*)&pmk_lifetime, buf, len);
//        buf += len;
//      }
//      if (tag == PTK_LIFETIME)
//      {
//        mem_rev_cpy ((uint8_t*)&ptk_lifetime, buf, len);
//        buf += len;
//      }
//      if (tag == GTK_LIFETIME)
//      {
//        mem_rev_cpy ((uint8_t*)&gtk_lifetime, buf, len);
//        buf += len;
//      }
//      if(tag == GTK_NEW_ACTIVATION_TIME)
//      {
//        mem_rev_cpy ((uint8_t*)&gtk_new_activation_time, buf, len);
//        buf += len;
//      }
//      if(tag == REVOCATION_LIFETIME_REDUCTION)
//      {
//        mem_rev_cpy ((uint8_t*)&revocation_lifetime_reduction, buf, len);
//        buf += len;
//      }
//      length -= (len+2);
//    } 
//  }
//  
//  /*Converting lifetime from minutes to seconds*/
//  pmk_lifetime *= 60;
//  ptk_lifetime *= 60;
//  gtk_lifetime *= 60;
//  
//  status = set_lbr_mac_lifetime_config (pmk_lifetime, 
//                                        ptk_lifetime, 
//                                        gtk_lifetime,
//                                        gtk_new_activation_time,
//                                        revocation_lifetime_reduction);
//  send_hif_conf_cb (SET_LBR_MAC_PMK_PTK_GTK_LIFETIME_CONFIG_CONF, status);
//}
/*----------------------------------------------------------------------------*/
extern uint16_t max_frag_size;
extern uint16_t frag_chunk_send;
//void process_set_lbr_mac_config_tbc(uint8_t *buf, uint16_t length)
//{
//  uint8_t status = 0xFF;
//  uint16_t pan_size = 0x0000;
//  uint16_t pan_id   = 0x0000;
//  uint8_t use_bs_sch = 0x00;
//  uint8_t routing_meth = 0;
//  uint8_t net_name[30] = {0};
//  uint8_t index = 0;
//  uint16_t max_sisl_mtusize = 0x0000;
//  mem_rev_cpy((uint8_t *)&pan_id ,&buf[0] ,2);
//  buf+=2;
//  length-=2;
//  mem_rev_cpy((uint8_t *)&pan_size ,&buf[0] ,2);
//  buf+=2;
//  length-=2;
//  use_bs_sch = *buf++;
//  length-=1;
//  routing_meth = *buf++;
//  length-=1;
//  while(*buf != 0x00)
//  {
//    
//    net_name[index++] = *buf++;
//    length-=1;
//  }
//   *buf++;//for NULL char
//  length-=1;
//  if(length != 0)
//  {
//    mem_rev_cpy((uint8_t *)&max_sisl_mtusize,&buf[0],2);
//    max_frag_size = frag_chunk_send = max_sisl_mtusize;
//    buf+=2;
//  }
// 
//  
//  status = set_lbr_mac_config(pan_size,
//                              pan_id,
//                              use_bs_sch,
//                              routing_meth,
//                              net_name);
//  
//  send_hif_conf_cb(SET_LBR_MAC_CONFIG_CONF,status);
//}
//void process_set_lbr_mac_config(uint8_t *buf, uint16_t length)
//{
//  uint8_t status = 0xFF;
//  uint16_t pan_size = 0x0000;
//  uint16_t pan_id   = 0x0000;
//  uint8_t use_bs_sch = 0x00;
//  uint8_t routing_meth = 0;
//  uint8_t net_name[30] = {0};
//  mem_rev_cpy((uint8_t *)&pan_id ,&buf[0] ,2);
//  buf+=2;
//  mem_rev_cpy((uint8_t *)&pan_size ,&buf[0] ,2);
//  buf+=2;
//  use_bs_sch = *buf++;
//  routing_meth = *buf++;
//  
//  uint16_t net_name_len = 0x00;
//  mem_rev_cpy((uint8_t *)&net_name_len ,&buf[0] ,2);
//  buf+=2;
//  memcpy((uint8_t *)&net_name ,&buf[0] ,net_name_len);
//  buf+=net_name_len;
//  
//  status = set_lbr_mac_config(pan_size,
//                              pan_id,
//                              use_bs_sch,
//                              routing_meth,
//                              net_name);
//  
//  send_hif_conf_cb(SET_LBR_MAC_CONFIG_CONF,status);
//}

/*----------------------------------------------------------------------------*/

void process_reset_rpl_msg_rate(uint8_t *buf, uint16_t length)
{
  uint8_t status = 0xFF;
  uint8_t rpl_messege = *buf;//*(buf+3);
  status = reset_rpl_msg_rate(rpl_messege);
  send_hif_conf_cb(RESET_RPL_MSG_RATE_CONF,status);
}

/*----------------------------------------------------------------------------*/

void process_get_sec_keys(uint8_t *buf, uint16_t length)
{
  uint8_t SecKey[64] = {0x00};
  memcpy(&SecKey[0],mac_key_list.MAC_SECURITY_KEY_LIST[0].MAC_SECURITY_KEY,16);
  memcpy(&SecKey[16],mac_key_list.MAC_SECURITY_KEY_LIST[1].MAC_SECURITY_KEY,16);
  memcpy(&SecKey[32],mac_key_list.MAC_SECURITY_KEY_LIST[2].MAC_SECURITY_KEY,16);
  memcpy(&SecKey[48],mac_key_list.MAC_SECURITY_KEY_LIST[3].MAC_SECURITY_KEY,16);
    send_hif_seckey_cb(GET_SEC_KEYS_CONF,SecKey);
}

/*----------------------------------------------------------------------------*/

void process_get_ip_address(uint8_t *buf, uint16_t length)
{
  get_ip_address();
}

/*----------------------------------------------------------------------------*/


void process_wan_ping_reply(uint8_t *buf, uint16_t length)
{
  recved_wan_ping_reply (*buf);
}

/*----------------------------------------------------------------------------*/

void send_udp_request(uint8_t *buf, uint16_t length)
{
  uint8_t src_ipv6_addr[16] = {0x00};
  uint16_t src_port_num = 0x0000;
  uint8_t dst_ipv6_addr[16] = {0x00};
  uint16_t dst_port_num = 0x0000;
  uint16_t data_length = 0x00;

        memcpy(src_ipv6_addr,&buf[0],16);
        buf+=16;
        mem_rev_cpy((uint8_t *)&src_port_num,&buf[0],2);
        buf+=2;
        memcpy(dst_ipv6_addr,&buf[0],16);
        buf+=16;
        mem_rev_cpy((uint8_t *)&dst_port_num,&buf[0],2);
        buf+=2;        

  mem_rev_cpy((uint8_t *)&data_length,&buf[0],2);
  buf+=data_length;  
  //memcpy(data,&buf[0],data_length);
//  send_udp(src_ipv6_addr,src_port_num,dst_ipv6_addr,dst_port_num,buf,data_length);
}

/*------------------------------Arjun for swagger----------------------------------------------*/
void start_timer_to_send_ping (uint64_t timeval, void *data);
extern void send_icmpv6_with_count(uint8_t *buf, uint16_t length);

/*----------------------------------------------------------------------------*/

void send_icmpv6_after_delay (void *data)
{
  uint8_t *buf = NULL;
  uint16_t length = 0;
  memcpy ((uint8_t *)&length, (uint8_t *)data, 2);
  buf = (uint8_t *)data + 2;
  send_icmpv6 (buf, length);
  app_bm_free ((uint8_t *)data);
}

/*----------------------------------------------------------------------------*/

//void send_icmpv6_request_tbc(uint8_t *buf, uint16_t length)
//{
//  uint8_t status = 0;
//  send_hif_conf_cb(SEND_ICMPv6_CONF,status);
//  uint8_t *ping_data = app_bm_alloc (length + 2);
//  memcpy (ping_data, &length, 2);
//  memcpy (ping_data + 2, buf, length);
//  start_timer_to_send_ping (200, ping_data);       /*Ping will be out after 200 milliseconds*/
//}

/*----------------------------------------------------------------------------*/

void send_icmpv6_request(uint8_t *buf, uint16_t length)
{
  send_icmpv6_with_count(buf,length);
}

/*----------------------------------------------------------------------------*/

//void send_icmpv6_request(uint8_t *buf, uint16_t length)
//{
//  uint8_t src_ipv6_addr[16] = {0x00};
//  uint8_t dst_ipv6_addr[16] = {0x00};
//  uint16_t data_length = 0x00;
////  uint8_t data[200]={0};
//  
////  if(ipv6_address_flag==1)
////  {
//    memcpy(src_ipv6_addr,&buf[0],16);
//    buf+=16;
//    memcpy(dst_ipv6_addr,&buf[0],16);
//    buf+=16;
////  }
////  else
////  {
////    memcpy(src_ipv6_addr,&buf[0],8);
////    buf+=8;
////    memcpy(dst_ipv6_addr,&buf[0],8);
////    buf+=8;
////  } 
//  mem_rev_cpy((uint8_t *)&data_length,&buf[0],2);
//  buf+=2;
////  memcpy(&data,&buf[0],data_length);
////  buf+=data_length;  
////  memcpy(data,&buf[0],data_length);
//  
//  
////  send_icmpv6
////              (
////               src_ipv6_addr,
////               0,
////               dst_ipv6_addr,
////               0,
////               0,
////               0,
////               buf,
////               data_length 
////               );
//  
//  
//}
/*----------------------------------------------------------------------------*/
//void trigger_subscribe_packet(uint8_t *buf, uint16_t length)
//{
//  uint8_t status = 0xFF;
// // uint8_t fwd_address[8] = {0};
//  //uint16_t fwd_port = 0;
//  uint8_t command = *buf++;
//  //uint8_t pakt_type = *buf++;//*(buf+3);         //Arjun(28-09-17): discarded as of api 1.0.5
// //buf+=4;
// // memcpy(fwd_address,&buf[0],8);
//  //buf+=8;
//  //mem_rev_cpy((uint8_t *)&fwd_port,&buf[0],2);
//  //buf+=2;
//  if(command == 1){
//    sendSubscribedPacket = 1;
//  }
//  else{
//    sendSubscribedPacket = 0;
//  }
//  status = 0;//subscribe_packets(command,fwd_address,fwd_port);
//  send_hif_conf_cb(SUBSCRIBE_PACKETS_CONF,status);
//}
/*----------------------------------------------------------------------------*/
void process_get_dodag_routers(uint8_t *buf, uint16_t length)
{
    get_dodag_routers();
}
/*----------------------------------------------------------------------------*/
void process_get_neighbor_table(uint8_t *buf, uint16_t length)
{
    get_neighbor_table();
}

void process_get_current_join_state(uint8_t *buf, uint16_t length)
{
    uint8_t join_state  = get_current_join_state();
    send_hif_conf_cb(SEND_CURRENT_JOIN_STATE,join_state);
}
/*----------------------------------------------------------------------------*/

//void process_set_pa_level(uint8_t *buf, uint16_t length)
//{
//	uint8_t level;
//        // Since now we are sending PA level as Enum, and Enum is 4 Byte value 
//        // The Value of the PA level is in the 4th location
//      level = *buf;
//      TRX_Set_PA_Level(level);
//      send_hif_conf_cb(TELEC_SET_PA_LEVEL_MCR_CONF,0);	
//}

/*----------------------------------------------------------------------------*/

//void process_set_router_config_tbc(uint8_t *buf, uint16_t length)
//{
//  uint8_t status = 0xFF;
//  uint8_t routing_method = 0;
//  uint16_t pan_size = 0x0000;
//  uint8_t net_name[30] = {0};
//  uint8_t index = 0;
//  uint16_t max_sisl_mtusize = 0x00;
//  routing_method = *buf++;
//  length--;
//  while(*buf != 0x00)
//  {
//    net_name[index++] = *buf++;
//    length--;
//  }
//  *buf++;
//  length--;
//  buf+=2; //routing cost offset 
//  length -= 2;
//  buf+=2; //pancost cost offset 
//  length -= 2;
//  if(length != 0)
//  {
//    mem_rev_cpy((uint8_t *)&max_sisl_mtusize,&buf[0],2);
//    max_frag_size = frag_chunk_send = max_sisl_mtusize;
//    buf+=2;
//  }
//  status = set_router_config(net_name, routing_method, pan_size);
//  send_hif_conf_cb(SET_ROUTER_CONFIG_CONF,status);
//}
/*----------------------------------------------------------------------------*/
//void process_set_router_config(uint8_t *buf, uint16_t length)
//{
//    uint8_t status = 0xFF;
//    uint8_t routing_method = 0;
//    uint16_t pan_size = 0x0000;
//    uint8_t net_name[30] = {0};    
//    uint16_t net_name_len = 0x00;
//    mem_rev_cpy((uint8_t *)&net_name_len ,&buf[0] ,2);
//    buf+=2;
//    memcpy((uint8_t *)&net_name ,&buf[0] ,net_name_len);
//    buf+=net_name_len;
//    mem_rev_cpy((uint8_t *)&pan_size ,&buf[0] ,2);
//    buf+=2;
//    routing_method = *buf++;
//    status = set_router_config(net_name, routing_method, pan_size);
//    send_hif_conf_cb(SET_ROUTER_CONFIG_CONF,status);
//}

/*----------------------------------------------------------------------------*/
//void process_set_mac_whitelist_ontbc(uint8_t *buf, uint16_t length)
//{
//  uint8_t status = 0xFF;
//  status = set_mac_white_list_tbc(buf,length);
//  send_hif_conf_cb(SET_MAC_WHITELIST_CONF,status);
//}
/*----------------------------------------------------------------------------*/
//void process_set_mac_whitelist(uint8_t *buf, uint16_t length)
//{
//  uint8_t status = 0xFF;
//  status = set_mac_white_list(buf,length);
//  send_hif_conf_cb(SET_MAC_WHITELIST_CONF,status);
//}
/*----------------------------------------------------------------------------*/
//void process_get_mac_whitelist(uint8_t *buf, uint16_t length)
//{
//  get_mac_white_list();
//}
/*----------------------------------------------------------------------------*/
//void process_set_revoaction_key(uint8_t *buf, uint16_t length)
//{
//  send_hif_conf_cb(SET_REVOCATION_KEY_CONF,0);
//  set_revoaction_key(buf, length);
//}
/*----------------------------------------------------------------------------*/
void process_get_prefered_parents(uint8_t *buf, uint16_t length)
{
  get_prefered_parents(buf,length);
}

/*----------------------------------------------------------------------------*/
#if(FAN_EDFE_FEATURE_ENABLED == 1)
void process_edfe_frame_exchange_req(uint8_t *buf, uint16_t length)
{
  send_edfe_exchange_frame(buf,length);
  send_hif_conf_cb(EDFE_FRAME_EXCHANGE_CONF,0);
}
#endif

/*----------------------------------------------------------------------------*/

void process_start_network_scale_req(uint8_t *buf, uint16_t length)
{
//  uint8_t network_scal_type = *buf++;
//  if(network_scal_type == 0x01)
//  {
//    rpl_dio_interval_MIN = 19;
//    rpl_cfg_dio_interval_DUB = 1;
//    trickle_IMIN = 60;
//    trickle_IMAX = 4;
//  }
//  else if(network_scal_type == 0x00)
//  {
//    rpl_dio_interval_MIN = 15;
//    rpl_cfg_dio_interval_DUB = 2;
//    trickle_IMIN = 15;
//    trickle_IMAX = 2;
//  }
//  send_hif_conf_cb(SET_START_NETWORK_SCALE_CONF,0);
}

/*----------------------------------------------------------------------------*/

//void enable_desmac_sec(uint8_t *buf, uint16_t length)
//{
//  set_mac_security_enable_disable(*(buf+3));
//}


/*----------------------------------------------------------------------------*/

//void send_eapol_packt_to_lbr(uint8_t *buff , uint16_t len , uint8_t *self_addr)
//{
//#if 0
//    enet_msg_t* msg = allocate_hif_msg( len+25 );
//    uint8_t* buf = msg->data;
//     if(msg != NULL)
//     {
//       *buf++ = SEND_EAPOL_PACKT_TRANSMIT;
//       memcpy(buf,self_addr,8);
//       buf+=8;
//       memcpy(buf,buff,len);
//       buf += len; 
//       *buf++ = DUMMY_COMPORT;//Dummy Comport 
//       msg->data_length  = buf - msg->data;
//       hif_send_msg_up(msg->data,(msg->data_length-1),response_laye_ID,PROTOCOL_ID_FOR_APP);      
////#if (PRINT_DEBUG_LEVEL == CONSOLE_ERROR_DEBUG)
////       stack_print_debug ("E->H\n");
////#endif
//          free_hif_msg( msg );
//     }
//#else
//     enet_msg_t* msg = allocate_hif_msg( len+25 );
//     uint8_t* buf = msg->data;
//     if(msg != NULL)
//     {
//       memcpy(buf,self_addr,8);
//       buf+=8;
//       memcpy(buf,buff,len);
//       buf += len; 
//       msg->data_length  = buf - msg->data;
//       recv_data_from_app(NULL,msg->data,msg->data_length);
//       free_hif_msg( msg );
//     }
//#endif    
//    
//}
/*----------------------------------------------------------------------------*/

//void send_wpa_supplicant_for_bootup()
//{
//    uint8_t hif_Send_buff [5] = {0};
//    uint8_t* buf = &hif_Send_buff [0];
//
//    *buf++ = BOOT_WPA_SUPPLICANT;
//    *buf++ = DUMMY_COMPORT;//Dummy Comport 
//
//    hif_send_msg_up(&hif_Send_buff[0],1,6);
//     
//}



/*----------------------------------------------------------------------------*/

//void send_host_apd_bootup()
//{
//    uint8_t hifBuff[15] = {0};
//    uint8_t* buf = &hifBuff[0];
//
//    *buf++ = SEND_START_CMD_TO_HOST_APD;
//    *buf++ = DUMMY_COMPORT;//Dummy Comport 
//     hif_send_msg_up(&hifBuff[0], 1,response_laye_ID,PROTOCOL_ID_FOR_APP);  
//}

/* Debdeep :: Now we send ack for every eapol packet received from Linux Hostapd */
//void send_ack_to_hostapd_for_eapol_packet()
//{
//    uint8_t hifBuff[15] = {0};
//    uint8_t* buf = &hifBuff[0];
//
//    *buf++ = SEND_EAPOL_PACKET_ACK;
//    *buf++ = DUMMY_COMPORT;//Dummy Comport 
//     hif_send_msg_up(&hifBuff[0], 1,response_laye_ID,PROTOCOL_ID_FOR_APP);  
//}
/*----------------------------------------------------------------------------*/
//void set_mac_address(uint8_t *buf,uint16_t len);
//void set_gtk_key_auth(uint8_t *buf,uint16_t len);
//void send_mac_addr(uint8_t *buff , uint16_t len)
//{
//#if 0  
//  uint8_t hifBuff[15] = {0};
//  uint8_t* buf = &hifBuff[0];
//  
//  *buf++ = SEND_SELF_MAC_ADDR;
//   mem_rev_cpy(buf,buff,len);
//   buf+=8;
//   *buf++ = DUMMY_COMPORT;//Dummy Comport 
//   //len + 1  --> 1 is for Dummy COM Port
//   hif_send_msg_up(&hifBuff[0], (len +1 ),response_laye_ID,PROTOCOL_ID_FOR_APP);
//#else
//   uint8_t temp_gtk_hash_buf[65] = {0};
//   uint8_t temp_addr[8] = {0};
//   mem_rev_cpy(temp_addr,buff,8);
//   uint8_t *temp_ptr = &temp_gtk_hash_buf[0];
//   set_mac_address(&temp_addr[0],len);
//   *temp_ptr++ = fan_nwk_manager_app.node_basic_cfg.gtkhash_ie.gtkl;
//   memcpy(temp_ptr,(uint8_t *)fan_nwk_manager_app.node_basic_cfg.gtkhash_ie.gtk0_key,16);
//   temp_ptr += 16;
//   memcpy(temp_ptr,(uint8_t *)fan_nwk_manager_app.node_basic_cfg.gtkhash_ie.gtk1_key,16);
//   temp_ptr += 16;
//   memcpy(temp_ptr,(uint8_t *)fan_nwk_manager_app.node_basic_cfg.gtkhash_ie.gtk2_key,16);
//   temp_ptr += 16;
//   memcpy(temp_ptr,(uint8_t *)fan_nwk_manager_app.node_basic_cfg.gtkhash_ie.gtk3_key,16);
//   set_gtk_key_auth(&temp_gtk_hash_buf[0],len);
//   start_host_apd();
//#endif   
//
//}

void set_seq_key(uint8_t live_key_id_index)
{
  uint8_t tempBuff[8] = {0};
  generate_MAC_Security_Key(live_key_id_index,0);
  key_id_index = live_key_id_index;
  set_mac_security_on_LBR(&tempBuff[0],8);

#if(FAN_FRQ_HOPPING_FEATURE_ENABLED == 1)  
  /* Debdeep :: For LBR we need to start scheduling for freq hopping 
  after doing handshake between Linux-Hostapd and Embedded-LBR */
  fan_freq_hop_start_hopping(NULL);
#endif
  
}

void recv_data_from_eapol_auth(uint8_t *pBuff,uint16_t len)
{
  TANSIT_KMP_ID = *pBuff++;        
  len -= 1; // Raka : 03-Dec-2017 Since KMP ID is extracted 
  
  if((pBuff[0] == 0x00)&&(pBuff[1] == 0x00)&&(pBuff[2] == 0x00)&&
     (pBuff[3] == 0x00)&&(pBuff[4] == 0x00)&&(pBuff[5] == 0x00)&&
       (pBuff[6] == 0x00)&&(pBuff[7] == 0x00))
  {
    /* Send EAPOL Key Packet Over Radio RF when received from the Linux Suplicant
    Recevied Destination Addess will all ZERO
    */
    #if(FAN_EAPOL_FEATURE_ENABLED == 1)
    send_eapol_data_to_mac_request(pBuff,len);
#endif
  }
  else
  {
    /*send eapol packet to make udp packet and send as udp*/
    if((relay_reply_flag == 0x01  || (!memcmp(authnt_interfac_id,pBuff,8)
                                      || is_send_as_udp(pBuff))
        ))
    {
      //#if (PRINT_DEBUG_LEVEL == CONSOLE_ERROR_DEBUG)
      //        stack_print_debug ("H->E-R\n");
      //#endif
#if(FAN_EAPOL_FEATURE_ENABLED == 1)
      send_data_to_eapol_relay_process(pBuff,len);
#endif
      relay_reply_flag = 0xFF;
    }
    else
    {
      //#if (PRINT_DEBUG_LEVEL == CONSOLE_ERROR_DEBUG)
      //        stack_print_debug ("H->E-M\n");
      //#endif
      /*send packet on mac layer */
#if(FAN_EAPOL_FEATURE_ENABLED == 1)
      send_eapol_data_to_mac_request(pBuff,len);
#endif
    }
  }    
}

void send_gtk_update_indication(uint8_t *pBuff,uint16_t len)

{
  uint8_t old_gtkl = fan_nwk_manager_app.node_basic_cfg.gtkhash_ie.gtkl;
  uint8_t new_gtkl = *pBuff++;
  uint8_t number_of_gtk_made_zero = *pBuff++;
  
  for (uint8_t ii = 0; ii < 4; ii++)
  {
    if (old_gtkl & (0x01<<ii)) /* Previously installed */
    {
      if(!(new_gtkl & (0x01<<ii)))   /* Currently Not used */
      {
        reset_incoming_frame_counter_for_stale_key (ii);
      }
    }
  }
  
  fan_nwk_manager_app.node_basic_cfg.gtkhash_ie.gtkl = new_gtkl;
  fan_nwk_manager_app.node_basic_cfg.panvar_ie.PANVERSION += number_of_gtk_made_zero;
  memcpy (fan_nwk_manager_app.node_basic_cfg.gtkhash_ie.gtk0_key, pBuff, 16);
  pBuff += 16;
  memcpy (fan_nwk_manager_app.node_basic_cfg.gtkhash_ie.gtk1_key, pBuff, 16);
  pBuff += 16;
  memcpy (fan_nwk_manager_app.node_basic_cfg.gtkhash_ie.gtk2_key, pBuff, 16);
  pBuff += 16;
  memcpy (fan_nwk_manager_app.node_basic_cfg.gtkhash_ie.gtk3_key, pBuff, 16);
  
  trickle_timer_consistency_pc ();
  
  FAN_MAC_MLME_SET_Request
    (
     WISUN_INFO_PAYLOAD_IE_ID,/* header ie or payload ie */
     WISUN_IE_SUBID_GTKHASH_IE,/* subid for each ie */	        
     sizeof(gtk_key_t),/*(65+1)=(fan_nwk_manager_app.node_basic_cfg.gtkhash_ie.length+1),*/
     (uint8_t *)& fan_nwk_manager_app.node_basic_cfg.gtkhash_ie
       );
}

void send_mac_security_set_request(uint8_t *pBuff,uint16_t len)
{
  key_id_index = 0x00;//*pBuff++;                    /* Active Key index */
  generate_MAC_Security_Key(key_id_index,0);
  reset_mac_frame_counter_cmd ();
  add_security_key_descriptor_on_MAC ();
  trickle_timer_consistency_pc ();
  FAN_MAC_MLME_SET_Request
    (
     WISUN_INFO_PAYLOAD_IE_ID,/* header ie or payload ie */
     WISUN_IE_SUBID_GTKHASH_IE,/* subid for each ie */	        
     sizeof(gtk_key_t),/*(65+1)=(fan_nwk_manager_app.node_basic_cfg.gtkhash_ie.length+1),*/
     (uint8_t *)& fan_nwk_manager_app.node_basic_cfg.gtkhash_ie
       );
}
/*----------------------------------------------------------------------------*/

void send_gtkhash_to_hostapd(uint8_t *buff , uint16_t len)
{

#if(AUTO_CONFIG_ENABLE == 0)
  // Len is always 64 Byte
  
  uint8_t hif_Send_buff [70] = {0};
  uint8_t* buf = &hif_Send_buff [0];
  
  *buf++ = SEND_GTK_HASH;
  memcpy(buf,buff,len);
  buf+=len;
  *buf++ = DUMMY_COMPORT;//Dummy Comport 
  hif_send_msg_up(&hif_Send_buff[0], (len +1 ),response_laye_ID,PROTOCOL_ID_FOR_APP);
#endif
}


/****************************** HIF Message Communication **********************/

/*---------------------------------------------------------------------------*/
uint8_t send_hif_seckey_cb( uint8_t cmd_id,uint8_t* seckey )
{
  uint8_t hif_Send_buff [70] = {0};
  uint8_t* buf = &hif_Send_buff [0];
  
  *buf++ = cmd_id;
  
  memcpy(buf,&seckey[0],64);
  buf+=64;
  *buf++ = DUMMY_COMPORT;//Dummy Comport 		            	     
  hif_send_msg_up(&hif_Send_buff[0], 65,response_laye_ID,PROTOCOL_ID_FOR_APP);   
      return 0;
}

/*----------------------------------------------------------------------------*/

uint8_t send_SHA256_Update_data_to_hif(uint8_t *buf, uint16_t length)
{
    uint8_t buffData [200] = {0};
    uint16_t  iCnt = 0;
    buffData[0] = 0xF2;
#if 0
    for ( iCnt = 0; (iCnt <= length); iCnt+=4)
    {
        uint32_t tempVal = (uint32_t) *((uint32_t *)(buf+iCnt));
        mem_rev_cpy(&buffData[1+iCnt],(uint8_t *)&tempVal,4);
    }
    
#else
    memcpy(&buffData[1+iCnt],buf,length);
#endif
    
    //hif_send_msg_up(&buffData[0],1+iCnt,1);
    //hif_send_msg_up(&buffData[0],length,1);

    return 0;
}

/*----------------------------------------------------------------------------*/
uint8_t send_hif_conf_cb (uint8_t cmd_id,uint8_t status )
{
  uint8_t hif_Send_buff [ 5] = {0};
  uint8_t* buf = &hif_Send_buff [0];
  
  *buf++ = cmd_id;
  *buf++ = status;
  *buf++ = DUMMY_COMPORT;//Dummy Comport 
  
  //hif_send_msg_up (&hif_Send_buff[0], 2, response_laye_ID, PROTOCOL_ID_FOR_APP);
  return 0;
}
/*----------------------------------------------------------------------------*/


/*
** =============================================================================
** Private Function Definitions
** =============================================================================
*/

static void process_send_udp(uint8_t *buf, uint16_t length)
{
  uint8_t status = 1;
   status = send_udp(buf,length);
  send_hif_conf_cb(SEND_UDP_ECHO_RESP,status); 
}


static void send_version_info (void)
{
  uint8_t hifBuff[15];
  hifBuff[0] = GET_VERSION_INFO_RESP;
  memcpy (&hifBuff[1], VERSION_NUMBER, strlen(VERSION_NUMBER));
  //hif_send_msg_up(hifBuff, strlen(VERSION_NUMBER)+1,response_laye_ID,PROTOCOL_ID_FOR_APP);
}

/*----------------------------------------------------------------------------*/
uint8_t hif_2_App_Interface_cb( uint8_t* pBuff,uint16_t len)
{
  response_laye_ID = APP_DEF_LAYER_ID_TOOL;
  uint8_t status = 0xFF;
  node_basic_config_t* p_basic_cfg = NULL;
  uint8_t cmd_id = *pBuff; 
  pBuff+=4; 
  
  
  switch ( cmd_id )
  {
    
    /******************************************************************************
    FAN Stack Validation Tool Command ID
    *******************************************************************************/   
  case SWITCH_OPERATIONAL_MODE:
    p_basic_cfg = &(fan_nwk_manager_app.node_basic_cfg);
    p_basic_cfg->operational_mode = (pBuff[0]);
    //    store_nvm_param.operational_mode = p_basic_cfg->operational_mode;
#if(APP_NVM_FEATURE_ENABLED == 1)
    nvm_store_node_basic_info();
#endif
    send_hif_conf_cb(SWITCH_OPERATIONAL_MODE_CONF,0);
    NVIC_SystemReset();
    break;
    
  case GET_OPERATIONAL_MODE:
    p_basic_cfg = &(fan_nwk_manager_app.node_basic_cfg);
    send_hif_conf_cb(SEND_OPERATINAL_MODE,p_basic_cfg->operational_mode );
    break;
    
  case SET_BASIC_CONFIG:  // FACtory Mode and RUN mode 
    {
      // Form Tool Config Page 
      status = process_telec_set_operating_country(pBuff[0]);     
      send_hif_conf_cb(SET_BASIC_CONFIG_CONF,status);
    }
    break;
    
    
  case SYSTEM_RESET:
    NVIC_SystemReset();
    break;
    
    
//  case APP_2_FAN_UDP_REQ:
//    if(fan_nwk_manager_app.node_basic_cfg.operational_mode == RUN_MODE)
//      trigger_udp_request(pBuff);
//    else
//      send_cmd_not_support();
//    break;            
    
    
    /******************************************************************************
    SwaggerHub Command ID
    *******************************************************************************/   
    
  case NODE_START_STOP: // In auto mode we are starting the Node ...
    if(fan_nwk_manager_app.node_basic_cfg.operational_mode == FACTORY_MODE)
    {
      if(pBuff[0] == FAN_STOP_NODE)
      {
#if(APP_NVM_FEATURE_ENABLED == 1)
        if(fan_nwk_manager_app.nvm_write_to_start == true)
        {
          change_join_state_for_nvm();
          fan_nwk_manager_app.nvm_write_to_start = false;
        }
#endif
      }
      process_set_node_start_stop(pBuff, len); 
    }
    else
      send_cmd_not_support();
    break;    
    
  case SET_FACTORY_MODE_PA_LEVEL_API:
    process_set_facort_mode_PA_level_api(pBuff, len);
    break;
    
  case SET_MAC_CHAN_PLAN_FIXED:    
    process_set_mac_fixed_chan(pBuff, len);
    break;
    
  case RESET_RPL_MSG_RATE:
    
    if(fan_nwk_manager_app.node_basic_cfg.operational_mode == RUN_MODE)
      process_reset_rpl_msg_rate(pBuff, len);
    else
      send_cmd_not_support();
    break;
    
  case GET_SEC_KEYS:
    if(fan_nwk_manager_app.node_basic_cfg.operational_mode == RUN_MODE)
      process_get_sec_keys(pBuff, len);
    else
      send_cmd_not_support();
    break;
    
  case GET_IP_ADDRESSES:
    if(fan_nwk_manager_app.node_basic_cfg.operational_mode == RUN_MODE)
      process_get_ip_address(pBuff, len);
    else
      send_cmd_not_support();
    break;
    
  case SEND_UDP: // Sending UDP 
    if(fan_nwk_manager_app.node_basic_cfg.operational_mode == RUN_MODE)
      process_send_udp(pBuff, len);
    else
      send_cmd_not_support();
    break;
    
  case SEND_ICMPv6: // Sending IPv6 Ping
    if(fan_nwk_manager_app.node_basic_cfg.operational_mode == RUN_MODE)
      send_icmpv6_request(pBuff, len);
    else
      send_cmd_not_support();
    break;
    
  case API_GET_DODAG_ROUTES:
    if(fan_nwk_manager_app.node_basic_cfg.operational_mode == RUN_MODE)
      process_get_dodag_routers(pBuff, len);
    else
      send_cmd_not_support();
    break;
    
  case API_GET_NEIGHBOR_TABLE:
    if(fan_nwk_manager_app.node_basic_cfg.operational_mode == RUN_MODE)
      process_get_neighbor_table(pBuff, len);
    else
      send_cmd_not_support();
    break;
    
  case API_GET_JOIN_STATE:
    if(fan_nwk_manager_app.node_basic_cfg.operational_mode == RUN_MODE)
      process_get_current_join_state(pBuff, len);
    else
      send_cmd_not_support();
    break;
    
  case GET_DEVICE_PRIMERY_PARENTS_REQ:
    if(fan_nwk_manager_app.node_basic_cfg.operational_mode == RUN_MODE)
      process_get_prefered_parents(pBuff,len);
    else
      send_cmd_not_support();
    break;
    
  case SET_TX_PKT_CONFIG:
    if(fan_nwk_manager_app.node_basic_cfg.operational_mode == FACTORY_MODE)
      process_set_pkt_tx(pBuff, len);
    else
      send_cmd_not_support();
    break;
    
  case START_PACKET_TX_REQ:
    if(fan_nwk_manager_app.node_basic_cfg.operational_mode == FACTORY_MODE)
      process_start_tx(pBuff, len);
    else
      send_cmd_not_support();
    break;
    
  case STOP_PACKET_TX_REQ:
    if(fan_nwk_manager_app.node_basic_cfg.operational_mode == FACTORY_MODE)
      process_stop_tx(pBuff, len);
    else
      send_cmd_not_support();
    break;
    
  case START_PACKET_RX_REQ:
    if(fan_nwk_manager_app.node_basic_cfg.operational_mode == FACTORY_MODE)
      process_start_rx(pBuff, len);
    else
      send_cmd_not_support();
    break;
    
  case STOP_PACKET_RX_REQ:
    if(fan_nwk_manager_app.node_basic_cfg.operational_mode == FACTORY_MODE)
      process_stop_rx(pBuff, len);
    else
      send_cmd_not_support();
    break;
    
  case START_START_CONTINUOUS_TX_REQ:
    if(fan_nwk_manager_app.node_basic_cfg.operational_mode == FACTORY_MODE)
      process_start_continuous_tx(pBuff, len);
    else
      send_cmd_not_support();
    break; 
    
  case STOP_CONTINUOUS_TX_REQ:
    if(fan_nwk_manager_app.node_basic_cfg.operational_mode == FACTORY_MODE)
      process_stop_continuous_tx(pBuff, len);
    else
      send_cmd_not_support();
    break;
    
  case START_CONTINUOUS_RX_REQ:
    if(fan_nwk_manager_app.node_basic_cfg.operational_mode == FACTORY_MODE)
      process_start_continuous_rx(pBuff, len);
    else
      send_cmd_not_support();
    break;
    
  case STOP_CONTINUOUS_RX_REQ:
    if(fan_nwk_manager_app.node_basic_cfg.operational_mode == FACTORY_MODE)
      process_stop_continuous_rx(pBuff, len);
    else
      send_cmd_not_support();
    break;
    
  case GET_RX_COUNT_DETAILS:
    if(fan_nwk_manager_app.node_basic_cfg.operational_mode == FACTORY_MODE)
      process_get_rx_details(pBuff, len);
    else
      send_cmd_not_support();
    break;
    
  case PHY_ENC_TEST_TX:
    if(fan_nwk_manager_app.node_basic_cfg.operational_mode == FACTORY_MODE)
      process_test_phy_enc(pBuff, len);
    else
      send_cmd_not_support();
    break;
    
  case CMD_FACTROY_MODE_CHANNEL_SCAN_REQ:
    if(fan_nwk_manager_app.node_basic_cfg.operational_mode == FACTORY_MODE)
      process_factory_mode_ch_scanning_req(pBuff, len);
    else
      send_cmd_not_support();
    break;
    
  case SET_RSSI_THRESHOLD:
    process_set_rssi_threshold (pBuff, len);
    break;
    
  case GET_VERSION_INFO_REQ:
    send_version_info ();       /*VERSION_NUMBER*/
    break;
    
  case GET_CONFIG_INFO_REQ:
    process_get_config_info_req ();
    break;
    
  case GET_ALL_PARAM_INFO_REQ:
    send_all_param_info_req ();
    break;
    
  case SOFTWARE_RESET:
    node_start_stop (FAN_STOP_NODE);
    break;
    
  default:    
    send_cmd_not_support();
    break;
  }
  return 0;
  //return HIF_SUCCESS;
}

