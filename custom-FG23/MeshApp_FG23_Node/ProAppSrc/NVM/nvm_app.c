/** \file nvm_app.c
 *******************************************************************************
 ** \brief This file has the function to store and retrieve data from NVM
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

#if(APP_NVM_FEATURE_ENABLED == 1)

#include "stddef.h"
#if(APP_EXTERNAL_FLASH_FEATURE_ENABLED == 1) 
#include "mx25flash_spi.h"
#include "em_gpio.h"
#include "em_cmu.h"
#else
#include "eeprom_emulation.h"
#endif

#include "queue_latest.h"
#include "mac_config.h"
#include "mac.h"
#include "mac_interface_layer.h"
#include "ie_element_info.h"
#include "sm.h"
#include "network-manager.h"
#include "fan_app_test_harness.h"
#include "fan_mac_interface.h"
#include "mac_defs_sec.h"
#include "fan_mac_security.h"
#include "contiki-net.h"
#include "sw_timer.h"
#include "timer_service.h"
#include "fan_api.h"
#include "nvm_app.h"


/*****************************************************************************
******************************************************************************/ 
#if(APP_EXTERNAL_FLASH_FEATURE_ENABLED == 0) 
#define TOTAL_TOKENS                 128  // Raka : 10 - Aug- 2018 Do not use NVM
//#define NVM_EAPOL_TOKEN              110 //suneet :: store the eapol cread
#define NVM_MAC_JOIN_STATE_TOKEN     8 //suneet :: store mac join state info
#define NVM_MAC_SELF_INFO_TOKEN      140 //suneet :: store 
#define NVM_MAC_SEC_TOKEN            36 //suneet :: store information mac seurity
#define NVM_MAC_FRAME_COUNTER        2 //use for mac frame counter
////#define NVM_EAPOL_PARENT_TOKEN       84 //use for eapol parents and child information
#define NVM_DEV_DESC_DATA_TOKEN      14*5 //suneet :: store Dev Desc data
#define NVM_MAC_NBR_TOKEN            160*5 //suneet:: used for store Mac NBR TABLE
#define NVM_DS6_DATA_TOKEN           38*5 //use for ds6 table data
#define NVM_NODE_INFO_TOKEN           18*5  //use for store RPL DIO INFO
#if (EFR32FG13P_LBR == 0x00)
#define NVM_RPL_DIO_INFO_TOKEN       56*5  //use for store RPL DIO INFO
#define NVM_LINK_STATS_TOKEN         32*5       // use for nbr link states
#endif
#define NVM_MAC_WHITE_LIST_TOKEN     122  //use fo mac white list 


///* Define the non-volatile variables. */
//static EE_Variable_TypeDef token[TOTAL_TOKENS] = {0};  // Raka : 10 - Aug- 2018 Do not use NVM
//static EE_Variable_TypeDef MAC_INFO[NVM_MAC_NBR_TOKEN] = {0};
////static EE_Variable_TypeDef EAPOL_INFO[NVM_EAPOL_TOKEN] = {0};
//static EE_Variable_TypeDef JOIN_StATE[NVM_MAC_JOIN_STATE_TOKEN] = {0};
//static EE_Variable_TypeDef MAC_SELF_INFO[NVM_MAC_SELF_INFO_TOKEN] = {0};
//static EE_Variable_TypeDef MAC_SECURITY[NVM_MAC_SEC_TOKEN] = {0};
//static EE_Variable_TypeDef DEV_DESC[NVM_DEV_DESC_DATA_TOKEN] = {0};
//static EE_Variable_TypeDef MAC_FRAME_COUNTER[NVM_MAC_FRAME_COUNTER] = {0};
////static EE_Variable_TypeDef EAPOL_PAR_CHI_INFO[NVM_EAPOL_PARENT_TOKEN] = {0};
//static EE_Variable_TypeDef DS6_DATA_INFO[NVM_DS6_DATA_TOKEN] = {0};
//static EE_Variable_TypeDef DAO_INFO[NVM_NODE_INFO_TOKEN] = {0};
////static EE_Variable_TypeDef LINK_STATES_INFO[NVM_LINK_STATS_TOKEN] = {0};
//static EE_Variable_TypeDef WHITE_LIST[NVM_MAC_WHITE_LIST_TOKEN] = {0};

//void update_parameter_from_nvm (nvm_structure_t store_nvm_param);
//void update_parameter_to_nvm (nvm_structure_t *store_nvm_param);
//void update_nvm_parameter();
//void format_nvm();
#else
#define  START_NVM_FLASH_TARGET_ADDR  0x000E0000
#endif  //APP_EXTERNAL_FLASH_FEATURE_ENABLED
//extern uint8_t select_best_prefered_parent(void);


void add_ds6_nbr_from_nvm(uip_ds6_nbr_t *nbr);
extern void store_device_desc_from_nvm(device_descriptor_t *dev_desc);
extern void store_mac_nbr_from_nvm(mac_nbr_descriptor_t *p_nbr_desc);
extern void *app_bm_alloc(uint16_t length);      
extern void app_bm_free(uint8_t *pMem);
static nvm_structure_t store_nvm_param;
extern fan_mac_information_sm_t fan_mac_information_data;
extern self_info_fan_mac_t mac_self_fan_info;
extern fan_mac_security mac_key_list;
extern mac_security_data_t     mac_security_data;
extern fan_mac_nbr_t fan_mac_nbr;
extern white_list_t white_mac_list;
extern fan_nwk_manager_sm_t fan_nwk_manager_app;
extern void store_device_desc(device_descriptor_t *dev_desc);
//void nvm_store_dao_info(rpl_dag_t *dag,const uip_ipaddr_t *child_address, const uip_ipaddr_t *parents_address);
sw_tmr_t update_nvm_dur_tmr;
extern void fan_nwk_manager_init( );
extern void update_parameter_to_nvm (nvm_structure_t *store_nvm_param);
//extern void store_mac_key_table_from_nvm(key_descriptor_t *p_kd);

/*****************************************************************************
******************************************************************************/ 

