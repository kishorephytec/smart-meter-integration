/*****************************************************************************
* fan_mac_freq_hopping.c
*****************************************************************************/

/** \file fan_mac_freq_hopping.c
*******************************************************************************
** \brief This application provides embedded demo for RIT data transmission
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
*****************************************************************************
* @ingroup sysdoc
*
* @{
*****************************************************************************/

/*
********************************************************************************
* File inclusion
********************************************************************************
*/

#include "StackMACConf.h"
#include "common.h"

#if(FAN_FRQ_HOPPING_FEATURE_ENABLED == 1)
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
#include "mac.h"
#include "mac_pib.h"
#include "mac_defs_sec.h"
#include "ie_element_info.h"
#include "mac_defs.h"
#include "sm.h"
#include "tri_tmr.h"
#include "ie_manager.h"
#include "fan_sm.h"
#include "mac_frame_build.h"
#include "ccasm.h"
#include "trxsm.h"
#include "mac_config.h"
#include "ie_manager.h"
#include "mac_interface_layer.h"
#include "sm.h"
#include "event_manager.h"
#include "ie_element_info.h"
#include "fan_mac_freq_hopping.h"
#include "l3_configuration.h"
#include "l3_process_interface.h"
#include "fan_mac_interface.h"






#ifdef WISUN_FAN_MAC

/*
** =============================================================================
** Private Macro definitions
** =============================================================================
*/
/*Broadcast error offset time: Time difference between two node to enter same 
  broadcast slot is around 12 milliseconds*/
#define BROADCAST_ERROR_OFFSET 20
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

static uint8_t unicast_timer_started =0;
uint8_t set_channel_in_unicast_sech = 0x01;
//static uint8_t set_channel_in_broadcast_dwell_interval = 0;
/*
** =============================================================================
** Private Function Prototypes
** =============================================================================
*/


static uint8_t freq_hopping_process_event_handler(l3_process_event_t ev, l3_process_data_t data);
L3_PROCESS(freq_hopping_process, "FAN Frequency Hopping Process");

/*
** =============================================================================
** Private Function Definitions
** =============================================================================
*/

static void start_timer_channel_hop_listen(void);
void start_broadcast_timer_channel_hop_listen(void);
void unicast_channel_change();
void start_unicast_dwell_timer(uint64_t unicast_dwell_interval);
void start_broadcast_dwellinterval_timer(uint64_t broadcast_dwell_interval);
void start_brocast_shedule_interval_timer(uint64_t broadcast_interval);
void start_timer_to_start_broadcast_schdeule(uint64_t expire_int);
void stop_broadcast_dwellinterval_timer (void);
void procubed_broadcast_error_offset_timer_start (uint64_t time_val);

/*
** =============================================================================
** External Variable Declarations
** =============================================================================
*/
#ifdef WISUN_FAN_MAC
extern self_info_fan_mac_t mac_self_fan_info;
extern trxsm_t *trxsm_p;
extern uint16_t total_nos_of_channel;
//extern int32_t HopSequenceTable[150];
extern phy_pib_t phy_pib;
//extern uint8_t create_channel_list_for_tr51();
extern int getUnicastChannelIndex(uint16_t slotNumber, uint8_t* MACAddr, uint16_t nChannels);
extern int getBroadcastChannelIndex(uint16_t slotNumber, uint16_t broadcastSchedID, uint16_t nChannels);
//extern ch_change_set ch_set_val [128];
//extern uint8_t TRX_Set_Channel_form_hopFrequency_list( uint8_t intc ,uint32_t frac_val, uint8_t set_ch_num );
extern phy_status_t backup_trx_state( void );
extern uint8_t is_hif_receive_in_process (void);
uint64_t get_time_now_64 (void);
extern valid_channel_list_t usable_channel;
void trigger_explicit_sicslowpan_packet (uint8_t schedule_type);
void int_l3_queue (void);
mac_nbr_descriptor_t* get_nbr_desc_from_addr (uint8_t *addr);

