/*
 * Copyright (c) 2008, Swedish Institute of Computer Science.
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
 */

/**
 * \file
 *         6lowpan implementation (RFC4944 and draft-ietf-6lowpan-hc-06)
 *
 * \author Adam Dunkels <adam@sics.se>
 * \author Nicolas Tsiftes <nvt@sics.se>
 * \author Niclas Finne <nfi@sics.se>
 * \author Mathilde Durvy <mdurvy@cisco.com>
 * \author Julien Abeille <jabeille@cisco.com>
 * \author Joakim Eriksson <joakime@sics.se>
 * \author Joel Hoglund <joel@sics.se>
 */

/**
 * \addtogroup sicslowpan
 * @{
 */

/**
 * FOR HC-06 COMPLIANCE TODO:
 * -Add compression options to UDP, currently only supports
 *  both ports compressed or both ports elided
 *
 * -Verify TC/FL compression works
 *
 * -Add stateless multicast option
 */
#include "Stack6LoWPANConf.h"
#include <stdio.h>
#include "stdlib.h"
#include "l3_configuration.h"
#include "l3_timer_utility.h"
#include "contiki-net.h"


#define DEBUG DEBUG_NONE
#include "uip-debug.h"
#if DEBUG
/* PRINTFI and PRINTFO are defined for input and output to debug one without changing the timing of the other */
uint8_t p;
#include <stdio.h>
#define PRINTFI(...) PRINTF(__VA_ARGS__)
#define PRINTFO(...) PRINTF(__VA_ARGS__)
#else
#define PRINTFI(...)
#define PRINTFO(...)
#endif /* DEBUG == 1*/

#if UIP_LOGGING
#include <stdio.h>
void uip_log(char *msg);
#define UIP_LOG(m) uip_log(m)
#else
#define UIP_LOG(m)
#endif /* UIP_LOGGING == 1 */

#ifndef SICSLOWPAN_COMPRESSION
#ifdef SICSLOWPAN_CONF_COMPRESSION
#define SICSLOWPAN_COMPRESSION SICSLOWPAN_CONF_COMPRESSION
#else
#define SICSLOWPAN_COMPRESSION SICSLOWPAN_COMPRESSION_IPV6
#endif /* SICSLOWPAN_CONF_COMPRESSION */
#endif /* SICSLOWPAN_COMPRESSION */

#define GET16(ptr,index) (((uint16_t)((ptr)[index] << 8)) | ((ptr)[(index) + 1]))
#define SET16(ptr,index,value) do {     \
  (ptr)[index] = ((value) >> 8) & 0xff; \
  (ptr)[index + 1] = (value) & 0xff;    \
} while(0)


#define DEFAULT_FRAG_SIZE   500 
#define DEFAULT_FRAG_CHUNK_SIZE 400
uint16_t max_frag_size = DEFAULT_FRAG_SIZE; //Suneet :; for tbc mtu size handle 
uint16_t frag_chunk_send = DEFAULT_FRAG_CHUNK_SIZE;

/** \name Pointers in the packetbuf buffer
 *  @{
 */
#define PACKETBUF_FRAG_PTR           (packetbuf_ptr)
#define PACKETBUF_FRAG_DISPATCH_SIZE 0   /* 16 bit */
#define PACKETBUF_FRAG_TAG           2   /* 16 bit */
#define PACKETBUF_FRAG_OFFSET        4   /* 8 bit */

/* define the buffer as a byte array */
#define PACKETBUF_IPHC_BUF              ((uint8_t *)(packetbuf_ptr + packetbuf_hdr_len))
#define PACKETBUF_PAYLOAD_END           ((uint8_t *)(packetbuf_ptr + max_frag_size))

#define PACKETBUF_HC1_PTR            (packetbuf_ptr + packetbuf_hdr_len)
#define PACKETBUF_HC1_DISPATCH       0 /* 8 bit */
#define PACKETBUF_HC1_ENCODING       1 /* 8 bit */
#define PACKETBUF_HC1_TTL            2 /* 8 bit */

#define PACKETBUF_HC1_HC_UDP_PTR           (packetbuf_ptr + packetbuf_hdr_len)
#define PACKETBUF_HC1_HC_UDP_DISPATCH      0 /* 8 bit */
#define PACKETBUF_HC1_HC_UDP_HC1_ENCODING  1 /* 8 bit */
#define PACKETBUF_HC1_HC_UDP_UDP_ENCODING  2 /* 8 bit */
#define PACKETBUF_HC1_HC_UDP_TTL           3 /* 8 bit */
#define PACKETBUF_HC1_HC_UDP_PORTS         4 /* 8 bit */
#define PACKETBUF_HC1_HC_UDP_CHKSUM        5 /* 16 bit */

/** \name Pointers in the sicslowpan and uip buffer
 *  @{
 */

/* NOTE: In the multiple-reassembly context there is only room for the header / first fragment */
#define SICSLOWPAN_IP_BUF(buf)   ((struct uip_ip_hdr *)buf)
#define SICSLOWPAN_UDP_BUF(buf)  ((struct uip_udp_hdr *)&buf[UIP_IPH_LEN])
#define SICSLOWPAN_IPPAYLOAD_BUF(buf) (&buf[UIP_IPH_LEN])

#define UIP_IP_BUF          ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])
#define UIP_UDP_BUF          ((struct uip_udp_hdr *)&uip_buf[UIP_LLIPH_LEN])
#define UIP_TCP_BUF          ((struct uip_tcp_hdr *)&uip_buf[UIP_LLIPH_LEN])
#define UIP_ICMP_BUF          ((struct uip_icmp_hdr *)&uip_buf[UIP_LLIPH_LEN])
#define UIP_IPPAYLOAD_BUF(pos) (&uip_buf[UIP_LLIPH_LEN + pos])  
#define UIP_IPv6_HDR_COMP(pos)      ((struct uip_ip_hdr *)&uip_buf[pos])
/** @} */


/** \brief Maximum available size for frame headers,
           link layer security-related overhead, as well as
           6LoWPAN payload. */

//max psdu size as per this profile is 255( MAC hdr + mac pld + 2byte FCS )
#define FCS_SIZE                                2// bytes
#define MAC_HDR_SIZE_WO_SEC_BCAST               15   
#define MAC_HDR_SIZE_WITH_SEC_BCAST             15//plus max 6   
#define MAC_HDR_SIZE_WO_SEC_UCAST               37//21//FCF=2,SN=1,src+dst=16,utt=7,bt=9,LT=2  
#define MAC_HDR_SIZE_WITH_SEC_UCAST             21//plus max 6   
#define SEC_HDR_AND_MIC_BYTE                    10// 6 Byte Security Header + 4 Byte MIC  // Raka...09-Apr-2015
//#define MAC_MAX_PAYLOAD                         DEFAULT_FRAG_SIZE//2047//1050//no support for fragmentation

/** \brief Some MAC layers need a minimum payload, which is configurable
    through the SICSLOWPAN_CONF_COMPRESSION_THRESHOLD option. */
#ifdef SICSLOWPAN_CONF_COMPRESSION_THRESHOLD
#define COMPRESSION_THRESHOLD SICSLOWPAN_CONF_COMPRESSION_THRESHOLD
#else
#define COMPRESSION_THRESHOLD 0
#endif

/** \brief Fixed size of a frame header. This value is
 * used in case framer returns an error or if SICSLOWPAN_USE_FIXED_HDRLEN
 * is defined.
 */
#ifndef SICSLOWPAN_FIXED_HDRLEN
#define SICSLOWPAN_FIXED_HDRLEN 21
/* set this to zero if not compressing EXT_HDR - for backwards compatibility */
#ifdef SICSLOWPAN_CONF_COMPRESS_EXT_HDR
#define COMPRESS_EXT_HDR SICSLOWPAN_CONF_COMPRESS_EXT_HDR
#else
/* Compressing on by default - turn off to be compatible with older versions */
#define COMPRESS_EXT_HDR 1
#endif
#if COMPRESS_EXT_HDR
#define IS_COMPRESSABLE_PROTO(x) (x == UIP_PROTO_UDP               \
                                  || x == UIP_PROTO_HBHO           \
                                  || x == UIP_PROTO_DESTO          \
                                  || x == UIP_PROTO_ROUTING        \
                                  || x == UIP_PROTO_FRAG           \
                                  || x == UIP_PROTO_IPv6)
#else
#define IS_COMPRESSABLE_PROTO(x) (x == UIP_PROTO_UDP)
#endif /* COMPRESS_EXT_HDR */

/** \name General variables
 *  @{
 */

/**
 * A pointer to the packetbuf buffer.
 * We initialize it to the beginning of the packetbuf buffer, then
 * access different fields by updating the offset packetbuf_hdr_len.
 */
static uint8_t *packetbuf_ptr;

/**
 * packetbuf_hdr_len is the total length of (the processed) 6lowpan headers
 * (fragment headers, IPV6 or HC1, HC2, and HC1 and HC2 non compressed
 * fields).
 */
static uint8_t packetbuf_hdr_len;

/**
 * The length of the payload in the Packetbuf buffer.
 * The payload is what comes after the compressed or uncompressed
 * headers (can be the IP payload if the IP header only is compressed
 * or the UDP payload if the UDP header is also compressed)
 */
static int packetbuf_payload_len;

/**
 * uncomp_hdr_len is the length of the headers before compression (if HC2
 * is used this includes the UDP header in addition to the IP header).
 */
static uint8_t uncomp_hdr_len;

uint16_t exten_header_offset = 0;
/**
 * the result of the last transmitted fragment
 */
static int last_tx_status;
/** @} */


static int last_rssi;

#if(FAN_EDFE_FEATURE_ENABLED == 1)
extern void send_status_to_edfe_fragment(uint8_t status);
#endif
/* ----------------------------------------------------------------- */
/* Support for reassembling multiple packets                         */
/* ----------------------------------------------------------------- */

#if SICSLOWPAN_CONF_FRAG
//#ifdef SICSLOWPAN_CONF_FRAG             /*Umesh changed here*/
static uint16_t my_tag;

/** The total length of the IPv6 packet in the sicslowpan_buf. */

/* This needs to be defined in NBR / Nodes depending on available RAM   */
/*   and expected reassembly requirements                               */
#ifdef SICSLOWPAN_CONF_FRAGMENT_BUFFERS
#define SICSLOWPAN_FRAGMENT_BUFFERS SICSLOWPAN_CONF_FRAGMENT_BUFFERS
#else
#define SICSLOWPAN_FRAGMENT_BUFFERS 25
#endif

/* REASS_CONTEXTS corresponds to the number of simultaneous
 * reassemblies that can be made. NOTE: the first buffer for each
 * reassembly is stored in the context since it can be larger than the
 * rest of the fragments due to header compression.
 **/
#ifdef SICSLOWPAN_CONF_REASS_CONTEXTS
#define SICSLOWPAN_REASS_CONTEXTS SICSLOWPAN_CONF_REASS_CONTEXTS
#else
#define SICSLOWPAN_REASS_CONTEXTS 1
#endif

/* The size of each fragment (IP payload) for the 6lowpan fragmentation */
//#ifdef SICSLOWPAN_CONF_FRAGMENT_SIZE
////#define SICSLOWPAN_FRAGMENT_SIZE SICSLOWPAN_CONF_FRAGMENT_SIZE
//#else
////#define SICSLOWPAN_FRAGMENT_SIZE max_frag_size
//#endif


/* Assuming that the worst growth for uncompression is 38 bytes */
//#define SICSLOWPAN_FIRST_FRAGMENT_SIZE (SICSLOWPAN_FRAGMENT_SIZE + 38)

/* all information needed for reassembly */
struct sicslowpan_frag_info {
  /** When reassembling, the source address of the fragments being merged */
  linkaddr_t sender;
  /** The destination address of the fragments being merged */
  linkaddr_t receiver;
  /** When reassembling, the tag in the fragments being merged. */
  uint16_t tag;
  /** Total length of the fragmented packet */
  uint16_t len;
  /** Current length of reassembled fragments */
  uint16_t reassembled_len;
  /** Reassembly %process %timer. */
  l3_timer_t reass_timer;

  /** Fragment size of first fragment */
  uint16_t first_frag_len;
  /** First fragment - needs a larger buffer since the size is uncompressed size
   and we need to know total size to know when we have received last fragment. */
  uint8_t *first_frag;//[SICSLOWPAN_FIRST_FRAGMENT_SIZE];
};

static struct sicslowpan_frag_info frag_info[SICSLOWPAN_REASS_CONTEXTS];

struct sicslowpan_frag_buf {
  /* the index of the frag_info */
  uint8_t index;
  /* Fragment offset */
  uint8_t offset;
  /* Length of this fragment (if zero this buffer is not allocated) */
  uint16_t len;
 uint8_t *data; /*uint8_t data[SICSLOWPAN_FRAGMENT_SIZE];*/ //Suneet :: for allocated daynamnic memory 
};

static struct sicslowpan_frag_buf frag_buf[SICSLOWPAN_FRAGMENT_BUFFERS];

/*---------------------------------------------------------------------------*/
extern void app_bm_free( uint8_t *pMem  );
static int clear_fragments(uint8_t frag_info_index)
{
  int i, clear_count;
  clear_count = 0;
  frag_info[frag_info_index].len = 0;
  if(frag_info[frag_info_index].first_frag != NULL)
  {
    app_bm_free(frag_info[frag_info_index].first_frag); //Suneet :: free fragment memory after copy all data to main buf
    frag_info[frag_info_index].first_frag = NULL;
  }
  for(i = 0; i < SICSLOWPAN_FRAGMENT_BUFFERS; i++) {
    if(frag_buf[i].len > 0 && frag_buf[i].index == frag_info_index) {
      /* deallocate the buffer */
      app_bm_free(frag_buf[i].data); //Suneet :: free fragment memory after copy all data to main buf
      frag_buf[i].data = NULL;
      frag_buf[i].len = 0;
      clear_count++;
    }
  }
  return clear_count;
}
/*---------------------------------------------------------------------------*/
static int
timeout_fragments(int not_context)
{
  int i;
  int count = 0;
  for(i = 0; i < SICSLOWPAN_REASS_CONTEXTS; i++) {
    if(frag_info[i].len > 0 && i != not_context &&
       l3_timer_expired(&frag_info[i].reass_timer)) {
      /* This context can be freed */
      count += clear_fragments(i);
    }
  }
  return count;
}
/*---------------------------------------------------------------------------*/
extern void * app_bm_alloc(uint16_t length );

