/*
 * Copyright (c) 2005, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 */
#ifndef CONTIKI_NET_H_
#define CONTIKI_NET_H_

   
#include "l3_configuration.h"
#include "l3_timer_utility.h"
#include "l3_process_interface.h"
#include "l3_list_utility.h"
#include "l3_memb_utility.h"
#include "l3_random_utility.h"


/*------------------------------*/
#include "nbr-table.h"
#include "link-stats.h"
#include "packetbuf.h"
/*------------------------------*/
#include "uip-split.h"
#include "uip-packetqueue.h"

/*------------------------------*/
#include "tcpip.h"
#include "uip.h"
#include "uip-udp-packet.h"
#include "simple-udp.h"
#include "net-debug.h" 
#include "uip-mcast6-engines.h"
#include "uip-mcast6-route.h"
#include "uip-mcast6.h"
#include "uip-icmp6.h"

#if NETSTACK_CONF_WITH_IPV6
#include "uip-ds6.h"
#include "uip-nd6.h"
#endif
#include "uip-ds6-route.h"

#if UIP_ND6_SEND_NA
#include "uip-ds6-nbr.h"
#endif

#include "lib/random.h"
/*------------------------------*/
#include "mpl.h"
#include "smrf.h"
#include "esmrf.h"
#include "roll-tm.h"
//#include "rime.h"
/*------------------------------*/
#include "rpl-conf.h"
#if UIP_CONF_IPV6_RPL
#include "rpl.h"
#include "rpl-private.h"
#endif
#include "rpl-ns.h"
#include "rpl-dag-root.h"
/*------------------------------*/
#include "sicslowpan.h"
#include "netstack.h"
#include "contiki_mac.h"
#include "net-debug.h"
#include "queuebuf.h"

uip_ipaddr_t *
set_global_address(void);
//#include "net/netstack.h"
#endif /* CONTIKI_NET_H_ */
