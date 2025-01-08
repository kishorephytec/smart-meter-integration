/** \file fan_api.c
 *******************************************************************************
 ** \brief Implements the fan application API's
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
 **\endcond
 */

/*
*******************************************************************************
* File inclusion
********************************************************************************
*/

#include "StackAppConf.h"
#include <stdarg.h>
#include "common.h"
#include "em_device.h"
#include "list_latest.h"
#include "queue_latest.h"
#include "uart_hal.h"
#include "mac.h"
#include "phy.h"
#include "hif_utility.h"
#include "hif_service.h"
#include "buff_mgmt.h"
#include "buffer_service.h"
#include "hw_tmr.h"
#include "sw_timer.h"
#include "mac_interface_layer.h"
#include "sm.h"
#include "l3_configuration.h"
#include "contiki-net.h"
#include "ie_element_info.h"
#include "network-manager.h"
#include "fan_api.h"
#include "fan_app_auto.h"
#include "fan_mac_nbr_info.h"
#include "timer_service.h"
#include "fan_app_test_harness.h"
/*
** ============================================================================
** Private Macro definitions
** ============================================================================
*/

#define PARENT_BS_IE_USE        1
#define FAN_TPS_VERSION         1
#define TOTAL_LEN_PAN_IE        7//[parent_bs_ie_use=1, routing_methood=1, fan_tps_version=1, pan_size=2, routing_cost=2]



/*
** ============================================================================
** Private Structures, Unions & enums Type Definitions
** ============================================================================
*/


white_list_t white_mac_list;
/*None*/

/*
** ============================================================================
** Private Variable Definitions
** ============================================================================
*/

static uint8_t rev_self_addr[8]={0};

/*
** ============================================================================
** Private Function Prototypes
** ============================================================================
*/

static void set_phy_mode_using_reg_dom_oprating_class_usi();
static void set_phy_mode_using_explicit();

/*
** ============================================================================
** Public Variable Definitions
** ============================================================================
*/ 

/*None*/

/*
** ============================================================================
** Public Function Prototypes
** ============================================================================
*/

void get_self_extended_address(uint8_t *self_mac_addr);

/*
** ============================================================================
** External Variable Declarations
** ============================================================================
*/
extern int8_t TRX_RAIL_PHY_Mode_Change (uint8_t phyModeID);
extern fan_nwk_manager_sm_t fan_nwk_manager_app;
extern uint8_t phyModeMapArr[8];
extern uint8_t global_addr_device[16];
extern uint8_t root_device;
extern uint8_t response_laye_ID;
/*this varriable not used in this file*/
#if(APP_NVM_FEATURE_ENABLED == 1)
extern nvm_structure_t store_nvm_param;
#endif

extern fan_mac_nbr_t fan_mac_nbr;
/*
** =============================================================================
** External Function Declarations
** =============================================================================
*/

extern uint8_t get_max_supported_channel_on_phy_selected();
extern void fan_nwk_manager_init( );
extern void rpl_reset_periodic_timer(void);
//extern void rpl_reset_dio_timer(rpl_instance_t *);
extern uip_ds6_maddr_t *get_mcast_addr();
/*Umesh : 25-01-2018*/
extern int8_t get_nbr_addr_count(uint8_t index);
extern uint8_t* get_nbr_address(int index);
//extern rpl_rank_t rpl_get_parent_rank(uip_lladdr_t *addr); 
//extern rpl_parent_t *rpl_get_parent(uip_lladdr_t *addr);
//extern rpl_rank_t rpl_rank_via_parent(rpl_parent_t *p);
//extern int get_rssi_from_parent(rpl_parent_t *p);
extern void send_mac_addr(uint8_t *buff , uint16_t len);
extern void set_async_channel_range(void);
void send_revocation_key_to_hostapd(uint8_t *buff , uint16_t len);
//extern uip_ipaddr_t* get_prefered_parents_addr(uip_ipaddr_t *child_ip_addr);
void wan_uip_icmp6_send_echo_reply (void);
/*this function not used in this file*/

uint8_t send_hif_conf_cb( uint8_t cmd_id,uint8_t status );

void send_gtkhash_to_hostapd(uint8_t *buff , uint16_t len);
int get_join_state (void);
void trickle_timer_inconsistency_pc(void);
void get_linklocal_and_global_address_from_mac_address (uint8_t *mac_addr, uint8_t *link_local_addr, uint8_t *global_addr);

#if(FAN_EDFE_FEATURE_ENABLED == 1)
extern void send_edfe_initial_frame(uint8_t *src_addr , uint8_t value,uint8_t edfe_frame_type);
#endif

/*
** ============================================================================
** Public Function Definitions
** ============================================================================
*/  

uint8_t node_start_stop( uint8_t start)
{
  if(start == FAN_STOP_NODE)
  {
    /*Stop – Node ceases execution of its FAN protocol stack 
    and returns to an idle state waiting for subsequent API commands.*/
    fan_nwk_manager_app.node_basic_cfg.board_reset = 0x01;
    //    store_nvm_param.board_reset = 0x01;
#if (PRINT_DEBUG_LEVEL==SHORT_PRINT)        
    stack_print_debug ("Got NODE STOP Request\n");
#endif     
    
#if(APP_NVM_FEATURE_ENABLED == 1)
    nvm_store_node_basic_info();
#endif
    
    NVIC_SystemReset();
  }
  if(start == FAN_START_NODE)
  {
    /*Cold Boot – node executes boot process of its FAN protocol stack 
    using factory defaults plus any administrative configuration applied. 
    Any runtime state acquired from prior 
    execution sessions is destroyed (PMK, PTK, GTKs, neighbor tables, etc.).*/
    //set_async_channel_range();
#if (PRINT_DEBUG_LEVEL==SHORT_PRINT)        
    stack_print_debug ("Got NODE START Request\n");
#endif     
    if (fan_nwk_manager_app.state_ind != NODE_MAC_READY_STATE)
      fan_nwk_manager_init( );
    else
      send_hif_conf_cb(NODE_START_STOP_CONF,2);
  }
  
  return 0;
}
/*============================================================================*/
uint8_t set_phy(uint8_t modulation, uint8_t symbol_rate , float modulation_index ,uint8_t pa_level)
{
    fan_nwk_manager_app.node_basic_cfg.sybbol_rate = symbol_rate;
    fan_nwk_manager_app.node_basic_cfg.modulation_index = modulation_index;
  // The Value of the PA level is in the 4th location
 
    if(pa_level > 0x7f)
    {
      pa_level = 0x7f;
    }
    if(pa_level < 0x01)
    {
      pa_level = 0x7f;
    }
    
    fan_nwk_manager_app.node_basic_cfg.pa_level = pa_level;            
//    TRX_Set_PA_Level( fan_nwk_manager_app.node_basic_cfg.pa_level );
//    store_nvm_param.modulation_index = modulation_index;
//    store_nvm_param.pa_level = pa_level;
//    store_nvm_param.sybbol_rate = symbol_rate;
//    nvm_store_node_basic_info(); 
    return 0;
}

/*============================================================================*/
int8_t set_RAIL_phyMode(uint8_t modeID)
{
   
    return  TRX_RAIL_PHY_Mode_Change ( modeID );
}