static int store_fragment(uint8_t index, uint8_t offset)
{
  int i;
  for(i = 0; i < SICSLOWPAN_FRAGMENT_BUFFERS; i++) {
    if(frag_buf[i].len == 0) {
      /* copy over the data from packetbuf into the fragment buffer and store offset and len */
      /*frag_buf[i].offset = offset << 3;*/ /* frag offset */
      frag_buf[i].offset = offset;
      frag_buf[i].len = packetbuf_datalen() - packetbuf_hdr_len;
      frag_buf[i].index = index;
      frag_buf[i].data =  (uint8_t *)app_bm_alloc(frag_buf[i].len);  //Suneet :: Allocated dynamic memory for fragment buffer
      memcpy(frag_buf[i].data, packetbuf_ptr + packetbuf_hdr_len,
             packetbuf_datalen() - packetbuf_hdr_len);

      PRINTF("Fragsize: %d\n", frag_buf[i].len);
      /* return the length of the stored fragment */
      return frag_buf[i].len;
    }
  }
  /* failed */
  return -1;
}
/*---------------------------------------------------------------------------*/
/* add a new fragment to the buffer */
static int8_t
add_fragment(uint16_t tag, uint16_t frag_size, uint8_t offset)
{
  int i;
  int len;
  int8_t found = -1;

  if(offset == 0) {
    /* This is a first fragment - check if we can add this */
    for(i = 0; i < SICSLOWPAN_REASS_CONTEXTS; i++) {
      /* clear all fragment info with expired timer to free all fragment buffers */
      if(frag_info[i].len > 0 && l3_timer_expired(&frag_info[i].reass_timer)) {
	clear_fragments(i);
      }

      /* We use len as indication on used or not used */
      if(found < 0 && frag_info[i].len == 0) {
        /* We remember the first free fragment info but must continue
           the loop to free any other expired fragment buffers. */
        found = i;
      }
    }

    if(found < 0) {
      PRINTF("*** Failed to store new fragment session - tag: %d\n", tag);
      return -1;
    }

    /* Found a free fragment info to store data in */
    frag_info[found].len = frag_size;
    frag_info[found].tag = tag;
    linkaddr_copy(&frag_info[found].sender,
                  packetbuf_addr(PACKETBUF_ADDR_SENDER));
    l3_timer_set(&frag_info[found].reass_timer, SICSLOWPAN_REASS_MAXAGE * CLOCK_SECOND / 16);
    /* first fragment can not be stored immediately but is moved into
       the buffer while uncompressing */
    return found;
  }

  /* This is a N-fragment - should find the info */
  for(i = 0; i < SICSLOWPAN_REASS_CONTEXTS; i++) {
    if(frag_info[i].tag == tag && frag_info[i].len > 0 &&
       linkaddr_cmp(&frag_info[i].sender, packetbuf_addr(PACKETBUF_ADDR_SENDER))) {
      /* Tag and Sender match - this must be the correct info to store in */
      found = i;
      break;
    }
  }

  if(found < 0) {
    /* no entry found for storing the new fragment */
    PRINTF("*** Failed to store N-fragment - could not find session - tag: %d offset: %d\n", tag, offset);
    return -1;
  }

  /* i is the index of the reassembly context */
  len = store_fragment(i, offset);
  if(len < 0 && timeout_fragments(i) > 0) {
    len = store_fragment(i, offset);
  }
  if(len > 0) {
    frag_info[i].reassembled_len += len;
    return i;
  } else {
    /* should we also clear all fragments since we failed to store
       this fragment? */
    PRINTF("*** Failed to store fragment - packet reassembly will fail tag:%d l\n", frag_info[i].tag);
    return -1;
  }
}
/*---------------------------------------------------------------------------*/
/* Copy all the fragments that are associated with a specific context
   into uip */
static void
copy_frags2uip(int context)
{
  int i;

  /* Copy from the fragment context info buffer first */
  memcpy((uint8_t *)UIP_IP_BUF, (uint8_t *)frag_info[context].first_frag,
	 frag_info[context].first_frag_len);
  for(i = 0; i < SICSLOWPAN_FRAGMENT_BUFFERS; i++) {
    /* And also copy all matching fragments */
    if(frag_buf[i].len > 0 && frag_buf[i].index == context) {
      memcpy((uint8_t *)UIP_IP_BUF + (uint16_t)(frag_buf[i].offset << 3),
	     (uint8_t *)frag_buf[i].data, frag_buf[i].len);
    }
  }
  /* deallocate all the fragments for this context */
  clear_fragments(context);
}
#endif /* SICSLOWPAN_CONF_FRAG */

static int last_rssi;

/*-------------------------------------------------------------------------*/
/* Rime Sniffer support for one single listener to enable powertrace of IP */
/*-------------------------------------------------------------------------*/
static struct rime_sniffer *callback = NULL;

void
rime_sniffer_add(struct rime_sniffer *s)
{
  callback = s;
}
/*---------------------------------------------------------------------------*/
void
rime_sniffer_remove(struct rime_sniffer *s)
{
  callback = NULL;
}
/*---------------------------------------------------------------------------*/
static void
set_packet_attrs(void)
{
  int c = 0;
  /* set protocol in NETWORK_ID */
  packetbuf_set_attr(PACKETBUF_ATTR_NETWORK_ID, UIP_IP_BUF->proto);

  /* assign values to the channel attribute (port or type + code) */
  if(UIP_IP_BUF->proto == UIP_PROTO_UDP) {
    c = UIP_UDP_BUF->srcport;
    if(UIP_UDP_BUF->destport < c) {
      c = UIP_UDP_BUF->destport;
    }
  } else if(UIP_IP_BUF->proto == UIP_PROTO_TCP) {
    c = UIP_TCP_BUF->srcport;
    if(UIP_TCP_BUF->destport < c) {
      c = UIP_TCP_BUF->destport;
    }
  } else if(UIP_IP_BUF->proto == UIP_PROTO_ICMP6) {
    c = UIP_ICMP_BUF->type << 8 | UIP_ICMP_BUF->icode;
  }

  packetbuf_set_attr(PACKETBUF_ATTR_CHANNEL, c);

/*   if(uip_ds6_is_my_addr(&UIP_IP_BUF->srcipaddr)) { */
/*     own = 1; */
/*   } */

}



#if SICSLOWPAN_COMPRESSION == SICSLOWPAN_COMPRESSION_HC06
/** \name HC06 specific variables
 *  @{
 */

/** Addresses contexts for IPHC. */
#if SICSLOWPAN_CONF_MAX_ADDR_CONTEXTS > 0
static struct sicslowpan_addr_context
addr_contexts[SICSLOWPAN_CONF_MAX_ADDR_CONTEXTS];
#endif

/** pointer to an address context. */
static struct sicslowpan_addr_context *context;

/** pointer to the byte where to write next inline field. */
static uint8_t *hc06_ptr;

/* Uncompression of linklocal */
/*   0 -> 16 bytes from packet  */
/*   1 -> 2 bytes from prefix - bunch of zeroes and 8 from packet */
/*   2 -> 2 bytes from prefix - 0000::00ff:fe00:XXXX from packet */
/*   3 -> 2 bytes from prefix - infer 8 bytes from lladdr */
/*   NOTE: => the uncompress function does change 0xf to 0x10 */
/*   NOTE: 0x00 => no-autoconfig => unspecified */
const uint8_t unc_llconf[] = {0x0f,0x28,0x22,0x20};

/* Uncompression of ctx-based */
/*   0 -> 0 bits from packet [unspecified / reserved] */
/*   1 -> 8 bytes from prefix - bunch of zeroes and 8 from packet */
/*   2 -> 8 bytes from prefix - 0000::00ff:fe00:XXXX + 2 from packet */
/*   3 -> 8 bytes from prefix - infer 8 bytes from lladdr */
const uint8_t unc_ctxconf[] = {0x00,0x88,0x82,0x80};

/* Uncompression of ctx-based */
/*   0 -> 0 bits from packet  */
/*   1 -> 2 bytes from prefix - bunch of zeroes 5 from packet */
/*   2 -> 2 bytes from prefix - zeroes + 3 from packet */
/*   3 -> 2 bytes from prefix - infer 1 bytes from lladdr */
const uint8_t unc_mxconf[] = {0x0f, 0x25, 0x23, 0x21};

/* Link local prefix */
const uint8_t llprefix[] = {0xfe, 0x80};

/* TTL uncompression values */
static const uint8_t ttl_values[] = {0, 1, 64, 255};

/*--------------------------------------------------------------------*/
/** \name IPHC related functions
 * @{                                                                 */
/*--------------------------------------------------------------------*/
/** \brief find the context corresponding to prefix ipaddr */
static struct sicslowpan_addr_context*
addr_context_lookup_by_prefix(uip_ipaddr_t *ipaddr)
{
/* Remove code to avoid warnings and save flash if no context is used */
#if SICSLOWPAN_CONF_MAX_ADDR_CONTEXTS > 0
  int i;
  for(i = 0; i < SICSLOWPAN_CONF_MAX_ADDR_CONTEXTS; i++) {
    if((addr_contexts[i].used == 1) &&
       uip_ipaddr_prefixcmp(&addr_contexts[i].prefix, ipaddr, 64)) {
      return &addr_contexts[i];
    }
  }
#endif /* SICSLOWPAN_CONF_MAX_ADDR_CONTEXTS > 0 */
  return NULL;
}
/*--------------------------------------------------------------------*/
/** \brief find the context with the given number */
static struct sicslowpan_addr_context*
addr_context_lookup_by_number(uint8_t number)
{
/* Remove code to avoid warnings and save flash if no context is used */
#if SICSLOWPAN_CONF_MAX_ADDR_CONTEXTS > 0
  int i;
  for(i = 0; i < SICSLOWPAN_CONF_MAX_ADDR_CONTEXTS; i++) {
    if((addr_contexts[i].used == 1) &&
       addr_contexts[i].number == number) {
      return &addr_contexts[i];
    }
  }
#endif /* SICSLOWPAN_CONF_MAX_ADDR_CONTEXTS > 0 */
  return NULL;
}
/*--------------------------------------------------------------------*/
static uint8_t
compress_addr_64(uint8_t bitpos, uip_ipaddr_t *ipaddr, uip_lladdr_t *lladdr)
{
  if(uip_is_addr_mac_addr_based(ipaddr, lladdr)) {
    return 3 << bitpos; /* 0-bits */
  } else if(sicslowpan_is_iid_16_bit_compressable(ipaddr)) {
    /* compress IID to 16 bits xxxx::0000:00ff:fe00:XXXX */
    memcpy(hc06_ptr, &ipaddr->u16[7], 2);
    hc06_ptr += 2;
    return 2 << bitpos; /* 16-bits */
  } else {
    /* do not compress IID => xxxx::IID */
    memcpy(hc06_ptr, &ipaddr->u16[4], 8);
    hc06_ptr += 8;
    return 1 << bitpos; /* 64-bits */
  }
}

/*-------------------------------------------------------------------- */
/* Uncompress addresses based on a prefix and a postfix with zeroes in
 * between. If the postfix is zero in length it will use the link address
 * to configure the IP address (autoconf style).
 * pref_post_count takes a byte where the first nibble specify prefix count
 * and the second postfix count (NOTE: 15/0xf => 16 bytes copy).
 */
static void
uncompress_addr(uip_ipaddr_t *ipaddr, uint8_t const prefix[],
                uint8_t pref_post_count, uip_lladdr_t *lladdr)
{
  uint8_t prefcount = pref_post_count >> 4;
  uint8_t postcount = pref_post_count & 0x0f;
  /* full nibble 15 => 16 */
  prefcount = prefcount == 15 ? 16 : prefcount;
  postcount = postcount == 15 ? 16 : postcount;

  PRINTF("Uncompressing %d + %d => ", prefcount, postcount);

  if(prefcount > 0) {
    memcpy(ipaddr, prefix, prefcount);
  }
  if(prefcount + postcount < 16) {
    memset(&ipaddr->u8[prefcount], 0, 16 - (prefcount + postcount));
  }
  if(postcount > 0) {
    memcpy(&ipaddr->u8[16 - postcount], hc06_ptr, postcount);
    if(postcount == 2 && prefcount < 11) {
      /* 16 bits uncompression => 0000:00ff:fe00:XXXX */
      ipaddr->u8[11] = 0xff;
      ipaddr->u8[12] = 0xfe;
    }
    hc06_ptr += postcount;
  } else if (prefcount > 0) {
    /* no IID based configuration if no prefix and no data => unspec */
    uip_ds6_set_addr_iid(ipaddr, lladdr);
  }

  PRINT6ADDR(ipaddr);
  PRINTF("\n");
}

/*--------------------------------------------------------------------*/
/**
 * \brief Compress IP/UDP header
 *
 * This function is called by the 6lowpan code to create a compressed
 * 6lowpan packet in the packetbuf buffer from a full IPv6 packet in the
 * uip_buf buffer.
 *
 *
 * IPHC (RFC 6282)\n
 * http://tools.ietf.org/html/
 *
 * \note We do not support ISA100_UDP header compression
 *
 * For LOWPAN_UDP compression, we either compress both ports or none.
 * General format with LOWPAN_UDP compression is
 * \verbatim
 *                      1                   2                   3
 *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |0|1|1|TF |N|HLI|C|S|SAM|M|D|DAM| SCI   | DCI   | comp. IPv6 hdr|
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * | compressed IPv6 fields .....                                  |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * | LOWPAN_UDP    | non compressed UDP fields ...                 |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * | L4 data ...                                                   |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * \endverbatim
 * \note The context number 00 is reserved for the link local prefix.
 * For unicast addresses, if we cannot compress the prefix, we neither
 * compress the IID.
 * \param link_destaddr L2 destination address, needed to compress IP
 * dest
 */