void init_nvm(void)
{
#if(APP_EXTERNAL_FLASH_FEATURE_ENABLED == 0)   
  EE_Variable_TypeDef token[TOTAL_TOKENS] = {0};
  memset(&token[0],0,TOTAL_TOKENS);
  EE_Variable_TypeDef JOIN_StATE[NVM_MAC_JOIN_STATE_TOKEN] = {0};
  memset(&JOIN_StATE[0],0,NVM_MAC_JOIN_STATE_TOKEN);
  EE_Variable_TypeDef MAC_SECURITY[NVM_MAC_SEC_TOKEN] = {0};
  memset(&MAC_SECURITY[0],0,NVM_MAC_SEC_TOKEN);
  EE_Variable_TypeDef MAC_FRAME_COUNTER[NVM_MAC_FRAME_COUNTER] = {0};
  memset(&MAC_FRAME_COUNTER[0],0,NVM_MAC_FRAME_COUNTER);
  EE_Variable_TypeDef MAC_SELF_INFO[NVM_MAC_SELF_INFO_TOKEN] = {0};
  memset(&MAC_SELF_INFO[0],0,NVM_MAC_SELF_INFO_TOKEN);
  EE_Variable_TypeDef DEV_DESC[NVM_DEV_DESC_DATA_TOKEN] = {0};
  memset(&DEV_DESC[0],0,NVM_DEV_DESC_DATA_TOKEN);
  EE_Variable_TypeDef MAC_INFO[NVM_MAC_NBR_TOKEN] = {0};
  memset(&MAC_INFO[0],0,NVM_MAC_NBR_TOKEN);
  EE_Variable_TypeDef DS6_DATA_INFO[NVM_DS6_DATA_TOKEN] = {0};
  memset(&DS6_DATA_INFO[0],0,NVM_DS6_DATA_TOKEN);
  EE_Variable_TypeDef DAO_INFO[NVM_NODE_INFO_TOKEN] = {0};
  memset(&DAO_INFO[0],0,NVM_NODE_INFO_TOKEN);
#if (EFR32FG13P_LBR == 0x00)   
  EE_Variable_TypeDef LINK_STATES_INFO[NVM_LINK_STATS_TOKEN] = {0};
  memset(&LINK_STATES_INFO[0],0,NVM_LINK_STATS_TOKEN);
  EE_Variable_TypeDef RPL_DIO_INFO[NVM_RPL_DIO_INFO_TOKEN] = {0};
  memset(&RPL_DIO_INFO[0],0,NVM_RPL_DIO_INFO_TOKEN);
#endif  
    /* Initialize the eeprom emulator using 3 pages. */
    if ( !EE_Init(3) ) {
      
      /* If the initialization fails we have to take some measure
      * to obtain a valid set of pages. In this example we simply 
      * format the pages */
      EE_Format(3);
    }
    
    for(int i =0;i<TOTAL_TOKENS;i++)
    {
      EE_DeclareVariable(&token[i]);
    }
    
    for(int i =0;i<NVM_MAC_NBR_TOKEN;i++)
    {
      EE_DeclareVariable(&MAC_INFO[i]);
    }
//    
//    for(int i =0;i<NVM_EAPOL_TOKEN;i++)
//    {
//      EE_DeclareVariable(&EAPOL_INFO[i]);
//    }
//    
    for(int i =0;i<NVM_MAC_JOIN_STATE_TOKEN;i++)
    {
      EE_DeclareVariable(&JOIN_StATE[i]);
    }
    
    for(int i =0;i<NVM_MAC_SELF_INFO_TOKEN;i++)
    {
      EE_DeclareVariable(&MAC_SELF_INFO[i]);
    }
    
    for(int i =0;i<NVM_MAC_SEC_TOKEN;i++)
    {
      EE_DeclareVariable(&MAC_SECURITY[i]);
    }
    for(int i =0;i<NVM_DEV_DESC_DATA_TOKEN;i++)
    {
      EE_DeclareVariable(&DEV_DESC[i]);
    }
    for(int i =0;i<NVM_MAC_FRAME_COUNTER;i++)
    {
      EE_DeclareVariable(&MAC_FRAME_COUNTER[i]);
    }
////    for(int i =0;i<NVM_EAPOL_PARENT_TOKEN;i++)
////    {
////      EE_DeclareVariable(&EAPOL_PAR_CHI_INFO[i]);
////    }
    for(int i =0;i<NVM_DS6_DATA_TOKEN;i++)
    {
      EE_DeclareVariable(&DS6_DATA_INFO[i]);
    }
     for(int i =0;i<NVM_NODE_INFO_TOKEN;i++)
    {
      EE_DeclareVariable(&DAO_INFO[i]);
    }
#if (EFR32FG13P_LBR == 0x00)    
    for(int i =0;i<NVM_LINK_STATS_TOKEN;i++)
    {
      EE_DeclareVariable(&LINK_STATES_INFO[i]);
    }
    
    for(int i =0;i<NVM_RPL_DIO_INFO_TOKEN;i++)
    {
      EE_DeclareVariable(&RPL_DIO_INFO[i]);
    }
#endif    
    
    for(int i =0;i<NVM_MAC_WHITE_LIST_TOKEN;i++)
    {
      EE_DeclareVariable(&WHITE_LIST[i]);
    }
#endif  //APP_EXTERNAL_FLASH_FEATURE_ENABLED
}
/******************************************************************************/ 
void check_status_to_start_network()
{
   nvm_load_read_fan_join_info();
   if((fan_mac_information_data.state_ind == JOIN_STATE_5)
      &&(fan_mac_information_data.upper_layer_started == TRUE))
   {
     fan_nwk_manager_app.nvm_write_to_start = true;
   }
   
  if(fan_nwk_manager_app.nvm_write_to_start == true)
  {
    tmr_create_one_shot_timer
      (
       &update_nvm_dur_tmr,
       150000000,//3,min Delay
       (sw_tmr_cb_t)&update_nvm_parameter,
       NULL
         );
    
    nvm_load_read_node_basic_info();
    /*Loading Node basic configuration from EEAPROM */
    if((fan_mac_information_data.state_ind == JOIN_STATE_5)
       &&(fan_mac_information_data.upper_layer_started == TRUE))
    {
      fan_mac_information_data.is_start_from_nvm = true;
      fan_nwk_manager_init( );
    }
    else
    {
      fan_mac_information_data.is_start_from_nvm = false;
      fan_mac_information_data.upper_layer_started = false;
      tmr_start_relative(&update_nvm_dur_tmr);
    }
  }
  else
  {
     nvm_load_read_node_basic_info();
  }
}
/******************************************************************************/ 
/*Loading Node Basic Configuration From EEAPROM*/
void nvm_load_read_node_basic_info( void )
{
#if(APP_EXTERNAL_FLASH_FEATURE_ENABLED == 0)  
  EE_Variable_TypeDef token[TOTAL_TOKENS] = {0};
  memset(&token[0],0,TOTAL_TOKENS);
  uint16_t yy = 1;
  for(uint16_t ii = 0; ii < TOTAL_TOKENS; ++ii)
  {
    token[ii].virtualAddress = yy;
    yy++;
  }
  uint16_t i;
  uint16_t number_of_token = sizeof (store_nvm_param) / 2;
 
  memset ((uint8_t *)&store_nvm_param, 0, sizeof (store_nvm_param));
  uint16_t* p_node_basic_info = (uint16_t*)&(store_nvm_param);
  
  for (i = 0; i < number_of_token; i++)
  {      
    EE_Read(&token[i], p_node_basic_info);
    p_node_basic_info++;
  }
#else
    uint32_t  flash_addr = START_NVM_FLASH_TARGET_ADDR;
    memset ((uint8_t *)&store_nvm_param, 0, sizeof (store_nvm_param));
    uint8_t* p_node_basic_info = (uint8_t*)&(store_nvm_param);
    /* Read flash memory data to memory buffer */
    MX25_READ( flash_addr, p_node_basic_info, sizeof (store_nvm_param) );
#endif
  update_parameter_from_nvm(store_nvm_param);
}
/******************************************************************************/
/*Storeing Node Basic Configuration To EEAPROM*/
void nvm_store_node_basic_info( void )
{
#if(APP_EXTERNAL_FLASH_FEATURE_ENABLED == 0)  
    EE_Variable_TypeDef token[TOTAL_TOKENS] = {0};
    memset(&token[0],0,TOTAL_TOKENS);
    uint16_t yy = 1;
    for(uint16_t ii = 0; ii < TOTAL_TOKENS; ++ii)
    {
      token[ii].virtualAddress = yy;
      yy++;
    }
    uint16_t i;
    uint16_t number_of_token = sizeof (store_nvm_param) / 2;
  
  uint16_t* p_node_basic_info = (uint16_t*)&(store_nvm_param);
  
  memset (&store_nvm_param, 0, sizeof (store_nvm_param));
  update_parameter_to_nvm (&store_nvm_param); 
    for (i = 0; i < number_of_token; i++)
    { 
      EE_Write(&token[i], *p_node_basic_info);
      p_node_basic_info++;
    }
#else
  uint32_t  flash_addr = START_NVM_FLASH_TARGET_ADDR;
  uint8_t* p_node_basic_info = (uint8_t*)&(store_nvm_param);
  memset (&store_nvm_param, 0, sizeof (store_nvm_param));
  update_parameter_to_nvm (&store_nvm_param);
  MX25_PP( flash_addr, p_node_basic_info, sizeof (store_nvm_param) );
#endif
 
}
/******************************************************************************/
void nvm_load_read_mac_nbr(void)
{
#if(APP_EXTERNAL_FLASH_FEATURE_ENABLED == 0)   
  EE_Variable_TypeDef MAC_INFO[NVM_MAC_NBR_TOKEN] = {0};
  memset(&MAC_INFO[0],0,NVM_MAC_NBR_TOKEN);
  uint16_t yy = 1;
  for(uint16_t ii = 0 ; ii < NVM_MAC_NBR_TOKEN; ++ii)
  {
    MAC_INFO[ii].virtualAddress = TOTAL_TOKENS + NVM_MAC_JOIN_STATE_TOKEN + NVM_MAC_SEC_TOKEN + NVM_MAC_FRAME_COUNTER +NVM_MAC_SELF_INFO_TOKEN + NVM_DEV_DESC_DATA_TOKEN + yy;
    yy++;
  }
  
  uint16_t device_index = 0 ;
  EE_Read(&MAC_INFO[0], &device_index);
  uint16_t device_token = 0;
  while(device_index)
  {
    mac_nbr_descriptor_t *nbr_desc = NULL;
    nbr_desc = (mac_nbr_descriptor_t *)app_bm_alloc (sizeof (mac_nbr_descriptor_t));
    static uint16_t i = 0;
    uint16_t number_of_token = sizeof (nbr_desc[0]) / 2;
    device_token += number_of_token;
    memset (nbr_desc, 0, sizeof (nbr_desc[0]));
    uint16_t* mac_nbr_info = (uint16_t*)&(nbr_desc->index);
    
    for (; i < device_token; i++)
    {      
      EE_Read(&MAC_INFO[i+1], mac_nbr_info);
      mac_nbr_info++;
    }
    nbr_desc->nbrchannel_usable_list.broad_usable_channel_list = NULL;
    nbr_desc->nbrchannel_usable_list.unicast_usable_channel_list = NULL;
    store_mac_nbr_from_nvm(nbr_desc);
    device_index--;
  }
#else

#endif  
}
/******************************************************************************/
void nvm_store_write_mac_nbr()
{
#if(APP_EXTERNAL_FLASH_FEATURE_ENABLED == 0)   
  EE_Variable_TypeDef MAC_INFO[NVM_MAC_NBR_TOKEN] = {0};
  memset(&MAC_INFO[0],0,NVM_MAC_NBR_TOKEN);
  uint16_t yy = 1;
  for(uint16_t ii = 0 ; ii < NVM_MAC_NBR_TOKEN; ++ii)
  {
    MAC_INFO[ii].virtualAddress = TOTAL_TOKENS + NVM_MAC_JOIN_STATE_TOKEN + NVM_MAC_SEC_TOKEN + NVM_MAC_FRAME_COUNTER +NVM_MAC_SELF_INFO_TOKEN + NVM_DEV_DESC_DATA_TOKEN + yy;
    yy++;
  }
  
 
  mac_nbr_descriptor_t *p_nbr_desc = NULL;
  uint16_t ii = 0;
  uint16_t number_of_token = 0;
  uchar i = fan_mac_nbr.mac_nbr_info_table_entries;
  for(uchar j = 0;j<=i;j++)
  {
    p_nbr_desc =  (mac_nbr_descriptor_t*)queue_item_read_from (&fan_mac_nbr.desc_table, j);
    if (p_nbr_desc != NULL_POINTER)
    {
      EE_Write(&MAC_INFO[0], j+1);
      mac_nbr_descriptor_t *nbr_dev = &p_nbr_desc[0];
      number_of_token += sizeof (nbr_dev[0]) / 2;
      uint16_t* mac_nbr_info = (uint16_t*)&(nbr_dev->index);
      for (; ii < number_of_token; ii++)
      {             
        EE_Write(&MAC_INFO[ii+1], *mac_nbr_info);
        mac_nbr_info++;
      }  
    }
  }
#else

#endif  
}
/******************************************************************************/
//void nvm_load_read_eapol_info(void)
//{
//  uint16_t i;
//  uint16_t number_of_token = sizeof (supp_cred) / 2;
//  
//  memset ((uint8_t *)&supp_cred, 0, sizeof (supp_cred));
//  uint16_t* p_node_eapol_info = (uint16_t*)&(supp_cred);
//  
//  for (i = 0; i < number_of_token; i++)
//  {      
//    EE_Read(&EAPOL_INFO[i], p_node_eapol_info);
//    p_node_eapol_info++;
//  }
//  
//}
/******************************************************************************/
//void nvm_store_write_eapol_info(void)
//{
//  uint16_t i;
//  uint16_t number_of_token = sizeof (supp_cred) / 2;
//  uint16_t* p_node_eapol_info = (uint16_t*)&(supp_cred);
//  //  memset (&supp_cred, 0, sizeof (supp_cred));  
//  for (i = 0; i < number_of_token; i++)
//  {             
//    EE_Write(&EAPOL_INFO[i], *p_node_eapol_info);
//    p_node_eapol_info++;
//  }  
//}