/*============================================================================*/
uint8_t set_mac_chan_plan_reg_op
(
      uint8_t regulatory_domain,
      uint8_t operating_class 
)
{
    /*
        The Channel Plan field is a 3 bit unsigned integer populated as follows: 1488
        1. The Channel Plan field MUST be set to 0  For Regulatory Domain and Operating Class 
        2. The Channel Plan field MUST be set to 1 For CH0,Channel Spacing, and Number Of Channel
  */
  
  if((regulatory_domain == 0x01 )&&
     ((operating_class==0x01) ||
      (operating_class==0x02) || 
        (operating_class==0x03)))
  {
    fan_nwk_manager_app.node_basic_cfg.us_ie.un_channel_plan.ch_reg_op.reg_domain = regulatory_domain;
    fan_nwk_manager_app.node_basic_cfg.bs_ie.un_channel_plan.ch_reg_op.reg_domain = regulatory_domain;
    fan_nwk_manager_app.node_basic_cfg.us_ie.un_channel_plan.ch_reg_op.op_class = operating_class;  
    fan_nwk_manager_app.node_basic_cfg.bs_ie.un_channel_plan.ch_reg_op.op_class = operating_class; 
    
    fan_nwk_manager_app.node_basic_cfg.us_ie.us_schedule.channel_plan = 0x00;
    fan_nwk_manager_app.node_basic_cfg.bs_ie.bs_schedule.channel_plan = 0x00;
    

    set_phy_mode_using_reg_dom_oprating_class_usi();

    return 0;
  }
  return 1;
}
/*============================================================================*/
uint8_t set_mac_unicast_chan_plan
(
uint8_t dwell_interval, 
uint8_t channel_function,
uint8_t excluded_channel_control,
uint8_t excluded_channel_number,
uint16_t *excluded_channel_range,
uint8_t *excluded_channel_Mask
)
{ 
  
  
  uint8_t total_ex_channel = 0;
  int i = 0, j = 0;
  fan_nwk_manager_app.node_basic_cfg.us_ie.us_schedule.dwell_interval = dwell_interval;
  fan_nwk_manager_app.node_basic_cfg.us_ie.us_schedule.channel_function = channel_function;
  //        fan_nwk_manager_app.node_basic_cfg.us_ie.us_schedule.channel_plan = 0x00;
  
  fan_nwk_manager_app.node_basic_cfg.us_ie.us_schedule.excludded_channel_control = excluded_channel_control;
  fan_nwk_manager_app.node_basic_cfg.us_ie.us_schedule.excluded_channels.excluded_channel_ranges.num_of_ranges = excluded_channel_number;
  total_ex_channel = fan_nwk_manager_app.node_basic_cfg.us_ie.us_schedule.excluded_channels.excluded_channel_ranges.num_of_ranges;
  
  if(fan_nwk_manager_app.node_basic_cfg.us_ie.us_schedule.excludded_channel_control == 0x01)
  {
    if(total_ex_channel <= MAXIMUM_EXCLUDE_CHANNEL_SUPPORT)
    {
      j = 0;
      for(i = 0; i < total_ex_channel; i++)
      {
        memcpy(&fan_nwk_manager_app.node_basic_cfg.us_ie.us_schedule.excluded_channels.excluded_channel_ranges.ex_ch_range[i].start_ch, &excluded_channel_range[j], 2);
        j++;
        memcpy(&fan_nwk_manager_app.node_basic_cfg.us_ie.us_schedule.excluded_channels.excluded_channel_ranges.ex_ch_range[i].end_ch, &excluded_channel_range[j], 2);
        j++;
      }
    }
  }
  else if(fan_nwk_manager_app.node_basic_cfg.us_ie.us_schedule.excludded_channel_control == 0x02)
  {
    memcpy((uint8_t *)& fan_nwk_manager_app.node_basic_cfg.us_ie.us_schedule.excluded_channels.excluded_channel_mask,excluded_channel_Mask,17);
  }
  else
  {
    memset ((uint8_t *)& fan_nwk_manager_app.node_basic_cfg.us_ie.us_schedule.excluded_channels, 0 , sizeof (excluded_chan_range_info_t));
  }

  return 0;
}
/*============================================================================*/
uint8_t  set_mac_chan_plan_explicit
(
 
    uint32_t ch0,
    uint8_t channel_spacing, 
    uint16_t number_of_channel  
)
{
  
      /*
        The Channel Plan field is a 3 bit unsigned integer populated as follows: 1488
        1. The Channel Plan field MUST be set to 0  For Regulatory Domain and Operating Class 
        2. The Channel Plan field MUST be set to 1 For CH0,Channel Spacing, and Number Of Channel
  */
  uint32_t channel_plan;
  uint32_t channel_numbers;
  uint8_t max_supported_channel = 0xff;
  fan_nwk_manager_app.node_basic_cfg.us_ie.un_channel_plan.ch_explicit.channel_spacing = channel_spacing;
  fan_nwk_manager_app.node_basic_cfg.bs_ie.un_channel_plan.ch_explicit.channel_spacing = channel_spacing;
  fan_nwk_manager_app.node_basic_cfg.us_ie.us_schedule.channel_plan = 0x01;
  fan_nwk_manager_app.node_basic_cfg.bs_ie.bs_schedule.channel_plan = 0x01;
  channel_plan =fan_nwk_manager_app.node_basic_cfg.us_ie.us_schedule.channel_plan;
  set_phy_mode_using_explicit();
  
  if((fan_nwk_manager_app.node_basic_cfg.phy_mode == 0x01 ) || (fan_nwk_manager_app.node_basic_cfg.phy_mode == 0x02 ))
  {
    max_supported_channel = 129;
  }
  if((fan_nwk_manager_app.node_basic_cfg.phy_mode == 0x04 ) || (fan_nwk_manager_app.node_basic_cfg.phy_mode == 0x08 ))
  {
    max_supported_channel = 64;
  }
  if(fan_nwk_manager_app.node_basic_cfg.phy_mode == 16 )
  {
    max_supported_channel = 43;
  }
  if(number_of_channel <= max_supported_channel)
  {
    fan_nwk_manager_app.node_basic_cfg.us_ie.un_channel_plan.ch_explicit.ch0 = ch0;
    fan_nwk_manager_app.node_basic_cfg.bs_ie.un_channel_plan.ch_explicit.ch0 = ch0;
    fan_nwk_manager_app.node_basic_cfg.us_ie.un_channel_plan.ch_explicit.num_chans = number_of_channel;
    fan_nwk_manager_app.node_basic_cfg.bs_ie.un_channel_plan.ch_explicit.num_chans = number_of_channel;
    channel_numbers = number_of_channel;
    
    PLME_set_request( phyExplicit_canter_freq,4,&ch0);
    PLME_set_request( phyExplicit_total_ch_number,1,&channel_numbers);
    PLME_set_request( phyExplicit_Channel_plan,1,&channel_plan);

    return 0;
  }
  return 1;
}
/*============================================================================*/
uint8_t set_mac_bcast_chan_plan
(
uint32_t broadcast_interval,
uint16_t broadcast_Schedule_identifier,
uint8_t  dwell_interval,
uint8_t  channel_function,
uint8_t excluded_channel_control,
uint8_t  excluded_channel_number, 
uint16_t  *excluded_channel_Range,
uint8_t  *excluded_channel_Mask

)
{
  uint8_t total_ex_channel = 0;
  int i = 0, j = 0;
  fan_nwk_manager_app.node_basic_cfg.bs_ie.bcast_interval = broadcast_interval;
  fan_nwk_manager_app.node_basic_cfg.bs_ie.bcast_sched_id = broadcast_Schedule_identifier;
  fan_nwk_manager_app.node_basic_cfg.bs_ie.bs_schedule.dwell_interval = dwell_interval;
  fan_nwk_manager_app.node_basic_cfg.bs_ie.bs_schedule.channel_function = channel_function;
  //    fan_nwk_manager_app.node_basic_cfg.bs_ie.bs_schedule.channel_plan = 0x00;
  
  fan_nwk_manager_app.node_basic_cfg.bs_ie.bs_schedule.excludded_channel_control = excluded_channel_control;
  fan_nwk_manager_app.node_basic_cfg.bs_ie.bs_schedule.excluded_channels.excluded_channel_ranges.num_of_ranges = excluded_channel_number;
  total_ex_channel = fan_nwk_manager_app.node_basic_cfg.bs_ie.bs_schedule.excluded_channels.excluded_channel_ranges.num_of_ranges;
  if(fan_nwk_manager_app.node_basic_cfg.bs_ie.bs_schedule.excludded_channel_control == 0x01)
  {
    if(total_ex_channel <= MAXIMUM_EXCLUDE_CHANNEL_SUPPORT)
    {
      j = 0;
      for(i = 0; i < total_ex_channel; i++)
      {
        memcpy(&fan_nwk_manager_app.node_basic_cfg.bs_ie.bs_schedule.excluded_channels.excluded_channel_ranges.ex_ch_range[i].start_ch, &excluded_channel_Range[j], 2);
        j+=1;
        memcpy(&fan_nwk_manager_app.node_basic_cfg.bs_ie.bs_schedule.excluded_channels.excluded_channel_ranges.ex_ch_range[i].end_ch, &excluded_channel_Range[j], 2);
        j+=1;
      }
    }
  }
  else if(fan_nwk_manager_app.node_basic_cfg.bs_ie.bs_schedule.excludded_channel_control == 0x02)
  {
    memcpy((uint8_t *)& fan_nwk_manager_app.node_basic_cfg.bs_ie.bs_schedule.excluded_channels.excluded_channel_mask,excluded_channel_Mask,17);
  }
  else
  {
    memset ((uint8_t *)& fan_nwk_manager_app.node_basic_cfg.bs_ie.bs_schedule.excluded_channels, 0 , sizeof (excluded_chan_range_info_t));
  }

  return 0;
}
/*============================================================================*/
uint8_t set_mac_chan_plan_fixed(uint16_t fixed_channel_number)
{
  uint8_t max_supported_channel = 0xff;
  if((fan_nwk_manager_app.node_basic_cfg.phy_mode == 0x01 ) || (fan_nwk_manager_app.node_basic_cfg.phy_mode == 0x02 ))
  {
    max_supported_channel = 129;
  }
  if((fan_nwk_manager_app.node_basic_cfg.phy_mode == 0x04 ) || (fan_nwk_manager_app.node_basic_cfg.phy_mode == 0x08 ))
  {
    max_supported_channel = 64;
  }
  if(fan_nwk_manager_app.node_basic_cfg.phy_mode == 16 )
  {
    max_supported_channel = 43;
  }
  if(fixed_channel_number <= max_supported_channel)
  {
    fan_nwk_manager_app.node_basic_cfg.us_ie.channel_fixed.fixed_chan = fixed_channel_number;
    fan_nwk_manager_app.node_basic_cfg.us_ie.us_schedule.channel_function = 0x00;
    fan_nwk_manager_app.node_basic_cfg.bs_ie.channel_fixed.fixed_chan = fixed_channel_number;
    fan_nwk_manager_app.node_basic_cfg.bs_ie.bs_schedule.channel_function = 0x00;
    fan_nwk_manager_app.node_basic_cfg.selected_channel = fixed_channel_number;
    
    fan_nwk_manager_app.node_basic_cfg.us_ie.us_schedule.excludded_channel_control = 0x00;
    fan_nwk_manager_app.node_basic_cfg.bs_ie.bs_schedule.excludded_channel_control = 0x00;

    return 0;
  }
  else
  {
    return 1;
  }
}
/*============================================================================*/