static void
compress_hdr_iphc(linkaddr_t *link_destaddr)
{
  uint8_t tmp, iphc0, iphc1, *next_hdr, *next_nhc,iphc2,iphc3;
  uint16_t total_hdr_len = 0;
  int ext_hdr_len;
  struct uip_udp_hdr *udp_buf = NULL;
  struct uip_ip_hdr *ipv6_comp_buf = NULL;
  struct uip_ext_hdr *ext_hdr = NULL;
#if DEBUG
  { uint16_t ndx;
    PRINTF("before compression (%d): ", UIP_IP_BUF->len[1]);
    for(ndx = 0; ndx < UIP_IP_BUF->len[1] + 40; ndx++) {
      uint8_t data = ((uint8_t *) (UIP_IP_BUF))[ndx];
      PRINTF("%02x", data);
    }
    PRINTF("\n");
  }
#endif
#define CHECK_BUFFER_SPACE(writelen) do { \
  if(hc06_ptr + (writelen) >= PACKETBUF_PAYLOAD_END) { \
    LOG_WARN("Not enough packetbuf space to compress header (%u bytes, %u left). Aborting.\n", \
                (unsigned)(writelen), (unsigned)(PACKETBUF_PAYLOAD_END - hc06_ptr)); \
    return 0; \
  } \
} while(0);

  hc06_ptr = PACKETBUF_IPHC_BUF + 2;

  /* Check if there is enough space for the compressed IPv6 header, in the
   * worst case (least compressed case). Extension headers and transport
   * layer will be checked when they are compressed. */


  iphc0 = SICSLOWPAN_DISPATCH_IPHC;
  iphc1 = 0;
  PACKETBUF_IPHC_BUF[2] = 0; /* might not be used - but needs to be cleared */

  /*
   * Address handling needs to be made first since it might
   * cause an extra byte with [ SCI | DCI ]
   *
   */


  /* check if dest context exists (for allocating third byte) */
  /* TODO: fix this so that it remembers the looked up values for
     avoiding two lookups - or set the lookup values immediately */
  if(addr_context_lookup_by_prefix(&UIP_IP_BUF->destipaddr) != NULL ||
     addr_context_lookup_by_prefix(&UIP_IP_BUF->srcipaddr) != NULL) {
    /* set context flag and increase hc06_ptr */
    PRINTF("IPHC: compressing dest or src ipaddr - setting CID\n");
    iphc1 |= SICSLOWPAN_IPHC_CID;
    hc06_ptr++;
  }

  /*
   * Traffic class, flow label
   * If flow label is 0, compress it. If traffic class is 0, compress it
   * We have to process both in the same time as the offset of traffic class
   * depends on the presence of version and flow label
   */

  /* IPHC format of tc is ECN | DSCP , original is DSCP | ECN */

  tmp = (UIP_IP_BUF->vtc << 4) | (UIP_IP_BUF->tcflow >> 4);
  tmp = ((tmp & 0x03) << 6) | (tmp >> 2);

  if(((UIP_IP_BUF->tcflow & 0x0F) == 0) &&
     (UIP_IP_BUF->flow == 0)) {
    /* flow label can be compressed */
    iphc0 |= SICSLOWPAN_IPHC_FL_C;
    if(((UIP_IP_BUF->vtc & 0x0F) == 0) &&
       ((UIP_IP_BUF->tcflow & 0xF0) == 0)) {
      /* compress (elide) all */
      iphc0 |= SICSLOWPAN_IPHC_TC_C;
    } else {
      /* compress only the flow label */
     *hc06_ptr = tmp;
      hc06_ptr += 1;
    }
  } else {
    /* Flow label cannot be compressed */
    if(((UIP_IP_BUF->vtc & 0x0F) == 0) &&
       ((UIP_IP_BUF->tcflow & 0xF0) == 0)) {
      /* compress only traffic class */
      iphc0 |= SICSLOWPAN_IPHC_TC_C;
      *hc06_ptr = (tmp & 0xc0) |
        (UIP_IP_BUF->tcflow & 0x0F);
      memcpy(hc06_ptr + 1, &UIP_IP_BUF->flow, 2);
      hc06_ptr += 3;
    } else {
      /* compress nothing */
      memcpy(hc06_ptr, &UIP_IP_BUF->vtc, 4);
      /* but replace the top byte with the new ECN | DSCP format*/
      *hc06_ptr = tmp;
      hc06_ptr += 4;
   }
  }

  /* Note that the payload length is always compressed */

  /* Next header. We compress it is compressable. */
  if(IS_COMPRESSABLE_PROTO(UIP_IP_BUF->proto)) {
    iphc0 |= SICSLOWPAN_IPHC_NH_C;
  }

  /* Add proto header unless it is compressed */
  if((iphc0 & SICSLOWPAN_IPHC_NH_C) == 0) {
    *hc06_ptr = UIP_IP_BUF->proto;
    hc06_ptr += 1;
  }
  /*
   * Hop limit
   * if 1: compress, encoding is 01
   * if 64: compress, encoding is 10
   * if 255: compress, encoding is 11
   * else do not compress
   */
  switch(UIP_IP_BUF->ttl) {
    case 1:
      iphc0 |= SICSLOWPAN_IPHC_TTL_1;
      break;
    case 64:
      iphc0 |= SICSLOWPAN_IPHC_TTL_64;
      break;
    case 255:
      iphc0 |= SICSLOWPAN_IPHC_TTL_255;
      break;
    default:
      *hc06_ptr = UIP_IP_BUF->ttl;
      hc06_ptr += 1;
      break;
  }

  /* source address - cannot be multicast */
  if(uip_is_addr_unspecified(&UIP_IP_BUF->srcipaddr)) {
    PRINTF("IPHC: compressing unspecified - setting SAC\n");
    iphc1 |= SICSLOWPAN_IPHC_SAC;
    iphc1 |= SICSLOWPAN_IPHC_SAM_00;
  } else if((context = addr_context_lookup_by_prefix(&UIP_IP_BUF->srcipaddr))
     != NULL) {
    /* elide the prefix - indicate by CID and set context + SAC */
    PRINTF("IPHC: compressing src with context - setting CID & SAC ctx: %d\n",
           context->number);
    iphc1 |= SICSLOWPAN_IPHC_CID | SICSLOWPAN_IPHC_SAC;
    PACKETBUF_IPHC_BUF[2] |= context->number << 4;
    /* compession compare with this nodes address (source) */

    iphc1 |= compress_addr_64(SICSLOWPAN_IPHC_SAM_BIT,
                              &UIP_IP_BUF->srcipaddr, &uip_lladdr);
    /* No context found for this address */
  } else if(uip_is_addr_linklocal(&UIP_IP_BUF->srcipaddr) &&
            UIP_IP_BUF->destipaddr.u16[1] == 0 &&
            UIP_IP_BUF->destipaddr.u16[2] == 0 &&
            UIP_IP_BUF->destipaddr.u16[3] == 0) {
    iphc1 |= compress_addr_64(SICSLOWPAN_IPHC_SAM_BIT,
                              &UIP_IP_BUF->srcipaddr, &uip_lladdr);
  } else {
    /* send the full address => SAC = 0, SAM = 00 */
    iphc1 |= SICSLOWPAN_IPHC_SAM_00; /* 128-bits */
    memcpy(hc06_ptr, &UIP_IP_BUF->srcipaddr.u16[0], 16);
    hc06_ptr += 16;
  }

  /* dest address*/
  if(uip_is_addr_mcast(&UIP_IP_BUF->destipaddr)) {
    /* Address is multicast, try to compress */
    iphc1 |= SICSLOWPAN_IPHC_M;
    if(sicslowpan_is_mcast_addr_compressable8(&UIP_IP_BUF->destipaddr)) {
      iphc1 |= SICSLOWPAN_IPHC_DAM_11;
      /* use last byte */
      *hc06_ptr = UIP_IP_BUF->destipaddr.u8[15];
      hc06_ptr += 1;
    } else if(sicslowpan_is_mcast_addr_compressable32(&UIP_IP_BUF->destipaddr)) {
      iphc1 |= SICSLOWPAN_IPHC_DAM_10;
      /* second byte + the last three */
      *hc06_ptr = UIP_IP_BUF->destipaddr.u8[1];
      memcpy(hc06_ptr + 1, &UIP_IP_BUF->destipaddr.u8[13], 3);
      hc06_ptr += 4;
    } else if(sicslowpan_is_mcast_addr_compressable48(&UIP_IP_BUF->destipaddr)) {
      iphc1 |= SICSLOWPAN_IPHC_DAM_01;
      /* second byte + the last five */
      *hc06_ptr = UIP_IP_BUF->destipaddr.u8[1];
      memcpy(hc06_ptr + 1, &UIP_IP_BUF->destipaddr.u8[11], 5);
      hc06_ptr += 6;
    } else {
      iphc1 |= SICSLOWPAN_IPHC_DAM_00;
      /* full address */
      memcpy(hc06_ptr, &UIP_IP_BUF->destipaddr.u8[0], 16);
      hc06_ptr += 16;
    }
  } else {
    /* Address is unicast, try to compress */
    if((context = addr_context_lookup_by_prefix(&UIP_IP_BUF->destipaddr)) != NULL) {
      /* elide the prefix */
      iphc1 |= SICSLOWPAN_IPHC_DAC;
      PACKETBUF_IPHC_BUF[2] |= context->number;
      /* compession compare with link adress (destination) */

      iphc1 |= compress_addr_64(SICSLOWPAN_IPHC_DAM_BIT,
                                &UIP_IP_BUF->destipaddr,
                                (uip_lladdr_t *)link_destaddr);
      /* No context found for this address */
    } else if(uip_is_addr_linklocal(&UIP_IP_BUF->destipaddr) &&
              UIP_IP_BUF->destipaddr.u16[1] == 0 &&
              UIP_IP_BUF->destipaddr.u16[2] == 0 &&
              UIP_IP_BUF->destipaddr.u16[3] == 0) {
      iphc1 |= compress_addr_64(SICSLOWPAN_IPHC_DAM_BIT,
               &UIP_IP_BUF->destipaddr, (uip_lladdr_t *)link_destaddr);
    } else {
      /* send the full address */
      iphc1 |= SICSLOWPAN_IPHC_DAM_00; /* 128-bits */
      memcpy(hc06_ptr, &UIP_IP_BUF->destipaddr.u16[0], 16);
      hc06_ptr += 16;
    }
  }

  uncomp_hdr_len = UIP_IPH_LEN;

  /* Start of ext hdr compression or UDP compression */
  /* pick out the next-header position */
  next_hdr = &UIP_IP_BUF->proto;
  next_nhc = hc06_ptr; /* here we set the next header is compressed. */
  ext_hdr_len = 0;
  /* reserve the write place of this next header position */
  while(next_hdr != NULL && IS_COMPRESSABLE_PROTO(*next_hdr)) {\
    int proto = -1; /* used for the specific ext hdr */
    /* UDP and EXT header compression */
    switch(*next_hdr) {
    case UIP_PROTO_HBHO:
       proto = SICSLOWPAN_NHC_ETX_HDR_HBHO;
    case UIP_PROTO_ROUTING:
      proto = proto == -1 ? SICSLOWPAN_NHC_ETX_HDR_ROUTING : proto;
    case UIP_PROTO_FRAG:
      proto = proto == -1 ? SICSLOWPAN_NHC_ETX_HDR_FRAG : proto;
    case UIP_PROTO_DESTO:
      proto = proto == -1 ? SICSLOWPAN_NHC_ETX_HDR_DESTO : proto;
      /* Handle the header here! */
      {
        struct uip_ext_hdr *ext_hdr =
          (struct uip_ext_hdr *) UIP_IPPAYLOAD_BUF(ext_hdr_len);
        int len;
        /* Len is defined to be in octets from the length byte */
        len = (ext_hdr->len << 3) + 8;
        /* pick up the next header */
        next_hdr = &ext_hdr->next;
        /* If the next header is not compressable we need to reserve the
           NHC byte extra - before the next header here. This is due to
           next not being elided in that case. */
        if(!IS_COMPRESSABLE_PROTO(*next_hdr)) {
          //CHECK_BUFFER_SPACE(1);
          hc06_ptr++;
        }
        /* copy the ext-hdr into the hc06 buffer */
        //CHECK_BUFFER_SPACE(len);
        memcpy(hc06_ptr, ext_hdr, len);
        /* modify the len to octets */
        ext_hdr = (struct uip_ext_hdr *) hc06_ptr;
        ext_hdr->len = len - 2; /* Len should be in bytes from len byte*/
        ext_hdr_len += len;
        hc06_ptr += len;
        uncomp_hdr_len += len;

        /* Write this next header - with its NHC header - including flag
           to tell if next header is elided in this one also- */
        *next_nhc = SICSLOWPAN_NHC_EXT_HDR |
          (IS_COMPRESSABLE_PROTO(*next_hdr) ? SICSLOWPAN_NHC_BIT : 0) |
          (proto << 1);
        /* update the position of the next header */
        next_nhc = hc06_ptr;
      }
      break;
    case UIP_PROTO_IPv6:      
      proto = proto == -1 ? SICSLOWPAN_NHC_ETX_HDR_IPV6 : proto;
      total_hdr_len = hc06_ptr - packetbuf_ptr;
      hc06_ptr += 3;
      iphc2 = SICSLOWPAN_DISPATCH_IPHC;
      iphc3 = 0;
      ipv6_comp_buf = (struct uip_ip_hdr *)UIP_IPv6_HDR_COMP(uncomp_hdr_len);
      tmp = (ipv6_comp_buf->vtc << 4) | (ipv6_comp_buf->tcflow >> 4);
      tmp = ((tmp & 0x03) << 6) | (tmp >> 2);
      
      if(((ipv6_comp_buf->tcflow & 0x0F) == 0) &&
         (ipv6_comp_buf->flow == 0)) {
           /* flow label can be compressed */
           iphc2 |= SICSLOWPAN_IPHC_FL_C;
           if(((ipv6_comp_buf->vtc & 0x0F) == 0) &&
              ((ipv6_comp_buf->tcflow & 0xF0) == 0)) {
                /* compress (elide) all */
                iphc2 |= SICSLOWPAN_IPHC_TC_C;
              } else {
                /* compress only the flow label */
                *hc06_ptr = tmp;
                hc06_ptr += 1;
              }
         } else {
           /* Flow label cannot be compressed */
           if(((ipv6_comp_buf->vtc & 0x0F) == 0) &&
              ((ipv6_comp_buf->tcflow & 0xF0) == 0)) {
                /* compress only traffic class */
                iphc2 |= SICSLOWPAN_IPHC_TC_C;
                *hc06_ptr = (tmp & 0xc0) |
                  (ipv6_comp_buf->tcflow & 0x0F);
                memcpy(hc06_ptr + 1, &ipv6_comp_buf->flow, 2);
                hc06_ptr += 3;
              } else {
                /* compress nothing */
                memcpy(hc06_ptr, &ipv6_comp_buf->vtc, 4);
                /* but replace the top byte with the new ECN | DSCP format*/
                *hc06_ptr = tmp;
                hc06_ptr += 4;
              }
         }
   
      /* Next header. We compress it is compressable. */
      if(IS_COMPRESSABLE_PROTO(ipv6_comp_buf->proto)) {
        iphc2 |= SICSLOWPAN_IPHC_NH_C;
      }
      
      /* Add proto header unless it is compressed */
      if((iphc2 & SICSLOWPAN_IPHC_NH_C) == 0) {
        *hc06_ptr = ipv6_comp_buf->proto;
        hc06_ptr += 1;
      }
      /*
      * Hop limit
      * if 1: compress, encoding is 01
      * if 64: compress, encoding is 10
      * if 255: compress, encoding is 11
      * else do not compress
      */
      switch(ipv6_comp_buf->ttl) {
      case 1:
        iphc2 |= SICSLOWPAN_IPHC_TTL_1;
        break;
      case 64:
        iphc2 |= SICSLOWPAN_IPHC_TTL_64;
        break;
      case 255:
        iphc2 |= SICSLOWPAN_IPHC_TTL_255;
        break;
      default:
        *hc06_ptr = ipv6_comp_buf->ttl;
        hc06_ptr += 1;
        break;
      }
      
      /* source address - cannot be multicast */
      if(uip_is_addr_unspecified(&ipv6_comp_buf->srcipaddr)) {
        PRINTF("IPHC: compressing unspecified - setting SAC\n");
        iphc3 |= SICSLOWPAN_IPHC_SAC;
        iphc3 |= SICSLOWPAN_IPHC_SAM_00;
      } else if((context = addr_context_lookup_by_prefix(&ipv6_comp_buf->srcipaddr))
                != NULL) {
                  /* elide the prefix - indicate by CID and set context + SAC */
                  PRINTF("IPHC: compressing src with context - setting CID & SAC ctx: %d\n",
                         context->number);
                  iphc3 |= SICSLOWPAN_IPHC_CID | SICSLOWPAN_IPHC_SAC;
                  PACKETBUF_IPHC_BUF[2] |= context->number << 4;
                  /* compession compare with this nodes address (source) */
                  
                  iphc3 |= compress_addr_64(SICSLOWPAN_IPHC_SAM_BIT,
                                            &ipv6_comp_buf->srcipaddr, &uip_lladdr);
                  /* No context found for this address */
                } else if(uip_is_addr_linklocal(&ipv6_comp_buf->srcipaddr) &&
                          ipv6_comp_buf->destipaddr.u16[1] == 0 &&
                            ipv6_comp_buf->destipaddr.u16[2] == 0 &&
                              ipv6_comp_buf->destipaddr.u16[3] == 0) {
                                iphc3 |= compress_addr_64(SICSLOWPAN_IPHC_SAM_BIT,
                                                          &ipv6_comp_buf->srcipaddr, &uip_lladdr);
                              } else {
                                /* send the full address => SAC = 0, SAM = 00 */
                                iphc3 |= SICSLOWPAN_IPHC_SAM_00; /* 128-bits */
                                memcpy(hc06_ptr, &ipv6_comp_buf->srcipaddr.u16[0], 16);
                                hc06_ptr += 16;
                              }
      
      /* dest address*/
      if(uip_is_addr_mcast(&ipv6_comp_buf->destipaddr)) {
        /* Address is multicast, try to compress */
        iphc3 |= SICSLOWPAN_IPHC_M;
        if(sicslowpan_is_mcast_addr_compressable8(&ipv6_comp_buf->destipaddr)) {
          iphc3 |= SICSLOWPAN_IPHC_DAM_11;
          /* use last byte */
          *hc06_ptr = ipv6_comp_buf->destipaddr.u8[15];
          hc06_ptr += 1;
        } else if(sicslowpan_is_mcast_addr_compressable32(&ipv6_comp_buf->destipaddr)) {
          iphc3 |= SICSLOWPAN_IPHC_DAM_10;
          /* second byte + the last three */
          *hc06_ptr = ipv6_comp_buf->destipaddr.u8[1];
          memcpy(hc06_ptr + 1, &ipv6_comp_buf->destipaddr.u8[13], 3);
          hc06_ptr += 4;
        } else if(sicslowpan_is_mcast_addr_compressable48(&ipv6_comp_buf->destipaddr)) {
          iphc3 |= SICSLOWPAN_IPHC_DAM_01;
          /* second byte + the last five */
          *hc06_ptr = ipv6_comp_buf->destipaddr.u8[1];
          memcpy(hc06_ptr + 1, &ipv6_comp_buf->destipaddr.u8[11], 5);
          hc06_ptr += 6;
        } else {
          iphc3 |= SICSLOWPAN_IPHC_DAM_00;
          /* full address */
          memcpy(hc06_ptr, &ipv6_comp_buf->destipaddr.u8[0], 16);
          hc06_ptr += 16;
        }
      } else {
        /* Address is unicast, try to compress */
        if((context = addr_context_lookup_by_prefix(&ipv6_comp_buf->destipaddr)) != NULL) {
          /* elide the prefix */
          iphc3 |= SICSLOWPAN_IPHC_DAC;
          PACKETBUF_IPHC_BUF[2] |= context->number;
          /* compession compare with link adress (destination) */
          
          iphc3 |= compress_addr_64(SICSLOWPAN_IPHC_DAM_BIT,
                                    &ipv6_comp_buf->destipaddr,
                                    (uip_lladdr_t *)link_destaddr);
          /* No context found for this address */
        } else if(uip_is_addr_linklocal(&ipv6_comp_buf->destipaddr) &&
                  ipv6_comp_buf->destipaddr.u16[1] == 0 &&
                    ipv6_comp_buf->destipaddr.u16[2] == 0 &&
                      ipv6_comp_buf->destipaddr.u16[3] == 0) {
                        iphc3 |= compress_addr_64(SICSLOWPAN_IPHC_DAM_BIT,
                                                  &ipv6_comp_buf->destipaddr, (uip_lladdr_t *)link_destaddr);
                      } else {
                        /* send the full address */
                        iphc3 |= SICSLOWPAN_IPHC_DAM_00; /* 128-bits */
                        memcpy(hc06_ptr, &ipv6_comp_buf->destipaddr.u16[0], 16);
                        hc06_ptr += 16;
                      }
      }
     
      
       ext_hdr = (struct uip_ext_hdr *) UIP_IPPAYLOAD_BUF(ext_hdr_len + UIP_IPH_LEN);
//        int len;
        /* Len is defined to be in octets from the length byte */
        //len = (ext_hdr->len << 3) + 8;
        /* pick up the next header */
        next_hdr = &ext_hdr->next;
        /* If the next header is not compressable we need to reserve the
           NHC byte extra - before the next header here. This is due to
           next not being elided in that case. */
//        if(!IS_COMPRESSABLE_PROTO(*next_hdr)) {
//          //CHECK_BUFFER_SPACE(1);
//          hc06_ptr++;
//        }
        /* copy the ext-hdr into the hc06 buffer */
        //CHECK_BUFFER_SPACE(len);
       // memcpy(hc06_ptr, ext_hdr, len);
        /* modify the len to octets */
        //ext_hdr = (struct uip_ext_hdr *) hc06_ptr;
        //ext_hdr->len = len - 2; /* Len should be in bytes from len byte*/
//        ext_hdr_len += len;
//        hc06_ptr += len;
//        uncomp_hdr_len += len;

        /* Write this next header - with its NHC header - including flag
           to tell if next header is elided in this one also- */
        *next_nhc = SICSLOWPAN_NHC_EXT_HDR |
          (IS_COMPRESSABLE_PROTO(*next_hdr) ? SICSLOWPAN_NHC_BIT : 0) |
          (proto << 1);
        /* update the position of the next header */
        next_nhc = hc06_ptr;
      /*
       struct uip_ext_hdr *ext_hdr =
          (struct uip_ext_hdr *) UIP_IPPAYLOAD_BUF(ext_hdr_len);
        next_hdr = &ipv6_comp_buf->proto;
        *next_nhc = SICSLOWPAN_NHC_EXT_HDR |
          (IS_COMPRESSABLE_PROTO(*next_hdr) ? SICSLOWPAN_NHC_BIT : 0) |
            (proto << 1);
        next_nhc = hc06_ptr;*/ 
      
        PACKETBUF_IPHC_BUF[total_hdr_len+1] = iphc2;
        PACKETBUF_IPHC_BUF[total_hdr_len+2] = iphc3;
        uncomp_hdr_len += UIP_IPH_LEN;// total_hdr_len +1; //((total_hdr_len +1) - ext_hdr_len);
        ext_hdr_len += UIP_IPH_LEN;
        next_hdr = &ipv6_comp_buf->proto;
      break;
	  
    case UIP_PROTO_UDP:
      /* allocate a byte for the next header posision as UDP has no next */
      hc06_ptr++;
      udp_buf = (struct uip_udp_hdr *) UIP_IPPAYLOAD_BUF(ext_hdr_len);
      /* Mask out the last 4 bits can be used as a mask */
      if(((UIP_HTONS(udp_buf->srcport) & 0xfff0) == SICSLOWPAN_UDP_4_BIT_PORT_MIN) &&
         ((UIP_HTONS(udp_buf->destport) & 0xfff0) == SICSLOWPAN_UDP_4_BIT_PORT_MIN)) {
        /* we can compress 12 bits of both source and dest */
        *next_nhc = SICSLOWPAN_NHC_UDP_CS_P_11;
//        CHECK_BUFFER_SPACE(1);
        *hc06_ptr =
          (uint8_t)((UIP_HTONS(udp_buf->srcport) -
                     SICSLOWPAN_UDP_4_BIT_PORT_MIN) << 4) +
          (uint8_t)((UIP_HTONS(udp_buf->destport) -
                     SICSLOWPAN_UDP_4_BIT_PORT_MIN));
        hc06_ptr += 1;
      } else if((UIP_HTONS(udp_buf->destport) & 0xff00) == SICSLOWPAN_UDP_8_BIT_PORT_MIN) {
        /* we can compress 8 bits of dest, leave source. */
        *next_nhc = SICSLOWPAN_NHC_UDP_CS_P_01;
//        CHECK_BUFFER_SPACE(3);
        memcpy(hc06_ptr, &udp_buf->srcport, 2);
        *(hc06_ptr + 2) =
          (uint8_t)((UIP_HTONS(udp_buf->destport) -
                     SICSLOWPAN_UDP_8_BIT_PORT_MIN));
        hc06_ptr += 3;
      } else if((UIP_HTONS(udp_buf->srcport) & 0xff00) == SICSLOWPAN_UDP_8_BIT_PORT_MIN) {
        /* we can compress 8 bits of src, leave dest. Copy compressed port */
        *next_nhc = SICSLOWPAN_NHC_UDP_CS_P_10;
//        CHECK_BUFFER_SPACE(3);
        *hc06_ptr =
          (uint8_t)((UIP_HTONS(udp_buf->srcport) -
                     SICSLOWPAN_UDP_8_BIT_PORT_MIN));
        memcpy(hc06_ptr + 1, &udp_buf->destport, 2);
        hc06_ptr += 3;
      } else {
        /* we cannot compress. Copy uncompressed ports, full checksum  */
        *next_nhc = SICSLOWPAN_NHC_UDP_CS_P_00;
//        CHECK_BUFFER_SPACE(4);
        memcpy(hc06_ptr, &udp_buf->srcport, 4);
        hc06_ptr += 4;
      }
      /* always inline the checksum  */
//      CHECK_BUFFER_SPACE(2);
      memcpy(hc06_ptr, &udp_buf->udpchksum, 2);
      hc06_ptr += 2;
      uncomp_hdr_len += UIP_UDPH_LEN;
      /* this is the final header... */
      next_hdr = NULL;
      break;
    default:
      PRINTF("IPHC: could not handle compression of header");
    }
  }
  if(next_hdr != NULL) {
    /* Last header could not be compressed - we assume that this is then OK!*/
    /* as the last EXT_HDR should be "uncompressed" and have the next there */
  }

