/** \file fan_app_auto.c
*******************************************************************************
** \brief Implements the auto configuration for running the code in auto mode
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

/*******************************************************************************
* File inclusion
*******************************************************************************/
#include "StackAppConf.h"
#include "common.h"
#include "em_device.h"
#include "em_system.h"
#include "list_latest.h"
#include "queue_latest.h"
#include "uart_hal.h"
#include "mac.h"
#include "hif_utility.h"
#include "hif_service.h"
#include "buff_mgmt.h"
#include "buffer_service.h"
#include "hw_tmr.h"
#include "sw_timer.h"
#include "mac_interface_layer.h"
#include "sm.h"
#include "contiki-net.h"
#include "ie_element_info.h"
#include "network-manager.h"
#include "fan_app_auto.h"
#include "fan_app_test_harness.h"
#include "fan_api.h"
#include "fan_factorycmd.h"
#include "phy.h"

/*
** ============================================================================
** Private Structures, Unions & enums Type Definitions
** ============================================================================
*/

/*None*/

/*
** ============================================================================
** Private Variable Definitions
** ============================================================================
*/

/*None*/

/*
** ============================================================================
** Public Variable Definitions
** ============================================================================
*/

broadcast_timing_t broadcast_timing;
bc_ch_schedule_t broadcast_ie;
us_ch_schedule_t unicast_ie;
pan_metrics_t pan_ie_param;
gtk_key_t gtk_hash_ie;
panversion_t panversion_ie;

/*
** ============================================================================
** External Variable Declarations
** ============================================================================
*/

extern void send_host_apd_bootup();
extern void send_mac_addr(uint8_t *buff , uint16_t len);
extern uint8_t phyModeMapArr[8];
void set_async_channel_range(void);
uint8_t total_channel_lenth_uicast = 0x00; 

#if(APP_NVM_FEATURE_ENABLED == 1)
extern nvm_structure_t store_nvm_param;
#endif

extern uint8_t process_telec_set_operating_country( uint8_t CountryCode  ) ;
/*
** ============================================================================
** Private Macro definitions
** ============================================================================
*/



/*

USA configuration:
******************************

Index   PhyMode    RegDomain  OPClass DataRate(ksymbol/s)  ChanSpacing (kHz)        
0       1              1        1       50                 200
1       2              1        1       100                200
2       3              1        2       150                400
3       4              1        2       200                400    [ Not Used In FAN] 
4       5              1        3       300                600

India Configuration:
***************************
Freq Band (MHz)	Region	Regulatory      Operating          PHY Modes                  ChanSpacing (kHz)	      Total Num Chan	  Chan Center Freq0 (MHz)
                        Domain Value    Class 
 							
865-867d	IN	   0x05	        1	        Operating Mode #1a	        100	                        19	        865.1
                                        2	        Operating Mode #2a & #3	        200	                        10	        865.1


Index   PhyMode    RegDomain  OPClass DataRate(ksymbol/s)  ChanSpacing (kHz)        
0       1              5        1       50                 100
1       2              5        2       100                200
2       3              5        2       150                200

*/

#define  STACK_OPERATIONAL_MODE       RUN_MODE          //FACTORY_MODE ; //RUN_MODE;

#define DEF_PHYMODE             PHY_MODE_SELECT_ID    // 0 : 50 , 1: 100 , 2 : 150

#define DEF_FIXEDCHANNELNO      6
#define DEF_REGULATORYDOMAIN    5  // INDIA
#define DEF_OPERATINGCLASS      2    // 1 : 50 ; 2 100 , 
#define DEF_CHANEELFUNCTION     0 // 0 for fixed and 2 For DH1CF 

#define DEF_USEPARENTBSIE       1
#define DEF_ROUTINGMETHOD       1  //0 – Layer 2 routing  :: 1 - Layer 3 routing
#define DEF_PANVERSION          0x1234
#define DEF_FANTPSVERSION       0x01