uint8_t  set_lbr_mac_gtks_config
(
    uint8_t *gtk0,
    uint8_t *gtk1,
    uint8_t *gtk2,
    uint8_t *gtk3,
    uint8_t gtkl
)
{
  if(fan_nwk_manager_app.node_basic_cfg.fan_device_type == LBR_TYPE)
  {
    if (gtkl & 0x01)
    {
      fan_nwk_manager_app.node_basic_cfg.gtkhash_ie.gtkl |= 0x01;
      memcpy((uint8_t *)fan_nwk_manager_app.node_basic_cfg.gtkhash_ie.gtk0_key,gtk0,16);
    }
    if (gtkl & 0x02)
    {
      fan_nwk_manager_app.node_basic_cfg.gtkhash_ie.gtkl |= 0x02;
      memcpy((uint8_t *)fan_nwk_manager_app.node_basic_cfg.gtkhash_ie.gtk1_key,gtk1,16);
    }
    if (gtkl & 0x04)
    {
      fan_nwk_manager_app.node_basic_cfg.gtkhash_ie.gtkl |= 0x04;
      memcpy((uint8_t *)fan_nwk_manager_app.node_basic_cfg.gtkhash_ie.gtk2_key,gtk2,16);
    }
    if (gtkl & 0x08)
    {
      fan_nwk_manager_app.node_basic_cfg.gtkhash_ie.gtkl |= 0x08;
      memcpy((uint8_t *)fan_nwk_manager_app.node_basic_cfg.gtkhash_ie.gtk3_key,gtk3,16);
    }

    if (get_join_state () > 0)
    {
 
      FAN_MAC_MLME_SET_Request
        (
         WISUN_INFO_PAYLOAD_IE_ID,/* header ie or payload ie */
         WISUN_IE_SUBID_GTKHASH_IE,/* subid for each ie */	        
         sizeof(gtk_key_t),/*(65+1)=(fan_nwk_manager_app.node_basic_cfg.gtkhash_ie.length+1),*/
         (uint8_t *)& fan_nwk_manager_app.node_basic_cfg.gtkhash_ie
           ); 
      
//      trickle_timer_inconsistency_pc ();
      uint8_t temp_gtk_hash_buf[65] = {0};
      uint8_t *temp_ptr = &temp_gtk_hash_buf[0];
      *temp_ptr++ = gtkl;
      memcpy(temp_ptr,(uint8_t *)fan_nwk_manager_app.node_basic_cfg.gtkhash_ie.gtk0_key,16);
      temp_ptr += 16;
      memcpy(temp_ptr,(uint8_t *)fan_nwk_manager_app.node_basic_cfg.gtkhash_ie.gtk1_key,16);
      temp_ptr += 16;
      memcpy(temp_ptr,(uint8_t *)fan_nwk_manager_app.node_basic_cfg.gtkhash_ie.gtk2_key,16);
      temp_ptr += 16;
      memcpy(temp_ptr,(uint8_t *)fan_nwk_manager_app.node_basic_cfg.gtkhash_ie.gtk3_key,16);
      temp_ptr += 16;
      send_gtkhash_to_hostapd (temp_gtk_hash_buf, (temp_ptr - temp_gtk_hash_buf));
    }
    return 0;
  }
  return 1;
}
/*============================================================================*/
uint8_t set_lbr_mac_lifetime_config (uint32_t pmk_lifetime,
                                     uint32_t ptk_lifetime,
                                     uint32_t gtk_lifetime,
                                     uint32_t gtk_new_activation_time,
                                     uint32_t revocation_lifetime_reduction)
{
  
 
  return 1;
}
/*============================================================================*/
uint8_t  set_lbr_mac_config
(
    uint16_t pan_size,
    uint16_t pan_id,
    uint8_t use_parent_broadcast_schedule,
    uint8_t routing_method,
    uint8_t *network_Name
)
{
  if(fan_nwk_manager_app.node_basic_cfg.fan_device_type == LBR_TYPE)
  {
        uint8_t length = strlen((char const*)network_Name);
        fan_nwk_manager_app.node_basic_cfg.pan_ie.pan_size = pan_size;
        if(pan_id == 0xFFFF)
        {
          return 2;
        }
        fan_nwk_manager_app.node_basic_cfg.selected_pan_id = pan_id;
        fan_nwk_manager_app.node_basic_cfg.pan_ie.pan_id = pan_id;
        if(use_parent_broadcast_schedule <= 0x01)
          fan_nwk_manager_app.node_basic_cfg.pan_ie.parent_bs_ie_use = use_parent_broadcast_schedule;
        else
          fan_nwk_manager_app.node_basic_cfg.pan_ie.parent_bs_ie_use = 1;
        fan_nwk_manager_app.node_basic_cfg.pan_ie.routing_methood = routing_method;
        fan_nwk_manager_app.node_basic_cfg.netname_ie.length = length;
        memcpy(&fan_nwk_manager_app.node_basic_cfg.netname_ie.network_name,network_Name,length);
   
        return 0;
  }
  return 1;
}
/*============================================================================*/
uint8_t reset_rpl_msg_rate(uint8_t message)
{
  

    return 0;
  
}
/*============================================================================*/
void get_sec_keys
(
        uint8_t  status_code,
        uint8_t  status_description,
        uint8_t pmk,
        uint8_t ptk,
        uint8_t gtk0,
        uint8_t gtk1,
        uint8_t gtk2,
        uint8_t gtk3

)
{
    uint8_t arr [2] = {0x00};
    arr[0] = CMD_NOT_SUPPORTED;
    arr[1] = 1;
    hif_send_msg_up(arr,2,response_laye_ID,PROTOCOL_ID_FOR_APP);
}   
/*============================================================================*/

void get_ip_address()
{
  uint8_t ip_data[50] = {0};
  uip_ds6_addr_t* self_ip_addr = NULL;
  uip_ds6_addr_t* link_ip_addr = NULL;
  uip_ds6_maddr_t* mcast_ip_addr = NULL;
  uint8_t* buf = ip_data;
  
  *buf++ = SEND_IPv6_ADDRESS;
  
  if(fan_nwk_manager_app.node_basic_cfg.fan_device_type == LBR_TYPE)
  {
    *buf++ = 0x01;
    self_ip_addr  = uip_ds6_get_global (ADDR_PREFERRED); 
    memcpy (buf, self_ip_addr->ipaddr.u8, 16);
    buf += 16;
  }
  else
  {
    memcpy (buf, global_addr_device, 16);
    buf += 16;
  }
  
  link_ip_addr = uip_ds6_get_link_local(ADDR_PREFERRED);
  mcast_ip_addr = get_mcast_addr();
  
  if (link_ip_addr != NULL)
  {
    memcpy (buf, link_ip_addr->ipaddr.u8, 16);
    buf += 16;
  }
  if (mcast_ip_addr != NULL)
  {
    memcpy (buf, mcast_ip_addr->ipaddr.u8, 16);
    buf += 16;
  }
  
  hif_send_msg_up (ip_data, (buf - ip_data), response_laye_ID, PROTOCOL_ID_FOR_APP);            
}
/*============================================================================*/

uint8_t subscribe_packets
(
  uint8_t command,
  uint8_t *forwarding_address,
  uint16_t forwarding_port
)
{
 return 1;
}

/*============================================================================*/

void send_runtime_log (char *format, ...)
{
  va_list vargs;
  char hifBuff[200] = {0};
  char *buf = hifBuff;
  *buf++ = SEND_RUNTIME_LOG;
  va_start (vargs, format);
  vsprintf (buf, format, vargs);
  buf += (strlen (hifBuff) - 1);
  va_end (vargs);
  *buf = DUMMY_COMPORT;
  hif_send_msg_up ((uint8_t *)hifBuff, buf - hifBuff, 5, 1);
}

/*============================================================================*/
void get_dodag_routers(void)
{
  if(fan_nwk_manager_app.node_basic_cfg.fan_device_type == LBR_TYPE)
  {
    rpl_ns_node_t *l = NULL;
    unsigned char addr[16] = {0};
    uint8_t hifBuff[1000] = {0};
    uint16_t len =0;
    uint16_t dodag_count = 0;
    uint8_t* buf = &hifBuff[0];
    *buf++ = SEND_DODAG_ROUTER_TABLE;
#if (PRINT_DEBUG_LEVEL==SHORT_PRINT)        
    stack_print_debug ("Returning DODAG\n");
#endif    
    for(l = rpl_ns_node_head(); l != NULL; l = rpl_ns_node_next(l)) 
    {
      dodag_count++;
      memcpy(addr, l->link_identifier, 16); 
#if (PRINT_DEBUG_LEVEL==SHORT_PRINT)        
      stack_print_debug ("DODAG %d\n", dodag_count);
      stack_print_debug ("--%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X\n", 
                         addr[0],addr[1],addr[2],addr[3],addr[4],addr[5],addr[6],addr[7],
                         addr[8],addr[9],addr[10],addr[11],addr[12],addr[13],addr[14],addr[15]);
#endif       
      memcpy( buf,addr, 16 );     
      buf+=16;
      len+=16;      
    }
    if (dodag_count > 0)
    {
      *buf++ = DUMMY_COMPORT;//Dummy Comport      
      hif_send_msg_up(&hifBuff[0], len+1,response_laye_ID,PROTOCOL_ID_FOR_APP); // (8 + 1) Dummy Port
      return;
    }
    else
    {
      uint8_t arr[2] = {0x00};
      arr[0] = SEND_DODAG_ROUTE_NULL;
      arr[1] = 1;
      hif_send_msg_up(arr,2,response_laye_ID,PROTOCOL_ID_FOR_APP);  
    }
  }
  else
  {
    uint8_t arr[2] = {0x00};
    arr[0] = CMD_NOT_SUPPORTED;
    arr[1] = 1;   
    hif_send_msg_up(arr,2,response_laye_ID,PROTOCOL_ID_FOR_APP);
  }
}

/*============================================================================*/

void trig_wan_ping (uint8_t *address)
{
//  uint8_t hifBuff[40] = {0};
//  uint8_t* buf = hifBuff;
//  *buf++ = TRIG_WAN_PING_REQST;
//  memcpy (buf, address, 16);
//  buf += 16;
//  *buf = DUMMY_COMPORT;
//  hif_send_msg_up (&hifBuff[0], buf - hifBuff, 5, 1);
  wan_uip_icmp6_send_echo_reply ();   //suneet :: send wan ping reply 
}

/*============================================================================*/

void recved_wan_ping_reply (uint8_t wan_reply_status)
{
  if (wan_reply_status == 0x00)
    wan_uip_icmp6_send_echo_reply ();
}

/*============================================================================*/