#endif /*UIP_CONF_UDP*/
  /* before the packetbuf_hdr_len operation */
  PACKETBUF_IPHC_BUF[0] = iphc0;
  PACKETBUF_IPHC_BUF[1] = iphc1;

  packetbuf_hdr_len = hc06_ptr - packetbuf_ptr;
  return;
}

/*--------------------------------------------------------------------*/
/**
 * \brief Uncompress IPHC (i.e., IPHC and LOWPAN_UDP) headers and put
 * them in sicslowpan_buf
 *
 * This function is called by the input function when the dispatch is
 * IPHC.
 * We %process the packet in the packetbuf buffer, uncompress the header
 * fields, and copy the result in the sicslowpan buffer.
 * At the end of the decompression, packetbuf_hdr_len and uncompressed_hdr_len
 * are set to the appropriate values
 *
 * \param buf Pointer to the buffer to uncompress the packet into.
 * \param ip_len Equal to 0 if the packet is not a fragment (IP length
 * is then inferred from the L2 length), non 0 if the packet is a 1st
 * fragment.
 */
static void
uncompress_hdr_iphc(uint8_t *buf, uint16_t ip_len)
{
  uint8_t tmp, iphc0, iphc1,iphc2,iphc3,nhc;
  uint16_t total_hdr_len = 0;
  uint16_t total_uncompress_len = 0;

/* Suneet :: These pointers should be initialised with NULL, Otherwise in high optimisation these store garbage value */
  struct uip_ext_hdr *exthdr = NULL;
  uint8_t* last_nextheader = NULL;
  uint8_t* ip_payload = NULL;
  struct uip_ip_hdr *ipv6_uncomm_buf = NULL;
  uint8_t ext_hdr_len = 0;

  /* at least two byte will be used for the encoding */
  hc06_ptr = packetbuf_ptr + packetbuf_hdr_len + 2;
  int total_len = 0;
  iphc0 = PACKETBUF_IPHC_BUF[0];
  iphc1 = PACKETBUF_IPHC_BUF[1];

  /* another if the CID flag is set */
  if(iphc1 & SICSLOWPAN_IPHC_CID) {
    PRINTF("IPHC: CID flag set - increase header with one\n");
    hc06_ptr++;
  }

  /* Traffic class and flow label */
    if((iphc0 & SICSLOWPAN_IPHC_FL_C) == 0) {
      /* Flow label are carried inline */
      if((iphc0 & SICSLOWPAN_IPHC_TC_C) == 0) {
        /* Traffic class is carried inline */
        memcpy(&SICSLOWPAN_IP_BUF(buf)->tcflow, hc06_ptr + 1, 3);
        tmp = *hc06_ptr;
        hc06_ptr += 4;
        /* IPHC format of tc is ECN | DSCP , original is DSCP | ECN */
        /* set version, pick highest DSCP bits and set in vtc */
        SICSLOWPAN_IP_BUF(buf)->vtc = 0x60 | ((tmp >> 2) & 0x0f);
        /* ECN rolled down two steps + lowest DSCP bits at top two bits */
        SICSLOWPAN_IP_BUF(buf)->tcflow = ((tmp >> 2) & 0x30) | (tmp << 6) |
	  (SICSLOWPAN_IP_BUF(buf)->tcflow & 0x0f);
      } else {
        /* Traffic class is compressed (set version and no TC)*/
        SICSLOWPAN_IP_BUF(buf)->vtc = 0x60;
        /* highest flow label bits + ECN bits */
        SICSLOWPAN_IP_BUF(buf)->tcflow = (*hc06_ptr & 0x0F) |
          ((*hc06_ptr >> 2) & 0x30);
        memcpy(&SICSLOWPAN_IP_BUF(buf)->flow, hc06_ptr + 1, 2);
        hc06_ptr += 3;
      }
    } else {
      /* Version is always 6! */
      /* Version and flow label are compressed */
      if((iphc0 & SICSLOWPAN_IPHC_TC_C) == 0) {
        /* Traffic class is inline */
	SICSLOWPAN_IP_BUF(buf)->vtc = 0x60 | ((*hc06_ptr >> 2) & 0x0f);
          SICSLOWPAN_IP_BUF(buf)->tcflow = ((*hc06_ptr << 6) & 0xC0) | ((*hc06_ptr >> 2) & 0x30);
          SICSLOWPAN_IP_BUF(buf)->flow = 0;
          hc06_ptr += 1;
      } else {
        /* Traffic class is compressed */
        SICSLOWPAN_IP_BUF(buf)->vtc = 0x60;
        SICSLOWPAN_IP_BUF(buf)->tcflow = 0;
        SICSLOWPAN_IP_BUF(buf)->flow = 0;
      }
    }

  /* Next Header */
  if((iphc0 & SICSLOWPAN_IPHC_NH_C) == 0) {
    /* Next header is carried inline */
    SICSLOWPAN_IP_BUF(buf)->proto = *hc06_ptr;
    PRINTF("IPHC: next header inline: %d\n", SICSLOWPAN_IP_BUF(buf)->proto);
    hc06_ptr += 1;
  }

  /* Hop limit */
  if((iphc0 & 0x03) != SICSLOWPAN_IPHC_TTL_I) {
    SICSLOWPAN_IP_BUF(buf)->ttl = ttl_values[iphc0 & 0x03];
  } else {
    SICSLOWPAN_IP_BUF(buf)->ttl = *hc06_ptr;
    hc06_ptr += 1;
  }

  /* put the source address compression mode SAM in the tmp var */
  tmp = ((iphc1 & SICSLOWPAN_IPHC_SAM_11) >> SICSLOWPAN_IPHC_SAM_BIT) & 0x03;

  /* context based compression */
  if(iphc1 & SICSLOWPAN_IPHC_SAC) {
    uint8_t sci = (iphc1 & SICSLOWPAN_IPHC_CID) ?
      PACKETBUF_IPHC_BUF[2] >> 4 : 0;

    /* Source address - check context != NULL only if SAM bits are != 0*/
    if (tmp != 0) {
      context = addr_context_lookup_by_number(sci);
      if(context == NULL) {
        PRINTF("sicslowpan uncompress_hdr: error context not found\n");
        return;
      }
    }
    /* if tmp == 0 we do not have a context and therefore no prefix */
    uncompress_addr(&SICSLOWPAN_IP_BUF(buf)->srcipaddr,
                    tmp != 0 ? context->prefix : NULL, unc_ctxconf[tmp],
                    (uip_lladdr_t *)packetbuf_addr(PACKETBUF_ADDR_SENDER));
  } else {
    /* no compression and link local */
    uncompress_addr(&SICSLOWPAN_IP_BUF(buf)->srcipaddr, llprefix, unc_llconf[tmp],
                    (uip_lladdr_t *)packetbuf_addr(PACKETBUF_ADDR_SENDER));
  }

  /* Destination address */
  /* put the destination address compression mode into tmp */
  tmp = ((iphc1 & SICSLOWPAN_IPHC_DAM_11) >> SICSLOWPAN_IPHC_DAM_BIT) & 0x03;

  /* multicast compression */
  if(iphc1 & SICSLOWPAN_IPHC_M) {
    /* context based multicast compression */
    if(iphc1 & SICSLOWPAN_IPHC_DAC) {
      /* TODO: implement this */
    } else {
      /* non-context based multicast compression - */
      /* DAM_00: 128 bits  */
      /* DAM_01:  48 bits FFXX::00XX:XXXX:XXXX */
      /* DAM_10:  32 bits FFXX::00XX:XXXX */
      /* DAM_11:   8 bits FF02::00XX */
      uint8_t prefix[] = {0xff, 0x02};
      if(tmp > 0 && tmp < 3) {
        prefix[1] = *hc06_ptr;
        hc06_ptr++;
      }

      uncompress_addr(&SICSLOWPAN_IP_BUF(buf)->destipaddr, prefix,
                      unc_mxconf[tmp], NULL);
    }
  } else {
    /* no multicast */
    /* Context based */
    if(iphc1 & SICSLOWPAN_IPHC_DAC) {
      uint8_t dci = (iphc1 & SICSLOWPAN_IPHC_CID) ? PACKETBUF_IPHC_BUF[2] & 0x0f : 0;
      context = addr_context_lookup_by_number(dci);

      /* all valid cases below need the context! */
      if(context == NULL) {
        PRINTF("sicslowpan uncompress_hdr: error context not found\n");
        return;
      }
      uncompress_addr(&SICSLOWPAN_IP_BUF(buf)->destipaddr, context->prefix,
                      unc_ctxconf[tmp],
                      (uip_lladdr_t *)packetbuf_addr(PACKETBUF_ADDR_RECEIVER));
    } else {
      /* not context based => link local M = 0, DAC = 0 - same as SAC */
      uncompress_addr(&SICSLOWPAN_IP_BUF(buf)->destipaddr, llprefix,
                      unc_llconf[tmp],
                      (uip_lladdr_t *)packetbuf_addr(PACKETBUF_ADDR_RECEIVER));
    }
  }
  uncomp_hdr_len += UIP_IPH_LEN;
  exten_header_offset = UIP_IPH_LEN;
  /* Next header processing - continued */
  nhc = iphc0 & SICSLOWPAN_IPHC_NH_C;
  /* The next header is compressed, NHC is following */
  last_nextheader =  &SICSLOWPAN_IP_BUF(buf)->proto;
  ip_payload = SICSLOWPAN_IPPAYLOAD_BUF(buf);

  while(nhc && (*hc06_ptr & SICSLOWPAN_NHC_MASK) == SICSLOWPAN_NHC_EXT_HDR) {
    uint8_t eid = (*hc06_ptr & 0x0e) >> 1;
    /* next header compression flag */
    uint8_t nh = (*hc06_ptr & 0x01);
    uint8_t next = 0;
    uint8_t len;
    int proto = -1;

    nhc = nh;

    hc06_ptr++;
    if(!nh) {
      next = *hc06_ptr;
      hc06_ptr++;
      PRINTF("Next header is not compressed... next: %d\n", next);
    }
    len = *hc06_ptr;
    hc06_ptr++;

    PRINTF("IPHC: Found extension-header id=%d next=%d len=%d\n", eid, next, len);
    switch(eid) {
    case SICSLOWPAN_NHC_ETX_HDR_HBHO:
      proto = UIP_PROTO_HBHO;
    case SICSLOWPAN_NHC_ETX_HDR_ROUTING:
      proto = proto == -1 ? UIP_PROTO_ROUTING : proto;
    case SICSLOWPAN_NHC_ETX_HDR_FRAG:
      proto = proto == -1 ? UIP_PROTO_FRAG : proto;
    case SICSLOWPAN_NHC_ETX_HDR_DESTO:
      proto = proto == -1 ? UIP_PROTO_DESTO : proto;
      {
        *last_nextheader = proto;
        /* uncompress the extension header */
        exthdr = (struct uip_ext_hdr *)ip_payload;
        exthdr->len = (2 + len) / 8 - 1;
        exthdr->next = next;
        last_nextheader = &exthdr->next;
        memcpy((uint8_t*)exthdr + 2, hc06_ptr, len);
        hc06_ptr += len;
        uncomp_hdr_len += (exthdr->len + 1) * 8;
        ip_payload += (exthdr->len + 1) * 8;
        ext_hdr_len += (exthdr->len + 1) * 8;
        exten_header_offset = uncomp_hdr_len;
        PRINTF("Uncompressed EXT hdr: %d len:%d exhdrlen:%d (calc: %d)\n",
               proto, len, exthdr->len, (exthdr->len + 1) * 8);
      }
      break;
    case SICSLOWPAN_NHC_ETX_HDR_IPV6:
      proto = proto == -1 ? UIP_PROTO_IPv6 : proto;
      ipv6_uncomm_buf = (struct uip_ip_hdr *)ip_payload;
      total_hdr_len = hc06_ptr - (packetbuf_ptr + packetbuf_hdr_len);
      iphc2 = PACKETBUF_IPHC_BUF[total_hdr_len - 2];
      iphc3 = PACKETBUF_IPHC_BUF[total_hdr_len - 1];
      /* Traffic class and flow label */
      if((iphc2 & SICSLOWPAN_IPHC_FL_C) == 0) {
        /* Flow label are carried inline */
        if((iphc2 & SICSLOWPAN_IPHC_TC_C) == 0) {
          /* Traffic class is carried inline */
          memcpy(&ipv6_uncomm_buf->tcflow, hc06_ptr + 1, 3);
          tmp = *hc06_ptr;
          hc06_ptr += 4;
          /* IPHC format of tc is ECN | DSCP , original is DSCP | ECN */
          /* set version, pick highest DSCP bits and set in vtc */
          ipv6_uncomm_buf->vtc = 0x60 | ((tmp >> 2) & 0x0f);
          /* ECN rolled down two steps + lowest DSCP bits at top two bits */
          ipv6_uncomm_buf->tcflow = ((tmp >> 2) & 0x30) | (tmp << 6) |
            (ipv6_uncomm_buf->tcflow & 0x0f);
        } else {
          /* Traffic class is compressed (set version and no TC)*/
          ipv6_uncomm_buf->vtc = 0x60;
          /* highest flow label bits + ECN bits */
          ipv6_uncomm_buf->tcflow = (*hc06_ptr & 0x0F) |
            ((*hc06_ptr >> 2) & 0x30);
          memcpy(&ipv6_uncomm_buf->flow, hc06_ptr + 1, 2);
          hc06_ptr += 3;
        }
      } else {
        /* Version is always 6! */
        /* Version and flow label are compressed */
        if((iphc2 & SICSLOWPAN_IPHC_TC_C) == 0) {
          /* Traffic class is inline */
          ipv6_uncomm_buf->vtc = 0x60 | ((*hc06_ptr >> 2) & 0x0f);
          ipv6_uncomm_buf->tcflow = ((*hc06_ptr << 6) & 0xC0) | ((*hc06_ptr >> 2) & 0x30);
          ipv6_uncomm_buf->flow = 0;
          hc06_ptr += 1;
        } else {
          /* Traffic class is compressed */
          ipv6_uncomm_buf->vtc = 0x60;
          ipv6_uncomm_buf->tcflow = 0;
          ipv6_uncomm_buf->flow = 0;
        }
      }
      
      /* Next Header */
      if((iphc2 & SICSLOWPAN_IPHC_NH_C) == 0) {
        /* Next header is carried inline */
        ipv6_uncomm_buf->proto = *hc06_ptr;
        PRINTF("IPHC: next header inline: %d\n", SICSLOWPAN_IP_BUF(buf)->proto);
        hc06_ptr += 1;
      }
      
      /* Hop limit */
      if((iphc2 & 0x03) != SICSLOWPAN_IPHC_TTL_I) {
        ipv6_uncomm_buf->ttl = ttl_values[iphc2 & 0x03];
      } else {
        ipv6_uncomm_buf->ttl = *hc06_ptr;
        hc06_ptr += 1;
      }
      
      /* put the source address compression mode SAM in the tmp var */
      tmp = ((iphc3 & SICSLOWPAN_IPHC_SAM_11) >> SICSLOWPAN_IPHC_SAM_BIT) & 0x03;
      
      /* context based compression */
      if(iphc3 & SICSLOWPAN_IPHC_SAC) {
        uint8_t sci = (iphc3 & SICSLOWPAN_IPHC_CID) ?
          PACKETBUF_IPHC_BUF[2] >> 4 : 0;
          
          /* Source address - check context != NULL only if SAM bits are != 0*/
          if (tmp != 0) {
            context = addr_context_lookup_by_number(sci);
            if(context == NULL) {
              PRINTF("sicslowpan uncompress_hdr: error context not found\n");
              return;
            }
          }
          /* if tmp == 0 we do not have a context and therefore no prefix */
          uncompress_addr(&ipv6_uncomm_buf->srcipaddr,
                          tmp != 0 ? context->prefix : NULL, unc_ctxconf[tmp],
                          (uip_lladdr_t *)packetbuf_addr(PACKETBUF_ADDR_SENDER));
      } else {
        /* no compression and link local */
        uncompress_addr(&ipv6_uncomm_buf->srcipaddr, llprefix, unc_llconf[tmp],
                        (uip_lladdr_t *)packetbuf_addr(PACKETBUF_ADDR_SENDER));
      }
      
      /* Destination address */
      /* put the destination address compression mode into tmp */
      tmp = ((iphc3 & SICSLOWPAN_IPHC_DAM_11) >> SICSLOWPAN_IPHC_DAM_BIT) & 0x03;
      
      /* multicast compression */
      if(iphc3 & SICSLOWPAN_IPHC_M) {
        /* context based multicast compression */
        if(iphc3 & SICSLOWPAN_IPHC_DAC) {
          /* TODO: implement this */
        } else {
          /* non-context based multicast compression - */
          /* DAM_00: 128 bits  */
          /* DAM_01:  48 bits FFXX::00XX:XXXX:XXXX */
          /* DAM_10:  32 bits FFXX::00XX:XXXX */
          /* DAM_11:   8 bits FF02::00XX */
          uint8_t prefix[] = {0xff, 0x02};
          if(tmp > 0 && tmp < 3) {
            prefix[1] = *hc06_ptr;
            hc06_ptr++;
          }
          
          uncompress_addr(&ipv6_uncomm_buf->destipaddr, prefix,
                          unc_mxconf[tmp], NULL);
        }
      } else {
        /* no multicast */
        /* Context based */
        if(iphc3 & SICSLOWPAN_IPHC_DAC) {
          uint8_t dci = (iphc3 & SICSLOWPAN_IPHC_CID) ? PACKETBUF_IPHC_BUF[2] & 0x0f : 0;
          context = addr_context_lookup_by_number(dci);
          
          /* all valid cases below need the context! */
          if(context == NULL) {
            PRINTF("sicslowpan uncompress_hdr: error context not found\n");
            return;
          }
          uncompress_addr(&ipv6_uncomm_buf->destipaddr, context->prefix,
                          unc_ctxconf[tmp],
                          (uip_lladdr_t *)packetbuf_addr(PACKETBUF_ADDR_RECEIVER));
        } else {
          /* not context based => link local M = 0, DAC = 0 - same as SAC */
          uncompress_addr(&ipv6_uncomm_buf->destipaddr, llprefix,
                          unc_llconf[tmp],
                          (uip_lladdr_t *)packetbuf_addr(PACKETBUF_ADDR_RECEIVER));
        }
      } 
      *last_nextheader = proto;
      /* uncompress the extension header */
      //exthdr = (struct uip_ext_hdr *)ip_payload;
//      exthdr->len = (2 + len) / 8 - 1;
//      exthdr->next = next;
      last_nextheader =  &ipv6_uncomm_buf->proto;
      nhc = ((*hc06_ptr & SICSLOWPAN_NHC_UDP_MASK) == SICSLOWPAN_NHC_UDP_ID);
      total_uncompress_len = (hc06_ptr - packetbuf_ptr);
      uncomp_hdr_len += UIP_IPH_LEN;
      if(ip_len == 0)
      {
        total_len = (packetbuf_datalen() - total_uncompress_len);
      }
      else
      {
        total_len  = (ip_len - uncomp_hdr_len);
       
      }
      ipv6_uncomm_buf->len[0] = total_len >> 8;
      ipv6_uncomm_buf->len[1] = total_len & 0x00FF;
      ip_payload += UIP_IPH_LEN;
      ext_hdr_len += UIP_IPH_LEN;
      break;
    default:
      PRINTF("IPHC: error unsupported extension-header\n");
      return;
    }
  }

  /* The next header is compressed, NHC is following */
  if(nhc && (*hc06_ptr & SICSLOWPAN_NHC_UDP_MASK) == SICSLOWPAN_NHC_UDP_ID) {
    struct uip_udp_hdr *udp_buf = (struct uip_udp_hdr *)ip_payload;
    uint16_t udp_len;
    uint8_t checksum_compressed;
    *last_nextheader = UIP_PROTO_UDP;
    checksum_compressed = *hc06_ptr & SICSLOWPAN_NHC_UDP_CHECKSUMC;
    PRINTF("IPHC: Incoming header value: %i\n", *hc06_ptr);
    switch(*hc06_ptr & SICSLOWPAN_NHC_UDP_CS_P_11) {
    case SICSLOWPAN_NHC_UDP_CS_P_00:
      /* 1 byte for NHC, 4 byte for ports, 2 bytes chksum */
      memcpy(&udp_buf->srcport, hc06_ptr + 1, 2);
      memcpy(&udp_buf->destport, hc06_ptr + 3, 2);
      PRINTF("IPHC: Uncompressed UDP ports (ptr+5): %x, %x\n",
             UIP_HTONS(udp_buf->srcport),
             UIP_HTONS(udp_buf->destport));
      hc06_ptr += 5;
      break;

    case SICSLOWPAN_NHC_UDP_CS_P_01:
      /* 1 byte for NHC + source 16bit inline, dest = 0xF0 + 8 bit inline */
      PRINTF("IPHC: Decompressing destination\n");
      memcpy(&udp_buf->srcport, hc06_ptr + 1, 2);
      udp_buf->destport = UIP_HTONS(SICSLOWPAN_UDP_8_BIT_PORT_MIN + (*(hc06_ptr + 3)));
      PRINTF("IPHC: Uncompressed UDP ports (ptr+4): %x, %x\n",
             UIP_HTONS(udp_buf->srcport), UIP_HTONS(udp_buf->destport));
      hc06_ptr += 4;
      break;

    case SICSLOWPAN_NHC_UDP_CS_P_10:
      /* 1 byte for NHC + source = 0xF0 + 8bit inline, dest = 16 bit inline*/
      PRINTF("IPHC: Decompressing source\n");
      udp_buf->srcport = UIP_HTONS(SICSLOWPAN_UDP_8_BIT_PORT_MIN +
                                   (*(hc06_ptr + 1)));
      memcpy(&udp_buf->destport, hc06_ptr + 2, 2);
      PRINTF("IPHC: Uncompressed UDP ports (ptr+4): %x, %x\n",
             UIP_HTONS(udp_buf->srcport), UIP_HTONS(udp_buf->destport));
      hc06_ptr += 4;
      break;

    case SICSLOWPAN_NHC_UDP_CS_P_11:
      /* 1 byte for NHC, 1 byte for ports */
      udp_buf->srcport = UIP_HTONS(SICSLOWPAN_UDP_4_BIT_PORT_MIN +
                                   (*(hc06_ptr + 1) >> 4));
      udp_buf->destport = UIP_HTONS(SICSLOWPAN_UDP_4_BIT_PORT_MIN +
                                    ((*(hc06_ptr + 1)) & 0x0F));
      PRINTF("IPHC: Uncompressed UDP ports (ptr+2): %x, %x\n",
             UIP_HTONS(udp_buf->srcport), UIP_HTONS(udp_buf->destport));
      hc06_ptr += 2;
      break;
    default:
      PRINTF("sicslowpan uncompress_hdr: error unsupported UDP compression\n");
      return;
    }
    if(!checksum_compressed) { /* has_checksum, default  */
      memcpy(&udp_buf->udpchksum, hc06_ptr, 2);
      hc06_ptr += 2;
      PRINTF("IPHC: sicslowpan uncompress_hdr: checksum included\n");
    } else {
      PRINTF("IPHC: sicslowpan uncompress_hdr: checksum *NOT* included\n");
    }

    /* length field in UDP header (8 byte header + payload) */
    udp_len = 8 + packetbuf_datalen() - (hc06_ptr - packetbuf_ptr);
    udp_buf->udplen = UIP_HTONS(ip_len == 0 ? udp_len :
                                ip_len - UIP_IPH_LEN - ext_hdr_len);
    PRINTF("Setting UDP length: %u (ext: %u) ip_len: %d udp_len:%d\n",
           UIP_HTONS(udp_buf->udplen), ext_hdr_len, ip_len, udp_len);
     
    uncomp_hdr_len += UIP_UDPH_LEN;
    if(ipv6_uncomm_buf != NULL)
    {
      int len = udp_len;
      if(ip_len == 0)
      {
        ipv6_uncomm_buf->len[0] = len >> 8;
        ipv6_uncomm_buf->len[1] = len & 0x00FF;  //Suneet :: added for mantain ipv6_header compression 
      }
      else
      {
        len  = (ip_len - UIP_IPH_LEN - ext_hdr_len);
        ipv6_uncomm_buf->len[0] = len >> 8;
        ipv6_uncomm_buf->len[1] = len & 0x00FF;  //Suneet :: added for mantain ipv6_header compression 
      }
    }
  }

  packetbuf_hdr_len = hc06_ptr - packetbuf_ptr;

  /* IP length field. */
  if(ip_len == 0) {
    int len = packetbuf_datalen() - packetbuf_hdr_len + uncomp_hdr_len - UIP_IPH_LEN;
    PRINTF("IP payload length: %d. %u - %u + %u - %u\n", len,
           packetbuf_datalen(), packetbuf_hdr_len, uncomp_hdr_len, UIP_IPH_LEN);

    /* This is not a fragmented packet */
    SICSLOWPAN_IP_BUF(buf)->len[0] = len >> 8;
    SICSLOWPAN_IP_BUF(buf)->len[1] = len & 0x00FF;
  } else {
    /* This is a 1st fragment */
    SICSLOWPAN_IP_BUF(buf)->len[0] = (ip_len - UIP_IPH_LEN) >> 8;
    SICSLOWPAN_IP_BUF(buf)->len[1] = (ip_len - UIP_IPH_LEN) & 0x00FF;
  }
  /* length field in UDP header */
  if(SICSLOWPAN_IP_BUF(buf)->proto == UIP_PROTO_UDP) {
    memcpy(&SICSLOWPAN_UDP_BUF(buf)->udplen, &SICSLOWPAN_IP_BUF(buf)->len[0], 2);
  }

  return;
}
/** @} */
#endif /* SICSLOWPAN_COMPRESSION == SICSLOWPAN_COMPRESSION_HC06 */

