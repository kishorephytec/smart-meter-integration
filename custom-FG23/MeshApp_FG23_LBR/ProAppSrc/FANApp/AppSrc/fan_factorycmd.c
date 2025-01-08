/** \file fan_factorycmd.c
 *******************************************************************************
 ** \brief 
 ** Implements the RADIO PHY testing commnads
 **
 ** \cond STD_FILE_HEADER
 **
 ** COPYRIGHT
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
#include <stdlib.h>
#include "common.h"
#include "em_device.h"
#include "em_gpio.h"
#include "em_usart.h"
#include "list_latest.h"
#include "queue_latest.h"
#include "buff_mgmt.h"
#include "uart_hal.h"
#include "hif_utility.h"
#include "hif_service.h"
#include "hw_tmr.h"
#include "sw_timer.h"
#include "timer_service.h"
#include "phy.h"
#include "trx_access.h"
#include "mac.h"
#include "mac_app_build_config.h"
#include "mac_interface_layer.h"
#include "mac_pib.h"
#include "sm.h"
#include "ie_element_info.h"
#include "network-manager.h"
#include "fan_factorycmd.h"
#include "fan_api.h"



/*
** ============================================================================
** Public Function Definitions
** ============================================================================
*/
#define MAX_TOOL_UART_DATA_BUFF 512


uint8_t TRX_RAIL_setPower(uint8_t powerDbm);
uint8_t get_cuurunt_active_operation_mode();
sm_result_t node_factory_mode_state(fan_nwk_manager_sm_t *s, const sm_event_t *e);
static sm_result_t fsm_sending_packets(fan_nwk_manager_sm_t *s, const sm_event_t *e);
static sm_result_t fsm_receive_mode(fan_nwk_manager_sm_t *s, const sm_event_t *e);
static sm_result_t tsm_sending_unmod_stream( fan_nwk_manager_sm_t *s, const sm_event_t *e );
static sm_result_t tsm_sending_mod_stream( fan_nwk_manager_sm_t *s, const sm_event_t *e );
void send_next_pkt(void *s,void* tmr);
static void perform_bcast_data_tx( void* s );
void update_total_sending_dur_for_hr( void *s, void* tmr );
void tsm_alarm(void *s, void* tmr );
void set_datapkt_type();
void init_factory_mode();
extern uint32_t get_sun_page_value(void);
extern uint16_t get_sun_channel(void);
extern uint8_t phyModeMapArr[8];

/*
** =============================================================================
** External Variable Declarations
** =============================================================================
*/
extern void TRX_RAIL_do_EnergyDetection_Scanning ();
extern uint8_t send_hif_conf_cb( uint8_t cmd_id,uint8_t status );
extern mac_pib_t mac_pib;
extern fan_nwk_manager_sm_t fan_nwk_manager_app;
//extern uint8_t TRX_Config_CRC_for_RX_pkt();
extern void * app_bm_alloc( uint16_t length );
extern void app_bm_free( uint8_t *pMem );
uint32_t total_sending_in_hr[6];
uint32_t total_packets_sent[6];
uint8_t total_sending_in_hr_index = 0;
uint16_t total_packets_sent_in_an_hr = 0;
extern uint8_t response_laye_ID;
uint8_t mac_limit_reached_count = 0;
uint8_t max_sending_dur_reached = 0;
uint32_t txx_complete_event = 0;
p3time_t telec_start_time = 0;
p3time_t telec_exp_time = 0;
uint32_t time_stamp[2]={0};
rx_mac_frame_info_t rx_mac_data_ind;

int8_t set_RAIL_phyMode(uint8_t modeID);
void App_factory_mode_Data_ind_cb( uint16_t msduLength,uint8_t* pMsdu, int8_t mpduLinkQuality,uint16_t fcs_length);

/*==================================================================================*/
/*==================================================================================*/


/*----------------------------------------------------------------------------*/
void App_factory_mode_Data_ind_cb( uint16_t msduLength,uint8_t* pMsdu, int8_t mpduLinkQuality,uint16_t fcs_length)
{
  sm_event_t e = { (sm_trigger_t) TSM_TRIGGER_RX_COMPLETE, { 0 } };
//  char RSSIval = 0;     
  
//  RSSIval = convert_LQI_To_RSSI( mpduLinkQuality );
  rx_mac_data_ind.rssival  = mpduLinkQuality;
  rx_mac_data_ind.FCS_Length = fcs_length;
  rx_mac_data_ind.msduLength = msduLength;
  rx_mac_data_ind.pMsdu = pMsdu;
  e.param.vector = &rx_mac_data_ind;
  fan_nwk_manager_app.factory_mod_st.rx_packet_num += 0x01;
  //:: Suneet Added to despacth the event  recived pkt ic completed 
  SM_DISPATCH((sm_t *) &fan_nwk_manager_app,&e);
}

/*----------------------------------------------------------------------------*/

void init_factory_mode()
{
  uint32_t current_page = get_sun_page_value();;
  uint32_t channel = get_sun_channel();
  uint32_t rssi_threshold = fan_nwk_manager_app.factory_mod_st.rssi_threshold;
  PLME_set_request(phyCurrentSUNPageEntry,4,&current_page);
  PLME_set_request(phyCurrentChannel,2,&channel);
  PLME_set_request(phyRSSIThreshold, 1, &rssi_threshold);
    
  
  fan_nwk_manager_app.factory_mod_st.gmsduu = app_bm_alloc( MAX_TOOL_UART_DATA_BUFF); //Alocate memory 
  fan_nwk_manager_sm_t *s = &fan_nwk_manager_app;
  tmr_stop( &(s->factory_mod_st.ipdelay) );
  tmr_create_one_shot_timer
    (
     &(s->factory_mod_st.ipdelay),
     2000,
     (sw_tmr_cb_t)&tsm_alarm,
     s
       );
  tmr_stop( &(s->factory_mod_st.send_dur_tmr) );
  tmr_create_one_shot_timer
    (
     &(s->factory_mod_st.send_dur_tmr),
     3600000000L,
     (sw_tmr_cb_t)&update_total_sending_dur_for_hr,
     s
       );
  /*Suneet :: set 1 sec time to send next pkt 24-10-2016*/
  tmr_create_one_shot_timer
    (
     &(s->factory_mod_st.delay_next_pkt),
     50000,//1000000,//1sec Delay
     (sw_tmr_cb_t)&send_next_pkt,
     s
       );
}