void get_neighbor_table(void)
{
  if(fan_nwk_manager_app.node_basic_cfg.fan_device_type != LBR_TYPE)
  {
    int ii = 0;
    uint8_t hifBuff[1500] = {0};
    uint8_t* buf = &hifBuff[0];
    uint16_t len =0;
    uint16_t nbr_count = 0;
    p3time_t time_diff;
    uint8_t link_local_addr[16] = {0};
    uint8_t global_addr[16] = {0};
    uint32_t routing_cost = 0;
    mac_nbr_descriptor_t *p_nbr_desc = NULL;
    
    *buf++ = SEND_NBR_TABLE;
    
    for (ii = 0; ii < fan_mac_nbr.mac_nbr_info_table_entries; ii++)
    {
      p_nbr_desc = (mac_nbr_descriptor_t*)queue_item_read_from (&fan_mac_nbr.desc_table, ii);
      if (p_nbr_desc != NULL)
      {
        nbr_count++;
        mem_rev_cpy( buf, p_nbr_desc->mac_addr, 8);              //Arjun: 05-01-2018:: Copying MAC addr from Neighbor table
        buf+=8;
        mem_rev_cpy( buf, (uint8_t *)&p_nbr_desc->etx, 2);              //Arjun: 05-01-2018:: Copying etx from Neighbor table
        buf+=2;
        *buf++= p_nbr_desc->rsl;                             //Arjun: 05-01-2018:: Copying rsl from Neighbor table
        *buf++= p_nbr_desc->rssi;                            //Arjun: 05-01-2018:: Copying rssi from Neighbor table
        mem_rev_cpy( buf, (uint8_t *)&p_nbr_desc->rev_pan_metrics.pan_size, 2);              //Arjun: 05-01-2018:: Copying pan_size from Neighbor table
        buf+=2;
        routing_cost = p_nbr_desc->rev_pan_metrics.routing_cost;
        mem_rev_cpy( buf, (uint8_t *)&routing_cost, 4);              //Arjun: 05-01-2018:: Copying routing_cost from Neighbor table
        buf+=4;
        memset (link_local_addr, 0, 16);
        memset (global_addr, 0, 16);
        get_linklocal_and_global_address_from_mac_address (p_nbr_desc->mac_addr, link_local_addr, global_addr);
        memcpy( buf, link_local_addr, 16);              //Arjun: 05-01-2018:: Copying link_local_addr from Neighbor table
        buf+=16;
        memcpy( buf, global_addr, 16);              //Arjun: 05-01-2018:: Copying global_addr from Neighbor table
        buf+=16;
        time_diff = timer_current_time_get() - 
          p_nbr_desc->ut_ie_rx_time;
        time_diff /= 1000;
        mem_rev_cpy( buf, (uint8_t *)&time_diff, 4);              //Arjun: 05-01-2018:: Copying time_since_last_rx from Neighbor table   
        buf+=4;
        *buf++=p_nbr_desc->is_parent_status;                         //Arjun: 05-01-2018:: Copying is_parent_status from Neighbor table
        len+=55;
      }
    } 
    if (nbr_count > 0)
    {
      *buf++ = DUMMY_COMPORT;//Dummy Comport
      hif_send_msg_up(&hifBuff[0], len+1,response_laye_ID,PROTOCOL_ID_FOR_APP);
      return;  
    }
    else
    {
      uint8_t arr [2] = {0x00};
      arr[0] = SEND_NBR_TABLE_NULL;
      arr[1] = 1;   
      hif_send_msg_up(arr,2,response_laye_ID,PROTOCOL_ID_FOR_APP);
    }
  }
  else
  {
    uint8_t arr [2] = {0x00};
    arr[0] = CMD_NOT_SUPPORTED;
    arr[1] = 1;
    hif_send_msg_up(arr,2,response_laye_ID,PROTOCOL_ID_FOR_APP);  
  }
}
/*============================================================================*/

uint8_t set_router_config(uint8_t *network_Name,uint8_t routing_method, uint16_t pan_size)
{
    if(fan_nwk_manager_app.node_basic_cfg.fan_device_type != LBR_TYPE)
    {
      uint8_t length = strlen((char const*)network_Name);
      fan_nwk_manager_app.node_basic_cfg.netname_ie.length = length;
      memset(&fan_nwk_manager_app.node_basic_cfg.netname_ie.network_name,0,32);
      memcpy(&fan_nwk_manager_app.node_basic_cfg.netname_ie.network_name,network_Name,fan_nwk_manager_app.node_basic_cfg.netname_ie.length);
      
      fan_nwk_manager_app.node_basic_cfg.pan_ie.parent_bs_ie_use = PARENT_BS_IE_USE;
      fan_nwk_manager_app.node_basic_cfg.pan_ie.routing_methood = routing_method;
      fan_nwk_manager_app.node_basic_cfg.pan_ie.fan_tps_version = FAN_TPS_VERSION;
      fan_nwk_manager_app.node_basic_cfg.pan_ie.pan_size = pan_size;
      fan_nwk_manager_app.node_basic_cfg.pan_ie.routing_cost = 0;//it should be calculated for ROUTER, for node startup it is set to 0
   
      return 0;
    }
    return 1;
}
/*============================================================================*/
uint8_t set_mac_white_list_tbc(uint8_t *white_list,uint16_t length)
{
  white_mac_list.wht_mac_index = 0;
  while(length!=0)
  {
    mem_rev_cpy((uint8_t *)&white_mac_list.wht_list_macaddr[white_mac_list.wht_mac_index++],white_list,8);
    white_list+=8;
    length -= 8;
  }
  return 0;
}

/*============================================================================*/

uint8_t set_mac_white_list(uint8_t *white_list,uint16_t length)
{
  white_mac_list.wht_mac_index = 0;
  
  white_list++;
  length--;
  while(length!=0)
  {
    mem_rev_cpy((uint8_t *)&white_mac_list.wht_list_macaddr[white_mac_list.wht_mac_index++],white_list,8);
    white_list+=8;
    length -= 8;
  }
  return 0;
}
/*============================================================================*/
//void get_mac_white_list()
//{
//  uint8_t hifBuff[500] = {0};
//   uint8_t* buf = &hifBuff[0];
//   uint8_t to_mac_add = white_mac_list.wht_mac_index;
//   uint8_t  i =0;
//   uint16_t len = 0;
//   if(white_mac_list.wht_mac_index != 0x00)
//   {
//     *buf++ = GET_MAC_WHITELIST_RESP;
//     while(to_mac_add!=0)
//     {
//       mem_rev_cpy( buf, (uint8_t *)&white_mac_list.wht_list_macaddr[i++], 8);
//       buf += 8;
//       len += 8;
//       to_mac_add--;
//     }
//     *buf++ = DUMMY_COMPORT;//Dummy Comport
//     hif_send_msg_up(&hifBuff[0], len+1,response_laye_ID,PROTOCOL_ID_FOR_APP);
//   }
//   else
//   {
//     uint8_t arr [2] = {0x00};
//     arr[0] = MAC_WHITELIST_NULL;
//     arr[1] = 1;   
//     hif_send_msg_up(arr,2,response_laye_ID,PROTOCOL_ID_FOR_APP);
//   }
//}
/*============================================================================*/
uint8_t set_revoaction_key(uint8_t *revoaction_list, uint16_t len)
{

//  if(fan_nwk_manager_app.node_basic_cfg.fan_device_type == LBR_TYPE)
//  {
//    send_revocation_key_to_hostapd (revoaction_list, len);
//    return 0;   /*Success*/
//  }
  return 1;     /*Failure*/
}
/*============================================================================*/
uint8_t get_prefered_parents(uint8_t *child_add, uint16_t len)
{
//  uint8_t hifBuff[20] = {0};
//  uint8_t *buf = &hifBuff[0];
//  *buf++ = GET_DEVICE_PRIMERY_PARENTS_RESP;
//  if(fan_nwk_manager_app.node_basic_cfg.fan_device_type == LBR_TYPE)
//  {
//    unsigned char addr[16] = {0};
//    uip_ipaddr_t child_ip_addr;
//    memcpy (&child_ip_addr, child_add, 16);
//    uip_ipaddr_t *preferd_parents = get_prefered_parents_addr (&child_ip_addr);
//    if (preferd_parents == NULL)
//      goto end;   
//    memcpy (addr, preferd_parents, 16);   
//#if (PRINT_DEBUG_LEVEL==SHORT_PRINT)        
//    stack_print_debug ("Returned Preferred Parent\n");
//    stack_print_debug ("--%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X\n", 
//                       addr[0],addr[1],addr[2],addr[3],addr[4],addr[5],addr[6],addr[7],
//                       addr[8],addr[9],addr[10],addr[11],addr[12],addr[13],addr[14],addr[15]);
//#endif    
//    memcpy (buf, addr, 16);
//    buf+=16;
//  }
//end:
//  *buf++ = DUMMY_COMPORT;//Dummy Comport
//  hif_send_msg_up (hifBuff, buf - hifBuff, response_laye_ID, PROTOCOL_ID_FOR_APP);
  return 0;
}

/*============================================================================*/