/*--------------------------------------------------------------------*/
/** \name IPv6 dispatch "compression" function
 * @{                                                                 */
/*--------------------------------------------------------------------*/
/* \brief Packets "Compression" when only IPv6 dispatch is used
 *
 * There is no compression in this case, all fields are sent
 * inline. We just add the IPv6 dispatch byte before the packet.
 * \verbatim
 * 0               1                   2                   3
 * 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * | IPv6 Dsp      | IPv6 header and payload ...
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * \endverbatim
 */
static void
compress_hdr_ipv6(linkaddr_t *link_destaddr)
{
  *packetbuf_ptr = SICSLOWPAN_DISPATCH_IPV6;
  packetbuf_hdr_len += SICSLOWPAN_IPV6_HDR_LEN;
  memcpy(packetbuf_ptr + packetbuf_hdr_len, UIP_IP_BUF, UIP_IPH_LEN);
  packetbuf_hdr_len += UIP_IPH_LEN;
  uncomp_hdr_len += UIP_IPH_LEN;
  return;
}
/** @} */

/*--------------------------------------------------------------------*/
/** \name Input/output functions common to all compression schemes
 * @{                                                                 */
/*--------------------------------------------------------------------*/
/**
 * Callback function for the MAC packet sent callback
 */
static void
packet_sent(void *ptr, int status, int transmissions)
{
  uip_ds6_link_neighbor_callback(status, transmissions);

  if(callback != NULL) {
//    callback->output_callback(status); //suneet :: this is use for RIME_SNIFFER
 }  
  last_tx_status = status;
}
/*--------------------------------------------------------------------*/
extern void watchdog_periodic(void);
/**
 * \brief This function is called by the 6lowpan code to send out a
 * packet.
 * \param dest the link layer destination address of the packet
 */
