/** \file mac_nbr_manager.c
 *******************************************************************************
 ** \brief 
 **
 ** \cond 
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



/*******************************************************************************
* File inclusion
*******************************************************************************/
#include "StackMACConf.h"
#include "common.h"
#include "queue_latest.h"
#include "buff_mgmt.h"
#include "event_manager.h"
#include "hw_tmr.h"
#include "phy.h"
#include "mac_config.h"
#include "mac.h"
#include "mac_defs_sec.h"
#include "ie_element_info.h"
#include "mac_defs.h"
#include "mac_pib.h"
#include "fan_config_param.h"
#include "mac_nbr_manager.h"
#include "fan_mac_ie.h"
#include "tri_tmr.h"
#include "sm.h" 
#include "fan_mac_interface.h"
#include "fan_mac_nbr_info.h"
//#include "common_function.h"
/*
** =============================================================================
** Private Macro definitions
** =============================================================================
*/
	
#define PAN_SIZE_OFFSET         2
#define ROUTING_COST_OFFSET     2
#define NS_PROBING_TIME         3*60*1000   //1000 CLOCK_CONF_SECOND
/*None*/

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

#ifdef WISUN_FAN_MAC
uint8_t authnt_interfac_id [8] = {0};
uint8_t recived_pa_packet = 0x00;
uint8_t pas_send_once;
uint64_t time_local=0;
uint16_t slott_no=0;  
uint32_t received_time =0;
self_info_fan_mac_t mac_self_fan_info;
mac_nbr_data_t mac_nbr_data;

fan_mac_nbr_t fan_mac_nbr = {
  .mac_nbr_info_table_entries = 0x00
};//created for static_mac
#endif

/*
** =============================================================================
** Public Variable Definitions
** =============================================================================
**/

/* None */

/*
** =============================================================================
** Public Function Definitions
** =============================================================================
**/

void chack_node_goes_in_consis_inconsi(mac_rx_t *mrp);
void find_lowest_pancost_from_nbr_table_for_mac_address(uint8_t *src_addr);
void calu_pan_cost_recv_pan();
uint16_t find_lowest_pancostfrom_nbr_table();
uint64_t get_time_now_64 (void);
int get_join_state (void);
void trickle_timer_inconsistency_pc(void);
#ifdef WISUN_FAN_MAC
mac_nbr_descriptor_t* get_nbr_desc_from_addr (uint8_t *addr);
extern uint8_t get_fan_device_type(void);

#if(FAN_EAPOL_FEATURE_ENABLED == 1)
extern void get_eapol_parent_address(uint8_t *eapol_parent);

#if(APP_LBR_ROUTER == 1)
extern parent_child_info_tag eapol_parent_child_info;
#endif
#endif

void stop_broadcast_ctimer(void);
uint8_t check_probing_is_needed(uint8_t *device_ll_addr);
typedef  struct comp_rssi{
  int val;
  uint8_t  mac_addr[8];
}comp_rssi_t;
typedef struct comp_pancost
{
  float pancval;
  uint8_t  mac_addr[8];
}comp_pancost_t;
static void quickSort_float(comp_pancost_t arr[], int low, int high);
extern void quickSort(comp_rssi_t arr[], int low, int high);
#endif
/*
** =============================================================================
** External Variable Declarations
** =============================================================================
*/


/*
** =============================================================================
** External Function Declarations
** =============================================================================
*/


extern void *app_bm_alloc(uint16_t length);      
extern void app_bm_free(uint8_t *pMem);
extern void mem_rev_cpy(uint8_t* dest, uint8_t* src, uint16_t len );
#if(APP_LBR_ROUTER == 1)
extern void uip_ds6_nbr_update_rsl_ie(uint8_t* , uint8_t* , int32_t );
#endif
extern void uip_ds6_update_rsl_ie_event(uint8_t* rcvd_addr, int32_t rssi);
void acquire_lbr_brodcast_shedule_start_self_broadcast_shedule (mac_rx_t *mrp);
void start_brodacst_schdeule();

#if(FAN_FRQ_HOPPING_FEATURE_ENABLED == 1)
extern uint16_t broadcast_slot_nuumber;
#endif


/*
** ============================================================================
** Private Function Prototypes
** ============================================================================
*/

void update_self_pan_version (uint16_t src_pan_version);
void make_nbr_valid_channel_list(mac_nbr_descriptor_t* nbr_desc);
void update_pc_consistency(void);
/* None */

/*
** ============================================================================
** Private Function Definitions
** ============================================================================
*/

/* None */




/******************************************************************************/

/*
** ============================================================================
** Public Function Definitions
** ============================================================================
*/

#ifdef WISUN_FAN_MAC
/******************************************************************************/

void initialise_nbr_table (void)
{
  queue_initialise(&fan_mac_nbr.desc_table);
}

void increment_self_pan_size_on_lbr(void)
{
  mac_self_fan_info.pan_metrics.pan_size++;
}

void increment_self_pan_size_routing_cost (void)
{
  if (get_fan_device_type() != 0x00)        //not LBR
  {
    mac_self_fan_info.pan_metrics.pan_size += PAN_SIZE_OFFSET;
    mac_self_fan_info.pan_metrics.routing_cost += ROUTING_COST_OFFSET;
  }
}