/*==================================================================================*/

static sm_result_t fsm_sending_packets(fan_nwk_manager_sm_t *s, const sm_event_t *e)
{
  uint8_t i;
  uint16_t sent_pld_len = 0;
  uint8_t host_pkt[7] = { TX_PACKET_CONF,0x00};
  //	int u=0xFFFF;
  uint16_t max_rand_pkt_size = 1;
  switch ((fan_nwk_manager_sm_trigger_t) e->trigger)
  {
  case TRIGGER_ENTRY:
    s->state_ind = NODE_SENDIND_PACKET;
    //    
    //    /*choose to perform either broadcast or unicast based on
    //    the destination address setting*/
    s->factory_mod_st.fn_alarm = &perform_bcast_data_tx;
    for(i=0;i<8;i++)
    {
      if( s->factory_mod_st.trx_pkt_param.pkt_mode_param.dest_addr.address.ieee_address[i] != 0xFF )
      {
        //s->factory_mod_st.fn_alarm = &perform_64_bit_data_tx;
        break;
      }
    }
    
    telec_start_time =  timer_current_time_get();
    s->factory_mod_st.fn_alarm(s);
    tmr_start_relative( &(s->factory_mod_st.send_dur_tmr));
    break;
  case TSM_TRIGGER_TX_COMPLETE:
    //txx_complete_event++;
    telec_exp_time =  timer_current_time_get();
    s->result = (factory_sm_result_t) e->param.scalar;
    
    if( e->param.scalar == MAC_LIMIT_REACHED )
    {				                               
      if( !max_sending_dur_reached )
      {
        mac_limit_reached_count++;
        max_sending_dur_reached = 1;
        //telec_exp_time =  timer_current_time_get();
        host_pkt[1] = e->param.scalar;
        host_pkt[3] = (uint8_t)(mac_limit_reached_count);
        host_pkt[2] = (uint8_t)((mac_limit_reached_count)>>8);
        host_pkt[4] = (uint8_t)((((fan_nwk_manager_sm_t*)s)->factory_mod_st.tx_packet_counter)>>8);
        host_pkt[5] = (uint8_t)(((fan_nwk_manager_sm_t*)s)->factory_mod_st.tx_packet_counter);                                    
        hif_send_msg_up( host_pkt,5,response_laye_ID,PROTOCOL_ID_FOR_APP);
      }
      if( s->factory_mod_st.trx_pkt_param.pkt_mode_param.random_packet_len )
      {
        sm_transit((sm_t *)s, (sm_state_t)&node_factory_mode_state);
      }
      
    }
    
    if( (e->param.scalar != MAC_SUCCESS) && ((e->param.scalar != MAC_NO_ACK)))
    {
      ((fan_nwk_manager_sm_t*)s)->factory_mod_st.tx_packet_counter--;
    }
    
    if( e->param.scalar != MAC_LIMIT_REACHED )
    {
      
      max_sending_dur_reached = 0;
      /* Do not send confirmation if the status is MAC_LIMIT_REACHED as this event keeps happening once encounters the T108 restriction and the prints will be very fast*/
      host_pkt[1] = e->param.scalar; 
      /*Suneet :: if destination address is not zero */
      if(((fan_nwk_manager_sm_t*)s)->factory_mod_st.trx_pkt_param.pkt_mode_param.dest_addr.address_mode == ADDR_MODE_EXTENDED)
        sent_pld_len = (((fan_nwk_manager_sm_t*)s)->factory_mod_st.trx_pkt_param.pkt_mode_param.pkt_len) + 21 + ((((fan_nwk_manager_sm_t*)s)->factory_mod_st.trx_pkt_param.pkt_mode_param.fcslength)?4:2);
      else
        sent_pld_len = (((fan_nwk_manager_sm_t*)s)->factory_mod_st.trx_pkt_param.pkt_mode_param.pkt_len) + ((((fan_nwk_manager_sm_t*)s)->factory_mod_st.trx_pkt_param.pkt_mode_param.fcslength)?4:2);
      
      host_pkt[2] = (uint8_t)(sent_pld_len>>8);
      host_pkt[3] = (uint8_t)(sent_pld_len);
      host_pkt[4] = (uint8_t)((((fan_nwk_manager_sm_t*)s)->factory_mod_st.tx_packet_counter)>>8);
      host_pkt[5] = (uint8_t)(((fan_nwk_manager_sm_t*)s)->factory_mod_st.tx_packet_counter);
      host_pkt[6] = DUMMY_COMPORT; //Dummy Com port VAlue 
      hif_send_msg_up( host_pkt,6,response_laye_ID,PROTOCOL_ID_FOR_APP);
    }
    
    if ( ( s->factory_mod_st.trx_pkt_param.pkt_mode_param.pkt_cnt > s->factory_mod_st.tx_packet_counter ) && ( e->param.scalar != MAC_NO_ACK ) )//inserted ANAND:
    {
      telec_start_time =  timer_current_time_get();
      
      if(s->factory_mod_st.trx_pkt_param.pkt_mode_param.random_packet_len)
      { 
        max_rand_pkt_size = (s->factory_mod_st.trx_pkt_param.pkt_mode_param.fcslength)?2029:2031;
        while(!(sent_pld_len = (rand() % max_rand_pkt_size)));                                                                      
        s->factory_mod_st.trx_pkt_param.pkt_mode_param.pkt_len = sent_pld_len;
      }                                                                        
      //tsm_alarm(s,NULL);//ANAND send the next packet:
      time_stamp[0]= timer_current_time_get();
      tmr_start_relative( &(s->factory_mod_st.delay_next_pkt)); 
    }
    else
    {
#ifdef UTELTEST
      utu_timestamp(UTUL_PKT_TEST_DONE, s->tx_packet_counter );
#endif
      /*all the packets have been txed, so switch to idle state*/
      sm_transit((sm_t *)s, (sm_state_t)&node_factory_mode_state);
    }
    break;
  case TSM_TRIGGER_STOP_TX:
    tmr_stop( &(s->factory_mod_st.ipdelay) );
    tmr_stop( &(((fan_nwk_manager_sm_t*)s)->factory_mod_st.delay_next_pkt));    
    sm_transit((sm_t *)s, (sm_state_t)&node_factory_mode_state);
    break;
  default:
    break;
  }
  return NULL;
}