static void
send_packet(linkaddr_t *dest)
{
  /* Set the link layer destination address for the packet as a
   * packetbuf attribute. The MAC layer can access the destination
   * address with the function packetbuf_addr(PACKETBUF_ADDR_RECEIVER).
   */
  packetbuf_set_addr(PACKETBUF_ADDR_RECEIVER, dest);
  
  //const linkaddr_t *dest = packetbuf_addr(PACKETBUF_ADDR_RECEIVER);
  if(!linkaddr_cmp(dest, &linkaddr_null)) {
    /* Update neighbor link statistics */
    link_stats_packet_sent(dest, 0, 1);//santosh, only for unicast packet, 0 - MAC TX OK, 1 - num of tx count    
  } 

#if NETSTACK_CONF_BRIDGE_MODE
  /* This needs to be explicitly set here for bridge mode to work */
  packetbuf_set_addr(PACKETBUF_ADDR_SENDER,(void*)&uip_lladdr);
#endif

  /* Provide a callback function to receive the result of
     a packet transmission. */
 // NETSTACK_LLSEC.send(&packet_sent, NULL);
 NETSTACK_MAC.send(&packet_sent, NULL);//goes to send_packet()...in mac_2_6lp_data_handler


  /* If we are sending multiple packets in a row, we need to let the
     watchdog know that we are still alive. */
#ifndef EFM32LG295F256
  //watchdog_periodic();
#endif  
}
/*--------------------------------------------------------------------*/
/** \brief Take an IP packet and format it to be sent on an 802.15.4
 *  network using 6lowpan.
 *  \param localdest The MAC address of the destination
 *
 *  The IP packet is initially in uip_buf. Its header is compressed
 *  and if necessary it is fragmented. The resulting
 *  packet/fragments are put in packetbuf and delivered to the 802.15.4
 *  MAC.
 */

