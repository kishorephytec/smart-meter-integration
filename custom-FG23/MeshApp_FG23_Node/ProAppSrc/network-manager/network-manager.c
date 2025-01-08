/*****************************************************************************
 * network-manager.c
 *****************************************************************************/

/** \file network-manager.c
 *******************************************************************************
 ** \brief This file is used to maintain the state machine and 
 **            802.15.4 MAC and PHY configurations
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

/**
 *****************************************************************************
 * @ingroup sysdoc
 *
 * @{
 *****************************************************************************/

/**
 *****************************************************************************
 * @defgroup RITSimpleDataNBpanc LE-RIT Data Transmission Demo-PAN Coordinator(non-beaconing)
 * @brief This module implements the PAN coordinator application for
 * demonstrating the Simple 16-bit and 64-bit addressing \n data transmission in a non-beacon
 * enabled network
 *
 * @attention This application is built over NO-SEC MAC library and Multi Band Multi Mode MRFSK-PHY library
 *
 * This application executes the following steps.\n\n
 * 1) Initialize the application variables by calling lrwpan_app_init()
 * and enter into  IDLE_STATE state\n\n
 * 2) Issue MLME-Reset.request primitive to the MAC-PHY Layer and wait for \n
 * MLME-Reset.confirm primitive.\n\n
 * 3) After getting  MLME-RESET.confirm primitive from MAC-PHY Layer, issue \n
 * MLME-SET.request primitive to set macIEEEAddress PIB which sets the device's \n
 * local 64-bit address for communication.\n\n
 * 4) Issue MLME-SET.request primitive to MAC-PHY layer for setting macShortAddress \n
 * PIB which sets the 16-bit short address used for communication.\n\n
 * 5) Issue MLME-Scan.request primitive to MAC layer to perform ED scanning on the \n
 * default channel list, for the default scan duration.\n\n
 * 6) Store the ED values reported in the MLME-Scan.confirm primitive issued by the \n
 * MAC-PHY Layer.  \n\n
 * 7) Issue MLME-Scan.request primitive for performing ACTIVE scanning on all the channels\n\n
 * 8) After getting MLME-Scan.confirm primitive from MAC-PHY Layer for the active scan request\n
 * select the best channel(least noisy) and a PAN ID which does not conflict with any of the \n
 * discovered network using ACTIVE scanning.\n\n
 * 9) Issue MLME-START.request primitive with the selected channel,the selected PAN ID,\n
 * PANCoordinator parameter set to TRUE and BO and SO set to 15 for establishing a \n
 * non-beacon enabled PAN coordinator.\n\n
 * 10) After getting MLME-START.confirm primitive with SUCCESS as the status from the MAC-PHY Layer,\n
 * issue MLME-SET.request for setting  macRxOnWhenIdle and macAssociationPermit \n
 * PIB attribute to TRUE. This is to keep the reciever ON and enable association \n
 * of new devices in the network respectively.\n\n
 * 11) Once the macAssociationPermit is set to TRUE successfully, PAN coordinator will \n
 * be in a state where it can receive associations from other devices.\n\n
 * 12) Accept association request from other devices and assign short addresses to the \n
 * attempting devices. Short addresses to be assigned as preconfirgured in child_address[] array.\n\n
 * 13) Issue MLME-SET.request primitive to set macLEenabled PIB to TRUE.\n\n
 * 14) Issue MLME-SET.request primitive to set the macRITDataWaitDuration PIB to DEF_RIT_DATA_WAIT_DURATION\n
 * value, and macRITTxWaitDuartion PIB to DEF_RIT_TX_WAIT_DURATION value and macRITPeriod to DEF_RIT_PERIOD\n
 * value.\n\n
 * 15) Set the macRITDataReqCmdConfig proprietary PIB to 1 value.\n\n
 * 16) This PAN Coordinator starts a timer to send data to its joined with the TxOption as indirect.\n
 * Once the timer expires the data request packet is put in the indirect queue and then the PAN Coordinator\n
 * keeps it receiver ON for macRITTxWaitDuartion duration and look only for RIT data request command frame from the\n
 * child device.\n\n
 * 17) After receiving the RIT data request command frame from the joined device,data is sent out and MCPS-Data.confirmation\n
 * is send from the MAC.\n\n
 * 18) This PAN Coordinator also receives MCPS-Data.indication primitive which carries the data \n
 * transmitted by the joined device.After receiving data from the child timer is started again to \n
 * send data to the joined device.\n\n
 ******************************************************************************/
 
/*
********************************************************************************
* File inclusion
********************************************************************************
*/

#include "StackAppConf.h"
#include "common.h"
#include "list_latest.h"
#include "queue_latest.h"
#include "buff_mgmt.h"
#include "uart_hal.h"
#include "hif_utility.h"
#include "hw_tmr.h"
#include "sw_timer.h"
#include "timer_service.h"
#include "phy.h"
#include "mac.h"
#include "fan_mac_ie.h"
#include "mac_app_build_config.h"
#include "mac_interface_layer.h"
#include "sm.h"
#include "event_manager.h"
#include "buffer_service.h"
#include "ie_element_info.h"
#include "fan_app_test_harness.h"
#include "network-manager.h"
#include "l3_configuration.h"
#include "l3_process_interface.h"   
#include "fan_mac_interface.h"
#include "l2_l3_interface.h"
#include "fan_api.h"
#include "fan_app_auto.h"
//#include "fan_factorycmd.h"
#include "fan_mac_security.h"
//#include "response_print.h"
#include "app_log.h"
/*
** =============================================================================
** Private Macro definitions
** =============================================================================
*/

/*
** =============================================================================
** Private Structures, Unions & enums Type Definitions
** =============================================================================
**/

/* None */

/*
** =============================================================================
** Private Variable Definitions
** =============================================================================
*/

/********************************
Operating country selection  ::: Raka :: 23- Nov-2016 ....

********************************/

opearating_zone_t opearating_zone_list[15] = {
  
{7, 0xFF,0x4B800000},   // USA and CANADA
{15, 0,0x4F800000 },    //USA (V2) AND MEXICO
{15, 1,0x4F880000 },    //BRAZIL
{15, 2,0x4F900000 },    //ANZ
{15, 3,0x4F980000 },    //EU#3
{15, 4,0x4FA00000 },    //PHILIPPINES
{15, 5,0x4FA80000 },    //MALAYSIA
{15, 7,0x4FB80000 },    //HONGKONG , SINGAPORE 1, THAILAND, VIETNAM
{8, 0xFF,0x4C000000},   //KOREA
{15, 6,0x4FB00000},     //CHINA
{9, 0xFF,0x4C800000 },  //JAPAN
{4, 0xFF,0x4A000000 },  //EU#1
{11, 0xFF,0x4D800000 }, //EU#2
{14, 0xFF,0x4F000000 }, // INDIA
{15, 8,0x4FC00000 }     //SINGAPORE #2
};

static uint8_t secrity_status_enab_dis = 0x00;
uint8_t router_addr[8] = {0x00};

/*
** =============================================================================
** Private Function Prototypes
** =============================================================================
*/
#if(APP_NWK_MANAGER_PROCESS_FEATURE_ENABLED == 1)
static uint8_t nwk_manager_process_event_handler(l3_process_event_t ev, l3_process_data_t data);
L3_PROCESS(nwk_manager_process, "FAN NWK Mgr process");
void fan_nwk_mgr_post_event( l3_process_event_t ev, uint8_t* data);

#endif

static void trigger_FAN_MAC_Start(void);
static sm_result_t node_idle(fan_nwk_manager_sm_t *s, const sm_event_t *e);
static sm_result_t node_startup(fan_nwk_manager_sm_t *s, const sm_event_t *e);
static sm_result_t node_resetting(fan_nwk_manager_sm_t *s, const sm_event_t *e);
static sm_result_t node_initializing(fan_nwk_manager_sm_t *s, const sm_event_t *e);
static sm_result_t dev_ready_without_scanning(fan_nwk_manager_sm_t *s, const sm_event_t *e);
static sm_result_t node_starting_network(fan_nwk_manager_sm_t *s, const sm_event_t *e);
static sm_result_t node_mac_ready(fan_nwk_manager_sm_t *s, const sm_event_t *e);
static sm_result_t node_mac_sec_set(fan_nwk_manager_sm_t *s, const sm_event_t *e);
uint32_t get_sun_page_value(void);
uint16_t get_sun_channel(void);


