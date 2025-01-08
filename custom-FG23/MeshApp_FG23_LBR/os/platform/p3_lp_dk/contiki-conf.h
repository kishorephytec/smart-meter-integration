#ifndef __CONTIKI_CONF_H__
#define __CONTIKI_CONF_H__


#include "cc.h"
#include <string.h>

#include "common.h"
#include "fan_config_param.h"

#define CCIF
#define CLIF
#define NETSTACK_CONF_WITH_IPV6              1
/* Include Project Specific conf */
#ifdef PROJECT_CONF_H
#include "project-conf.h"
#endif /* PROJECT_CONF_H */

/*
 * To help syntax checkers with parsing of sdcc keywords. It basically defines
 * empty macros for keywords such as __sfr etc. Does nothing when compiling
 */
//#include <lint.h>

/* Time type. */
//typedef unsigned int clock_time_t;
//typedef unsigned long long clock_time_t;

#ifndef UIPSTATS
#define UIPSTATS
typedef unsigned short uip_stats_t;
#endif

/* Defines tick counts for a second. */
#define CLOCK_CONF_SECOND		1000    //16//256


/* Energest Module */
#ifndef ENERGEST_CONF_ON
#define ENERGEST_CONF_ON      0
#endif
#define CC_CONF_VA_ARGS                0
#define AUTOSTART_ENABLE 1

/* Verbose Startup? Turning this off saves plenty of bytes of CODE in HOME */
#define STARTUP_CONF_VERBOSE  0

/* More CODE space savings by turning off process names */
#define PROCESS_CONF_NO_PROCESS_NAMES 1

/*
 * USARTs:
 *   SmartRF RS232 -> USART0 / Alternative 1 (UART)
 *   SmartRF LCD   -> USART1 / Alternative 2 (SPI)
 *   Default: Both Disabled
 */
#define UART_ON_USART     0

#define UART1_CONF_ENABLE 0

#ifndef UART0_CONF_ENABLE
#define UART0_CONF_ENABLE  1
#endif
#ifndef UART0_CONF_WITH_INPUT
#define UART0_CONF_WITH_INPUT 0
#endif

#ifndef UART0_CONF_HIGH_SPEED
#define UART0_CONF_HIGH_SPEED 0
#endif

/* Are we a SLIP bridge? */
#if SLIP_ARCH_CONF_ENABLE
/* Make sure the UART is enabled, with interrupts */
#undef UART0_CONF_ENABLE
#undef UART0_CONF_WITH_INPUT
#define UART0_CONF_ENABLE  1
#define UART0_CONF_WITH_INPUT 1
#define UIP_FALLBACK_INTERFACE slip_interface
#endif

/* Output all captured frames over the UART in hexdump format */
#ifndef CC2530_RF_CONF_HEXDUMP
#define CC2530_RF_CONF_HEXDUMP 0
#endif

#if CC2530_RF_CONF_HEXDUMP
/* We need UART1 output */
#undef UART_ZERO_CONF_ENABLE
#define UART_ZERO_CONF_ENABLE   1
#endif

/* Code Shortcuts */
/*
 * When set, the RF driver is no longer a contiki process and the RX ISR is
 * disabled. Instead of polling the radio process when data arrives, we
 * periodically check for data by directly invoking the driver from main()

 * When set, this directive also configures the following bypasses:
 *   - process_post_synch() in tcpip_input() (we call packet_input())
 *   - process_post_synch() in tcpip_uipcall (we call the relevant pthread)
 *   - mac_call_sent_callback() is replaced with sent() in various places
 *
 * These are good things to do, we reduce stack usage, RAM size and code size
 */
#define NETSTACK_CONF_SHORTCUTS   0

/*
 * Sensors
 * It is harmless to #define XYZ 1
 * even if the sensor is not present on our device
 */
#ifndef BUTTON_SENSOR_CONF_ON
#define BUTTON_SENSOR_CONF_ON   1  /* Buttons */
#endif
/* ADC - Turning this off will disable everything below */
#ifndef ADC_SENSOR_CONF_ON
#define ADC_SENSOR_CONF_ON      1
#endif
#define TEMP_SENSOR_CONF_ON     1  /* Temperature */
#define VDD_SENSOR_CONF_ON      1  /* Supply Voltage */
#define BATTERY_SENSOR_CONF_ON  0  /* Battery */

/* Low Power Modes - We only support PM0/Idle and PM1 */
#ifndef LPM_CONF_MODE
#define LPM_CONF_MODE         0 /* 0: no LPM, 1: MCU IDLE, 2: Drop to PM1 */
#endif

/* DMA Configuration */
#ifndef DMA_CONF_ON
#define DMA_CONF_ON 0
#endif

/* Viztool on by default for IPv6 builds */

#if NETSTACK_CONF_WITH_IPV6
#ifndef VIZTOOL_CONF_ON
#define VIZTOOL_CONF_ON        1
#endif /* VIZTOOL_CONF_ON */
#endif /* NETSTACK_CONF_WITH_IPV6 */

/* Network Stack */
#ifndef NETSTACK_CONF_NETWORK
#if NETSTACK_CONF_WITH_IPV6
#define NETSTACK_CONF_NETWORK sicslowpan_driver
#else
#define NETSTACK_CONF_NETWORK rime_driver
#endif /* NETSTACK_CONF_WITH_IPV6 */
#endif /* NETSTACK_CONF_NETWORK */

#ifndef NETSTACK_CONF_MAC
#define NETSTACK_CONF_MAC     pro_mac_driver
#endif