/******************************************************************************/

void nvm_load_read_fan_join_info(void)
{
#if(APP_EXTERNAL_FLASH_FEATURE_ENABLED == 0)  
    EE_Variable_TypeDef JOIN_StATE[NVM_MAC_JOIN_STATE_TOKEN] = {0};
    memset(&JOIN_StATE[0],0,NVM_MAC_JOIN_STATE_TOKEN);
    uint16_t yy = 1;
    for(uint16_t ii = 0; ii < NVM_MAC_JOIN_STATE_TOKEN; ++ii)
    {
      JOIN_StATE[ii].virtualAddress = TOTAL_TOKENS + yy;
       yy++;
    }
    
    uint16_t i;
    int16_t number_of_token  = (1 + sizeof (fan_mac_information_data) / 2 );
  
    memset ((uint8_t *)&fan_mac_information_data, 0, sizeof (fan_mac_information_data));
    uint16_t* p_node_join_state_info = (uint16_t*)&(fan_mac_information_data);
    
    for (i = 0; i < number_of_token; i++)
    {      
      EE_Read(&JOIN_StATE[i], p_node_join_state_info);
      p_node_join_state_info++;
    }
#else
    uint32_t  flash_addr = START_NVM_FLASH_TARGET_ADDR + sizeof (store_nvm_param)+27;
    uint8_t* p_node_join_state_info = (uint8_t*)&(fan_mac_information_data);
    memset ((uint8_t *)&fan_mac_information_data, 0, sizeof (fan_mac_information_data));  
    MX25_READ( flash_addr, p_node_join_state_info, sizeof (fan_mac_information_data) );
#endif  
}