void trickle_timer_inconsistency_pc(void);
void update_key_table_entry( uchar table_entry);
extern void update_pan_version_from_eapol_parent (void);
extern void init_factory_mode();
void send_mac_security_set_request(uint8_t *pBuff,uint16_t len);
/*
** =============================================================================
** Private Function Definitions
** =============================================================================
*/

/* None */

/*
** =============================================================================
** External Variable Declarations
** =============================================================================
*/
#if(FAN_FRQ_HOPPING_FEATURE_ENABLED == 1)
extern void fan_freq_hop_start_hopping (void *data);
#endif

extern self_info_fan_mac_t mac_self_fan_info;


#if(FAN_EDFE_FEATURE_ENABLED == 1)
extern void start_edfe_responce_timer(uint64_t responce_wait);
extern void get_edfe_dst_ieee_addr(uint8_t *dest_ieee_addr);
extern void start_edfe_receiver_responce_timer(uint64_t responce_wait);
extern void stop_edfe_reciver_timer();
extern edfe_info_t edfe_information; 
void enable_disable_edfe_frame(uint8_t value,uint8_t edfe_frame_type);
void send_edfe_initial_frame(uint8_t *src_addr , uint8_t value,uint8_t edfe_frame_type);
uint8_t is_edfe_enable();
#endif

extern uint8_t MACSecKey1[16];

#if WITH_SECURITY 
//uint8_t neighbour_addr[8] = {LBR_ADDRESS};//for without epol with security
uint8_t neighbour_addr[8] = {ROUTER_ADDRESS};//for without epol with security
#endif

/*
** =============================================================================
** External Function Prototypes
** =============================================================================
*/
extern uint8_t response_laye_ID;
extern void app_bm_free(uint8_t *pMem  );
extern void start_upper_layer(uint8_t* lladdress);
extern void send_mac_addr(uint8_t *buff , uint16_t len);
extern uint8_t send_hif_conf_cb( uint8_t cmd_id,uint8_t status );
extern void trickle_timer_pcs_stop(void);
extern void set_mac_self_info_from_lbr(uint8_t *rec_ptr,uint16_t rec_len);
extern void FAN_MAC_MLME_SET_Request
(
        uint8_t ie_identifier,		        /* header ie or payload ie */
        uint8_t ie_subid, 			/* subid for each ie */	        
        uint16_t Length,		  	/* length of value */
        void *mac_value	                        /* pointer to the value*/
);
//extern void change_join_state(uint8_t attemt_index);
extern void find_lowest_pancost_from_nbr_table_for_mac_address(uint8_t *src_addr);
extern uint8_t generate_MAC_Security_Key (uint8_t live_gtk_key_index,uint16_t len);
extern void add_dev_desc_on_MAC_for_security(uint8_t* macAddrOfNeighbour);

/*Umesh : 31-01-2018*/
extern void  set_mac_frame_counter_cmd ( uint32_t frameCntr );
/*this not nedded this file*/
extern uint8_t relay_reply_flag;
extern void update_total_sending_dur_for_hr( void *s, void* tmr );
extern void tsm_alarm(void *s, void* tmr );
extern void send_next_pkt(void *s,void* tmr);
#if(FAN_EAPOL_FEATURE_ENABLED == 1)
extern void get_eapol_parent_address(uint8_t *eapol_parent);
#endif
extern void throttle_pc (void);
 
/*
** =============================================================================
** Public Variables Definitions
** =============================================================================
**/

bool upper_layer_started = false;
fan_nwk_manager_sm_t fan_nwk_manager_app;
#ifdef WITH_SECURITY
uint8_t key_id_index  = 0x00;//0xFF;//temp //Umesh
#else
uint8_t key_id_index  = 0xFF;
#endif
/*
** =============================================================================
** Public Function Prototypes
** =============================================================================
*/
/*Umesh :31-01-2018*/
//void App_MLME_ws_async_frame_Conf_cb( uint8_t status ,uint8_t async_frame_type );
/*this is already declaired in header file*/

void fan_nwk_manager_init( ); 
void set_mac_security_on_LBR(uint8_t *rec_buff, uint16_t len);  
void set_mac_security_on_router_node(uint8_t *rec_buff, uint16_t len);
void App_MLME_WS_ASYNC_FRAME_Ind_cb
(
     uint8_t status,
     uint8_t frame_type,
     mac_address_t *p_Srcaddr // Src address details
	
);


extern sm_result_t node_factory_mode_state(fan_nwk_manager_sm_t *s, const sm_event_t *e);

/*Umesh : 31-01-2018*/
void get_self_pairing_id(uint8_t* p_pairing_id);
void set_mac_security_enable_disable(uint8_t enable_security_flag); 
void set_net_name_ie();
void ws_asynch_frame_req();
/*this function not needed here not using*/
uint8_t get_current_join_state();
uint8_t process_telec_set_operating_country( uint8_t CountryCode  ) ;


void App_FAN_ack_Ind_cb( mac_address_t*  pSrcaddr,
    mac_address_t*  pDstaddr,
    uint8_t DSN,
    uint8_t rsl_value,
    uint8_t security_status
    );
void App_FAN_no_ack_Ind_cb (void);
/*
** =============================================================================
** Public Function Definitions
** =============================================================================
*/
#if(APP_NWK_MANAGER_PROCESS_FEATURE_ENABLED == 1)

L3_PROCESS_THREAD(nwk_manager_process, ev, data)
{
  L3_PROCESS_BEGIN();
  
  while(1) 
  {
    L3_PROCESS_YIELD();
    nwk_manager_process_event_handler(ev, data);
  }
  L3_PROCESS_END();
}
/*----------------------------------------------------------------------------*/

static uint8_t nwk_manager_process_event_handler(l3_process_event_t ev, l3_process_data_t data)
{
  if (ev == FAN_NWK_MGR_START )
  {
     fan_nwk_manager_init( );
  }
  else if (ev == FAN_NWK_MGR_START_UPPER_LAYER )
  {
     start_upper_layer(fan_nwk_manager_app.node_basic_cfg.self_ieee_addr);        
  }
  else if (ev == SEND_MAC_ADDRESS_TO_LINUX )
  {  
    //send_mac_addr(fan_nwk_manager_app.node_basic_cfg.self_ieee_addr,8);  
  }
  else if (ev == SEND_FAN_MAC_START_CMD )
  {
     fan_mac_init(fan_nwk_manager_app.node_basic_cfg.fan_device_type);       
  }
  else if (ev == MAC_2_NHLE_CONFIRMATION)
  {
      process_mil_msgs ();
  }
  return 0;
}
/*----------------------------------------------------------------------------*/
void fan_nwk_mgr_post_event( l3_process_event_t ev, uint8_t* data)
{
  l3_process_post(&nwk_manager_process, ev, data);
}

void node_start_upper_layer_ready(void)
{
  start_upper_layer(fan_nwk_manager_app.node_basic_cfg.self_ieee_addr);
  //fan_nwk_mgr_post_event(FAN_NWK_MGR_START_UPPER_LAYER, NULL);    
} 

void send_mac_address_to_linux(void)
{
  fan_nwk_mgr_post_event(SEND_MAC_ADDRESS_TO_LINUX, NULL);    
} 

void send_mac_2_nhle_conf (void)
{
  fan_nwk_mgr_post_event(MAC_2_NHLE_CONFIRMATION, NULL);    
} 
     
static void trigger_FAN_MAC_Start(void)
{
  fan_nwk_mgr_post_event(SEND_FAN_MAC_START_CMD, NULL); 

} 
#else

void node_start_upper_layer_ready(void)
{
  start_upper_layer(fan_nwk_manager_app.node_basic_cfg.self_ieee_addr);    
} 

static void trigger_FAN_MAC_Start(void)
{
  fan_mac_init(fan_nwk_manager_app.node_basic_cfg.fan_device_type);      

} 

void send_mac_2_nhle_conf (void)
{
  process_mil_msgs ();   
} 
   

#endif // #if(APP_NWK_MANAGER_PROCESS_FEATURE_ENABLED == 1)
/******************************************************************************/


