/** \file mac_2_6lowpan_data_handler.c
*******************************************************************************
** \brief Implements the OS dependant part of the timer service
**
** \cond STD_FILE_HEADER
**
** COPYRIGHT(c) 2010-11 Procubed Technology Solutions Pvt Ltd. 
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

/*******************************************************************************
* File inclusion
*******************************************************************************/

#include "l3_configuration.h"
#include "l2_l3_interface.h"
#include "l3_process_interface.h"
#include "list_latest.h"
#include "queue_latest.h"
#include "linkaddr.h"
#include "buff_mgmt.h"
#include "mac_app_build_config.h"
#include "mac.h"
#include "sw_timer.h"
#include "timer_service.h"
#include "fan_mac_ie.h"
#include "mac_interface_layer.h"
#include "sm.h"
#include "../../../ProAppSrc/Include/mac.h" 
#include "mac_queue_manager.h"
#include "tcpip.h"
#include "sicslowpan.h"
#include "fan_mac_ie.h"
#include "mac_defs_sec.h"
#include "ie_element_info.h"
#include "mac_pib.h" 
#include "fan_mac_interface.h"
#include "mac_defs.h"
#include "packetbuf.h"
#include "rpl.h"
#include "fan_mac_interface.h"
#include "common_function.h"
/*
** ============================================================================
** Private Macro definitions
** ============================================================================
*/

/* None */

/*
** ============================================================================
** Private Structures, Unions & enums Type Definitions
** ============================================================================
*/

typedef struct mac_data_conf_cb_tag
{
  mac_callback_t sent;
  void *ptr;
}mac_data_conf_cb_t;

/*
** ============================================================================
** Private Variable Definitions
** ============================================================================
*/

static uint8_t dest_reversed[UIP_LLADDR_LEN];
static uint8_t src_reversed[UIP_LLADDR_LEN];
static uint8_t msduHandle = 0;
static struct l3_ctimer retransmit_ns_with_aro_timer;
void set_datapkt_type();
/******************************************************************************/

/*
** ============================================================================
** Public Variable Definitions
** ============================================================================
*/
/*Umesh : 24-01-2018*/
uint8_t gl_nbr_rssi_idx=0x00;
/*this varriable no ware used*/
uint8_t force_ie_list_in_frame_control = 0;

/*
** ============================================================================
** External Variable Declarations
** ============================================================================
*/

/*Umesh : 23-01-2018*/
extern uint8_t no_routing_required; // Raka...
/*this varriable no ware used ,not even got defination*/

extern mac_pib_t mac_pib;
extern self_info_fan_mac_t mac_self_fan_info;
extern fan_mac_param_t fan_mac_params;
extern uint8_t key_id_index;
#if(FAN_EDFE_FEATURE_ENABLED == 1)
extern edfe_info_t edfe_information;
#endif


/*
** ============================================================================
** External Function Declarations
** ============================================================================
*/

extern  void mem_reverse_cpy(uint8_t* dest, uint8_t* src, uint16_t len );
struct l3_ctimer dao_send_timer;
void uip_ds6_nbr_update_rsl_ie(uint8_t* rcvd_addr, uint8_t* self_addr, int32_t rssi);

/*
** ============================================================================
** Private Function Prototypes
** ============================================================================
*/
static void input_packet(void);
static int on(void);
static int off(int keep_radio_on);
static void init(void);
static unsigned short channel_check_interval(void);
static void byte_reverse(uint8_t * bytes, uint8_t num);
static void start_timer_for_sending_ns_with_aro_again (void);

/*
** ============================================================================
** Public Function Definitions
** ============================================================================
*/