#if(FAN_EAPOL_FEATURE_ENABLED == 1)
extern void get_eapol_parent_address(uint8_t *eapol_parent);
#if APP_LBR_ROUTER
extern parent_child_info_tag eapol_parent_child_info;
#endif
#endif  // #if(FAN_EAPOL_FEATURE_ENABLED == 1)

uint8_t dont_change_ubcast_channel = 0xFF;

#if(FAN_EDFE_FEATURE_ENABLED == 1)
extern edfe_info_t edfe_information;  
#endif

#endif
/*
** =============================================================================
** External Function Prototypes
** =============================================================================
*/




/*
** =============================================================================
** Public Variables Definitions
** =============================================================================
**/

uint16_t unicast_slot_no=0;
uint16_t broadcast_slot_nuumber=0;

/*
** =============================================================================
** Public Function Prototypes
** =============================================================================
*/
void unicast_channel_change_timer(void *a);
void broadcast_channel_change_timer(void *a);
void broadcast_channel_change();
/*
** =============================================================================
** Public Function Definitions
** =============================================================================
*/

L3_PROCESS_THREAD(freq_hopping_process, ev, data)
{
  L3_PROCESS_BEGIN();
  
  while(1) 
  {
    L3_PROCESS_YIELD();
    freq_hopping_process_event_handler(ev, data);
  }
  L3_PROCESS_END();
}
/*----------------------------------------------------------------------------*/
static uint8_t freq_hopping_process_event_handler(l3_process_event_t ev, l3_process_data_t data)
{
  if (ev == FAN_FREQ_HOPPING_START )
  {
    if((mac_self_fan_info.pan_metrics.parent_bs_ie_use == 0x00) || (fan_mac_information_data.fan_node_type == 0x00)) //0x00 for LBR
    {
      start_broadcast_timer_channel_hop_listen();//start broadcast timer 
    }
//    if(mac_self_fan_info.unicast_listening_sched.us_schedule.channel_function == CF_TR51)
//    {
//      create_channel_list_for_tr51();
//      start_timer_channel_hop_listen();
//    }
    if(mac_self_fan_info.unicast_listening_sched.us_schedule.channel_function == CF_DH1)
    {
      start_timer_channel_hop_listen();
    }
  }
  else if (ev == FAN_CHANGE_UNICAST_SLOT )
  {
    unicast_channel_change();
    /* TO-DO */      
  }
  else if (ev == FAN_CHANGE_BROADCAST_SLOT )
  {
    broadcast_channel_change();
    /* TO-DO */  
  }
  else if (ev == TRIGGER_EXPLICIT_UNICAST_PACKET)
  {
#if APP_LBR_ROUTER     
    trigger_explicit_sicslowpan_packet (1);
#endif    
  }
  else if (ev == TRIGGER_EXPLICIT_BROADCAST_PACKET)
  {
#if APP_LBR_ROUTER     
    trigger_explicit_sicslowpan_packet (0);
#endif    
  }
  return 0;
}
/*----------------------------------------------------------------------------*/
static void fan_freq_hop_post_event( l3_process_event_t ev, uint8_t* data)
{
  l3_process_post(&freq_hopping_process, ev, data);
}

void fan_freq_hop_start_hopping (void *data)
{
  if(mac_self_fan_info.bcast_sched.is_broadcast_sch_active == 1)
  {
    mac_self_fan_info.bcast_sched.is_broadcast_sch_active = 0;
  }
  fan_freq_hop_post_event (FAN_FREQ_HOPPING_START, (uint8_t *)data);
  int_l3_queue ();
} 

void fan_freq_hop_update_unicast_slot (void *data)
{
  fan_freq_hop_post_event (FAN_CHANGE_UNICAST_SLOT, (uint8_t *)data);    
} 

void fan_freq_hop_update_broadcast_slot (void *data)
{
  fan_freq_hop_post_event (FAN_CHANGE_BROADCAST_SLOT, (uint8_t *)data);    
} 

void trigger_explicit_unicast_packet (void *data)
{
  fan_freq_hop_post_event (TRIGGER_EXPLICIT_UNICAST_PACKET, (uint8_t *)data);
}