void update_mac_routing_cost (uint16_t etx, uint8_t* addr)
{
  uint8_t ii = 0;
  mac_nbr_descriptor_t *p_nbr_desc = NULL;
  uint8_t entries = fan_mac_nbr.mac_nbr_info_table_entries;
  uint8_t eapol_parent_addr[8] = {0};
  uint8_t mac_addr[8] = {0};
  uint16_t routing_cost = 0;
  
  if (addr == NULL)
    return;
  
  mem_rev_cpy (mac_addr, addr, 8);
#if(FAN_EAPOL_FEATURE_ENABLED == 1)
  get_eapol_parent_address (eapol_parent_addr);
#endif
  
  for (ii = 0; ii < entries; ii++)
  {
    p_nbr_desc = (mac_nbr_descriptor_t*)queue_item_read_from (&fan_mac_nbr.desc_table, ii);
    if (p_nbr_desc == NULL)
      continue; //Should not happen
    
    if (!memcmp (mac_addr, p_nbr_desc->mac_addr, IEEE_ADDRESS_LENGTH))
    {
      if (!(memcmp (eapol_parent_addr, addr, IEEE_ADDRESS_LENGTH))
          || (p_nbr_desc->is_parent_status == 2))       //2 for PREFERRED_PARENT
      {
        mac_self_fan_info.pan_metrics.pan_size = p_nbr_desc->rev_pan_metrics.pan_size;
        routing_cost = p_nbr_desc->rev_pan_metrics.routing_cost + etx;
        mac_self_fan_info.pan_metrics.routing_cost = routing_cost;
      }
      break;
    }
  }

  return;
}
 
void update_nbr_etx (uint8_t *addr, uint16_t etx)
{
  uint8_t mac_addr[8] = {0};
  mac_nbr_descriptor_t *p_nbr_desc = NULL;
  
  mem_rev_cpy (mac_addr, addr, IEEE_ADDRESS_LENGTH);
  p_nbr_desc = get_nbr_desc_from_addr (mac_addr);
  if (NULL != p_nbr_desc)
    p_nbr_desc->etx = etx;
}

void set_parent_status_in_mac_nbr_table (uint8_t *addr, uint8_t parent_status)
{
  uint8_t mac_addr[8] = {0};
  mac_nbr_descriptor_t *p_nbr_desc = NULL;
  
  mem_rev_cpy (mac_addr, addr, IEEE_ADDRESS_LENGTH);
  p_nbr_desc = get_nbr_desc_from_addr (mac_addr);
  if (NULL != p_nbr_desc)
    p_nbr_desc->is_parent_status = parent_status;
  
#if ((APP_LBR_ROUTER == 1) && (FAN_EAPOL_FEATURE_ENABLED == 1))
  if (parent_status == 2)       /* 2 for PREFERRED_PARENT */
    memcpy (eapol_parent_child_info.sle_eapol_parent, mac_addr, 8);
#endif
  
}

mac_nbr_descriptor_t* get_nbr_desc_from_addr (uint8_t *addr)
{ 
  uint8_t ii = 0;
  mac_nbr_descriptor_t *p_nbr_desc = NULL;
  uint8_t entries = fan_mac_nbr.mac_nbr_info_table_entries;
  
  if (addr == NULL)
    return NULL;
  
  for (ii = 0; ii < entries; ii++)
  {
    p_nbr_desc = (mac_nbr_descriptor_t*)queue_item_read_from (&fan_mac_nbr.desc_table, ii);
    if (p_nbr_desc == NULL)
      continue; //Should not happen
    
    if (0 == memcmp (p_nbr_desc->mac_addr, addr, IEEE_ADDRESS_LENGTH))
      return p_nbr_desc;
  }
  
  return NULL;
}
/******************************************************************************/
mac_nbr_descriptor_t* add_device_to_nbr_desc(mac_rx_t *mrp)
{
  mac_nbr_descriptor_t *p_nbr_desc = NULL;
  
  if (mrp == NULL)
    return NULL;
  
  if(mrp->recived_frame_type == 0x00)
    recived_pa_packet++; //Debdeep
  
  p_nbr_desc = get_nbr_desc_from_addr (mrp->src.address.ieee_address);
  if (NULL != p_nbr_desc)
    return p_nbr_desc;
  
  if (fan_mac_nbr.mac_nbr_info_table_entries == MAX_NBR_SUPPORT)
    return NULL;
  
  p_nbr_desc = (mac_nbr_descriptor_t *)app_bm_alloc (sizeof (mac_nbr_descriptor_t));
  if (p_nbr_desc == NULL)
  {
#if ((PRINT_DEBUG_LEVEL==ERROR_DEBUG)||(PRINT_DEBUG_LEVEL==CONSOLE_ERROR_DEBUG))
    stack_print_debug ("### mac_nbr_descriptor_t malloc fail\n");
#endif    
    return NULL;
  }
  
  memset (p_nbr_desc, 0, sizeof (mac_nbr_descriptor_t));
  p_nbr_desc->nbrchannel_usable_list.unicast_usable_channel_list = NULL;
  p_nbr_desc->nbrchannel_usable_list.broad_usable_channel_list = NULL;
  memcpy (p_nbr_desc->mac_addr, mrp->src.address.ieee_address, 8);
  p_nbr_desc->device_status = 0x01;
  queue_item_put ((queue_t *)&fan_mac_nbr.desc_table, (queue_item_t*)p_nbr_desc);
  fan_mac_nbr.mac_nbr_info_table_entries = queue_count_get ((queue_t *)&fan_mac_nbr.desc_table);
  
#if ((PRINT_DEBUG_LEVEL==ERROR_DEBUG)||(PRINT_DEBUG_LEVEL==CONSOLE_ERROR_DEBUG))
  stack_print_debug ("NBR added [%d] ", fan_mac_nbr.mac_nbr_info_table_entries);
  print_mac_address (p_nbr_desc->mac_addr);
#endif  
  
#if (PRINT_DEBUG_LEVEL==RUNTIME_LOG_FILE)
  send_runtime_log ("NBR added "LOG_MAC_ADDRESS(p_nbr_desc->mac_addr));
#endif
  
  p_nbr_desc = NULL;
  p_nbr_desc = get_nbr_desc_from_addr (mrp->src.address.ieee_address);
  return p_nbr_desc;   
}

