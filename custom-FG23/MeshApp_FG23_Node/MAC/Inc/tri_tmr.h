/** \file tri_tmr.h
 *******************************************************************************
 ** \brief Implements a software timer functionality
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

#ifndef TRI_TMR_H_
#define TRI_TMR_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"
#include "sw_timer.h"   
#define TRICKLE_TIMER_TX_SUPPRESS             0
#define TRICKLE_TIMER_TX_OK                   1


#define TRICKLE_TIMER_INFINITE_REDUNDANCY         0x00
#define TRICKLE_TIMER_ERROR                   0
#define TRICKLE_TIMER_SUCCESS                 1  
  
#define TRICKLE_TIMER_IS_STOPPED              0  
/*---------------------------------------------------------------------------*/ 
typedef uint64_t clock_tick;  


typedef struct trickle_timer {
  clock_tick i_min;     /**< Imin: Clock ticks */
  clock_tick i_cur;     /**< I: Current interval in clock_ticks */
  clock_tick i_start;   /**< Start of this interval (absolute clock_time) */
  clock_tick i_max_abs; /**< Maximum interval size in clock ticks (and not in*/
  uint8_t i_max;          /**< Imax: Max number of doublings */
  uint8_t k;              /**< k: Redundancy Constant */
  uint8_t c;              /**< c: Consistency Counter */
}trickle_tmr;


void trickle_timer_config_pas_send();
void trickle_timer_config_pa_send();
void trickle_timer_config_pc_send();
void trickle_timer_config_pcs_send();
/*---------------------------------------------------------------------------*/ 
void trickle_timer_consistency_pas();
void trickle_timer_consistency_pa();
void trickle_timer_consistency_pc();
void trickle_timer_consistency_pcs();
void trickle_timer_send_pa_pkt_immediately();
void trickle_timer_send_pc_pkt_immediately();
/*---------------------------------------------------------------------------*/
void trickle_timer_inconsistency_pa(void);
void trickle_timer_inconsistency_pc(void);
/*---------------------------------------------------------------------------*/
void trickle_timer_pas_stop();
/*---------------------------------------------------------------------------*/
#define trickle_timer_reset_event(tt) trickle_timer_inconsistency(tt)

#define TRICKLE_TIMER_CLOCK_MAX ((clock_tick)~0)

extern bool tmr_start_relative( sw_tmr_t *pTmr_ins );    

enum
{
   PAN_ADVERT,
   PAN_ADVERT_SOL,
   PAN_CONF, 
   PAN_CONFIG_SOL 
};


enum{
  ASYNC_START,
  ASYNC_STOP
} ;

typedef struct fan_pkt_tmr_tag
{
  uint8_t operation;
  uint8_t frame_type;
  uint8_t length_of_channel_list;
  uint8_t ready_to_change_state;
} fan_pkt_tmr_t; 

void calcChanTable (uint16_t n, uint16_t m, int32_t * chanTable) ;
void create_channel_slots_list();


//void send_pa_in_channel_list(uint8_t* channel_list, uint8_t length);
//void send_pc_in_channel_list(uint8_t* channel_list, uint8_t length);
//void send_pas_in_channel_list(uint8_t* channel_list, uint8_t length);
//void send_pcs_in_channel_list(uint8_t* channel_list, uint8_t length);
void trickle_timer_pcs_stop(void);
uint8_t recv_pcs_attement_count();
#ifdef __cplusplus
}
#endif
#endif /* _HW_TMR_H_ */