/*==================================================================================*/
/*==================================================================================*/
/*==================================================================================*/

sm_result_t node_factory_mode_state(fan_nwk_manager_sm_t *s, const sm_event_t *e)
{
  uint32_t val = 0;
  //uhf_tmr_create();
  
  //set_datapkt_type();
  switch ((fan_nwk_manager_sm_trigger_t) e->trigger)
  {
  case TRIGGER_ENTRY:
    s->state_ind = NODE_FACTORY_MODE_STATE;
    val = 1;
    PLME_set_request(phyFSKScramblePSDU,1,&val);
    break;
    
  case TRIGGER_START_NODE:  
    break;      
    
  case TSM_TRIGGER_PACKETS:
    if ( !( s->factory_mod_st.tx_packet_counter ) )
    {
      /*check if the packet counter is set to zero, this indicates that the event
      has been triggered as a result of user issuing the start transmission
      command */
      
      sm_transit((sm_t *)s, (sm_state_t)&fsm_sending_packets); 
    }
    break;
    
  case TSM_TRIGGER_START_RECEIVE:
    s->factory_mod_st.trx_pkt_param.display_rx_data = (e->param.scalar & 0x01);                                      
    s->one_byte_value = ((e->param.scalar & 0x02)?0x01:0x00);
    
    MLME_SET_Request
      (
       macPromiscuousMode,
       0,
       1,
       &(s->one_byte_value)
         );        
    
    /*triggered outside the telec sm from inter packet delay timer expiry*/
    sm_transit((sm_t *)s, (sm_state_t)&fsm_receive_mode);
    break;
    
  case TSM_TRIGGER_STREAM:
    if( s->factory_mod_st.trx_pkt_param.strm_mode_param == MOD_TYPE_UNMODULATED_CARRIER )
    {
      sm_transit((sm_t *)s, (sm_state_t)&tsm_sending_unmod_stream);
    }
    else
    {
      sm_transit((sm_t *)s, (sm_state_t)&tsm_sending_mod_stream);
    }
    break;
  default:
    break;          
  }
  return NULL;
}

/*==================================================================================*/
/*==================================================================================*/
/*==================================================================================*/

void process_set_pkt_tx( uint8_t *buf, uint16_t length )
{
	uint8_t status = CMD_SUCCESS;
	packet_mode_t* p_pkt_mode_param = &(fan_nwk_manager_app.factory_mod_st.trx_pkt_param.pkt_mode_param);
        uint32_t val = 0;
        
        uint32_t app_pkt_count = get_ulong_BE(buf);
        buf += 4 ;
        uint16_t app_pld_len = get_ushort_BE(buf);
        buf+=2;
        memcpy(fan_nwk_manager_app.factory_mod_st.gmsduu,buf,app_pld_len);
        buf += app_pld_len ;
       
        uint8_t fcs_len = *buf++;
//        uint8_t is_bcast = true;        
        
        uint16_t max_rand_pkt_size = 1;
        
//        for(val=0;val<8;val++)
//        {
//          if( buf[1+val] != 0xFF )
//          {
//            is_bcast = false;
//            break;
//          }
//        }
           
	//if( (app_pld_len + ((fcs_len)?4:2)+ 13+ ((is_bcast)?SHORT_ADDRESS_LENGTH:IEEE_ADDRESS_LENGTH)) > aMaxPHYPacketSize )
        
        if( app_pld_len  > aMaxPHYPacketSize )
	{
		status = 1;//OUT_OF_RANGE;
                //so that even if this status is ognored and tester tries to 
                //initiate the transmission assuming that he had done pkt 
                //configurations correctly, then this will stop the node from 
                //transmistting the packets with the previous correct 
                //configurations
                fan_nwk_manager_app.factory_mod_st.trx_pkt_param.tx_mode = 0;
	}
	else
	{
		fan_nwk_manager_app.factory_mod_st.trx_pkt_param.tx_mode = CONTINUOUS_PACKET_TX_MODE;
		p_pkt_mode_param->pkt_cnt = app_pkt_count;//get_ulong_BE(buf);
		p_pkt_mode_param->pkt_len = app_pld_len;
                p_pkt_mode_param->fcslength  = fcs_len;
		p_pkt_mode_param->dw_enabled = *buf++;//*(buf+7);
		val = p_pkt_mode_param->dw_enabled;                
                
                mem_rev_cpy
                ( 
                    p_pkt_mode_param->dest_addr.address.ieee_address,
                    buf, 
                    IEEE_ADDRESS_LENGTH 
                );
                buf+=8;
                p_pkt_mode_param->random_packet_len = *buf++;
                if(p_pkt_mode_param->random_packet_len)
                { 
                  max_rand_pkt_size = (p_pkt_mode_param->fcslength)?2029:2031;
                  while(!(app_pld_len = (rand() % max_rand_pkt_size))); 
                  p_pkt_mode_param->pkt_len = app_pld_len;
                }                
		PLME_set_request(phyFSKScramblePSDU,1,&val);
	}
	
	//host_pkt[1] = status;
	//hif_send_msg_up( host_pkt,1,0x03 );
	send_hif_conf_cb(SET_TX_PKT_CONFIG_CONF,status);	
}

/*==================================================================================*/
/*==================================================================================*/
/*==================================================================================*/

