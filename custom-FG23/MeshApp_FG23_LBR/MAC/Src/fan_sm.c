/** \file fan_sm.c
 *******************************************************************************
 ** \brief Provides APIs for the Start State Machine
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

/**
 ** \file
 ** \addtogroup start_sm Start Request State Machine (START-SM)
 ** \ingroup all_sm
 ** @{
 **
 ** \brief Start Request State Machine (START-SM)
 **
 ** The Start Request State Machine (START-SM) is responsible for
 ** the proper initiation of a new PAN or the modification of
 ** the superframe configuration as required by a MLME-START.request
 ** primitive received from the next higher layer.
 **
 ** Upon the reception of a MLME-START.request primitive it controls
 ** the following tasks as required by the current state of the MAC:
 **
 ** - updating the superframe configuration and the next beacon
 ** - updating the PIB
 ** - sending a Coordinator Realignment command frame
 ** - initiating an MLME-START.confirm primitive
 **
 **/

/*******************************************************************************
* File inclusion
*******************************************************************************/
#include "StackMACConf.h"
#include "common.h"
#include "queue_latest.h"
#include "list_latest.h"
#include "sw_timer.h"
#include "timer_service.h"
#include "phy.h"
#include "mac_config.h"
#include "mac.h"
#include "mac_defs_sec.h"
#include "ie_element_info.h"
#include "mac_defs.h"
#include "sm.h"
#include "tri_tmr.h"
#include "fan_sm.h"
#include "fan_mac_interface.h"


/*
** =============================================================================
** Private Macro definitions
** =============================================================================
*/
//#define                 MAX_CHANNEL_PKT_TX_ON_TRICKLE      43             
//#define                 UNICAST_LISTEN_CHANNEL_TIME        200*1000L//2*60*1000*1000L
//
////120*1000L  // 60 msec
//#define                 BROADCAST_LISTEN_CHANNEL_TIME     120*1000L // 200 msec
//#define                 COMPLETE_BROADCAST_TIME_SLOT      400*1000L //400 msec
//#define                 BRAODCAST_UNLISTEN_TIME      (COMPLETE_BROADCAST_TIME_SLOT-BROADCAST_LISTEN_CHANNEL_TIME)


/*
** =============================================================================
** Private Structures, Unions & enums Type Definitions
** =============================================================================
**/

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

#ifdef WISUN_FAN_MAC
extern self_info_fan_mac_t mac_self_fan_info;
fan_sm_t fan_sm ;
valid_channel_list_t usable_channel;

#if(FAN_EDFE_FEATURE_ENABLED == 1)
edfe_info_t edfe_information;  
#endif

#endif


/*
** ============================================================================
** External Variable Declarations
** ============================================================================
*/

int32_t set_timer_value=0;
uint8_t unicast_timer_started =0;
uint32_t channel_index_listen=0;
extern uint16_t total_nos_of_channel;
extern fan_pkt_tmr_t pas_timer;
extern fan_pkt_tmr_t pa_timer;
extern fan_pkt_tmr_t pcs_timer;
extern fan_pkt_tmr_t pc_timer;

#if(FAN_EAPOL_FEATURE_ENABLED == 1)
extern uint32_t eapol_count;
#endif

extern phy_pib_t phy_pib;
uint8_t channel_index=0;
fan_mac_param_t fan_mac_params;
uint8_t max_channel_tx_on_trickle = 0 ;
/*
** ============================================================================
** Private Function Prototypes
** ============================================================================
*/

static sm_result_t fan_startsm_idle(fan_sm_t *s, const sm_event_t *e );
void start_timer_channel_hop_listen(void);
uint8_t check_excluded_channel(uint16_t channel);
uint8_t check_brodacst_channel_excluded(uint16_t channel);

#if(FAN_FRQ_HOPPING_FEATURE_ENABLED == 1)
void  start_broadcast_timer_channel_hop_listen(void);
extern int getUnicastChannelIndex(uint16_t slotNumber, uint8_t* MACAddr, uint16_t nChannels);
extern int getBroadcastChannelIndex(uint16_t slotNumber, uint16_t broadcastSchedID, uint16_t nChannels);
#endif

void  get_self_extended_address_reverse (uint8_t *macAddr);
extern uchar mac_check_cap_message_ucast(void);
extern uchar mac_check_cap_message_bcast();
extern uint8_t send_hop_data_to_hif(uint8_t *buf, uint16_t length);
extern phy_status_t phy_prepare_channel_list( uint16_t channel );
static uint8_t make_valid_channel_list(uint16 tot_length);