/******************************************************************************/

void nvm_store_write_fan_join_info(void)
{
#if(APP_EXTERNAL_FLASH_FEATURE_ENABLED == 0)  
  EE_Variable_TypeDef JOIN_StATE[NVM_MAC_JOIN_STATE_TOKEN] = {0};
  memset(&JOIN_StATE[0],0,NVM_MAC_JOIN_STATE_TOKEN);
  uint16_t yy = 1;
  for(uint16_t ii = 0 ; ii < NVM_MAC_JOIN_STATE_TOKEN; ++ii)
  {
    JOIN_StATE[ii].virtualAddress = TOTAL_TOKENS + yy;
    yy++;
  }
  
  if(fan_nwk_manager_app.nvm_write_to_start == true)
  {
    uint16_t i;
    uint16_t number_of_token  = (1 + sizeof (fan_mac_information_data) / 2 );
    uint16_t* p_node_join_state_info = (uint16_t*)&(fan_mac_information_data);
    //  memset (&supp_cred, 0, sizeof (supp_cred));  
    for (i = 0; i < number_of_token; i++)
    {             
      EE_Write(&JOIN_StATE[i], *p_node_join_state_info);
      p_node_join_state_info++;
    }  
  }
  else
  {
    //do nothing
  }
#else
  uint32_t  flash_addr = START_NVM_FLASH_TARGET_ADDR + sizeof (store_nvm_param)+27;
  uint8_t* p_node_join_state_info = (uint8_t*)&(fan_mac_information_data);
  MX25_PP( flash_addr, p_node_join_state_info, sizeof (fan_mac_information_data) );  
#endif  
  
}
/******************************************************************************/

void nvm_load_read_fan_macself_info(void)
{
#if(APP_EXTERNAL_FLASH_FEATURE_ENABLED == 0)  
  EE_Variable_TypeDef MAC_SELF_INFO[NVM_MAC_SELF_INFO_TOKEN] = {0};
  memset(&MAC_SELF_INFO[0],0,NVM_MAC_SELF_INFO_TOKEN);
  uint16_t yy = 1;
  for(uint16_t ii = 0 ; ii < NVM_MAC_SELF_INFO_TOKEN; ++ii)
  {
    MAC_SELF_INFO[ii].virtualAddress = TOTAL_TOKENS + NVM_MAC_JOIN_STATE_TOKEN + NVM_MAC_SEC_TOKEN + NVM_MAC_FRAME_COUNTER + yy;
    yy++;
  }
  
  uint16_t i;
  uint16_t number_of_token = sizeof (mac_self_fan_info) / 2;
  
  memset ((uint8_t *)&mac_self_fan_info, 0, sizeof (mac_self_fan_info));
  uint16_t* p_node_macself_info = (uint16_t*)&(mac_self_fan_info);
  
  for (i = 0; i < number_of_token; i++)
  {      
    EE_Read(&MAC_SELF_INFO[i], p_node_macself_info);
    p_node_macself_info++;
  }
#else

  
#endif  
}
/******************************************************************************/

void nvm_store_write_fan_macself_info(void)
{
#if(APP_EXTERNAL_FLASH_FEATURE_ENABLED == 0)  
  EE_Variable_TypeDef MAC_SELF_INFO[NVM_MAC_SELF_INFO_TOKEN] = {0};
  memset(&MAC_SELF_INFO[0],0,NVM_MAC_SELF_INFO_TOKEN);
  uint16_t yy = 1;
  for(uint16_t ii = 0 ; ii < NVM_MAC_SELF_INFO_TOKEN; ++ii)
  {
    MAC_SELF_INFO[ii].virtualAddress = TOTAL_TOKENS + NVM_MAC_JOIN_STATE_TOKEN + NVM_MAC_SEC_TOKEN + NVM_MAC_FRAME_COUNTER + yy;
    yy++;
  }
  
  if(fan_nwk_manager_app.nvm_write_to_start == true)
  {
    uint16_t i;
    uint16_t number_of_token = sizeof (mac_self_fan_info) / 2;
    uint16_t* p_node_macself_info = (uint16_t*)&(mac_self_fan_info);
    //  memset (&supp_cred, 0, sizeof (supp_cred));  
    for (i = 0; i < number_of_token; i++)
    {             
      EE_Write(&MAC_SELF_INFO[i], *p_node_macself_info);
      p_node_macself_info++;
    }  
  }
  else
  {
    
  }
#else

#endif  
  
}
/******************************************************************************/

void nvm_load_read_fan_macsecurity_info(void)
{
#if(APP_EXTERNAL_FLASH_FEATURE_ENABLED == 0)  
  EE_Variable_TypeDef MAC_SEC[NVM_MAC_SEC_TOKEN] = {0};
  memset(&MAC_SEC[0],0,NVM_MAC_SEC_TOKEN);
  uint16_t yy = 1;
  for(uint16_t ii = 0; ii < NVM_MAC_SEC_TOKEN; ++ii)
  {
    MAC_SEC[ii].virtualAddress = TOTAL_TOKENS + NVM_MAC_JOIN_STATE_TOKEN + yy;
    yy++;
  }
  
  uint16_t i;
  uint16_t number_of_token = 1+ sizeof (mac_key_list) / 2;
  
  memset ((uint8_t *)&mac_key_list, 0, sizeof (mac_key_list));
  uint16_t* p_node_macsecurity_info = (uint16_t*)&(mac_key_list);
  
  for (i = 0; i < number_of_token; i++)
  {      
    EE_Read(&MAC_SEC[i], p_node_macsecurity_info);
    p_node_macsecurity_info++;
  }
#else

#endif  
}
/******************************************************************************/

void nvm_store_write_fan_macsecurity_info(void)
{
#if(APP_EXTERNAL_FLASH_FEATURE_ENABLED == 0)  
  EE_Variable_TypeDef MAC_SEC[NVM_MAC_SEC_TOKEN] = {0};
  memset(&MAC_SEC[0],0,NVM_MAC_SEC_TOKEN);
  uint16_t yy = 1;
  for(uint16_t ii = 0 ; ii < NVM_MAC_SEC_TOKEN; ++ii)
  {
    MAC_SEC[ii].virtualAddress = TOTAL_TOKENS + NVM_MAC_JOIN_STATE_TOKEN + yy;
    yy++;
  }
  
  if(fan_nwk_manager_app.nvm_write_to_start == true)
  {
    uint16_t i;
    uint16_t number_of_token =  1+ sizeof (mac_key_list) / 2;  
    uint16_t* p_node_macsecurity_info = (uint16_t*)&(mac_key_list);
    //  memset (&supp_cred, 0, sizeof (supp_cred));  
    for (i = 0; i < number_of_token; i++)
    {             
      EE_Write(&MAC_SEC[i], *p_node_macsecurity_info);
      p_node_macsecurity_info++;
    }
  }
  else
  {
    //do nothing
  }
#else

#endif  
}
/******************************************************************************/