/*start trigger for packet transmission*/
void process_start_tx( uint8_t *buf, uint16_t length )
{
  

        trx_params_t* p_pkt_mode_param = &(fan_nwk_manager_app.factory_mod_st.trx_pkt_param);
         
	sm_event_t event;
        uint8_t status = CMD_SUCCESS;
	//uint8_t host_pkt[2] = { TELEC_START_PACKET_TX_CONF, CMD_SUCCESS };
	uint8_t tx_mode = p_pkt_mode_param->tx_mode;

	
	if ( (( tx_mode != CONTINUOUS_PACKET_TX_MODE ) && 
	( tx_mode != CONTINUOUS_STREAM_TX_MODE  )) ||
	( fan_nwk_manager_app.state_ind > NODE_FACTORY_MODE_STATE) )
	{
		//host_pkt[1] = CMD_INVALID;
                  status = CMD_INVALID;
		
        }

	
	if ( fan_nwk_manager_app.state_ind > NODE_FACTORY_MODE_STATE )
	{
		//host_pkt[1] = NODE_BUSY;
                status = NODE_BUSY;
	}
	else if(  tx_mode != CONTINUOUS_PACKET_TX_MODE  )
	{
		//host_pkt[1] = PKT_TX_CONFIG_NOT_SET;
                status = PKT_TX_CONFIG_NOT_SET;
	}

	/*send the confirmation to the host*/
	//hif_send_msg_up( host_pkt,1,0x03 );
        send_hif_conf_cb(START_PACKET_TX_REQ_CONF,status);

	if( status == CMD_SUCCESS )
	{
		/* packet tx mode */
		fan_nwk_manager_app.factory_mod_st.tx_packet_counter = 0;
		//telecsm.rf_app_data_req_exec_time = sw_current_time_get( gpTmr_mod_ins );
               // telecsm.rf_app_data_req_exec_time = (sw_current_time_get( gpTmr_mod_ins ) - telecsm.rf_app_data_req_exec_time );
		
		/*trigger transmission of continuos packets encoded using PN9*/
		event.trigger = (sm_trigger_t) TSM_TRIGGER_PACKETS;
		
		SM_DISPATCH( (sm_t *) &fan_nwk_manager_app, &event );
	}	
}

/*==================================================================================*/
/*==================================================================================*/
/*==================================================================================*/

static sm_result_t fsm_receive_mode(fan_nwk_manager_sm_t *s, const sm_event_t *e)
{
	uint16_t packet_size;
	
	uint8_t* p_host_pkt =  s->factory_mod_st.gmsduu;
        
	switch ((fan_nwk_manager_sm_trigger_t) e->trigger)
	{
	case TRIGGER_ENTRY:
		s->state_ind = NODE_RECEIVEING_PACKET;
                PLME_Set_TRX_State(PHY_RX_ON);
//                TRX_Set_RX_Buffer();
		s->factory_mod_st.rx_packet_num = 0;
		break;
        /* Note:  */
     case TSM_TRIGGER_RX_COMPLETE:
       {
                uint8_t phost[2];
                uint8_t *fcs_length = &phost[0];
                memcpy( fcs_length,&(((rx_mac_frame_info_t*)(e->param.vector))->FCS_Length),2);
                uint16_t uart_tx_size = 0;
		packet_size = ((rx_mac_frame_info_t*)(e->param.vector))->msduLength;
                memset (p_host_pkt, 0x00, 2033);
                
//                if( fan_nwk_manager_app.factory_mod_st.trx_pkt_param.promiscous_mode )
//                {
//                    packet_size += (get_ushort(fcs_length) & 0x7FFF);                  
//                }

		p_host_pkt[0] = RX_FRAME_IND_DISPLAY_OFF;
		p_host_pkt[1] = CMD_SUCCESS;		
		p_host_pkt[2] = (uint8_t)((((fan_nwk_manager_sm_t*)s)->factory_mod_st.rx_packet_num)>>8);
		p_host_pkt[3] = (uint8_t)(((fan_nwk_manager_sm_t*)s)->factory_mod_st.rx_packet_num);
		
                p_host_pkt[4] = (int8_t)(((rx_mac_frame_info_t*)(e->param.vector))->rssival);
               
                //mem_rev_cpy(&p_host_pkt[4],(uint8_t*)&(((rx_mac_frame_info_t*)(e->param.vector))->rssival),4);
                 
                p_host_pkt[5] = (uint8_t)(packet_size>>8);
		p_host_pkt[6] = (uint8_t)packet_size;                                         
               
                uart_tx_size = 7;
                
          
          if( fan_nwk_manager_app.factory_mod_st.trx_pkt_param.display_rx_data )
          {
               p_host_pkt[0] = RX_FRAME_IND;
               memcpy(&(p_host_pkt[uart_tx_size]),((rx_mac_frame_info_t*)(e->param.vector))->pMsdu,packet_size);
               uart_tx_size +=packet_size;
          }
                
              // p_host_pkt[packet_size+9+1] = 0x55;//Dummy com port 
               //hif_send_msg_up( p_host_pkt,(packet_size+10),0x03 );//+1 For Com port
         // }
//          else
//          {
////                mem_rev_cpy(&p_host_pkt[6],(uint8_t*)&(((rx_mac_frame_info_t*)(e->param.vector))->rssival),4);
//                p_host_pkt[10] = 0x55;//Dummy com port 
//                hif_send_msg_up( p_host_pkt,(8+1+1),0x03 );//+1 For Com port 
//          }
          
         
          p_host_pkt[uart_tx_size++] = DUMMY_COMPORT;//Dummy com port 
          hif_send_msg_up( p_host_pkt,(uart_tx_size-1),response_laye_ID,PROTOCOL_ID_FOR_APP);//-1 For p_host_pkt[0] = TELEC_RX_FRAME_IND;     
       }     
          break;
          
     case TSM_TRIGGER_STOP_RX:
     	/* Bring the RF in PHY on mode */
     
       
       s->one_byte_value = 0;
         
	 MLME_SET_Request
	 (
		macPromiscuousMode,
		0,
		1,
		&(s->one_byte_value)
	 );
         
        
        PLME_Set_TRX_State(PHY_DEV_ON);
        send_hif_conf_cb(STOP_PACKET_RX_REQ_CONF,0);
        
     	sm_transit((sm_t *)&fan_nwk_manager_app, (sm_state_t)&node_factory_mode_state );
     	break;
        
     case TRIGGER_SET_CONF:
    
	if( s->result == (factory_sm_result_t)MAC_SUCCESS )
	{
               

              if ( e->param.scalar == phyRSSIThreshold )
              {                                      
                    s->factory_mod_st.rssi_threshold = s->one_byte_value;                                      
              }
              else if( e->param.scalar == macLBTSamplingDuration )
              {
                  s->factory_mod_st.trx_pkt_param.pkt_mode_param.is_cca_on = s->one_byte_value; 
                   
              }
              else if ( e->param.scalar == phyCurrentSUNPageEntry )
              {                                     
                    //s->current_phy_mode = s->one_byte_value;                                                                           
              } 
              else if( e->param.scalar == macPromiscuousMode  )
              {
                s->factory_mod_st.trx_pkt_param.promiscous_mode = s->one_byte_value;
              }
	}
	

     default:
     	break;
	}

	return NULL;
}