/*----------------------------------------------------------------------------*/
void App_factory_mode_conf_cb(uint8_t status)
{
  sm_event_t e = { (sm_trigger_t) TSM_TRIGGER_TX_COMPLETE, { 0 }};
  e.param.scalar = status;
  SM_DISPATCH( (sm_t *) &fan_nwk_manager_app, &e );
}

/*----------------------------------------------------------------------------*/
uint8_t process_telec_set_operating_country( uint8_t CountryCode  )  
{   
    if(CountryCode == 0x0D)
    {
      uint8_t i = 0;
      uint8_t ext_freq_band_id = 0xFF; 
      uint32_t freq_band_id = 0xFFFFFFFF;
      uint32_t temp_val = 0xFFFFFFFF;
      fan_nwk_manager_app.node_basic_cfg.operatinng_country  = 0xFF;
      temp_val = CountryCode; //ChangeEndianness ((*(uint32_t *)CountryCode));
      freq_band_id = (uint32_t) opearating_zone_list[temp_val].current_phy_band_ID;
      
      if ( freq_band_id == 15)
        ext_freq_band_id = (uint8_t)opearating_zone_list[temp_val].current_ext_phy_band_ID;

      for(i=0;i<(MAX_REGULATORY_DOMAIN_SUPPORTED);i++)
      {
        //{14, 0xFF,0x4F000000 }
        if((14 == freq_band_id ) &&  (0xFF == ext_freq_band_id ))
        {
          fan_nwk_manager_app.node_basic_cfg.operatinng_country  = temp_val;  
        }
      }
    }    
    else
    {
      return 1;
    }
   return 0;
}

/*----------------------------------------------------------------------------*/
void fan_nwk_manager_init( )
{
    fan_nwk_manager_sm_t *app = &fan_nwk_manager_app;
  uint8_t def_chans[8] = {DEFAULT_CHANNEL_BIT_MAP_ARRAY};
  sm_event_t e = { (sm_trigger_t) TRIGGER_ENTRY, { 0 } };
  
  process_telec_set_operating_country( COUNTRY_CODE_VAL  )  ;
  
  memcpy((uint8_t*)&(app->def_chan_bit_map),def_chans,8);
  app->result = MAC_SUCCESS;
  //app->active_layer = DEF_LAYER_ID;
  if(fan_nwk_manager_app.node_basic_cfg.operational_mode == FACTORY_MODE)
  {
    app_log("Device is in FACTORY_MODE\r\n");
    process_telec_set_operating_country(fan_nwk_manager_app.node_basic_cfg.operatinng_country);
    init_factory_mode();
    app->super.state = (sm_state_t)&node_factory_mode_state;
    SM_DISPATCH((sm_t *)app, &e);
    send_hif_conf_cb(NODE_START_STOP_CONF,0x00);
  }
  else
  {
    app_log("Device is in RUN_MODE\r\n");
    app->super.state = (sm_state_t)&node_startup;
    SM_DISPATCH((sm_t *)app, &e);
  }
}


/*----------------------------------------------------------------------------*/
static sm_result_t node_idle(fan_nwk_manager_sm_t *s, const sm_event_t *e)
{
    node_basic_config_t* p_basic_conf = &(fan_nwk_manager_app.node_basic_cfg);

    switch ((fan_nwk_manager_sm_trigger_t) e->trigger)
    {
      case TRIGGER_ENTRY:
        s->state_ind = IDLE_STATE;
        break;
      
      case TRIGGER_START_NODE:  
        if( ((p_basic_conf->phy_mode != FSK_PHY_MODE_2) &&
          ( p_basic_conf->phy_mode != FSK_PHY_MODE_1 ))||
          (  p_basic_conf->selected_channel == 0xFFFF  ) ||
          (  p_basic_conf->selected_pan_id == 0xFFFF )            
          )
          {
            return NULL;
          }
        else
          {  
              sm_transit((sm_t *)s, (sm_state_t)&node_startup );
          }
        break;      

      default:
        break;          
    }
    return NULL;
}
/*----------------------------------------------------------------------------*/
static sm_result_t node_startup(fan_nwk_manager_sm_t *s, const sm_event_t *e)
{
    switch ((fan_nwk_manager_sm_trigger_t) e->trigger)
    {
      case TRIGGER_ENTRY:
        s->state_ind = STARTUP_STATE;

        if((fan_mac_information_data.state_ind == JOIN_STATE_5)
           
           #if(APP_NVM_FEATURE_ENABLED == 1)
         &&(fan_mac_information_data.is_start_from_nvm == true)
#endif
           )
          /*when ever device bootup from join state 5 no need to update mlme reset request*/
          MLME_RESET_Request( false  );
        else
        /*When ever a device wants to start or join network first it should reset
        its PAN by issuing MLME-Reset primtive to the MAC Layer*/
          MLME_RESET_Request( true  );

        sm_transit((sm_t *)s, (sm_state_t)&node_resetting );
        break;
      
      case SM_TRIGGER_EXIT:
        s->previous_state =  s->state_ind;
        break;

      case TRIGGER_START_NODE:
        break;

      default:
        break;
    }
    return NULL;
}
/*----------------------------------------------------------------------------*/
static sm_result_t node_resetting(fan_nwk_manager_sm_t *s, const sm_event_t *e)
{
    switch ((fan_nwk_manager_sm_trigger_t) e->trigger)
    {
      case TRIGGER_ENTRY:
        s->state_ind = RESETTING_STATE;
        break;

      case TRIGGER_START_NODE:
        break;    

      case TRIGGER_RESET_CONF:
        if( s->result == MAC_SUCCESS )
        {
          MLME_SET_Request
          (
              macIEEEAddress,
              0,
              IEEE_ADDRESS_LENGTH,
              s->node_basic_cfg.self_ieee_addr
          );
          
          sm_transit((sm_t *)s, (sm_state_t)&node_initializing );
        }
        else
        {
          sm_transit((sm_t *)s, (sm_state_t)&node_idle );
        }
        break;

      case SM_TRIGGER_EXIT:
        s->previous_state =  s->state_ind;
        break;
      
      default:
        break;
    }
    return NULL;
}
/*----------------------------------------------------------------------------*/
static sm_result_t node_initializing(fan_nwk_manager_sm_t *s, const sm_event_t *e)
{
    uint32_t current_page = 0;
  
    switch ((fan_nwk_manager_sm_trigger_t) e->trigger)
	{
            case TRIGGER_ENTRY:
                  s->state_ind = INITIALIZING_STATE;
                  break;
            case TRIGGER_START_NODE:

                
                  break;
            case TRIGGER_SET_CONF:
                /*Once the extended address is set then the short address is alloted*/
	    	if( s->result == MAC_SUCCESS )
	    	{
                   switch( e->param.scalar )
                   {
                      case macIEEEAddress:
                        
                            current_page  = get_sun_page_value();
                             MLME_SET_Request
                             (
                                  phyCurrentSUNPageEntry,
                                  0,
                                  4,
                                  &current_page
                             );                      
                     break;

                    case phyCurrentSUNPageEntry:
                      /* once set the phy current page then set operating class 
                      logic here*/

                          
                           s->self_short_addr = SELF_SHORT_ADDRESS;
                            MLME_SET_Request
                            (
                                 macShortAddress,
                                 0,
                                 SHORT_ADDRESS_LENGTH,
                                 &(s->self_short_addr)
                            );                     
                      break;
                      
                    case macShortAddress:
                     
                        s->one_byte_value = 0x0; //false

                            MLME_SET_Request
                            (
                                 macAutoRequest,
                                 0,
                                 0x01,
                                 &(s->one_byte_value)
                            );
                        break;
                         
                    case macAutoRequest:
                      // this was set since we are trying to create a PAN coordinator, so keep the receiver ON when idle
                           s->one_byte_value = 1;
                           MLME_SET_Request
                             (
                                     macRxOnWhenIdle,
                                     0,
                                     1,
                                     &(s->one_byte_value)
                             );
                       break;
                       
                    case macRxOnWhenIdle:
                         sm_transit((sm_t *)s, (sm_state_t)&dev_ready_without_scanning );
                     return NULL;
                     
                      break;
                    
                    default:
                      break;
                        }	    		
	    	}
	    	else
	    	{
                  sm_transit((sm_t *)s, (sm_state_t)&node_idle );
	    	}
	    	break;
          
          case SM_TRIGGER_EXIT:
            s->previous_state =  s->state_ind;
          break;
          
	     default:
	     	break;
	}
	return NULL;
}
/*----------------------------------------------------------------------------*/