void trigger_explicit_broadcast_packet (void *data)
{
  fan_freq_hop_post_event (TRIGGER_EXPLICIT_BROADCAST_PACKET, (uint8_t *)data);
}
/******************************************************************************/
static void start_timer_channel_hop_listen(void)
{
  uint8_t mac_addr[8] = {0};
  get_self_extended_address_reverse(mac_addr);
  if(!unicast_timer_started)
  {
    uint32_t unicast_hop_channel=0;
    int unicast_channel_index = 0;
    if(mac_self_fan_info.unicast_listening_sched.us_schedule.channel_function == CF_DH1)
    {
      
      trxsm_p->uc_chan_hop_seq_start_time = get_time_now_64();
      
      unicast_channel_index = getUnicastChannelIndex(unicast_slot_no,mac_addr,usable_channel.total_usable_ch_unicast);
      unicast_hop_channel = usable_channel.unicast_usable_channel_list[unicast_channel_index];
    }
    else if(mac_self_fan_info.unicast_listening_sched.us_schedule.channel_function == CF_TR51)
    {
//      if(unicast_slot_no==total_nos_of_channel)
//      {
//        unicast_slot_no =0;
//      }  
//      trxsm_p->uc_chan_hop_seq_start_time = get_time_now_64();
//      unicast_hop_channel = HopSequenceTable[unicast_slot_no];
    }
    if(phy_pib.TRXState==PHY_RX_ON)
    {  
      if(set_channel_in_unicast_sech == 0x01)    // if in braodcast hop channel is already set
      {
        if((dont_change_ubcast_channel != 0x88) && (is_hif_receive_in_process() != 1))
        {
          PLME_set_request(phyCurrentChannel,2,&unicast_hop_channel);
        }
      }
    }    
#if APP_LBR_ROUTER       
    start_unicast_dwell_timer(mac_self_fan_info.unicast_listening_sched.us_schedule.dwell_interval);
#endif    
    unicast_timer_started = 1;
  }
} 