/*==================================================================================*/
/*==================================================================================*/
/*==================================================================================*/

static sm_result_t tsm_sending_unmod_stream( fan_nwk_manager_sm_t *s, const sm_event_t *e )
{
	switch ((fan_nwk_manager_sm_trigger_t) e->trigger)
	{
	case TRIGGER_ENTRY:
		s->state_ind = NODE_CONTINIOUS_UNMOD_STREAM_STATE;

		/*mod scheme has been set as "carrier only". Switch on the transmitter now
		for the carrier transmission */
		//TRX_TX_On();
		TRX_Xmit_StreamModeOn( (RAIL_StreamMode_t ) fan_nwk_manager_app.factory_mod_st.trx_pkt_param.mod_source  );

		break;

	case TSM_TRIGGER_STOP_TX:
		/*bring the TRX in phy on state*/
		TRX_On();
                
                TRX_Stop_Tx_Steram();
                
                 fan_nwk_manager_app.factory_mod_st.trx_pkt_param.tx_mode = 0;
                 
		/*TODO: send the hif confirmation from here indicating that the tx has been
		stopped successfully */
		sm_transit((sm_t *)&fan_nwk_manager_app, (sm_state_t)&node_factory_mode_state );
		break;

	case TSM_TRIGGER_RX_COMPLETE:
     	break;

	default:
		break;
	}

	return NULL;

}

/*==================================================================================*/
/*==================================================================================*/
/*==================================================================================*/

static sm_result_t tsm_sending_mod_stream( fan_nwk_manager_sm_t *s, const sm_event_t *e )
{
	switch ((fan_nwk_manager_sm_trigger_t) e->trigger)
	{
	case TRIGGER_ENTRY:
		s->state_ind = NODE_CONTINIOUS_MOD_STREAM_STATE;//fall through deliberately
      
		/*switch the tranmitter once before the continuous tx is triggered */
		//TRX_TX_On();
		//TRX_Xmit_On();
                TRX_Xmit_StreamModeOn( (RAIL_StreamMode_t ) fan_nwk_manager_app.factory_mod_st.trx_pkt_param.mod_source  );
                
		break;

	case TSM_TRIGGER_TX_COMPLETE:
		/* No action taken as it is a continuous modualated transmisison */
		break;

	case TSM_TRIGGER_STOP_TX:

		TRX_On();
                
                TRX_Stop_Tx_Steram();
                
                 fan_nwk_manager_app.factory_mod_st.trx_pkt_param.tx_mode = 0;
                 
		sm_transit((sm_t *)&fan_nwk_manager_app, (sm_state_t)&node_factory_mode_state );

		break;

	case TSM_TRIGGER_RX_COMPLETE:
     	break;

	default:
		break;
	}

	return NULL;

}

/*==================================================================================*/
/*==================================================================================*/
/*==================================================================================*/

void process_stop_tx( uint8_t *buf, uint16_t length )
{
        uint8_t status = 0x00;
	//uint8_t host_pkt[2] = { TELEC_STOP_PACKET_TX_CONF, CMD_SUCCESS };
	sm_event_t event;
	if( fan_nwk_manager_app.state_ind > NODE_FACTORY_MODE_STATE )
	{
          event.trigger = (sm_trigger_t) TSM_TRIGGER_STOP_TX;	
          SM_DISPATCH( (sm_t *) &fan_nwk_manager_app, &event );
	}
	else
	{
		// host_pkt[2]	= PKT_TX_NOT_IN_PROGRESS;
                  status = 0x01;
	}
	
	/*first send the confirmation to the host*/
	//hif_send_msg_up( host_pkt,1,0x03 );
          send_hif_conf_cb(STOP_PACKET_TX_REQ_CONF,status);	
}


/*==================================================================================*/
/*==================================================================================*/
/*==================================================================================*/

/*start trigger for packet reception*/
void process_start_rx( uint8_t *buf, uint16_t length )
{
        uint8_t status = CMD_SUCCESS;
	sm_event_t event;
        uint8_t data_promiscous_mode = 0; //bit 0 to display data=1 and bit 1 promiscous mode =1
	/* valid combinations
	display data=1 and promiscous mode =1
        display data=0 and promiscous mode =1
        */
	if( *buf )
        {
          data_promiscous_mode = 0x01;
        }
        
        if( *(buf+1) )
        {
          data_promiscous_mode |= 0x02;
        }
        
     
	if(  ( fan_nwk_manager_app.state_ind == NODE_FACTORY_MODE_STATE ) )
	{
     		/*send the start rx event only if the telec is in idle state*/
		event.trigger = (sm_trigger_t) TSM_TRIGGER_START_RECEIVE;
                event.param.scalar = data_promiscous_mode;
    		SM_DISPATCH( (sm_t *) &fan_nwk_manager_app, &event );             
	}
	else
	{
		/* either the device role is not set as rx or the telec is already is in a 
		mode other than IDLE(for eg, in Tx mode) */
		//host_pkt[1] = CMD_INVALID;
                status = 0x01;//CMD_INVALID;
		
	}
	/*first send the confirmation to the host*/
       // hif_send_msg_up( host_pkt,1,0x03 );
        send_hif_conf_cb(START_PACKET_RX_CONF,status);
        /*Suneet :: 26:10 chack fcs lenth it's is short or long Here 0x00 is shortfcs lenth */
//        if(fan_nwk_manager_app.factory_mod_st.trx_pkt_param.pkt_mode_param.fcslength == 0x00)
//        {
//           TRX_Config_CRC_for_RX_pkt(); 
//        }	
}