#if(FAN_FRQ_HOPPING_FEATURE_ENABLED == 1)
extern void fan_freq_hop_start_hopping (void *data);
#endif

extern void * app_bm_alloc( uint16_t length);
/*
** ============================================================================
** Public Function Definitions
** ============================================================================
*/

#ifdef WISUN_FAN_MAC
uint8_t get_node_type( void );
void fan_startsm_initialise(void)
{
  uint16_t length = 0;
  uint32_t current_max_channels=0x00;
  if((mac_self_fan_info.unicast_listening_sched.us_schedule.channel_plan == 0x00)
     ||(mac_self_fan_info.bcast_sched.bs_schedule.channel_plan == 0x00))
  {
    PLME_get_request( phyMaxSUNChannelSupported, &length, &current_max_channels );
  }
  else
  {
    current_max_channels = mac_self_fan_info.unicast_listening_sched.un_channel_plan.ch_explicit.num_chans;
  }
  
#if(FAN_FRQ_HOPPING_FEATURE_ENABLED == 1)  
  if((mac_self_fan_info.bcast_sched.bs_schedule.channel_function == CF_DH1) || 
     (mac_self_fan_info.bcast_sched.bs_schedule.channel_function == CF_TR51)
       || (mac_self_fan_info.unicast_listening_sched.us_schedule.channel_function == CF_DH1)
         ||((mac_self_fan_info.unicast_listening_sched.us_schedule.channel_function == CF_TR51)))
  {
#if 0  
    phy_prepare_channel_list( 0 );
#endif 
    max_channel_tx_on_trickle = make_valid_channel_list((uint16_t)current_max_channels);
  }
#endif
  
  
#if(FAN_FRQ_HOPPING_FEATURE_ENABLED == 1)
//  /* Debdeep :: Start scheduling fro freq hopping only for router from here */
//  if (get_node_type () == 0x01)   //no eapol supoorted 
  fan_freq_hop_start_hopping(NULL);
#endif
  
  fan_sm_t *app = &fan_sm; 
  sm_event_t e;
  e.trigger = (sm_trigger_t) FAN_SM_TRIGGER_ENTRY;
  e.param.scalar = 0;  
  app->super.state = (sm_state_t)&fan_startsm_idle;
  SM_DISPATCH((sm_t *)app, &e);
}

/******************************************************************************/
/*
** ============================================================================
** Private Function Definitions
** ============================================================================
*/
/******************************************************************************/