void nvm_load_read_fan_device_desc_info(void)
{
#if(APP_EXTERNAL_FLASH_FEATURE_ENABLED == 0)  
//  EE_Variable_TypeDef DEV_DESC[NVM_DEV_DESC_DATA_TOKEN] = {0};
//  memset(&DEV_DESC[0],0,NVM_DEV_DESC_DATA_TOKEN);
//  uint16_t yy = 1;
//  for(uint16_t ii = 0 ; ii < NVM_DEV_DESC_DATA_TOKEN; ++ii)
//  {
//    DEV_DESC[ii].virtualAddress = TOTAL_TOKENS + NVM_MAC_JOIN_STATE_TOKEN + NVM_MAC_SEC_TOKEN + NVM_MAC_FRAME_COUNTER +NVM_MAC_SELF_INFO_TOKEN + yy;
//    yy++;
//  }
//  
//  uint16_t device_index = 0 ;
//  EE_Read(&DEV_DESC[0], &device_index);
//  uint16_t device_token = 0;
//  while(device_index)
//  {
//    device_descriptor_t *p = NULL;
//    p = (device_descriptor_t *) app_bm_alloc(sizeof(device_descriptor_t));
//    static uint16_t i = 0;
//    uint16_t number_of_token = sizeof (p[0]) / 2;
//    device_token += number_of_token;
//    memset (p, 0, sizeof (p[0]));
//    uint16_t* dev_desc = (uint16_t*)&(p->ieee_addr[0]);
//    
//    for (; i < device_token; i++)
//    {      
//      EE_Read(&DEV_DESC[i+1], dev_desc);
//      dev_desc++;
//    }
//    store_device_desc_from_nvm(p);
//    device_index--;
//  }
#else

#endif  
  
}
/******************************************************************************/

void nvm_store_write_fan_device_desc_info()
{
#if(APP_EXTERNAL_FLASH_FEATURE_ENABLED == 0)  
//  EE_Variable_TypeDef DEV_DESC[NVM_DEV_DESC_DATA_TOKEN] = {0};
//  memset(&DEV_DESC[0],0,NVM_DEV_DESC_DATA_TOKEN);
//  uint16_t yy = 1;
//  for(uint16_t ii = 0 ; ii < NVM_DEV_DESC_DATA_TOKEN; ++ii)
//  {
//    DEV_DESC[ii].virtualAddress = TOTAL_TOKENS + NVM_MAC_JOIN_STATE_TOKEN + NVM_MAC_SEC_TOKEN + NVM_MAC_FRAME_COUNTER +NVM_MAC_SELF_INFO_TOKEN + yy;
//    yy++;
//  }
//  
//  device_descriptor_t *temp;
//  uint16_t ii = 0;
//  uint16_t number_of_token = 0;
//  uchar i = mac_security_data.pib.mac_device_table_entries;
//  for(uchar j = 0;j<=i;j++)
//  {
//    temp =  (device_descriptor_t*) queue_item_read_from( (queue_t *)&mac_security_data.pib.mac_device_table, 
//                                                        j);
//    if (temp != NULL_POINTER)
//    {
//      EE_Write(&DEV_DESC[0], j+1);
//      device_descriptor_t *dev_desc = &temp[0];
//      number_of_token += sizeof (dev_desc[0]) / 2;
//      uint16_t* dev_desc_info = (uint16_t*)&(dev_desc->ieee_addr[0]);
//      for (; ii < number_of_token; ii++)
//      {             
//        EE_Write(&DEV_DESC[ii+1], *dev_desc_info);
//        dev_desc_info++;
//      }  
//    }
//  }
#else

#endif  
}
/******************************************************************************/

void nvm_load_mac_frame_counter(void)
{
#if(APP_EXTERNAL_FLASH_FEATURE_ENABLED == 0)  
//  EE_Variable_TypeDef MAC_FRAME_COUNTER[NVM_MAC_FRAME_COUNTER] = {0};
//  memset(&MAC_FRAME_COUNTER[0],0,NVM_MAC_FRAME_COUNTER);
//  uint16_t yy = 1;
//  for(uint16_t ii = 0 ; ii < NVM_MAC_FRAME_COUNTER; ++ii)
//  {
//    MAC_FRAME_COUNTER[ii].virtualAddress = TOTAL_TOKENS + NVM_MAC_JOIN_STATE_TOKEN + NVM_MAC_SEC_TOKEN +yy;
//    yy++;
//  }
//  
//  uint16_t i;
//  uint16_t number_of_token = sizeof (mac_security_data.pib.mac_frame_counter) / 2;
//  
//  memset ((uint8_t *)&mac_security_data.pib.mac_frame_counter, 0, sizeof (mac_security_data.pib.mac_frame_counter));
//  uint16_t* mac_frame_counter = (uint16_t*)&(mac_security_data.pib.mac_frame_counter);
//  
//  for (i = 0; i < number_of_token; i++)
//  {      
//    EE_Read(&MAC_FRAME_COUNTER[i], mac_frame_counter);
//    mac_frame_counter++;
//  }
#else

#endif  
}
/******************************************************************************/

void nvm_store_write_mac_frame_counter(void)
{
#if(APP_EXTERNAL_FLASH_FEATURE_ENABLED == 0)  
  EE_Variable_TypeDef MAC_FRAME_COUNTER[NVM_MAC_FRAME_COUNTER] = {0};
  memset(&MAC_FRAME_COUNTER[0],0,NVM_MAC_FRAME_COUNTER);
  uint16_t yy = 1;
  for(uint16_t ii = 0 ; ii < NVM_MAC_FRAME_COUNTER; ++ii)
  {
    MAC_FRAME_COUNTER[ii].virtualAddress = TOTAL_TOKENS + NVM_MAC_JOIN_STATE_TOKEN + NVM_MAC_SEC_TOKEN +yy;
    yy++;
  }
  
  uint16_t i;
  uint16_t number_of_token = sizeof (mac_security_data.pib.mac_frame_counter) / 2;
  uint16_t* mac_frame_counter = (uint16_t*)&(mac_security_data.pib.mac_frame_counter);
  //  memset (&supp_cred, 0, sizeof (supp_cred));  
  for (i = 0; i < number_of_token; i++)
  {             
    EE_Write(&MAC_FRAME_COUNTER[i], *mac_frame_counter);
    mac_frame_counter++;
  }
#else

#endif  
}

/******************************************************************************/

