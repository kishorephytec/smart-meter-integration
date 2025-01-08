#include "StackMACConf.h"
#include "tri_tmr.h"
#include "common.h"
#include "queue_latest.h"
#include "list_latest.h"
#include "hw_tmr.h"
#include "sw_timer.h"
#include "phy.h"


struct trickle_timer tt_pas;

extern uint32_t ChangeEndianness(uint32_t value);
extern void broadcast_shedule_start(void);
void start_trickle_timer(uint8_t *buf, uint16_t length);
uint8_t join_state=0;

uint8_t operation_typeMapArr[4]={0x00,0x01};
uint8_t frame_typeMapArr[4]={0x0A,0x09,0x0C,0x0B};

void trickle_tx( uint8_t  suppress);
uint8_t operation_type=0;;
uint8_t frame_type=0;


//extern  void WS_ASYNC_FRAME_request(
//                             uint8_t operation, 
//                             uint8_t frametype,                             
//                             uint8_t* channellist
//                           );

#define NEW_TOKEN_INTERVAL  10 * CLOCK_SECOND
#define NEW_TOKEN_PROB      2


void start_trickle_timer(uint8_t *buf, uint16_t length)
{
                     uint32_t channel_set=0x00;
                      uint32_t temp_operation_type = 0xFFFFFFFF;
                      temp_operation_type =  ChangeEndianness ((*(uint32_t *)buf));                
                      operation_type = operation_typeMapArr[temp_operation_type]; buf += 4;
                     
                      uint32_t temp_frame_type = 0xFFFFFFFF;
                      temp_frame_type =  ChangeEndianness ((*(uint32_t *)buf));                
                      frame_type = frame_typeMapArr[temp_frame_type]; buf += 4;
                      uint8_t channel_number=0xff;
                      //First byte is channel_number
                      channel_number=*buf++;
                      if(channel_number!=0x00)
                      {
                          /*Run time channel set */
                           channel_set=*buf++;
                           PLME_set_request(phyCurrentChannel,2,&channel_set);
                           *buf++;
                      }
                      else
                      {
                          *buf++;
                      }
                      
                      
              if(frame_type==PAN_ADVERT)
              {
                    /*When root pwer up send both pan advert and pan configration pkt*/
                      trickle_timer_config_pa_send();
//                      trickle_timer_config_pc_send();
                     
              }
               if(frame_type==PAN_ADVERT_SOL)
              {     
                    trickle_timer_config_pas_send();
 
              }
              else if(frame_type==PAN_CONFIG_SOL)
              {
                      //trickle_timer_config_pcs_send();
                      
              }
              else if(frame_type==PAN_CONF)
              {
                         /*set broadcast schedule root */
                      //trickle_timer_config_pc_send();
                       broadcast_shedule_start();
              }
}