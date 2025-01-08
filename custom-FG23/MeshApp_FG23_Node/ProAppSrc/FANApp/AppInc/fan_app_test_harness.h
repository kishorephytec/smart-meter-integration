/** \file fan_app_test_harness.h
 *******************************************************************************
 ** \brief Provides APIs for FAN stack test.
 **
 **  This file contains the public APIs and structures for FAN Testharness header 
 **   module.
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

#ifndef _ENET_CONTROLLER_INTERFACE_H_
#define _ENET_CONTROLLER_INTERFACE_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 *****************************************************************************
 * @ingroup sysdoc
 *
 * @{
 *****************************************************************************/

  #define SEND_PING_PKT_TO_HIF
  #define SEND_SICSLOWPAN_PKT_TO_HIF
  #define SEND_UDP_PKT_TO_HIF

/**
 *****************************************************************************
 * 
 * @} end of sys doc
 *****************************************************************************/

/*
** ============================================================================
** Public Macro definitions
** ============================================================================
*/
#define  NO_ERR                                      0x00   
#define GROUP_ID_ENET_TEST_IF                        0x06  
#define GROUP_ID_FAN_TEST_IF                         0x02
#define GROUP_ID_FOR_TBC                             0x03
#define GROUP_ID_EAPOL_TEST_IF_BOARD_2_BOARD         0x09


#define SET_BASIC_CONFIG                             0x8B
#define SET_BASIC_CONFIG_CONF                        0x8C
  
#define START_FAN_NETWORK                            0x8D
#define START_FAN_NETWORK_CONF                       0x8E
  
#define SYSTEM_RESET                                 0x8F
//#define APP_2_ENET_ECHO_REQ			     0x90// for triggering SETI
//#define APP_2_ENET_ECHO_RESP                         0x91

#define APP_2_FAN_UDP_REQ                            0x92
#define APP_2_FAN_UDP_RCV_CB                         0x93

#define ENET_2_APP_DEST_UNREACHABLE_CMD_ID	     0x94
#define ENET_2_APP_PARA_PROB_IND_CMD_ID		     0x95
#define ENET_2_APP_NA_IND_CMD_ID                     0x96
#define ENET_2_APP_NS_IND_CMD_ID                     0x97
#define ENET_2_APP_2_ECHO_REPLY_IND_CMD_ID           0x98
#define ENET_2_APP_ECHO_REQ_IND_CMD_ID               0x99
   
#define ENET_2_APP_FIRST_FRAG			     0x9A
#define ENET_2_APP_FIRST_FRAGN			     0x9B
#define ENET_2_APP_IPHC				     0x9C
//#define TX_SECHEDULE_END                             0x9D
//#define TX_SECHEDULE_START                           0x9E  

#define    CONTROLLER_APP_START_REQ                  0x9F
#define    CONTROLLER_APP_START_CONF                 0xA0

//#define FAN_BROADCAST_TIMING_SET_REQUEST             0xA1
//#define FAN_BROADCAST_TIMING_SET_CONFIRM             0xA2
  
//#define FAN_NETNAME_IE_SET_REQUEST                   0xA3
//#define FAN_NETNAME_IE_SET_CONFIRM                   0xA4
  
//#define FAN_PAN_IE_SET_REQUEST                       0xA5
//#define FAN_PAN_IE_SET_CONFIRM                       0xA6
//#define FAN_GTK_HASH_IE_SET_REQUEST                  0xA7
//#define FAN_GTK_HASH_IE_SET_CONFIRM                  0xA8
//#define FAN_PAN_VER_IE_SET_REQUEST                   0xA9
//#define FAN_PAN_VER_IE_SET_CONFIRM                   0xAA
//#define FAN_BROADCAST_IE_SET_REQUEST                 0xAB
//#define FAN_BROADCAST_IE_SET_CONFIRM                 0xAC
//#define FAN_UNICAST_IE_SET_REQUEST                   0xAD
//#define FAN_UNICAST_IE_SET_CONFIRM                   0xAE