//void nvm_load_eapol_parents_and_child_info(void)
//{
//  uint16_t i;
//  uint16_t number_of_token = sizeof (eapol_parent_child_info) / 2;
//  
//  memset ((uint8_t *)&eapol_parent_child_info, 0, sizeof (eapol_parent_child_info));
//  uint16_t* mac_eapol_parents_info = (uint16_t*)&(eapol_parent_child_info);
//  
//  for (i = 0; i < number_of_token; i++)
//  {      
//    EE_Read(&EAPOL_PAR_CHI_INFO[i], mac_eapol_parents_info);
//    mac_eapol_parents_info++;
//  }
//}

/******************************************************************************/

//void nvm_store_write_eapol_parents_and_child_info(void)
//{
//  uint16_t i;
//  uint16_t number_of_token = sizeof (eapol_parent_child_info) / 2;
//  uint16_t* mac_eapol_parents_info = (uint16_t*)&(eapol_parent_child_info);
//  //  memset (&supp_cred, 0, sizeof (supp_cred));  
//  for (i = 0; i < number_of_token; i++)
//  {             
//    EE_Write(&EAPOL_PAR_CHI_INFO[i], *mac_eapol_parents_info);
//    mac_eapol_parents_info++;
//  }  
//}
/******************************************************************************/

void nvm_load_ds6_info(void)
{
#if(APP_EXTERNAL_FLASH_FEATURE_ENABLED == 0)   
  EE_Variable_TypeDef DS6_DATA_INFO[NVM_DS6_DATA_TOKEN] = {0};
  memset(&DS6_DATA_INFO[0],0,NVM_DS6_DATA_TOKEN);
  uint16_t yy = 1;
  for(uint16_t ii = 0 ; ii < NVM_DS6_DATA_TOKEN; ++ii)
  {
    DS6_DATA_INFO[ii].virtualAddress = TOTAL_TOKENS + NVM_MAC_JOIN_STATE_TOKEN + NVM_MAC_SEC_TOKEN + NVM_MAC_FRAME_COUNTER +NVM_MAC_SELF_INFO_TOKEN + NVM_DEV_DESC_DATA_TOKEN + NVM_MAC_NBR_TOKEN + yy ;
    yy++;
  }
  
  uint16_t device_index = 0 ;
  EE_Read(&DS6_DATA_INFO[0], &device_index);
  uint16_t device_token = 0;
  while(device_index)
  {
    uip_ds6_nbr_t nbr;
    static uint16_t i = 0;
    uint16_t number_of_token = sizeof (nbr) / 2;
    device_token += number_of_token;
    memset (&nbr, 0, sizeof (nbr));
    uint16_t* ds6_nbr_info = (uint16_t*)&(nbr);
    
    for (; i < device_token; i++)
    {      
      EE_Read(&DS6_DATA_INFO[i+1], ds6_nbr_info);
      ds6_nbr_info++;
    }
    nbr.state = NBR_PROBE;
    add_ds6_nbr_from_nvm(&nbr);
    device_index--;
  }
#else

#endif  
}

/******************************************************************************/

void store_l3_data_after_join_state_5()
{
#if(APP_EXTERNAL_FLASH_FEATURE_ENABLED == 0)   
  EE_Variable_TypeDef DS6_DATA_INFO[NVM_DS6_DATA_TOKEN] = {0};
  memset(&DS6_DATA_INFO[0],0,NVM_DS6_DATA_TOKEN);
  uint16_t yy = 1;
  for(uint16_t ii = 0 ; ii < NVM_DS6_DATA_TOKEN; ++ii)
  {
    DS6_DATA_INFO[ii].virtualAddress = TOTAL_TOKENS + NVM_MAC_JOIN_STATE_TOKEN + NVM_MAC_SEC_TOKEN + NVM_MAC_FRAME_COUNTER +NVM_MAC_SELF_INFO_TOKEN + NVM_DEV_DESC_DATA_TOKEN + NVM_MAC_NBR_TOKEN + yy ;
    yy++;
  }
  
  uip_ds6_nbr_t *nbr = nbr_table_head(ds6_neighbors);
  uint16_t total_entry = 0;
  uint16_t i = 0;
  uint16_t total_device_token = 0;
  while(nbr != NULL) 
  {
    uip_ds6_nbr_t l_nbr = *nbr;
    total_entry++;
    EE_Write(&DS6_DATA_INFO[0], total_entry); 
    uint16_t number_of_token = sizeof (l_nbr) / 2;
    total_device_token += number_of_token;
    uint16_t* ds6_nbr = (uint16_t*)&(l_nbr);
    //  memset (&supp_cred, 0, sizeof (supp_cred));  
    for (; i < total_device_token; i++)
    {             
      EE_Write(&DS6_DATA_INFO[i+1], *ds6_nbr);
      ds6_nbr++;
    }  
    nbr = nbr_table_next(ds6_neighbors, nbr);
  }
#else

#endif  
}

/******************************************************************************/
#if (EFR32FG13P_LBR == 0x00) 
void nvm_load_rpl_dio_info(void)
{
#if(APP_EXTERNAL_FLASH_FEATURE_ENABLED == 0)   
  EE_Variable_TypeDef RPL_DIO_INFO[NVM_RPL_DIO_INFO_TOKEN] = {0};
  memset(&RPL_DIO_INFO[0],0,NVM_RPL_DIO_INFO_TOKEN);
  uint16_t yy = 1;
  for(uint16_t ii = 0 ; ii < NVM_RPL_DIO_INFO_TOKEN; ++ii)
  {
    RPL_DIO_INFO[ii].virtualAddress = TOTAL_TOKENS + NVM_MAC_JOIN_STATE_TOKEN + NVM_MAC_SEC_TOKEN + NVM_MAC_FRAME_COUNTER +NVM_MAC_SELF_INFO_TOKEN + NVM_DEV_DESC_DATA_TOKEN + NVM_MAC_NBR_TOKEN + NVM_DS6_DATA_TOKEN +NVM_NODE_INFO_TOKEN + NVM_LINK_STATS_TOKEN + yy ;
    yy++;
  }
  
  rpl_instance_t *instance;
  uint16_t device_index = 0 ;
  EE_Read(&RPL_DIO_INFO[0], &device_index);
  uint16_t device_token = 0;
  while(device_index)
  {
    uip_ipaddr_t from;
    rpl_dio_t dio;
    static uint16_t i = 0;
    uint16_t number_of_token = sizeof (from) / 2;
    device_token += number_of_token;
    memset (&from, 0, sizeof (from));
    uint16_t* rpl_dio_info = (uint16_t*)&(from);
    
    for (; i < device_token; i++)
    {      
      EE_Read(&RPL_DIO_INFO[i+1], rpl_dio_info);
      rpl_dio_info++;
    }
    device_token += sizeof (dio) / 2;
    memset (&dio, 0, sizeof (dio));
    rpl_dio_info = (uint16_t*)&(dio);
    for (; i < device_token; i++)
    {      
      EE_Read(&RPL_DIO_INFO[i+1], rpl_dio_info);
      rpl_dio_info++;
    }
    rpl_process_dio(&from, &dio);
    device_index--;
  }
  instance = rpl_get_instance(instance_table[0].instance_id);
  rpl_schedule_probing(instance);
  rpl_reset_dio_timer(instance);//for sending dio after receiving DAO-ACK input in router
  select_best_prefered_parent();
#else

#endif  
}
/******************************************************************************/