static sm_result_t fan_startsm_idle( fan_sm_t *s, const sm_event_t *e )
{
  uint32_t Start_Channel = 0x00;
  //uint8_t status = 0xFF;
    switch( (fan_sm_trigger_t) e->trigger )
    {//switch start
      case FAN_SM_TRIGGER_ENTRY:
          fan_sm.fan_param.pas_channel_index = 0;
          fan_sm.fan_param.pa_channel_index = 0;
          s->state_ind = FAN_SM_STATE_IDLE;
      break;

      case FAN_SM_TRIGGER_PAS_PKT:
      {
          if(mac_self_fan_info.unicast_listening_sched.us_schedule.channel_function == 0x00)
          {
            Start_Channel = mac_self_fan_info.unicast_listening_sched.channel_fixed.fixed_chan; 
          }
          else
          {
           //Start_Channel = pas_timer.channel_list[fan_sm.fan_param.pas_channel_index];
           Start_Channel = usable_channel.unicast_usable_channel_list[fan_sm.fan_param.pas_channel_index];
          } 
          
          Create_MLME_WS_ASYNC_FRAME_Request(  Start_Channel, 
                                               PAN_ADVERT_SOLICIT, 
                                               UTT_IE_MASK,//PAN_ADVERT_SOLICIT_HEDR_IE_LIST, 
                                               US_IE_MASK | NETNAME_IE_MASK);//PAN_ADVERT_SOLICIT_PAYLOAD_IE_LIST);
          break;
      }      
      case FAN_SM_TRIGGER_PCS_PKT:
      {
          if(mac_self_fan_info.unicast_listening_sched.us_schedule.channel_function == 0x00)
          {
            Start_Channel = mac_self_fan_info.unicast_listening_sched.channel_fixed.fixed_chan;
          }
          else
          {
  
           //Start_Channel = pcs_timer.channel_list[fan_sm.fan_param.pcs_channel_index];
           Start_Channel = usable_channel.unicast_usable_channel_list[fan_sm.fan_param.pcs_channel_index];
          } 
          
          
           Create_MLME_WS_ASYNC_FRAME_Request(  Start_Channel, 
                                                           PAN_CONFIG_SOLICIT, 
                                                           UTT_IE_MASK,//PAN_CONFIG_SOLICIT_HEDR_IE_LIST, 
                                                           US_IE_MASK | NETNAME_IE_MASK);//PAN_CONFIG_SOLICIT_PAYLOAD_IE_LIST);
          break;
      }

      case FAN_SM_TRIGGER_PA_PKT:
      {  
          if(mac_self_fan_info.unicast_listening_sched.us_schedule.channel_function == 0x00)
          {

            Start_Channel = mac_self_fan_info.unicast_listening_sched.channel_fixed.fixed_chan;
           
          }
          else
          {
           Start_Channel = usable_channel.unicast_usable_channel_list[fan_sm.fan_param.pa_channel_index];
          } 
         
           Create_MLME_WS_ASYNC_FRAME_Request(  Start_Channel, 
                                                           PAN_ADVERT_FRAME, 
                                                           UTT_IE_MASK,//PAN_ADVERT_HEDR_IE_LIST, 
                                                           US_IE_MASK | PAN_IE_MASK | NETNAME_IE_MASK);//PAN_ADVERT_PAYLOAD_IE_LIST);
          break;
      } 
      
      case FAN_SM_TRIGGER_PC_PKT:
      {
          if(mac_self_fan_info.unicast_listening_sched.us_schedule.channel_function == 0x00)
          {
            Start_Channel = mac_self_fan_info.unicast_listening_sched.channel_fixed.fixed_chan;
          }
          else
          {
            Start_Channel = usable_channel.unicast_usable_channel_list[fan_sm.fan_param.pc_channel_index];  
          } 
         
           Create_MLME_WS_ASYNC_FRAME_Request(  Start_Channel, 
                                                           PAN_CONFIG, 
                                                           UTT_IE_MASK | BT_IE_MASK,//PAN_CONFIG_HEDR_IE_LIST, 
                                                           US_IE_MASK | BS_IE_MASK | PAN_VER_IE_MASK | GTK_HASH_IE_MASK);//PAN_CONFIG_PAYLOAD_IE_LIST);
          break;
      }

      default:
          break;
    }//switch end
    return NULL_POINTER;
}

/******************************************************************************/
//MLME_WS_ASYNC_FRAME_Request_on_trickel

/* Debdeep restructured this function :: Channel index is handeled properly now*/
void ws_send_pkt(uint8_t pkt_type, uint8_t* channal_list,uint8_t length_channel_list)
{
  //suneet :: if edfe mode is enable stop currunt async pkt
#if(FAN_EDFE_FEATURE_ENABLED == 1)
  if(edfe_information.edfe_frame_enabled == 0x01)
  {
    return;
  }
#endif
  
  switch(pkt_type)
  {
  case PAN_ADVERT_SOL: 
    {
      fan_sm.fan_param.length_channel_pas = length_channel_list;
      
      sm_event_t e2 = { (sm_trigger_t) FAN_SM_TRIGGER_PAS_PKT, { 0 } };   
      fan_sm.fan_param.pas_channel_list = channal_list;      
      SM_DISPATCH((sm_t *)&fan_sm, &e2);
      break;
    }
      
  case PAN_ADVERT: 
    {
      fan_sm.fan_param.length_channel_pa = length_channel_list;
      
      sm_event_t e2 = { (sm_trigger_t) FAN_SM_TRIGGER_PA_PKT, { 0 } };   
      fan_sm.fan_param.pa_channel_list = channal_list;  
      SM_DISPATCH((sm_t *)&fan_sm, &e2);
      break;
    }
    
  case PAN_CONFIG_SOL:
    {
      fan_sm.fan_param.length_channel_pcs = length_channel_list;

      sm_event_t e2 = { (sm_trigger_t) FAN_SM_TRIGGER_PCS_PKT, { 0 } };   
      fan_sm.fan_param.pcs_channel_list = channal_list;  
      SM_DISPATCH((sm_t *)&fan_sm, &e2);        
      break;
    }
      
  case PAN_CONF:
    {
      fan_sm.fan_param.length_channel_pc = length_channel_list;

      sm_event_t e2 = { (sm_trigger_t) FAN_SM_TRIGGER_PC_PKT, { 0 } };   
      fan_sm.fan_param.pc_channel_list = channal_list;       
      SM_DISPATCH((sm_t *)&fan_sm, &e2);        
      break;
    }
  }//end switch 
}