static uint8_t
output(const uip_lladdr_t *localdest)
{
//  int framer_hdrlen;

  /* The MAC address of the destination of the packet */
  linkaddr_t dest;

//  uint16_t processed_ip_out_len;
  uint16_t max_mac_payload  = max_frag_size;
  /* init */
  uncomp_hdr_len = 0;
  packetbuf_hdr_len = 0;

  uint8_t packet_type = packetbuf_attr(PACKETBUF_ATTR_PACKET_TYPE);
  /* reset packetbuf buffer */
  packetbuf_clear();
  packetbuf_ptr = packetbuf_dataptr();

  packetbuf_set_attr(PACKETBUF_ATTR_PACKET_TYPE,packet_type);
  if(callback) {
    /* call the attribution when the callback comes, but set attributes
       here ! */
    set_packet_attrs();
  }

#if PACKETBUF_WITH_PACKET_TYPE
#define TCP_FIN 0x01
#define TCP_ACK 0x10
#define TCP_CTL 0x3f
  /* Set stream mode for all TCP packets, except FIN packets. */
  if(UIP_IP_BUF->proto == UIP_PROTO_TCP &&
     (UIP_TCP_BUF->flags & TCP_FIN) == 0 &&
     (UIP_TCP_BUF->flags & TCP_CTL) != TCP_ACK) {
    packetbuf_set_attr(PACKETBUF_ATTR_PACKET_TYPE,
                       PACKETBUF_ATTR_PACKET_TYPE_STREAM);
  } else if(UIP_IP_BUF->proto == UIP_PROTO_TCP &&
            (UIP_TCP_BUF->flags & TCP_FIN) == TCP_FIN) {
    packetbuf_set_attr(PACKETBUF_ATTR_PACKET_TYPE,
                       PACKETBUF_ATTR_PACKET_TYPE_STREAM_END);
  }
#endif

  /*
   * The destination address will be tagged to each outbound
   * packet. If the argument localdest is NULL, we are sending a
   * broadcast packet.
   */
  if(localdest == NULL) {
    linkaddr_copy(&dest, &linkaddr_null);
  } else {
    linkaddr_copy(&dest, (const linkaddr_t *)localdest);
  }

  PRINTFO("sicslowpan output: sending packet len %d\n", uip_len);

  if(uip_len > COMPRESSION_THRESHOLD) {
    /* Try to compress the headers */
#if SICSLOWPAN_COMPRESSION == SICSLOWPAN_COMPRESSION_IPV6
    compress_hdr_ipv6(&dest);
#endif /* SICSLOWPAN_COMPRESSION == SICSLOWPAN_COMPRESSION_IPV6 */
#if SICSLOWPAN_COMPRESSION == SICSLOWPAN_COMPRESSION_HC06
    compress_hdr_iphc(&dest);
#endif /* SICSLOWPAN_COMPRESSION == SICSLOWPAN_COMPRESSION_HC06 */
  } else {
    compress_hdr_ipv6(&dest);
  }
  PRINTFO("sicslowpan output: header of len %d\n", packetbuf_hdr_len);
 
 //
#if 0  //Suneet :: if 0 we use it on fan stack for fragment enable this when want to use econet-light packet fragment 
  if( packetbuf_ptr[1]&SICSLOWPAN_IPHC_M ){
    //since the dest ip address is a multicast address, dest address at mac will be broadcast
    max_mac_payload = MAX_FRAME_SIZE_CAPABILITY - MAC_HDR_SIZE_WO_SEC_BCAST - FCS_SIZE;
  }
  else{
    //max_mac_payload = MAX_FRAME_SIZE_CAPABILITY - MAC_HDR_SIZE_WO_SEC_UCAST/*21*/ - FCS_SIZE/*2*/;
    max_mac_payload = max_frag_size/*255*/ - MAC_HDR_SIZE_WO_SEC_UCAST/*21*/ - FCS_SIZE/*2*/;
  }
#endif 
  
#if 0 // Raka : 03-Sept-2015... Not Needed since we are currently not using NETSTACK_FRAMER and NETSTACK_RDC
  /* Calculate NETSTACK_FRAMER's header length, that will be added in the NETSTACK_RDC.
   * We calculate it here only to make a better decision of whether the outgoing packet
   * needs to be fragmented or not. */
#ifndef SICSLOWPAN_USE_FIXED_HDRLEN
  packetbuf_set_addr(PACKETBUF_ADDR_RECEIVER, &dest);
  framer_hdrlen = NETSTACK_FRAMER.length();
  if(framer_hdrlen < 0) {
    /* Framing failed, we assume the maximum header length */
    framer_hdrlen = SICSLOWPAN_FIXED_HDRLEN;
  }
#else /* USE_FRAMER_HDRLEN */
  framer_hdrlen = SICSLOWPAN_FIXED_HDRLEN;
#endif /* USE_FRAMER_HDRLEN */

  max_payload = max_frag_size - framer_hdrlen;

#endif // Raka :: for if 0
  if((int)uip_len - (int)uncomp_hdr_len > max_mac_payload - (int)packetbuf_hdr_len) {
#if SICSLOWPAN_CONF_FRAG
//#ifdef SICSLOWPAN_CONF_FRAG/*Umesh changed here*/
    /* Number of bytes processed. */
    uint16_t processed_ip_out_len;

    struct queuebuf *q;
    uint16_t frag_tag;

    /*
     * The outbound IPv6 packet is too large to fit into a single 15.4
     * packet, so we fragment it into multiple packets and send them.
     * The first fragment contains frag1 dispatch, then
     * IPv6/IPHC/HC_UDP dispatchs/headers.
     * The following fragments contain only the fragn dispatch.
     */
    int estimated_fragments = ((int)uip_len) / (frag_chunk_send - SICSLOWPAN_FRAGN_HDR_LEN) + 1;
    int freebuf = queuebuf_numfree() - 1;
    PRINTFO("uip_len: %d, fragments: %d, free bufs: %d\n", uip_len, estimated_fragments, freebuf);
    if(freebuf < estimated_fragments) {
      PRINTFO("Dropping packet, not enough free bufs\n");
      return 0;
    }
    
#if(FAN_EDFE_FEATURE_ENABLED == 1)
    send_status_to_edfe_fragment(estimated_fragments); //suneet ::   not supported yet 
#endif
    
    PRINTFO("Fragmentation sending packet len %d\n", uip_len);

    /* Create 1st Fragment */
    PRINTFO("sicslowpan output: 1rst fragment ");

    /* Reset last tx status to ok in case the fragment transmissions are deferred */
    last_tx_status = MAC_TX_OK;

    /* move IPHC/IPv6 header */
    memmove(packetbuf_ptr + SICSLOWPAN_FRAG1_HDR_LEN/*4*/, packetbuf_ptr, packetbuf_hdr_len);

    /*
     * FRAG1 dispatch + header
     * Note that the length is in units of 8 bytes
     */
/*     PACKETBUF_FRAG_BUF->dispatch_size = */
/*       uip_htons((SICSLOWPAN_DISPATCH_FRAG1 << 8) | uip_len); */
    SET16(PACKETBUF_FRAG_PTR, PACKETBUF_FRAG_DISPATCH_SIZE,
          ((SICSLOWPAN_DISPATCH_FRAG1 << 8) | uip_len));
    if(my_tag == 0xFFFF)
    {
      my_tag = 0;
    }
    frag_tag = my_tag++;
    SET16(PACKETBUF_FRAG_PTR, PACKETBUF_FRAG_TAG, frag_tag);

    /* Copy payload and send */
    packetbuf_hdr_len += SICSLOWPAN_FRAG1_HDR_LEN;
    packetbuf_payload_len = (frag_chunk_send - packetbuf_hdr_len) & 0xfffffff8;
    //packetbuf_payload_len = (frag_chunk_send - packetbuf_hdr_len);
    PRINTFO("(len %d, tag %d)\n", packetbuf_payload_len, frag_tag);
    memcpy(packetbuf_ptr + packetbuf_hdr_len,
           (uint8_t *)UIP_IP_BUF + uncomp_hdr_len, packetbuf_payload_len);
    packetbuf_set_datalen(packetbuf_payload_len + packetbuf_hdr_len);
    q = queuebuf_new_from_packetbuf();
    if(q == NULL) {
      PRINTFO("could not allocate queuebuf for first fragment, dropping packet\n");
      return 0;
    }
    send_packet(&dest);
    queuebuf_to_packetbuf(q);
    queuebuf_free(q);
    q = NULL;

    /* Check tx result. */
    if((last_tx_status == MAC_TX_COLLISION) ||
       (last_tx_status == MAC_TX_ERR) ||
       (last_tx_status == MAC_TX_ERR_FATAL)) {
      PRINTFO("error in fragment tx, dropping subsequent fragments.\n");
      return 0;
    }
    
    /* set processed_ip_out_len to what we already sent from the IP payload*/
    processed_ip_out_len = packetbuf_payload_len + uncomp_hdr_len;

    /*
     * Create following fragments
     * Datagram tag is already in the buffer, we need to set the
     * FRAGN dispatch and for each fragment, the offset
     */
    packetbuf_hdr_len = SICSLOWPAN_FRAGN_HDR_LEN;
/*     PACKETBUF_FRAG_BUF->dispatch_size = */
/*       uip_htons((SICSLOWPAN_DISPATCH_FRAGN << 8) | uip_len); */
    SET16(PACKETBUF_FRAG_PTR, PACKETBUF_FRAG_DISPATCH_SIZE,
          ((SICSLOWPAN_DISPATCH_FRAGN << 8) | uip_len));
    packetbuf_payload_len = (frag_chunk_send - packetbuf_hdr_len) & 0xfffffff8;
    //packetbuf_payload_len = (frag_chunk_send - packetbuf_hdr_len);
    while(processed_ip_out_len < uip_len) {
      PRINTFO("sicslowpan output: fragment ");
      PACKETBUF_FRAG_PTR[PACKETBUF_FRAG_OFFSET] = processed_ip_out_len >> 3;

      /* Copy payload and send */
      if(uip_len - processed_ip_out_len < packetbuf_payload_len) {
        /* last fragment */
        packetbuf_payload_len = uip_len - processed_ip_out_len;
      }
      PRINTFO("(offset %d, len %d, tag %d)\n",
             processed_ip_out_len >> 3, packetbuf_payload_len, frag_tag);
      memcpy(packetbuf_ptr + packetbuf_hdr_len,
             (uint8_t *)UIP_IP_BUF + processed_ip_out_len, packetbuf_payload_len);
      packetbuf_set_datalen(packetbuf_payload_len + packetbuf_hdr_len);
      q = queuebuf_new_from_packetbuf();
      if(q == NULL) {
        PRINTFO("could not allocate queuebuf, dropping fragment\n");
        return 0;
      }
      send_packet(&dest);
      queuebuf_to_packetbuf(q);
      queuebuf_free(q);
      q = NULL;
      processed_ip_out_len += packetbuf_payload_len;

      /* Check tx result. */
      if((last_tx_status == MAC_TX_COLLISION) ||
         (last_tx_status == MAC_TX_ERR) ||
         (last_tx_status == MAC_TX_ERR_FATAL)) {
        PRINTFO("error in fragment tx, dropping subsequent fragments.\n");
        return 0;
      }
    }
#else /* SICSLOWPAN_CONF_FRAG */
    PRINTFO("sicslowpan output: Packet too large to be sent without fragmentation support; dropping packet\n");
    return 0;
#endif /* SICSLOWPAN_CONF_FRAG */
  } else {

    /*
     * The packet does not need to be fragmented
     * copy "payload" and send
     */
    memcpy(packetbuf_ptr + packetbuf_hdr_len, (uint8_t *)UIP_IP_BUF + uncomp_hdr_len,
           uip_len - uncomp_hdr_len);
    packetbuf_set_datalen(uip_len - uncomp_hdr_len + packetbuf_hdr_len);
    send_packet(&dest);
  }
  return 1;
}