void nvm_store_write_rpl_dio_info(rpl_dio_t *dio,uip_ipaddr_t *from)
{
#if(APP_EXTERNAL_FLASH_FEATURE_ENABLED == 0)   
  EE_Variable_TypeDef RPL_DIO_INFO[NVM_RPL_DIO_INFO_TOKEN] = {0};
  memset(&RPL_DIO_INFO[0],0,NVM_RPL_DIO_INFO_TOKEN);
  uint16_t yy = 1;
  for(uint16_t ii = 0 ; ii < NVM_RPL_DIO_INFO_TOKEN; ++ii)
  {
    RPL_DIO_INFO[ii].virtualAddress = TOTAL_TOKENS + NVM_MAC_JOIN_STATE_TOKEN + NVM_MAC_SEC_TOKEN + NVM_MAC_FRAME_COUNTER +NVM_MAC_SELF_INFO_TOKEN + NVM_DEV_DESC_DATA_TOKEN + NVM_MAC_NBR_TOKEN + NVM_DS6_DATA_TOKEN +NVM_NODE_INFO_TOKEN + NVM_LINK_STATS_TOKEN + yy ;
    yy++;
  }
  
  static uint16_t total_entry = 0;
  uip_ds6_nbr_t *nbr;
  nbr = uip_ds6_nbr_lookup(from);
  static uint16_t i = 0;
  static uint16_t total_device_token = 0;
  if(nbr == NULL)
  {
    total_entry++;
    EE_Write(&RPL_DIO_INFO[0], total_entry);
   
    uint16_t number_of_token = sizeof (from[0]) / 2;
    total_device_token += number_of_token;
    uint16_t* rpl_dio_info = (uint16_t*)&(from[0]);
    //  memset (&supp_cred, 0, sizeof (supp_cred));  
    for (; i < total_device_token; i++)
    {             
      EE_Write(&RPL_DIO_INFO[i+1], *rpl_dio_info);
      rpl_dio_info++;
    }
    total_device_token += sizeof (dio[0]) / 2;
    rpl_dio_info = (uint16_t*)&(dio[0]);
    for (; i < total_device_token; i++)
    {             
      EE_Write(&RPL_DIO_INFO[i+1], *rpl_dio_info);
      rpl_dio_info++;
    }
  }
#else

#endif  
}

/******************************************************************************/

void nvm_load_link_stats_info(void)
{
#if(APP_EXTERNAL_FLASH_FEATURE_ENABLED == 0)   
  EE_Variable_TypeDef LINK_STATES_INFO[NVM_LINK_STATS_TOKEN] = {0};
  memset(&LINK_STATES_INFO[0],0,NVM_LINK_STATS_TOKEN);
  uint16_t yy = 1;
  for(uint16_t ii = 0 ; ii < NVM_LINK_STATS_TOKEN; ++ii)
  {
    LINK_STATES_INFO[ii].virtualAddress = TOTAL_TOKENS + NVM_MAC_JOIN_STATE_TOKEN + NVM_MAC_SEC_TOKEN + NVM_MAC_FRAME_COUNTER +NVM_MAC_SELF_INFO_TOKEN + NVM_DEV_DESC_DATA_TOKEN + NVM_MAC_NBR_TOKEN + NVM_DS6_DATA_TOKEN +NVM_NODE_INFO_TOKEN + yy ;
    yy++;
  }
  
  uint16_t device_index = 0 ;
  EE_Read(&LINK_STATES_INFO[0], &device_index);
  uint16_t device_token = 0;
  while(device_index)
  {
    struct link_stats nbr_stats;
    static uint16_t i = 0;
    uint16_t number_of_token = sizeof (nbr_stats) / 2;
    device_token += number_of_token;
    memset (&nbr_stats, 0, sizeof (nbr_stats));
    uint16_t* link_stats = (uint16_t*)&(nbr_stats);
    
    for (; i < device_token; i++)
    {      
      EE_Read(&LINK_STATES_INFO[i+1], link_stats);
      link_stats++;
    }
    add_link_stats_nbr_from_nvm(&nbr_stats);
    device_index--;
  }
#else

#endif  
}

/******************************************************************************/