/******************************************************************************/
/* Debdeep restructured this function :: Channel index is handeled properly now*/
void send_another_pkt_on_channel_hop(uint8_t pkt_type)   // this function send same packet to next channel
{
  //suneet :: if edfe mode is enable stop currunt async pkt
#if(FAN_EDFE_FEATURE_ENABLED == 1)
  if(edfe_information.edfe_frame_enabled == 0x01)
  {
    return;
  }
#endif
  switch (pkt_type)
  {
  case PAN_ADVERT_SOLICIT: 
      if (mac_self_fan_info.unicast_listening_sched.us_schedule.channel_function != CF_FIXED_CHANNEL)
      {
        fan_sm.fan_param.pas_channel_index++;
        if (fan_sm.fan_param.pas_channel_index < fan_sm.fan_param.length_channel_pas)
        {
          sm_event_t e2 = { (sm_trigger_t) FAN_SM_TRIGGER_PAS_PKT, { 0 } };    
          SM_DISPATCH((sm_t *)&fan_sm, &e2);
        } 
        else
          fan_sm.fan_param.pas_channel_index = 0;
      }
    break;
  case PAN_ADVERT_FRAME: 
      if (mac_self_fan_info.unicast_listening_sched.us_schedule.channel_function != CF_FIXED_CHANNEL)
      {
        fan_sm.fan_param.pa_channel_index++;
        if (fan_sm.fan_param.pa_channel_index < fan_sm.fan_param.length_channel_pa)
        {
          sm_event_t e2 = { (sm_trigger_t) FAN_SM_TRIGGER_PA_PKT, { 0 } };    
          SM_DISPATCH((sm_t *)&fan_sm, &e2);
        } 
        else
          fan_sm.fan_param.pa_channel_index = 0;
      }
    break;
  case PAN_CONFIG:
      if (mac_self_fan_info.unicast_listening_sched.us_schedule.channel_function != CF_FIXED_CHANNEL)
      {
        fan_sm.fan_param.pc_channel_index++;
        if (fan_sm.fan_param.pc_channel_index < fan_sm.fan_param.length_channel_pc)
        {
          sm_event_t e2 = { (sm_trigger_t) FAN_SM_TRIGGER_PC_PKT, { 0 } };    
          SM_DISPATCH((sm_t *)&fan_sm, &e2);
        }
        else
          fan_sm.fan_param.pc_channel_index = 0;
      }
    break;
  case PAN_CONFIG_SOLICIT:
      if (mac_self_fan_info.unicast_listening_sched.us_schedule.channel_function != CF_FIXED_CHANNEL)
      {
        fan_sm.fan_param.pcs_channel_index++;
        if (fan_sm.fan_param.pcs_channel_index < fan_sm.fan_param.length_channel_pcs)
        {
          sm_event_t e2 = { (sm_trigger_t) FAN_SM_TRIGGER_PCS_PKT, { 0 } };    
          SM_DISPATCH((sm_t *)&fan_sm, &e2);
        }
        else
          fan_sm.fan_param.pcs_channel_index = 0;
      }
    break;  
  default:
    break;
  }
}

uint8_t get_async_sent_count (uint8_t sub_type)
{
  uint8_t count = 0;
  switch (sub_type)
  {
  case PAN_ADVERT_SOLICIT:
    count = fan_sm.fan_param.pas_channel_index;
    break;
  case PAN_ADVERT_FRAME:
    count = fan_sm.fan_param.pa_channel_index;
    break;
  case PAN_CONFIG:
    count = fan_sm.fan_param.pc_channel_index;
    break;
  case PAN_CONFIG_SOLICIT:
    count = fan_sm.fan_param.pcs_channel_index;
    break;
  }
  return count;
}

