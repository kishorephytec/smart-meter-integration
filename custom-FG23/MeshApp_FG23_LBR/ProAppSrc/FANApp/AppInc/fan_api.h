/** \file fan_api.h
 *******************************************************************************
 ** \brief This file breifly describes the configuration of FAN stack and command IDs
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

#ifndef FAN_API_H
#define FAN_API_H

#ifdef __cplusplus
extern "C" {
#endif

/*
** =============================================================================
** Public Macro definitions
** =============================================================================
*/

#define FACTORY_MODE    0x00
#define RUN_MODE        0x01 

  
// For Running the code Using Rest Server
//#define TLV_FRAME_FORMATIS_ENABLE         0
  
// For Running the Code in Auto Mode and with Fixed Channel or Frequency Hopping
#define AUTO_CONFIG_ENABLE                1

  
  
#define PROTOCOL_ID_FOR_APP                      1 
  
#define DUMMY_COMPORT                           0x55    
  

#define APP_DEF_LAYER_ID_TBC  5

  
//#define APP_DEF_LAYER_ID_TOOL 7   // for UART Demon Linux Application  
  
#define APP_DEF_LAYER_ID_TOOL 2    // for Procubed Test Tool
  
  
#define APP_DEF_LAYER_ID_DHCP   7  
#define APP_DEF_LAYER_ID_RADIUS   8
  


/*
** =============================================================================
** Public Structures, Unions & enums Type Definitions
** =============================================================================
**/

/**
 *******************************************************************************
 ** \enum ccasm_trigger_t
 ** Specific triggers for CCA state machines
 *******************************************************************************
**/ 
/******************************************************************************
                        SwaggerHub Command ID
*******************************************************************************/

#define NODE_START_STOP                                         0x30
#define NODE_START_STOP_CONF                                    0x31  
//#define SET_PHY                                                 0x32
//#define CONF_SET_PHY                                            0x33

//#define SET_MAC_CHANNEL_PLAN_REG_OP_API                         0x34
//#define SET_MAC_CHANNEL_PLAN_REG_OP_API_CONF                    0x35  


#define SET_FACTORY_MODE_PA_LEVEL_API                         0x34
#define SET_FACTORY_MODE_PA_LEVEL_API_CONF                    0x35  
  
  
//#define SET_MAC_CHANNEL_PLAN_EXPLICIT_API                       0x36
//#define SET_MAC_CHANNEL_PLAN_EXPLICIT_API_CONF                  0x37   
  