#if(FAN_EDFE_FEATURE_ENABLED == 1)
void send_edfe_exchange_frame(uint8_t *edfe_buf, uint16_t len)
{
  uint8_t mac_addr[8];
  memcpy(mac_addr,edfe_buf,8);
  edfe_buf+= 8;
  uint8_t frame_type = *edfe_buf++;
  send_edfe_initial_frame(mac_addr,1,frame_type);
}
#endif
/*============================================================================*/
void send_all_param_info_req (void)
{
  uint8_t hif_buf[500];
  uint16_t indx = 0;
  uint32_t symbol_rate = 0;
  uint8_t modulation_index = 0;
  uint8_t zero_buf[4] = {0};
  
  /* Factory mode Configuration */
  hif_buf[indx++] = GET_ALL_PARAM_INFO_RESP;
  hif_buf[indx++] = fan_nwk_manager_app.node_basic_cfg.operational_mode;
  mem_rev_cpy (&hif_buf[indx],(uint8_t *)&fan_nwk_manager_app.factory_mod_st.serial_baudrate, 4);
  indx += 4;
  memcpy (&hif_buf[indx], fan_nwk_manager_app.node_basic_cfg.self_ieee_addr, 8);
  indx += 8;
  hif_buf[indx++] = fan_nwk_manager_app.factory_mod_st.rssi_threshold;
  hif_buf[indx++] = fan_nwk_manager_app.factory_mod_st.xtal_adjust;
  hif_buf[indx++] = 0;  /* Always 2FSK */
  
  if (fan_nwk_manager_app.node_basic_cfg.sybbol_rate == 20)
    symbol_rate = 50000;
  if (fan_nwk_manager_app.node_basic_cfg.sybbol_rate == 10)
    symbol_rate = 100000;
  if (fan_nwk_manager_app.node_basic_cfg.sybbol_rate == 7)
    symbol_rate = 150000;
  if (fan_nwk_manager_app.node_basic_cfg.sybbol_rate == 5)
    symbol_rate = 200000;
  if (fan_nwk_manager_app.node_basic_cfg.sybbol_rate == 3)
    symbol_rate = 300000;
  mem_rev_cpy (&hif_buf[indx], (uint8_t *)&symbol_rate, sizeof(symbol_rate));
  indx += sizeof(symbol_rate);
  
  if (fan_nwk_manager_app.node_basic_cfg.modulation_index == (float)0.5)
      modulation_index = 0;
  if (fan_nwk_manager_app.node_basic_cfg.modulation_index == (float)1.0)
      modulation_index = 1;
  hif_buf[indx++] = modulation_index;
  
  hif_buf[indx++] = fan_nwk_manager_app.node_basic_cfg.pa_level;
  if (fan_nwk_manager_app.node_basic_cfg.us_ie.us_schedule.channel_plan == 0x01)
  {
    hif_buf[indx++] = 0;
    hif_buf[indx++] = 0;
  }
  else
  {
    hif_buf[indx++] = fan_nwk_manager_app.node_basic_cfg.us_ie.un_channel_plan.ch_reg_op.reg_domain;
    hif_buf[indx++] = fan_nwk_manager_app.node_basic_cfg.us_ie.un_channel_plan.ch_reg_op.op_class;
  }
  mem_rev_cpy (&hif_buf[indx], (uint8_t *)&fan_nwk_manager_app.node_basic_cfg.selected_channel, 2);
  indx += 2;
  
  /* FAN related Configuration */
  if (fan_nwk_manager_app.node_basic_cfg.us_ie.us_schedule.channel_plan == 0x00)
  {
    mem_rev_cpy (&hif_buf[indx], zero_buf, 4);
    indx += 4;
    hif_buf[indx++] = zero_buf[0];
    mem_rev_cpy (&hif_buf[indx], zero_buf, 2);
    indx += 2;
  }
  else
  {
    mem_rev_cpy (&hif_buf[indx], (uint8_t *)&fan_nwk_manager_app.node_basic_cfg.us_ie.un_channel_plan.ch_explicit.ch0, 4);
    indx += 4;
    hif_buf[indx++] = fan_nwk_manager_app.node_basic_cfg.us_ie.un_channel_plan.ch_explicit.channel_spacing;
    mem_rev_cpy (&hif_buf[indx], (uint8_t *)&fan_nwk_manager_app.node_basic_cfg.us_ie.un_channel_plan.ch_explicit.num_chans, 2);
    indx += 2;
  }
  hif_buf[indx++] = fan_nwk_manager_app.node_basic_cfg.us_ie.us_schedule.dwell_interval;
  hif_buf[indx++] = fan_nwk_manager_app.node_basic_cfg.us_ie.us_schedule.channel_function;
  hif_buf[indx++] = sizeof(excluded_chan_range_info_t);
  memcpy (&hif_buf[indx], (uint8_t *)&fan_nwk_manager_app.node_basic_cfg.us_ie.us_schedule.excluded_channels.excluded_channel_ranges, sizeof(excluded_chan_range_info_t));
  indx += sizeof(excluded_chan_range_info_t);
  memcpy (&hif_buf[indx], fan_nwk_manager_app.node_basic_cfg.us_ie.us_schedule.excluded_channels.excluded_channel_mask, 16);
  indx += 16;
  mem_rev_cpy (&hif_buf[indx], (uint8_t *)&fan_nwk_manager_app.node_basic_cfg.bs_ie.bcast_interval, 4);
  indx += 4;
  mem_rev_cpy (&hif_buf[indx], (uint8_t *)&fan_nwk_manager_app.node_basic_cfg.bs_ie.bcast_sched_id, 2);
  indx += 2;
  hif_buf[indx++] = fan_nwk_manager_app.node_basic_cfg.bs_ie.bs_schedule.dwell_interval;
  hif_buf[indx++] = fan_nwk_manager_app.node_basic_cfg.bs_ie.bs_schedule.channel_function;
  hif_buf[indx++] = sizeof(excluded_chan_range_info_t);
  memcpy (&hif_buf[indx], (uint8_t *)&fan_nwk_manager_app.node_basic_cfg.bs_ie.bs_schedule.excluded_channels.excluded_channel_ranges, sizeof(excluded_chan_range_info_t));
  indx += sizeof(excluded_chan_range_info_t);
  memcpy (&hif_buf[indx], fan_nwk_manager_app.node_basic_cfg.bs_ie.bs_schedule.excluded_channels.excluded_channel_mask, 16);
  indx += 16;
  mem_rev_cpy (&hif_buf[indx], (uint8_t *)&fan_nwk_manager_app.node_basic_cfg.pan_ie.pan_id, 2);
  indx += 2;
  mem_rev_cpy (&hif_buf[indx], (uint8_t *)&fan_nwk_manager_app.node_basic_cfg.pan_ie.pan_size, 2);
  indx += 2;
  hif_buf[indx++] = fan_nwk_manager_app.node_basic_cfg.pan_ie.parent_bs_ie_use;
  hif_buf[indx++] = fan_nwk_manager_app.node_basic_cfg.pan_ie.routing_methood;
  hif_buf[indx++] = fan_nwk_manager_app.node_basic_cfg.netname_ie.length;
  memcpy (&hif_buf[indx], &fan_nwk_manager_app.node_basic_cfg.netname_ie.network_name, fan_nwk_manager_app.node_basic_cfg.netname_ie.length);
  indx += fan_nwk_manager_app.node_basic_cfg.netname_ie.length;
  memcpy (&hif_buf[indx], fan_nwk_manager_app.node_basic_cfg.gtkhash_ie.gtk0_key, 16);
  indx += 16;
  memcpy (&hif_buf[indx], fan_nwk_manager_app.node_basic_cfg.gtkhash_ie.gtk1_key, 16);
  indx += 16;
  memcpy (&hif_buf[indx], fan_nwk_manager_app.node_basic_cfg.gtkhash_ie.gtk2_key, 16);
  indx += 16;
  memcpy (&hif_buf[indx], fan_nwk_manager_app.node_basic_cfg.gtkhash_ie.gtk3_key, 16);
  indx += 16;
  hif_send_msg_up (hif_buf, indx, response_laye_ID,PROTOCOL_ID_FOR_APP);
}

/*============================================================================*/
static void set_phy_mode_using_reg_dom_oprating_class_usi()
{
    uint8_t temp_phy_mode = 0;
    if(fan_nwk_manager_app.node_basic_cfg.us_ie.un_channel_plan.ch_reg_op.reg_domain == 0x01)
    {
      if(fan_nwk_manager_app.node_basic_cfg.us_ie.un_channel_plan.ch_reg_op.op_class == 0x01)
      {
        
            if((fan_nwk_manager_app.node_basic_cfg.sybbol_rate == 0x14) && (fan_nwk_manager_app.node_basic_cfg.modulation_index == 1))
            {
               temp_phy_mode = 0x00;//ChangeEndianness(set_param_nvm.phy_mode);                
               fan_nwk_manager_app.node_basic_cfg.phy_mode = phyModeMapArr[temp_phy_mode]; 
            } 
            if((fan_nwk_manager_app.node_basic_cfg.sybbol_rate == 0x0A) && (fan_nwk_manager_app.node_basic_cfg.modulation_index == 0.5))
            {
               temp_phy_mode = 0x01;//ChangeEndianness(set_param_nvm.phy_mode);                
               fan_nwk_manager_app.node_basic_cfg.phy_mode = phyModeMapArr[temp_phy_mode]; 
            }
      }
      if(fan_nwk_manager_app.node_basic_cfg.us_ie.un_channel_plan.ch_reg_op.op_class == 0x02)
      {
            if((fan_nwk_manager_app.node_basic_cfg.sybbol_rate == 7) && (fan_nwk_manager_app.node_basic_cfg.modulation_index == 0.5))
            {
                temp_phy_mode = 0x02;//ChangeEndianness(set_param_nvm.phy_mode);                
                fan_nwk_manager_app.node_basic_cfg.phy_mode = phyModeMapArr[temp_phy_mode];
            }
            if((fan_nwk_manager_app.node_basic_cfg.sybbol_rate == 5) && (fan_nwk_manager_app.node_basic_cfg.modulation_index == 0.5))
            {
                temp_phy_mode = 0x03;//ChangeEndianness(set_param_nvm.phy_mode);                
                fan_nwk_manager_app.node_basic_cfg.phy_mode = phyModeMapArr[temp_phy_mode];
            }
      
      }
      if(fan_nwk_manager_app.node_basic_cfg.us_ie.un_channel_plan.ch_reg_op.op_class == 0x03)
      {
           if((fan_nwk_manager_app.node_basic_cfg.sybbol_rate == 3) && (fan_nwk_manager_app.node_basic_cfg.modulation_index == 0.5))
            {
                 temp_phy_mode = 0x04;//ChangeEndianness(set_param_nvm.phy_mode);                
                 fan_nwk_manager_app.node_basic_cfg.phy_mode = phyModeMapArr[temp_phy_mode];
            }
      }
    }
//    store_nvm_param.phy_mode = fan_nwk_manager_app.node_basic_cfg.phy_mode;
}

/*============================================================================*/
static void set_phy_mode_using_explicit()
{
  uint8_t temp_phy_mode = 0;
  if(fan_nwk_manager_app.node_basic_cfg.us_ie.us_schedule.channel_plan == 0x01)
  {
    if(fan_nwk_manager_app.node_basic_cfg.us_ie.un_channel_plan.ch_explicit.channel_spacing == 0x00)
    {
      if((fan_nwk_manager_app.node_basic_cfg.sybbol_rate == 0x14) && (fan_nwk_manager_app.node_basic_cfg.modulation_index == 1))
      {
        temp_phy_mode = 0x00;//ChangeEndianness(set_param_nvm.phy_mode);                
        fan_nwk_manager_app.node_basic_cfg.phy_mode = phyModeMapArr[temp_phy_mode]; 
      } 
      if((fan_nwk_manager_app.node_basic_cfg.sybbol_rate == 0x0A) && (fan_nwk_manager_app.node_basic_cfg.modulation_index == 0.5))
      {
        temp_phy_mode = 0x01;//ChangeEndianness(set_param_nvm.phy_mode);                
        fan_nwk_manager_app.node_basic_cfg.phy_mode = phyModeMapArr[temp_phy_mode]; 
      }
    }
    if(fan_nwk_manager_app.node_basic_cfg.us_ie.un_channel_plan.ch_explicit.channel_spacing == 0x01)
    {
      if((fan_nwk_manager_app.node_basic_cfg.sybbol_rate == 7) && (fan_nwk_manager_app.node_basic_cfg.modulation_index == 0.5))
      {
        temp_phy_mode = 0x02;//ChangeEndianness(set_param_nvm.phy_mode);                
        fan_nwk_manager_app.node_basic_cfg.phy_mode = phyModeMapArr[temp_phy_mode];
      }
      if((fan_nwk_manager_app.node_basic_cfg.sybbol_rate == 5) && (fan_nwk_manager_app.node_basic_cfg.modulation_index == 0.5))
      {
        temp_phy_mode = 0x03;//ChangeEndianness(set_param_nvm.phy_mode);                
        fan_nwk_manager_app.node_basic_cfg.phy_mode = phyModeMapArr[temp_phy_mode];
      }
    }
    if(fan_nwk_manager_app.node_basic_cfg.us_ie.un_channel_plan.ch_explicit.channel_spacing == 0x02)
    {   
      if((fan_nwk_manager_app.node_basic_cfg.sybbol_rate == 3) && (fan_nwk_manager_app.node_basic_cfg.modulation_index == 0.5))
      {
        temp_phy_mode = 0x04;//ChangeEndianness(set_param_nvm.phy_mode);                
        fan_nwk_manager_app.node_basic_cfg.phy_mode = phyModeMapArr[temp_phy_mode];
      }
    }
  }
  
}


