/** \file ie_element_info.h
 *******************************************************************************
 ** \brief Provides information about the IE Elements of the Wi-SUN FAN Specification
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

#ifndef IE_ELEMENT_INFO_H
#define IE_ELEMENT_INFO_H

#ifdef __cplusplus
extern "C" {
#endif
#if APP_LBR_ROUTER
#include "fan_config_param.h"
#endif
/*
** ============================================================================
** Public Macro definitions
** ============================================================================
*/

#include "common.h"
                                                     
/********************* FAN Related primitives ********************/
//#ifdef WISUN_FAN_MAC

#define CH_REG_OP_PRESENT   0x00
/*Macro defining status of  reg_op present */
#define CH_EXPLICIT_PRESENT   0x01
/*Macro defining status of  EXCLUDED present */
#define EXCLUDED_CHANNEL_PRESENT   0x01
/*Macro defining status of  EXCLUDED present */
#define MAX_CHANNEL_HOP_LIST       4                                                                    
/*Macro defining max number of channel hop list */
#define EXCLUDED_MASK_NO           17                                                                    
/*Macro defining max number of channel mask list */

#define MAX_NBR_SUPPORT            APP_CFG_MAC_MAX_DEV_SUPPORT

/*Macro defining total nbr supported */
//#define  MAXIMUM_PARAM_LIST             32 
/*Macro defining status of max param list */

#define EXCLUDED_CHANNEL_MASK_PRESENT  0x02                                                                    
/*Macro defining status of  EXCLUDED present */                                                                  

   /*! Defines command id for FAN_PAN_SOLICIT_REQUEST*/
#define FAN_PAN_ADV_SOLCIT 					0x1D
                                                     
/*! Defines command id for FAN_PAN_ADVT*/
#define FAN_PAN_ADV      					0x1D                                                     

/*! Defines command id for FAN_PAS_REQUEST*/
#define FAN_PAN_CONF_SOLCIT					0x1E
                                                                                                          
/*! Defines SUB beacon type for Pan Advt*/
#define FAN_PA_SUB_TYPE                                         0x04  
                                                                    
#define PAN_ADVERT_FRAME                                        0x00                                                                     
#define PAN_ADVERT_SOLICIT                                      0x01
#define PAN_CONFIG                                              0x02
#define PAN_CONFIG_SOLICIT                                      0x03
#define FAN_DATA_PKT                                            0x04
#define FAN_ACK                                                 0x05

#if(FAN_EAPOL_FEATURE_ENABLED == 1)
#define EAPOL                                                   0x06                                                                 
#endif                                                     

//#endif
                                                     
/********************* End of FAN Related primitives **************************/
                                                    
/*
** ============================================================================
** Public Structures, Unions & enums Type Definitions
** ============================================================================
*/

/**
 *******************************************************************************
 ** \struct 
 ** Structure to store details FAN MAC parameters
 *******************************************************************************
 **/
//#ifdef WISUN_FAN_MAC
#define Get_My_ChannelPlan()   (mac_pib.ch_sched.channel_plan)
#define Get_My_ChannelFunction()   (mac_pib.ch_sched.channel_function)
#define Get_My_ExcludedChannelCtrl()   (mac_pib.ch_sched.excludded_channel_control)
#define MAX_EXCLUDED_CHANNEL_RANGES_SUPPORTED   5
   
__packed typedef struct excluded_chan_range_tag
{
    uint16_t start_ch;
    uint16_t end_ch;
}excluded_chan_range_t;

__packed typedef struct excluded_chan_range_info_tag
{
    uchar num_of_ranges;
    excluded_chan_range_t ex_ch_range[MAX_EXCLUDED_CHANNEL_RANGES_SUPPORTED];
}excluded_chan_range_info_t;

__packed typedef union excluded_channels_tag
{
    uint8_t excluded_channel_mask[32];
    excluded_chan_range_info_t excluded_channel_ranges;
}excluded_channels_t;

__packed typedef struct panversion_ele_tag{
    uint16_t PANVERSION;
}panversion_t;

__packed typedef struct ch_schedule_tag
{  
    uint8_t dwell_interval;
    uint8_t clock_drift;//Using MLME_SET PIB
    uint8_t timing_accuracy;//Using MLME_SET PIB
    uint8_t channel_plan;
    uint8_t channel_function;
    uint8_t excludded_channel_control;
    uint8_t chan_hop_count;
    uint8_t length;
    uint8_t chan_hop_list[MAX_CHANNEL_HOP_LIST];   
    excluded_channels_t excluded_channels;                            
    uint32_t local_time_node;               
}ch_schedule_t;