void start_broadcast_timer_channel_hop_listen(void)
{
  uint16_t max = 0;
  uint32_t broadcast_hop_channel=0;
  int brodcast_channel_index = 0;
  if((mac_self_fan_info.pan_metrics.parent_bs_ie_use == 0x00) || (fan_mac_information_data.fan_node_type == 0x01))
  {
    broadcast_slot_nuumber++;
    mac_self_fan_info.bcast_sched.rcvd_broadcast_slot_nuumber = broadcast_slot_nuumber;
  }
  
  if(mac_self_fan_info.bcast_sched.is_broadcast_sch_active == 0)
  {
    max = (mac_self_fan_info.unicast_listening_sched.us_schedule.dwell_interval > mac_self_fan_info.bcast_sched.bs_schedule.dwell_interval) ? mac_self_fan_info.unicast_listening_sched.us_schedule.dwell_interval : mac_self_fan_info.bcast_sched.bs_schedule.dwell_interval;
    mac_self_fan_info.bcast_sched.bcast_interval = (max*4);
    if (fan_mac_information_data.fan_node_type == 0x01)
      trxsm_p->bc_chan_hop_seq_start_time = mac_self_fan_info.bcast_sched.rcvd_t1_2 * 1000;
    else
      trxsm_p->bc_chan_hop_seq_start_time = get_time_now_64();  /*Usec*/
    trxsm_p->bc_chan_hop_seq_current_slot_time = trxsm_p->bc_chan_hop_seq_start_time;
#if APP_LBR_ROUTER    
    start_brocast_shedule_interval_timer(mac_self_fan_info.bcast_sched.bcast_interval);
    start_broadcast_dwellinterval_timer(mac_self_fan_info.bcast_sched.bs_schedule.dwell_interval);
#endif    
    
    if((mac_self_fan_info.bcast_sched.bs_schedule.channel_function == CF_DH1) ||
       (mac_self_fan_info.bcast_sched.bs_schedule.channel_function == CF_FIXED_CHANNEL))
    {
      if(fan_mac_information_data.fan_node_type == 0x01)
      {
        mac_nbr_descriptor_t *p_nbr_desc = NULL;
        #if APP_LBR_ROUTER
        p_nbr_desc = get_nbr_desc_from_addr (eapol_parent_child_info.sle_eapol_parent);
        brodcast_channel_index = getBroadcastChannelIndex(broadcast_slot_nuumber,mac_self_fan_info.bcast_sched.bcast_sched_id,p_nbr_desc->nbrchannel_usable_list.total_usable_ch_broadcast);
        broadcast_hop_channel =  p_nbr_desc->nbrchannel_usable_list.broad_usable_channel_list[brodcast_channel_index];
        #endif
      }
      else
      {
        brodcast_channel_index = getBroadcastChannelIndex(broadcast_slot_nuumber,mac_self_fan_info.bcast_sched.bcast_sched_id,usable_channel.total_usable_ch_broadcast);
        broadcast_hop_channel = usable_channel.broad_usable_channel_list[brodcast_channel_index];
      }
    }
    else if(mac_self_fan_info.bcast_sched.bs_schedule.channel_function == CF_TR51)
    {
//      if(broadcast_slot_nuumber==total_nos_of_channel)
//        broadcast_slot_nuumber =0;
//
//      broadcast_hop_channel =HopSequenceTable[broadcast_slot_nuumber];
    }
    
    if(mac_self_fan_info.bcast_sched.bs_schedule.channel_function != 0x00)
    {  
      if ((dont_change_ubcast_channel != 0x88) && (is_hif_receive_in_process() != 1))
      {
//        if(( phy_pib.TRXState != PHY_BUSY_RX ) && ( phy_pib.TRXState != PHY_BUSY_TX  ))
//        {
//          TRX_Set_Channel_form_hopFrequency_list( ch_set_val[ broadcast_hop_channel].intC_ch_set ,ch_set_val[ broadcast_hop_channel].frac_val,broadcast_hop_channel);
//        }
        PLME_set_request(phyCurrentChannel,2,&broadcast_hop_channel);
      }
    }
    
    mac_self_fan_info.bcast_sched.is_broadcast_sch_active = 1;
  }
}


void unicast_channel_change()
{
  uint8_t mac_addr[8] = {0};
  get_self_extended_address_reverse(mac_addr);
//  uint32_t current_max_channels=0x00;
  uint32_t unicast_hop_channel=0;
  int unicast_channel_index = 0;
  uint64_t time_now = get_time_now_64();
  if(mac_self_fan_info.unicast_listening_sched.us_schedule.channel_function == CF_DH1)
  {
    unicast_channel_index = getUnicastChannelIndex(unicast_slot_no,mac_addr,usable_channel.total_usable_ch_unicast);
    unicast_hop_channel = usable_channel.unicast_usable_channel_list[unicast_channel_index];
  }
  else if(mac_self_fan_info.unicast_listening_sched.us_schedule.channel_function == CF_TR51)
  {
//    if(unicast_slot_no == total_nos_of_channel)
//    {
//      unicast_slot_no =0;
//      trxsm_p->uc_chan_hop_seq_start_time = get_time_now_64();
//    }  
//    
//    unicast_hop_channel =HopSequenceTable[unicast_slot_no];
  }
  if(phy_pib.TRXState==PHY_RX_ON)
  {  
    if(set_channel_in_unicast_sech == 0x01)    // if in braodcast hop channel is already set
    {
      if((dont_change_ubcast_channel != 0x88) && (is_hif_receive_in_process() != 1))
      {
        PLME_set_request(phyCurrentChannel,2,&unicast_hop_channel);
      }
    }
  }
}
/******************************************************************************/
//static uint64_t mod_time_diff (int64_t time_diff)
//{
//  if (time_diff >= (int64_t)0)
//    return (uint64_t)time_diff;
//  else
//    return (uint64_t)(time_diff * (int64_t)-1);
//}