#ifndef NETSTACK_CONF_RDC
#define NETSTACK_CONF_RDC     nullrdc_driver//santosh, multicast
#define NULLRDC_802154_AUTOACK 1
#define NULLRDC_802154_AUTOACK_HW 1
#endif

#ifndef NETSTACK_CONF_RDC_CHANNEL_CHECK_RATE
#define NETSTACK_CONF_RDC_CHANNEL_CHECK_RATE 8
#endif

#ifndef NETSTACK_CONF_FRAMER
#define NETSTACK_CONF_FRAMER  framer_802154
#endif

#define NETSTACK_CONF_RADIO   cc2530_rf_driver

/* RF Config */
#ifndef IEEE802154_CONF_PANID
#define IEEE802154_CONF_PANID 0x5449 /* TI */
#endif

#ifndef CC2530_RF_CONF_CHANNEL
#define CC2530_RF_CONF_CHANNEL    25
#endif /* CC2530_RF_CONF_CHANNEL */

#ifndef CC2530_RF_CONF_AUTOACK
#define CC2530_RF_CONF_AUTOACK 1
#endif /* CC2530_CONF_AUTOACK */

#if NETSTACK_CONF_WITH_IPV6
/* Addresses, Sizes and Interfaces */
/* 8-byte addresses here, 2 otherwise */
#define LINKADDR_CONF_SIZE                   8
#define UIP_CONF_LL_802154                   1
#define UIP_CONF_LLH_LEN                     0
#define UIP_CONF_NETIF_MAX_ADDRESSES         3

/* TCP, UDP, ICMP */
#define UIP_CONF_TCP                         0
#define UIP_CONF_UDP                         1
#define UIP_CONF_UDP_CHECKSUMS               1
#define UIP_CONF_ICMP6                       1

/* ND and Routing */
#ifndef UIP_CONF_ROUTER
#define UIP_CONF_ROUTER                      1
#endif

//#define UIP_CONF_ROUTER                      0

#define UIP_CONF_IPV6_RPL                    1
//#define UIP_CONF_ND6_SEND_RA                 0 // Raka  : it is set 0 in GG-nodeRouter and in leafNode workspace
#define UIP_CONF_ND6_SEND_RA                 0
#define UIP_CONF_ADDRESS_RESOLUTION          1
#define UIP_CONF_IP_FORWARD                  0
#define RPL_CONF_STATS                       0
#define RPL_CONF_MAX_DAG_ENTRIES             1
#ifndef RPL_CONF_OF
#define RPL_CONF_OF rpl_mrhof
#define RPL_CONF_DAO_ACK                     1
#endif

//#define UIP_CONF_ND6_REACHABLE_TIME     600000
#define UIP_CONF_ND6_RETRANS_TIMER       10000

#ifndef NBR_TABLE_CONF_MAX_NEIGHBORS
#define NBR_TABLE_CONF_MAX_NEIGHBORS            APP_CFG_MAX_DEV_SUPPORT
#endif

#ifndef UIP_CONF_MAX_ROUTES
#define UIP_CONF_MAX_ROUTES                     APP_CFG_MAX_DEV_SUPPORT
#endif

/* uIP */
#ifndef UIP_CONF_BUFFER_SIZE
//#define UIP_CONF_BUFFER_SIZE               250

//#if MTU_SIZE
//#define UIP_CONF_BUFFER_SIZE (UIP_LLH_LEN + MTU_SIZE)
//#else /* MTU_SIZE */
#define UIP_CONF_BUFFER_SIZE (UIP_MAX_CONF_BUFFER_SIZE) 

//#endif /* MTU_SIZE */

#endif
#define UIP_CONF_IPV6_QUEUE_PKT              1
#define UIP_CONF_IPV6_CHECKS                 1
#define UIP_CONF_IPV6_REASSEMBLY             0
#define UIP_UDP_SEND_UNREACH_NOPORT          1

/* 6lowpan */
#define SICSLOWPAN_CONF_COMPRESSION          SICSLOWPAN_COMPRESSION_HC06
#ifndef SICSLOWPAN_CONF_FRAG
#define SICSLOWPAN_CONF_FRAG                 0 /* About 2KB of CODE if 1 */
#endif
#define SICSLOWPAN_CONF_MAXAGE               6

/* Define our IPv6 prefixes/contexts here */
#define SICSLOWPAN_CONF_MAX_ADDR_CONTEXTS    1
#define SICSLOWPAN_CONF_ADDR_CONTEXT_0 { \
  addr_contexts[0].prefix[0] = 0xaa; \
  addr_contexts[0].prefix[1] = 0xaa; \
}

#define MAC_CONF_CHANNEL_CHECK_RATE          8

#ifndef QUEUEBUF_CONF_NUM
#define QUEUEBUF_CONF_NUM                    26  //Suneet :: Fragment Free Buf 
#endif
//#define PACKETBUF_CONF_SIZE                  1500

#else /* NETSTACK_CONF_WITH_IPV6 */
/* Network setup for non-IPv6 (rime). */
#define UIP_CONF_IP_FORWARD                  1
#define UIP_CONF_BUFFER_SIZE               108
#define RIME_CONF_NO_POLITE_ANNOUCEMENTS     0
#define QUEUEBUF_CONF_NUM                    8
#endif /* NETSTACK_CONF_WITH_IPV6 */

/* Prevent SDCC compile error when UIP_CONF_ROUTER == 0 */
#if !UIP_CONF_ROUTER
#define UIP_CONF_DS6_AADDR_NBU               0
#endif

#endif /* CONTIKI_CONF_H_ */