/******************************************************************************/
uint8_t make_valid_channel_list(uint16 tot_length)
{
  uint8_t buf_unicast[129] = {0};
  uint8_t buf_broadcast[129] = {0};
  if( mac_self_fan_info.unicast_listening_sched.us_schedule.excludded_channel_control == EXCLUDED_CHANNEL_PRESENT)
  {
    uint8_t i=0,j=0,k=0,l=0;
    uint8_t chack_length = tot_length; 
    for(i=0;i<chack_length;i++)
    {
      if(/*send_chan_list[j]*/i == mac_self_fan_info.unicast_listening_sched.us_schedule.excluded_channels.excluded_channel_ranges.ex_ch_range[k].start_ch)
      {
        while(mac_self_fan_info.unicast_listening_sched.us_schedule.excluded_channels.excluded_channel_ranges.ex_ch_range[k].end_ch!= i)
        {
          j++;
          i++;
        }
        k++;
        j++;
      }
      else
      {
        buf_unicast[l] = /*send_chan_list[j]*/i;
        l++;
        j++;
      }
    }
    usable_channel.total_usable_ch_unicast = l;
  }
  if(mac_self_fan_info.bcast_sched.bs_schedule.excludded_channel_control == EXCLUDED_CHANNEL_PRESENT)
  {
    uint8_t i=0,j=0,k=0,l=0;
    uint8_t chack_length = tot_length; 
    for(i=0;i<chack_length;i++)
    {
      if(/*send_chan_list[j]*/i == mac_self_fan_info.bcast_sched.bs_schedule.excluded_channels.excluded_channel_ranges.ex_ch_range[k].start_ch)
      {
        while(mac_self_fan_info.bcast_sched.bs_schedule.excluded_channels.excluded_channel_ranges.ex_ch_range[k].end_ch!= i)
        {
          j++;
          i++;
        }
        k++;
        j++;
      }
      else
      {
        buf_broadcast[l] = /*send_chan_list[j]*/i;
        l++;
        j++;
      }
    }
    usable_channel.total_usable_ch_broadcast = l;
  }
  if(mac_self_fan_info.unicast_listening_sched.us_schedule.excludded_channel_control == 0x00)
  {
    uint8_t l=0;
    for(l=0; l<tot_length ; l++)
    {
      buf_unicast[l] = l;/*send_chan_list[j++]*/;
    }
    usable_channel.total_usable_ch_unicast = l;
  }
  if(mac_self_fan_info.bcast_sched.bs_schedule.excludded_channel_control == 0x00)
  {
    uint8_t l=0;
    for(l=0; l<tot_length ; l++)
    {
      buf_broadcast[l] = l;
    }
    usable_channel.total_usable_ch_broadcast = l;
  }
  if(mac_self_fan_info.bcast_sched.bs_schedule.excludded_channel_control == EXCLUDED_CHANNEL_MASK_PRESENT)
  {
    uint8_t loop_count = tot_length/8;
    uint8_t m=0,i=0,j=0,l=0;
    for(j=0;j<loop_count;j++)
    {
      for(i=0;i<8;i++)
      {
        if(mac_self_fan_info.bcast_sched.bs_schedule.excluded_channels.excluded_channel_mask[j]& (0x01<<i))
        {
          m++;
        }
        else
        {
          buf_broadcast[l++] = m;/*send_chan_list[m]*/;
          m++;
        }  
        
      }
    }
    usable_channel.total_usable_ch_broadcast = l;
  }
  if(mac_self_fan_info.unicast_listening_sched.us_schedule.excludded_channel_control == EXCLUDED_CHANNEL_MASK_PRESENT)
  {
    uint8_t loop_count = tot_length/8;
    uint8_t m=0,i=0,j=0,l=0;;
    for(j=0;j<loop_count;j++)
    {
      for(i=0;i<8;i++)
      {
        if(mac_self_fan_info.unicast_listening_sched.us_schedule.excluded_channels.excluded_channel_mask[j]& (0x01<<i))
        {
          m++;
        }
        else
        {
          buf_unicast[l++] = m;/*send_chan_list[m]*/;
          m++;
        }  
        
      }
    }
    usable_channel.total_usable_ch_unicast = l;
  }
  
  usable_channel.unicast_usable_channel_list = (uint8_t *)app_bm_alloc( usable_channel.total_usable_ch_unicast);
  if(usable_channel.unicast_usable_channel_list!=NULL)
  memcpy(usable_channel.unicast_usable_channel_list,buf_unicast,usable_channel.total_usable_ch_unicast);
  usable_channel.broad_usable_channel_list = (uint8_t *)app_bm_alloc( usable_channel.total_usable_ch_broadcast);
  if(usable_channel.broad_usable_channel_list != NULL)
  memcpy(usable_channel.broad_usable_channel_list,buf_broadcast,usable_channel.total_usable_ch_broadcast);
  return usable_channel.total_usable_ch_unicast;
}
#endif
/******************************************************************************/