/*==================================================================================*/
/*==================================================================================*/
/*==================================================================================*/

void process_set_stream_tx()
{
  fan_nwk_manager_app.factory_mod_st.trx_pkt_param.tx_mode = CONTINUOUS_STREAM_TX_MODE;
  fan_nwk_manager_app.factory_mod_st.trx_pkt_param.strm_mode_param = 0;  //FSK2
  fan_nwk_manager_app.factory_mod_st.trx_pkt_param.mod_type  = 0;//FSK2;
  //fan_nwk_manager_app.factory_mod_st.trx_pkt_param.mod_source = MOD_SRC_PN9;
}

/*==================================================================================*/
/*==================================================================================*/
/*==================================================================================*/

void  process_start_continuous_tx( uint8_t *buf, uint16_t length )
{
	sm_event_t event;
        uint8_t status = 0x00;
	uint8_t host_pkt[2] = { START_START_CONTINUOUS_TX_REQ_CONF, CMD_SUCCESS };
        process_set_stream_tx();
	uint8_t tx_mode = fan_nwk_manager_app.factory_mod_st.trx_pkt_param.tx_mode;
	
	if(( fan_nwk_manager_app.state_ind > NODE_FACTORY_MODE_STATE ) || ( tx_mode != CONTINUOUS_STREAM_TX_MODE ))
	{
		//host_pkt[1] = NODE_BUSY ;
                status = 0x01;
	}
//	else if( tx_mode != CONTINUOUS_STREAM_TX_MODE )
//	{
//		//host_pkt[1] = MOD_TYPE_NOT_CONFIGURED;
//                status = 0x01;
//	}
	
        host_pkt[1] = status;
        
	/*send the confirmation to the host*/
	send_hif_conf_cb(START_START_CONTINUOUS_TX_REQ_CONF,status);
			
	if( host_pkt[1] == CMD_SUCCESS )
	{
          
          fan_nwk_manager_app.factory_mod_st.trx_pkt_param.mod_source = *buf ;
		//TRX_On();
//		TRX_Mod_Scheme( (uint8_t)( fan_nwk_manager_app.factory_mod_st.trx_pkt_param.mod_type ) );
//		TRX_Set_Mod_Source((mod_src_t )fan_nwk_manager_app.factory_mod_st.trx_pkt_param.mod_source);
		//TRX_Set_PA_Level( fan_nwk_manager_app.node_basic_cfg.pa_level );
                
		/*trigger transmission of continuous packets encoded using PN9*/
		event.trigger = (sm_trigger_t) TSM_TRIGGER_STREAM;	

		SM_DISPATCH( (sm_t *) &fan_nwk_manager_app, &event );
	}
}

/*==================================================================================*/
/*==================================================================================*/
/*==================================================================================*/

void  process_stop_continuous_tx( uint8_t *buf, uint16_t length )
{
        uint8_t status = CMD_SUCCESS;
	sm_event_t event;
	if( fan_nwk_manager_app.state_ind > NODE_FACTORY_MODE_STATE )
	{
		event.trigger = (sm_trigger_t) TSM_TRIGGER_STOP_TX;		
               SM_DISPATCH( (sm_t *) &fan_nwk_manager_app, &event );
	}
	else
	{
		//host_pkt[1]	= CONTINUOUS_TX_NOT_IN_PROGRESS;
                status = 0x01;
	}
	
	/*first send the confirmation to the host*/
	//hif_send_msg_up( host_pkt,1,0x03 );
        send_hif_conf_cb(STOP_CONTINUOUS_TX_CONF,status);
}

/*==================================================================================*/
/*==================================================================================*/
/*==================================================================================*/

/*stop trigger for packet reception*/
void process_stop_rx( uint8_t *buf, uint16_t length )
{
	sm_event_t event = { (sm_trigger_t) TSM_TRIGGER_STOP_RX, { 0 } };
	SM_DISPATCH( (sm_t *) &fan_nwk_manager_app, &event );
}

/*==================================================================================*/
/*==================================================================================*/
/*==================================================================================*/
                
void process_start_continuous_rx( uint8_t *buf, uint16_t length )
{
        uint8_t status  = CMD_SUCCESS;
	sm_event_t event;
	
		
	if(  ( fan_nwk_manager_app.state_ind == NODE_FACTORY_MODE_STATE ) )
	{
     		/*send the start rx event only if the telec is in idle state*/
		event.trigger = (sm_trigger_t) TSM_TRIGGER_START_RECEIVE;	
    		SM_DISPATCH( (sm_t *) &fan_nwk_manager_app, &event );
	}
	else
	{
                  status = 0x01;//NODE_BUSY;
	}
	
	/*first send the confirmation to the host*/
        send_hif_conf_cb(START_CONTINUOUS_RX_CONF,status);	
}

/*==================================================================================*/
/*==================================================================================*/
/*==================================================================================*/

void process_stop_continuous_rx( uint8_t *buf, uint16_t length )
{
        uint8_t status = CMD_SUCCESS; 
	sm_event_t event = { (sm_trigger_t) TSM_TRIGGER_STOP_RX, { 0 } };
	if( ( fan_nwk_manager_app.state_ind == NODE_RECEIVEING_PACKET ) )
	{
		SM_DISPATCH( (sm_t *) &fan_nwk_manager_app, &event );
	}
	else
	{
                   status = 0x01;//CONTINUOUS_RX_NOT_IN_PROGRESS;
	}
         send_hif_conf_cb(STOP_CONTINUOUS_RX_CONF,status);    
}

/*==================================================================================*/
/*==================================================================================*/
/*==================================================================================*/

void send_next_pkt(void *s,void* tmr)  // added with delay on timer expiry of 45 msec - shubham
{
   tmr_stop( &(((fan_nwk_manager_sm_t*)s)->factory_mod_st.delay_next_pkt));      
   (*((fan_nwk_manager_sm_t *)s)->factory_mod_st.fn_alarm)(s);
} 