__packed typedef struct channel_reg_op_tag                          //Arjun: breaking as per swagger
{
  uint8_t reg_domain;
  uint8_t op_class;
}channel_reg_op_t;

__packed typedef struct channel_explicit_tag                         //Arjun: breaking as per swagger
{
  uint32_t ch0;
  uint8_t channel_spacing;
  uint16_t num_chans;
}channel_explicit_t;

__packed typedef union channel_plan_tag                              //Arjun: union for reg_op and explicit
{
  channel_reg_op_t ch_reg_op;
  channel_explicit_t ch_explicit;
}union_channel_plan_t;

__packed typedef struct channel_fixed_tag                             //Arjun: breaking as per swagger
{
  uint16_t fixed_chan;
}channel_fixed_t;

__packed typedef struct us_ch_schedule_tag                            //Arjun: struct for unicast schedule
{
  ch_schedule_t us_schedule;
  union_channel_plan_t un_channel_plan;
  channel_fixed_t channel_fixed;
}us_ch_schedule_t;

__packed typedef struct bc_ch_schedule_tag                            //Arjun: struct for bcast schedule
{
  uint32_t bcast_interval;
  uint16_t bcast_sched_id;
  //uint8_t length;                                          check why do we need this
  ch_schedule_t bs_schedule;                                        //Arjun : change this acc
  union_channel_plan_t un_channel_plan;
  channel_fixed_t channel_fixed;
  uint8_t is_broadcast_sch_active;
  uint16_t rcvd_broadcast_slot_nuumber;
  uint64_t rcvd_t1_2;
}bc_ch_schedule_t;

typedef struct pan_metrics_tag
{
    uint16_t pan_size;
    uint16_t routing_cost;
    uint8_t parent_bs_ie_use;
    uint8_t routing_methood;
    uint8_t fan_tps_version;
    uint16_t pan_id;
}pan_metrics_t;

typedef struct gtk_hash_element_tag
{
    uint8_t gtk0_hash[8];
    uint8_t gtk1_hash[8];
    uint8_t gtk2_hash[8];
    uint8_t gtk3_hash[8];
}gtk_hash_t;

typedef struct gtk_key_element_tag
{
  uint8_t gtkl;
  uint8_t gtk0_key[16];
  uint8_t gtk1_key[16];
  uint8_t gtk2_key[16];
  uint8_t gtk3_key[16];
}gtk_key_t;

typedef struct key_lifetime_tag
{
  uint32_t pmk_lifetime;                        //in Seconds
  uint32_t ptk_lifetime;                        //in Seconds
  uint32_t gtk_lifetime;                        //in Seconds
  uint32_t gtk_new_activation_time;             //fraction of gtk_lifetime
  uint32_t revocation_lifetime_reduction;       //fraction of gtk_lifetime
}key_lifetime_t;

typedef struct mac_gtk_hash_element_tag
{
  uint8_t gtkl;
  uint8_t MAC_GTK0_Key[16];
  uint8_t MAC_GTK1_Key[16];
  uint8_t MAC_GTK2_Key[16];
  uint8_t MAC_GTK3_Key[16];
}mac_gtk_hash_t;


typedef struct self_info_fan_tag
{
    us_ch_schedule_t unicast_listening_sched;
    bc_ch_schedule_t bcast_sched;                    //Arjun : change this acc
    pan_metrics_t pan_metrics;
    uint8_t net_name_length;
    uint8_t net_name[32];
    gtk_hash_t gtk_hash_ele;                          //Arjun : In some function we are using both gkt , so dont use union in here
    mac_gtk_hash_t mac_gtk_hash_ele;
    uint16_t pan_ver;
    uint16_t bcast_slot_no;
    uint32_t bcast_frac_inter_offset;        
}self_info_fan_mac_t;

typedef struct broadcast_timing_tag {
    uint16_t broadcast_slotno;
    uint32_t broadcast_frac_interval_offset;
}broadcast_timing_t;

typedef struct eui_64_element_information_tag
{
 uint8_t src_addr[8];
}eui_64_ei_t;

typedef struct valid_channel_list
{
  uint8_t *unicast_usable_channel_list;
  uint8_t *broad_usable_channel_list;
  uint8_t total_usable_ch_unicast;
  uint8_t total_usable_ch_broadcast;
 
}valid_channel_list_t;