/*============================================================================*/
//static void set_phy_mode_using_reg_dom_oprating_class_bsi()
//{
//    uint8_t temp_phy_mode = 0;
//    if(fan_nwk_manager_app.node_basic_cfg.bs_ie.un_channel_plan.ch_reg_op.reg_domain == 0x01)
//    {
//      if(fan_nwk_manager_app.node_basic_cfg.bs_ie.un_channel_plan.ch_reg_op.op_class == 0x01)
//      {
//        
//            if((fan_nwk_manager_app.node_basic_cfg.sybbol_rate == 0x14) && (fan_nwk_manager_app.node_basic_cfg.modulation_index == 1))
//            {
//               temp_phy_mode = 0x00;//ChangeEndianness(set_param_nvm.phy_mode);                
//               fan_nwk_manager_app.node_basic_cfg.phy_mode = phyModeMapArr[temp_phy_mode]; 
//            } 
//            if((fan_nwk_manager_app.node_basic_cfg.sybbol_rate == 0x0A) && (fan_nwk_manager_app.node_basic_cfg.modulation_index == 0.5))
//            {
//               temp_phy_mode = 0x01;//ChangeEndianness(set_param_nvm.phy_mode);                
//               fan_nwk_manager_app.node_basic_cfg.phy_mode = phyModeMapArr[temp_phy_mode]; 
//            }
//      }
//      if(fan_nwk_manager_app.node_basic_cfg.bs_ie.un_channel_plan.ch_reg_op.op_class == 0x02)
//      {
//            if((fan_nwk_manager_app.node_basic_cfg.sybbol_rate == 7) && (fan_nwk_manager_app.node_basic_cfg.modulation_index == 0.5))
//            {
//                temp_phy_mode = 0x02;//ChangeEndianness(set_param_nvm.phy_mode);                
//                fan_nwk_manager_app.node_basic_cfg.phy_mode = phyModeMapArr[temp_phy_mode];
//            }
//            if((fan_nwk_manager_app.node_basic_cfg.sybbol_rate == 5) && (fan_nwk_manager_app.node_basic_cfg.modulation_index == 0.5))
//            {
//                temp_phy_mode = 0x03;//ChangeEndianness(set_param_nvm.phy_mode);                
//                fan_nwk_manager_app.node_basic_cfg.phy_mode = phyModeMapArr[temp_phy_mode];
//            }
//      
//      }
//      if(fan_nwk_manager_app.node_basic_cfg.bs_ie.un_channel_plan.ch_reg_op.op_class == 0x03)
//      {
//           if((fan_nwk_manager_app.node_basic_cfg.sybbol_rate == 3) && (fan_nwk_manager_app.node_basic_cfg.modulation_index == 0.5))
//            {
//                 temp_phy_mode = 0x04;//ChangeEndianness(set_param_nvm.phy_mode);                
//                 fan_nwk_manager_app.node_basic_cfg.phy_mode = phyModeMapArr[temp_phy_mode];
//            }
//      }
//    }
//}
/*============================================================================*/

uint8_t* get_self_address(void)
{
  mem_rev_cpy(rev_self_addr, fan_nwk_manager_app.node_basic_cfg.self_ieee_addr, 8);
  return rev_self_addr;
}
/*----------------------------------------------------------------------------*/
uint8_t get_fan_device_type(void)
{
  return fan_nwk_manager_app.node_basic_cfg.fan_device_type;
}
/******************************************************************************/
#if(APP_NVM_FEATURE_ENABLED == 1)
void update_parameter_from_nvm (nvm_structure_t store_nvm_param)
{
  //fan_nwk_manager_app.node_basic_cfg.operatinng_country = store_nvm_param.operatinng_country;
  fan_nwk_manager_app.node_basic_cfg.board_reset = store_nvm_param.board_reset;
  fan_nwk_manager_app.node_basic_cfg.operational_mode = store_nvm_param.operational_mode;
  fan_nwk_manager_app.node_basic_cfg.modulation_index = store_nvm_param.modulation_index;
  fan_nwk_manager_app.node_basic_cfg.pa_level = store_nvm_param.pa_level;
  fan_nwk_manager_app.node_basic_cfg.fan_device_type = store_nvm_param.node_type;
  fan_nwk_manager_app.node_basic_cfg.phy_mode  = store_nvm_param.phy_mode;
  fan_nwk_manager_app.node_basic_cfg.selected_channel = store_nvm_param.selected_channel;
  fan_nwk_manager_app.node_basic_cfg.sybbol_rate = store_nvm_param.sybbol_rate;
  memcpy(fan_nwk_manager_app.node_basic_cfg.self_ieee_addr,store_nvm_param.self_ieee_addr,8);
  fan_nwk_manager_app.node_basic_cfg.us_ie.un_channel_plan.ch_reg_op.reg_domain = store_nvm_param.reg_domain;
  fan_nwk_manager_app.node_basic_cfg.us_ie.un_channel_plan.ch_reg_op.op_class = store_nvm_param.oprating_class;
  fan_nwk_manager_app.node_basic_cfg.us_ie.channel_fixed.fixed_chan = fan_nwk_manager_app.node_basic_cfg.selected_channel;
  fan_nwk_manager_app.node_basic_cfg.bs_ie.channel_fixed.fixed_chan = fan_nwk_manager_app.node_basic_cfg.selected_channel;
  fan_nwk_manager_app.factory_mod_st.rssi_threshold = store_nvm_param.rssi_threshold;
  fan_nwk_manager_app.factory_mod_st.xtal_adjust = store_nvm_param.xtal_adjust;
  fan_nwk_manager_app.factory_mod_st.serial_baudrate = store_nvm_param.serial_baudrate;
//  if((fan_nwk_manager_app.node_basic_cfg.pa_level <= 0x7F ) && (fan_nwk_manager_app.node_basic_cfg.pa_level >20))
//  TRX_Set_PA_Level( fan_nwk_manager_app.node_basic_cfg.pa_level );
  
  fan_nwk_manager_app.node_basic_cfg.us_ie.un_channel_plan.ch_explicit.channel_spacing = store_nvm_param.fan_config_param.channel_spacing;
  fan_nwk_manager_app.node_basic_cfg.bs_ie.un_channel_plan.ch_explicit.channel_spacing = store_nvm_param.fan_config_param.channel_spacing;
  fan_nwk_manager_app.node_basic_cfg.us_ie.us_schedule.dwell_interval = store_nvm_param.fan_config_param.unicast_dwell_interval;
  fan_nwk_manager_app.node_basic_cfg.us_ie.us_schedule.channel_function = store_nvm_param.fan_config_param.unicast_channel_function;
  fan_nwk_manager_app.node_basic_cfg.bs_ie.bs_schedule.dwell_interval = store_nvm_param.fan_config_param.bcast_dwell_interval;
  fan_nwk_manager_app.node_basic_cfg.bs_ie.bs_schedule.channel_function = store_nvm_param.fan_config_param.bcast_channel_function;
  fan_nwk_manager_app.node_basic_cfg.pan_ie.parent_bs_ie_use = store_nvm_param.fan_config_param.parent_bs_ie_use;
  fan_nwk_manager_app.node_basic_cfg.pan_ie.routing_methood = store_nvm_param.fan_config_param.routing_methood;
  fan_nwk_manager_app.node_basic_cfg.netname_ie.length = store_nvm_param.fan_config_param.network_name_length;
  fan_nwk_manager_app.node_basic_cfg.us_ie.un_channel_plan.ch_explicit.num_chans = store_nvm_param.fan_config_param.num_chans;
  fan_nwk_manager_app.node_basic_cfg.bs_ie.un_channel_plan.ch_explicit.num_chans = store_nvm_param.fan_config_param.num_chans;
  fan_nwk_manager_app.node_basic_cfg.bs_ie.bcast_sched_id = store_nvm_param.fan_config_param.bcast_sched_id;
  fan_nwk_manager_app.node_basic_cfg.pan_ie.pan_id = store_nvm_param.fan_config_param.pan_id;
  fan_nwk_manager_app.node_basic_cfg.pan_ie.pan_size = store_nvm_param.fan_config_param.pan_size;
  fan_nwk_manager_app.node_basic_cfg.us_ie.un_channel_plan.ch_explicit.ch0 = store_nvm_param.fan_config_param.ch0;
  fan_nwk_manager_app.node_basic_cfg.bs_ie.un_channel_plan.ch_explicit.ch0 = store_nvm_param.fan_config_param.ch0;
  fan_nwk_manager_app.node_basic_cfg.bs_ie.bcast_interval = store_nvm_param.fan_config_param.bcast_interval;
  memcpy (fan_nwk_manager_app.node_basic_cfg.netname_ie.network_name, store_nvm_param.fan_config_param.network_name, store_nvm_param.fan_config_param.network_name_length);
#if (EFR32FG13P_LBR == 0x00)  
  if(fan_nwk_manager_app.nvm_write_to_start ==true)  
    memcpy(fan_nwk_manager_app.node_basic_cfg.self_global_addr,store_nvm_param.self_global_address,16);
#endif  
/* Debdeep :: If we fetch GTKs from NVM, then number of installed GTKs can not be changed from TBC */
//  memcpy (fan_nwk_manager_app.node_basic_cfg.gtkhash_ie.gtk0_key, store_nvm_param.fan_config_param.gtk0, 16);
//  memcpy (fan_nwk_manager_app.node_basic_cfg.gtkhash_ie.gtk1_key, store_nvm_param.fan_config_param.gtk1, 16);
//  memcpy (fan_nwk_manager_app.node_basic_cfg.gtkhash_ie.gtk2_key, store_nvm_param.fan_config_param.gtk2, 16);
//  memcpy (fan_nwk_manager_app.node_basic_cfg.gtkhash_ie.gtk3_key, store_nvm_param.fan_config_param.gtk3, 16);
  memcpy ((uint8_t *)&fan_nwk_manager_app.node_basic_cfg.us_ie.us_schedule.excluded_channels, (uint8_t *)&store_nvm_param.fan_config_param.unicast_excluded_channels, sizeof (store_nvm_param.fan_config_param.unicast_excluded_channels));
  memcpy ((uint8_t *)&fan_nwk_manager_app.node_basic_cfg.bs_ie.bs_schedule.excluded_channels, (uint8_t *)&store_nvm_param.fan_config_param.bcast_excluded_channels, sizeof (store_nvm_param.fan_config_param.bcast_excluded_channels));
//  fan_nwk_manager_app.node_basic_cfg.gtkhash_ie.gtkl = store_nvm_param.fan_config_param.gtkl;
  fan_nwk_manager_app.node_basic_cfg.us_ie.us_schedule.channel_plan = store_nvm_param.fan_config_param.channel_plan;
}