void unicast_channel_change_timer(void *a)
{ 
  int64_t time_offset = 0;
  uint64_t twos_complement_offset = 0;
  uint16_t temp_ucast_slot_number = 0;
  uint64_t time_now = 0;
  uint64_t next_udi = 0;
  
  if((mac_self_fan_info.unicast_listening_sched.us_schedule.channel_function == CF_DH1)
     || (mac_self_fan_info.unicast_listening_sched.us_schedule.channel_function == CF_TR51))
  {
calculate_again:
    time_now = get_time_now_64();
    temp_ucast_slot_number = (time_now - trxsm_p->uc_chan_hop_seq_start_time)
      / (uint64_t)((uint64_t)mac_self_fan_info.unicast_listening_sched.us_schedule.dwell_interval * (uint64_t)1000);
    
    time_offset = (int64_t)(time_now - trxsm_p->uc_chan_hop_seq_start_time) - 
      (int64_t)((int64_t)temp_ucast_slot_number * 
                (int64_t)mac_self_fan_info.unicast_listening_sched.us_schedule.dwell_interval * (int64_t)1000);
    time_offset /= 1000;
    twos_complement_offset = ~time_offset;
    twos_complement_offset += 1;
    next_udi = (uint64_t)mac_self_fan_info.unicast_listening_sched.us_schedule.dwell_interval 
                              + twos_complement_offset;
    if (next_udi < (uint64_t)2)
      goto calculate_again;
    
    if (next_udi > (uint64_t)mac_self_fan_info.unicast_listening_sched.us_schedule.dwell_interval)
      next_udi = (uint64_t)mac_self_fan_info.unicast_listening_sched.us_schedule.dwell_interval;
    
    unicast_slot_no = temp_ucast_slot_number;
    if(unicast_slot_no== MAXIMUM_SLOT_NUMBER)
    {
      unicast_slot_no = 0;
      trxsm_p->uc_chan_hop_seq_start_time = time_now;
    }
#if APP_LBR_ROUTER    
    start_unicast_dwell_timer (next_udi);
#endif    
    fan_freq_hop_update_unicast_slot(NULL);
  }
}