static sm_result_t dev_ready_without_scanning(fan_nwk_manager_sm_t *s, const sm_event_t *e)
{
   
        uint8_t parentAddr[8] ={0};
	switch ((fan_nwk_manager_sm_trigger_t) e->trigger)
	{
		case TRIGGER_ENTRY:
                    s->state_ind = DEV_READY_WITHOUT_SCANNING_STATE;
                    
                    MLME_SET_Request
                              (
                                   macCoordExtendedAddress,
                                   0,
                                   IEEE_ADDRESS_LENGTH,
                                   (uint8_t*)(parentAddr)
                              );
        
                    break;
                    
          case TRIGGER_SET_CONF:
              /*Once the extended address is set then the short address is alloted*/
               if( s->result == MAC_SUCCESS )
               {
                 switch( e->param.scalar )
                 {
                 case macCoordExtendedAddress:
                   
                   MLME_SET_Request
                     (
                      macPANId,
                      0,
                      SHORT_ADDRESS_LENGTH,
                      &(s->node_basic_cfg.selected_pan_id)
                        );
                   
                   break;
                 case macPANId:  
                   {
                     uint16_t channel = get_sun_channel();
                     MLME_SET_Request
                       (
                        phyCurrentChannel,
                        0,
                        2,
                        &channel
                          );
                   }
                   break;
                 case phyCurrentChannel:                           
                   {
                     sm_transit((sm_t *)s, (sm_state_t)&node_starting_network );
                   }
                   break;          
                 default:
                   break;
                    }
               }
               break;
               
   
          case SM_TRIGGER_EXIT:
            s->previous_state =  s->state_ind;
          break;
	     default:
	     	break;
	}
	return NULL;
}
/*----------------------------------------------------------------------------*/
static sm_result_t node_starting_network(fan_nwk_manager_sm_t *s, const sm_event_t *e)
{
    switch ((fan_nwk_manager_sm_trigger_t) e->trigger)
    {
    case TRIGGER_ENTRY:
      {
        s->state_ind = STARTING_NETWORK_STATE;
        
        if((fan_mac_information_data.state_ind == JOIN_STATE_5)
#if(APP_NVM_FEATURE_ENABLED == 1)
           &&(fan_mac_information_data.is_start_from_nvm == true)
#endif
             )
        {
          sm_event_t e = { (sm_trigger_t) TRIGGER_NWK_START_CONF, { 0 } };
          SM_DISPATCH((sm_t *)&fan_nwk_manager_app, &e);
        }  
        else
          /*This function now starts a network on the selected channel and PAN */ 
          FAN_MAC_MLME_SET_Request
            (
             WISUN_INFO_PAYLOAD_IE_ID,/* header ie or payload ie */
             WISUN_IE_SUBID_NETNAME_IE,/* subid for each ie */	  
             fan_nwk_manager_app.node_basic_cfg.netname_ie.length , /* length of value */
             (uint8_t*)(&fan_nwk_manager_app.node_basic_cfg.netname_ie.network_name)		  	
               /* pointing to first element*/
               ); 
      }
      break;
      
      case TRIGGER_START_NODE:
      break;
      
      case TRIGGER_NWK_START_CONF:
      {
        /*Suneet :: when run with tbc its send success before uper layer started problem come to fatch get ip address on lbr side  */
        if( fan_nwk_manager_app.node_basic_cfg.fan_device_type == COORD_NODE_TYPE )
          // Send Start Confirmation to Rest Server .. RAKA [16-11-2017]
          send_hif_conf_cb(NODE_START_STOP_CONF,0x00);
#if(APP_NVM_FEATURE_ENABLED == 1)
          nvm_store_node_basic_info(); //Debdeep
#endif
          // Raka :: For Enable Data Whiting             
          uint32_t val = ENABLE_DATA_WHITENING;
          PLME_set_request(phyFSKScramblePSDU,1,&val);                                 
          /*Once the network is successfully established the device keeps
          it receiver ON by setting the macRxOnWhenIdle PIB to TRUE*/
          s->one_byte_value = 0x01;

          MLME_SET_Request
          (
                macAssociationPermit,
                0,
                1,
                &(s->one_byte_value)
                );
          }
      break;

      case TRIGGER_RESET_CONF:
      break;

      case TRIGGER_SET_CONF:
      if( s->result == MAC_SUCCESS )
      {
        if( e->param.scalar == macAssociationPermit  )
        {
          s->one_byte_value = 0x00;

          MLME_SET_Request
          (
                macBeaconAutoRespond,
                0,
                1,
                &(s->one_byte_value)
                );
        }
        else if( e->param.scalar == macBeaconAutoRespond )
        {
            /*If the device is started as a PAN Cordinator then
            it has successfully formed the network now,else if
            the device was trying to join then it has successfully joined*/
          
          sm_event_t e = { (sm_trigger_t) TRIGGER_ENTRY, { 0 } };
          s->super.state = (sm_state_t)&node_mac_sec_set;
          SM_DISPATCH((sm_t *)s, &e);  
//            sm_event_t e = { (sm_trigger_t) TRIGGER_FAN_MAC_INIT, { 0 } };
//            s->super.state = (sm_state_t)&node_mac_ready;
//            SM_DISPATCH((sm_t *)s, &e);          
        }
      }
      else
      {
        sm_transit((sm_t *)s, (sm_state_t)&node_idle );
      }
      break;
      
      case   TRIGGER_SET_FAN_MAC_IE_CONF:
      if( s->result == MAC_SUCCESS )
      {
        switch( e->param.scalar )
        {
          case WISUN_IE_SUBID_NETNAME_IE: 
            FAN_MAC_MLME_SET_Request
            (
                WISUN_INFO_HEADER_IE_ID,/* header ie or payload ie */
                WISUN_IE_SUBID_BT_IE,/* subid for each ie */	        
                fan_nwk_manager_app.fan_mac_header_ie.bt_ie.length,/* length of value */
                (uint8_t*)&fan_nwk_manager_app.fan_mac_header_ie.bt_ie		  	
                /* pointing to first element*/
            ); 
          break;

          case WISUN_IE_SUBID_BT_IE:
            FAN_MAC_MLME_SET_Request
            (
                WISUN_INFO_PAYLOAD_IE_ID,/* header ie or payload ie */
                WISUN_IE_SUBID_PAN_IE,/* subid for each ie */	
                8,/*(7+1)=(fan_nwk_manager_app.node_basic_cfg.pan_ie.length+1), pointing to first element*/
                (uint8_t *)&fan_nwk_manager_app.node_basic_cfg.pan_ie/* length of value */
            );  
          break;
          
          case WISUN_IE_SUBID_PAN_IE:
            FAN_MAC_MLME_SET_Request
            (
                WISUN_INFO_PAYLOAD_IE_ID,/* header ie or payload ie */
                WISUN_IE_SUBID_GTKHASH_IE,/* subid for each ie */	        
                65,/*(64+1)=(fan_nwk_manager_app.node_basic_cfg.gtkhash_ie.length+1),*/
                (uint8_t *)& fan_nwk_manager_app.node_basic_cfg.gtkhash_ie
            ); 
          break;

          case WISUN_IE_SUBID_GTKHASH_IE:
            FAN_MAC_MLME_SET_Request
            (
                WISUN_INFO_PAYLOAD_IE_ID,/* header ie or payload ie */
                WISUN_IE_SUBID_PANVER_IE,/* subid for each ie */	 
                3,/*(2+1)=(fan_nwk_manager_app.node_basic_cfg.panvar_ie.length+1),*/
                (uint8_t*)&fan_nwk_manager_app.node_basic_cfg.panvar_ie
            ); 
          break;

          case WISUN_IE_SUBID_PANVER_IE:
            FAN_MAC_MLME_SET_Request
            (
                WISUN_INFO_PAYLOAD_IE_ID,/* header ie or payload ie */
                WISUN_IE_SUBID_BS_IE,/* subid for each ie */	        
                sizeof(fan_nwk_manager_app.node_basic_cfg.bs_ie),
                (uint8_t *)& fan_nwk_manager_app.node_basic_cfg.bs_ie
            ); 
          break;

          case WISUN_IE_SUBID_BS_IE:
            FAN_MAC_MLME_SET_Request
            (
                WISUN_INFO_PAYLOAD_IE_ID,/* header ie or payload ie */
                WISUN_IE_SUBID_US_IE,/* subid for each ie */	        
                sizeof(fan_nwk_manager_app.node_basic_cfg.us_ie),
                (uint8_t *)& fan_nwk_manager_app.node_basic_cfg.us_ie
            ); 
          break;
          
          case WISUN_IE_SUBID_US_IE:   
            {
                sm_event_t e = { (sm_trigger_t) TRIGGER_NWK_START_CONF, { 0 } };
                SM_DISPATCH((sm_t *)&fan_nwk_manager_app, &e);
            }
          break;  
          default:
          break;
        }
      }
      else
      {
        sm_transit((sm_t *)s, (sm_state_t)&node_idle );
      } 
      break;
      
      case SM_TRIGGER_EXIT:
      s->previous_state =  s->state_ind;
      break;
      
      default:
      break;
    }
    return NULL;
}
/*----------------------------------------------------------------------------*/