/*==================================================================================*/
/*==================================================================================*/
/*==================================================================================*/

void update_total_sending_dur_for_hr( void *s, void* tmr )
{

	  total_sending_in_hr[total_sending_in_hr_index] = mac_pib.LBTPrevHrTotalSendingDurUS;

	  //total_packets_sent[total_sending_in_hr_index] = (telecsm.tx_packet_counter)-((total_sending_in_hr_index)?total_packets_sent[total_sending_in_hr_index-1]:0);
           total_packets_sent[total_sending_in_hr_index] = total_packets_sent_in_an_hr;
           total_packets_sent_in_an_hr = 0;
	  if( ++total_sending_in_hr_index == 6 )
	  {
		  total_sending_in_hr_index = 0;
	  }
          
          tmr_stop( &(((fan_nwk_manager_sm_t*)s)->factory_mod_st.send_dur_tmr));
          tmr_start_relative(&(((fan_nwk_manager_sm_t*)s)->factory_mod_st.send_dur_tmr));
}

/*==================================================================================*/
/*==================================================================================*/
/*==================================================================================*/

void tsm_alarm(void *s, void* tmr )
{
    (*((fan_nwk_manager_sm_t *)s)->factory_mod_st.fn_alarm)(s);
}

/*==================================================================================*/
/*==================================================================================*/
/*==================================================================================*/

void process_get_rx_details( uint8_t* buf, uint16_t length )
{
      trx_details_t rx_details;
      uint8_t host_pkt[9] = { SEND_RX_COUNT_DETAILS,CMD_SUCCESS };
      uint8_t* p_host_pkt = &host_pkt[0];
      get_trx_details(&rx_details,1);
      p_host_pkt[3] = (uint8_t)rx_details.pkt_rx_irq;
      p_host_pkt[2] = (uint8_t)(rx_details.pkt_rx_irq>>8);
      p_host_pkt[5] = (uint8_t) rx_details.crc16_failures ;
      p_host_pkt[4] = (uint8_t)(rx_details.crc16_failures>>8);
      p_host_pkt[7] = (uint8_t) rx_details.crc32_failures ;
      p_host_pkt[6] = (uint8_t)(rx_details.crc32_failures>>8) ;
      p_host_pkt[8] =DUMMY_COMPORT ;//Dummy Comport
      hif_send_msg_up( host_pkt,9,response_laye_ID,PROTOCOL_ID_FOR_APP);
}

/*==================================================================================*/
/*==================================================================================*/


void process_factory_mode_ch_scanning_req( uint8_t* buf, uint16_t length )
{
  
   TRX_RAIL_do_EnergyDetection_Scanning ();
  send_hif_conf_cb (CMD_FACTROY_MODE_CHANNEL_SCAN_CONF, 0);
}
/*==================================================================================*/

void process_test_phy_enc( uint8_t* buf, uint16_t length )
{
  uint8_t pd_data_req_content[45]; 
  uint32_t TxChan = 0;
  uint16_t piblen = 0, i;
  phy_tx_t* p_pd_data = (phy_tx_t*)pd_data_req_content;
  PLME_get_request( phyCurrentChannel, &piblen, &TxChan );
  
  p_pd_data->TxChannel = (uint16_t)TxChan;		
  p_pd_data->PPDUCoding = 0;				
  p_pd_data->FCSLength = buf[0];				
  p_pd_data->ModeSwitch = 0;				
  p_pd_data->NewModeSUNPage = 0x0;			
  p_pd_data->ModeSwitchParameterEntry = 0x0;
  p_pd_data->psduLength = (p_pd_data->FCSLength)?16:18;	
  
  for(i=0; i<p_pd_data->psduLength; i++)
  {
    p_pd_data->psdu[i] = i;
  }
  
  PLME_Set_TRX_State( PHY_DEV_ON );
  
  PLME_Set_TRX_State( PHY_TX_ON );
  
  PD_Data_Request( NULL, p_pd_data );
}

/*==================================================================================*/
/*==================================================================================*/
/*==================================================================================*/

int32_t get_rssi_max_threshold (void);
int32_t get_rssi_min_threshold (void);

void process_set_rssi_threshold (uint8_t* buf, uint16_t length)
{
  int8_t rssi_threshold = *buf;
  
  if((rssi_threshold >= get_rssi_min_threshold()) && (rssi_threshold <= get_rssi_max_threshold()) && (length == 1))
  {
    fan_nwk_manager_app.factory_mod_st.rssi_threshold = rssi_threshold;
    send_hif_conf_cb (SET_RSSI_THRESHOLD_CONF, 0);
  }
  else
  {
    send_hif_conf_cb (SET_RSSI_THRESHOLD_CONF, 1);
  }
}

/*==================================================================================*/
/*==================================================================================*/
/*==================================================================================*/

//void process_set_adjust_freq (uint8_t* buf, uint16_t length)
//{
//  uint8_t cap_bank = *buf;
//  
//  if ((cap_bank > 0x01) && (cap_bank <= 0x7F))
//  {
//    fan_nwk_manager_app.factory_mod_st.xtal_adjust = cap_bank;
//    set_cap_bank_val (cap_bank);
//    send_hif_conf_cb (SET_ADJUST_FREQ_CONF, 0);
//  }
//  else
//    send_hif_conf_cb (SET_ADJUST_FREQ_CONF, 1);
//}

/*==================================================================================*/
/*==================================================================================*/
/*==================================================================================*/

//void process_set_serial_baudrate (uint8_t* buf, uint16_t length)
//{
//  uint32_t baudrate;
//  
//  if (*buf == 0)
//    baudrate = 115200;
//  if (*buf == 1)
//    baudrate = 230400;
//  if (*buf == 2)
//    baudrate = 460800;
//  
//  if (fan_nwk_manager_app.node_basic_cfg.fan_device_type == 0)
//  {
//    send_hif_conf_cb (SET_SERIAL_BAUDRATE_CONF, 2);     /*CMD not supported*/
//    return;
//  }
//  
//  if ((baudrate != 115200) && (baudrate != 230400) && (baudrate != 460800))
//  {
//    send_hif_conf_cb (SET_SERIAL_BAUDRATE_CONF, 1);     /*Invalid Baudrate*/
//    return;
//  }
//  
//  fan_nwk_manager_app.factory_mod_st.serial_baudrate = baudrate;
//  
//  nvm_store_node_basic_info();
//  UART_init ();
//}