/*============================================================================*/

void update_parameter_to_nvm (nvm_structure_t *store_nvm_param)
{
  store_nvm_param->board_reset = fan_nwk_manager_app.node_basic_cfg.board_reset;
  store_nvm_param->operational_mode = fan_nwk_manager_app.node_basic_cfg.operational_mode;
  store_nvm_param->modulation_index = fan_nwk_manager_app.node_basic_cfg.modulation_index;
  store_nvm_param->pa_level = fan_nwk_manager_app.node_basic_cfg.pa_level;
  store_nvm_param->node_type = fan_nwk_manager_app.node_basic_cfg.fan_device_type;
  store_nvm_param->phy_mode = fan_nwk_manager_app.node_basic_cfg.phy_mode;
  store_nvm_param->selected_channel = fan_nwk_manager_app.node_basic_cfg.selected_channel;
  store_nvm_param->sybbol_rate = fan_nwk_manager_app.node_basic_cfg.sybbol_rate;
  memcpy(store_nvm_param->self_ieee_addr, fan_nwk_manager_app.node_basic_cfg.self_ieee_addr, 8);
  store_nvm_param->reg_domain = fan_nwk_manager_app.node_basic_cfg.us_ie.un_channel_plan.ch_reg_op.reg_domain;
  store_nvm_param->oprating_class = fan_nwk_manager_app.node_basic_cfg.us_ie.un_channel_plan.ch_reg_op.op_class;
  store_nvm_param->operatinng_country = fan_nwk_manager_app.node_basic_cfg.operatinng_country;
  store_nvm_param->rssi_threshold = fan_nwk_manager_app.factory_mod_st.rssi_threshold;
  store_nvm_param->xtal_adjust = fan_nwk_manager_app.factory_mod_st.xtal_adjust;
  store_nvm_param->serial_baudrate = fan_nwk_manager_app.factory_mod_st.serial_baudrate;
  
  store_nvm_param->fan_config_param.channel_spacing = fan_nwk_manager_app.node_basic_cfg.us_ie.un_channel_plan.ch_explicit.channel_spacing;
  store_nvm_param->fan_config_param.unicast_dwell_interval = fan_nwk_manager_app.node_basic_cfg.us_ie.us_schedule.dwell_interval;
  store_nvm_param->fan_config_param.unicast_channel_function = fan_nwk_manager_app.node_basic_cfg.us_ie.us_schedule.channel_function;
  store_nvm_param->fan_config_param.bcast_dwell_interval = fan_nwk_manager_app.node_basic_cfg.bs_ie.bs_schedule.dwell_interval;
  store_nvm_param->fan_config_param.bcast_channel_function = fan_nwk_manager_app.node_basic_cfg.bs_ie.bs_schedule.channel_function;
  store_nvm_param->fan_config_param.parent_bs_ie_use = fan_nwk_manager_app.node_basic_cfg.pan_ie.parent_bs_ie_use;
  store_nvm_param->fan_config_param.routing_methood = fan_nwk_manager_app.node_basic_cfg.pan_ie.routing_methood;
  store_nvm_param->fan_config_param.network_name_length = fan_nwk_manager_app.node_basic_cfg.netname_ie.length;
  store_nvm_param->fan_config_param.num_chans = fan_nwk_manager_app.node_basic_cfg.us_ie.un_channel_plan.ch_explicit.num_chans;
  store_nvm_param->fan_config_param.bcast_sched_id = fan_nwk_manager_app.node_basic_cfg.bs_ie.bcast_sched_id;
  store_nvm_param->fan_config_param.pan_id = fan_nwk_manager_app.node_basic_cfg.pan_ie.pan_id;
  store_nvm_param->fan_config_param.pan_size = fan_nwk_manager_app.node_basic_cfg.pan_ie.pan_size;
  store_nvm_param->fan_config_param.ch0 = fan_nwk_manager_app.node_basic_cfg.us_ie.un_channel_plan.ch_explicit.ch0;
  store_nvm_param->fan_config_param.bcast_interval = fan_nwk_manager_app.node_basic_cfg.bs_ie.bcast_interval;
  memcpy (store_nvm_param->fan_config_param.network_name, fan_nwk_manager_app.node_basic_cfg.netname_ie.network_name, store_nvm_param->fan_config_param.network_name_length);
#if (EFR32FG13P_LBR == 0x00)   
  if(fan_nwk_manager_app.nvm_write_to_start ==true)    
    memcpy(store_nvm_param->self_global_address,fan_nwk_manager_app.node_basic_cfg.self_global_addr,16);
#endif  
/* Debdeep :: If we fetch GTKs from NVM, then number of installed GTKs can not be changed from TBC */
//  memcpy (store_nvm_param->fan_config_param.gtk0, fan_nwk_manager_app.node_basic_cfg.gtkhash_ie.gtk0_key, 16);
//  memcpy (store_nvm_param->fan_config_param.gtk1, fan_nwk_manager_app.node_basic_cfg.gtkhash_ie.gtk1_key, 16);
//  memcpy (store_nvm_param->fan_config_param.gtk2, fan_nwk_manager_app.node_basic_cfg.gtkhash_ie.gtk2_key, 16);
//  memcpy (store_nvm_param->fan_config_param.gtk3, fan_nwk_manager_app.node_basic_cfg.gtkhash_ie.gtk3_key, 16);
  memcpy ((uint8_t *)&store_nvm_param->fan_config_param.unicast_excluded_channels, (uint8_t *)&fan_nwk_manager_app.node_basic_cfg.us_ie.us_schedule.excluded_channels, sizeof (store_nvm_param->fan_config_param.unicast_excluded_channels));
  memcpy ((uint8_t *)&store_nvm_param->fan_config_param.bcast_excluded_channels, (uint8_t *)&fan_nwk_manager_app.node_basic_cfg.bs_ie.bs_schedule.excluded_channels, sizeof (store_nvm_param->fan_config_param.bcast_excluded_channels));
//  store_nvm_param->fan_config_param.gtkl = fan_nwk_manager_app.node_basic_cfg.gtkhash_ie.gtkl;
  store_nvm_param->fan_config_param.channel_plan = fan_nwk_manager_app.node_basic_cfg.us_ie.us_schedule.channel_plan;
}


#endif // #if(APP_NVM_FEATURE_ENABLED == 1)

/*============================================================================*/

/*----------------------------------------------------------------------------*/

void process_set_net_name_ie(uint8_t *buf, uint16_t length)
{
   memcpy((uint8_t *)fan_nwk_manager_app.node_basic_cfg.netname_ie.network_name,&buf[0],length);   
   fan_nwk_manager_app.node_basic_cfg.netname_ie.length=length;
   
#if(AUTO_CONFIG_ENABLE == 0)
   send_hif_conf_cb(FAN_NETNAME_IE_SET_CONFIRM,0x00);
   gu2pcapp_node_config_set_conf(0x00);
#endif
}

/*============================================================================*/

/*----------------------------------------------------------------------------*/
void process_set_panversion_ie(uint8_t *buf, uint16_t length)
{
 mem_rev_cpy((uint8_t *)&fan_nwk_manager_app.node_basic_cfg.panvar_ie.PANVERSION,&buf[0],2);
 
}


/*----------------------------------------------------------------------------*/

void process_set_gtk_hash_ie(uint8_t *buf, uint16_t length)
{
  
  
  fan_nwk_manager_app.node_basic_cfg.gtkhash_ie.gtkl = *buf++;
  memcpy((uint8_t *)fan_nwk_manager_app.node_basic_cfg.gtkhash_ie.gtk0_key,buf,16);
  buf += 16;
  memcpy((uint8_t *)fan_nwk_manager_app.node_basic_cfg.gtkhash_ie.gtk1_key,buf,16);
  buf += 16;
  memcpy((uint8_t *)fan_nwk_manager_app.node_basic_cfg.gtkhash_ie.gtk2_key,buf,16);
  buf += 16;
  memcpy((uint8_t *)fan_nwk_manager_app.node_basic_cfg.gtkhash_ie.gtk3_key,buf,16);
  if (get_join_state () > 0)
  {

    FAN_MAC_MLME_SET_Request
      (
       WISUN_INFO_PAYLOAD_IE_ID,/* header ie or payload ie */
       WISUN_IE_SUBID_GTKHASH_IE,/* subid for each ie */	        
       sizeof(gtk_key_t),/*(65+1)=(fan_nwk_manager_app.node_basic_cfg.gtkhash_ie.length+1),*/
       (uint8_t *)& fan_nwk_manager_app.node_basic_cfg.gtkhash_ie
         ); 
    
    uint8_t temp_gtk_hash_buf[65] = {0};
    uint8_t *temp_ptr = &temp_gtk_hash_buf[0];
    *temp_ptr++ = fan_nwk_manager_app.node_basic_cfg.gtkhash_ie.gtkl;
    memcpy(temp_ptr,(uint8_t *)fan_nwk_manager_app.node_basic_cfg.gtkhash_ie.gtk0_key,16);
    temp_ptr += 16;
    memcpy(temp_ptr,(uint8_t *)fan_nwk_manager_app.node_basic_cfg.gtkhash_ie.gtk1_key,16);
    temp_ptr += 16;
    memcpy(temp_ptr,(uint8_t *)fan_nwk_manager_app.node_basic_cfg.gtkhash_ie.gtk2_key,16);
    temp_ptr += 16;
    memcpy(temp_ptr,(uint8_t *)fan_nwk_manager_app.node_basic_cfg.gtkhash_ie.gtk3_key,16);
    temp_ptr += 16;
    send_gtkhash_to_hostapd (temp_gtk_hash_buf, (temp_ptr - temp_gtk_hash_buf));
  }
}
/*============================================================================*/