void mac_2_6lp_data_conf_cb
(
uint8_t msduHandle, 
uint8_t status, 
uint8_t NumBackoffs,
uint32_t Timestamp
)
{
}
/******************************************************************************/
void mac_2_6lp_ack_ind_cb (mac_address_t*  pSrcaddr, mac_address_t*  pDstaddr, uint8_t DSN, uint8_t rsl_value, uint8_t security_status)
{
  uint8_t is_this_ns_witharo = 0xFF; //Suneet :: after dhcp complete check this normal ns_with aro not probing ns
  uint8_t send_ns_pkt_seq_number = 0;
  uip_ds6_nbr_update_rsl_ie(pSrcaddr->address.ieee_address, \
    pSrcaddr->address.ieee_address,rsl_value);//rcvd address, self address, rssi (RSL-IE)
  
  is_this_ns_witharo = packetbuf_attr(SEND_NS_WITH_ARO_AFTER_COMPLETE_DHCP);
  if(is_this_ns_witharo == 0x01)
  {
    send_ns_pkt_seq_number = packetbuf_attr(PACKETBUF_ATTR_MAC_SEQNO);
    if(DSN == send_ns_pkt_seq_number)
    {
      if (security_status == 0)
      {
#if (PRINT_DEBUG_LEVEL==ERROR_DEBUG)
        stack_print_debug ("Security Success::Rcvd ACK for NS with ARO from ");
        print_mac_address (pSrcaddr->address.ieee_address);
#endif        
        uip_ipaddr_t *target = uip_ds6_defrt_choose();//choosing from defaultrouterlist
        uip_lladdr_t ll_addr;
        memcpy(ll_addr.addr, &target->u8[8], 8); 
        ll_addr.addr[0] ^= 0x02;
        
#if (PRINT_DEBUG_LEVEL==ERROR_DEBUG)
        stack_print_debug ("Triggering DAO to ");
        print_mac_address (&target->u8[8]);
#endif 
        
#if (PRINT_DEBUG_LEVEL==RUNTIME_LOG_FILE)
    send_runtime_log ("Sending DAO to "LOG_MAC_ADDRESS(ll_addr.addr));
#endif         
        
        rpl_parent_t *p_parent = rpl_get_parent(&ll_addr);
        l3_ctimer_set(&dao_send_timer, 2*CLOCK_SECOND,
                   handle_dao_callback, p_parent); 
        is_this_ns_witharo = 0;
        packetbuf_set_attr(SEND_NS_WITH_ARO_AFTER_COMPLETE_DHCP,is_this_ns_witharo);
        packetbuf_set_attr(PACKETBUF_ATTR_MAC_SEQNO,is_this_ns_witharo);
      }
      else
      {
#if (PRINT_DEBUG_LEVEL==ERROR_DEBUG)
        stack_print_debug ("Security FAIL::Rcvd ACK for NS with ARO from ");
        print_mac_address (pSrcaddr->address.ieee_address);
#endif        
//        tcpip_post_event( NBR_SEND_NS_ARO_EVENT, NULL );
        start_timer_for_sending_ns_with_aro_again ();
      }
    }
    else
    {
#if (PRINT_DEBUG_LEVEL==ERROR_DEBUG)
      stack_print_debug ("**** Rcvd ACK for NS with ARO with different SN: Rcvd = %d, Stored = %d form ", DSN, send_ns_pkt_seq_number);
      print_mac_address (pSrcaddr->address.ieee_address);
#endif        
//      tcpip_post_event( NBR_SEND_NS_ARO_EVENT, NULL );
      start_timer_for_sending_ns_with_aro_again ();
    }
  }
}

void mac_2_6lp_no_ack_ind_cb (void)
{
  uint8_t is_this_ns_witharo = packetbuf_attr(SEND_NS_WITH_ARO_AFTER_COMPLETE_DHCP);
  
#if (PRINT_DEBUG_LEVEL==ERROR_DEBUG)
  stack_print_debug ("**** No ACK rcvd for NS with ARO\r\n");
#endif   
  
  if (is_this_ns_witharo == 0x01)
  {
    start_timer_for_sending_ns_with_aro_again ();
  }
}

static void retransmit_ns_with_aro_timer_cb (void *ptr)
{
//  uip_ds6_nbr_t *primary_parent = (uip_ds6_nbr_t *)uip_get_prio_based_parent_nbr (1);
//  struct link_stats *nbr_stats = (struct link_stats *)link_stats_from_lladdr ((linkaddr_t*)primary_parent->lladdr.addr);
//  if ((nbr_stats != NULL) && (nbr_stats->etx < 512))
//  {
//    tcpip_post_event( NBR_SEND_NS_ARO_EVENT, NULL );
//  }
//  else
//  {
//    packetbuf_set_attr(SEND_NS_WITH_ARO_AFTER_COMPLETE_DHCP, 0);
//    packetbuf_set_attr(PACKETBUF_ATTR_MAC_SEQNO, 0);
//  }   //suneet :: open when testing
}