typedef struct fcie_tag
{
  uint8_t transmit_flow_cont;
  uint8_t receive_flow_cont;
}fcie_tag_t;

enum
{
  INCOMING_PKT_STATUS_NONE = 0x00,
  ACK_TRIGRED_FOR_INCOMING_PKT = 0x01,
  INCOMING_PKT_SEND_TO_UPL = 0x02
};

typedef struct mac_nbr_descriptor_struct
{
  struct mac_nbr_descriptor_struct *next;
  uchar index;                            
  us_ch_schedule_t ucl_sched;
  bc_ch_schedule_t bcl_sched;                         
  pan_metrics_t rev_pan_metrics;
  eui_64_ei_t eui_infor;
  uint8_t net_name_length;
  uint8_t net_name[32];      
  uint64_t ut_ie_rx_time;     //Debdeep
  uint32_t ufsi;
  uchar mac_addr[8];//mac address of nbr
  fcie_tag_t recv_fcie;
  uint8_t device_status;//joined or disconnected
  uint16_t broad_cast_slno;
  uint32_t broad_frac_int_offset;
  uint16_t pan_ver;
  gtk_hash_t recv_gtk_hash;                            //Arjun : we are not using MAC_GTK in this struct so not adding mac_gtk_hash here
  uint32_t own_local_time;
  uint8_t rsl;
  uint16_t etx;
  float pan_cost_clc;         //Debdeep
  uint64_t btie_rcvd_timestamp;       //Debdeep
  valid_channel_list_t nbrchannel_usable_list;
  uint8_t sent_dis_count;
  int16_t rssi;	//Debdeep
  uint8_t is_parent_status;
  uint8_t incoming_pkt_seq_no;
  uint8_t packet_status;
#if(FAN_EAPOL_FEATURE_ENABLED == 1)
  uint8_t eapol_parent_unresponsive;
#endif
  uint64_t ulp_rx_time;
} mac_nbr_descriptor_t;

typedef struct mac_nbr_data_tag
{
    queue_t mac_nbr_info_table;
    uchar mac_nbr_info_table_entries;      
} mac_nbr_data_t;

//mac_static
typedef struct fan_mac_nbr_struct
{
    queue_t desc_table;
    uchar mac_nbr_info_table_entries;      
}fan_mac_nbr_t;

typedef struct neighbor_link_param{
    uint8_t mac_address_t[8];
}neighbor_link_param_t;
    
    
typedef struct netname_element_tag
{
  uint8_t length;
  uint8_t network_name[32];  // maximum size of network name
}netname_t;

  
typedef struct node_basic_config_tag
{
     uint8_t operational_mode;
     uint8_t fan_device_type; 
     uint8_t phy_mode;
     uint8_t pa_level;
     uint16_t selected_channel;
     uint16_t selected_pan_id;
     uint8_t self_ieee_addr[8];         /**< holds it extended address*/
     uint8_t self_global_addr[16];      /**< holds it global ip address*/
     uint8_t operatinng_country;
     uint8_t sybbol_rate;
     uint8_t board_reset;
    float modulation_index;
    us_ch_schedule_t us_ie;	        //Unicast Schedule Information Element (US-IE)
    bc_ch_schedule_t bs_ie;                 //Broadcast Schedule Information Element (BS-IE)
    pan_metrics_t pan_ie;              //PAN Information Element (PAN-IE)
    netname_t netname_ie;      //Network Name Information Element (NETNAME-IE)
    panversion_t panvar_ie;        //PAN Version Information Element (PANVER-IE)
    gtk_key_t gtkhash_ie;      // GTK Hash Information Element (GTKHASH-IE)
    key_lifetime_t key_lifetime;
}node_basic_config_t;
//#endif


/*
** ============================================================================
** Public Variable Declarations
** ============================================================================
*/

/* None */

/*
** ============================================================================
** Private Variable Definitions
** ============================================================================
*/

/* None */

/*
** =============================================================================
** Public Variables Definitions
** =============================================================================
**/

/* None */

/*
** ============================================================================
** External Variable Declarations
** ============================================================================
*/

/* None */

/*
**=========================================================================
**  Public Function Prototypes
**=========================================================================
*/



#ifdef __cplusplus
}
#endif
#endif /* IE_ELEMENT_INFO_H */