static sm_result_t node_mac_ready(fan_nwk_manager_sm_t *s, const sm_event_t *e)
{
#if (EFR32FG13P_LBR == 0x00)
    switch ((fan_nwk_manager_sm_trigger_t) e->trigger)
    {
      case TRIGGER_ENTRY:
            s->state_ind = NODE_MAC_READY_STATE;  
            break;

      case TRIGGER_RESET_CONF:
          break;
          
      case TRIGGER_CHANGE_JOIN_STATE:
           change_join_state(0);
           if(fan_nwk_manager_app.node_basic_cfg.fan_device_type == LBR_TYPE)
           {
              sm_transit((sm_t *)s, (sm_state_t)&node_mac_ready );
           }
          break;
          
    case TRIGGER_FAN_MAC_INIT:
            
            trigger_FAN_MAC_Start();
          break;
      
      case SM_TRIGGER_EXIT:
            s->previous_state =  s->state_ind;
            break;

      case TRIGGER_SET_CONF:
      case   TRIGGER_SET_FAN_MAC_IE_CONF:  
           if( s->result == MAC_SUCCESS )
           {
                switch( e->param.scalar )
                {
                      case macSecurityEnabled:
                            send_hif_conf_cb(MAC_SECURITY_ENABLE_DIASBLE_CONF,0x00);
                            break;  
                      case macDeviceTable:
                        // Raka .. if the device table is set for second device in the mac successfully 
                        // you will come here...
                        // this will come after the call of
                        // add_dev_desc_on_MAC_for_security(&router_addr[0]);
                            break;
                      case WISUN_IE_SUBID_GTKHASH_IE: 
                         //you will come here...
                        // this will come after the call of
                        // set_lbr_mac_gtks_config()
                            fan_nwk_manager_app.node_basic_cfg.panvar_ie.PANVERSION++;
                            FAN_MAC_MLME_SET_Request
                            (
                                WISUN_INFO_PAYLOAD_IE_ID,/* header ie or payload ie */
                                WISUN_IE_SUBID_PANVER_IE,/* subid for each ie */	 
                                3,/*(2+1)=(fan_nwk_manager_app.node_basic_cfg.panvar_ie.length+1),*/
                                (uint8_t*)&fan_nwk_manager_app.node_basic_cfg.panvar_ie
                            ); 
                           break; 
                           
                      case WISUN_IE_SUBID_PANVER_IE:
                        trickle_timer_inconsistency_pc ();
                        throttle_pc ();
                            break;
                      default:
                            break;
                }
           }
      break;

      default:
          break;
    }
    return NULL;
#else  
    switch ((fan_nwk_manager_sm_trigger_t) e->trigger)
    {
      case TRIGGER_ENTRY:
            s->state_ind = NODE_MAC_READY_STATE;  
            break;

      case TRIGGER_RESET_CONF:
          break;
          
      case TRIGGER_CHANGE_JOIN_STATE:
           change_join_state(0);
           if(fan_nwk_manager_app.node_basic_cfg.fan_device_type == LBR_TYPE)
           {
             #if(FAN_FRQ_HOPPING_FEATURE_ENABLED == 1)
              fan_freq_hop_start_hopping(NULL);
              #endif
              sm_transit((sm_t *)s, (sm_state_t)&node_mac_ready );
           }
          break;
          
    case TRIGGER_FAN_MAC_INIT: 
      if((fan_mac_information_data.state_ind == JOIN_STATE_5)
#if(APP_NVM_FEATURE_ENABLED == 1)
         &&(fan_mac_information_data.is_start_from_nvm == true)
#endif
           )
      {
#if(APP_NVM_FEATURE_ENABLED == 1)
        nvm_load_read_fan_macself_info();
#endif
        set_mac_security_on_LBR(&neighbour_addr[0],8);
        fan_mac_init(fan_nwk_manager_app.node_basic_cfg.fan_device_type);
//        fan_freq_hop_start_hopping(NULL);
        //change_join_state(0);      
      }
      else
      {
          fan_mac_init(fan_nwk_manager_app.node_basic_cfg.fan_device_type);

          trigger_FAN_MAC_Start();

#if(FAN_FRQ_HOPPING_FEATURE_ENABLED == 1)
          fan_freq_hop_start_hopping(NULL);
#endif
      }
            
          break;
      
      case SM_TRIGGER_EXIT:
            s->previous_state =  s->state_ind;
            break;

      case TRIGGER_SET_CONF:
      case   TRIGGER_SET_FAN_MAC_IE_CONF:  
           if( s->result == MAC_SUCCESS )
           {
#if(APP_NVM_FEATURE_ENABLED == 1)
                nvm_load_mac_frame_counter();
#endif
                switch( e->param.scalar )
                {
                      case macSecurityEnabled:
                            send_hif_conf_cb(MAC_SECURITY_ENABLE_DIASBLE_CONF,0x00);
                            break;  
                      case macDeviceTable:
                        // Raka .. if the device table is set for second device in the mac successfully 
                        // you will come here...
                        // this will come after the call of
                         add_dev_desc_on_MAC_for_security(&router_addr[0]);
                            break;
                      case WISUN_IE_SUBID_GTKHASH_IE: 
                         //you will come here...
                        // this will come after the call of
                        // set_lbr_mac_gtks_config()
                            fan_nwk_manager_app.node_basic_cfg.panvar_ie.PANVERSION++;
                            FAN_MAC_MLME_SET_Request
                            (
                                WISUN_INFO_PAYLOAD_IE_ID,/* header ie or payload ie */
                                WISUN_IE_SUBID_PANVER_IE,/* subid for each ie */	 
                                3,/*(2+1)=(fan_nwk_manager_app.node_basic_cfg.panvar_ie.length+1),*/
                                (uint8_t*)&fan_nwk_manager_app.node_basic_cfg.panvar_ie
                            ); 
                           break; 
                           
                      case WISUN_IE_SUBID_PANVER_IE:
                        trickle_timer_inconsistency_pc ();
                        throttle_pc ();
                            break;
                      default:
                            break;
                }
           }
      break;

      default:
          break;
    }
    return NULL;
#endif    
}


/*----------------------------------------------------------------------------*/


