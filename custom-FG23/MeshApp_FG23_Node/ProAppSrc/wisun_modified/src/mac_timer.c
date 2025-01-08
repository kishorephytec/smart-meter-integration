/** \file mac_timer.c
 *******************************************************************************
 ** \brief Provides APIs for MAC beacons
 **
 ** \cond STD_FILE_HEADER
 **
 ** COPYRIGHT(c) 2019-24 Procubed innovation pvt ltd. 
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
#include "StackMACConf.h"
#include "common.h"
#include "l3_configuration.h"
#include "l3_timer_utility.h" 
/*
** =============================================================================
** Private Macro definitions
** =============================================================================
*/
	
/* None */

/*
** =============================================================================
** Private Structures, Unions & enums Type Definitions
** =============================================================================
**/
l3_ctimer_t unicast_dwell_interval_timer;
l3_ctimer_t broadcat_dwell_interval_timer;
l3_ctimer_t broadcast_shedule_interval;
l3_ctimer_t brodcast_schdeule_start;
l3_ctimer_t brodcast_error_offset_timer;
l3_ctimer_t async_pa_timer;
l3_ctimer_t async_pc_timer;
l3_ctimer_t async_pas_timer;
l3_ctimer_t async_pcs_timer;
#if(FAN_EDFE_FEATURE_ENABLED == 1)
l3_ctimer_t edfe_rx_timer;
l3_ctimer_t edfe_tx_timer;
#endif
l3_ctimer_t resp_delay;
l3_ctimer_t random_dealy;
l3_ctimer_t ping_delay_timer;
#if(FAN_EAPOL_FEATURE_ENABLED == 1)
l3_ctimer_t eapol_timeout_timer;
#endif


/*
** ============================================================================
** Private Variable Definitions
** ============================================================================
*/

/* None */

/*
** ============================================================================
** Private Function Prototypes
** ============================================================================
*/
void trigger_pa_frame_on_timer (void *a);
void trigger_pc_frame_on_timer (void *a);
void trigger_pas_frame_on_timer (void *a);
void trigger_pcs_frame_on_timer (void *a);
#if(FAN_EDFE_FEATURE_ENABLED == 1)
void check_edfe_reciver_timer(void *a);
void check_edfe_trnssmit_timer(void *a);
void stop_edfe_reciver_timer();
void start_edfe_receiver_responce_timer(uint64_t responce_wait);
#endif

void out_data_pkt_after_responce_delay(void *a);
void responce_delay(uint8_t responce_delay_time,void *pkt_ptr);
void create_random_delay(uint8_t rand_value);
void send_icmpv6_after_delay(void *data);
#if(FAN_EAPOL_FEATURE_ENABLED == 1)
void eapol_timeout_cleanup (void *a);
#endif

/*
** ============================================================================
** Private Function Definitions
** ============================================================================
*/

/* None */

/*
** ============================================================================
** External Function Declarations
** ============================================================================
*/
extern void start_brodacst_schdeule();
extern void send_packt_after_some_randomness();
extern  void unicast_channel_change_timer(void *a);
extern  void broadcast_channel_change_timer(void *a);
extern void broadcast_idle_time(void *a);
extern void trigger_explicit_broadcast_packet (void *data);
/*
** ============================================================================
** External Variable Declarations
** ============================================================================
*/

/* None */

/*
** =============================================================================
** Public Variables Definitions
** =============================================================================
**/

/* None */

/*
** ============================================================================
** Public Function Definitions
** ============================================================================
*/

void start_unicast_dwell_timer(uint64_t unicast_dwell_interval)
{
  l3_ctimer_set (&unicast_dwell_interval_timer, unicast_dwell_interval, unicast_channel_change_timer, NULL);
}
void stop_unicast_dwell_timer (void)
{
  l3_ctimer_stop(&unicast_dwell_interval_timer);
}
void start_broadcast_dwellinterval_timer(uint64_t broadcast_dwell_interval)
{
  l3_ctimer_set (&broadcat_dwell_interval_timer, broadcast_dwell_interval,broadcast_idle_time , NULL);
}
void stop_broadcast_dwellinterval_timer (void)
{
  l3_ctimer_stop(&broadcat_dwell_interval_timer);
  l3_ctimer_stop(&broadcast_shedule_interval);
  l3_ctimer_stop(&brodcast_schdeule_start);
}
void start_brocast_shedule_interval_timer(uint64_t broadcast_interval)
{
   l3_ctimer_set (&broadcast_shedule_interval, broadcast_interval, broadcast_channel_change_timer, NULL);
}
void start_timer_to_start_broadcast_schdeule(uint64_t expire_int)
{
  l3_ctimer_set (&brodcast_schdeule_start, expire_int, start_brodacst_schdeule, NULL);
}
void stop_broadcast_ctimer(void)
{
  l3_ctimer_stop(&broadcast_shedule_interval);
}
void procubed_broadcast_error_offset_timer_start (uint64_t time_val)
{
  l3_ctimer_set (&brodcast_error_offset_timer, time_val, trigger_explicit_broadcast_packet, NULL);
}
void start_async_pa_timer (void)
{
  l3_ctimer_set (&async_pa_timer, 20, trigger_pa_frame_on_timer, NULL);
}
void start_async_pc_timer (void)
{
  l3_ctimer_set (&async_pc_timer, 20, trigger_pc_frame_on_timer, NULL);
}
void start_async_pas_timer (void)
{
  l3_ctimer_set (&async_pas_timer, 20, trigger_pas_frame_on_timer, NULL);
}
void start_async_pcs_timer (void)
{
  l3_ctimer_set (&async_pcs_timer, 20, trigger_pcs_frame_on_timer, NULL);
}

#if(FAN_EDFE_FEATURE_ENABLED == 1)

void start_edfe_receiver_responce_timer(uint64_t responce_wait)
{
  l3_ctimer_set (&edfe_rx_timer, responce_wait, check_edfe_reciver_timer, NULL);
}


void stop_edfe_reciver_timer()
{
  l3_ctimer_stop(&edfe_rx_timer);
}

void start_edfe_transmit_responce_timer(uint64_t responce_wait)
{
  l3_ctimer_set (&edfe_tx_timer, responce_wait, check_edfe_trnssmit_timer, NULL);
}

void stop_edfe_transmit_timer()
{
  l3_ctimer_stop(&edfe_tx_timer);
}

#endif

void responce_delay(uint8_t responce_delay_time,void *pkt_ptr)
{
  l3_ctimer_set (&resp_delay, responce_delay_time, out_data_pkt_after_responce_delay, pkt_ptr);
}

void create_random_delay(uint8_t rand_value)
{
  l3_ctimer_set (&random_dealy, rand_value, send_packt_after_some_randomness, NULL);
}

void start_timer_to_send_ping (uint64_t timeval, void *data)
{
  l3_ctimer_set (&ping_delay_timer, timeval, send_icmpv6_after_delay, data);
}