void store_link_stats_data()
{
#if(APP_EXTERNAL_FLASH_FEATURE_ENABLED == 0)   
  EE_Variable_TypeDef LINK_STATES_INFO[NVM_LINK_STATS_TOKEN] = {0};
  memset(&LINK_STATES_INFO[0],0,NVM_LINK_STATS_TOKEN);
  uint16_t yy = 1;
  for(uint16_t ii = 0 ; ii < NVM_LINK_STATS_TOKEN; ++ii)
  {
    LINK_STATES_INFO[ii].virtualAddress = TOTAL_TOKENS + NVM_MAC_JOIN_STATE_TOKEN + NVM_MAC_SEC_TOKEN + NVM_MAC_FRAME_COUNTER +NVM_MAC_SELF_INFO_TOKEN + NVM_DEV_DESC_DATA_TOKEN + NVM_MAC_NBR_TOKEN + NVM_DS6_DATA_TOKEN +NVM_NODE_INFO_TOKEN + yy ;
    yy++;
  }
  
  rpl_parent_t *p;
  p = nbr_table_head(rpl_parents);
  uint16_t total_entry = 0;
  uint16_t i = 0;
  uint16_t total_device_token = 0;
  while(p != NULL) 
  {
    const struct link_stats *stats = rpl_get_parent_link_stats(p);
    struct link_stats nbr_stats = *stats;
    total_entry++;
    EE_Write(&LINK_STATES_INFO[0], total_entry); 
    uint16_t number_of_token = sizeof (nbr_stats) / 2;
    total_device_token += number_of_token;
    uint16_t* link_stats = (uint16_t*)&(nbr_stats);
    //  memset (&supp_cred, 0, sizeof (supp_cred));  
    for (; i < total_device_token; i++)
    {             
      EE_Write(&LINK_STATES_INFO[i+1], *link_stats);
      link_stats++;
    }  
     p = nbr_table_next(rpl_parents, p);
  }
#else

#endif  
}
#endif
/******************************************************************************/
#if (EFR32FG13P_LBR == 0x01) 
void nvm_load_rpl_dio_info(void)
{
#if(APP_EXTERNAL_FLASH_FEATURE_ENABLED == 0)   
  EE_Variable_TypeDef DAO_INFO[NVM_NODE_INFO_TOKEN] = {0};
  memset(&DAO_INFO[0],0,NVM_NODE_INFO_TOKEN);
  uint16_t yy = 1;
  for(uint16_t ii = 0 ; ii < NVM_NODE_INFO_TOKEN; ++ii)
  {
    DAO_INFO[ii].virtualAddress = TOTAL_TOKENS + NVM_MAC_JOIN_STATE_TOKEN + NVM_MAC_SEC_TOKEN + NVM_MAC_FRAME_COUNTER +NVM_MAC_SELF_INFO_TOKEN + NVM_DEV_DESC_DATA_TOKEN + NVM_MAC_NBR_TOKEN + NVM_DS6_DATA_TOKEN + yy ;
    yy++;
  }

  if(fan_nwk_manager_app.nvm_write_to_start == true)
  {
    rpl_instance_t *instance;
    rpl_dag_t *dag;
    uint8_t lifetime;
    uint16_t device_index = 0 ;
    EE_Read(&DAO_INFO[0], &device_index);
    uint16_t device_token = 0;
    instance = rpl_get_instance(instance_table[0].instance_id);
    dag = instance->current_dag;
    lifetime = instance->default_lifetime;
    while(device_index)
    {
      uip_ipaddr_t child_address;
      uip_ipaddr_t parents_address;
      static uint16_t i = 0;
      uint16_t number_of_token = sizeof (child_address) / 2;
      device_token += number_of_token;
      memset (&child_address, 0, sizeof (child_address));
      uint16_t* rpl_dao_info = (uint16_t*)&(child_address);
      
      for (; i < device_token; i++)
      {      
        EE_Read(&DAO_INFO[i+1], rpl_dao_info);
        rpl_dao_info++;
      }
      device_token += sizeof (parents_address) / 2;
      memset (&parents_address, 0, sizeof (parents_address));
      rpl_dao_info = (uint16_t*)&(parents_address);
      for (; i < device_token; i++)
      {      
        EE_Read(&DAO_INFO[i+1], rpl_dao_info);
        rpl_dao_info++;
      }
      rpl_ns_update_node(dag, &child_address, &parents_address, RPL_LIFETIME(instance, lifetime));
      device_index--;
    }
  }
#else

#endif  
}
#endif
/******************************************************************************/
void nvm_store_dao_info(rpl_dag_t *dag,const uip_ipaddr_t *child_address, const uip_ipaddr_t *parents_address)
{
#if(APP_EXTERNAL_FLASH_FEATURE_ENABLED == 0)   
  EE_Variable_TypeDef DAO_INFO[NVM_NODE_INFO_TOKEN] = {0};
  memset(&DAO_INFO[0],0,NVM_NODE_INFO_TOKEN);
  uint16_t yy = 1;
  for(uint16_t ii = 0 ; ii < NVM_NODE_INFO_TOKEN; ++ii)
  {
    DAO_INFO[ii].virtualAddress = TOTAL_TOKENS + NVM_MAC_JOIN_STATE_TOKEN + NVM_MAC_SEC_TOKEN + NVM_MAC_FRAME_COUNTER +NVM_MAC_SELF_INFO_TOKEN + NVM_DEV_DESC_DATA_TOKEN + NVM_MAC_NBR_TOKEN + NVM_DS6_DATA_TOKEN + yy ;
    yy++;
  }
  
  
  if(fan_nwk_manager_app.nvm_write_to_start == true)
  {
    rpl_ns_node_t *child_node = rpl_ns_get_node(dag, child_address);
    static uint16_t total_entry = 0;
    static uint16_t total_device_token = 0;
    static uint16_t i = 0;
    if(child_node == NULL)
    {
      total_entry++;
      EE_Write(&DAO_INFO[0], total_entry);
      
      uint16_t number_of_token = sizeof (child_address[0]) / 2;
      total_device_token += number_of_token;
      uint16_t* rpl_dao_info = (uint16_t*)&(child_address[0]);
      //  memset (&supp_cred, 0, sizeof (supp_cred));  
      for (; i < total_device_token; i++)
      {             
        EE_Write(&DAO_INFO[i+1], *rpl_dao_info);
        rpl_dao_info++;
      }
      total_device_token += sizeof (parents_address[0]) / 2;
      rpl_dao_info = (uint16_t*)&(parents_address[0]);
      for (; i < total_device_token; i++)
      {             
        EE_Write(&DAO_INFO[i+1], *rpl_dao_info);
        rpl_dao_info++;
      }
    }
  }
  else
  {
    //do nothing
  }
#else

#endif  
}
/******************************************************************************/
void nvm_load_mac_white_list_info( void )
{
//  if(fan_nwk_manager_app.nvm_write_to_start == true)
//  {
//    uint16_t i;
//    uint16_t number_of_token = 1 + sizeof (white_mac_list) / 2;
//    
//    memset ((uint8_t *)&white_mac_list, 0, sizeof (white_mac_list));
//    uint16_t* white_addr_list = (uint16_t*)&(white_mac_list);
//    
//    for (i = 0; i < number_of_token; i++)
//    {      
//      EE_Read(&WHITE_LIST[i], white_addr_list);
//      white_addr_list++;
//    }
//  }
//  else
//  {
//    //do nothing
//  }
}

/******************************************************************************/

void nvm_store_mac_white_list_info( void )
{
//  if(fan_nwk_manager_app.nvm_write_to_start == true)
//  {
//    uint16_t i;
//    uint16_t number_of_token = 1 + sizeof (white_mac_list) / 2;
//    uint16_t* white_addr_list = (uint16_t*)&(white_mac_list);
//    for (i = 0; i < number_of_token; i++)
//    {             
//      EE_Write(&WHITE_LIST[i], *white_addr_list);
//      white_addr_list++;
//    }  
//  }
//  else
//  {
//    //do nothing
//  }
}

/******************************************************************************/

void update_nvm_parameter()
{
 if(fan_nwk_manager_app.nvm_write_to_start == true)   
 {
  nvm_store_write_mac_nbr();
  nvm_store_write_fan_device_desc_info();
  nvm_store_write_mac_frame_counter();
  //nvm_store_write_eapol_parents_and_child_info();
  store_l3_data_after_join_state_5();
#if (EFR32FG13P_LBR == 0x00)   
  store_link_stats_data();
#endif  
  tmr_stop(&update_nvm_dur_tmr);
  tmr_start_relative(&update_nvm_dur_tmr);
 }
 else
 {
    //do nothing 
 }
  
}

/******************************************************************************/

void upload_parameter_from_nvm()
{
  if(fan_nwk_manager_app.nvm_write_to_start == true)  
  {
    nvm_load_mac_white_list_info();
    nvm_load_read_mac_nbr();
#if (EFR32FG13P_LBR == 0x00)     
    nvm_load_read_fan_macself_info();
#endif    
    //nvm_load_read_eapol_info();
    //nvm_load_eapol_parents_and_child_info();
    nvm_load_read_fan_macsecurity_info();
    nvm_load_read_fan_device_desc_info();
    nvm_load_mac_frame_counter();
#if (EFR32FG13P_LBR == 0x00)     
    store_mac_key_table_from_nvm();
#endif    
    node_start_upper_layer_ready();
    tmr_start_relative(&update_nvm_dur_tmr);
  }
  else
  {
    //do nothing
  }
}

/******************************************************************************/

void rpl_update_info_from_nvm()
{
  if(fan_nwk_manager_app.nvm_write_to_start == true)
  {
    if(fan_mac_information_data.is_start_from_nvm == true)
    {
      nvm_load_ds6_info();
      nvm_load_rpl_dio_info();
    }
  }
  else
  {
    //do nothing
  }
}

/******************************************************************************/

void format_nvm()    //suneet :: format nvm every time when its recieve start and stop command from  nvm 
{
#if(APP_EXTERNAL_FLASH_FEATURE_ENABLED == 0)   
  EE_Format(3);
#endif  
}
/******************************************************************************/
#endif