static sm_result_t node_mac_sec_set(fan_nwk_manager_sm_t *s, const sm_event_t *e)
{
   uint8_t enable_security_flag = 0x01;
   uint8_t enable_mac_frame_counter_perkey = 0x01;
  switch ((fan_nwk_manager_sm_trigger_t) e->trigger)
    { 
      case TRIGGER_ENTRY:
      s->state_ind = NODE_SET_SECURITY_PARAM_STATE;
              MLME_SET_Request
              (
                  macSecurityEnabled,		
                  0,
                  0x0001,	
                  &enable_security_flag	
              );
      break;
      case TRIGGER_RESET_CONF:
      break;
      
      case SM_TRIGGER_EXIT:
      s->previous_state =  s->state_ind;
      break;
      case TRIGGER_SET_CONF:
              /*Once the extended address is set then the short address is alloted*/
               if( s->result == MAC_SUCCESS )
               {
                    switch( e->param.scalar )
                    {
                    case macSecurityEnabled:
                      MLME_SET_Request
                        (
                         secFrameCounterPerk,		
                         0,
                         0x0001,	
                         &enable_mac_frame_counter_perkey	
                           );
                      break;
                    case secFrameCounterPerk:
                        reset_mac_frame_counter_cmd();
                      break;
                    case macFrameCounter :
                       add_security_key_descriptor_on_MAC();//for with seq without eapol add seq here hardcoded
                       break;
                    case macKeyTable:
/*Need to check this for sec is it correct logic after merging code*/
                      {/*Umesh added for temp*/ //without eapol with security
#if WITH_SECURITY                    
                        add_dev_desc_on_MAC_for_security(&neighbour_addr[0]);             
#else
                      if(fan_nwk_manager_app.node_basic_cfg.fan_device_type == LBR_TYPE)
                      {
                          secrity_status_enab_dis = 0x01;
                          sm_event_t e = { (sm_trigger_t) TRIGGER_CHANGE_JOIN_STATE, { 0 } };
                          s->super.state = (sm_state_t)&node_mac_ready;
                          SM_DISPATCH((sm_t *)s, &e);
                      }
                      else
                      {
                          s->state_ind = NODE_MAC_READY_STATE;  
                        //add_dev_desc_on_MAC_for_security(&router_addr[0]);
                          //secrity_status_enab_dis = 0x01;
                          sm_event_t e = { (sm_trigger_t) TRIGGER_CHANGE_JOIN_STATE, { 0 } };
                          s->super.state = (sm_state_t)&node_mac_ready;
                          SM_DISPATCH((sm_t *)s, &e);
                      }

#endif
                      }
                        break;
                    case macDeviceTable:
                      {
                          /*If the device is started as a PAN Cordinator then
                          it has successfully formed the network now,else if
                          the device was trying to join then it has successfully joined*/

                          set_mac_security_on_LBR(&neighbour_addr[0],8);
                          secrity_status_enab_dis = 0x01;
                          sm_event_t e = { (sm_trigger_t) TRIGGER_FAN_MAC_INIT, { 0 } };
                          s->super.state = (sm_state_t)&node_mac_ready;
                          SM_DISPATCH((sm_t *)s, &e);

                         
                      }
                     break; 
                   break;            
                 default:
                   break;
                    }
               }
      default:
      break;
    }
    return NULL;

}
/*******************************************************************************/
void set_mac_security_on_LBR(uint8_t *rec_buff, uint16_t len)
{
  if(secrity_status_enab_dis == 0x00)
  {
     send_mac_security_set_request(NULL,0); //set secrity key 
//     secrity_status_enab_dis = 0x01;
     memcpy(router_addr,rec_buff,8);
//     fan_nwk_manager_sm_t *app = &fan_nwk_manager_app; 
//     sm_event_t e = { (sm_trigger_t) TRIGGER_ENTRY, { 0 } };
//     app->super.state = (sm_state_t)&node_mac_sec_set;
//     SM_DISPATCH((sm_t *)app, &e);
  }
}

void set_mac_security_on_router_node(uint8_t *rec_buff, uint16_t len)
{
  if(secrity_status_enab_dis != 0x01)
  {
    generate_MAC_Security_Key(key_id_index,0);
    
        if (get_join_state() != 5)
    {
      fan_nwk_manager_sm_t *app = &fan_nwk_manager_app; 
      sm_event_t e = { (sm_trigger_t) TRIGGER_ENTRY, { 0 } };
      app->super.state = (sm_state_t)&node_mac_sec_set;
      SM_DISPATCH((sm_t *)app, &e);
    }
//    if(fan_nwk_manager_app.node_basic_cfg.fan_device_type != LBR_TYPE)
//    {
//      //uint8_t recv_from_mac_addr[8] = {0x00};
//      /* Though EAPOL parent's MAC address comes in rec_buf but 
//      we should fetch parent's MAC address from eapol_parent structure.
//      Because for 2nd rank and 3rd rank devices, parent's address in wpa_supplicant
//      will be LBR's address. But the device will be joined with one of the 1st rank device. */
//      //get_eapol_parent_address(router_addr);
//    }
//    else
//    {
//      memcpy(router_addr,rec_buff,8);
//    }
//    
//    rec_buff += 8;
//    len -= 8;
//    mac_self_fan_info.mac_gtk_hash_ele.gtkl = *rec_buff++;
//    key_id_index = *rec_buff++;
//    
//    set_mac_self_info_from_lbr(rec_buff,64);
//#if !(WITH_SECURITY) //Umesh without EAPOL with Security 
//    generate_MAC_Security_Key(key_id_index,0);
//#endif
    
//    if (get_join_state() != 5)
//    {
//      fan_nwk_manager_sm_t *app = &fan_nwk_manager_app; 
//      sm_event_t e = { (sm_trigger_t) TRIGGER_ENTRY, { 0 } };
//      app->super.state = (sm_state_t)&node_mac_sec_set;
//      SM_DISPATCH((sm_t *)app, &e);
//    }
//    else
//    {
//      update_key_table_entry(key_id_index);
//      update_pan_version_from_eapol_parent ();
//    }
  }        
}
/*----------------------------------------------------------------------------*/
uint8_t get_node_type( void )
{
    return fan_nwk_manager_app.node_basic_cfg.fan_device_type;
}
/*----------------------------------------------------------------------------*/
void  get_self_extended_address (uint8_t *macAddr)
{ 
    memcpy( macAddr, fan_nwk_manager_app.node_basic_cfg.self_ieee_addr,IEEE_ADDRESS_LENGTH );  
}
/*----------------------------------------------------------------------------*/
void  get_self_extended_address_reverse (uint8_t *macAddr)
{ 
    mem_rev_cpy( macAddr, fan_nwk_manager_app.node_basic_cfg.self_ieee_addr,IEEE_ADDRESS_LENGTH );  
}
/*----------------------------------------------------------------------------*/

uint16_t get_current_pan_id( void )
{  
  return fan_nwk_manager_app.node_basic_cfg.selected_pan_id;
}


/*----------------------------------------------------------------------------*/
uint32_t get_sun_page_value(void)
{
     if((fan_nwk_manager_app.node_basic_cfg.us_ie.us_schedule.channel_plan == 0x01 ) ||
       fan_nwk_manager_app.node_basic_cfg.us_ie.us_schedule.channel_plan  == 0x00)  // diff from reg domain
    {
      /*if reg domain and operating class is define check operating class and set that 
      phy mode*/  
      /* as of now we are directly taking phymode not from operating class--TBD*/
     return (opearating_zone_list[fan_nwk_manager_app.node_basic_cfg.operatinng_country].SunPage|(fan_nwk_manager_app.node_basic_cfg.phy_mode)); 
    }
    return NULL;
}  
/*----------------------------------------------------------------------------*/
uint16_t get_sun_channel(void)
{
    if(fan_nwk_manager_app.node_basic_cfg.us_ie.us_schedule.channel_function == 0x00)
      {
        if(fan_nwk_manager_app.node_basic_cfg.us_ie.us_schedule.channel_plan  == 0x00)
        {
            return(fan_nwk_manager_app.node_basic_cfg.us_ie.channel_fixed.fixed_chan);
        }
        else if (fan_nwk_manager_app.node_basic_cfg.us_ie.us_schedule.channel_plan  == 0x01)
        {
           return(fan_nwk_manager_app.node_basic_cfg.us_ie.channel_fixed.fixed_chan);
        }
        else
          return 0;
      } 
      else
      {
        return 0;
      } 
}  
/*----------------------------------------------------------------------------*/

void update_pan_id_from_border_router(uint16_t pan_id)
{
    fan_nwk_manager_app.node_basic_cfg.selected_pan_id = pan_id;
    mac_self_fan_info.pan_metrics.pan_id = pan_id;
    MLME_SET_Request
    (
        macPANId,
        0,
        SHORT_ADDRESS_LENGTH,
        &(fan_nwk_manager_app.node_basic_cfg.selected_pan_id)
    ); 
}
/*----------------------------------------------------------------------------*/
void update_self_global_addr(uint8_t *g_ip_addr)
{
  memcpy(fan_nwk_manager_app.node_basic_cfg.self_global_addr, g_ip_addr, 16);
#if ((EFR32FG13P_LBR == 0x00)  && (APP_NVM_FEATURE_ENABLED == 1))
    nvm_store_node_basic_info(); 
#endif    
  return;
}
/*----------------------------------------------------------------------------*/
uint8_t* get_self_global_addr()
{
  return fan_nwk_manager_app.node_basic_cfg.self_global_addr;
}
/*----------------------------------------------------------------------------*/
/******************************************************************************

              MAC to Application Indications

*******************************************************************************/