//#define SET_MAC_UNICAST_API                                     0x38    
//#define SET_MAC_UNICAST_API_CONF                                0x39  
//#define SET_MAC_BROADCAST_API                                   0x3A
//#define SET_MAC_BROADCAST_API_CONF                              0x3B  
#define SET_MAC_CHAN_PLAN_FIXED                                 0x3C
#define SET_MAC_CHAN_PLAN_FIXED_CONF                            0x3D  
//#define SET_LBR_MAC_CONFIG                                      0x3E
//#define SET_LBR_MAC_CONFIG_CONF                                 0x3F
//#define SET_LBR_MAC_GTKS_CONFIG                                 0x40
//#define SET_LBR_MAC_GTKS_CONFIG_CONF                            0x41
//#define SET_ROUTER_CONFIG                                       0x42
//#define SET_ROUTER_CONFIG_CONF                                  0x43
#define RESET_RPL_MSG_RATE                                      0x44
#define RESET_RPL_MSG_RATE_CONF                                 0x45
//#define SUBSCRIBE_PACKETS                                       0x46
//#define SUBSCRIBE_PACKETS_CONF                                  0x47 
#define SEND_UDP                                                0x48
#define SEND_UDP_ECHO_RESP                                      0x49
#define SEND_ICMPv6                                             0x4A
#define SEND_ICMPv6_CONF                                        0x4B
#define GET_IP_ADDRESSES                                        0x4C
#define SEND_IPv6_ADDRESS                                       0x4D
#define GET_SEC_KEYS                                            0x4E
#define GET_SEC_KEYS_CONF                                       0x4F  
#define API_GET_DODAG_ROUTES                                    0x50
#define SEND_DODAG_ROUTER_TABLE                                 0x51
#define API_GET_NEIGHBOR_TABLE                                  0x52 
#define SEND_NBR_TABLE                                          0x53
#define RECV_ICMPv6_REQ                                         0x54
#define API_GET_JOIN_STATE                                      0x55
#define SEND_CURRENT_JOIN_STATE                                 0X56 
#define RECV_ICMPV6_REPLY_IND                                   0x57
//#define ASYNC_FRAME_REQ_CONF_TO_TOOL                            0x58
//#define ASYNC_FRAME_INDICATION_TO_TOOL                          0x59
#define SEND_DODAG_ROUTE_NULL                                   0x5A
#define SEND_NBR_TABLE_NULL                                     0x5B
//#define SET_LBR_MAC_PMK_PTK_GTK_LIFETIME_CONFIG                 0x5C
//#define SET_LBR_MAC_PMK_PTK_GTK_LIFETIME_CONFIG_CONF            0x5D
//#define SET_MAC_WHITELIST                                       0x5E
//#define SET_MAC_WHITELIST_CONF                                  0x5F
//#define SET_REVOCATION_KEY                                      0x60
//#define SET_REVOCATION_KEY_CONF                                 0x61  
#define GET_VERSION_INFO_REQ                                    0x62
#define GET_VERSION_INFO_RESP                                   0x63
#define GET_ALL_PARAM_INFO_REQ                                  0x64
#define GET_ALL_PARAM_INFO_RESP                                 0x65
#define SOFTWARE_RESET                                          0x66
#define GET_DEVICE_PRIMERY_PARENTS_REQ                          0x67
#define GET_DEVICE_PRIMERY_PARENTS_RESP                         0x68
//#define EDFE_FRAME_EXCHANGE_REQ                                 0x69
//#define EDFE_FRAME_EXCHANGE_CONF                                0x70
//#define GET_MAC_WHITELIST                                       0x71
//#define GET_MAC_WHITELIST_RESP                                  0x72
//#define MAC_WHITELIST_NULL                                      0x73
//#define SET_START_NETWORK_SCALE                                 0x74
//#define SET_START_NETWORK_SCALE_CONF                            0x75
//#define REVOKE_STA_REQ                                          0x76
//#define REVOKE_STA_CONF                                         0x77
#define TRIG_WAN_PING_REQST                                     0xB1
#define RECV_WAN_PING_REPLY                                     0xB2
#define SEND_RUNTIME_LOG                                        0xB3
#define SEND_DHCP_DATA_TO_SERVER                                0xB4
#define RECV_DHCP_DATA_FROM_SERVER                              0xB5
#define SEND_DATA_TO_RADIOUS_SERVER                             0xB6
#define RECV_DATA_FROM_RADIOUS_SERVER                           0xB7
#define SEND_RECV_PKT_ACK                                       0xB8
//#define TELEC_SET_PA_LEVEL_MCR					0x80
//#define TELEC_SET_PA_LEVEL_MCR_CONF				0x81  
#define OUT_OF_RANGE                                            0x07  
#define LBR_TYPE                                                0x00
#define SEND_NOT_SUPPORTED                                      0x0A
//#define DEVICE_NODE_CONFIG                                      0x82
//#define DEVICE_NODE_CONFIG_CONF                                 0x83
#define MAC_SECURITY_ENABLE_DIASBLE                             0x84
#define MAC_SECURITY_ENABLE_DIASBLE_CONF                        0x85  
//#define UDP_PORT_REGISTER                                       0x86
//#define UDP_PORT_REGISTER_CONF                                  0x87
#define UDP_RECIVED_IND                                         0x88
#define SEND_UDP_PACKET                                         0x89
#define SEND_ICMPV6_CONF                                        0x8A


  
   
  

/******************************************************************************
  ****************************************************************************/
  