#define DWELL_INTERVAL_US                             0x01
#define CHANNEL_FUNCTION_US                           DWELL_INTERVAL_US+1
#define EXC_CHAN_RANGE_US                             CHANNEL_FUNCTION_US+1
#define EXC_CHAN_MASK_US                              EXC_CHAN_RANGE_US+1
#define BCAST_INTERVAL_BS                             EXC_CHAN_MASK_US+1
#define BCAST_SCH_IDNT_BS                             BCAST_INTERVAL_BS+1
#define DWELL_INTERVAL_BS                             BCAST_SCH_IDNT_BS+1
#define CHANNEL_FUNCTION_BS                           DWELL_INTERVAL_BS+1
#define EXC_CHAN_RANGE_BS                             CHANNEL_FUNCTION_BS+1
#define EXC_CHAN_MASK_BS                              EXC_CHAN_RANGE_BS+1
#define GTK0                                          EXC_CHAN_MASK_BS+1
#define GTK1                                          GTK0+1
#define GTK2                                          GTK1+1
#define GTK3                                          GTK2+1
#define PMK_LIFETIME                                  GTK3+1
#define PTK_LIFETIME                                  PMK_LIFETIME+1                    
#define GTK_LIFETIME                                  PTK_LIFETIME+1
#define GTK_NEW_ACTIVATION_TIME                       GTK_LIFETIME+1            
#define REVOCATION_LIFETIME_REDUCTION                 GTK_NEW_ACTIVATION_TIME+1
#define ICMPV6_PING_SRC_ADDR                          REVOCATION_LIFETIME_REDUCTION+1
#define ICMPV6_PING_DST_ADDR                          ICMPV6_PING_SRC_ADDR+1
#define ICMPV6_PING_HOP_LIMIT                         ICMPV6_PING_DST_ADDR+1
#define ICMPV6_PING_ECHO_DATA                         ICMPV6_PING_HOP_LIMIT+1
#define ICMPV6_PING_FRAME_EXCHANGE_PATTERN            ICMPV6_PING_ECHO_DATA+1
#define ICMPV6_PING_IDENTIFIER                        ICMPV6_PING_FRAME_EXCHANGE_PATTERN+1
#define ICMPV6_PING_SEQUENCE_NUMBER                   ICMPV6_PING_IDENTIFIER+1
/*
** ============================================================================
** Public Structures, Unions & enums Type Definitions
** ============================================================================
*/ 



 /* None */

/*
** ============================================================================
** Public Variable Declarations
** ============================================================================
*/

extern uint8_t sendSubscribedPacket;

/*
** ============================================================================
** Public Function Prototypes
** ============================================================================
*/

void APPhifForToolTest_Init(void);
uint8_t hif_2_App_Interface_cb( uint8_t* pBuff,uint16_t len);