/******************************************************************************/
void broadcast_idle_time(void *a)
{
//  set_channel_in_broadcast_dwell_interval = 0;
  set_channel_in_unicast_sech = 0x01;
  trigger_explicit_unicast_packet(NULL);
}  
/******************************************************************************/
void broadcast_channel_change_timer(void *a)
{
  int64_t time_offset = 0;
  uint64_t twos_complement_offset = 0;
  uint16_t temp_bcast_slot_number = 0;
  uint64_t time_now = 0;
  uint64_t next_bi = 0;
  
  if((mac_self_fan_info.bcast_sched.bs_schedule.channel_function == CF_DH1) ||
     (mac_self_fan_info.bcast_sched.bs_schedule.channel_function == CF_FIXED_CHANNEL)
       ||(mac_self_fan_info.bcast_sched.bs_schedule.channel_function == CF_TR51))
  {
calculate_again:
    time_now = get_time_now_64();
    temp_bcast_slot_number = mac_self_fan_info.bcast_sched.rcvd_broadcast_slot_nuumber +
      ((time_now - trxsm_p->bc_chan_hop_seq_start_time) / 
       (uint64_t)((uint64_t)mac_self_fan_info.bcast_sched.bcast_interval * (uint64_t)1000));
    
    time_offset = (int64_t)(time_now - trxsm_p->bc_chan_hop_seq_start_time) - 
      (int64_t)((int64_t)(temp_bcast_slot_number - mac_self_fan_info.bcast_sched.rcvd_broadcast_slot_nuumber) 
                * (int64_t)mac_self_fan_info.bcast_sched.bcast_interval * (int64_t)1000);
    time_offset /= 1000;
    twos_complement_offset = ~time_offset;
    twos_complement_offset += 1;
    next_bi = (uint64_t)mac_self_fan_info.bcast_sched.bcast_interval + twos_complement_offset;
    if (next_bi < (uint64_t)2)
      goto calculate_again;
    
    if (next_bi > (uint64_t)mac_self_fan_info.bcast_sched.bcast_interval)
      next_bi = (uint64_t)mac_self_fan_info.bcast_sched.bcast_interval;
    
    broadcast_slot_nuumber = temp_bcast_slot_number;
    if(broadcast_slot_nuumber == MAXIMUM_SLOT_NUMBER)
    {
      broadcast_slot_nuumber = 0;
      trxsm_p->bc_chan_hop_seq_start_time = time_now;
    }
    
//    log_slot_number (5, broadcast_slot_nuumber, 0xFF);
#if APP_LBR_ROUTER    
    start_brocast_shedule_interval_timer (next_bi);
    start_broadcast_dwellinterval_timer
      ((uint64_t)mac_self_fan_info.bcast_sched.bs_schedule.dwell_interval + 
       twos_complement_offset);
#endif    
    
   /*start timer to avoid the forbidden gaurd band of Broadcast Interval*/
    if (next_bi >= (uint64_t)(mac_self_fan_info.bcast_sched.bcast_interval - 3))
      procubed_broadcast_error_offset_timer_start (BROADCAST_ERROR_OFFSET);

/* Suneet :: Typecast is required :: otherwise values are overflowed */    
    trxsm_p->bc_chan_hop_seq_current_slot_time = trxsm_p->bc_chan_hop_seq_start_time + 
      ((uint64_t)(broadcast_slot_nuumber - mac_self_fan_info.bcast_sched.rcvd_broadcast_slot_nuumber) * 
       mac_self_fan_info.bcast_sched.bcast_interval * 1000);
//    set_channel_in_broadcast_dwell_interval = 1;
    set_channel_in_unicast_sech = 0x00;
    fan_freq_hop_update_broadcast_slot(NULL);
  }
}
/******************************************************************************/
void broadcast_channel_change()
{
  uint32_t braodcast_hop_channel=0;
  int brodcast_channel_index = 0;
  if(mac_self_fan_info.bcast_sched.bs_schedule.channel_function== CF_DH1)
  {
    if(fan_mac_information_data.fan_node_type == 0x01)
    {
      mac_nbr_descriptor_t *p_nbr_desc = NULL;
      #if APP_LBR_ROUTER
      p_nbr_desc = get_nbr_desc_from_addr (eapol_parent_child_info.sle_eapol_parent);
      brodcast_channel_index = getBroadcastChannelIndex(broadcast_slot_nuumber,mac_self_fan_info.bcast_sched.bcast_sched_id,p_nbr_desc->nbrchannel_usable_list.total_usable_ch_broadcast);
      braodcast_hop_channel =  p_nbr_desc->nbrchannel_usable_list.broad_usable_channel_list[brodcast_channel_index];
      #endif
    }
    else
    {
      brodcast_channel_index = getBroadcastChannelIndex(broadcast_slot_nuumber,mac_self_fan_info.bcast_sched.bcast_sched_id,usable_channel.total_usable_ch_broadcast);
      braodcast_hop_channel = usable_channel.broad_usable_channel_list[brodcast_channel_index];
    }
  }
  else if(mac_self_fan_info.bcast_sched.bs_schedule.channel_function == CF_TR51)
  {
//    if(broadcast_slot_nuumber==total_nos_of_channel)
//    {
//      broadcast_slot_nuumber =0;
//      trxsm_p->bc_chan_hop_seq_start_time = get_time_now_64();
//    }  
//    
//    braodcast_hop_channel = HopSequenceTable[broadcast_slot_nuumber];
  }
  if(phy_pib.TRXState==PHY_RX_ON)
  {
    if(mac_self_fan_info.bcast_sched.bs_schedule.channel_function != 0x00)
    { 
      if((dont_change_ubcast_channel != 0x88) && (is_hif_receive_in_process() != 1))
      {  
        PLME_set_request(phyCurrentChannel,2,&braodcast_hop_channel);
      }
    }
  }
}

#if(FAN_FRQ_HOPPING_FEATURE_ENABLED == 1)
uint64_t get_current_bcast_slot_start_time (void)
{
  return trxsm_p->bc_chan_hop_seq_current_slot_time;
}
#endif