enum
{
  FAN_STOP_NODE,
  FAN_START_NODE

};


/******************************************************************************
  ****************************************************************************/
  

uint8_t subscribe_packets
(
  uint8_t command, /*Start or Stop packet subscription.*/
  uint8_t *forwarding_address,/*Address to which packet is to be forwarded.*/
  uint16_t forwarding_port /*Port to which packet is to be forwarded.*/
);

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

);


uint8_t node_start_stop( uint8_t start);    
uint8_t set_phy(uint8_t modulation, uint8_t symbol_rate , float modulation_index ,uint8_t pa_level);
uint8_t set_mac_chan_plan_reg_op
(
      uint8_t regulatory_domain,
      uint8_t operating_class
);
uint8_t set_mac_chan_plan_explicit
(
 
    uint32_t ch0,
    uint8_t channel_spacing, 
    uint16_t number_of_channel
  
);

uint8_t  set_mac_unicast_chan_plan
(
      uint8_t  dwell_interval,
      uint8_t  channel_function,
      uint8_t excluded_channel_control,
      uint8_t  excluded_channel_number,
      uint16_t  *excluded_channel_Range,
      uint8_t  *excluded_channel_Mask

);

uint8_t set_mac_bcast_chan_plan
(
    uint32_t broadcast_interval,
    uint16_t broadcast_schedule_identifier,
    uint8_t dwell_interval,
    uint8_t channel_function,
    uint8_t excluded_channel_control,
    uint8_t excluded_channel_number,
    uint16_t *excluded_channel_Range,
    uint8_t *excluded_channel_Mask
);

uint8_t set_mac_chan_plan_fixed(uint16_t fixed_channel_number);

uint8_t  set_lbr_mac_gtks_config
(
    uint8_t *gtk0,
    uint8_t *gtk1,
    uint8_t *gtk2,
    uint8_t *gtk3,
    uint8_t gtkl
);

uint8_t  set_lbr_mac_config
(
    uint16_t pan_size,
    uint16_t pan_id,
    uint8_t use_parent_broadcast_schedule,
    uint8_t routing_method,
    uint8_t *network_Name
);

uint8_t reset_rpl_msg_rate(uint8_t message);
void get_dodag_routers(void);
void get_neighbor_table(void);
void get_ip_address();
void recved_wan_ping_reply (uint8_t wan_reply_status);
uint8_t send_responce(uint8_t status_code);
uint8_t set_router_config(uint8_t *network_Name,uint8_t routing_method, uint16_t pan_size);
uint8_t set_mac_white_list(uint8_t *white_list,uint16_t length);
uint8_t set_mac_white_list_tbc(uint8_t *white_list,uint16_t length);
uint8_t set_revoaction_key(uint8_t *revoaction_list, uint16_t len);
uint8_t get_prefered_parents(uint8_t *child_add, uint16_t len);
//void send_edfe_exchange_frame(uint8_t *edfe_buf, uint16_t len);
//extern uint8_t TRX_Set_PA_Level( uint8_t pa_level );
//void send_lifetime_to_hostapd(uint8_t *buff , uint16_t len);
//void  get_mac_white_list();
extern void FAN_MAC_MLME_SET_Request
(
        uint8_t ie_identifier,		        /* header ie or payload ie */
        uint8_t ie_subid, 			/* subid for each ie */	        
        uint16_t Length,		  	/* length of value */
        void *mac_value	                        /* pointer to the value*/
);
#if(APP_NVM_FEATURE_ENABLED == 1)
void nvm_store_node_basic_info( void );
#endif

int get_join_state (void);
typedef struct filter_mac_add_tag
{
  uint8_t mac_addr[8];
}filter_mac_add_tag;


typedef struct white_list_tag
{
  filter_mac_add_tag wht_list_macaddr[APP_CFG_MAX_DEV_SUPPORT];
  uint8_t wht_mac_index;
}white_list_t;

#ifdef __cplusplus
}
#endif
#endif /* FAN_API_H */