void process_set_brodcast_timing_info_ie(uint8_t *buf, uint16_t length);
void process_set_net_name_ie(uint8_t *buf, uint16_t length);
void process_set_pan_ie(uint8_t *buf, uint16_t length);
void process_set_gtk_hash_ie(uint8_t *buf, uint16_t length);
void process_set_panversion_ie(uint8_t *buf, uint16_t length);
void process_broadcast_ie_set_req(uint8_t *buf, uint16_t length);
void process_unicast_ie_set_req(uint8_t *buf, uint16_t length);
uint8_t gu2pcapp_node_config_set_conf( uint8_t status );
uint32_t ChangeEndianness(uint32_t value);
void send_mac_addr(uint8_t *buff , uint16_t len);
/*this for fan_api */
void process_set_node_start_stop(uint8_t *buf, uint16_t length);
//void process_set_phy(uint8_t *buf, uint16_t length);
void process_set_mac_reg_op_api(uint8_t *buf, uint16_t length);
void process_set_mac_explicit_api(uint8_t *buf, uint16_t length);
void process_set_mac_unicast_api(uint8_t *buf, uint16_t length);
void process_set_mac_unicast_api_tbc(uint8_t *buf, uint16_t length);
void process_set_mac_broadcast_api(uint8_t *buf, uint16_t length);
void process_set_mac_broadcast_api_tbc(uint8_t *buf, uint16_t length);
void process_set_mac_fixed_chan(uint8_t *buf, uint16_t length);
void process_set_lbr_mac_config(uint8_t *buf, uint16_t length);
void process_set_lbr_mac_config_tbc(uint8_t *buf, uint16_t length);
void process_set_lbr_mac_gtks_config(uint8_t *buf, uint16_t length);
void process_set_lbr_mac_gtks_config_tbc(uint8_t *buf, uint16_t length);
void process_reset_rpl_msg_rate(uint8_t *buf, uint16_t length);
void process_get_sec_keys(uint8_t *buf, uint16_t length);
void process_get_ip_address(uint8_t *buf, uint16_t length);
void send_udp_request(uint8_t *buf, uint16_t length);
void send_icmpv6_request(uint8_t *buf, uint16_t length);
//void send_icmpv6_request_tbc(uint8_t *buf, uint16_t length);
//void trigger_subscribe_packet(uint8_t *buf, uint16_t length);
void process_get_dodag_routers(uint8_t *buf, uint16_t length);
void process_get_neighbor_table(uint8_t *buf, uint16_t length);
//void process_set_pa_level(uint8_t *buf, uint16_t length);
void process_set_router_config(uint8_t *buf, uint16_t length);
void process_set_router_config_tbc(uint8_t *buf, uint16_t length);
void process_get_current_join_state(uint8_t *buf, uint16_t length);
//void process_set_mac_whitelist(uint8_t *buf, uint16_t length);
void process_set_mac_whitelist_ontbc(uint8_t *buf, uint16_t length);
//void process_set_revoaction_key(uint8_t *buf, uint16_t length);
void process_get_prefered_parents(uint8_t *buf, uint16_t length);
void process_edfe_frame_exchange_req(uint8_t *buf, uint16_t length);
//void process_get_mac_whitelist(uint8_t *buf, uint16_t length);
void process_start_network_scale_req(uint8_t *buf, uint16_t length);
//void enable_desmac_sec(uint8_t *buf, uint16_t length);;

/**
 *******************************************************************************
 ** \brief This function hook the ENET test harness entry function for incoming 
 **          messages from Host application 
 ** \param - 
 ** \retval - 
 *******************************************************************************/
void enet_test_harness_init( void );

typedef struct enet_msg_struct
{
    struct enet_msg_struct *next;   /**< make this queueable    */
    uint16_t data_length;           /**< length of this message */
    uchar data[1];  		/**< serialised primitive, then parameters */
}enet_msg_t;

typedef struct
{
  uint8_t channel_spacing;
  uint8_t unicast_dwell_interval;
  uint8_t unicast_channel_function;
  uint8_t bcast_dwell_interval;
  uint8_t bcast_channel_function;
  uint8_t parent_bs_ie_use;
  uint8_t routing_methood;
  uint8_t network_name_length;
  uint16_t num_chans;
  uint16_t bcast_sched_id;
  uint16_t pan_id;
  uint16_t pan_size;
  uint32_t ch0;
  uint32_t bcast_interval;
  uint8_t network_name[32];
  uint8_t gtk0[16];
  uint8_t gtk1[16];
  uint8_t gtk2[16];
  uint8_t gtk3[16];
  excluded_channels_t unicast_excluded_channels;
  excluded_channels_t bcast_excluded_channels;
  uint8_t gtkl;
  uint8_t channel_plan;
}fan_config_param_t;

#if(APP_NVM_FEATURE_ENABLED == 1)
typedef struct nvm_structure_tag
{
  uint8_t operational_mode;
  uint8_t node_type; 
  uint8_t phy_mode;
  uint8_t pa_level;
  uint16_t selected_channel;
  uint8_t self_ieee_addr[8];         /**< holds it extended address*/
#if (EFR32FG13P_LBR == 0x00)  
  uint8_t self_global_address[16];
#endif  
  uint8_t operatinng_country;
  uint8_t sybbol_rate;
  uint8_t board_reset;
  float modulation_index;
  uint8_t reg_domain;
  uint8_t oprating_class;
  int8_t rssi_threshold;
  uint8_t xtal_adjust;
  uint32_t serial_baudrate;
  fan_config_param_t fan_config_param;
}nvm_structure_t;

void update_parameter_from_nvm (nvm_structure_t store_nvm_param);

#endif

#ifdef __cplusplus
}
#endif
#endif /* _ENET_CONTROLLER_INTERFACE_H_ */