static void start_timer_for_sending_ns_with_aro_again (void)
{
  l3_ctimer_set (&retransmit_ns_with_aro_timer, 20*CLOCK_SECOND, retransmit_ns_with_aro_timer_cb, NULL);
}

void mac_2_6lp_data_ind_cb
(
mac_address_t*  pSrcaddr,
mac_address_t*  pDstaddr,
uint16_t msduLength,
uint8_t* pMsdu,
uint8_t mpduLinkQuality,
uint8_t DSN,
uint8_t pld_ies_present,
uint32_t Timestamp,
security_params_t* pSec 
)
{
  /* Finally, get the stuff into the rime buffer.... */
  packetbuf_copyfrom(pMsdu, msduLength);
  packetbuf_set_datalen(msduLength);
  memcpy(src_reversed, (uint8_t *)pSrcaddr->address.ieee_address, UIP_LLADDR_LEN);
  byte_reverse((uint8_t *)src_reversed, UIP_LLADDR_LEN);
  /*copy the addresses*/
  memcpy(dest_reversed, (uint8_t *)pDstaddr->address.ieee_address, UIP_LLADDR_LEN);
  byte_reverse((uint8_t *)dest_reversed, UIP_LLADDR_LEN);
  /*set the packet attributes to be used by UIP */
  packetbuf_set_addr(PACKETBUF_ADDR_RECEIVER, (const linkaddr_t *)dest_reversed);
  packetbuf_set_addr(PACKETBUF_ADDR_SENDER, (const linkaddr_t *)src_reversed);
  
  packetbuf_set_attr(PACKETBUF_ATTR_PACKET_ID, DSN);
  input_packet();	
}

/******************************************************************************/

/*
** ============================================================================
** Private Function Definitions
** ============================================================================
*/
static void input_packet(void)
{
  NETSTACK_NETWORK.input();
}
/*---------------------------------------------------------------------------*/
static int on(void)
{
  
  return 0;
}
/*---------------------------------------------------------------------------*/
static int off(int keep_radio_on)
{
  
  return 0;
}
/*---------------------------------------------------------------------------*/
static void init(void)
{
  
}
/*---------------------------------------------------------------------------*/
static unsigned short channel_check_interval(void)
{
  return 0;
}
/*---------------------------------------------------------------------------*/
static void byte_reverse(uint8_t * bytes, uint8_t num)
{
  uint8_t tempbyte;
  
  uint8_t i, j;
  
  i = 0;
  j = num - 1;
  
  while(i < j) {
    tempbyte = bytes[i];
    bytes[i] = bytes[j];
    bytes[j] = tempbyte;
    
    j--;
    i++; 
  }
  
  return;
}
/*---------------------------------------------------------------------------*/