/*----------------------------------------------------------------------------*/
void process_set_pan_ie(uint8_t *buf, uint16_t length)
{
  mem_rev_cpy((uint8_t *)& fan_nwk_manager_app.node_basic_cfg.pan_ie.pan_size,buf,2);
  buf+=2;
  mem_rev_cpy((uint8_t *)& fan_nwk_manager_app.node_basic_cfg.pan_ie.routing_cost,buf,2);
  buf+=2;
  
  fan_nwk_manager_app.node_basic_cfg.pan_ie.parent_bs_ie_use = *buf++;
  fan_nwk_manager_app.node_basic_cfg.pan_ie.routing_methood = *buf++;
  fan_nwk_manager_app.node_basic_cfg.pan_ie.fan_tps_version = *buf++; 

#if(AUTO_CONFIG_ENABLE == 0)
  fan_nwk_manager_app.node_basic_cfg.pan_ie.length = length;                                  
  send_hif_conf_cb(FAN_PAN_IE_SET_CONFIRM,0x00);
#endif
}


/*----------------------------------------------------------------------------*/
void process_broadcast_ie_set_req(uint8_t *buf, uint16_t length)
{
  //  uint8_t channel_plan=0;  
  uint8_t total_ex_channel = 0;
  int index = 0;
  int i = 0;
  mem_rev_cpy((uint8_t *)& fan_nwk_manager_app.node_basic_cfg.bs_ie.bcast_interval,&buf[index],4);
  index+=4;
  mem_rev_cpy((uint8_t *)& fan_nwk_manager_app.node_basic_cfg.bs_ie.bcast_sched_id,&buf[index],2);
  index+=2;
  fan_nwk_manager_app.node_basic_cfg.bs_ie.bs_schedule.dwell_interval=buf[index++];
  fan_nwk_manager_app.node_basic_cfg.bs_ie.bs_schedule.clock_drift=buf[index++];
  fan_nwk_manager_app.node_basic_cfg.bs_ie.bs_schedule.timing_accuracy=buf[index++]; 
  fan_nwk_manager_app.node_basic_cfg.bs_ie.bs_schedule.channel_plan= buf[index++];
  fan_nwk_manager_app.node_basic_cfg.bs_ie.bs_schedule.channel_function = buf[index++];
  fan_nwk_manager_app.node_basic_cfg.bs_ie.bs_schedule.excludded_channel_control = buf[index++];
  fan_nwk_manager_app.node_basic_cfg.bs_ie.bs_schedule.chan_hop_count = buf[index++];
  fan_nwk_manager_app.node_basic_cfg.bs_ie.bs_schedule.length = buf[index++];
  
  index+=4;            //Arjun: Incrementing pointer by 100 to bypass chan_hop_list
  
  if(fan_nwk_manager_app.node_basic_cfg.bs_ie.bs_schedule.excludded_channel_control == EXCLUDED_CHANNEL_PRESENT)                      //Arjun: checking excludded_channel_control value to deceide whether this uninon contains ex_channel_mask/range
  {
    fan_nwk_manager_app.node_basic_cfg.bs_ie.bs_schedule.excluded_channels.excluded_channel_ranges.num_of_ranges = buf[index++];
    total_ex_channel = fan_nwk_manager_app.node_basic_cfg.bs_ie.bs_schedule.excluded_channels.excluded_channel_ranges.num_of_ranges;
    if(total_ex_channel <= MAXIMUM_EXCLUDE_CHANNEL_SUPPORT)
    {
      for(i = 0; i < total_ex_channel; i++)
      {
        memcpy(&fan_nwk_manager_app.node_basic_cfg.bs_ie.bs_schedule.excluded_channels.excluded_channel_ranges.ex_ch_range[i].start_ch, &buf[index], 2);
        index+=2;
        memcpy(&fan_nwk_manager_app.node_basic_cfg.bs_ie.bs_schedule.excluded_channels.excluded_channel_ranges.ex_ch_range[i].end_ch, &buf[index], 2);
        index+=2;
      }
    }
    uint8_t incPointer = (MAX_EXCLUDED_CHANNEL_RANGES_SUPPORTED-total_ex_channel)*4;
    incPointer += 11;
    index+=incPointer;
  }
  else if(fan_nwk_manager_app.node_basic_cfg.bs_ie.bs_schedule.excludded_channel_control == EXCLUDED_CHANNEL_MASK_PRESENT)
  {
    memcpy(fan_nwk_manager_app.node_basic_cfg.bs_ie.bs_schedule.excluded_channels.excluded_channel_mask,&buf[index],17);
    index+=32;
  }
  else
  {
    index+=32;
  }
  
  index+=4; //Arjun: bypassing the local_time_node;
  
  if(fan_nwk_manager_app.node_basic_cfg.bs_ie.bs_schedule.channel_plan == CH_REG_OP_PRESENT)                            //Arjun: checking whether reg_op is present
  {
    fan_nwk_manager_app.node_basic_cfg.bs_ie.un_channel_plan.ch_reg_op.reg_domain = buf[index++];
    fan_nwk_manager_app.node_basic_cfg.bs_ie.un_channel_plan.ch_reg_op.op_class = buf[index++];
    index+=5;
  }
  else if(fan_nwk_manager_app.node_basic_cfg.bs_ie.bs_schedule.channel_plan == CH_EXPLICIT_PRESENT)
  {
    memcpy((uint8_t *)& fan_nwk_manager_app.node_basic_cfg.bs_ie.un_channel_plan.ch_explicit.ch0,&buf[index],4);
    index+=4;
    fan_nwk_manager_app.node_basic_cfg.bs_ie.un_channel_plan.ch_explicit.channel_spacing = buf[index++];
    mem_rev_cpy((uint8_t *)& fan_nwk_manager_app.node_basic_cfg.bs_ie.un_channel_plan.ch_explicit.num_chans,&buf[index],2);
    index+=2;
  }
  mem_rev_cpy((uint8_t *)& fan_nwk_manager_app.node_basic_cfg.bs_ie.channel_fixed.fixed_chan,&buf[index],2);
  index+=2;
   //send_hif_conf_cb(FAN_BROADCAST_IE_SET_CONFIRM,0x00);
}



/*----------------------------------------------------------------------------*/
void process_unicast_ie_set_req(uint8_t *buf, uint16_t length)
{
    uint8_t total_ex_channel = 0;
    int index = 0;
    int i = 0;
    fan_nwk_manager_app.node_basic_cfg.us_ie.us_schedule.dwell_interval=buf[index++];
    fan_nwk_manager_app.node_basic_cfg.us_ie.us_schedule.clock_drift=buf[index++];
    fan_nwk_manager_app.node_basic_cfg.us_ie.us_schedule.timing_accuracy=buf[index++]; 
    fan_nwk_manager_app.node_basic_cfg.us_ie.us_schedule.channel_plan=buf[index++];
    fan_nwk_manager_app.node_basic_cfg.us_ie.us_schedule.channel_function = buf[index++];
    fan_nwk_manager_app.node_basic_cfg.us_ie.us_schedule.excludded_channel_control = buf[index++];
    fan_nwk_manager_app.node_basic_cfg.us_ie.us_schedule.chan_hop_count = buf[index++];
    fan_nwk_manager_app.node_basic_cfg.us_ie.us_schedule.length = buf[index++];
    
    index+=4;//Arjun: Incrementing pointer by 100 to bypass chan_hop_list
    
    if(fan_nwk_manager_app.node_basic_cfg.us_ie.us_schedule.excludded_channel_control == EXCLUDED_CHANNEL_PRESENT)     //Arjun: checking excludded_channel_control value to deceide whether this uninon contains ex_channel_mask/range
    {
      fan_nwk_manager_app.node_basic_cfg.us_ie.us_schedule.excluded_channels.excluded_channel_ranges.num_of_ranges = buf[index++];
      total_ex_channel = fan_nwk_manager_app.node_basic_cfg.us_ie.us_schedule.excluded_channels.excluded_channel_ranges.num_of_ranges;
      for(i = 0; i < total_ex_channel; i++)
      {
        memcpy(&fan_nwk_manager_app.node_basic_cfg.us_ie.us_schedule.excluded_channels.excluded_channel_ranges.ex_ch_range[i].start_ch, &buf[index], 2);
        index+=2;
        memcpy(&fan_nwk_manager_app.node_basic_cfg.us_ie.us_schedule.excluded_channels.excluded_channel_ranges.ex_ch_range[i].end_ch, &buf[index], 2);
        index+=2;
      }
      uint8_t incPointer = (MAX_EXCLUDED_CHANNEL_RANGES_SUPPORTED-total_ex_channel)*4;
      incPointer += 11;
      index+=incPointer;
    }
    else if(fan_nwk_manager_app.node_basic_cfg.us_ie.us_schedule.excludded_channel_control == EXCLUDED_CHANNEL_MASK_PRESENT)
    {
      memcpy(&fan_nwk_manager_app.node_basic_cfg.us_ie.us_schedule.excluded_channels.excluded_channel_mask, &buf[index], 17); 
      index+=32;
    }
    else
    {
      index+=32;
    }
    
    index+=4; //Arjun: bypassing the local_time_node;
    
    if(fan_nwk_manager_app.node_basic_cfg.us_ie.us_schedule.channel_plan == CH_REG_OP_PRESENT)           //Arjun: checking whether reg_op is present
    {
      fan_nwk_manager_app.node_basic_cfg.us_ie.un_channel_plan.ch_reg_op.reg_domain = buf[index++];
      fan_nwk_manager_app.node_basic_cfg.us_ie.un_channel_plan.ch_reg_op.op_class = buf[index++];
      index+=5;
    }
    else if(fan_nwk_manager_app.node_basic_cfg.us_ie.us_schedule.channel_plan == CH_EXPLICIT_PRESENT)
    {
      memcpy((uint8_t *)& fan_nwk_manager_app.node_basic_cfg.us_ie.un_channel_plan.ch_explicit.ch0,&buf[index],4);
      index+=4;
      fan_nwk_manager_app.node_basic_cfg.us_ie.un_channel_plan.ch_explicit.channel_spacing = buf[index++];
      mem_rev_cpy((uint8_t *)& fan_nwk_manager_app.node_basic_cfg.us_ie.un_channel_plan.ch_explicit.num_chans,&buf[index],2);
      index+=2;
    }
    mem_rev_cpy((uint8_t *)& fan_nwk_manager_app.node_basic_cfg.us_ie.channel_fixed.fixed_chan,&buf[index],2);
    
  //send_hif_conf_cb(FAN_UNICAST_IE_SET_CONFIRM,0x00);
  
}


