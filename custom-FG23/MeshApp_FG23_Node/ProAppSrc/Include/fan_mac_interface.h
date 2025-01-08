/** \file fan_mac_interface.h
 *******************************************************************************
 ** \brief Provides information about the MAC Layer
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
#ifndef _FAN_MAC_INTERFACE_H
#define _FAN_MAC_INTERFACE_H

#ifdef __cplusplus
extern "C" {
#endif
  
#define BORDER_ROUTER_NODE      0
#define ROUTER_NODE             1
#define LEAF_NODE               2
  
#define PARSE_SUCCESS           1  
#if APP_LBR_ROUTER
#include "fan_config_param.h"
#endif
//#define ENABLE_FAN_MAC_WITHOUT_SECURITY   1//1 Umesh  // Value of 1 will send the packet without security
                                          //1 by default 
                                          //0 for with security
  
#define MAX_PAS_ATTEMENT                2  //Suneet :: MAX packet attement for recieving all nbr pa   

/*----------------------------------------------------------------------------*/
/**
 *******************************************************************************
 ** \enum fan_mac_state_t
 **  Specific triggers for states on fan mac
 *******************************************************************************
 **/
typedef enum
{
    FAN_MAC_TRIGGER_ENTRY = SM_TRIGGER_ENTRY, /**< Event to enter into a new state*/
    FAN_MAC_TRIGGER_EXIT  = SM_TRIGGER_EXIT,  /**< Event to exit from the current state*/
    FAN_SYN_PKT_SEND_ERROR,
    SEND_PAS_WS_SYNC_PKT,
    SEND_PA_WS_SYNC_PKT,
    SEND_PCS_WS_SYNC_PKT,
    SEND_PC_WS_SYNC_PKT,
    STOP_ASYNC_STOP_PKT,
    RECEIVED_PA_PKT,
    RECEIVED_PAS_PKT,
    RECEIVED_PCS_PKT,
    RECEIVED_PC_PKT,
#if(FAN_EAPOL_FEATURE_ENABLED == 1)
    RECEIVED_EAPOL_PACKET,
#endif
    UNICAST_LISTEN_SCHEDULE,
    RECEIVED_MAX_NBR_PA_PACKET,
}fan_mac_manager_sm_trigger_t;  

  
/*----------------------------------------------------------------------------*/
/**
 *******************************************************************************
 ** \enum fan_mac_state_t
 **  Specific triggers for states on fan mac
 *******************************************************************************
 **/
typedef enum
{
    FAN_MAC_IDLE_STATE,  
    FAN_MAC_STARTUP_STATE,
    JOIN_STATE_1,
    JOIN_STATE_2,
    JOIN_STATE_3,
    JOIN_STATE_4,
    JOIN_STATE_5
} fan_mac_state_t;
/*----------------------------------------------------------------------------*/
/* *******************************************************************************
 ** \struct neighbor_table_tag
 **  Structure that can store the heaader ie element and payload ie of a
 **  device
 *******************************************************************************
 **/
typedef struct fan_mac_information_sm_struct
{
    sm_t super;					/**< current state, represented as a general state machine */
    fan_mac_state_t state_ind;	/**< current state indicator */
    fan_mac_state_t previous_state;
    uint8_t fan_node_type;    
    bool upper_layer_started;
#if(APP_NVM_FEATURE_ENABLED == 1)
    bool is_start_from_nvm;
#endif
}fan_mac_information_sm_t;

extern fan_mac_information_sm_t fan_mac_information_data;

/*----------------------------------------------------------------------------*/
void fan_mac_init(uint8_t fan_node_type);
void send_ws_async_frame(uint8_t op_type, uint8_t frame_type, uint8_t* channel_list , uint8_t length);
void send_error_ws_async_pkt(void);
void add_ie_list_in_pan_adv_solicite_pkt(void);
void add_ie_list_in_pan_adv_pkt(void);
void add_ie_list_in_pan_config_pkt(void);
void add_ie_list_in_pan_solicite_pkt(void);
void add_ie_list_in_ulad_pkt(void);

#if(FAN_EAPOL_FEATURE_ENABLED == 1)
void add_ie_list_in_eapol_pkt(void);
void send_eapol_request(void);
#endif

void change_to_join_state_05(void);

typedef struct child_info_tag
{
  uint8_t child_addr[8];
}child_info_t;


#if APP_LBR_ROUTER
typedef struct parent_child_info_tag
{
  uint8_t sle_eapol_parent[8];
  child_info_t child_info[APP_CFG_MAX_DEV_SUPPORT];
}parent_child_info_tag;
#endif
enum
{  
        INITIAL_FRAME				= 0x00,
	RESPONSE_FRAME,
	FINAL_RESPONSE_FRAME,
};

#if(FAN_EDFE_FEATURE_ENABLED == 1)
typedef struct edfe_info
{
  uchar edfe_frame_enabled;
  uchar edfe_frame_tx_type;
  uchar edfe_frame_rx_type;
  uchar edfe_ini_mac_addr[8];
  uchar edfe_transmit_flow_contrl;
  uchar edfe_receiver_flow_contrl;
  uint16_t edfe_trigger_packt;
  uint16_t edfe_sent_pkt;
  uint8_t is_fragmented;
}edfe_info_t;


extern uchar create_edfe_frame(uint8_t *dest_addr,uint32_t sub_hdr_bitmap, uint32_t sub_pld_bitmap);
#endif

#if(APP_NVM_FEATURE_ENABLED == 1)
void nvm_load_read_node_basic_info( void );
void nvm_load_read_mac_nbr(void);
void nvm_load_read_fan_join_info(void);
void nvm_store_write_fan_join_info(void);
void nvm_load_read_fan_macself_info(void);
void nvm_store_write_fan_macself_info(void);
void nvm_load_read_fan_macsecurity_info(void);
void nvm_store_write_fan_macsecurity_info(void);
void nvm_load_mac_frame_counter(void);
void nvm_store_write_mac_frame_counter(void);
void nvm_store_write_fan_device_desc_info();
void nvm_load_read_fan_device_desc_info(void);
void upload_parameter_from_nvm();
void update_nvm_parameter();

#endif  //#if(APP_NVM_FEATURE_ENABLED == 1)

void store_l3_data_after_join_state_5(void);

#endif