#define DEF_BROADCASTDWELL      100
#define DEF_UNICASTDWELL        15
#define DEF_BROADCASTINTERVAL   1020  // 4*( max of (UDI, BDI)
#define DEF_CLOCKDRIFT          0x00
#define DEF_TA                  0x00
#define DEF_BSI                 0x2233

#define DEF_NETNAME             "Phytec"
#define DEF_PANID               0xCAFE
#define DEF_ROUTINGCOST         0
#define DEF_PANSIZE             1000

#define RUNNING_DEVICE_TYPE     COORD_NODE_TYPE //PAN_COORD_NODE_TYPE (0x00): for LBR :::  COORD_NODE_TYPE (0x01) :for  router    


extern void check_status_to_start_network();
/*
** ============================================================================
** Private Function Prototypes
** ============================================================================
*/

/*None*/

/*
** ============================================================================
** Public Function Definitions
** ============================================================================
*/

 
  
  
void configure_device_run_param()
{
#if(AUTO_CONFIG_ENABLE == 1)
  
  /*
  The Channel Plan field is a 3 bit unsigned integer populated as follows: 1488
  1. The Channel Plan field MUST be set to 0  For Regulatory Domain and Operating Class 
  2. The Channel Plan field MUST be set to 1 For CH0,Channel Spacing, and Number Of Channel
  
  The Channel Function 
  1. The Channel Function field MUST be set to 0 when a single, Fixed Channel
  2. The Channel Function field MUST be set to 1 For TR51CF
  3. The Channel Function field MUST be set to 2 For DH1CF 
  4. The Channel Function field MUST be set to 3 For Vendor Defined 
  
  The Excluded Channel Control field 
  1. The Excluded Channel Control field MUST be set to 0 when no excluded channels
  2. The Excluded Channel Control field MUST be set to 1 when an Excluded Channel Range field 
  3. The Excluded Channel Control field MUST be set to 2 when an Excluded Channel Mask field 
  */
  uint8_t   Ch_plan = 0;
  uint8_t   ch_func = DEF_CHANEELFUNCTION;
  uint8_t   ex_ch_control = 0;
  
  uint64_t MACAddr = 0;
  
  node_basic_config_t* p_basic_cfg = &(fan_nwk_manager_app.node_basic_cfg);
  
  uint8_t self_ieee_addr[8] = {SELF_LONG_ADDRESS};
  uint8_t net_name[] = {DEF_NETNAME};
  
//  uint8_t GTK0_Key_temp[16] = { 0x00, 0x00, 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00};
  uint8_t GTK0_Key_temp[16] = { 0x01, 0x02, 0x03 , 0x04 , 0x05 , 0x06 , 0x07 , 0x08 , 0x09 , 0x0A , 0x0B , 0x0C , 0x0D , 0x0E , 0x0F , 0x10};
//  uint8_t GTK1_Key_temp[16] = { 0x00, 0x00, 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00};
  uint8_t GTK1_Key_temp[16] = { 0x11, 0x12, 0x13 , 0x14 , 0x15 , 0x16 , 0x17 , 0x18 , 0x19 , 0x1A , 0x1B , 0x1C , 0x1D , 0x1E , 0x1F , 0x22};
//  uint8_t GTK2_Key_temp[16] = { 0x00, 0x00, 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00};
  uint8_t GTK2_Key_temp[16] = { 0x21, 0x22, 0x23 , 0x24 , 0x25 , 0x26 , 0x27 , 0x28 , 0x29 , 0x2A , 0x2B , 0x2C , 0x2D , 0x2E , 0x2F , 0x33};
//  uint8_t GTK3_Key_temp[16] = { 0x00, 0x00, 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00};
  uint8_t GTK3_Key_temp[16] = { 0x31, 0x32, 0x33 , 0x34 , 0x35 , 0x36 , 0x37 , 0x38 , 0x39 , 0x3A , 0x3B , 0x3C , 0x3D , 0x3E , 0x3F , 0x44};
  
  gtk_hash_ie.gtkl = 0x0F; /* 0x01 for 1 GTK; 0x03 for 2 GTK; 0x07 for 3 GTK; 0x0F for 4 GTK */
  
  uint8_t b_ex_ch_mask[17] = {0x0A,0x1B,0x2C,0x3D,0x00,0x00,0x4E,0x5F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
  uint8_t u_ex_ch_mask[17] = {0x0A,0x1B,0x2C,0x3D,0x00,0x00,0x4E,0x5F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
  
  
  //process_telec_set_operating_country( RUNNING_COUNTRY_CODE ) ;
  
  p_basic_cfg->fan_device_type = RUNNING_DEVICE_TYPE ;
  p_basic_cfg->phy_mode = phyModeMapArr[DEF_PHYMODE]; 
  p_basic_cfg->selected_channel = DEF_FIXEDCHANNELNO;   
  p_basic_cfg->selected_pan_id = DEF_PANID;
  
 
  // INDIA Configuration ...
  
  if(p_basic_cfg->phy_mode == 0x01) 
  {
    total_channel_lenth_uicast = 29;
  }
  else if((p_basic_cfg->phy_mode == 0x02) || (p_basic_cfg->phy_mode == 0x04))
  {
    total_channel_lenth_uicast = 15;
  }
    
  
  // Set the MAC address As device ID ....
  //MACAddr =  SYSTEM_GetUnique();  
  //mem_rev_cpy( p_basic_cfg->self_ieee_addr, (uint8_t *)&MACAddr, sizeof (uint64_t) );
  mem_rev_cpy( p_basic_cfg->self_ieee_addr, &self_ieee_addr[0], sizeof (uint64_t) ) ;
  
  
  //cmd-6
  process_set_net_name_ie(net_name, strlen((char const*)net_name));   
  
  //cmd-7
  panversion_ie.PANVERSION = UIP_HTONS(DEF_PANVERSION);  
  process_set_panversion_ie((uint8_t *)&panversion_ie, 2);    
  
  //cmd-8
  memcpy(gtk_hash_ie.gtk0_key, GTK0_Key_temp, 16);
  memcpy(gtk_hash_ie.gtk1_key, GTK1_Key_temp, 16);
  memcpy(gtk_hash_ie.gtk2_key, GTK2_Key_temp, 16);
  memcpy(gtk_hash_ie.gtk3_key, GTK3_Key_temp, 16);
  
  //if(fan_nwk_manager_app.node_basic_cfg.fan_device_type != COORD_NODE_TYPE)
    process_set_gtk_hash_ie((uint8_t *)&gtk_hash_ie, sizeof (gtk_hash_ie));  
  
  //cmd-9
  //best parent selection using pan-size,root=0,R1=1, R2=2, R3=3    
  pan_ie_param.pan_size = UIP_HTONS(DEF_PANSIZE);
  //for root it should be 0 and for router and leaf node, it is updated after J5 from ETX.
  pan_ie_param.routing_cost = UIP_HTONS(DEF_ROUTINGCOST);
  pan_ie_param.parent_bs_ie_use = DEF_USEPARENTBSIE;
  pan_ie_param.routing_methood = DEF_ROUTINGMETHOD;
  pan_ie_param.fan_tps_version = DEF_FANTPSVERSION; 
  
  process_set_pan_ie((uint8_t *)&pan_ie_param, 7);
  
  /*cmd-4:Set Broadcast Information Element IE*/
  broadcast_ie.bcast_interval = UIP_HTONL(DEF_BROADCASTINTERVAL);
  broadcast_ie.bcast_sched_id = UIP_HTONS(DEF_BSI);
  broadcast_ie.bs_schedule.dwell_interval = DEF_BROADCASTDWELL;
  broadcast_ie.bs_schedule.clock_drift = DEF_CLOCKDRIFT;
  broadcast_ie.bs_schedule.timing_accuracy = DEF_TA;
  
  broadcast_ie.bs_schedule.channel_plan = Ch_plan; //0x01;
  broadcast_ie.bs_schedule.channel_function = ch_func; //0x00;
  broadcast_ie.bs_schedule.excludded_channel_control = ex_ch_control; //0x02;
  
  
  if(broadcast_ie.bs_schedule.channel_plan == CH_REG_OP_PRESENT)
  {
    broadcast_ie.un_channel_plan.ch_reg_op.reg_domain = DEF_REGULATORYDOMAIN;
    broadcast_ie.un_channel_plan.ch_reg_op.op_class = DEF_OPERATINGCLASS;
  }
  else if(broadcast_ie.bs_schedule.channel_plan ==CH_EXPLICIT_PRESENT)
  {
    
    // Raka :: 04-July-2018, we are not using this configuration.
    broadcast_ie.un_channel_plan.ch_explicit.ch0 = 0x0DC438;
    broadcast_ie.un_channel_plan.ch_explicit.channel_spacing = 0x02;
    broadcast_ie.un_channel_plan.ch_explicit.num_chans = UIP_HTONS( total_channel_lenth_uicast); 
  }
  
  broadcast_ie.channel_fixed.fixed_chan = UIP_HTONS(p_basic_cfg->selected_channel); 
  broadcast_ie.bs_schedule.chan_hop_count = 0x00;
  broadcast_ie.bs_schedule.chan_hop_list[0] = 0x00;
  
  if (broadcast_ie.bs_schedule.excludded_channel_control == 0x01)
  {
    // Excluded channel Range
    memset(broadcast_ie.bs_schedule.excluded_channels.excluded_channel_mask,0,32);
    //Total number of excluded channels
    broadcast_ie.bs_schedule.excluded_channels.excluded_channel_ranges.num_of_ranges = 0x03;
    broadcast_ie.bs_schedule.excluded_channels.excluded_channel_ranges.ex_ch_range[0].start_ch = 0x000A;
    broadcast_ie.bs_schedule.excluded_channels.excluded_channel_ranges.ex_ch_range[0].end_ch = 0x0014;
    broadcast_ie.bs_schedule.excluded_channels.excluded_channel_ranges.ex_ch_range[1].start_ch = 0x0032;
    broadcast_ie.bs_schedule.excluded_channels.excluded_channel_ranges.ex_ch_range[1].end_ch = 0x003C; 
    broadcast_ie.bs_schedule.excluded_channels.excluded_channel_ranges.ex_ch_range[2].start_ch = 0x0064;
    broadcast_ie.bs_schedule.excluded_channels.excluded_channel_ranges.ex_ch_range[2].end_ch = 0x0080;
    broadcast_ie.bs_schedule.excluded_channels.excluded_channel_ranges.ex_ch_range[3].start_ch = 0x0000;
    broadcast_ie.bs_schedule.excluded_channels.excluded_channel_ranges.ex_ch_range[3].end_ch = 0x0000;  
    broadcast_ie.bs_schedule.excluded_channels.excluded_channel_ranges.ex_ch_range[4].start_ch = 0x0000;
    broadcast_ie.bs_schedule.excluded_channels.excluded_channel_ranges.ex_ch_range[4].end_ch = 0x0000;
  }
  else if (broadcast_ie.bs_schedule.excludded_channel_control == 0x02)
  {
    //Excluded channel mask 
    memcpy(broadcast_ie.bs_schedule.excluded_channels.excluded_channel_mask, &b_ex_ch_mask[0], 17); 
  }
  //ex_channel_mask;    
  process_broadcast_ie_set_req((uint8_t *)&broadcast_ie, sizeof(broadcast_ie)); 
  
  
  
  /* Unicast Channel Information  */
  unicast_ie.us_schedule.dwell_interval = DEF_UNICASTDWELL;
  unicast_ie.us_schedule.clock_drift = DEF_CLOCKDRIFT;
  unicast_ie.us_schedule.timing_accuracy = DEF_TA;
  
  unicast_ie.us_schedule.channel_plan = Ch_plan ; 
  unicast_ie.us_schedule.channel_function = ch_func; 
  unicast_ie.us_schedule.excludded_channel_control = ex_ch_control;  
  
  if(unicast_ie.us_schedule.channel_plan ==CH_REG_OP_PRESENT)
  {
    unicast_ie.un_channel_plan.ch_reg_op.reg_domain = DEF_REGULATORYDOMAIN;
    unicast_ie.un_channel_plan.ch_reg_op.op_class = DEF_OPERATINGCLASS;
  }
  else if(unicast_ie.us_schedule.channel_plan ==CH_EXPLICIT_PRESENT)
  {
    // Raka :: 04-July-2018, we are not using this configuration.
    unicast_ie.un_channel_plan.ch_explicit.ch0 = 0x0DC438;
    unicast_ie.un_channel_plan.ch_explicit.channel_spacing = 0x02;
    unicast_ie.un_channel_plan.ch_explicit.num_chans = UIP_HTONS (total_channel_lenth_uicast); 
  }
  unicast_ie.channel_fixed.fixed_chan = UIP_HTONS(p_basic_cfg->selected_channel); 
  unicast_ie.us_schedule.chan_hop_count = 0x00;
  unicast_ie.us_schedule.chan_hop_list[0] = 0x00;
  
  if (unicast_ie.us_schedule.excludded_channel_control == 0x01)
  {
    // Excluded channel Range
    memset(unicast_ie.us_schedule.excluded_channels.excluded_channel_mask,0,32);
    //total number of excluded channels
    unicast_ie.us_schedule.excluded_channels.excluded_channel_ranges.num_of_ranges = 0x02;
    unicast_ie.us_schedule.excluded_channels.excluded_channel_ranges.ex_ch_range[0].start_ch = 0x000A;
    unicast_ie.us_schedule.excluded_channels.excluded_channel_ranges.ex_ch_range[0].end_ch = 0x0014;
    unicast_ie.us_schedule.excluded_channels.excluded_channel_ranges.ex_ch_range[1].start_ch = 0x0032;
    unicast_ie.us_schedule.excluded_channels.excluded_channel_ranges.ex_ch_range[1].end_ch = 0x003C; 
    unicast_ie.us_schedule.excluded_channels.excluded_channel_ranges.ex_ch_range[2].start_ch = 0x006E;
    unicast_ie.us_schedule.excluded_channels.excluded_channel_ranges.ex_ch_range[2].end_ch = 0x0079;
    unicast_ie.us_schedule.excluded_channels.excluded_channel_ranges.ex_ch_range[3].start_ch = 0x0079;
    unicast_ie.us_schedule.excluded_channels.excluded_channel_ranges.ex_ch_range[3].end_ch = 0x003B;  
    unicast_ie.us_schedule.excluded_channels.excluded_channel_ranges.ex_ch_range[4].start_ch = 0x003D;
    unicast_ie.us_schedule.excluded_channels.excluded_channel_ranges.ex_ch_range[4].end_ch = 0x0080; 
  }
  else if (unicast_ie.us_schedule.excludded_channel_control == 0x02)
  {
    //Excluded channel mask 
    memcpy(unicast_ie.us_schedule.excluded_channels.excluded_channel_mask, &u_ex_ch_mask[0], 17);
  }
  
  fan_nwk_manager_app.node_basic_cfg.operational_mode = STACK_OPERATIONAL_MODE; 
  process_unicast_ie_set_req((uint8_t *)&unicast_ie, sizeof(unicast_ie)); 


#if (APP_NVM_FEATURE_ENABLED == 1)
  
  fan_nwk_manager_app.nvm_write_to_start = true;
  nvm_store_node_basic_info();
  check_status_to_start_network();
#endif

#else
  /* configure through tool enable this*/
  fan_nwk_manager_app.node_basic_cfg.fan_device_type = DEVICE_TYPE;
#endif   // for #if(AUTO_CONFIG_ENABLE == 1)
  
}