/*--------------------------------------------------------------------*/
/** \brief Process a received 6lowpan packet.
 *
 *  The 6lowpan packet is put in packetbuf by the MAC. If its a frag1 or
 *  a non-fragmented packet we first uncompress the IP header. The
 *  6lowpan payload and possibly the uncompressed IP header are then
 *  copied in siclowpan_buf. If the IP packet is complete it is copied
 *  to uip_buf and the IP layer is called.
 *
 * \note We do not check for overlapping sicslowpan fragments
 * (it is a SHALL in the RFC 4944 and should never happen)
 */
static void
input(void)
{
  /* size of the IP packet (read from fragment) */
  uint16_t frag_size = 0;
  /* offset of the fragment in the IP packet */
  uint8_t frag_offset = 0;
  uint8_t *buffer;

#if SICSLOWPAN_CONF_FRAG
//#ifdef SICSLOWPAN_CONF_FRAG/*Umesh changed here*/
  uint8_t is_fragment = 0;
  int8_t frag_context = 0;

  /* tag of the fragment */     
  uint16_t frag_tag = 0;
  uint8_t first_fragment = 0, last_fragment = 0;
#endif /*SICSLOWPAN_CONF_FRAG*/

  /* Update link statistics *///updating rssi
//  link_stats_input_callback(packetbuf_addr(PACKETBUF_ADDR_SENDER));

  /* init */
  uncomp_hdr_len = 0;
  packetbuf_hdr_len = 0;

  /* The MAC puts the 15.4 payload inside the packetbuf data buffer */
  packetbuf_ptr = packetbuf_dataptr();

  /* This is default uip_buf since we assume that this is not fragmented */
  buffer = (uint8_t *)UIP_IP_BUF;

  /* Save the RSSI of the incoming packet in case the upper layer will
     want to query us for it later. */
  last_rssi = (signed short)packetbuf_attr(PACKETBUF_ATTR_RSSI);

#if SICSLOWPAN_CONF_FRAG
//#ifdef SICSLOWPAN_CONF_FRAG/*Umesh changed here*/

  /*
   * Since we don't support the mesh and broadcast header, the first header
   * we look for is the fragmentation header
   */
  switch((GET16(PACKETBUF_FRAG_PTR, PACKETBUF_FRAG_DISPATCH_SIZE) & 0xf800) >> 8) {
    case SICSLOWPAN_DISPATCH_FRAG1:
      PRINTFI("sicslowpan input: FRAG1 ");
      frag_offset = 0;
/*       frag_size = (uip_ntohs(PACKETBUF_FRAG_BUF->dispatch_size) & 0x07ff); */
      frag_size = GET16(PACKETBUF_FRAG_PTR, PACKETBUF_FRAG_DISPATCH_SIZE) & 0x07ff;
/*       frag_tag = uip_ntohs(PACKETBUF_FRAG_BUF->tag); */
      frag_tag = GET16(PACKETBUF_FRAG_PTR, PACKETBUF_FRAG_TAG);
      PRINTFI("size %d, tag %d, offset %d)\n",
             frag_size, frag_tag, frag_offset);
      packetbuf_hdr_len += SICSLOWPAN_FRAG1_HDR_LEN;
      /*      printf("frag1 %d %d\n", reass_tag, frag_tag);*/
      first_fragment = 1;
      
      is_fragment = 1;

      /* Add the fragment to the fragmentation context */
      frag_context = add_fragment(frag_tag, frag_size, frag_offset);

      if(frag_context == -1) {
        return;
      }

      //buffer = frag_info[frag_context].first_frag;

      break;
    case SICSLOWPAN_DISPATCH_FRAGN:
      /*
       * set offset, tag, size
       * Offset is in units of 8 bytes
       */
      PRINTFI("sicslowpan input: FRAGN ");
      frag_offset = PACKETBUF_FRAG_PTR[PACKETBUF_FRAG_OFFSET];
      frag_tag = GET16(PACKETBUF_FRAG_PTR, PACKETBUF_FRAG_TAG);
      frag_size = GET16(PACKETBUF_FRAG_PTR, PACKETBUF_FRAG_DISPATCH_SIZE) & 0x07ff;
      PRINTFI("size %d, tag %d, offset %d)\n",
             frag_size, frag_tag, frag_offset);
      packetbuf_hdr_len += SICSLOWPAN_FRAGN_HDR_LEN;

      /* If this is the last fragment, we may shave off any extrenous
         bytes at the end. We must be liberal in what we accept. */
      PRINTFI("last_fragment?: packetbuf_payload_len %d frag_size %d\n",
              packetbuf_datalen() - packetbuf_hdr_len, frag_size);

      /* Add the fragment to the fragmentation context (this will also
         copy the payload) */
      frag_context = add_fragment(frag_tag, frag_size, frag_offset);

      if(frag_context == -1) {
        return;
      }

      /* Ok - add_fragment will store the fragment automatically - so
         we should not store more */
      buffer = NULL;

      if(frag_info[frag_context].reassembled_len >= frag_size) {
        last_fragment = 1;
      }
      is_fragment = 1;
      break;
    default:
      break;
  }

  if(is_fragment && !first_fragment) {
    /* this is a FRAGN, skip the header compression dispatch section */
    goto copypayload;
  }
#endif /* SICSLOWPAN_CONF_FRAG */

  /* Process next dispatch and headers */
#if SICSLOWPAN_COMPRESSION == SICSLOWPAN_COMPRESSION_HC06
  if((PACKETBUF_HC1_PTR[PACKETBUF_HC1_DISPATCH] & 0xe0) == SICSLOWPAN_DISPATCH_IPHC) {
    PRINTFI("sicslowpan input: IPHC\n");
    uncompress_hdr_iphc(buffer, frag_size);
  } else
#endif /* SICSLOWPAN_COMPRESSION == SICSLOWPAN_COMPRESSION_HC06 */
    switch(PACKETBUF_HC1_PTR[PACKETBUF_HC1_DISPATCH]) {
    case SICSLOWPAN_DISPATCH_IPV6:
      PRINTFI("sicslowpan input: IPV6\n");
      packetbuf_hdr_len += SICSLOWPAN_IPV6_HDR_LEN;

      /* Put uncompressed IP header in sicslowpan_buf. */
      memcpy(buffer, packetbuf_ptr + packetbuf_hdr_len, UIP_IPH_LEN);

      /* Update uncomp_hdr_len and packetbuf_hdr_len. */
      packetbuf_hdr_len += UIP_IPH_LEN;
      uncomp_hdr_len += UIP_IPH_LEN;
      break;
    default:
      /* unknown header */
      PRINTFI("sicslowpan input: unknown dispatch: %u\n",
             PACKETBUF_HC1_PTR[PACKETBUF_HC1_DISPATCH]);
      return;
  }


#if SICSLOWPAN_CONF_FRAG
//#ifdef SICSLOWPAN_CONF_FRAG/*Umesh changed here*/
 copypayload:
#endif /*SICSLOWPAN_CONF_FRAG*/
  /*
   * copy "payload" from the packetbuf buffer to the sicslowpan_buf
   * if this is a first fragment or not fragmented packet,
   * we have already copied the compressed headers, uncomp_hdr_len
   * and packetbuf_hdr_len are non 0, frag_offset is.
   * If this is a subsequent fragment, this is the contrary.
   */
  if(packetbuf_datalen() < packetbuf_hdr_len) {
    PRINTF("SICSLOWPAN: packet dropped due to header > total packet\n");
    return;
  }
  packetbuf_payload_len = packetbuf_datalen() - packetbuf_hdr_len;

  /* Sanity-check size of incoming packet to avoid buffer overflow */
  {
    int req_size = UIP_LLH_LEN + uncomp_hdr_len + (uint16_t)(frag_offset << 3)
        + packetbuf_payload_len;
    /*Suneet :: uip_buf = MAX BUFFER "UIP_MAX_CONF_BUFFER_SIZE'*/
    if(req_size > sizeof(uip_buf)) {
      PRINTF(
          "SICSLOWPAN: packet dropped, minimum required IP_BUF size: %d+%d+%d+%d=%d (current size: %u)\n",
          UIP_LLH_LEN, uncomp_hdr_len, (uint16_t)(frag_offset << 3),
          packetbuf_payload_len, req_size, (unsigned)sizeof(uip_buf));
      return;
    }
  }

  /* copy the payload if buffer is non-null - which is only the case with first fragment
     or packets that are non fragmented */
  if(buffer != NULL) {
    memcpy((uint8_t *)buffer + uncomp_hdr_len, packetbuf_ptr + packetbuf_hdr_len, packetbuf_payload_len);
  }

  /* update processed_ip_in_len if fragment, sicslowpan_len otherwise */

#if SICSLOWPAN_CONF_FRAG
//#ifdef SICSLOWPAN_CONF_FRAG/*Umesh changed here*/
  if(frag_size > 0) {
    /* Add the size of the header only for the first fragment. */
    if(first_fragment != 0) {
      frag_info[frag_context].reassembled_len = uncomp_hdr_len + packetbuf_payload_len;
      frag_info[frag_context].first_frag_len = uncomp_hdr_len + packetbuf_payload_len;
      if(frag_info[frag_context].first_frag == NULL)
      {
        frag_info[frag_context].first_frag = (uint8_t *)app_bm_alloc(frag_info[frag_context].first_frag_len);
        memcpy(frag_info[frag_context].first_frag,buffer,frag_info[frag_context].first_frag_len);
      }
    }
    /* For the last fragment, we are OK if there is extrenous bytes at
       the end of the packet. */
    if(last_fragment != 0) {
      frag_info[frag_context].reassembled_len = frag_size;
      /* copy to uip */
      copy_frags2uip(frag_context);
    }
  }

  /*
   * If we have a full IP packet in sicslowpan_buf, deliver it to
   * the IP stack
   */
  if(!is_fragment || last_fragment) {
    /* packet is in uip already - just set length */
    if(is_fragment != 0 && last_fragment != 0) {
      uip_len = frag_size;
    } else {
      uip_len = packetbuf_payload_len + uncomp_hdr_len;
    }
#else
    uip_len = packetbuf_payload_len + uncomp_hdr_len;
#endif /* SICSLOWPAN_CONF_FRAG */
    PRINTFI("sicslowpan input: IP packet ready (length %d)\n",
	    uip_len);

#if DEBUG
    {
      uint16_t ndx;
      PRINTF("after decompression %u:", UIP_IP_BUF->len[1]);
      for (ndx = 0; ndx < UIP_IP_BUF->len[1] + 40; ndx++) {
        uint8_t data = ((uint8_t *) (UIP_IP_BUF))[ndx];
        PRINTF("%02x", data);
      }
      PRINTF("\n");
    }
#endif

    /* if callback is set then set attributes and call */
    if(callback) {
      set_packet_attrs();
      //callback->input_callback();   //suneet :: this is use for RIME_SNIFFER
    }

    tcpip_input();
#if SICSLOWPAN_CONF_FRAG
//#ifdef SICSLOWPAN_CONF_FRAG/*Umesh changed here*/
  }
#endif /* SICSLOWPAN_CONF_FRAG */
}
/** @} */

/*--------------------------------------------------------------------*/
/* \brief 6lowpan init function (called by the MAC layer)             */
/*--------------------------------------------------------------------*/
void
sicslowpan_init(void)
{
  /*
   * Set out output function as the function to be called from uIP to
   * send a packet.
   */

  tcpip_set_outputfunc(output);
  my_tag = rand() % 65535;
#if SICSLOWPAN_COMPRESSION == SICSLOWPAN_COMPRESSION_HC06
/* Preinitialize any address contexts for better header compression
 * (Saves up to 13 bytes per 6lowpan packet)
 * The platform contiki-conf.h file can override this using e.g.
 * #define SICSLOWPAN_CONF_ADDR_CONTEXT_0 {addr_contexts[0].prefix[0]=0xbb;addr_contexts[0].prefix[1]=0xbb;}
 */
#if SICSLOWPAN_CONF_MAX_ADDR_CONTEXTS > 0
  addr_contexts[0].used   = 1;
  addr_contexts[0].number = 0;
#ifdef SICSLOWPAN_CONF_ADDR_CONTEXT_0
  SICSLOWPAN_CONF_ADDR_CONTEXT_0;
#else
  addr_contexts[0].prefix[0] = UIP_DS6_DEFAULT_PREFIX_0;
  addr_contexts[0].prefix[1] = UIP_DS6_DEFAULT_PREFIX_1;
#endif
#endif /* SICSLOWPAN_CONF_MAX_ADDR_CONTEXTS > 0 */

#if SICSLOWPAN_CONF_MAX_ADDR_CONTEXTS > 1
  {
    int i;
    for(i = 1; i < SICSLOWPAN_CONF_MAX_ADDR_CONTEXTS; i++) {
#ifdef SICSLOWPAN_CONF_ADDR_CONTEXT_1
      if (i==1) {
        addr_contexts[1].used   = 1;
        addr_contexts[1].number = 1;
        SICSLOWPAN_CONF_ADDR_CONTEXT_1;
#ifdef SICSLOWPAN_CONF_ADDR_CONTEXT_2
      } else if (i==2) {
        addr_contexts[2].used   = 1;
        addr_contexts[2].number = 2;
        SICSLOWPAN_CONF_ADDR_CONTEXT_2;
#endif
      } else {
        addr_contexts[i].used = 0;
      }
#else
      addr_contexts[i].used = 0;
#endif /* SICSLOWPAN_CONF_ADDR_CONTEXT_1 */
    }
  }
#endif /* SICSLOWPAN_CONF_MAX_ADDR_CONTEXTS > 1 */

#endif /* SICSLOWPAN_COMPRESSION == SICSLOWPAN_COMPRESSION_HC06 */
}
/*--------------------------------------------------------------------*/
int
sicslowpan_get_last_rssi(void)
{
  return last_rssi;
}
/*--------------------------------------------------------------------*/
const struct network_driver sicslowpan_driver = {
  "sicslowpan",
  sicslowpan_init,
  input
};
/*--------------------------------------------------------------------*/
uint8_t get_uncomp_hdr_len (void)
{
  return exten_header_offset;
}
/** @} */