static void send_packet(mac_callback_t sent, void *ptr)
{
  uint8_t tx_options = 0;
  uint32_t TxChannel = 0;
  uint8_t i =0;
  uint16_t len = 0;
  uint8_t rev_addr[8];
  uint32_t sub_hdr_bitmap = 0;
  uint32_t sub_pld_bitmap = 0;
  mac_address_t SrcAddr;
  mac_address_t DstAddr;
  linkaddr_t* p_rx_addr = (linkaddr_t*) packetbuf_addr(PACKETBUF_ADDR_RECEIVER);
  uint8_t node_type = get_node_type( );
  
  msduHandle = packetbuf_attr(PACKETBUF_ATTR_PACKET_TYPE);
#if !(ENABLE_FAN_MAC_WITHOUT_SECURITY )   
  security_params_t sec_data;   
  sec_data.security_level = 0x06; 
  sec_data.key_id_mode = 0x01;
  memset(sec_data.key_identifier, 0xFF, 0x08);
  sec_data.key_identifier[8] = /*0x01;*/ (key_id_index+1);    
#endif
  
  fan_mac_params.type = FAN_DATA_PKT;
  fan_mac_params.transfer_type = 0x00;
  fan_mac_params.multiplex_id = 0xA0ED;
  uint8_t transaction_id = 0x01;
  
  //Raka ..
  fan_mac_params.transfer_type =((transaction_id << 3) | fan_mac_params.transfer_type);
  
  mem_reverse_cpy(rev_addr,p_rx_addr->u8,8);
  
  /*if the rev_add is not same as its parent address
  the packet has to be routed using HAN router node*/
  PLME_get_request( phyCurrentChannel, &len, &TxChannel );

#if(FAN_EDFE_FEATURE_ENABLED == 1)
  if((edfe_information.edfe_frame_enabled  == 0x01))
  {
    if((edfe_information.edfe_frame_tx_type == RESPONSE_FRAME))
    {
      SrcAddr.address_mode = ADDR_MODE_NONE;
    }
    if(edfe_information.edfe_frame_tx_type == FINAL_RESPONSE_FRAME)
    {
      //send_edfe_initial_frame(edfe_information.edfe_ini_mac_addr ,1,FINAL_RESPONSE_FRAME);
      //enable_disable_edfe_frame(0,255);
    }
    if(edfe_information.edfe_frame_tx_type == INITIAL_FRAME)
    {
      SrcAddr.pan_id = get_current_pan_id();
      SrcAddr.address_mode = ADDR_MODE_EXTENDED;
      SrcAddr.address.ieee_address = linkaddr_node_addr.u8;
      DstAddr.pan_id = get_current_pan_id();
    }
    else
    {
      SrcAddr.pan_id = get_current_pan_id();
      SrcAddr.address_mode = ADDR_MODE_EXTENDED;
      SrcAddr.address.ieee_address = linkaddr_node_addr.u8;
      DstAddr.pan_id = get_current_pan_id();
    }
  }
  else if(edfe_information.edfe_frame_enabled  == 0x00)
#endif
  {
    SrcAddr.pan_id = get_current_pan_id();
    SrcAddr.address_mode = ADDR_MODE_EXTENDED;
    SrcAddr.address.ieee_address = linkaddr_node_addr.u8;
    DstAddr.pan_id = get_current_pan_id();
  }
  
  
  for(i=0;i<8;i++)
  {
    if(  p_rx_addr->u8[i] != 0x00 )
      break;
  }
  
  if(i == 8)
  {
    /*all bytes are 0. So do a broadcast*/
    DstAddr.address_mode = ADDR_MODE_NONE;
    DstAddr.address.ieee_address = NULL;
    DstAddr.address.short_address = 0xFFFF;
  }
  else
  {
    DstAddr.address_mode = ADDR_MODE_EXTENDED;
    DstAddr.address.ieee_address = rev_addr;
 
#if(FAN_EDFE_FEATURE_ENABLED == 1)
    if((edfe_information.edfe_frame_enabled == 0x01)
       &&(edfe_information.edfe_frame_tx_type != 0x99))
    {
      memcpy(edfe_information.edfe_ini_mac_addr,p_rx_addr->u8,8);
      tx_options |= NO_ACKNOWLEDGED_TRANSMISSION;
    }
    else
#endif
    {
      tx_options |= ACKNOWLEDGED_TRANSMISSION;
    }
    tx_options |= MAC_INTRA_PAN;
  }
  
#if(FAN_EDFE_FEATURE_ENABLED == 1)
  /*use when send EDFE frame */
  if((edfe_information.edfe_frame_enabled == 0x01)
     &&(edfe_information.edfe_frame_tx_type != 0x99))
  {
    sub_hdr_bitmap = FC_IE_MASK | UTT_IE_MASK | BT_IE_MASK ;
    sub_pld_bitmap = US_IE_MASK | BS_IE_MASK;
  }
  else
#endif
  {
    sub_hdr_bitmap = UTT_IE_MASK | BT_IE_MASK;
    sub_pld_bitmap = US_IE_MASK | BS_IE_MASK;
  }
  
  if (((DstAddr.address_mode == ADDR_MODE_NONE)
       #if(FAN_FRQ_HOPPING_FEATURE_ENABLED == 1)
       &&(is_BDI_active())
       #endif 
      ) || ((DstAddr.address_mode == ADDR_MODE_EXTENDED)
       #if(FAN_FRQ_HOPPING_FEATURE_ENABLED == 1)
       &&(is_UDI_active())
      #endif
      ))
  { 
#if !(ENABLE_FAN_MAC_WITHOUT_SECURITY)
    if(mac_pib.mac_security_enabled)
    {
      FAN_MCPS_DATA_Request
        (
         &SrcAddr, /* Source Addressing */
         &DstAddr, /* Dst Addressing */		
         packetbuf_totlen(),					
         packetbuf_hdrptr(),
         msduHandle,//temp[0],								          		 
         tx_options,//tx_options
         (ushort)TxChannel, //ushort	 TxChannel,
         false, // bool PPDUCoding, 
         true, //uint8_t FCSLength,
         false, //bool  ModeSwitch,
         false, //uint8_t NewModeSUNPage,
         false, //uint8_t  ModeSwitchParameterEntry,
         false, //uint8_t  frameCtrlOptions,
         sub_hdr_bitmap,   
         sub_pld_bitmap,
         false,// sendmultipurpose,
         &sec_data
           );
    }
    else
#endif            
    {
      FAN_MCPS_DATA_Request
        (
         &SrcAddr, /* Source Addressing */
         &DstAddr, /* Dst Addressing */		
         packetbuf_totlen(),					
         packetbuf_hdrptr(),
         msduHandle,//temp[0],								          		 
         tx_options,//tx_options
         (ushort)TxChannel, //ushort	 TxChannel,
         false, // bool PPDUCoding, 
         true, //uint8_t FCSLength,
         false, //bool  ModeSwitch,
         false, //uint8_t NewModeSUNPage,
         false, //uint8_t  ModeSwitchParameterEntry,
         false, //uint8_t  frameCtrlOptions,
         sub_hdr_bitmap,  
         sub_pld_bitmap,
         false,// sendmultipurpose, 
         NULL               
           );  
    }
  }
  else if (DstAddr.address_mode == ADDR_MODE_EXTENDED)
  {
    l3_pkt_queue_t *item = (l3_pkt_queue_t *)app_bm_alloc (sizeof (l3_pkt_queue_t));
    
    item->length = packetbuf_totlen();
    item->data_ptr = (uint8_t *)app_bm_alloc (item->length);
    memcpy (item->data_ptr, packetbuf_hdrptr(), item->length);
    
    item->src.pan_id = SrcAddr.pan_id;
    item->src.address_mode = SrcAddr.address_mode;
    item->src.address.ieee_address = (uint8_t *)app_bm_alloc (8);
    memcpy (item->src.address.ieee_address, SrcAddr.address.ieee_address, 8);
    
    item->dst.pan_id = DstAddr.pan_id;
    item->dst.address_mode = DstAddr.address_mode; 
    item->dst.address.ieee_address = (uint8_t *)app_bm_alloc (8);
    memcpy (item->dst.address.ieee_address, DstAddr.address.ieee_address, 8);
    
    item->fan_packet_type = FAN_DATA_PKT;
    item->msduHandle = msduHandle;
    queue_item_put(fan_mac_params.l3_layer_unicast_queue, (queue_item_t *)item);
  }
  else if (DstAddr.address_mode == ADDR_MODE_NONE)
  {
    l3_pkt_queue_t *item = (l3_pkt_queue_t *)app_bm_alloc (sizeof (l3_pkt_queue_t));
    
    item->length = packetbuf_totlen();
    item->data_ptr = (uint8_t *)app_bm_alloc (item->length);
    memcpy (item->data_ptr, packetbuf_hdrptr(), item->length);
    
    item->src.pan_id = SrcAddr.pan_id;
    item->src.address_mode = SrcAddr.address_mode;
    item->src.address.ieee_address = (uint8_t *)app_bm_alloc (8);
    memcpy (item->src.address.ieee_address, SrcAddr.address.ieee_address, 8);
    
    item->dst.pan_id = DstAddr.pan_id;
    item->dst.address_mode = DstAddr.address_mode; 
    item->dst.address.ieee_address = (uint8_t *)app_bm_alloc (8);
    memcpy (item->dst.address.ieee_address, DstAddr.address.ieee_address, 8);
    
    item->fan_packet_type = FAN_DATA_PKT;
    item->msduHandle = msduHandle;
    queue_item_put(fan_mac_params.l3_layer_broadcast_queue, (queue_item_t *)item);
  }
}