uint8_t is_BDI_active (void)
{
//  uint64_t own_next_slot_start = 0;
  
  #if(FAN_EDFE_FEATURE_ENABLED == 1)
  if(edfe_information.edfe_frame_enabled == 0x01)
  {
    return 1;
  }
  else
#endif
  {
    if((mac_self_fan_info.unicast_listening_sched.us_schedule.channel_function == CF_DH1) ||
       (mac_self_fan_info.unicast_listening_sched.us_schedule.channel_function == CF_FIXED_CHANNEL))
    {
      /*Debdeep :: 07-sep-2018 :: we always return false and queue the broadcast packet. 
      We sent the packet out when next broadcast slot is started. */
      //    if (set_channel_in_broadcast_dwell_interval)
      //    {
      //      own_next_slot_start = mac_self_fan_info.bcast_sched.bs_schedule.dwell_interval - 
      //      (uint8_t)((get_time_now_64() - get_current_bcast_slot_start_time()) 
      //         / 1000);
      //      
      //      /* 3 milliseconds is taken as time to start transmit the packet*/
      //      if (own_next_slot_start > 3)
      //        return 1;
      //      else
      //        return 0;
      //    }
      //    else
      return 0;
    }
    else
    {
      return 1;
    }
  }
}

uint8_t is_UDI_active (void)
{
  if(edfe_information.edfe_frame_enabled == 0x01)
  {
    return 1;
  }
  else
  {
    if(mac_self_fan_info.unicast_listening_sched.us_schedule.channel_function != CF_FIXED_CHANNEL)
      return set_channel_in_unicast_sech;
    else
      return 1;
  }
}

/******************************************************************************/
void stop_broadcast_schedule (void)
{
  stop_broadcast_dwellinterval_timer ();
  broadcast_slot_nuumber = 0;
  mac_self_fan_info.bcast_sched.bcast_interval = 0;
  trxsm_p->bc_chan_hop_seq_start_time = 0;
  trxsm_p->bc_chan_hop_seq_current_slot_time = 0;
  mac_self_fan_info.bcast_sched.is_broadcast_sch_active = 0;
  mac_self_fan_info.bcast_sched.rcvd_broadcast_slot_nuumber = 0;
  set_channel_in_unicast_sech = 0x01;
}

int get_join_state (void);
void watch_unicast_broadcast_scheduling (void)
{
  static uint32_t loop_count;
  static uint16_t last_unicast_slot_number;
  static uint16_t last_broadcast_slot_number;
  
  loop_count++;
  if (loop_count < (uint32_t)999999)
    return;
  
  loop_count = 0;
  if(mac_self_fan_info.unicast_listening_sched.us_schedule.channel_function == CF_DH1)
  {
    if ((last_unicast_slot_number == unicast_slot_no) && (unicast_timer_started == 1))
    {
#if ((PRINT_DEBUG_LEVEL==ERROR_DEBUG)||(PRINT_DEBUG_LEVEL==CONSOLE_ERROR_DEBUG))
      stack_print_debug ("### Unicast schedule recovered\n");
#endif
      unicast_channel_change_timer (NULL);
    }
  }
  
  if((mac_self_fan_info.unicast_listening_sched.us_schedule.channel_function == CF_DH1) ||
     (mac_self_fan_info.unicast_listening_sched.us_schedule.channel_function == CF_FIXED_CHANNEL))
  {
    if ((last_broadcast_slot_number == broadcast_slot_nuumber) &&
        (mac_self_fan_info.bcast_sched.is_broadcast_sch_active == 1))
    {
#if ((PRINT_DEBUG_LEVEL==ERROR_DEBUG)||(PRINT_DEBUG_LEVEL==CONSOLE_ERROR_DEBUG))
      stack_print_debug ("### Broadcast schedule recovered\n");
#endif      
      broadcast_channel_change_timer (NULL);
    }
  }
  
  last_unicast_slot_number = unicast_slot_no;
  last_broadcast_slot_number = broadcast_slot_nuumber;
}
#endif //#ifdef WISUN_FAN_MAC

#endif //#if(FAN_FRQ_HOPPING_FEATURE_ENABLED == 1)