/*==================================================================================*/
/*==================================================================================*/
/*==================================================================================*/

void process_get_config_info_req (void)
{
  uint8_t hif_buf[36];
  uint8_t indx = 0;
  uint32_t symbol_rate = 0;
  uint8_t modulation_index = 0;
  
  hif_buf[indx++] = GET_CONFIG_INFO_RESP;
  hif_buf[indx++] = fan_nwk_manager_app.node_basic_cfg.operational_mode;
  mem_rev_cpy (&hif_buf[indx], (uint8_t *)&fan_nwk_manager_app.factory_mod_st.serial_baudrate, 4);
  indx += 4;
  memcpy (&hif_buf[indx], fan_nwk_manager_app.node_basic_cfg.self_ieee_addr, 8);
  indx += 8;
  hif_buf[indx++] = fan_nwk_manager_app.factory_mod_st.rssi_threshold;
  hif_buf[indx++] = fan_nwk_manager_app.factory_mod_st.xtal_adjust;
  hif_buf[indx++] = 0;  /* Always 2FSK */
  
  if (fan_nwk_manager_app.node_basic_cfg.sybbol_rate == 20)
    symbol_rate = 50000;
  if (fan_nwk_manager_app.node_basic_cfg.sybbol_rate == 10)
    symbol_rate = 100000;
  if (fan_nwk_manager_app.node_basic_cfg.sybbol_rate == 7)
    symbol_rate = 150000;
  if (fan_nwk_manager_app.node_basic_cfg.sybbol_rate == 5)
    symbol_rate = 200000;
  if (fan_nwk_manager_app.node_basic_cfg.sybbol_rate == 3)
    symbol_rate = 300000;
  mem_rev_cpy (&hif_buf[indx], (uint8_t *)&symbol_rate, sizeof(symbol_rate));
  indx += sizeof(symbol_rate);
  
  if (fan_nwk_manager_app.node_basic_cfg.modulation_index == (float)0.5)
      modulation_index = 0;
  if (fan_nwk_manager_app.node_basic_cfg.modulation_index == (float)1.0)
      modulation_index = 1;
  hif_buf[indx++] = modulation_index;
  
  hif_buf[indx++] = fan_nwk_manager_app.node_basic_cfg.pa_level;
  hif_buf[indx++] = fan_nwk_manager_app.node_basic_cfg.us_ie.un_channel_plan.ch_reg_op.reg_domain;
  hif_buf[indx++] = fan_nwk_manager_app.node_basic_cfg.us_ie.un_channel_plan.ch_reg_op.op_class;
  mem_rev_cpy (&hif_buf[indx], (uint8_t *)&fan_nwk_manager_app.node_basic_cfg.selected_channel, 2);
  indx += 2;
  
  hif_send_msg_up (hif_buf, indx, response_laye_ID,PROTOCOL_ID_FOR_APP);
}
 
/*==================================================================================*/                 
/*==================================================================================*/
/*==================================================================================*/


static void perform_bcast_data_tx( void* s )
{
  uint8_t pd_data_req_content[2100];
  phy_tx_t *phy_pkt = (phy_tx_t*)pd_data_req_content;
  uint32_t TxChannel = 0;
  uint16_t len = 0;
  PLME_get_request( phyCurrentChannel, &len, &TxChannel );
  uint8_t *bp = NULL;
  total_packets_sent_in_an_hr++;
  ((fan_nwk_manager_sm_t*)s)->factory_mod_st.tx_packet_counter++;
 
  phy_pkt->link = NULL; 
  phy_pkt->TxChannel = TxChannel;
  phy_pkt->PPDUCoding = 0;
  phy_pkt->FCSLength  = fan_nwk_manager_app.factory_mod_st.trx_pkt_param.pkt_mode_param.fcslength;
  phy_pkt->ModeSwitch = 0;
  phy_pkt->NewModeSUNPage = 0;
  phy_pkt->ModeSwitchParameterEntry = 0;
  phy_pkt->psduLength =fan_nwk_manager_app.factory_mod_st.trx_pkt_param.pkt_mode_param.pkt_len;
  bp = phy_pkt->psdu;
  memcpy( bp, fan_nwk_manager_app.factory_mod_st.gmsduu, phy_pkt->psduLength );
  PLME_Set_TRX_State( PHY_TX_ON );
  PD_Data_Request( NULL,phy_pkt );  
}

/*==================================================================================*/
/*==================================================================================*/
/*==================================================================================*/

uint8_t get_cuurunt_active_operation_mode()
{
  return fan_nwk_manager_app.node_basic_cfg.operational_mode;
}

/*==================================================================================*/

//void process_set_phy(uint8_t *buf, uint16_t length)
//{
//  int8_t status = 0xFF;
//  uint8_t PhyModeID = (*buf);
//  status = set_RAIL_phyMode( PhyModeID );
//  
//  if (status != 0x00 )
//    status = 1;
//  else
//  {
//      fan_nwk_manager_app.node_basic_cfg.phy_mode = phyModeMapArr[PhyModeID]; 
//      uint32_t current_page = get_sun_page_value();;
//      PLME_set_request(phyCurrentSUNPageEntry,4,&current_page);
//  }
//  
//  send_hif_conf_cb(CONF_SET_PHY,status);        
//}

/*==================================================================================*/

void App_factory_mode_channel_set()
{
  uint32_t channel = get_sun_channel();
  PLME_set_request(phyCurrentChannel,2,&channel);
}

/*==================================================================================*/

void process_set_facort_mode_PA_level_api(uint8_t *buf, uint16_t length)
{
  
   uint8_t status = 0xFF;
  uint8_t PALevelVal = *buf++;
  
  status = TRX_RAIL_setPower(PALevelVal);
  
  send_hif_conf_cb(SET_FACTORY_MODE_PA_LEVEL_API_CONF,status);  
}

/*==================================================================================*/
/*==================================================================================*/