/******************************************************************************/

const struct mac_driver pro_mac_driver = {
  "Procubed-MAC-PHY",
  init,
  send_packet,
  input_packet,
  on,
  off,
  channel_check_interval,
};

/******************************************************************************/

void int_l3_queue (void)
{
  fan_mac_params.l3_layer_unicast_queue = queue_manager_get_list( QUEUE_L3_UNICAST_PKT );
  fan_mac_params.l3_layer_broadcast_queue = queue_manager_get_list( QUEUE_L3_BROADCAST_PKT );
}

extern uint8_t TANSIT_KMP_ID;

void trigger_explicit_sicslowpan_packet (uint8_t schedule_type)
{
  l3_pkt_queue_t *item = NULL;
  
check_queue_count:
  if(schedule_type == 0x01) // unicast sch 
    item = (l3_pkt_queue_t *)queue_item_get (fan_mac_params.l3_layer_unicast_queue);
  else // broadcast sch
    item = (l3_pkt_queue_t *)queue_item_get (fan_mac_params.l3_layer_broadcast_queue);
  
  if (NULL_POINTER != item)
  {
    uint8_t tx_options = 0;
    uint32_t TxChannel = 0;
    uint16_t len = 0;
    uint32_t sub_hdr_bitmap = 0;
    uint32_t sub_pld_bitmap = 0;
    uint8_t node_type = get_node_type( );
    
#if !(ENABLE_FAN_MAC_WITHOUT_SECURITY )
    security_params_t sec_data; 
    if(item->fan_packet_type == FAN_DATA_PKT)
    {
      sec_data.security_level = 0x06; 
      sec_data.key_id_mode = 0x01;
      memset(sec_data.key_identifier, 0xFF, 0x08);
      sec_data.key_identifier[8] = /*0x01;*/ (key_id_index+1);
    }
#if(FAN_EAPOL_FEATURE_ENABLED == 1)
    if(item->fan_packet_type == EAPOL)
    {
      
      sec_data.security_level = 0x00; 
      sec_data.key_id_mode = 0x00;
      memset(sec_data.key_identifier, 0xFF, 0x08);
      sec_data.key_identifier[8] = /* 0x01; */ (key_id_index+1);  
    }
#endif
#endif
    if( item->fan_packet_type == FAN_DATA_PKT)
    {
      fan_mac_params.type = FAN_DATA_PKT;
      fan_mac_params.transfer_type = 0x00;
      fan_mac_params.multiplex_id = 0xA0ED;
      uint8_t transaction_id = 0x01;
      
      //Raka ..
      fan_mac_params.transfer_type =((transaction_id << 3) | fan_mac_params.transfer_type);
      
      /*if the rev_add is not same as its parent address
      the packet has to be routed using HAN router node*/
      PLME_get_request( phyCurrentChannel, &len, &TxChannel );
      /*use when send EDFE frame */
      if(item->dst.address_mode == ADDR_MODE_EXTENDED)
      {
#if(FAN_EDFE_FEATURE_ENABLED == 1)
        if((edfe_information.edfe_frame_enabled == 0x01)
           &&(edfe_information.edfe_frame_tx_type != 0x99))
        {
          memcpy(edfe_information.edfe_ini_mac_addr,item->dst.address.ieee_address,8);
          tx_options |= NO_ACKNOWLEDGED_TRANSMISSION;
        }
        else
#endif
        {
          tx_options |= ACKNOWLEDGED_TRANSMISSION;
        }
        tx_options |= MAC_INTRA_PAN;
      }
      
#if(FAN_EDFE_FEATURE_ENABLED == 1)
      /*use when send EDFE frame */
      if((edfe_information.edfe_frame_enabled == 0x01)
         &&(edfe_information.edfe_frame_tx_type != 0x99))
      {
        sub_hdr_bitmap = FC_IE_MASK | UTT_IE_MASK | BT_IE_MASK ;
        sub_pld_bitmap = US_IE_MASK | BS_IE_MASK;
      }
      else
#endif
      {
        sub_hdr_bitmap = UTT_IE_MASK | BT_IE_MASK;
        sub_pld_bitmap = US_IE_MASK | BS_IE_MASK;
      }
    }
#if(FAN_EAPOL_FEATURE_ENABLED == 1)
    else if( item->fan_packet_type == EAPOL)
    {
      fan_mac_params.type = EAPOL;
      fan_mac_params.transfer_type = 0x00;
      fan_mac_params.multiplex_id = 0x0001;
      uint8_t transaction_id = 0x01;
      
      //Raka ..
      fan_mac_params.transfer_type =((transaction_id << 3) | fan_mac_params.transfer_type);
      
      if(TANSIT_KMP_ID == 85 && get_node_type() == 0x00)
      {
        sub_hdr_bitmap = UTT_IE_MASK | BT_IE_MASK | EA_IE_MASK; 
        sub_pld_bitmap = BS_IE_MASK;
      }
      else if(TANSIT_KMP_ID == 85 && get_node_type() != 0x00 && mac_self_fan_info.pan_metrics.parent_bs_ie_use == 0x01)
      {
        sub_hdr_bitmap = UTT_IE_MASK | BT_IE_MASK | EA_IE_MASK;
        sub_pld_bitmap = BS_IE_MASK;
      }
      else
      {
        sub_hdr_bitmap = UTT_IE_MASK;
        sub_pld_bitmap = NO_IE_MASK;
      }
      
      if(TANSIT_KMP_ID == 0x01 || TANSIT_KMP_ID == 85)
        fan_mac_params.KMP_ID = 0x01;
      else if(TANSIT_KMP_ID == 0x06)
        fan_mac_params.KMP_ID = 0x06;
      else if (TANSIT_KMP_ID == 0x07)
        fan_mac_params.KMP_ID = 0x07;
      
      PLME_get_request( phyCurrentChannel, &len, &TxChannel );
      tx_options = ACKNOWLEDGED_TRANSMISSION;
      tx_options |= MAC_INTRA_PAN;
    } 
#endif //#if(FAN_EAPOL_FEATURE_ENABLED == 1)
#if !(ENABLE_FAN_MAC_WITHOUT_SECURITY)
    if(mac_pib.mac_security_enabled)
    {
      FAN_MCPS_DATA_Request
        (
         &item->src, /* Source Addressing */
         &item->dst, /* Dst Addressing */		
         item->length, 					
         item->data_ptr,
         item->msduHandle,//temp[0],								          		 
         tx_options,//tx_options
         (ushort)TxChannel, //ushort	 TxChannel,
         false, // bool PPDUCoding, 
         true, //uint8_t FCSLength,
         false, //bool  ModeSwitch,
         false, //uint8_t NewModeSUNPage,
         false, //uint8_t  ModeSwitchParameterEntry,
         false, //uint8_t  frameCtrlOptions,
         sub_hdr_bitmap,   
         sub_pld_bitmap,
         false,// sendmultipurpose,
         &sec_data
           );
    }
    else
#endif            
    {
      FAN_MCPS_DATA_Request
        (
         &item->src, /* Source Addressing */
         &item->dst, /* Dst Addressing */		
         item->length, 					
         item->data_ptr,
         item->msduHandle,//temp[0],								          		 
         tx_options,//tx_options
         (ushort)TxChannel, //ushort	 TxChannel,
         false, // bool PPDUCoding, 
         true, //uint8_t FCSLength,
         false, //bool  ModeSwitch,
         false, //uint8_t NewModeSUNPage,
         false, //uint8_t  ModeSwitchParameterEntry,
         false, //uint8_t  frameCtrlOptions,
         sub_hdr_bitmap,  
         sub_pld_bitmap,
         false,// sendmultipurpose, 
         NULL               
           );  
    }
    
    app_bm_free (item->src.address.ieee_address);
    app_bm_free (item->dst.address.ieee_address);
    app_bm_free (item->data_ptr);
    app_bm_free ((uint8_t *)item);
    
    goto check_queue_count;
  }
}
/******************************************************************************/
void set_datapkt_type()
{
  fan_mac_params.type = FAN_DATA_PKT;
}
/******************************************************************************/
uint16_t rsii_val = 0;
void update_rssi_for_every_pakt(int8_t mpduLinkQuality,uint8_t *recvd_mac_addr)
{
  int16_t rssi = 0;
  uint8_t reverse_mac_address[8];
  mem_rev_cpy (reverse_mac_address, recvd_mac_addr, 8);
  rssi = (char) get_LQI_from_RSSI( mpduLinkQuality );
  rsii_val = (char) get_LQI_from_RSSI( mpduLinkQuality );
  packetbuf_set_attr(PACKETBUF_ATTR_RSSI, rssi);
  link_stats_input_callback((linkaddr_t *)reverse_mac_address);
}

void update_link_stats_on_tx(uint8_t *lladdr)
{
  link_stats_packet_sent((linkaddr_t*)lladdr, 0, 1);
}
void update_seq_number_l2_attr(uint8_t seq_number)
{
  packetbuf_set_attr(PACKETBUF_ATTR_MAC_SEQNO,seq_number);
}