/*----------------------------------------------------------------------------*/
void App_MCPS_Purge_Conf_cb
(
    uint8_t msdu_handle,
    uint8_t status
)
{
  return ;
}
/*----------------------------------------------------------------------------*/
void App_MLME_Associate_Conf_cb
(
	uint16_t short_address,
	uint8_t status,
	uint8_t* pLowLatencyNwkInfo,
	uint16 channelOffset,
	uint8_t HoppingSequenceLength,
	uint8_t HoppingSequence,
	security_params_t *sec_params
)
{     
  return ;
}
/*----------------------------------------------------------------------------*/
void App_MLME_Associate_Ind_cb
(
	uint8_t* pChild_64_bit_addr,
	uint8_t CapabilityInformation,
	uint8_t* pLowLatencyNwkInfo,
	uint16 channelOffset,
	uint8_t  HoppingSequenceID,
	security_params_t *sec_params
)
{
  return ;  
}
/*----------------------------------------------------------------------------*/
void  App_MLME_Beacon_Notify_Ind_cb
(
	uint8_t bsn,
	pandesc_t* pPandesc,
	uint8_t PendAddrSpec,
	uint8_t* pPendaddrlist,
	uint16_t sdulen,
	uint8_t* pSdu,
	uint8_t ebsn,
	uint8_t beaconType,
	coex_spec_ie_t* coex_spec
)
{ 
  return; // not used in fan  
}
/*----------------------------------------------------------------------------*/
void  App_MLME_Comm_Status_Ind_cb
(
	mac_address_t* pSrcaddr,
	mac_address_t* pDstaddr,
	uint8_t status,
	security_params_t *sec_param
)
{  
  return ;  
}
/*----------------------------------------------------------------------------*/
void App_MLME_Poll_Conf_cb( uint8_t status )
{   
  return;  // not used in fan;
}
/*----------------------------------------------------------------------------*/
void  App_MLME_Reset_Conf_cb( uint8_t status )
{
    /*This function is used to send confirmation for the Reset request*/
    sm_event_t e = { (sm_trigger_t) TRIGGER_RESET_CONF, { 0 } };
    app_mlme_reset_conf_handler_t app_mlme_reset_conf_handler = NULL;
    
    if( status == MAC_SUCCESS )
    {
      fan_nwk_manager_app.result = MAC_SUCCESS;    
    }
    else
    {
      fan_nwk_manager_app.result = status;         
    }

    SM_DISPATCH((sm_t *)&fan_nwk_manager_app, &e);

    if( app_mlme_reset_conf_handler != NULL )
    {
      app_mlme_reset_conf_handler(status);
    }
    return ; 
}
/*----------------------------------------------------------------------------*/
void App_MLME_Scan_Conf_cb
(
	uint8_t status,
	uint8_t ScanType,
	uint8_t ChannelPage,
	uint8_t* p_unscannedChannels,
	uint8_t ResultListSize,
	void *ResultList
)
{	
  return;  // not used in fan    
}
/*----------------------------------------------------------------------------*/
void App_MLME_Start_Conf_cb
(
	uint8_t status
)
{
	/*This function is used to send confirmation for the start request*/
      sm_event_t e = { (sm_trigger_t) TRIGGER_NWK_START_CONF, { 0 } };
      app_mlme_start_conf_handler_t app_mlme_start_conf_handler = NULL;
      if( status == MAC_SUCCESS )
      {
        SM_DISPATCH((sm_t *)&fan_nwk_manager_app, &e);
      }
      else
      {
        /* go to idle state and start all over again */
        sm_transit((sm_t *)&fan_nwk_manager_app, (sm_state_t)&node_idle );
      }  
      if( app_mlme_start_conf_handler != NULL )
      {
        app_mlme_start_conf_handler(status);
      }
      return ;
}
/*----------------------------------------------------------------------------*/
void App_MLME_SET_Conf_cb
(
 uint8_t status,
 uint8_t PIBAttribute,
 uint8_t PIBAttributeIndex
)
{
    /*This function is used to send confirmation for the set request*/
    sm_event_t e = { (sm_trigger_t) TRIGGER_SET_CONF, { 0 } };

    app_mlme_set_conf_handler_t app_mlme_set_conf_handler = NULL;;

    if( status == MAC_SUCCESS )
    {
      fan_nwk_manager_app.result = MAC_SUCCESS;           
      e.param.scalar = PIBAttribute;
    }
    else
    {
      fan_nwk_manager_app.result = status;
      e.param.scalar = PIBAttribute;
    }

    SM_DISPATCH((sm_t *)&fan_nwk_manager_app, &e);
    if( app_mlme_set_conf_handler != NULL )
    {
      app_mlme_set_conf_handler(status,PIBAttribute,PIBAttributeIndex);
    }
    return ;
}
/*----------------------------------------------------------------------------*/
void App_MLME_GET_Conf_cb
(
    uint8_t status,
    uint8_t PIBAttribute,
    uint8_t PIBAttributeIndex,
    uint16_t PIBAttributeLength,
    void *PIBAttributeValue
)
{
  return ;// not used in fan 
}
/*----------------------------------------------------------------------------*/
void App_MLME_Dissociate_Ind_cb
(
    uint8_t* DeviceAddress,
    uint8_t DisassociateReason,
    security_params_t *sec_params
)
{
  return ;// not used in fan 
}
/*----------------------------------------------------------------------------*/
void App_MLME_Dissociate_Conf_cb
(
    uint8_t status,
    mac_address_t* Deviceaddr // device address
)
{
    return ;// not used in fan 
}
/*----------------------------------------------------------------------------*/
void App_MLME_Orphan_Ind_cb
(
    uint8_t* pOrphan64bitAddress, // 64bit address of orphan device
    security_params_t *sec_params
)
{
    return ;// not used in fan 
}
/*----------------------------------------------------------------------------*/
void App_MLME_Sync_Loss_Ind_cb
(
    uint8_t LossReason,			//loss reason
    uint16_t PANId,					//PANID
    uint8_t LogicalChannel, 		// LogicalChannel
    uint8_t ChannelPage,			// ChannelPage
    security_params_t *sec_params
)
{
  return ;// not used in fan 
}
/*----------------------------------------------------------------------------*/
void App_MLME_Beacon_Request_Ind_cb
(
    uchar bcn_type,
    mac_address_t* src_addr,
    ushort dest_pan_id,
    ushort ie_list_fld_size,
    uchar* p_ie_list
)
{
  return ;  // not used in fan   
}
/*----------------------------------------------------------------------------*/
void App_MLME_Beacon_Conf_cb( uchar status )
{
  return; // not used in fan 
}
/*----------------------------------------------------------------------------*/
void App_MAC_Set_PHY_Mode_Conf_cb
(
    uint8_t status
)
{
    return;  // not used in fan
}
/*----------------------------------------------------------------------------*/
void App_MCPS_Data_Conf_cb
(
    uint8_t msduHandle, 
    uint8_t status, 
    uint8_t NumBackoffs,
    uint32_t Timestamp
)
{
  // this function is not used when fan data send to the application
  return ;
}
/*----------------------------------------------------------------------------*/
void App_MCPS_Data_Ind_cb
(
	mac_address_t*  pSrcaddr,
	mac_address_t*  pDstaddr,
	uint16_t msduLength,
	uint8_t* pMsdu,
	uint8_t mpduLinkQuality,
	uint8_t DSN,
        uint8_t pld_ies_present,
	uint64_t Timestamp,
	security_params_t* pSec 
)
{
    return;
}