void delete_device_from_nbr_desc (mac_nbr_descriptor_t *nbr)
{
  uint8_t ii = 0;
  mac_nbr_descriptor_t *p_nbr_desc = NULL;
  uint8_t entries = fan_mac_nbr.mac_nbr_info_table_entries;
  
  for (ii = 0; ii < entries; ii++)
  {
    p_nbr_desc = (mac_nbr_descriptor_t*)queue_item_read_from (&fan_mac_nbr.desc_table, ii);
    if (p_nbr_desc == NULL)
      continue; //Should not happen
    
    if (0 == memcmp (p_nbr_desc->mac_addr, nbr->mac_addr, IEEE_ADDRESS_LENGTH))
    {
      if (p_nbr_desc->nbrchannel_usable_list.unicast_usable_channel_list != NULL)
        app_bm_free (p_nbr_desc->nbrchannel_usable_list.unicast_usable_channel_list);
      if (p_nbr_desc->nbrchannel_usable_list.broad_usable_channel_list != NULL)
        app_bm_free (p_nbr_desc->nbrchannel_usable_list.broad_usable_channel_list);
      queue_item_delete (&fan_mac_nbr.desc_table, ii);
      fan_mac_nbr.mac_nbr_info_table_entries = queue_count_get ((queue_t *)&fan_mac_nbr.desc_table);
      return;
    }
  }
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
void calu_pan_cost_recv_pan()
{
  /*In the case where a FAN node must choose from one of several available PANs, 
  this might be achieved by choosing the PAN with the lowest PAN Cost, 
  where PAN Cost is calculated as follows:
  (PanRoutingCost / PRC_WEIGHT_FACTOR)  + (PanSize / PS_WEIGHT_FACTOR) 
  
  PanRoutingCost = Routing Cost from candidate target’s PAN-IE + local ETX to the candidate target.
  PanSize = PAN Size from the candidate target’s PAN-IE.
  */
  
  uint8_t ii = 0;
  mac_nbr_descriptor_t *p_nbr_desc = NULL;
  uint16_t pan_routing_cost = 0x0000;
  uint16_t pan_size = 0x0000;
  uint8_t entries = fan_mac_nbr.mac_nbr_info_table_entries;
  
  for (ii = 0; ii < entries; ii++)
  {
    p_nbr_desc = (mac_nbr_descriptor_t*)queue_item_read_from (&fan_mac_nbr.desc_table, ii);
    if (p_nbr_desc == NULL)
      continue; //Should not happen
    
    pan_size = p_nbr_desc->rev_pan_metrics.pan_size;
    pan_routing_cost = p_nbr_desc->rev_pan_metrics.routing_cost;
    p_nbr_desc->pan_cost_clc = 
      (( (float)pan_routing_cost / PRC_WEIGHT_FACTOR )
         +( (float)pan_size / PS_WEIGHT_FACTOR ));
  }
}

/* Debdeep added these API to get threshold RSSI values for parent candidate */
int8_t get_min_sense_rssi()
{
  return MIN_RSSI_THRESHOLD;
}
int8_t get_cand_parent_threshold ()
{
  return APP_CFG_CAND_PARENT_THRESHOLD;
}
int8_t get_cand_parent_hysterysis()
{
  return APP_CFG_CAND_PARENT_HYSTERESIS;
}
    
uint8_t get_LQI_from_RSSI( int8_t rssi_val );
//static uint8_t is_cand_eligible_for_eapol_parent (int16_t rssi)
//{
//  uint8_t nbr_lqi = get_LQI_from_RSSI (rssi);
//  uint8_t threshold_lqi = get_LQI_from_RSSI (MIN_RSSI_THRESHOLD+
//                                             APP_CFG_CAND_PARENT_THRESHOLD+
//                                               APP_CFG_CAND_PARENT_HYSTERESIS);
//  
//  if (nbr_lqi >= threshold_lqi)
//    return 1;
//  else
//    return 0;
//}

void find_lowest_pancost_from_nbr_table_for_mac_address(uint8_t *src_addr)
{
  int rssi_best_range_high = 0;
  int rssi_best_range_low  = 0;
  uint8_t ii = 0;
  uint8_t jj = 0;
  mac_nbr_descriptor_t *p_nbr_desc = NULL;
  comp_rssi_t mac_nbr_entries[MAX_NBR_SUPPORT];
  comp_pancost_t mac_nbr_for_pancost[MAX_NBR_SUPPORT];
  uint8_t entries = fan_mac_nbr.mac_nbr_info_table_entries;
  
#if (PRINT_DEBUG_LEVEL==RUNTIME_LOG_FILE)
    send_runtime_log ("EAPOL Parent Selection");
#endif  
  
  if ((pas_send_once > 3) && (entries == 1))
  {
    p_nbr_desc = (mac_nbr_descriptor_t*)queue_item_read_from (&fan_mac_nbr.desc_table, ii);
#if(FAN_EAPOL_FEATURE_ENABLED == 1)
    if ((p_nbr_desc != NULL) && (p_nbr_desc->eapol_parent_unresponsive == 0))
      memcpy(src_addr, p_nbr_desc->mac_addr, 8);
#endif
    
#if (PRINT_DEBUG_LEVEL==RUNTIME_LOG_FILE)
    send_runtime_log ("Only 1 nbr found after sending 3 PAS");
    send_runtime_log ("--Selected EAPOL parent "LOG_MAC_ADDRESS(p_nbr_desc->mac_addr));
#endif
    return;
  }
  
  memset (mac_nbr_entries, 0x00, sizeof(mac_nbr_entries));
  memset (mac_nbr_for_pancost, 0x00, sizeof(mac_nbr_for_pancost));
  
  for (ii = 0; ii < entries; ii++)
  {
    p_nbr_desc = (mac_nbr_descriptor_t*)queue_item_read_from (&fan_mac_nbr.desc_table, ii);

#if(FAN_EAPOL_FEATURE_ENABLED == 1)
    if ((p_nbr_desc == NULL) || (p_nbr_desc->eapol_parent_unresponsive == 1))
    {
#if (PRINT_DEBUG_LEVEL==RUNTIME_LOG_FILE)
      send_runtime_log ("eapol_parent_unresponsive Discard "LOG_MAC_ADDRESS(p_nbr_desc->mac_addr));
#endif      
      continue;
    }
#endif // #if(FAN_EAPOL_FEATURE_ENABLED == 1)
    
    
#if (PRINT_DEBUG_LEVEL==RUNTIME_LOG_FILE)
    send_runtime_log ("NBR[%d] RSSI[%d]", ii, p_nbr_desc->rssi);
    send_runtime_log ("--ADDR "LOG_MAC_ADDRESS(p_nbr_desc->mac_addr));
#endif
    
    memcpy(&mac_nbr_entries[jj].mac_addr, p_nbr_desc->mac_addr,8);
    mac_nbr_entries[jj].val = p_nbr_desc->rssi;
    jj++;
  }
  quickSort (mac_nbr_entries,0, jj-1);
  rssi_best_range_high = mac_nbr_entries[jj-1].val;
  rssi_best_range_low  = (mac_nbr_entries[jj-1].val - APP_CFG_RSSI_BAND);
  jj = 0;
  
  for (ii = 0; ii < entries; ii++)
  {
    p_nbr_desc = (mac_nbr_descriptor_t*)queue_item_read_from (&fan_mac_nbr.desc_table, ii);
    
#if(FAN_EAPOL_FEATURE_ENABLED == 1)
    if ((p_nbr_desc == NULL) || (p_nbr_desc->eapol_parent_unresponsive == 1))
      continue;   
#endif
    
    if((p_nbr_desc->rssi <= rssi_best_range_high)
       &&((p_nbr_desc->rssi >= rssi_best_range_low)))
    {
      memcpy(&mac_nbr_for_pancost[jj].mac_addr, p_nbr_desc->mac_addr,8);
      mac_nbr_for_pancost[jj].pancval = p_nbr_desc->pan_cost_clc;
      jj++;
      
#if (PRINT_DEBUG_LEVEL==RUNTIME_LOG_FILE)
      send_runtime_log ("NBR[%d] PAN Cost[%f]", ii, p_nbr_desc->pan_cost_clc);
      send_runtime_log ("--ADDR "LOG_MAC_ADDRESS(p_nbr_desc->mac_addr));
#endif
    }
    else
    {
#if (PRINT_DEBUG_LEVEL==RUNTIME_LOG_FILE)
      send_runtime_log ("RSSI range Discard "LOG_MAC_ADDRESS(p_nbr_desc->mac_addr));
#endif  
    }
  }
  quickSort_float(mac_nbr_for_pancost,0,jj-1);
  memcpy(src_addr, mac_nbr_for_pancost[0].mac_addr, 8);
  
#if (PRINT_DEBUG_LEVEL==RUNTIME_LOG_FILE)
  send_runtime_log ("Selected EAPOL parent "LOG_MAC_ADDRESS(p_nbr_desc->mac_addr));
#endif
}

void set_eapol_parent_unresponsive (uint8_t *addr)
{
  mac_nbr_descriptor_t *p_nbr_desc = get_nbr_desc_from_addr (addr);
  
  #if(FAN_EAPOL_FEATURE_ENABLED == 1)
  if (p_nbr_desc != NULL)
    p_nbr_desc->eapol_parent_unresponsive = 1;
#endif
  
}

/* Debdeep modified this function to eliminate EAPOL parent candidate with weak RSSI value */
//void find_lowest_pancost_from_nbr_table_for_mac_address(uint8_t *src_addr)
//{
////  uint8_t ii = 0;
////  float smallest_pan_cost = 999.99 ;
////  mac_nbr_descriptor_t *p_nbr_desc = NULL;
////  uint8_t entries = fan_mac_nbr.mac_nbr_info_table_entries;
//  
//  //short_mac_enteries_for_eapol_parents_selection();
//  
////  if ((pas_send_once > 3) && (entries == 1))
////  {
////    p_nbr_desc = (mac_nbr_descriptor_t*)queue_item_read_from (&fan_mac_nbr.desc_table, ii);
////    if (p_nbr_desc != NULL)
////      memcpy(src_addr, p_nbr_desc->mac_addr,8);
////    return;
////  }
////  
////  for (ii = 0; ii < entries; ii++)
////  {
////    p_nbr_desc = (mac_nbr_descriptor_t*)queue_item_read_from (&fan_mac_nbr.desc_table, ii);
////    if (p_nbr_desc == NULL)
////      continue; //Should not happen
////    
////    if (is_cand_eligible_for_eapol_parent (p_nbr_desc->rssi) == 1)
////    {
////      if (p_nbr_desc->pan_cost_clc < smallest_pan_cost) 
////      {
////        smallest_pan_cost = p_nbr_desc->pan_cost_clc;
////      }
////    }
////  }
////  
////  for (ii = 0; ii < entries; ii++)
////  {
////    p_nbr_desc = (mac_nbr_descriptor_t*)queue_item_read_from (&fan_mac_nbr.desc_table, ii);
////    if (p_nbr_desc == NULL)
////      continue; //Should not happen
////    
////    if (is_cand_eligible_for_eapol_parent 
////        (p_nbr_desc->rssi) == 1)
////    {
////      if(smallest_pan_cost == p_nbr_desc->pan_cost_clc)
////      {
////        memcpy(src_addr, p_nbr_desc->mac_addr,8);
////      }
////    }
////  }
//}

uint16_t find_lowest_pancostfrom_nbr_table()
{
  uint8_t ii = 0;
  float smallest_pan_cost = 999.99 ;
  mac_nbr_descriptor_t *p_nbr_desc = NULL;
  uint8_t entries = fan_mac_nbr.mac_nbr_info_table_entries;
  
  for (ii = 0; ii < entries; ii++)
  {
    p_nbr_desc = (mac_nbr_descriptor_t*)queue_item_read_from (&fan_mac_nbr.desc_table, ii);
    if (p_nbr_desc == NULL)
      continue; //Should not happen
    
    if (p_nbr_desc->pan_cost_clc < smallest_pan_cost) 
    {
      smallest_pan_cost = p_nbr_desc->pan_cost_clc;
    }
  }
  
  for (ii = 0; ii < entries; ii++)
  {
    p_nbr_desc = (mac_nbr_descriptor_t*)queue_item_read_from (&fan_mac_nbr.desc_table, ii);
    if (p_nbr_desc == NULL)
      continue; //Should not happen
    
    if(smallest_pan_cost == p_nbr_desc->pan_cost_clc)
    {
      return p_nbr_desc->rev_pan_metrics.pan_id;
    }
  }
  
  return 0xFFAA;
}
/******************************************************************************/

/******************************************************************************/
void chack_node_goes_in_consis_inconsi(mac_rx_t *mrp)
{
  mac_nbr_descriptor_t* p_nbr_desc = get_nbr_desc_from_addr (mrp->src.address.ieee_address);
  
  if (p_nbr_desc == NULL)
    return;
  
  if(mrp->recived_frame_type == PAN_ADVERT_SOLICIT)
  {
    if(!(memcmp(p_nbr_desc->net_name,mac_self_fan_info.net_name,p_nbr_desc->net_name_length)))
    {
      if(fan_mac_information_data.state_ind == JOIN_STATE_5)
      {
        trickle_timer_inconsistency_pa();
      }
      else
      {
        trickle_timer_consistency_pas();
        delete_device_from_nbr_desc (p_nbr_desc);
//        fan_mac_nbr.mac_nbr_info_table_entries--;
//        p_nbr_desc->device_status = 0x00;
//        memset(p_nbr_desc, 0x00, sizeof(mac_nbr_descriptor_t));  
      }
    }
  }
  else if(mrp->recived_frame_type == PAN_ADVERT_FRAME)
  {
    if((p_nbr_desc->rev_pan_metrics.routing_cost <= mac_self_fan_info.pan_metrics.routing_cost )
       && (!(memcmp(p_nbr_desc->net_name,mac_self_fan_info.net_name,p_nbr_desc->net_name_length)))
         &&(mrp->src.pan_id == mac_self_fan_info.pan_metrics.pan_id))
    {
      trickle_timer_consistency_pa();
    }
    else if((p_nbr_desc->rev_pan_metrics.routing_cost > mac_self_fan_info.pan_metrics.routing_cost )
            && (!(memcmp(p_nbr_desc->net_name,mac_self_fan_info.net_name,p_nbr_desc->net_name_length)))
              &&(mrp->src.pan_id == mac_self_fan_info.pan_metrics.pan_id))
    {
      trickle_timer_inconsistency_pa();
    }
  }
  else if(mrp->recived_frame_type == PAN_CONFIG_SOLICIT)
  {
    if((!(memcmp(p_nbr_desc->net_name,mac_self_fan_info.net_name,p_nbr_desc->net_name_length)))
       &&(mrp->src.pan_id == mac_self_fan_info.pan_metrics.pan_id))
    {
      if(fan_mac_information_data.state_ind == JOIN_STATE_5)
      {
        trickle_timer_inconsistency_pc();
      }
      else
      {
        trickle_timer_consistency_pcs();
//        fan_mac_nbr.mac_nbr_info_table_entries--;
//        p_nbr_desc->device_status = 0x00;
//        memset(p_nbr_desc, 0x00, sizeof(mac_nbr_descriptor_t));
      }
    }
  }
  else if(mrp->recived_frame_type == PAN_CONFIG)
  {
    if((p_nbr_desc->pan_ver >= mac_self_fan_info.pan_ver )
       && (!(memcmp(p_nbr_desc->net_name,mac_self_fan_info.net_name,p_nbr_desc->net_name_length)))
         &&(mrp->src.pan_id == mac_self_fan_info.pan_metrics.pan_id))
    {
        trickle_timer_consistency_pc();
        update_pc_consistency();
    }
    else if((p_nbr_desc->pan_ver < mac_self_fan_info.pan_ver)
            && (!(memcmp(p_nbr_desc->net_name,mac_self_fan_info.net_name,p_nbr_desc->net_name_length)))
              &&(mrp->src.pan_id== mac_self_fan_info.pan_metrics.pan_id))
    {
      trickle_timer_inconsistency_pc();
    }
  }          
}
/******************************************************************************/
void throttle_pc (void);
void update_self_pan_version (uint16_t src_pan_version)
{
  /*Debdeep:: Decision should be taken here, whether to send 
  eapol key request for GTK update or not. 19-March-2018*/
  if (mac_self_fan_info.pan_ver < src_pan_version)
    mac_self_fan_info.pan_ver = src_pan_version;
  
  trickle_timer_inconsistency_pc ();
  throttle_pc ();
}
/******************************************************************************/
void update_pan_version_from_eapol_parent (void)
{
  mac_nbr_descriptor_t *p_nbr_desc = NULL;

#if(FAN_EAPOL_FEATURE_ENABLED == 1)
#if(APP_LBR_ROUTER == 1)
  p_nbr_desc = get_nbr_desc_from_addr (eapol_parent_child_info.sle_eapol_parent);
#endif
#endif
  
  if (NULL == p_nbr_desc)
    return;
  mac_self_fan_info.pan_ver = p_nbr_desc->pan_ver;
  trickle_timer_inconsistency_pc ();
  throttle_pc ();
}
/******************************************************************************/
void clean_mac_neighbour_table ()
{
  uint8_t ii = 0;
  mac_nbr_descriptor_t *p_nbr_desc = NULL;
  
  while ((ii = queue_count_get ((queue_t *)&fan_mac_nbr.desc_table)) > 0)
  {
    p_nbr_desc = (mac_nbr_descriptor_t*)queue_item_read_from (&fan_mac_nbr.desc_table, ii-1);
    if (p_nbr_desc->nbrchannel_usable_list.unicast_usable_channel_list != NULL)
      app_bm_free (p_nbr_desc->nbrchannel_usable_list.unicast_usable_channel_list);
    if (p_nbr_desc->nbrchannel_usable_list.broad_usable_channel_list != NULL)
      app_bm_free (p_nbr_desc->nbrchannel_usable_list.broad_usable_channel_list);
    queue_item_delete (&fan_mac_nbr.desc_table, ii-1);
  }
  fan_mac_nbr.mac_nbr_info_table_entries = 0;
  recived_pa_packet = 0;
  pas_send_once = 0;  
}
/******************************************************************************/
int fan_mac_nbr_num (void)
{
  return fan_mac_nbr.mac_nbr_info_table_entries;
}
/******************************************************************************/
void get_fan_mac_nbr_lladr_by_index (int index, uint8_t *mac_addr)
{  
  mac_nbr_descriptor_t *p_nbr_desc = (mac_nbr_descriptor_t *)queue_item_read_from (&fan_mac_nbr.desc_table, index);
  if (p_nbr_desc != NULL)  
    mem_rev_cpy (mac_addr, p_nbr_desc->mac_addr, 8);
}
uint8_t is_dis_sent (int index)
{
  mac_nbr_descriptor_t *p_nbr_desc = (mac_nbr_descriptor_t *)queue_item_read_from (&fan_mac_nbr.desc_table, index);
  if (p_nbr_desc != NULL)  
    return p_nbr_desc->sent_dis_count;
  
  return 0;
}
void mark_as_dis_sent (int index)
{
  mac_nbr_descriptor_t *p_nbr_desc = (mac_nbr_descriptor_t *)queue_item_read_from (&fan_mac_nbr.desc_table, index);
  if (p_nbr_desc != NULL)  
    p_nbr_desc->sent_dis_count++;
}
/******************************************************************************/
#if(FAN_FRQ_HOPPING_FEATURE_ENABLED == 1)

extern void start_broadcast_timer_channel_hop_listen(void);
int bcast_update_count = 0;
void start_timer_to_start_broadcast_schdeule(uint64_t expire_int);
uint64_t get_current_bcast_slot_start_time (void);

/******************************************************************************/

void start_brodacst_schdeule()
{
  start_broadcast_timer_channel_hop_listen();//start broadcast timer 
}
/******************************************************************************/
void acquire_lbr_brodcast_shedule_start_self_broadcast_shedule (mac_rx_t *mrp)
{
  uint64_t t1_2 = 0;
  uint32_t nbr_next_slot_start = 0;
  uint32_t own_next_slot_start = 0;
  uint32_t time_diff = 0;
  int32_t time_offset = 0;
  uint32_t twos_complement_offset = 0;
  mac_nbr_descriptor_t *p_nbr_desc = NULL;
  
  p_nbr_desc = (mac_nbr_descriptor_t *)get_nbr_desc_from_addr (mrp->src.address.ieee_address);
  if (p_nbr_desc == NULL)
	return;
  
  if ((get_join_state () == 5) && (p_nbr_desc->is_parent_status != 2))
    return; /* Dont Update broadcast schedule if in J5 and Rcved packet is not from Preferred Parent */
  
  uint64_t recv_time = (p_nbr_desc->btie_rcvd_timestamp / 1000);
  
  mac_self_fan_info.bcast_sched.bcast_sched_id = 
    p_nbr_desc->bcl_sched.bcast_sched_id;
  mac_self_fan_info.bcast_sched.bs_schedule.dwell_interval = 
    p_nbr_desc->bcl_sched.bs_schedule.dwell_interval;
  mac_self_fan_info.bcast_sched.bcast_interval = 
    p_nbr_desc->bcl_sched.bcast_interval;
  
  /*t1_1 – Node 0 broadcast_offset + Node 0 broadcast_interval*/
  t1_2 = (recv_time - p_nbr_desc->broad_frac_int_offset + 
          p_nbr_desc->bcl_sched.bcast_interval); 
  if (t1_2 < recv_time)
    return;     /*Failure: Corrupted BFIO*/
  
  nbr_next_slot_start = (uint32_t)(t1_2 - recv_time);
  
  own_next_slot_start = mac_self_fan_info.bcast_sched.bcast_interval - 
    ((uint32_t)(get_time_now_64() - get_current_bcast_slot_start_time()) 
     / 1000);
  
  if (nbr_next_slot_start > own_next_slot_start)
    time_diff = nbr_next_slot_start - own_next_slot_start;
  else
    time_diff = own_next_slot_start - nbr_next_slot_start;
  
  #if(FAN_FRQ_HOPPING_FEATURE_ENABLED == 1)
  if ((p_nbr_desc->broad_cast_slno == broadcast_slot_nuumber) /*&& (time_diff < (uint32_t)10) */ //suneet 9/9/2023 comented by need to check why we calculate duration here and compare like this :: 
      && (mac_self_fan_info.bcast_sched.is_broadcast_sch_active == 1))
#endif
    
    return;
  else
  {
    bcast_update_count++;
    mac_self_fan_info.bcast_sched.is_broadcast_sch_active = 0;
    mac_self_fan_info.bcast_sched.rcvd_t1_2 = t1_2;
    stop_broadcast_ctimer();
    time_offset = (uint32_t)((get_time_now_64() - p_nbr_desc->btie_rcvd_timestamp) / 1000);
    twos_complement_offset = ~time_offset;
    twos_complement_offset += 1;
    if ((uint32_t)(nbr_next_slot_start + twos_complement_offset) > mac_self_fan_info.bcast_sched.bcast_interval)
      start_timer_to_start_broadcast_schdeule(mac_self_fan_info.bcast_sched.bcast_interval);
    else
      start_timer_to_start_broadcast_schdeule(nbr_next_slot_start + twos_complement_offset);
    #if(FAN_FRQ_HOPPING_FEATURE_ENABLED == 1)
    broadcast_slot_nuumber = p_nbr_desc->broad_cast_slno;
#endif
  }
}

#endif

/******************************************************************************/
void make_nbr_valid_channel_list(mac_nbr_descriptor_t* nbr_desc)
{
  if((nbr_desc->nbrchannel_usable_list.unicast_usable_channel_list == NULL)
     || (nbr_desc->nbrchannel_usable_list.broad_usable_channel_list == NULL))
  {
    uint16_t length = 0;
    uint32_t current_max_channels=0x00;
     
    if((nbr_desc->ucl_sched.us_schedule.channel_plan == 0x00)
       ||(nbr_desc->bcl_sched.bs_schedule.channel_plan == 0x00))
    {
      if((fan_mac_information_data.state_ind == JOIN_STATE_5)
#if(APP_NVM_FEATURE_ENABLED == 1)
         &&(fan_mac_information_data.is_start_from_nvm == true)
#endif
           )
        current_max_channels = nbr_desc->nbrchannel_usable_list.total_usable_ch_unicast;  //suneet :: when store from nvm then recalulate information
      else
        PLME_get_request( phyMaxSUNChannelSupported, &length, &current_max_channels );
    }
    else
    {
      current_max_channels = nbr_desc->ucl_sched.un_channel_plan.ch_explicit.num_chans;
    }
    uint16_t tot_length = current_max_channels;
    uint8_t buf_unicast[129] = {0};
    uint8_t buf_broadcast[129] = {0};
    
    if( nbr_desc->ucl_sched.us_schedule.excludded_channel_control == EXCLUDED_CHANNEL_PRESENT)
    {
      uint8_t i=0,j=0,k=0,l=0;
      uint8_t chack_length = tot_length; 
      for(i=0;i<chack_length;i++)
      {
        if(/*send_chan_list[j]*/i == nbr_desc->ucl_sched.us_schedule.excluded_channels.excluded_channel_ranges.ex_ch_range[k].start_ch)
        {
          while(nbr_desc->ucl_sched.us_schedule.excluded_channels.excluded_channel_ranges.ex_ch_range[k].end_ch!= i)
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
      nbr_desc->nbrchannel_usable_list.total_usable_ch_unicast = l;
    }
    if(nbr_desc->bcl_sched.bs_schedule.excludded_channel_control == EXCLUDED_CHANNEL_PRESENT)
    {
      uint8_t i=0,j=0,k=0,l=0;
      uint8_t chack_length = tot_length; 
      for(i=0;i<chack_length;i++)
      {
        if(/*send_chan_list[j]*/i == nbr_desc->bcl_sched.bs_schedule.excluded_channels.excluded_channel_ranges.ex_ch_range[k].start_ch)
        {
          while(nbr_desc->bcl_sched.bs_schedule.excluded_channels.excluded_channel_ranges.ex_ch_range[k].end_ch!= i)
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
      nbr_desc->nbrchannel_usable_list.total_usable_ch_broadcast = l;
    }
    if(nbr_desc->bcl_sched.bs_schedule.excludded_channel_control == 0x00)
    {
      uint8_t l=0;
      for(l=0; l<tot_length ; l++)
      {
        buf_broadcast[l] = l;
      }
      nbr_desc->nbrchannel_usable_list.total_usable_ch_broadcast = l;
    }


    if(nbr_desc->ucl_sched.us_schedule.excludded_channel_control == 0x00)
    {
      uint8_t l=0;
      for(l=0; l<tot_length ; l++)
      {
        buf_unicast[l] = l;/*send_chan_list[j++]*/;
      }
      nbr_desc->nbrchannel_usable_list.total_usable_ch_unicast = l;
    }
    if(nbr_desc->bcl_sched.bs_schedule.excludded_channel_control == EXCLUDED_CHANNEL_MASK_PRESENT)
    {
      uint8_t loop_count = tot_length/8;
      uint8_t m=0,i=0,j=0 ,l=0;
      for(j=0;j<loop_count;j++)
      {
        for(i=0;i<8;i++)
        {
          if(nbr_desc->bcl_sched.bs_schedule.excluded_channels.excluded_channel_mask[j]& (0x01<<i))
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
      nbr_desc->nbrchannel_usable_list.total_usable_ch_broadcast = l;
    }
    if(nbr_desc->ucl_sched.us_schedule.excludded_channel_control == EXCLUDED_CHANNEL_MASK_PRESENT)
    {
      uint8_t loop_count = tot_length/8;
      uint8_t m=0,i=0,j=0 ,l=0;
      for(j=0;j<loop_count;j++)
      {
        for(i=0;i<8;i++)
        {
          if(nbr_desc->ucl_sched.us_schedule.excluded_channels.excluded_channel_mask[j]& (0x01<<i))
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
      nbr_desc->nbrchannel_usable_list.total_usable_ch_unicast = l;
    }
  
    if (nbr_desc->nbrchannel_usable_list.unicast_usable_channel_list == NULL)
    {
      nbr_desc->nbrchannel_usable_list.unicast_usable_channel_list = (uint8_t *)app_bm_alloc( nbr_desc->nbrchannel_usable_list.total_usable_ch_unicast);
      if(nbr_desc->nbrchannel_usable_list.unicast_usable_channel_list != NULL)
      {
        memcpy(nbr_desc->nbrchannel_usable_list.unicast_usable_channel_list,buf_unicast,nbr_desc->nbrchannel_usable_list.total_usable_ch_unicast);
      }
    }
    
    if (nbr_desc->nbrchannel_usable_list.broad_usable_channel_list == NULL)
    {
      nbr_desc->nbrchannel_usable_list.broad_usable_channel_list = (uint8_t *)app_bm_alloc( nbr_desc->nbrchannel_usable_list.total_usable_ch_broadcast);
      if(nbr_desc->nbrchannel_usable_list.broad_usable_channel_list != NULL)
      {
        memcpy(nbr_desc->nbrchannel_usable_list.broad_usable_channel_list,buf_broadcast,nbr_desc->nbrchannel_usable_list.total_usable_ch_broadcast);
      }
    }
  }
}
/*Suneet:: compare when prob is needed when last packet received  */
uint8_t check_probing_is_needed(uint8_t *device_ll_addr)
{
  uint8_t mac_addr[8] = {0x00};
  mem_rev_cpy(mac_addr,device_ll_addr,8);
   uint64_t current_time = get_time_now_64();
   uint32_t Diffrence_current_time_and__last_rx_time = 0x00;
    mac_nbr_descriptor_t *p_nbr_desc = (mac_nbr_descriptor_t *)get_nbr_desc_from_addr (mac_addr);
     if (p_nbr_desc != NULL)
     {
        Diffrence_current_time_and__last_rx_time = (uint32_t) (ceil ((float)(current_time - p_nbr_desc->ulp_rx_time) / (float)1000));
        if(Diffrence_current_time_and__last_rx_time >= NS_PROBING_TIME)
        {
          return 1;
        }
        return 0;
     }
     return 0;

}


/*----------------------------------------------------------------------------*/
// A utility function to swap two elements
static void swap(comp_pancost_t* a, comp_pancost_t* b)
{
    comp_pancost_t t = *a;
    *a = *b;
    *b = t;
}
/*---------------------------------------------------------------------------*/ 
/* This function takes last element as pivot, places
   the pivot element at its correct position in sorted
    array, and places all smaller (smaller than pivot)
   to left of pivot and all greater elements to right
   of pivot */
static int partition (comp_pancost_t arr[], int low, int high)
{
    float pivot = arr[high].pancval;// pivot
    int i = (low - 1);// Index of smaller element
 
    for (int j = low; j <= high- 1; j++)
    {
        // If current element is greater than or equal to pivot, for RSSI
        if (arr[j].pancval <= pivot)                   
        {
            i++;// increment index of smaller element
            swap(&arr[i], &arr[j]);
        }
    }
    swap(&arr[i + 1], &arr[high]);
    return (i + 1);
}
/*---------------------------------------------------------------------------*/ 
/* The main function that implements QuickSort
  arr[] --> Array to be sorted,
  low  --> Starting index,
  high  --> Ending index */
static void quickSort_float(comp_pancost_t arr[], int low, int high)
{
    if (low < high)
    {
        /* pi is partitioning index, arr[p] is now at right place */
        int pi = partition(arr, low, high);
 
        // Separately sort elements before partition and after partition
        quickSort_float(arr, low, pi - 1);
        quickSort_float(arr, pi + 1, high);
    }
}

#if(APP_NVM_FEATURE_ENABLED == 1)
void store_mac_nbr_from_nvm(mac_nbr_descriptor_t *p_nbr_desc)
{
  make_nbr_valid_channel_list(p_nbr_desc);
  queue_item_put ((queue_t *)&fan_mac_nbr.desc_table, (queue_item_t*)p_nbr_desc);
  fan_mac_nbr.mac_nbr_info_table_entries = queue_count_get ((queue_t *)&fan_mac_nbr.desc_table);
}
#endif

#endif