/*----------------------------------------------------------------------------*/
void App_FAN_MCPS_Data_Conf_cb
(
	uint8_t msduHandle, 
	uint8_t status, 
	uint8_t NumBackoffs,
	uint32_t Timestamp
)
{
    uint8_t hif_Send_buff [20] = {0};
    uint8_t* buf = &hif_Send_buff [0];
    
    *buf++ = FAN_MAC_MCPS_DATA_CONFIRM;
    *buf++ = msduHandle;
    *buf++ = status;
    *buf++ = NumBackoffs;
    memcpy(buf,(uint8_t*)&Timestamp,3);
    buf+=3;
    
    *buf++ = 0x55;//Dummy Comport 
    //hif_send_msg_up(&hif_Send_buff [0],7,1);
}


/*----------------------------------------------------------------------------*/
void App_FAN_ack_Ind_cb( mac_address_t*  pSrcaddr,
    mac_address_t*  pDstaddr,
    uint8_t DSN,
    uint8_t rsl_value,
    uint8_t security_status
    )
{
  mac_ack_ind_handler_t mac_to_6lp_ackind_handler = NULL;
  mac_to_6lp_ackind_handler = mac_2_6lp_ack_ind_cb;
  if(mac_to_6lp_ackind_handler!= NULL)
  {
    mac_to_6lp_ackind_handler
      (
      pSrcaddr,
      pDstaddr,
      DSN,
      rsl_value,
      security_status
    );
  }
  return ;
}

/******************************************************************************/

void App_FAN_no_ack_Ind_cb (void)
{
  mac_no_ack_ind_handler_t mac_to_6lp_no_ack_ind_handler = NULL;
  mac_to_6lp_no_ack_ind_handler = mac_2_6lp_no_ack_ind_cb;
  if(mac_to_6lp_no_ack_ind_handler!= NULL)
  {
    mac_to_6lp_no_ack_ind_handler ();
  }
  return ;
}

/******************************************************************************/

void App_FAN_MCPS_Data_Ind_cb
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
//    uint8_t joiner_dev_addr[8] = {0x00};
//    uint8_t kmp_id = 0xFF;
    uint8_t tran_cont = *pMsdu++;
    msduLength -= 1;
    uint16_t multiplex_id = 0x00;
    memcpy(&multiplex_id,pMsdu,2);
    pMsdu+=2;
    msduLength-=2;
    /*Need to check this for sec is it correct logic after merging code*/
#if ( FAN_EAPOL_FEATURE_ENABLED == 1)                           //!(WITHOUT_EAPOL)    
    if(multiplex_id == 0x0001)/*Key Management Protocol (as defined in [IEEE802.15.9])*/
    {
      mac_data_eapol_ind_handler_t mac_eapol_data_ind_handler = NULL;
      mac_eapol_data_ind_handler = mac_2_eapol_data_ind_cb;
      mac_data_eapol_ind_handler_t mac_eapol_relay_data_ind_handler = NULL;
      mac_eapol_relay_data_ind_handler = mac2_eapol_relay_ind_cb;
      
      kmp_id = *pMsdu++;
      msduLength-=1;
      if((fan_mac_information_data.fan_node_type == 0x01)
         &&(fan_mac_information_data.state_ind == JOIN_STATE_5)
#if(FAN_EAPOL_FEATURE_ENABLED == 1)
           &&(memcmp(pSrcaddr->address.ieee_address,eapol_parent_child_info.sle_eapol_parent,8))
#endif
             )
      {
        
        if(child_device_index <= APP_CFG_MAX_DEV_SUPPORT)
        {
          uint8_t index = get_relay_device_index(pSrcaddr->address.ieee_address);
#if(FAN_EAPOL_FEATURE_ENABLED == 1)
          if(memcmp(pSrcaddr->address.ieee_address,eapol_parent_child_info.child_info[index].child_addr,8) 
             &&(kmp_id == 0x06)|| (kmp_id == 0x07))
          {
            memcpy(&eapol_parent_child_info.child_info[child_device_index++].child_addr,pSrcaddr->address.ieee_address,8);
            mem_rev_cpy(joiner_dev_addr,pSrcaddr->address.ieee_address,8);
            add_dev_desc_on_MAC_for_security(&joiner_dev_addr[0]);
          }
#endif
        }
        
        
        
        mac_eapol_relay_data_ind_handler
          (
           pSrcaddr,
           pDstaddr,
           tran_cont,
           multiplex_id,
           kmp_id,
           msduLength,
           pMsdu
             );   
      }
      else
      {
        if(relay_reply_flag == 0x01)
        {
          relay_reply_flag = 0xFF;
        }
        mac_eapol_data_ind_handler
          (
           pSrcaddr,
           pDstaddr,
           tran_cont,
           multiplex_id,
           kmp_id,
           msduLength,
           pMsdu
             );
      }
    }
#endif //#if(FAN_EAPOL_FEATURE_ENABLED == 1)
    
    if(multiplex_id == 0xA0ED)/*LoWPAN encapsulation (see [RFC7973]).*/
    {

      mac_data_ind_handler_t mac_data_ind_handler = NULL;
      if(pld_ies_present)
      {
        mac_data_ind_handler = mac_2_6lp_data_ind_cb;
        if( pMsdu!= NULL) /*&p_data_msdu,*/   
        {
          if( mac_data_ind_handler != NULL )
          {
            mac_data_ind_handler
              (
               pSrcaddr,
               pDstaddr,
               msduLength,
               pMsdu,
               mpduLinkQuality,
               DSN,
               pld_ies_present,
               Timestamp,
               pSec 
                 );
          }
        }
      }
    }
    else if(multiplex_id == 0x0002)/*0x0002 Wi-SUN Defined Payload*/
    {
      return ;
    }

  return ;
}


/*----------------------------------------------------------------------------*/

void App_MLME_ws_async_frame_Conf_cb( uint8_t status , uint8_t async_frame_type )
{

}
/*----------------------------------------------------------------------------*/
void App_MLME_WS_ASYNC_FRAME_Ind_cb
(
      uint8_t status,
      uint8_t frame_type,
      mac_address_t *p_Srcaddr//Src address details
)
{
#if (AUTO_CONFIG_ENABLE == 0)
  if (response_laye_ID == APP_DEF_LAYER_ID_TOOL)
  {
    uint8_t hif_Send_buff [13] = {0};
    uint8_t* buf = &hif_Send_buff [0];
    
    *buf++ = ASYNC_FRAME_INDICATION_TO_TOOL;
    *buf++ = status;
    *buf++ = frame_type;
    memcpy( buf,p_Srcaddr->address.ieee_address, 8 );
    buf += 8;
    
    *buf++ = DUMMY_COMPORT;//Dummy Comport 
    hif_send_msg_up(&hif_Send_buff [0],11,response_laye_ID,PROTOCOL_ID_FOR_APP);
  }
#endif
}

/*----------------------------------------------------------------------------*/

void App_MLME_Set_Mac_Channel_Info_Conf_cb
(
	uint8_t status
)
{
    /*This function is used to send confirmation for the start request*/
    sm_event_t e = { (sm_trigger_t) TRIGGER_NWK_START_CONF, { 0 } };
   
    if( status == MAC_SUCCESS )
    {
      SM_DISPATCH((sm_t *)&fan_nwk_manager_app, &e);
    }
    else
    {
      /* go to idle state and start all over again */
      sm_transit((sm_t *)&fan_nwk_manager_app, (sm_state_t)&node_idle );
    }  
}
/*----------------------------------------------------------------------------*/
void App_MLME_Fan_Mac_Set_Conf_cb
(
	uint8_t status,
        uint8_t sub_ie_value
)
{
    /*This function is used to send confirmation for the start request*/
    sm_event_t e = { (sm_trigger_t) TRIGGER_SET_FAN_MAC_IE_CONF, { 0 } };
    
    if( status == MAC_SUCCESS )
    {
      fan_nwk_manager_app.result = MAC_SUCCESS;           
      e.param.scalar = sub_ie_value;
      SM_DISPATCH((sm_t *)&fan_nwk_manager_app, &e);
    }
    else
    {
      /* go to idle state and start all over again */
      sm_transit((sm_t *)&fan_nwk_manager_app, (sm_state_t)&node_idle );
    }     
}

/*****************************************************************************
       MAC to Application Indication Ends 
******************************************************************************/
uint8_t get_current_join_state()
{
  return fan_mac_information_data.state_ind;
}

/******************************************************************************/
